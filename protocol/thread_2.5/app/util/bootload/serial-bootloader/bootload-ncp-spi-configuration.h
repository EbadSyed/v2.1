#ifndef BOOTLOAD_NCP_SPI_CONFIGURATION_H
#define BOOTLOAD_NCP_SPI_CONFIGURATION_H

#define EMBER_APP_NAME "bootload-ncp-spi-app"
#define APP_SERIAL 0
#define EMBER_ASSERT_SERIAL_PORT 0
#define EMBER_SERIAL0_BLOCKING
#define EMBER_SERIAL0_MODE EMBER_SERIAL_FIFO
#define EMBER_SERIAL0_TX_QUEUE_SIZE 128
#define EMBER_SERIAL0_RX_QUEUE_SIZE 128
#define ARG_LENGTH 40

#endif // BOOTLOAD_NCP_SPI_CONFIGURATION_H