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


typedef enum tLongOperation {
  LongOpNone,
  LongOpRead,
  LongOpWrite
} LongOperation;

typedef struct tLongOpsData {
  uint8_t blockBuffer[512];
  uint8_t fileId;
  uint32_t blockAddress;
  uint32_t blockLength;
  void *context;
  LongOperation operation;
} LongOpsData;

static tFileSystemObject fileSystem;
static LongOpsData longOpsData;

static void pfsReadSector(uint32_t offset, uint8_t *buffer);
static void pfsReadFatSector(uint8_t fatNumber, uint32_t firstDword, uint32_t offset, uint8_t *buffer);
static void pfsReadRootDirectorySector(uint32_t offset, uint8_t *buffer);
static void pfsWriteRootDirectorySector(uint32_t offset, const uint8_t *buffer);
static void pfsWriteSector(uint32_t offset, const uint8_t *buffer);
static bool pfsReadDataArea(void *context, uint32_t offset, uint32_t count, uint8_t *buffer);
static bool pfsWriteDataArea(void *context, uint32_t offset, uint32_t count, const uint8_t *buffer);

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

  // Initialize Volumelabel record in RootDirectory
  directoryRecordClear(&fileSystem.volumeLabelRootRecord);
  directoryRecordSetCreationDateTime(&fileSystem.volumeLabelRootRecord, &initializationTime);
  directoryRecordSetAccessDate(&fileSystem.volumeLabelRootRecord, &initializationTime);
  directoryRecordSetWriteDateTime(&fileSystem.volumeLabelRootRecord, &initializationTime);
  fileSystem.volumeLabelRootRecord.attributes = AttrVolumeName;
  uint8_t volumeLabel[] = PFS_VOLUME_LABEL;
  memcpy(fileSystem.volumeLabelRootRecord.name, volumeLabel, 11);
  fileSystem.volumeLabelRootRecord.fileSize = 0;
  fileSystem.volumeLabelRootRecord.startClusterLow = 0;

  longOpsData.operation = LongOpNone;
}

bool pfsRead(void *context, uint32_t offset, uint32_t count, uint8_t *buffer)
{
  // Requested array must be aligned to sectors.
  if (((offset % PFS_BYTES_PER_SECTOR) != 0) || ((count % PFS_BYTES_PER_SECTOR) != 0)) {
    return true;
  }

  uint32_t sectorsCount = count / PFS_BYTES_PER_SECTOR;

  // Request to data area
  if ((offset >= fileSystem.dataAreaOffset) && (offset < (fileSystem.dataAreaOffset + fileSystem.dataAreaLength))) {
    return pfsReadDataArea(context, offset, count, buffer);
  }

  for (uint32_t i = 0; i < sectorsCount; i++) {
    pfsReadSector(offset + (i * PFS_BYTES_PER_SECTOR), buffer);
  }

  return true;
}

bool pfsWrite(void *context, uint32_t offset, uint32_t count, const uint8_t *buffer)
{
  // Requested array must be aligned to sectors.
  if (((offset % PFS_BYTES_PER_SECTOR) != 0) || ((count % PFS_BYTES_PER_SECTOR) != 0)) {
    return true;
  }

  uint32_t sectorsCount = count / PFS_BYTES_PER_SECTOR;

  // Request to data area
  if ((offset >= fileSystem.dataAreaOffset) && (offset < (fileSystem.dataAreaOffset + fileSystem.dataAreaLength))) {
    return pfsWriteDataArea(context, offset, count, buffer);
  }

  for (uint32_t i = 0; i < sectorsCount; i++) {
    pfsWriteSector(offset + (i * PFS_BYTES_PER_SECTOR), buffer);
  }

  return true;
}

uint32_t pfsGetTotalSectorsCount(void)
{
  return PFS_TOTAL_SECTORS;
}

static void pfsReadSector(uint32_t offset, uint8_t *buffer)
{
  if (offset == 0) {
    memcpy(buffer, &bootSector, PFS_BYTES_PER_SECTOR);
    return;
  }

  if ((offset >= fileSystem.fat1Offset) && (offset < (fileSystem.fat1Offset + fileSystem.fat1Length))) {
    pfsReadFatSector(2, fileSystem.fat1FirstDword, offset - fileSystem.fat1Offset, buffer);
    return;
  }

  if ((offset >= fileSystem.fat2Offset) && (offset < (fileSystem.fat2Offset + fileSystem.fat2Length))) {
    pfsReadFatSector(2, fileSystem.fat2FirstDword, offset - fileSystem.fat2Offset, buffer);
    return;
  }

  if ((offset >= fileSystem.rootOffset) && (offset < (fileSystem.rootOffset + fileSystem.rootLength))) {
    memset(buffer, 0, PFS_BYTES_PER_SECTOR);
    pfsReadRootDirectorySector(offset - fileSystem.rootOffset, buffer);
    return;
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
    pfsWriteRootDirectorySector(offset - fileSystem.rootOffset, buffer);
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

  for (uint16_t i = 0; i < PFS_BYTES_PER_SECTOR / 2; i++) {
    ((uint16_t *)buffer)[i] = 0xfff0; // Reserved by default
  }

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

static void pfsReadRootDirectorySector(uint32_t offset, uint8_t *buffer)
{
  uint16_t firstFileInSectorOfRoot = offset / sizeof(tDirectoryRecord);
  uint16_t filesPerSectorOfRoot = PFS_BYTES_PER_SECTOR / sizeof(tDirectoryRecord);

  // If sector with volume label.
  if (offset == 0) {
    memcpy(buffer, &fileSystem.volumeLabelRootRecord, sizeof(tDirectoryRecord));

    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot - 1; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex) >= fileSystem.filesCount) {
        break;
      }

      memcpy(buffer + (sizeof(tDirectoryRecord) * (fileIndex + 1)),
             &fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord,
             sizeof(tDirectoryRecord));
    }
  } else {
    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex - 1) >= fileSystem.filesCount) {
        break;
      }

      memcpy(buffer + (sizeof(tDirectoryRecord) * fileIndex),
             &fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex - 1].directoryRecord,
             sizeof(tDirectoryRecord));
    }
  }
}

