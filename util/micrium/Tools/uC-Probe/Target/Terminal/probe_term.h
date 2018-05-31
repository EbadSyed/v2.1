/*
*********************************************************************************************************
*                                              uC/Probe
*                                             Live Watch
*                                           Terminal Window
*
*                          (c) Copyright 2004-2013; Micrium, Inc.; Weston, FL
*
*                   All rights reserved.  Protected by international copyright laws.
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                      uC/Probe Terminal Window
*                                             Header File
*
* Filename      : probe_term.h
* Version       : V1.00.00
* Programmer(s) : JPB
*
*********************************************************************************************************
*/

#include  <stdint.h>
#include  <probe_term_cfg.h>                                    /* Application level configuration file.                */
#include  <probe_term_os.h>
#include  <probe_types.h>

#ifdef    PROBE_TERM_CFG_CMD_EN
#if       PROBE_TERM_CFG_CMD_EN > 0
#undef    PROBE_TERM_CFG_EN
#define   PROBE_TERM_CFG_EN       1
#endif
#else
#error   "PROBE_TERM_CFG_CMD_EN must be defined as shown in probe_term_cfg.h"
#endif

#ifdef    PROBE_TERM_CFG_TRC_EN
#if       PROBE_TERM_CFG_TRC_EN > 0
#undef    PROBE_TERM_CFG_EN
#define   PROBE_TERM_CFG_EN       1
#endif
#else
#error   "PROBE_TERM_CFG_TRC_EN must be defined as shown in probe_term_cfg.h"
#endif

#ifdef    PROBE_TERM_CFG_EN

#if       PROBE_TERM_CFG_EN     > 0


/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This terminal window header file is protected from multiple pre-processor inclusion
*               through the use of the pre-processor macro definition PROBE_TERM_MODULE_PRESENT.
*********************************************************************************************************
*/

#ifndef  PROBE_TERM_MODULE_PRESENT                              /* See Note #1.                                         */
#define  PROBE_TERM_MODULE_PRESENT


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include  <string.h>



/*
*********************************************************************************************************
*                                               EXTERNS
*********************************************************************************************************
*/

#ifdef   PROBE_TERM_MODULE
#define  PROBE_TERM_EXT
#else
#define  PROBE_TERM_EXT  extern
#endif


/*
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*/

#define  PROBE_TERM_MAX_BUF_SIZE            64u                 /* This is the buffer size that uC/Probe has allocated. */
                                                                /* This is not a configuration setting unless uC/Probe  */
                                                                /* is changed accordingly.                              */


/*
*********************************************************************************************************
*                                      TERMINAL WINDOW ERROR CODES
*********************************************************************************************************
*/

typedef  enum  probe_term_err {                                 /* uC/Probe terminal window error data type.            */
                                                                /* ---------------- GENERIC ERROR CODES --------------- */
    PROBE_TERM_ERR_NONE                 =    0u,
    PROBE_TERM_ERR_OS_INIT_FAIL         =    1u,                /* uC/Probe terminal window OS layer init failed.       */
    PROBE_TERM_ERR_MSG_EMPTY            =    2u,                /* The message to print is empty.                       */
    PROBE_TERM_ERR_MSG_TYPE_INVALID     =    3u,                /* The message type is invalid.                         */
    PROBE_TERM_ERR_Q_FULL               =    4u,                /* The queue is full.                                   */
    PROBE_TERM_ERR_Q_EMPTY              =    5u                 /* The queue is empty.                                  */
} PROBE_TERM_ERR;


/*
*********************************************************************************************************
*                                              DATA TYPES
*********************************************************************************************************
*/

typedef  void  (*PROBE_TERM_APP_HOOK_RX)(char *p_str);

#if PROBE_TERM_CFG_CMD_EN > 0
typedef  struct  probe_term_cmd_data {
    PROBE_INT08U           RTS;                                 /* Request-To-Send flag raised by the target/probe.     */
    PROBE_INT08U           DS;                                  /* Data-Sent       flag raised by the target/probe.     */
    PROBE_INT08U           CTS;                                 /* Clear-To-Send   flag raised by the target/probe.     */
    PROBE_CHAR             Buf[PROBE_TERM_CFG_BUF_SIZE];        /* Used to rx/tx the command/response from probe/target.*/
    PROBE_INT08U           BufLen;                              /* Number of chars in the buffer.                       */
} PROBE_TERM_CMD_DATA;

