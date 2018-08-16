/*
 * pfs_file_system_object.h
 *
 *  Created on: 12 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PREUDOFILESYSTEM_PFS_FILE_SYSTEM_OBJECT_H_
#define RIUSON_PREUDOFILESYSTEM_PFS_FILE_SYSTEM_OBJECT_H_

#include "pfs_file_info.h"
#include "pfs_boot_sector.h"

typedef struct tFileSystemObjects {
  const tBootSector *bootSector;
  tDirectoryRecord volumeLabelRootRecord;
  tFileInfo *fileInfoArray;
  uint8_t filesCount;
  uint32_t fat1Offset;
  uint32_t fat1Length;
  uint32_t fat1FirstDword;
  uint32_t fat2Offset;
  uint32_t fat2Length;
  uint32_t fat2FirstDword;
  uint32_t rootOffset;
  uint32_t rootLength;
  uint32_t dataAreaOffset;
  uint32_t dataAreaLength;
} tFileSystemObject;

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_FILE_SYSTEM_OBJECT_H_ */
