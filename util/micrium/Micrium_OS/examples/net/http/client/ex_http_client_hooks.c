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
* File : ex_http_client_hooks.c
*********************************************************************************************************
* Note(s) : (1) For additional details on the features available with Micrium OS HTTP Client,
*               the API, the installation, etc. Refer to the Micrium OS HTTP Client documentation
*               available at https://doc.micrium.com.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_HTTP_CLIENT_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <ex_description.h>

#include  "ex_http_client_hooks.h"
#include  "ex_http_client.h"
#include  "ex_http_client_files.h"

#include  <rtos/common/include/lib_str.h>
#include  <rtos/common/include/rtos_utils.h>



static  CPU_CHAR              Ex_HTTP_Client_Buf[8192];

extern  HTTPc_KEY_VAL         Ex_HTTP_Client_ReqQueryStrTbl[EX_HTTP_CLIENT_CFG_QUERY_STR_NBR_MAX];
extern  CPU_CHAR              Ex_HTTP_Client_ReqQueryStrKeyTbl[EX_HTTP_CLIENT_CFG_QUERY_STR_NBR_MAX][EX_HTTP_CLIENT_CFG_QUERY_STR_KEY_LEN_MAX];
extern  CPU_CHAR              Ex_HTTP_Client_ReqQueryStrValTbl[EX_HTTP_CLIENT_CFG_QUERY_STR_NBR_MAX][EX_HTTP_CLIENT_CFG_QUERY_STR_VAL_LEN_MAX];

extern  HTTPc_HDR             Ex_HTTP_Client_ReqHdrTbl[EX_HTTP_CLIENT_CFG_HDR_NBR_MAX];
extern  CPU_CHAR              Ex_HTTP_Client_ReqHdrValStrTbl[EX_HTTP_CLIENT_CFG_HDR_NBR_MAX][EX_HTTP_CLIENT_CFG_HDR_VAL_LEN_MAX];
extern  HTTPc_HDR             Ex_HTTP_Client_RespHdrTbl[EX_HTTP_CLIENT_CFG_HDR_NBR_MAX];
extern  CPU_CHAR              Ex_HTTP_Client_RespHdrValStrTbl[EX_HTTP_CLIENT_CFG_HDR_NBR_MAX][EX_HTTP_CLIENT_CFG_HDR_VAL_LEN_MAX];

extern  CPU_CHAR              Ex_HTTP_Client_FormBuf[EX_HTTP_CLIENT_CFG_FORM_BUF_SIZE];

extern  CPU_CHAR              Ex_HTTP_Client_FormValStrTbl[2*EX_HTTP_CLIENT_CFG_FORM_FIELD_NBR_MAX][EX_HTTP_CLIENT_CFG_FORM_FIELD_VAL_LEN_MAX];


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               LOGGING
*
* Note(s) : (1) This example outputs information to the console via the function printf() via a macro
*               called EX_TRACE(). This can be modified or disabled if printf() is not supported.
*********************************************************************************************************
*/

#ifndef EX_TRACE
#include  <stdio.h>
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif


