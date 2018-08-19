/*
 * pfs_boot_sector.c
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#include "pfs_boot_sector.h"
#include "pfs_conf.h"
#include <assert.h>

#if ((PFS_TOTAL_SECTORS * PFS_BYTES_PER_SECTOR) < 33554432ul)
#define PFS_TOTAL_SECTORS_16 PFS_TOTAL_SECTORS
#define PFS_TOTAL_SECTORS_32 (0u)
#else
#define PFS_TOTAL_SECTORS_16 (0u)
#define PFS_TOTAL_SECTORS_32 PFS_TOTAL_SECTORS
#endif

const tBootSector const bootSector = {
  { 0xeb, 0x3c, 0x90},
  PFS_OEM_ID,
  PFS_BYTES_PER_SECTOR,
  PFS_SECTORS_PER_CLUSTER,
  PFS_RESERVED_SECTORS,
  PFS_NUMBER_OF_FATS,
  PFS_ROOT_ENTRIES,
  PFS_TOTAL_SECTORS_16,
  0xf8u,
  PFS_SECTORS_PER_FAT,
  0x0020u,
  0x0040u,
  PFS_HIDDEN_SECTORS,
  PFS_TOTAL_SECTORS_32,
  0x80u,
  0x00u,
  0x29u,
  PFS_VOLUME_SERIAL_NUMBER,
  PFS_VOLUME_LABEL,
  PFS_SYSTEM_ID,
  { 0 },
  0xaa55u
};

void check(void)
{
  static_assert(sizeof(tBootSector) == 512, "Invalid size of bootsector!");


}
