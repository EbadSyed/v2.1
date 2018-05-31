/*
 * File: psk-table.c
 * Description: PSK table utilities
 *
 * Copyright 2017 by Silicon Labs. All rights reserved.                *80*
 */

#ifndef PSK_TABLE_H
#define PSK_TABLE_H

#define MAX_PSK_TABLE_SIZE 32

typedef struct {
  uint8_t psk[32];
  uint8_t pskLength;
  uint8_t address[16];
} PskEntry;

extern Buffer emPskTable;

uint8_t emGetPsk(uint8_t *psk, const uint8_t *address);

EmberStatus emAddPsk(const uint8_t *psk,
                     uint8_t pskLength,
                     const uint8_t *address);

#endif // PSK_TABLE_H
