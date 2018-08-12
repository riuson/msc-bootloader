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
  tFileInfo *fileInfoArray;
  uint8_t filesCount;
} tFileSystemObject;

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_FILE_SYSTEM_OBJECT_H_ */
