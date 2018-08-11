/*
 * pfs_boot_sector.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PREUDOFILESYSTEM_PFS_BOOT_SECTOR_H_
#define RIUSON_PREUDOFILESYSTEM_PFS_BOOT_SECTOR_H_

#include <stdint.h>

#pragma pack(push, 1)

typedef struct tBootSectors {
  uint8_t jumpCodeNop[3];
  uint8_t oemId[8];
  uint16_t bytesPerSector;
  uint8_t sectorsPerCluster;
  uint16_t reservedSectors;
  uint8_t numberOfFATs;
  uint16_t rootEntries;
  uint16_t totalSectors_16;
  uint8_t mediaDescriptor;
  uint16_t sectorsPerFAT;
  uint16_t sectorsPerTrack;
  uint16_t heads;
  uint32_t hiddenSectors;
  uint32_t totalSectors_32;
  uint8_t physicalDiskNumber;
  uint8_t __curentHead;
  uint8_t signature;
  uint32_t volumeSerialNumber;
  uint8_t volumeLabel[11];
  uint8_t systemId[8];
  uint8_t bootloaderCodeArea[448];
  uint16_t bootSignature;
} tBootSector;

#pragma pack(pop)

extern const tBootSector const bootSector;

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_BOOT_SECTOR_H_ */
