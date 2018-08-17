/*
 * bootloader.h
 *
 *  Created on: 13 aug. 2018 г.
 *      Author: Vladimir
 */

#ifndef RIUSON_BOOTLOADER_BOOTLOADER_H_
#define RIUSON_BOOTLOADER_BOOTLOADER_H_

#include "bootloader_conf.h"
#include <stdbool.h>
#include <stdint.h>

void bootloaderInit(void);
bool bootloaderIsFirmwarePresent(void);
bool bootloaderIsManualStartRequired(void);
void bootloaderRunFirmware(void);
bool bootloaderPrepareFimrwareArea(void);
bool bootloaderWriteFirmware(const uint8_t *buffer, uint32_t address, uint32_t count);
void bootloaderProcess();
bool bootloaderIsBusy();

#endif /* RIUSON_BOOTLOADER_BOOTLOADER_H_ */
