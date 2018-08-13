/*
 * pfs.c
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#include "pfs.h"
#include "pfs_boot_sector.h"
#include "pfs_conf.h"
#include "pfs_directory_record.h"
#include "pfs_file_system_object.h"
#include <string.h>

static tFileSystemObject fileSystem;

static void pfsReadSector(uint32_t offset, uint8_t *buffer);
static void pfsReadFatSector(uint8_t fatNumber, uint32_t firstDword, uint32_t offset, uint8_t *buffer);
static void pfsWriteSector(uint32_t offset, const uint8_t *buffer);

void pfsInitialize(uint8_t filesCount, tFileInfo *fileInfoArray)
{
  uint16_t cluster = 2;
  time_t initializationTime = 0;
  uint32_t dataAreaOffset = (PFS_RESERVED_SECTORS + (PFS_SECTORS_PER_FAT * 2)) * PFS_BYTES_PER_SECTOR + (PFS_ROOT_ENTRIES * 32);

  for (int32_t i = 0; i < filesCount; i++) {
    // Initialize data for FAT
    fileInfoArray[i].startCluster = cluster;
    uint16_t clustersCount = fileInfoArray[i].size / PFS_BYTES_PER_CLUSTER;

    if ((clustersCount * PFS_BYTES_PER_CLUSTER) < fileInfoArray[i].size) {
      clustersCount++;
    }

    fileInfoArray[i].clustersCount = clustersCount;

    // Initialize data for RootDirectory
    directoryRecordClear(&fileInfoArray[i].directoryRecord);
    directoryRecordSetCreationDateTime(&fileInfoArray[i].directoryRecord, &initializationTime);
    directoryRecordSetAccessDate(&fileInfoArray[i].directoryRecord, &initializationTime);
    directoryRecordSetWriteDateTime(&fileInfoArray[i].directoryRecord, &initializationTime);
    fileInfoArray[i].directoryRecord.attributes = AttrArchive | AttrReadOnly;
    memcpy(fileInfoArray[i].directoryRecord.name, fileInfoArray[i].name, 8);
    memcpy(fileInfoArray[i].directoryRecord.extension, fileInfoArray[i].extension, 3);
    fileInfoArray[i].directoryRecord.fileSize = fileInfoArray[i].size;
    fileInfoArray[i].directoryRecord.startClusterLow = fileInfoArray[i].startCluster;
    fileInfoArray[i].dataAreaOffset = dataAreaOffset + ((fileInfoArray[i].startCluster - 2) * PFS_BYTES_PER_CLUSTER);
    fileInfoArray[i].dataAreaLength = fileInfoArray[i].clustersCount * PFS_BYTES_PER_CLUSTER;

    cluster += clustersCount;
  }

  fileSystem.bootSector = &bootSector;
  fileSystem.fileInfoArray = fileInfoArray;
  fileSystem.filesCount = filesCount;
  fileSystem.dataAreaOffset = dataAreaOffset;
  fileSystem.dataAreaLength = ((uint32_t)PFS_TOTAL_SECTORS * (uint32_t)PFS_BYTES_PER_SECTOR) - dataAreaOffset;
  fileSystem.fat1Offset = PFS_RESERVED_SECTORS * PFS_BYTES_PER_SECTOR;
  fileSystem.fat2Offset = fileSystem.fat1Offset + (PFS_SECTORS_PER_FAT * PFS_BYTES_PER_SECTOR);
  fileSystem.fat1Length = fileSystem.fat2Length = PFS_SECTORS_PER_FAT * PFS_BYTES_PER_SECTOR;
  fileSystem.fat1FirstDword = fileSystem.fat2FirstDword = 0xfffffff8ul;
  fileSystem.rootOffset = fileSystem.fat2Offset + fileSystem.fat2Length;
  fileSystem.rootLength = PFS_ROOT_ENTRIES * sizeof(tDirectoryRecord);
}

void pfsRead(uint32_t offset, uint32_t count, uint8_t *buffer)
{
  // Requested array must be aligned to sectors.
  if (((offset % PFS_BYTES_PER_SECTOR) != 0) || ((count % PFS_BYTES_PER_SECTOR) != 0)) {
    return;
  }

  uint32_t sectorsCount = count / PFS_BYTES_PER_SECTOR;

  for (uint32_t i = 0; i < sectorsCount; i++) {
    pfsReadSector(offset + (i * PFS_BYTES_PER_SECTOR), buffer);
  }
}

void pfsWrite(uint32_t offset, uint32_t count, const uint8_t *buffer)
{
  // Requested array must be aligned to sectors.
  if (((offset % PFS_BYTES_PER_SECTOR) != 0) || ((count % PFS_BYTES_PER_SECTOR) != 0)) {
    return;
  }

  uint32_t sectorsCount = count / PFS_BYTES_PER_SECTOR;

  for (uint32_t i = 0; i < sectorsCount; i++) {
    pfsWriteSector(offset + (i * PFS_BYTES_PER_SECTOR), buffer);
  }
}

uint32_t pfsGetTotalSectorsCount(void)
{
  return PFS_TOTAL_SECTORS;
}

__weak void pfsFileReadCallback(uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count)
{
}

__weak void pfsFileWriteCallback(uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count)
{
}

static void pfsReadSector(uint32_t offset, uint8_t *buffer)
{
  if (offset == 0) {
    memcpy(buffer, &bootSector, PFS_BYTES_PER_SECTOR);
    return;
  }

  if ((offset >= fileSystem.fat1Offset) && (offset < (fileSystem.fat1Offset + fileSystem.fat1Length))) {
    offset -= fileSystem.fat1Offset;
    memset(buffer, 0, PFS_BYTES_PER_SECTOR);
    pfsReadFatSector(2, fileSystem.fat1FirstDword, offset, buffer);
    return;
  }

  if ((offset >= fileSystem.fat2Offset) && (offset < (fileSystem.fat2Offset + fileSystem.fat2Length))) {
    offset -= fileSystem.fat2Offset;
    memset(buffer, 0, PFS_BYTES_PER_SECTOR);
    pfsReadFatSector(2, fileSystem.fat2FirstDword, offset, buffer);
    return;
  }

  if ((offset >= fileSystem.rootOffset) && (offset < (fileSystem.rootOffset + fileSystem.rootLength))) {
    memset(buffer, 0, PFS_BYTES_PER_SECTOR);
    offset -= fileSystem.rootOffset;

    uint16_t firstFileInSectorOfRoot = offset / sizeof(tDirectoryRecord);
    uint16_t filesPerSectorOfRoot = PFS_BYTES_PER_SECTOR / sizeof(tDirectoryRecord);

    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex) >= fileSystem.filesCount) {
        break;
      }

      memcpy(buffer + (sizeof(tDirectoryRecord) * fileIndex),
             &fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord,
             sizeof(tDirectoryRecord));
    }

    return;
  }

  if ((offset >= fileSystem.dataAreaOffset) && (offset < (fileSystem.dataAreaOffset + fileSystem.dataAreaLength))) {
    memset(buffer, 0, PFS_BYTES_PER_SECTOR);

    for (uint32_t fileIndex = 0; fileIndex < fileSystem.filesCount; fileIndex++) {
      if ((fileSystem.fileInfoArray[fileIndex].dataAreaOffset <= offset) && (offset < (fileSystem.fileInfoArray[fileIndex].dataAreaOffset + fileSystem.fileInfoArray[fileIndex].dataAreaLength))) {
        offset -= fileSystem.fileInfoArray[fileIndex].dataAreaOffset;
        pfsFileReadCallback(fileSystem.fileInfoArray[fileIndex].id, buffer, offset, PFS_BYTES_PER_SECTOR);
        return;
      }
    }

    return;
  }
}

static void pfsReadFatSector(uint8_t fatNumber, uint32_t firstDword, uint32_t offset, uint8_t *buffer)
{
  if (fatNumber < 1 || fatNumber > 2) {
    return;
  }

  uint16_t requestedFatSectorStartCluster = offset / sizeof(uint16_t);
  uint16_t requestedFatSectorClustersCount = PFS_BYTES_PER_SECTOR / sizeof(uint16_t);
  uint16_t *buffer16 = (uint16_t *)buffer;
  uint16_t bufferIndex = 0;

  if (offset == 0) {
    memcpy(buffer, &firstDword, 4);
    bufferIndex += 2;
  }

  for (uint32_t fileIndex = 0; fileIndex < fileSystem.filesCount; fileIndex++) {
    if (((requestedFatSectorStartCluster + bufferIndex) >= fileSystem.fileInfoArray[fileIndex].startCluster) && (requestedFatSectorStartCluster < (fileSystem.fileInfoArray[fileIndex].startCluster + fileSystem.fileInfoArray[fileIndex].clustersCount))) {
      for (; bufferIndex < requestedFatSectorClustersCount; bufferIndex++) {
        uint16_t clusterIndex = requestedFatSectorStartCluster + bufferIndex;
        buffer16[bufferIndex] = clusterIndex + 1;

        if (clusterIndex == (fileSystem.fileInfoArray[fileIndex].startCluster + fileSystem.fileInfoArray[fileIndex].clustersCount - 1)) {
          buffer16[bufferIndex] = 0xffff;

          if (++fileIndex >= fileSystem.filesCount) {
            break;
          }
        }
      }

      break;
    }
  }
}


static void pfsWriteSector(uint32_t offset, const uint8_t *buffer)
{
  if (offset == 0) {
    // Keep bootsector unchanged.
    return;
  }

  if ((offset >= fileSystem.fat1Offset) && (offset < (fileSystem.fat1Offset + fileSystem.fat1Length))) {
    // Keep FAT unchanged.
    return;
  }

  if ((offset >= fileSystem.fat2Offset) && (offset < (fileSystem.fat2Offset + fileSystem.fat2Length))) {
    // Keep FAT unchanged.
  }

  if ((offset >= fileSystem.rootOffset) && (offset < (fileSystem.rootOffset + fileSystem.rootLength))) {
    offset -= fileSystem.rootOffset;

    uint16_t firstFileInSectorOfRoot = offset / sizeof(tDirectoryRecord);
    uint16_t filesPerSectorOfRoot = PFS_BYTES_PER_SECTOR / sizeof(tDirectoryRecord);

    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex) >= fileSystem.filesCount) {
        break;
      }

      // Only update existing files.
      if (!directoryRecordIsEmpty(&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord)) {
        // Apply only date/time changes.
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord) + 0x0d,
               buffer + (sizeof(tDirectoryRecord) * fileIndex) + 0x0d,
               7);
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord) + 0x16,
               buffer + (sizeof(tDirectoryRecord) * fileIndex) + 0x16,
               4);
      }
    }

    return;
  }

  if ((offset >= fileSystem.dataAreaOffset) && (offset < (fileSystem.dataAreaOffset + fileSystem.dataAreaLength))) {
    for (uint32_t fileIndex = 0; fileIndex < fileSystem.filesCount; fileIndex++) {
      if ((fileSystem.fileInfoArray[fileIndex].dataAreaOffset <= offset) && (offset < (fileSystem.fileInfoArray[fileIndex].dataAreaOffset + fileSystem.fileInfoArray[fileIndex].dataAreaLength))) {
        offset -= fileSystem.fileInfoArray[fileIndex].dataAreaOffset;
        pfsFileWriteCallback(fileSystem.fileInfoArray[fileIndex].id, buffer, offset, PFS_BYTES_PER_SECTOR);
        return;
      }
    }

    return;
  }
}
