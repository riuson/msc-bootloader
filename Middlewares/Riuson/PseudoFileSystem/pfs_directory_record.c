/*
 * pfs_directory_record.c
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#include "pfs_directory_record.h"
#include <assert.h>
#include <string.h>

void directoryRecordClear(tDirectoryRecord *record)
{
  static_assert(sizeof(tDirectoryRecord) == 32, "Invalid size of directory record!");
  memset(record, 0, sizeof(tDirectoryRecord));
}

bool directoryRecordIsEmpty(const tDirectoryRecord *record)
{
  const uint8_t *p = (const uint8_t *)record;

  for (int32_t i = 0; i < sizeof(tDirectoryRecord); i++) {
    if (*p++ != 0) {
      return false;
    }
  }

  return true;
}

void directoryRecordSetCreationDateTime(tDirectoryRecord *record, const time_t *value)
{
  struct tm buf;
  struct tm *dt = localtime_r(value, &buf);
  uint16_t time = (dt->tm_sec / 2) | (dt->tm_min << 5) | (dt->tm_hour << 11);
  uint16_t year = dt->tm_year - 1980;
  uint16_t date = (dt->tm_mday) | (dt->tm_mon << 5) | (year << 9);

  record->creationTime = time;
  record->creationDate = date;
  record->creationSeconds100 = 0;
}

void directoryRecordSetAccessDate(tDirectoryRecord *record, const time_t *value)
{
  struct tm buf;
  struct tm *dt = localtime_r(value, &buf);
  uint16_t year = dt->tm_year - 1980;
  uint16_t date = (dt->tm_mday) | (dt->tm_mon << 5) | (year << 9);

  record->accessDate = date;
}

void directoryRecordSetWriteDateTime(tDirectoryRecord *record, const time_t *value)
{
  struct tm buf;
  struct tm *dt = localtime_r(value, &buf);
  uint16_t time = (dt->tm_sec / 2) | (dt->tm_min << 5) | (dt->tm_hour << 11);
  uint16_t year = dt->tm_year - 1980;
  uint16_t date = (dt->tm_mday) | (dt->tm_mon << 5) | (year << 9);

  record->writeTime = time;
  record->writeDate = date;
}
