/**************************************************************************//**
 * @file main.c
 * @brief MicOS + uC/Shell Demo
 * @version 5.3.5
 ******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/
#include <bsp_cpu.h>
#include <bsp_os.h>
#include <rtos/common/include/lib_mem.h>
#include <rtos/common/include/rtos_err.h>
#include <rtos/common/include/rtos_utils.h>
#include <rtos/common/include/shell.h>
#include <rtos/cpu/include/cpu.h>
#include <rtos/kernel/include/os.h>
#include <stdio.h>

#include "em_chip.h"
#include "em_emu.h"
#include "cmd_declarations.h"
#include "common.h"
#include "retargetserial.h"
#include "shell_util.h"

// -----------------------------------------------------------------------------
// Local defines

#define  MAIN_START_TASK_PRIO              21u
#define  MAIN_START_TASK_STK_SIZE         512u
#define  INPUT_BUF_SIZE                   128u

// -----------------------------------------------------------------------------
// Local global variables

// Shell Task Stack
static            CPU_STK           shellTaskStk[MAIN_START_TASK_STK_SIZE];
static            OS_TCB            shellTaskTCB;
static  volatile  char              inputBuf[INPUT_BUF_SIZE];
static  const     SHELL_CMD_PARAM   emptyCmdParam = { 0 };

SHELL_CMD  commandTable[] =
{
  { "help", helpCmd },
  { "init", initCmd },
  { "selftest", selftestCmd },
  { "sleep", sleepCmd },
  { 0, 0 }
};

// -----------------------------------------------------------------------------
// Local function prototypes

static void shellTask(void *p_arg);
static void addCmdTable(void);
static void appExecute(char *cmdName);
static void getInput(char *buf);

// -----------------------------------------------------------------------------
// Global functions

/***************************************************************************//**
 * @brief
 *   This is the standard entry point for C applications.
 *   It is assumed that your code will call main() once you have performed
 *   all necessary initialization.
 ******************************************************************************/
int main(void)
{
  RTOS_ERR  err;

  CHIP_Init();
  EMU_UnlatchPinRetention();
  BSP_CPUInit();
  BSP_SystemInit();

  RETARGET_SerialInit();
  RETARGET_SerialCrLf(1);
  printf("== Micrium OS Shell Example ==\n");

  OSInit(&err); // Initialize the Kernel
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSTaskCreate(&shellTaskTCB, // Create the Start Task
               "Shell Task",
               shellTask,
               DEF_NULL,
               MAIN_START_TASK_PRIO,
               &shellTaskStk[0],
               (MAIN_START_TASK_STK_SIZE / 10u),
               MAIN_START_TASK_STK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  OSStart(&err); // Start the kernel
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE), 1);

  return (1);
}

/***************************************************************************//**
 * @brief
 *   Printf shell output function.
 *
 * @param pbuf
 *   String to be printed
 *
 * @param buf_len
 *   Lenght of the string
 *
 * @param popt
 *   Options, these are not used in this example
 *
 * @return
 *   How many characters that was printed.
 ******************************************************************************/
CPU_INT16S outputFunction(CPU_CHAR *pbuf,
                          CPU_INT16U buf_len,
                          void *popt)
{
  (void)popt; // Unused argument

  printf(pbuf);
  return buf_len;
}

// -----------------------------------------------------------------------------
// Local functions

/***************************************************************************//**
 * @brief
 *   This is the task that will be called by the Startup when all services
 *   are initializes successfully.
 *
 * @param p_arg
 *   Argument passed from task creation. Unused, in this case.
 ******************************************************************************/
void shellTask(void  *p_arg)
{
  RTOS_ERR  err;

  (void)p_arg; // Deliberately unused argument

  BSP_TickInit(); // Initialize Kernel tick source

#if (OS_CFG_STAT_TASK_EN == DEF_ENABLED)
  OSStatTaskCPUUsageInit(&err); // Initialize CPU Usage
  // Check error code
  APP_RTOS_ASSERT_DBG((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE),; );
#endif

  commonInit(); // Call common module initialization example
  BSP_OS_Init(); // Initialize peripherals
  addCmdTable();

  // Run startup commands
  initCmd(2, (char *[]){"init", "-chip" }, outputFunction, NULL);
  selftestCmd(1, (char *[]){"selftest" }, outputFunction, NULL);

  while (1) {
    printf("$ ");
    getInput((char *)inputBuf);

    if (!Str_Cmp((char *)inputBuf, "exit")) {
      break;
    }

    appExecute((char *)inputBuf);
  }
}

/***************************************************************************//**
 * @brief
 *   Populates uC/Shell table with the example commands.
 ******************************************************************************/
void addCmdTable(void)
{
  RTOS_ERR err;

  Shell_CmdTblRem("Sh", &err);
  Shell_CmdTblAdd("CMDs", commandTable, &err);

  if (RTOS_ERR_CODE_GET(err) != RTOS_ERR_NONE) {
    printf("Shell init failed: %s.\n", RTOS_ERR_STR_GET(RTOS_ERR_CODE_GET(err)));
  }
}

/***************************************************************************//**
 * @brief
 *   Execut a given line.
 *
 * @param input
 *   The string entered at prompt.
 ******************************************************************************/
void appExecute(char* input)
{
  RTOS_ERR err;

  Shell_Exec(input, outputFunction, (SHELL_CMD_PARAM *)&emptyCmdParam, &err);

  switch (RTOS_ERR_CODE_GET(err)) {
    case RTOS_ERR_NULL_PTR:
      printf("Error, NULL pointer passed.\n");
      break;
    case RTOS_ERR_NOT_FOUND:
      printf("Error, command not found: %s\n", input);
      break;
    case RTOS_ERR_NOT_SUPPORTED:
      printf("Error, command not supported.\n");
      break;
    case RTOS_ERR_INVALID_ARG:
      printf("Error, invalid arguments\n");
      break;
    case RTOS_ERR_SHELL_CMD_EXEC:
      printf("Error, command failed to execute.\n");
      break;
    case RTOS_ERR_NONE: /* No errors. */
      break;
    default:
      printf("Error, unknown error\n");
      break;
  }
}

/***************************************************************************//**
 * @brief
 *   Get text input from user.
 *
 * @param buf
 *   Buffer to hold the input string.
 ******************************************************************************/
static void getInput(char *buf)
{
  int c;
  size_t i;

  Mem_Set((char *)buf, '\0', INPUT_BUF_SIZE); // Clear previous input
  for (i = 0; i < INPUT_BUF_SIZE - 1; i++) {
    while ((c = RETARGET_ReadChar()) < 0) // Wait for valid input
      ;

    if (c == ASCII_CHAR_DELETE) { // User inputed backspace
      if (i) {
        RETARGET_WriteChar(ASCII_CHAR_DELETE);
        buf[--i] = '\0';
      }

      i--;
      continue;
    } else if (c == '\r' || c == '\n') {
      if (i) {
        printf("\n");
        break;
      } else {
        i--;
        continue;
      }
    }

    RETARGET_WriteChar(c); // Echo to terminal
    buf[i] = c;
  }

  buf[i] = '\0';
}
