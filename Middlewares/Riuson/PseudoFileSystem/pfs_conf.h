/*
 * pfs_conf.h
 *
 *  Created on: 11 aug. 2018 г.
 *      Author: Vladimir (riuson@gmail.com)
 */

#ifndef RIUSON_PSEUDOFILESYSTEM_PFS_CONF_H_
#define RIUSON_PSEUDOFILESYSTEM_PFS_CONF_H_

#include <stdint.h>

#define PFS_BYTES_PER_SECTOR      (512u)
#define PFS_SECTORS_PER_CLUSTER   (1u)
#define PFS_RESERVED_SECTORS      (1u)
#define PFS_NUMBER_OF_FATS        (2u)
#define PFS_ROOT_ENTRIES          (512u)
#define PFS_TOTAL_SECTORS         ((16u * 1024u * 1024u) / PFS_BYTES_PER_SECTOR)
#define PFS_SECTORS_PER_FAT       (128u)
#define PFS_HIDDEN_SECTORS        (0u)
#define PFS_VOLUME_SERIAL_NUMBER  (0x12345678ul)
#define PFS_VOLUME_LABEL          { 'P', 'S', 'E', 'U', 'D', 'O', 'F', 'I', 'L', 'E', 'S' }
#define PFS_OEM_ID                { 'R', 'I', 'U', 'S', 'O', 'N', ' ', ' ' }
#define PFS_SYSTEM_ID             { 'F', 'A', 'T', '1', '6', ' ', ' ', ' ' }

#define PFS_BYTES_PER_CLUSTER     (PFS_BYTES_PER_SECTOR * PFS_SECTORS_PER_CLUSTER)

#endif /* RIUSON_PSEUDOFILESYSTEM_PFS_CONF_H_ */