/*
*********************************************************************************************************
*                                  Ex_HTTP_Client_ReqQueryStrHook()
*
* Description : Add a Query String field to a specific HTTP Request.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               p_key_val   Pointer to key-value pair to recover from application.
*
* Return(s)   : DEF_YES,    if all the fields of the Query String have been added.
*               DEF_NO,     otherwise.
*
* Caller(s)   : HTTPcReq_QueryStrHook() vis 'p_req->OnQueryStrTx()'.
*
* Note(s)     : (1) In this example function, the query string will be added to all the requests on any
*                   connections.
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Client_ReqQueryStrHook (HTTPc_CONN_OBJ     *p_conn,
                                             HTTPc_REQ_OBJ      *p_req,
                                             HTTPc_KEY_VAL     **p_key_val)
{
    EX_HTTP_CLIENT_REQ_DATA  *p_app_data;
    HTTPc_KEY_VAL            *p_kvp;
    CPU_INT08U                index;


    PP_UNUSED_PARAM(p_conn);

#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    p_app_data = (EX_HTTP_CLIENT_REQ_DATA *)p_req->UserDataPtr;

    index = p_app_data->QueryStrIx;

    switch(index) {
        case 0:
             p_kvp         = &Ex_HTTP_Client_ReqQueryStrTbl[0];
             p_kvp->KeyPtr = &Ex_HTTP_Client_ReqQueryStrKeyTbl[0][0];
             p_kvp->ValPtr = &Ex_HTTP_Client_ReqQueryStrValTbl[0][0];

            (void)Str_Copy_N(p_kvp->KeyPtr, "Name", EX_HTTP_CLIENT_CFG_QUERY_STR_KEY_LEN_MAX);
            (void)Str_Copy_N(p_kvp->ValPtr, "John", EX_HTTP_CLIENT_CFG_QUERY_STR_VAL_LEN_MAX);

             p_kvp->KeyLen = Str_Len_N(p_kvp->KeyPtr, EX_HTTP_CLIENT_CFG_QUERY_STR_KEY_LEN_MAX);
             p_kvp->ValLen = Str_Len_N(p_kvp->ValPtr, EX_HTTP_CLIENT_CFG_QUERY_STR_VAL_LEN_MAX);

            *p_key_val = p_kvp;
             p_app_data->QueryStrIx++;
             return (DEF_NO);


        case 1:
             p_kvp         = &Ex_HTTP_Client_ReqQueryStrTbl[1];
             p_kvp->KeyPtr = &Ex_HTTP_Client_ReqQueryStrKeyTbl[1][0];
             p_kvp->ValPtr =  DEF_NULL;

            (void)Str_Copy_N(p_kvp->KeyPtr, "active", EX_HTTP_CLIENT_CFG_QUERY_STR_KEY_LEN_MAX);

             p_kvp->KeyLen = Str_Len_N(p_kvp->KeyPtr, EX_HTTP_CLIENT_CFG_QUERY_STR_KEY_LEN_MAX);

            *p_key_val = p_kvp;
             p_app_data->QueryStrIx = 0;
             return (DEF_YES);


        default:
            *p_key_val = DEF_NULL;
             p_app_data->QueryStrIx = 0;
             return (DEF_YES);
    }
#else
   (void)&p_kvp;
   (void)&index;
   (void)&p_app_data;

   *p_key_val = DEF_NULL;
    return (DEF_YES);
#endif
}


/*
*********************************************************************************************************
*                                     Ex_HTTP_Client_ReqHdrHook()
*
* Description : Add an header field to a specific HTTP Request.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               p_hdr       Pointer to header field to recover from application.
*
* Return(s)   : DEF_YES,    if all header fields have been added.
*               DEF_NO,     otherwise.
*
* Note(s)     : (1) In this example function, the header field will be added to all the requests on any
*                   connections.
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Client_ReqHdrHook (HTTPc_CONN_OBJ     *p_conn,
                                        HTTPc_REQ_OBJ      *p_req,
                                        HTTPc_HDR         **p_hdr)
{
    HTTPc_HDR  *p_hdr_tmp;


    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_req);

    p_hdr_tmp         = &Ex_HTTP_Client_ReqHdrTbl[0];
    p_hdr_tmp->ValPtr = &Ex_HTTP_Client_ReqHdrValStrTbl[0][0];

    p_hdr_tmp->HdrField = HTTP_HDR_FIELD_COOKIE;
    Str_Copy_N(p_hdr_tmp->ValPtr, "ID=234668", EX_HTTP_CLIENT_CFG_HDR_VAL_LEN_MAX);
    p_hdr_tmp->ValLen   = Str_Len_N(p_hdr_tmp->ValPtr, EX_HTTP_CLIENT_CFG_HDR_VAL_LEN_MAX);

   *p_hdr = p_hdr_tmp;

    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                    Ex_HTTP_Client_ReqBodyHook()
*
* Description : Specify the data to be sent in the Request body.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               p_data      Variable that will received the pointer to the data to include in the HTTP request.
*
*               p_buf       Pointer to HTTP transmit buffer.
*
*               buf_len     Length of space remaining in  the HTTP transmit buffer.
*
*               p_data_len  Length of the data.
*
* Return(s)   : DEF_YES, if all data to transmit was passed by the application
*               DEF_NO,  if data still remaining to be sent.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Client_ReqBodyHook (HTTPc_CONN_OBJ     *p_conn,
                                         HTTPc_REQ_OBJ      *p_req,
                                         void              **p_data,
                                         CPU_CHAR           *p_buf,
                                         CPU_INT16U          buf_len,
                                         CPU_INT16U         *p_data_len)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_buf);
    PP_UNUSED_PARAM(buf_len);

    switch (p_req->Method_reserved) {
        case HTTP_METHOD_POST:
            *p_data     = &Ex_HTTP_Client_FormBuf[0];
            *p_data_len =  Str_Len(&Ex_HTTP_Client_FormBuf[0]);
             return (DEF_YES);


        case HTTP_METHOD_PUT:
            *p_data     = (void *)&Ex_HTTP_Client_LogoGif[0];
            *p_data_len =  STATIC_LOGO_GIF_LEN;
             break;


        default:
            *p_data_len = 0;
             return (DEF_YES);

    }

    return (DEF_YES);
}


/*
*********************************************************************************************************
*                                    Ex_HTTP_Client_RespHdrHook()
*
* Description : Retrieve header fields in the HTTP response received.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               hdr_field   HTTP header type of the header field received in the HTTP response.
*
*               p_hdr_val   Pointer to the value string received in the Response header field.
*
*               val_len     Length of the value string.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_HTTP_Client_RespHdrHook (HTTPc_CONN_OBJ    *p_conn,
                                  HTTPc_REQ_OBJ     *p_req,
                                  HTTP_HDR_FIELD     hdr_field,
                                  CPU_CHAR          *p_hdr_val,
                                  CPU_INT16U         val_len)
{
    HTTPc_HDR  *p_hdr;


    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_req);

    p_hdr         = &Ex_HTTP_Client_RespHdrTbl[0];
    p_hdr->ValPtr = &Ex_HTTP_Client_RespHdrValStrTbl[0][0];

    switch (hdr_field) {
        case HTTP_HDR_FIELD_COOKIE:
             p_hdr->HdrField = hdr_field;
             Str_Copy_N(p_hdr->ValPtr, p_hdr_val, val_len);
             p_hdr->ValLen   = val_len;
             break;

        default:
             break;
    }
}


/*
*********************************************************************************************************
*                                    Ex_HTTP_CLient_RespBodyHook()
*
* Description : Retrieve data in HTTP Response received.
*
* Argument(s) : p_conn          Pointer to current HTTPc Connection object.
*
*               p_req           Pointer to current HTTPc Request object.
*
*               content_type    HTTP Content Type of the HTTP Response body's data.
*
*               p_data          Pointer to a data piece of the HTTP Response body.
*
*               data_len        Length of the data piece received.
*
*               last_chunk      DEF_YES, if this is the last piece of data.
*                               DEF_NO,  if more data is up coming.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_INT32U Ex_HTTP_Client_RespBodyHook (HTTPc_CONN_OBJ     *p_conn,
                                        HTTPc_REQ_OBJ      *p_req,
                                        HTTP_CONTENT_TYPE   content_type,
                                        void               *p_data,
                                        CPU_INT16U          data_len,
                                        CPU_BOOLEAN         last_chunk)
{
    CPU_CHAR  *p_data_str;


    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_req);
    PP_UNUSED_PARAM(content_type);

    p_data_str = &Ex_HTTP_Client_Buf[0];

    Mem_Copy(p_data_str, p_data, data_len);

    p_data_str +=  data_len;
   *p_data_str  = '\0';
    p_data_str  = &Ex_HTTP_Client_Buf[0];

    EX_TRACE("%s", p_data_str);

    if (last_chunk == DEF_YES) {
        EX_TRACE("\n\r");
    }

    return data_len;
}


/*
*********************************************************************************************************
*                                 Ex_HTTP_Client_FormMultipartHook()
*
* Description : Retrieve the value data for an Key-Val Extended object.
*
* Argument(s) : p_conn          Pointer to current HTTPc Connection object.
*
*               p_req           Pointer to current HTTPc Request object.
*
*               p_key_val_obj   Pointer to current Key-Value Extended object.
*
*               p_buf           Pointer to HTTP transmit buffer.
*
*               buf_len         Size remaining in HTTP buffer.
*
*               p_len_wr        Variable that will received the size of the data copied in the buffer.
*
* Return(s)   : DEF_YES, if all the data was transmitted.
*               DEF_NO,  otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/

CPU_BOOLEAN  Ex_HTTP_Client_FormMultipartHook (HTTPc_CONN_OBJ     *p_conn,
                                               HTTPc_REQ_OBJ      *p_req,
                                               HTTPc_KEY_VAL_EXT  *p_key_val_obj,
                                               CPU_CHAR           *p_buf,
                                               CPU_INT16U          buf_len,
                                               CPU_INT16U         *p_len_wr)
{
    EX_HTTP_CLIENT_REQ_DATA  *p_req_data;
    CPU_CHAR                 *p_data;
    CPU_INT16U                data_ix;
    CPU_INT16U                min_len;


    PP_UNUSED_PARAM(p_conn);

#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    p_req_data = (EX_HTTP_CLIENT_REQ_DATA *)p_req->UserDataPtr;

    data_ix    = p_req_data->FormIx;

    p_data     = &Ex_HTTP_Client_FormValStrTbl[1][0];

    min_len    = DEF_MIN(buf_len, (p_key_val_obj->ValLen - data_ix));

    if (min_len <= 0) {
        p_req_data->FormIx = 0;
       *p_len_wr = min_len;
        return (DEF_YES);
    }

    Str_Copy_N(p_buf, p_data, min_len);

   *p_len_wr  = min_len;
    data_ix  += min_len;

    p_req_data->FormIx = data_ix;

    return (DEF_NO);
#else
   (void)&p_req_data;
   (void)&p_data;
   (void)&data_ix;
   (void)&min_len;

    return (DEF_YES);
#endif
}


/*
*********************************************************************************************************
*                               Ex_HTTP_Client_FormMultipartFileHook()
*
* Description : Retrieve the file data for a Multipart File object.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               p_file_obj  Pointer to current Multipart File object.
*
*               p_buf       Pointer to HTTP transmit buffer.
*
*               buf_len     Size remaining in HTTP buffer.
*
*               p_len_wr    Variable that will received the size of the data copied in the buffer.
*
* Return(s)   : DEF_YES, if all the data was transmitted.
*               DEF_NO,  otherwise.
*
* Note(s)     : None.
*********************************************************************************************************
*/

