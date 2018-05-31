/***************************************************************************//**
 * @file fifo.c
 * @brief FIFO API implementation.
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

#include "fifo.h"
#include "em_core.h"

void FIFO_Init(Fifo_t *fifo, uint8_t *data, size_t size)
{
  fifo->size = size;
  fifo->data = data;
  fifo->content = 0;
  fifo->idxInp = 0;
  fifo->idxOut = 0;
  fifo->overflow = 0;
}

size_t FIFO_Content(Fifo_t *fifo)
{
  return fifo->content;
}

size_t FIFO_GetMulti(Fifo_t *fifo, uint8_t *data, size_t size)
{
  size_t act = 0;

  for (size_t i = 0; i < size; i++) {
    if (FIFO_Content(fifo) == 0) {
      break;
    }
    data[act] = FIFO_GetSingle(fifo);
    act++;
  }

  return act;
}

uint8_t FIFO_GetSingle(Fifo_t *fifo)
{
  uint8_t data = 0;

  CORE_ATOMIC_SECTION(
    if (fifo->content > 0) {
    data = fifo->data[fifo->idxOut];
    fifo->idxOut = (fifo->idxOut + 1) % fifo->size;
    fifo->content--;
  }
    )

  return data;
}

void FIFO_PutMultiple(Fifo_t *fifo, uint8_t *data, size_t count, bool expandCr)
{
  for (uint32_t i = 0; i < count; i++) {
    FIFO_PutSingle(fifo, data[i]);
    if ((expandCr) && (data[i] == '\r')) {
      FIFO_PutSingle(fifo, '\n');
    }
  }
}

void FIFO_PutSingle(Fifo_t *fifo, uint8_t data)
{
  CORE_ATOMIC_SECTION(
    if (fifo->content < fifo->size) {
    fifo->data[fifo->idxInp] = data;
    fifo->idxInp = (fifo->idxInp + 1) % fifo->size;
    fifo->content++;
  } else {
    fifo->overflow = 0x01;
  }
    )
}
