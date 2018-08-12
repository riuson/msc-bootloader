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
    strncpy(fileInfoArray[i].directoryRecord.name, fileInfoArray->name, 8);
    strncpy(fileInfoArray[i].directoryRecord.extension, fileInfoArray->extension, 3);
    fileInfoArray[i].directoryRecord.fileSize = fileInfoArray[i].size;
    fileInfoArray[i].directoryRecord.startClusterLow = fileInfoArray[i].startCluster;
    fileInfoArray[i].dataAreaOffset = dataAreaOffset + (fileInfoArray[i].startCluster * PFS_BYTES_PER_CLUSTER);
    fileInfoArray[i].dataAreaLength = fileInfoArray[i].clustersCount * PFS_BYTES_PER_CLUSTER;

    cluster += clustersCount;
  }

  fileSystem.bootSector = &bootSector;
  fileSystem.fileInfoArray = fileInfoArray;
  fileSystem.filesCount = filesCount;
}

void pfsRead(uint32_t offset, uint32_t count, uint8_t *buffer)
{
}
