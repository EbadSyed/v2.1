#ifndef __MFG_DATA_INTERFACE__
#define __MFG_DATA_INTERFACE__

// EUI64 options
#define EUI64_DEVICE_ID 1
#define EUI64_CUSTOM    2

// Install code return values
#define INSTALL_CODE_NOT_CONFIGURED -1
#define INSTALL_CODE_SUCCESS    0

// Eui64 is 8 bytes long
#define EUI64_LEN 8

// Field length of install code
#define INSTALL_CODE_FLAGS_LEN  2
#define INSTALL_CODE_MAX_LEN    16
#define INSTALL_CODE_CRC_LEN    2

int8_t getInstallCode(uint8_t *len, uint8_t *installCode, uint16_t *crc);
int8_t getEui64(uint8_t *eui64);

#endif //__MFG_DATA_INTERFACE__
