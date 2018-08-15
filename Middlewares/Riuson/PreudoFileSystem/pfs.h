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
#include <usbd_def.h>

void pfsInitialize(uint8_t filesCount, tFileInfo *fileInfoArray);
bool pfsRead(USBD_HandleTypeDef  *pdev, uint32_t offset, uint32_t count, uint8_t *buffer);
bool pfsWrite(USBD_HandleTypeDef  *pdev, uint32_t offset, uint32_t count, const uint8_t *buffer);
uint32_t pfsGetTotalSectorsCount(void);
bool pfsFileReadCallback(USBD_HandleTypeDef  *pdev, uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count);
bool pfsFileWriteCallback(USBD_HandleTypeDef  *pdev, uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count);

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_H_ */
