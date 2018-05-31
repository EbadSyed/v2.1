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
*                                                EXAMPLE
*
*                                  SMTP CLIENT APPLICATION FUNCTIONS FILE
*
* File : ex_smtp_client.c
*********************************************************************************************************
* Note(s) : (1) For additional details on the features available with Micrium OS SMTP Client, the
*               API, the installation, etc. Refer to the Micrium OS SMTP Client documentation
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

#if (defined(RTOS_MODULE_NET_SMTP_CLIENT_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/rtos_path.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/net/include/net_app.h>
#include  <rtos/net/include/smtp_client.h>




/*
*********************************************************************************************************
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef EX_SMTPc_SERVER_ADDR
#define EX_SMTPc_SERVER_ADDR        "smtp.isp.com"              /* TODO must be modified to specify the SMTP server to be used. */
#endif
#ifndef EX_SMTPc_TO_ADDR
#define EX_SMTPc_TO_ADDR            "test_to@gmail.com"         /* TODO must be modified to specify the destination address.    */
#endif

#ifndef EX_SMTPc_FROM_NAME
#define EX_SMTPc_FROM_NAME          "From Name"
#endif

#ifndef EX_SMTPc_FROM_ADDR
#define EX_SMTPc_FROM_ADDR          "test_from@gmail.com"
#endif

#ifndef EX_SMTPc_USERNAME
#define EX_SMTPc_USERNAME           DEF_NULL
#endif

#ifndef EX_SMTPc_PW
#define EX_SMTPc_PW                 DEF_NULL
#endif

#ifndef EX_SMTPc_MSG_SUBJECT
#define EX_SMTPc_MSG_SUBJECT        "Example Title"
#endif

#ifndef EX_SMTPc_MSG_BODY
#define EX_SMTPc_MSG_BODY           "Example email sent using Micrium OS"
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            Ex_SMTP_Client_Init()
*
* Description : Initialize the Micrium OS SMTP Client module for the example application.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_SMTP_Client_Init (void)
{
    RTOS_ERR  err;

                                                                /* ------------- INITIALIZE CLIENT SUITE -------------- */
    SMTPc_Init(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*                                        Ex_SMTP_Client_SendMail()
*
* Description : Send an email
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_SMTP_Client_SendMail (void)
{
    SMTPc_MSG  *p_msg;
    RTOS_ERR    err;


    p_msg = (SMTPc_MSG *)SMTPc_MsgAlloc(&err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    SMTPc_MsgSetParam(p_msg, SMTPc_FROM_ADDR, EX_SMTPc_FROM_ADDR, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    SMTPc_MsgSetParam(p_msg, SMTPc_FROM_DISPL_NAME, EX_SMTPc_FROM_NAME, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    SMTPc_MsgSetParam(p_msg, SMTPc_TO_ADDR, EX_SMTPc_TO_ADDR, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    SMTPc_MsgSetParam(p_msg, SMTPc_MSG_SUBJECT, EX_SMTPc_MSG_SUBJECT, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    SMTPc_MsgSetParam(p_msg, SMTPc_MSG_BODY, EX_SMTPc_MSG_BODY, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    SMTPc_SendMail(EX_SMTPc_SERVER_ADDR,
                   DEF_NULL,
                   EX_SMTPc_USERNAME,
                   EX_SMTPc_PW,
                   DEF_NULL,
                   p_msg,
                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    SMTPc_MsgFree(p_msg, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_SMTP_CLIENT_AVAIL */

