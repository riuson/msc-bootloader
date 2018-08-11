/*
 * pfs_directory_record.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PREUDOFILESYSTEM_PFS_DIRECTORY_RECORD_H_
#define RIUSON_PREUDOFILESYSTEM_PFS_DIRECTORY_RECORD_H_

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef enum tAttributes {
  AttrNone = 0x00,
  AttrReadOnly = 0x01,
  AttrHidden = 0x02,
  AttrSystem = 0x04,
  AttrVolumeName = 0x08,
  AttrDirectory = 0x10,
  AttrArchive = 0x20
} tAttribute;

#pragma pack(push, 1)

typedef struct tDirectoryRecords {
  uint8_t name[8];
  uint8_t extension[3];
  uint8_t attributes;
  uint8_t __reserved;
  uint8_t creationSeconds100;
  uint16_t creationTime;
  uint16_t creationDate;
  uint16_t accessDate;
  uint16_t startClusterHigh;
  uint16_t writeTime;
  uint16_t writeDate;
  uint16_t startClusterLow;
  uint32_t fileSize;
} tDirectoryRecord;

#pragma pack(pop)

void directoryRecordClear(tDirectoryRecord *record);
bool directoryRecordIsEmpty(const tDirectoryRecord *record);
void directoryRecordSetCreationDateTime(tDirectoryRecord *record, const time_t *value);
void directoryRecordSetAccessDate(tDirectoryRecord *record, const time_t *value);
void directoryRecordSetWriteDateTime(tDirectoryRecord *record, const time_t *value);

#endif /* RIUSON_PREUDOFILESYSTEM_PFS_DIRECTORY_RECORD_H_ */
