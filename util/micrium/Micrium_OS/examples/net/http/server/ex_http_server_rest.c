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
*                            HTTP SERVER INSTANCE WITH REST EXAMPLE
*
* File : ex_http_server_rest.c
*********************************************************************************************************
* Note(s) : none
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

#if (defined(RTOS_MODULE_NET_HTTP_SERVER_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/common/include/lib_ascii.h>
#include  <rtos/common/include/lib_str.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/net/include/net.h>
#include  <rtos/net/include/http_server.h>
#include  <rtos/net/include/http_server_addon_rest.h>
#include  <rtos/net/include/http_server_fs_port_static.h>

#include  "ex_http_server.h"
#include  "ex_http_server_rest_hooks.h"
#include  "files/_404_html.h"
#include  "files/form_html.h"
#include  "files/index_html.h"
#include  "files/list_html.h"
#include  "files/login_html.h"
#include  "files/logo_gif.h"
#include  "files/uc_style_css.h"


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_HTTP_Server_StaticFS_Prepare (void);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                   Ex_HTTP_Server_InstanceCreate()
*
* Description : Initialize HTTPs REST Example application.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : (1) HTTP server instance configurations content MUST remain persistent. You can also use
*                   global variable or constants.
*
*               (2) Prior to do any call to HTTP module, it must be initialized. This is done by
*                   calling HTTPs_Init(). If the process is successful, the Web server internal data structures are
*                   initialized.
*
*               (3) Each web server must be initialized before it can be started or stopped. HTTPs_InstanceInit()
*                   is responsible to allocate memory for the instance, initialize internal data structure and
*                   create the web server instance's task.
*
*                   (a) The first argument is the instance configuration, which should be modified following you
*                       requirements. The intance's configuration set the server's port, the number of connection that
*                       can be accepted, the hooks functions, etc.
*
*                   (b) The second argument is the pointer to the instance task configuration. It sets the task priority,
*                       the stack size of the task, etc.
*
*               (4) Once a web server instance is initialized, it can be started using HTTPs_InstanceStart() to
*                   become come accessible. This function starts the web server instance's task. Each instance has
*                   is own task and all accepted connection is processed with this single task.
*
*                   At this point you should be able to access your web server instance using the following
*                   address in your favorite web browser:
*
*                       http://<target_ip_address>
*********************************************************************************************************
*/