#ifdef RTOS_MODULE_FS_AVAIL
CPU_BOOLEAN  Ex_HTTP_Client_FormMultipartFileHook (HTTPc_CONN_OBJ        *p_conn,
                                                   HTTPc_REQ_OBJ         *p_req,
                                                   HTTPc_MULTIPART_FILE  *p_file_obj,
                                                   CPU_CHAR              *p_buf,
                                                   CPU_INT16U             buf_len,
                                                   CPU_INT16U            *p_len_wr)
{
    CPU_BOOLEAN               finish;
#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    EX_HTTP_CLIENT_REQ_DATA  *p_req_data;
    FS_ENTRY_INFO             file_info;
    CPU_SIZE_T                file_rem;
    FS_FILE_SIZE              file_pos;
    RTOS_ERR                  err;
#endif


    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_file_obj);

#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    p_req_data = (EX_HTTP_CLIENT_REQ_DATA *)p_req->UserDataPtr;

    if (FS_FILE_HANDLE_IS_NULL(p_req_data->FileHandle)) {
        FS_WRK_DIR_HANDLE  wrk_dir_handle;


        wrk_dir_handle = FSWrkDir_Open(FSWrkDir_NullHandle,
                                       p_req_data->WrkDirPtr,
                                      &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

        p_req_data->FileHandle = FSFile_Open(wrk_dir_handle,
                                            "\\index.html",
                                             FS_FILE_ACCESS_MODE_RD,
                                            &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

        FSWrkDir_Close(wrk_dir_handle, &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    }

    FSFile_Query(p_req_data->FileHandle,
                &file_info,
                &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    file_pos = FSFile_PosGet(p_req_data->FileHandle,
                            &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    file_rem = file_info.Size - file_pos;
    if (file_rem == 0u) {
       *p_len_wr = 0;

        FSFile_Close(p_req_data->FileHandle,
                    &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

        p_req_data->FileHandle = FSFile_NullHandle;

        finish = DEF_YES;
    } else {
        CPU_SIZE_T  size    = DEF_MIN(file_rem, buf_len);;
        CPU_SIZE_T  size_rd;


        size_rd = FSFile_Rd(p_req_data->FileHandle,
                            p_buf,
                            size,
                           &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

       *p_len_wr = size_rd;
        finish   = DEF_NO;
    }
#else
    PP_UNUSED_PARAM(p_req);
    PP_UNUSED_PARAM(p_buf);
    PP_UNUSED_PARAM(buf_len);
    PP_UNUSED_PARAM(p_len_wr);

    finish = DEF_YES;
    APP_RTOS_ASSERT_CRITICAL_FAIL(;);
#endif

    return (finish);
}
#endif


/*
*********************************************************************************************************
*                                Ex_HTTP_Client_ConnConnectCallback()
*
* Description : Callback to notify application that an HTTP connection connect process was completed.
*
* Argument(s) : p_conn          Pointer to current HTTPc Connection.
*
*               open_status     Status of the connection:
*
*                               DEF_OK,   if the connection with the server was successful.
*                               DEF_FAIL, otherwise.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if (HTTPc_CFG_MODE_ASYNC_TASK_EN == DEF_ENABLED)
void  Ex_HTTP_Client_ConnConnectCallback (HTTPc_CONN_OBJ  *p_conn,
                                          CPU_BOOLEAN      open_status)
{
    PP_UNUSED_PARAM(p_conn);

    if (open_status == DEF_OK) {
        EX_TRACE("Connection to server succeeded.\n\r");
    } else {
        EX_TRACE("Connection to server failed.\n\r");
    }
}
#endif


/*
*********************************************************************************************************
*                                 Ex_HTTP_CLient_ConnCloseCallback()
*
* Description : Callback to notify application that an HTTP connection was closed.
*
* Argument(s) : p_conn          Pointer to current HTTPc Connection.
*
*               close_status    Status of the connection closing:
*                                   HTTPc_CONN_CLOSE_STATUS_ERR_INTERNAL
*                                   HTTPc_CONN_CLOSE_STATUS_SERVER
*                                   HTTPc_CONN_CLOSE_STATUS_NO_PERSISTENT
*                                   HTTPc_CONN_CLOSE_STATUS_APP
*
*               err             Error Code when connection was closed.
*
* Return(s)   : None
*
* Note(s)     : none.
*********************************************************************************************************
*/
#if (HTTPc_CFG_MODE_ASYNC_TASK_EN == DEF_ENABLED)
void  Ex_HTTP_Client_ConnCloseCallback (HTTPc_CONN_OBJ           *p_conn,
                                        HTTPc_CONN_CLOSE_STATUS   close_status,
                                        RTOS_ERR                  err)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(close_status);
    PP_UNUSED_PARAM(err);

    EX_TRACE("Connection closed.\n\r");
}
#endif


/*
*********************************************************************************************************
*                                 Ex_HTTP_Client_TransDoneCallback()
*
* Description : Callback to notify application that an HTTP transaction was completed.
*
* Argument(s) : p_conn  Pointer to current HTTPc Connection object.
*
*               p_req   Pointer to current HTTPc Request object.
*
*               p_resp  Pointer to current HTTPc Response object.
*
*               status  Status of the transaction:
*
*                           DEF_OK,   transaction was successful.
*                           DEF_FAIL, otherwise.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if (HTTPc_CFG_MODE_ASYNC_TASK_EN == DEF_ENABLED)
void  Ex_HTTP_Client_TransDoneCallback (HTTPc_CONN_OBJ  *p_conn,
                                        HTTPc_REQ_OBJ   *p_req,
                                        HTTPc_RESP_OBJ  *p_resp,
                                        CPU_BOOLEAN      status)
{
#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    EX_HTTP_CLIENT_REQ_DATA  *p_data;
    CPU_SR_ALLOC();
#endif


    PP_UNUSED_PARAM(p_conn);

#if (HTTPc_CFG_USER_DATA_EN == DEF_ENABLED)
    if (status == DEF_OK) {
        EX_TRACE("Transaction Status Code: %s\n\r", p_resp->ReasonPhrasePtr);
    } else {
        EX_TRACE("Transaction failed\n\r");
    }

    p_data = (EX_HTTP_CLIENT_REQ_DATA *)p_req->UserDataPtr;
    CPU_CRITICAL_ENTER();
    p_data->Done = DEF_YES;
    CPU_CRITICAL_EXIT();
#else
    PP_UNUSED_PARAM(p_req);
    PP_UNUSED_PARAM(p_resp);
    PP_UNUSED_PARAM(status);

    APP_RTOS_ASSERT_CRITICAL_FAIL(;);
#endif
}
#endif


/*
*********************************************************************************************************
*                                      Ex_HTTP_Client_TransErrCallback()
*
* Description : Callback to notify application that an error occurred during an HTTP transaction.
*
* Argument(s) : p_conn      Pointer to current HTTPc Connection object.
*
*               p_req       Pointer to current HTTPc Request object.
*
*               err_code    Error Code.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/
#if (HTTPc_CFG_MODE_ASYNC_TASK_EN == DEF_ENABLED)
void Ex_HTTP_Client_TransErrCallback (HTTPc_CONN_OBJ  *p_conn,
                                      HTTPc_REQ_OBJ   *p_req,
                                      RTOS_ERR         err_code)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_req);

    EX_TRACE("Transaction error code %i, description: %s\n\r",
              err_code.Code,
              RTOS_ERR_STR_GET(err_code.Code));
}
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_HTTP_CLIENT_AVAIL */

