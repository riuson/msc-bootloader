/*
 * pfs.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PREUDOFILESYSTEM_PFS_H_
#define RIUSON_PREUDOFILESYSTEM_PFS_H_

#include "pfs_file_info.h"
#include <stdint.h>

void pfsInitialize(uint8_t filesCount, tFileInfo *fileInfoArray);
bool pfsRead(void *context, uint32_t offset, uint32_t count, uint8_t *buffer);
bool pfsWrite(void *context, uint32_t offset, uint32_t count, const uint8_t *buffer);
uint32_t pfsGetTotalSectorsCount(void);
void pfsProcessLongOps();
bool pfsIsBusy();

void pfsFileReadCallback(uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count);
void pfsFileWriteCallback(uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count);
void pfsFileReadCompletedCallback(void *context, const uint8_t *buffer, uint16_t length);
void pfsFileWriteCompletedCallback(void *context);

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_H_ */