static void pfsWriteRootDirectorySector(uint32_t offset, const uint8_t *buffer)
{
  uint16_t firstFileInSectorOfRoot = offset / sizeof(tDirectoryRecord);
  uint16_t filesPerSectorOfRoot = PFS_BYTES_PER_SECTOR / sizeof(tDirectoryRecord);

  if (offset == 0) {
    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot - 1; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex) >= fileSystem.filesCount) {
        break;
      }

      // Only update existing files.
      if (!directoryRecordIsEmpty(&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord)) {
        // Apply only date/time changes.
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord) + 0x0d,
               buffer + (sizeof(tDirectoryRecord) * (fileIndex + 1)) + 0x0d,
               7);
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord) + 0x16,
               buffer + (sizeof(tDirectoryRecord) * (fileIndex + 1)) + 0x16,
               4);
      }
    }
  } else {
    for (uint32_t fileIndex = 0; fileIndex < filesPerSectorOfRoot; fileIndex++) {
      if ((firstFileInSectorOfRoot + fileIndex - 1) >= fileSystem.filesCount) {
        break;
      }

      // Only update existing files.
      if (!directoryRecordIsEmpty(&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex].directoryRecord)) {
        // Apply only date/time changes.
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex - 1].directoryRecord) + 0x0d,
               buffer + (sizeof(tDirectoryRecord) * fileIndex) + 0x0d,
               7);
        memcpy(((uint8_t *)&fileSystem.fileInfoArray[firstFileInSectorOfRoot + fileIndex - 1].directoryRecord) + 0x16,
               buffer + (sizeof(tDirectoryRecord) * fileIndex) + 0x16,
               4);
      }
    }
  }

}

static bool pfsReadDataArea(void *context, uint32_t offset, uint32_t count, uint8_t *buffer)
{
  for (uint32_t fileIndex = 0; fileIndex < fileSystem.filesCount; fileIndex++) {
    if ((fileSystem.fileInfoArray[fileIndex].dataAreaOffset <= offset) && (offset < (fileSystem.fileInfoArray[fileIndex].dataAreaOffset + fileSystem.fileInfoArray[fileIndex].dataAreaLength))) {
      offset -= fileSystem.fileInfoArray[fileIndex].dataAreaOffset;
      longOpsData.blockAddress = offset;
      longOpsData.blockLength = count;
      longOpsData.context = context;
      longOpsData.fileId = fileSystem.fileInfoArray[fileIndex].id;
      longOpsData.operation = LongOpRead;
      return false;
    }
  }

  memset(buffer, 0, count);
  return true;
}

static bool pfsWriteDataArea(void *context, uint32_t offset, uint32_t count, const uint8_t *buffer)
{

  for (uint32_t fileIndex = 0; fileIndex < fileSystem.filesCount; fileIndex++) {
    if ((fileSystem.fileInfoArray[fileIndex].dataAreaOffset <= offset) && (offset < (fileSystem.fileInfoArray[fileIndex].dataAreaOffset + fileSystem.fileInfoArray[fileIndex].dataAreaLength))) {
      offset -= fileSystem.fileInfoArray[fileIndex].dataAreaOffset;
      memcpy(longOpsData.blockBuffer, buffer, count);
      longOpsData.blockAddress = offset;
      longOpsData.blockLength = count;
      longOpsData.context = context;
      longOpsData.fileId = fileSystem.fileInfoArray[fileIndex].id;
      longOpsData.operation = LongOpWrite;
      return false;
    }
  }

  return true;
}

void pfsProcessLongOps()
{
  switch (longOpsData.operation) {
    case LongOpNone:
    default: {
      break;
    }

    case LongOpRead: {
      pfsFileReadCallback(longOpsData.fileId, longOpsData.blockBuffer, longOpsData.blockAddress, longOpsData.blockLength);
      pfsFileReadCompletedCallback(longOpsData.context, longOpsData.blockBuffer, longOpsData.blockLength);
      longOpsData.operation = LongOpNone;
      break;
    }

    case LongOpWrite: {
      pfsFileWriteCallback(longOpsData.fileId, longOpsData.blockBuffer, longOpsData.blockAddress, longOpsData.blockLength);
      pfsFileWriteCompletedCallback(longOpsData.context);
      longOpsData.operation = LongOpNone;
      break;
    }
  }
}

bool pfsIsBusy()
{
  return (longOpsData.operation != LongOpNone ? true : false);
}

__weak void pfsFileReadCallback(uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count)
{
}

__weak void pfsFileWriteCallback(uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count)
{
}

__weak void pfsFileReadCompletedCallback(void *context, const uint8_t *buffer, uint16_t length)
{
}

__weak void pfsFileWriteCompletedCallback(void *context)
{
}