typedef  struct  probe_term_cmd {
    PROBE_TERM_CMD_DATA    Target;                              /* Embedded target variables.                           */
    PROBE_TERM_CMD_DATA    Probe;                               /* uC/Probe variables.                                  */
} PROBE_TERM_CMD;
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
typedef  struct  probe_term_trc_target {
    PROBE_INT08U           RTS;                                 /* Request-To-Send flag raised by the target.           */
    PROBE_INT08U           DS;                                  /* Data-Sent       flag raised by the target.           */
    PROBE_CHAR             Buf[PROBE_TERM_CFG_BUF_SIZE];        /* Used to tx the tracing message to probe.             */
    PROBE_INT08U           BufLen;                              /* Number of chars in the buffer.                       */
} PROBE_TERM_TRC_TARGET;

typedef  struct  probe_term_trc_probe {
    PROBE_INT08U           CTS;                                 /* Clear-To-Send   flag raised by probe.                */
} PROBE_TERM_TRC_PROBE;

typedef  struct  probe_term_trc {                               /* -------- UC/PROBE TERMINAL WINDOW VARIABLES -------- */
    PROBE_TERM_TRC_TARGET  Target;                              /* Embedded target variables.                           */
    PROBE_TERM_TRC_PROBE   Probe;                               /* uC/Probe variables.                                  */
} PROBE_TERM_TRC;
#endif

typedef  struct  probe_term {                                   /* ------------- UC/PROBE TERMINAL WINDOW ------------- */
#if PROBE_TERM_CFG_CMD_EN > 0
    PROBE_TERM_CMD         Cmd;                                 /* Command line interface.                              */
#endif
#if PROBE_TERM_CFG_TRC_EN > 0
    PROBE_TERM_TRC         Trc;                                 /* Tracing      interface.                              */
#endif
} PROBE_TERM;



/*
*********************************************************************************************************
*                                          GLOBAL VARIABLES
*********************************************************************************************************
*/

PROBE_TERM_EXT  PROBE_TERM   ProbeTerm;


/*
*********************************************************************************************************
*                                          APPLICATION HOOKS
*********************************************************************************************************
*/

PROBE_TERM_EXT  PROBE_TERM_APP_HOOK_RX  ProbeTermAppHookRxPtr;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void            ProbeTermInit             (PROBE_TERM_ERR*);

#if PROBE_TERM_CFG_CMD_EN > 0
void            ProbeTermCmdRxTaskHandler (void);
void            ProbeTermCmdTxTaskHandler (void);
PROBE_TERM_ERR  ProbeTermCmdPrint         (PROBE_CHAR*);
void            AppProbeTermSetAllHooks   (void);
#endif

#if PROBE_TERM_CFG_TRC_EN > 0
void            ProbeTermTrcTaskHandler   (void);
PROBE_TERM_ERR  ProbeTermTrcPrint         (PROBE_CHAR*);
#endif




/*
*********************************************************************************************************
*                                       OTHER FUNCTION PROTOTYPES
*                                   DEFINED IN OS's probe_term_os.c
*********************************************************************************************************
*/

void            ProbeTermOS_Init          (PROBE_TERM_ERR*);
#if PROBE_TERM_CFG_CMD_EN > 0
void            ProbeTermOS_Q_CmdMsgRdy   (void);
void            ProbeTermOS_Q_CmdMsgWait  (void);
#endif
#if PROBE_TERM_CFG_TRC_EN > 0
void            ProbeTermOS_Q_TrcMsgRdy   (void);
void            ProbeTermOS_Q_TrcMsgWait  (void);
#endif


/*
*********************************************************************************************************
*                                             MODULE END
*
* Note(s) : (1) See 'probe_term.h  MODULE'.
*********************************************************************************************************
*/

#endif                                                          /* End of terminal window module include.               */
#endif
#endif