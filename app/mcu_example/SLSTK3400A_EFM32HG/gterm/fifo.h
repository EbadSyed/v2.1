/***************************************************************************//**
 * @file fifo.h
 * @brief FIFO API definition.
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  size_t size;
  uint8_t *data;
  size_t content;
  size_t idxInp;
  size_t idxOut;
  uint8_t overflow;
} Fifo_t;

void FIFO_Init(Fifo_t *fifo, uint8_t *data, size_t size);
size_t FIFO_Content(Fifo_t *fifo);
size_t FIFO_GetMulti(Fifo_t *fifo, uint8_t *data, size_t size);
uint8_t FIFO_GetSingle(Fifo_t *fifo);
void FIFO_PutMultiple(Fifo_t *fifo, uint8_t *data, size_t count, bool expandCr);
void FIFO_PutSingle(Fifo_t *fifo, uint8_t data);

#ifdef __cplusplus
}
#endif

#endif // FIFO_H
