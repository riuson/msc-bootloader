/*
 * pfs_file_info.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PSEUDOFILESYSTEM_PFS_FILE_INFO_H_
#define RIUSON_PSEUDOFILESYSTEM_PFS_FILE_INFO_H_

#include "pfs_directory_record.h"

#pragma pack(push, 1)

typedef struct tFileInfos {
  uint8_t id;
  uint8_t name[8];
  uint8_t extension[3];
  uint32_t size;

  uint16_t startCluster;
  uint16_t clustersCount;

  uint32_t dataAreaOffset;
  uint32_t dataAreaLength;

  tDirectoryRecord directoryRecord;
} tFileInfo;

#pragma pack(pop)

#endif /* RIUSON_PSEUDOFILESYSTEM_PFS_FILE_INFO_H_ */
