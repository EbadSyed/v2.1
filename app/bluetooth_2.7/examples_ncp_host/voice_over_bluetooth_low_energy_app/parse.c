/***********************************************************************************************//**
 * \file   parse.c
 * \brief  Parse source file
 ***************************************************************************************************
 * <b> (C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

#include <string.h>

/* BG stack headers */
#include "gecko_bglib.h"

/* Own header */
#include "parse.h"
#include "config.h"

/***************************************************************************************************
 * Static Function Declarations
 **************************************************************************************************/
static int parse_address(const char *str, bd_addr *addr);
static void usage(void);
static void help(void);
static void printf_configuration(void);

/***************************************************************************************************
 * Public Variables
 **************************************************************************************************/

/***************************************************************************************************
 * Function Definitions
 **************************************************************************************************/

/***********************************************************************************************//**
 *  \brief  Initialize adc_sample_rate variable in configuration structure by data from argument list.
 *  \param[in]  sample rate
 **************************************************************************************************/
void init_sample_rate(adc_sample_rate_t sr)
{
  switch (sr) {
    case sr_8k:
      CONF_get()->adc_sample_rate = sr_8k;
      break;
    case sr_16k:
    default:
      CONF_get()->adc_sample_rate = sr_16k;
      break;
  }
}

/***********************************************************************************************//**
 *  \brief  Initialize adc_resolution variable in configuration structure by data from argument list.
 *  \param[in]  adc resolution
 **************************************************************************************************/
void init_adc_resolution(adc_resolution_t res)
{
  switch (res) {
    case adc_res_8b:
      CONF_get()->adc_resolution = adc_res_8b;
      break;
    case adc_res_12b:
    default:
      CONF_get()->adc_resolution = adc_res_12b;
      break;
  }
}

/***********************************************************************************************//**
 *  \brief  Print bluetooth address on stdout.
 *  \param[in]  bluetooth address
 **************************************************************************************************/
static void printf_address(bd_addr *addr)
{
  printf("Remote address:          %02x:%02x:%02x:%02x:%02x:%02x\n", addr->addr[5], addr->addr[4], addr->addr[3], addr->addr[2], addr->addr[1], addr->addr[0]);
}

/***********************************************************************************************//**
 *  \brief  Parse bluetooth address.
 *  \param[in]  data to parse
 *  \param[out] parsed bluetooth address
 *  \return 0 if success, otherwise -1
 **************************************************************************************************/
static int parse_address(const char *str, bd_addr *addr)
{
  int a[6];
  int i;
  i = sscanf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
             &a[5],
             &a[4],
             &a[3],
             &a[2],
             &a[1],
             &a[0]
             );
  if (i != 6) {
    return -1;
  }

  for (i = 0; i < 6; i++) {
    addr->addr[i] = (uint8_t)a[i];
  }

  return 0;
}

/***********************************************************************************************//**
 *  \brief  Print example of usage that application on stdout.
 **************************************************************************************************/
static void usage(void)
{
  printf("Example of usage:\n");
  printf("  voice_over_bluetooth_low_energy_app.exe -v -p COM1 -b 115200 -a 00:0b:57:1a:8c:2d -s 16 -r 12\n");
  printf("  voice_over_bluetooth_low_energy_app.exe -p COM1 -b 115200 -a 00:0b:57:1a:8c:2d \n");
  printf("  voice_over_bluetooth_low_energy_app.exe -a 00:0b:57:1a:8c:2d \n");
  printf("  voice_over_bluetooth_low_energy_app.exe -h \n");
}

/***********************************************************************************************//**
 *  \brief  Print help on stdout.
 **************************************************************************************************/
