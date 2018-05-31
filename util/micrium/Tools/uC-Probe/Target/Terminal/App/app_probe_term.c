/*
************************************************************************************************************************
*                                                      uC/Probe
*                                            Terminal Window for uC/Probe
*
*                                  (c) Copyright 2009-2012; Micrium, Inc.; Weston, FL
*                          All rights reserved.  Protected by international copyright laws.
*
*                                                  APPLICATION HOOKS
*
* File    : app_probe_term.c
* By      : JPB
* Version : V1.00.0
*
************************************************************************************************************************
*/

#include  <probe_term_cfg.h>
#include  <probe_term.h>

#if PROBE_TERM_CFG_CMD_EN > 0

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void  AppProbeTermHookRx (char  *p_str);


/*
************************************************************************************************************************
*                                              SET ALL APPLICATION HOOKS
*
* Description: Set ALL application hooks.
*
* Arguments  : none.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermSetAllHooks (void)
{
    PROBE_CPU_SR_ALLOC();


    PROBE_CPU_CRITICAL_ENTER();
    ProbeTermAppHookRxPtr = AppProbeTermHookRx;
    PROBE_CPU_CRITICAL_EXIT();
}


/*
************************************************************************************************************************
*                                             CLEAR ALL APPLICATION HOOKS
*
* Description: Clear ALL application hooks.
*
* Arguments  : none.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermClrAllHooks (void)
{
    PROBE_CPU_SR_ALLOC();


    PROBE_CPU_CRITICAL_ENTER();
    ProbeTermAppHookRxPtr = (PROBE_TERM_APP_HOOK_RX)0;
    PROBE_CPU_CRITICAL_EXIT();
}


/*
************************************************************************************************************************
*                                            APPLICATION RECEPTION HOOK
*
* Description: This function is called when the embedded target has received a string from µC/Probe.
*              The string from µC/Probe is the command than needs to be processed, for example 'dir'
*              this callback function allows you to parse the command, process the command and send
*              the response back to µC/Probe by calling ProbeTermCmdPrint() as shown below.
*
* Arguments  : p_str   is a pointer to the string.
*
* Note(s)    : none
************************************************************************************************************************
*/

void  AppProbeTermHookRx (char  *p_str)
{
    char  buf[PROBE_TERM_CFG_BUF_SIZE];


    if (strncmp(p_str, "dir", 3) == 0) {
        ProbeTermCmdPrint("file.txt\nphoto.jpg\nsong.mp3\n");
    } else if (strncmp(p_str, "ipconfig", 8) == 0) {
        ProbeTermCmdPrint("IPv4 Address......: 10.10.1.149\n");
        ProbeTermCmdPrint("Subnet Mask.......: 255.255.255.0\n");
        ProbeTermCmdPrint("Default Gateway...: 10.10.1.1\n");
    } else if (strncmp(p_str, "echo", 4) == 0) {
        if (strlen(p_str) > 5) {
            strncpy(&buf[0], &p_str[5], PROBE_TERM_CFG_BUF_SIZE - 2);
            strncat(&buf[strlen(buf)], "\n\0", 2);
        } else {
            buf[0] = '\0';
        }
        ProbeTermCmdPrint(&buf[0]);
    } else if (strncmp(p_str, "trace", 5) == 0) {
        if (strlen(p_str) > 6) {
            strncpy(&buf[0], &p_str[6], PROBE_TERM_CFG_BUF_SIZE - 2);
            strncat(&buf[strlen(buf)], "\n\0", 2);
        } else {
            buf[0] = '\0';
        }
        ProbeTermCmdPrint("Ok.\n");
        ProbeTermTrcPrint(&buf[0]);
    } else {
        ProbeTermCmdPrint("Error: unrecognized or incomplete command line.\n");
    }
}
#endif