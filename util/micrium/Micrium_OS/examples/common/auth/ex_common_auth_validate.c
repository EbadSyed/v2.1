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
*                                          COMMON AUTH EXAMPLE
*
* File : ex_common_auth_validate.c
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>
#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/auth.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/rtos_err.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           Ex_AuthValidate()
*
* Description : Provides example on how to use the Auth sub-module of Common to validate credentials.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_AuthValidate (void)
{
    AUTH_USER_HANDLE  user_handle;
    RTOS_ERR          err;


    (void)Auth_CreateUser("UserB", "PwdB", &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


    user_handle = Auth_ValidateCredentials("UserB", "Pwd0", &err);
    if (err.Code == RTOS_ERR_INVALID_CREDENTIALS) {
        /* Invalid user name/password combination. */
        /* At this point, we would normally return or indicate an error. */
    } else if (err.Code != RTOS_ERR_NONE) {
        /* Handle error. */
    }

    user_handle = Auth_ValidateCredentials("UserB", "PwdB", &err);
    if (err.Code == RTOS_ERR_INVALID_CREDENTIALS) {
        /* Invalid user name/password combination. */
    } else if (err.Code != RTOS_ERR_NONE) {
        /* Handle error. */
    }
    /* If credentials were good, we may continue and use the 'user_handle' obtained. */

    PP_UNUSED_PARAM(user_handle);
}
