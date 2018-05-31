/*
*********************************************************************************************************
*                                             EXAMPLE CODE
*********************************************************************************************************
* Licensing:
*   The licensor of this EXAMPLE CODE is Silicon Laboratories Inc.
*
*   Silicon Laboratories Inc. grants you a personal, worldwide, royalty-free, fully paid-up license to
*   use, copy, modify and distribute the EXAMPLE CODE software, or portions thereof, in any of your
*   products.
*
*   Your use of this EXAMPLE CODE is at your own risk. This EXAMPLE CODE does not come with any
*   warranties, and the licensor disclaims all implied warranties concerning performance, accuracy,
*   non-infringement, merchantability and fitness for your application.
*
*   The EXAMPLE CODE is provided "AS IS" and does not come with any support.
*
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*
*   You can contact us at: https://www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                               EXAMPLE
*
*                                    HTTP CLIENT HOOK FUNCTIONS FILE
*
* File : ex_http_client_hooks.h
*********************************************************************************************************
* Note(s) : (1) For additional details on the features available with Micrium OS HTTP Client,
*               the API, the installation, etc. Refer to the Micrium OS HTTP Client documentation
*               available at https://doc.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  EX_HTTP_CLIENT_HOOKS_H
#define  EX_HTTP_CLIENT_HOOKS_H


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/net/include/http_client.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Client_ReqQueryStrHook      (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTPc_KEY_VAL            **p_key_val);

CPU_BOOLEAN  Ex_HTTP_Client_ReqHdrHook           (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTPc_HDR                **p_hdr);

CPU_BOOLEAN  Ex_HTTP_Client_ReqBodyHook          (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  void                     **p_data,
                                                  CPU_CHAR                  *p_buf,
                                                  CPU_INT16U                 buf_len,
                                                  CPU_INT16U                *p_data_len);

void         Ex_HTTP_Client_RespHdrHook          (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTP_HDR_FIELD             hdr_field,
                                                  CPU_CHAR                  *p_hdr_val,
                                                  CPU_INT16U                 val_len);

CPU_INT32U   Ex_HTTP_Client_RespBodyHook         (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTP_CONTENT_TYPE          content_type,
                                                  void                      *p_data,
                                                  CPU_INT16U                 data_len,
                                                  CPU_BOOLEAN                last_chunk);

CPU_BOOLEAN  Ex_HTTP_Client_FormMultipartHook    (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTPc_KEY_VAL_EXT         *p_key_val_obj,
                                                  CPU_CHAR                  *p_buf,
                                                  CPU_INT16U                 buf_len,
                                                  CPU_INT16U                *p_len_wr);

#ifdef  RTOS_MODULE_FS_AVAIL
CPU_BOOLEAN  Ex_HTTP_Client_FormMultipartFileHook(HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTPc_MULTIPART_FILE      *p_file_obj,
                                                  CPU_CHAR                  *p_buf,
                                                  CPU_INT16U                 buf_len,
                                                  CPU_INT16U                *p_len_wr);
#endif

#if (HTTPc_CFG_MODE_ASYNC_TASK_EN == DEF_ENABLED)
void         Ex_HTTP_Client_ConnConnectCallback  (HTTPc_CONN_OBJ            *p_conn,
                                                  CPU_BOOLEAN                open_status);

void         Ex_HTTP_Client_ConnCloseCallback    (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_CONN_CLOSE_STATUS    close_status,
                                                  RTOS_ERR                   err);

void         Ex_HTTP_Client_TransDoneCallback    (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  HTTPc_RESP_OBJ            *p_resp,
                                                  CPU_BOOLEAN                status);

void         Ex_HTTP_Client_TransErrCallback     (HTTPc_CONN_OBJ            *p_conn,
                                                  HTTPc_REQ_OBJ             *p_req,
                                                  RTOS_ERR                   err_code);
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                              MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* EX_HTTP_CLIENT_HOOKS_H  */
