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
*                                 HTTP SERVER INSTANCE HOOKS EXAMPLE
*
* File : ex_http_server_hooks.h
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  EX_HTTP_SERVER_HOOKS_H
#define  EX_HTTP_SERVER_HOOKS_H


/*
*********************************************************************************************************
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Server_InstanceInitHook    (const  HTTPs_INSTANCE         *p_instance,
                                                 const  void                   *p_hook_cfg);

CPU_BOOLEAN  Ex_HTTP_Server_ReqHdrRxHook        (const  HTTPs_INSTANCE         *p_instance,
                                                 const  HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                        HTTP_HDR_FIELD          hdr_field);

CPU_BOOLEAN  Ex_HTTP_Server_ReqHookNoREST       (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);

CPU_BOOLEAN  Ex_HTTP_Server_ReqHook             (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);

CPU_BOOLEAN  Ex_HTTP_Server_ReqBodyRxHook       (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                        void                   *p_buf,
                                                 const  CPU_SIZE_T              buf_size,
                                                        CPU_SIZE_T             *p_buf_size_used);

CPU_BOOLEAN  Ex_HTTP_Server_ReqRdySignalHook    (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                 const  HTTPs_KEY_VAL          *p_data);

CPU_BOOLEAN  Ex_HTTP_Server_ReqRdyPollHook      (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);

CPU_BOOLEAN  Ex_HTTP_Server_RespHdrTxHook       (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);

CPU_BOOLEAN  Ex_HTTP_Server_RespTokenValGetHook (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                 const  CPU_CHAR               *p_token,
                                                        CPU_INT16U              token_len,
                                                        CPU_CHAR               *p_val,
                                                        CPU_INT16U              val_len_max);

CPU_BOOLEAN  Ex_HTTP_Server_RespChunkDataGetHook(const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                        void                   *p_buf,
                                                        CPU_SIZE_T              buf_len_max,
                                                        CPU_SIZE_T             *p_tx_len);

void         Ex_HTTP_Server_TransCompleteHook   (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);

void         Ex_HTTP_Server_ErrFileGetHook      (const  void                   *p_hook_cfg,
                                                        HTTP_STATUS_CODE        status_code,
                                                        CPU_CHAR               *p_file_str,
                                                        CPU_INT32U              file_len_max,
                                                        HTTPs_BODY_DATA_TYPE   *p_file_type,
                                                        HTTP_CONTENT_TYPE      *p_content_type,
                                                        void                  **p_data,
                                                        CPU_INT32U             *p_data_len);

void         Ex_HTTP_Server_ErrHook             (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg,
                                                        HTTPs_ERR               err);

void         Ex_HTTP_Server_ConnCloseHook       (const  HTTPs_INSTANCE         *p_instance,
                                                        HTTPs_CONN             *p_conn,
                                                 const  void                   *p_hook_cfg);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* EX_HTTP_SERVER_HOOKS_H */
