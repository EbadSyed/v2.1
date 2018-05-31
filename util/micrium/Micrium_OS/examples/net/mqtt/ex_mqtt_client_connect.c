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
*                                           MQTTc APPLICATION
*
* File : ex_mqtt_client_connect.c
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

#if (defined(RTOS_MODULE_NET_MQTT_CLIENT_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <ex_description.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_def.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/net/include/mqtt_client.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                                /* Broker's host name or IP address.                    */
#ifndef  EX_MQTTc_BROKER_NAME
    #define  EX_MQTTc_BROKER_NAME           "mqtt.micrium.com"
#endif

#ifndef  EX_MQTTc_USERNAME                                      /* Username from MQTT server portal, if any.            */
    #define  EX_MQTTc_USERNAME              "micrium"
#endif

#ifndef  EX_MQTTc_PASSWORD                                      /* Password or MD5 hash of your password.               */
    #define  EX_MQTTc_PASSWORD              "micrium"
#endif

#ifndef  EX_MQTTc_CLIENT_ID_NAME
#define  EX_MQTTc_CLIENT_ID_NAME            "micrium-example-basic"
#endif


#define  EX_MQTTc_MSG_LEN_MAX               128u
#define  EX_MQTTc_PUBLISH_RX_MSG_LEN_MAX    128u

#ifndef EX_TRACE
#include  <stdio.h>
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

static  MQTTc_CONN  Ex_MQTTc_Conn;

static  MQTTc_MSG   Ex_MQTTc_Msg;
static  CPU_INT08U  Ex_MQTTc_MsgBuf[EX_MQTTc_MSG_LEN_MAX];

static  MQTTc_MSG   Ex_MQTTc_MsgPublishRx;
static  MQTTc_MSG   Ex_MQTTc_MsgPublishRxBuf[EX_MQTTc_PUBLISH_RX_MSG_LEN_MAX];


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_MQTTc_OnCmplCallbackFnct       (MQTTc_CONN  *p_conn,
                                                 MQTTc_MSG   *p_msg,
                                                 void        *p_arg,
                                                 RTOS_ERR     err);

static  void  Ex_MQTTc_OnConnectCmplCallbackFnct(MQTTc_CONN  *p_conn,
                                                 MQTTc_MSG   *p_msg,
                                                 void        *p_arg,
                                                 RTOS_ERR     err);

static  void  Ex_MQTTc_OnErrCallbackFnct        (MQTTc_CONN  *p_conn,
                                                 void        *p_arg,
                                                 RTOS_ERR     err);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       Ex_MQTT_Client_Connect()