void  Ex_HTTP_Server_InstanceCreateREST (void)
{
    RTOS_TASK_CFG   *p_cfg_task;
    HTTPs_CFG       *p_cfg_http;
    HTTPs_INSTANCE  *p_instance;
    RTOS_ERR         err;


    Ex_HTTP_Server_REST_MemInit();

    Ex_HTTP_Server_StaticFS_Prepare();                          /* Add files to the static file system.                 */

                                                                /* ---------- ALLOC CFG STRUCT (SEE NOTE 1) ----------- */
    p_cfg_task = (RTOS_TASK_CFG *)Mem_SegAlloc("Ex HTTP Server task cfg struct",
                                                DEF_NULL,
                                                sizeof(RTOS_TASK_CFG),
                                               &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    p_cfg_http = (HTTPs_CFG *)Mem_SegAlloc("Ex HTTP Server cfg struct",
                                            DEF_NULL,
                                            sizeof(HTTPs_CFG),
                                           &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    p_cfg_http->FS_CfgPtr = Mem_SegAlloc("Ex HTTP Server FS cfg struct",
                                          DEF_NULL,
                                          sizeof(HTTPs_CFG_FS_STATIC),
                                         &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    p_cfg_http->HdrRxCfgPtr = (HTTPs_HDR_RX_CFG *)Mem_SegAlloc("Ex HTTP Server hdr rx cfg struct",
                                                                DEF_NULL,
                                                                sizeof(HTTPs_HDR_RX_CFG),
                                                               &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* ---------------- PREPARE CFG STRUCT ---------------- */
                                                                /* Server instance task cfg.                            */
    p_cfg_task->Prio            = 21u;
    p_cfg_task->StkSizeElements = 1024u;
    p_cfg_task->StkPtr          = DEF_NULL;

                                                                /* Server instance cfg.                                 */
   ((HTTPs_HDR_RX_CFG    *)p_cfg_http->HdrRxCfgPtr)->NbrPerConnMax =  15u;
   ((HTTPs_HDR_RX_CFG    *)p_cfg_http->HdrRxCfgPtr)->DataLenMax    =  128u;
   ((HTTPs_CFG_FS_STATIC *)p_cfg_http->FS_CfgPtr)->FS_API_Ptr      = &HTTPs_FS_API_Static;
    p_cfg_http->HdrTxCfgPtr                                        =  DEF_NULL;
    p_cfg_http->QueryStrCfgPtr                                     =  DEF_NULL;
    p_cfg_http->FormCfgPtr                                         =  DEF_NULL;
    p_cfg_http->TokenCfgPtr                                        =  DEF_NULL;
    p_cfg_http->OS_TaskDly_ms                                      =  2u;
    p_cfg_http->SockSel                                            =  HTTPs_SOCK_SEL_IPv4_IPv6;
    p_cfg_http->SecurePtr                                          =  DEF_NULL;
    p_cfg_http->Port                                               =  80u;
    p_cfg_http->ConnNbrMax                                         =  6u;
    p_cfg_http->ConnInactivityTimeout_s                            =  10u;
    p_cfg_http->BufLen                                             =  1460u;
    p_cfg_http->ConnPersistentEn                                   =  DEF_ENABLED;
    p_cfg_http->FS_Type                                            =  HTTPs_FS_TYPE_STATIC;
    p_cfg_http->PathLenMax                                         =  255u;
    p_cfg_http->DfltResourceNamePtr                                = "/list.html";
    p_cfg_http->HostNameLenMax                                     =  128u;
    p_cfg_http->HooksPtr                                           = &HTTPs_REST_HookCfg;
    p_cfg_http->Hooks_CfgPtr                                       = &HTTPs_REST_Cfg;

    Str_Copy_N(&Ex_HTTPs_Name[0], "REST with Native FS", 60);

    p_instance = HTTPs_InstanceInit(p_cfg_http,                 /* Instance configuration. See Note #3a.                */
                                    p_cfg_task,                 /* Instance task configuration. See Note #3b.           */
                                   &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    HTTPsREST_Publish(&Ex_HTTP_Server_REST_ListResource,
                       0u,
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    HTTPsREST_Publish(&Ex_HTTP_Server_REST_UserResource,
                       0u,
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    HTTPsREST_Publish(&Ex_HTTP_Server_REST_FileResource,
                       0u,
                      &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Start HTTPs Instance.                                */
    HTTPs_InstanceStart(p_instance, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                  Ex_HTTP_Server_StaticFS_Prepare()
*
* Description : Example of web files add to static file system.
*
* Argument(s) : None.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  Ex_HTTP_Server_StaticFS_Prepare (void)
{
    CPU_BOOLEAN  success;


    success = HTTPs_FS_Init();                                  /* Initialize static FS.                                */
    APP_RTOS_ASSERT_DBG((success == DEF_OK), ;);

                                                                /* Add files to static FS.                              */
    success = HTTPs_FS_AddFile(_404_HTML_NAME,
                               _404_HTML_CONTENT,
                               _404_HTML_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(FORM_HTML_NAME,
                               FORM_HTML_CONTENT,
                               FORM_HTML_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(INDEX_HTML_NAME,
                               INDEX_HTML_CONTENT,
                               INDEX_HTML_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(LIST_HTML_NAME,
                               LIST_HTML_CONTENT,
                               LIST_HTML_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(LOGIN_HTML_NAME,
                               LOGIN_HTML_CONTENT,
                               LOGIN_HTML_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(LOGO_GIF_NAME,
                               LOGO_GIF_CONTENT,
                               LOGO_GIF_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);

    success = HTTPs_FS_AddFile(UC_STYLE_CSS_NAME,
                               UC_STYLE_CSS_CONTENT,
                               UC_STYLE_CSS_SIZE);
    APP_RTOS_ASSERT_CRITICAL(success == DEF_OK, ;);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif /* RTOS_MODULE_NET_HTTP_SERVER_AVAIL && RTOS_MODULE_FS_AVAIL */

