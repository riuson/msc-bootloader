/*
 * bootloader.c
 *
 *  Created on: 13 aug. 2018 г.
 *      Author: Vladimir
 */

#include "bootloader.h"
#include "pfs.h"
#include "readme.h"
#include <string.h>

static tFileInfo pseudoFiles[] = {
  {
    1,
    { 'R', 'E', 'A', 'D', 'M', 'E', ' ', ' ' },
    { 'T', 'X', 'T' },
    0
  },
  {
    2,
    { 'F', 'I', 'R', 'M', 'W', 'A', 'R', 'E' },
    { 'B', 'I', 'N' },
    BOOTLOADER_FW_AREA_SIZE
  }
};

typedef struct tBootloaderData {
  bool isFlashPrepared;
} BootloaderData;

static BootloaderData bootloaderData;

void bootloaderInit(void)
{
  if (bootloaderIsFirmwarePresent() == true) {
    if (bootloaderIsManualStartRequired() == false) {
      bootloaderRunFirmware();
    }
  }

  bootloaderData.isFlashPrepared = false;

  pseudoFiles[0].size = readme_txt_size;
  pfsInitialize(2, pseudoFiles);
}

void pfsFileReadCallback(uint8_t fileId, uint8_t *buffer, uint32_t offset, uint32_t count)
{
  switch (fileId) {
    // readme.txt
    case 1: {
      memcpy(buffer,
             ((uint8_t *)readme_txt) + offset,
             ((offset + count) > readme_txt_size) ? readme_txt_size - offset : count);
      break;
    }

    case 2: {
      memcpy(buffer, (const uint8_t *)(BOOTLOADER_FW_AREA_START + offset), count);
      break;
    }

    default: {
      break;
    }
  }
}

void pfsFileWriteCallback(uint8_t fileId, const uint8_t *buffer, uint32_t offset, uint32_t count)
{
  switch (fileId) {
    // firmware.bin
    case 2: {
      if (bootloaderData.isFlashPrepared == false) {
        bootloaderPrepareFimrwareArea();
        bootloaderData.isFlashPrepared = true;
      }

      if (bootloaderWriteFirmware(buffer, BOOTLOADER_FW_AREA_START + offset, count) == false) {
        return;
      }

      break;
    }

    default: {
      break;
    }
  }
}


__weak bool bootloaderIsFirmwarePresent(void)
{
}

__weak bool bootloaderIsManualStartRequired(void)
{
}

__weak void bootloaderRunFirmware(void)
{
}

__weak bool bootloaderPrepareFimrwareArea(void)
{
}

__weak bool bootloaderWriteFirmware(const uint8_t *buffer, uint32_t address, uint32_t count)
{
}
