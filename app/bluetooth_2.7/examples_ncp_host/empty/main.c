/**
 * This an example application that demonstrates Bluetooth connectivity
 * using BGLIB C function definitions. The example enables Bluetooth advertisements
 * and connections.
 *
 * Most of the functionality in BGAPI uses a request-response-event pattern
 * where the module responds to a command with a command response indicating
 * it has processed the request and then later sending an event indicating
 * the requested operation has been completed. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "infrastructure.h"

/* BG stack headers */
#include "gecko_bglib.h"

/* hardware specific headers */
#include "uart.h"

/* application specific files */
#include "app.h"

/***************************************************************************************************
 * Local Macros and Definitions
 **************************************************************************************************/

BGLIB_DEFINE();

/** The default serial port to use for BGAPI communication. */
#if ((_WIN32 == 1) || (__CYGWIN__ == 1))
static char* default_uart_port = "COM0";
#elif __APPLE__ == 1
static char* default_uart_port = "/dev/tty.usbmodem14171";
#elif __linux == 1
static char* default_uart_port = "/dev/ttyACM0";
#else
static char* default_uart_port = "";
#endif

/** The default baud rate to use. */
static uint32_t default_baud_rate = 115200;

/** The serial port to use for BGAPI communication. */
static char* uart_port = NULL;

/** The baud rate to use. */
static uint32_t baud_rate = 0;

/** Usage string */
#define USAGE "Usage: %s [serial port] <baud rate> \n\n"

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/

static int appSerialPortInit(int argc, char* argv[], int32_t timeout);
static void on_message_send(uint32_t msg_len, uint8_t* msg_data);

/***************************************************************************************************
 * Public Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  The main program.
 *  \param[in] argc Argument count.
 *  \param[in] argv Buffer contaning Serial Port data.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
int main(int argc, char* argv[])
{
  struct gecko_cmd_packet* evt;

  /* Initialize BGLIB with our output function for sending messages. */
  BGLIB_INITIALIZE_NONBLOCK(on_message_send, uartRx, uartRxPeek);

  /* Initialise serial communication as non-blocking. */
  if (appSerialPortInit(argc, argv, 100) < 0) {
    printf("Non-blocking serial port init failure\n");
    exit(EXIT_FAILURE);
  }

  // Flush std output
  fflush(stdout);

  printf("Starting up...\nResetting NCP target...\n");

  /* Reset NCP to ensure it gets into a defined state.
   * Once the chip successfully boots, gecko_evt_system_boot_id event should be received. */
  gecko_cmd_system_reset(0);

  while (1) {
    /* Check for stack event. */
    evt = gecko_peek_event();
    /* Run application and event handler. */
    appHandleEvents(evt);
  }

  return -1;
}

/***************************************************************************************************
 * Static Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Function called when a message needs to be written to the serial port.
 *  \param[in] msg_len Length of the message.
 *  \param[in] msg_data Message data, including the header.
 **************************************************************************************************/
static void on_message_send(uint32_t msg_len, uint8_t* msg_data)
{
  /** Variable for storing function return values. */
  int32_t ret;

  ret = uartTx(msg_len, msg_data);
  if (ret < 0) {
    printf("Failed to write to serial port %s, ret: %d, errno: %d\n", uart_port, ret, errno);
    exit(EXIT_FAILURE);
  }
}

/***********************************************************************************************//**
 *  \brief  Serial Port initialisation routine.
 *  \param[in] argc Argument count.
 *  \param[in] argv Buffer contaning Serial Port data.
 *  \return  0 on success, -1 on failure.
 **************************************************************************************************/
static int appSerialPortInit(int argc, char* argv[], int32_t timeout)
{
  static char tmpbaud[50];

  /**
   * Handle the command-line arguments.
   */
  baud_rate = default_baud_rate;
  switch (argc) {
    case 3:
      baud_rate = atoi(argv[2]);
    /** Falls through on purpose. */
    case 2:
      uart_port = argv[1];
    /** Falls through on purpose. */
    default:
      break;
  }
  if (!uart_port) {/*no uart port given, ask from user*/
    printf("Uart port to use (default %s):", default_uart_port);
    if (fgets(tmpbaud, sizeof(tmpbaud), stdin)) {
      size_t len = strlen(tmpbaud);
      /* fgets is used, so remove the new line character from the end */
      if (tmpbaud[len - 1] == '\n') {
        len--;
        tmpbaud[len] = '\0';
      }
      if (tmpbaud[len - 1] == '\r') {
        len--;
        tmpbaud[len] = '\0';
      }

      if (len > 0) {
        uart_port = tmpbaud;
      } else {
        printf("Default port is used\n");
        uart_port = default_uart_port;
      }
    }
  }
  if (!uart_port || !baud_rate) {
    printf(USAGE, argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Initialise the serial port with RTS/CTS enabled. */
  return uartOpen((int8_t*)uart_port, baud_rate, 1, timeout);
}
