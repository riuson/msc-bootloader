/*
 * bootloader.c
 *
 *  Created on: 13 aug. 2018 г.
 *      Author: Vladimir
 */

#include "bootloader.h"
#include "bootloader_conf.h"
#include "pfs.h"
#include "readme.h"
#include "stm32f4xx_hal.h"
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

extern void SCSI_ProcessReadCompleted(USBD_HandleTypeDef  *pdev, const uint8_t *buffer, uint16_t length);
extern void SCSI_ProcessWriteCompleted(USBD_HandleTypeDef  *pdev);

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

__weak bool bootloaderIsFirmwarePresent(void)
{
  return false;
}

__weak bool bootloaderIsManualStartRequired(void)
{
  return false;
}

__weak void bootloaderRunFirmware(void)
{
  typedef void (*pFunction)(void);
  pFunction Jump_To_Application;
  volatile uint32_t *vectorsTable = (volatile uint32_t *)BOOTLOADER_FW_AREA_START;

  uint32_t JumpAddress = vectorsTable[1];
  Jump_To_Application = (pFunction) JumpAddress;

  /* Initialize user application's Stack Pointer */
  __set_MSP(vectorsTable[0]);
  Jump_To_Application();
}

static uint32_t GetSector(uint32_t Address);
static uint32_t GetSectorSize(uint32_t Sector);

__weak bool bootloaderPrepareFimrwareArea(void)
{
  HAL_FLASH_Unlock();

  uint32_t count = BOOTLOADER_FW_AREA_SIZE / sizeof(uint32_t);
  uint32_t *p = (uint32_t *)BOOTLOADER_FW_AREA_START;

  for (uint32_t i = 0 ; i < count; i++) {
    if (*p++ != 0) {
      FLASH_EraseInitTypeDef EraseInitStruct;
      uint32_t FirstSector = 0, NbOfSectors = 0;
      uint32_t Address = 0, SECTORError = 0;

      /* Get the 1st sector to erase */
      FirstSector = GetSector(BOOTLOADER_FW_AREA_START);
      /* Get the number of sector to erase from 1st sector*/
      NbOfSectors = GetSector(BOOTLOADER_FW_AREA_START + BOOTLOADER_FW_AREA_SIZE - 1) - FirstSector + 1;
      EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
      EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
      EraseInitStruct.Sector        = FirstSector;
      EraseInitStruct.NbSectors     = NbOfSectors;

      if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
        while (1) {
          __NOP();
        }
      }

      break;
    }
  }

  return true;
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

      for (uint32_t itemIndex = 0u; itemIndex < count; itemIndex += 4u) {
        const uint32_t *value = (const uint32_t *)&buffer[itemIndex];

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BOOTLOADER_FW_AREA_START + offset + itemIndex, (*value)) != HAL_OK) {
          return;
        }
      }

      break;
    }

    default: {
      break;
    }
  }
}

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Base address of the Flash sectors Bank 2 */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */


/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_SECTOR_0;
  } else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_SECTOR_1;
  } else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_SECTOR_2;
  } else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_SECTOR_3;
  } else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_SECTOR_4;
  } else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_SECTOR_5;
  } else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_SECTOR_6;
  } else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
    sector = FLASH_SECTOR_7;
  } else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
    sector = FLASH_SECTOR_8;
  } else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
    sector = FLASH_SECTOR_9;
  } else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
    sector = FLASH_SECTOR_10;
  } else if ((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11)) {
    sector = FLASH_SECTOR_11;
  } else if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12)) {
    sector = FLASH_SECTOR_12;
  } else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13)) {
    sector = FLASH_SECTOR_13;
  } else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14)) {
    sector = FLASH_SECTOR_14;
  } else if ((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15)) {
    sector = FLASH_SECTOR_15;
  } else if ((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16)) {
    sector = FLASH_SECTOR_16;
  } else if ((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17)) {
    sector = FLASH_SECTOR_17;
  } else if ((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18)) {
    sector = FLASH_SECTOR_18;
  } else if ((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19)) {
    sector = FLASH_SECTOR_19;
  } else if ((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20)) {
    sector = FLASH_SECTOR_20;
  } else if ((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21)) {
    sector = FLASH_SECTOR_21;
  } else if ((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22)) {
    sector = FLASH_SECTOR_22;
  } else { /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
    sector = FLASH_SECTOR_23;
  }

  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
static uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;

  if ((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || \
      (Sector == FLASH_SECTOR_3) || (Sector == FLASH_SECTOR_12) || (Sector == FLASH_SECTOR_13) || \
      (Sector == FLASH_SECTOR_14) || (Sector == FLASH_SECTOR_15)) {
    sectorsize = 16 * 1024;
  } else if ((Sector == FLASH_SECTOR_4) || (Sector == FLASH_SECTOR_16)) {
    sectorsize = 64 * 1024;
  } else {
    sectorsize = 128 * 1024;
  }

  return sectorsize;
}
