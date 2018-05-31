/***********************************************************************************************//**
 * @file   circular_buff.h
 * @brief  Circular Buffer API
 *******************************************************************************
 * # License
 * <b>Copyright 2016 Silicon Laboratories, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef CIRCULAR_BUFF_H_
#define CIRCULAR_BUFF_H_

#include <stdbool.h>
#include <stdint.h>

/***************************************************************************//**
 * @addtogroup Circular_Buffer
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup Circular_Buffer_Config_Settings Circular Buffer configuration
 * @{
 * @brief Circular Buffer configuration setting macro definitions
 ******************************************************************************/

/***************************************************************************************************
 * Public Macros and Definitions
 **************************************************************************************************/
//#define CIRCULAR_BUFF_SIZE 3*MIC_SAMPLE_BUFFER_SIZE
//#define CIRCULAR_BUFF_SIZE (5)

/***************************************************************************************************
 * Type Definitions
 **************************************************************************************************/
typedef enum {
  cb_err_ok = 0,            /**< No error */
  cb_err_full,              /**< Buffer is full */
  cb_err_empty,             /**< Buffer is empty */
  cb_err_no_mem,            /**< No memory for buffer allocation */
  cb_err_too_much_data,     /**< To much data to be push into the circular buffer */
  cb_err_insuff_data,       /**< Insufficient amount of data to be pop */
}cb_err_code_t;

typedef struct {
  void *buffer;             /**< Data buffer */
  void *buffer_end;         /**< End of data buffer */
  size_t capacity;           /**< Maximum number of items in the buffer */
  size_t count;             /**< Number of items in the buffer */
  size_t item_size;         /**< Size of each item in the buffer */
  void *head;               /**< Pointer to head */
  void *tail;               /**< Pointer to tail */
} circular_buffer_t;

/** @} {end defgroup Circular_Buffer_Config_Settings} */

/***************************************************************************//**
 * @defgroup Circular_Buffer_Functions Circular Buffer Functions
 * @{
 * @brief Circular Buffer support functions
 ******************************************************************************/

cb_err_code_t cb_init(circular_buffer_t *cb, size_t capacity, size_t sz);
cb_err_code_t cb_push_buff(circular_buffer_t *cb, void *inBuff, size_t len);
cb_err_code_t cb_pop_buff(circular_buffer_t *cb, void *outBuff, size_t len);
void cb_free(circular_buffer_t *cb);

/** @} {end defgroup Circular_Buffer_Functions}*/

/** @} {end addtogroup Circular_Buffer} */

#endif /* CIRCULAR_BUFF_H_ */