static void help(void)
{
  printf("Help:\n");
  printf("-p <port>       - COM port\n");
  printf("                  Default COM port is:\n");
  printf("                  Windows OS - %s\n", DEFAULT_WIN_OS_UART_PORT);
  printf("                  Apple OS   - %s\n", DEFAULT_APPLE_OS_UART_PORT);
  printf("                  Linux OS   - %s\n", DEFAULT_LINUX_OS_UART_PORT);
  printf("-b <baud_rate>  - Baud rate.\n");
  printf("                  Default %d b/s.\n", DEFAULT_UART_BAUD_RATE);
  printf("-o <file_name>  - Output file name.\n");
  printf("                  Default %s.\n", DEFAULT_OUTPUT_FILE_NAME);
  printf("-a <bt_address> - Remote device bluetooth address. \n");
  printf("                  No default bluetooth address.\n");
  printf("-s <8/16>       - ADC sampling rate.\n");
  printf("                  8 or 16 kHz sampling rate can be used. Default - 16 kHz.\n");
  printf("-r <8/12>       - ADC resolution.\n");
  printf("                  8 or 12 bits resolution can be used. Default - 12 bits.\n");
  printf("-f <1/0>        - Enable/Disable filtering.\n");
  printf("                  Default filtering disabled. When filtering enabled HPF filter is used.\n");
  printf("-e <1/0>        - Enable/Disable encoding.\n");
  printf("                  Encoding enabled by default.\n");
  printf("-h              - Help\n");
  printf("-v              - Verbose\n");
  usage();
  exit(EXIT_SUCCESS);
}

/***********************************************************************************************//**
 *  \brief  Print configuration parameters on stdout.
 **************************************************************************************************/
static void printf_configuration(void)
{
  printf("Parameters:\n");
  printf("  Baud rate:               %d\n", CONF_get()->baud_rate);
  printf("  UART port:               %s\n", CONF_get()->uart_port);
  printf("  File name:               %s\n", CONF_get()->out_file_name);
  printf("  "); printf_address(&CONF_get()->remote_address);
  printf("  Audio data notification: %s\n", CONF_get()->audio_data_notification ? "Enabled" : "Disabled");
  printf("  ADC sample rate:         %d[kHz]\n", CONF_get()->adc_sample_rate);
  printf("  ADC resolution:          %d-bits\n", CONF_get()->adc_resolution);
  printf("  Filtering:               %s\n", CONF_get()->filter_enabled ? "Enabled" : "Disabled");
  printf("  Encoding:                %s\n", CONF_get()->encoding_enabled ? "Enabled" : "Disabled");
  printf("\n");
  return;
}

void PAR_parse(int argc, char **argv)
{
  static char uart_port_name[STR_UART_PORT_SIZE];
  bool verbose = false;

  if (argc == 1) {
    help();
  }

  for (uint8_t i = 0; i < argc; i++) {
    if (argv[i][0] == '-') {
      if ( argv[i][1] == 'p') {
        size_t len_arg = strlen(argv[i + 1]);
        size_t len = (len_arg > (STR_UART_PORT_SIZE - 1)) ? STR_UART_PORT_SIZE : len_arg;

        memcpy(uart_port_name, argv[i + 1], len);
        uart_port_name[len + 1] = '\0';
        CONF_get()->uart_port = uart_port_name;
      }

      if ( argv[i][1] == 'o') {
        size_t fLen = strlen(argv[i + 1]) + 1;
        CONF_get()->out_file_name = malloc(fLen);
        memcpy(CONF_get()->out_file_name, argv[i + 1], fLen);
      }

      if ( argv[i][1] == 'a') {
        if (parse_address(argv[i + 1], &CONF_get()->remote_address)) {
          DEBUG_ERROR("Unable to parse address %s", argv[i + 1]);
          exit(EXIT_FAILURE);
        } else {
          CONF_get()->remote_address_set = true;
        }
      }

      if ( argv[i][1] == 'b') {
        CONF_get()->baud_rate = (int)atoi(argv[i + 1]);
      }

      if ( argv[i][1] == 's') {
        init_sample_rate(atoi(argv[i + 1]));
      }

      if ( argv[i][1] == 'r') {
        init_adc_resolution(atoi(argv[i + 1]));
      }

      if ( argv[i][1] == 'f') {
        CONF_get()->filter_enabled = (bool)atoi(argv[i + 1]);
      }

      if ( argv[i][1] == 'e') {
        CONF_get()->encoding_enabled = (bool)atoi(argv[i + 1]);
      }

      if ( argv[i][1] == 'h') {
        help();
      }

      if ( argv[i][1] == 'v') {
        verbose = true;
      }
    }
  }

  if (verbose) {
    printf_configuration();
  }
}
