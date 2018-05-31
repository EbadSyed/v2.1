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
* File : ex_common_auth_rights.c
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
*                                            Ex_AuthRights()
*
* Description : Provides example on how to use the Auth sub-module of Common to manage rights.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

void  Ex_AuthRights (void)
{
    AUTH_USER_HANDLE  user_a_unvalidated_handle;
    AUTH_USER_HANDLE  admin_validated_handle;
    AUTH_RIGHT        right;
    RTOS_ERR          err;


                                                                /* The user must have been created before, for this ... */
    user_a_unvalidated_handle = Auth_GetUser("UserA", &err);    /* call to be successful.                               */
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    /* If obtained, this only means that the user exists. */


    admin_validated_handle = Auth_ValidateCredentials("admin", "admin", &err);
    if (err.Code == RTOS_ERR_INVALID_CREDENTIALS) {
        /* Invalid user name/password combination. */
        /* At this point, we would normally return or indicate an error. */
    } else if (err.Code != RTOS_ERR_NONE) {
        /* Handle error. */
    }
    /* If credentials were good, we may continue and use the 'user_handle' obtained. */


    right = Auth_GetUserRight(user_a_unvalidated_handle, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    /* At this point, 'right' does not contain any bit set, since no right has been granted to that user. */


    Auth_GrantRight((AUTH_RIGHT_6 | AUTH_RIGHT_8), user_a_unvalidated_handle, admin_validated_handle, &err);
    if (err.Code == RTOS_ERR_PERMISSION) {
        /* This would mean that the 'as_user_handle' does not have the right to add. */
        /* At this point, we would normally return or indicate an error. */
    } else if (err.Code != RTOS_ERR_NONE) {
        /* Handle error. */
    }
    /* If no error, rights 6 and 8 were added, let's confirm. */


    right = Auth_GetUserRight(user_a_unvalidated_handle, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    /* At this point, 'right' has the AUTH_RIGHT_6 and AUTH_RIGHT_8 set, since they have been granted to that user. */


    Auth_RevokeRight(AUTH_RIGHT_6, user_a_unvalidated_handle, admin_validated_handle, &err);
    if (err.Code == RTOS_ERR_PERMISSION) {
        /* This would mean that the 'as_user_handle' does not have the right to revoke. */
        /* At this point, we would normally return or indicate an error. */
    } else if (err.Code != RTOS_ERR_NONE) {
        /* Handle error. */
    }
    /* If no error, right 6 was revoked. */


    right = Auth_GetUserRight(user_a_unvalidated_handle, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    /* At this point, 'right' has ONLY the AUTH_RIGHT_8 set, since AUTH_RIGHT_6 has been revoked. */

    PP_UNUSED_PARAM(right);
}