*
* Description : Just connect the client to the broker.
*
* Arguments   : none.
*
* Return(s)   : DEF_OK,   if NO error(s),
*               DEF_FAIL, otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  Ex_MQTT_Client_Connect (void)
{
    RTOS_ERR  err;


    MQTTc_MsgClr(&Ex_MQTTc_Msg, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_MsgSetParam(&Ex_MQTTc_Msg, MQTTc_PARAM_TYPE_MSG_BUF_PTR, (void *)&Ex_MQTTc_MsgBuf[0u], &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_MsgSetParam(&Ex_MQTTc_Msg, MQTTc_PARAM_TYPE_MSG_BUF_LEN, (void *)EX_MQTTc_MSG_LEN_MAX, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_MsgClr(&Ex_MQTTc_MsgPublishRx, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_MsgSetParam(&Ex_MQTTc_MsgPublishRx, MQTTc_PARAM_TYPE_MSG_BUF_PTR, (void *)&Ex_MQTTc_MsgPublishRxBuf[0u], &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_MsgSetParam(&Ex_MQTTc_MsgPublishRx, MQTTc_PARAM_TYPE_MSG_BUF_LEN, (void *)EX_MQTTc_PUBLISH_RX_MSG_LEN_MAX, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_ConnClr(&Ex_MQTTc_Conn, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Err handling should be done in your application.     */
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_BROKER_NAME,              (void *) EX_MQTTc_BROKER_NAME,               &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_CLIENT_ID_STR,            (void *) EX_MQTTc_CLIENT_ID_NAME,            &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_USERNAME_STR,             (void *) EX_MQTTc_USERNAME,                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_PASSWORD_STR,             (void *) EX_MQTTc_PASSWORD,                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_KEEP_ALIVE_TMR_SEC,       (void *) 1000u,                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_CALLBACK_ON_COMPL,        (void *) Ex_MQTTc_OnCmplCallbackFnct,        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_CALLBACK_ON_CONNECT_CMPL, (void *) Ex_MQTTc_OnConnectCmplCallbackFnct, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_CALLBACK_ON_ERR_CALLBACK, (void *) Ex_MQTTc_OnErrCallbackFnct,         &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_PUBLISH_RX_MSG_PTR,       (void *)&Ex_MQTTc_MsgPublishRx,              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    MQTTc_ConnSetParam(&Ex_MQTTc_Conn, MQTTc_PARAM_TYPE_TIMEOUT_MS,               (void *) 30000u,                             &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    MQTTc_ConnOpen(&Ex_MQTTc_Conn, MQTTc_FLAGS_NONE, &err);     /* Open conn to MQTT server with parameters set in Conn.*/
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    MQTTc_Connect(&Ex_MQTTc_Conn, &Ex_MQTTc_Msg, &err);         /* Send CONNECT msg to MQTT server.                     */
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    EX_TRACE("Initialization and CONNECT to server successful.\r\n");
}


/*
*********************************************************************************************************
*                                     Ex_MQTTc_OnCmplCallbackFnct()
*
* Description : Generic callback function for MQTTc module.
*
* Arguments   : p_conn          Pointer to MQTTc Connection object for which operation has completed.
*
*               p_msg           Pointer to MQTTc Message object used for operation.
*
*               p_arg           Pointer to argument set in MQTTc Connection using the parameter type
*                               MQTTc_PARAM_TYPE_CALLBACK_ARG_PTR.
*
*               err             Error code from processing message.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  Ex_MQTTc_OnCmplCallbackFnct (MQTTc_CONN  *p_conn,
                                           MQTTc_MSG   *p_msg,
                                           void        *p_arg,
                                           RTOS_ERR     err)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_arg);

    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    switch (p_msg->Type) {
        case MQTTc_MSG_TYPE_CONNECT:                            /* Gen callback called for event type: Connect Cmpl.    */
             EX_TRACE("Gen callback called for event type: Connect Cmpl.\n\r");
             break;


        case MQTTc_MSG_TYPE_PUBLISH:                            /* Gen callback called for event type: Publish Cmpl.    */
             EX_TRACE("Gen callback called for event type: Publish Cmpl.\n\r");
             break;


        case MQTTc_MSG_TYPE_SUBSCRIBE:                          /* Gen callback called for event type: Subscribe Cmpl.  */
             EX_TRACE("Gen callback called for event type: Subscribe Cmpl.\n\r");
             break;


        case MQTTc_MSG_TYPE_UNSUBSCRIBE:                        /* Gen callback called for event type: Unsubscribe Cmpl.*/
             EX_TRACE("Gen callback called for event type: Unsubscribe Cmpl.\n\r");
             break;


        case MQTTc_MSG_TYPE_PINGREQ:                            /* Gen callback called for event type: PingReq Cmpl.    */
             EX_TRACE("Gen callback called for event type: PingReq Cmpl.\n\r");
             break;


        case MQTTc_MSG_TYPE_DISCONNECT:                         /* Gen callback called for event type: Disconnect Cmpl. */
             EX_TRACE("Gen callback called for event type: Disconnect Cmpl.\n\r");
             break;


        default:
             EX_TRACE("Gen callback called for event type: default. !!! ERROR !!! %i\n\r", p_msg->Type);
             break;
    }
}


/*
*********************************************************************************************************
*                                 Ex_MQTTc_OnConnectCmplCallbackFnct()
*
* Description : Callback function for MQTTc module called when a CONNECT operation has completed.
*
* Arguments   : p_conn          Pointer to MQTTc Connection object for which operation has completed.
*
*               p_msg           Pointer to MQTTc Message object used for operation.
*
*               p_arg           Pointer to argument set in MQTTc Connection using the parameter type
*                               MQTTc_PARAM_TYPE_CALLBACK_ARG_PTR.
*
*               err             Error code from processing CONNECT message.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  Ex_MQTTc_OnConnectCmplCallbackFnct (MQTTc_CONN  *p_conn,
                                                  MQTTc_MSG   *p_msg,
                                                  void        *p_arg,
                                                  RTOS_ERR     err)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_msg);
    PP_UNUSED_PARAM(p_arg);

    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    EX_TRACE("ConnectCmpl callback called without err, ready to send/receive messages.\n\r");
}


/*
*********************************************************************************************************
*                                     Ex_MQTTc_OnErrCallbackFnct()
*
* Description : Callback function for MQTTc module called when an error occurs.
*
* Arguments   : p_conn          Pointer to MQTTc Connection object on which error occurred.
*
*               p_arg           Pointer to argument set in MQTTc Connection using the parameter type
*                               MQTTc_PARAM_TYPE_CALLBACK_ARG_PTR.
*
*               err             Error code.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  Ex_MQTTc_OnErrCallbackFnct (MQTTc_CONN  *p_conn,
                                          void        *p_arg,
                                          RTOS_ERR     err)
{
    PP_UNUSED_PARAM(p_conn);
    PP_UNUSED_PARAM(p_arg);

    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_MQTT_CLIENT_AVAIL */

