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

#ifndef  APP_STATIC_FILES_PRESENT
#define  APP_STATIC_FILES_PRESENT


#define  STATIC_LOGO_GIF_NAME               "\\logo.gif"
#define  STATIC_INDEX_HTML_NAME             "\\index.html"

#define  STATIC_LOGO_GIF_LEN                2066u
#define  STATIC_INDEX_HTML_LEN              3080u


extern const unsigned char Ex_HTTP_Client_LogoGif[];
extern const unsigned char Ex_HTTP_Client_IndexHtml[];

#endif
