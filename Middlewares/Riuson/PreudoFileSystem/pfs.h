/*
 * pfs.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PREUDOFILESYSTEM_PFS_H_
#define RIUSON_PREUDOFILESYSTEM_PFS_H_

#include <stdint.h>
#include "pfs_file_info.h"

void pfsInitialize(uint8_t filesCount, tFileInfo *fileInfoArray);
void pfsRead(uint32_t offset, uint32_t count, uint8_t *buffer);
void pfsWrite(uint32_t offset, uint32_t count, const uint8_t *buffer);
uint32_t pfsGetTotalSectorsCount(void);
void pfsFileReadCallback(uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count);
void pfsFileWriteCallback(uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count);

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_H_ */
