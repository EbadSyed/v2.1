#include COMMON_HEADER
#include "mfg_data_interface.h"

// Manufacturing data are data that is stored to persistans storage at
// manufacturing. We are referencing to manufacturing data here to provide
// the appropriate joining infromation to the network that this device is
// connecting to.
// The manufacturing data that is refereced: EUI64 (either device specific or
// custom) and Install Code. For more information please refer to the
// appropriate documentation.
//
// 1. USERDATA_BASE and LOCKBITS_BASE are coming from the device header
// (eg. platform/Device/SiliconLabs/EFR32MG1P/Include/efr32mg1p233f256gm48.h )
// 2. "Magic values" for the different location are coming from Ember token
// definition (platform/base/hal/micro/cortexm3/efm32/token-manufacturing.h)
// 3. Device info data (DEVINFO_TypeDef) is coming from device info header
// (eg. platform/Device/SiliconLabs/EFR32MG1P/Include/efr32mg1p_devinfo.h)

// USERDATA_BASE
#define MFG_EMBER_EUI_64_LOCATION        (0x1f0)  //   8 bytes
#define MFG_CUSTOM_EUI_64_LOCATION       (0x002)  //   8 bytes
// LOCKBITS_BASE
#define MFG_INSTALLATION_CODE_LOCATION   (0x270)  //  20 bytes

// Install code flag related code is coming from
// znet/stack/security/install-code-tokens.c
#define INSTALL_CODE_FLAGS_MASK 0x00FF
#define INSTALL_CODE_SPECIAL_FLAGS_MASK 0xFF00

#define INSTALL_CODE_DATA_FULL_LEN  (INSTALL_CODE_FLAGS_LEN \
                                     + INSTALL_CODE_MAX_LEN \
                                     + INSTALL_CODE_CRC_LEN)

int8_t getInstallCode(uint8_t *len, uint8_t *installCode, uint16_t *crc)
{
  // Install code flag related code is coming from
  // znet/stack/security/install-code-tokens.c
  static const uint8_t tokenFlagsToSizeArray[] = { 6, 8, 12, 16 };
  uint8_t installCodeData[INSTALL_CODE_DATA_FULL_LEN];
  uint16_t flags, installCodeFlags;
  uint32_t flashAddress = (LOCKBITS_BASE | MFG_INSTALLATION_CODE_LOCATION);

  // Copy all install code data out of flash
  memcpy(installCodeData, (uint8_t*)flashAddress, INSTALL_CODE_DATA_FULL_LEN);

  // Install code flags are stored LSByte
  installCodeFlags = ( ((uint16_t)installCodeData[1]) << 8) + installCodeData[0];

  flags = (INSTALL_CODE_FLAGS_MASK & installCodeFlags) >> 1;
  if (flags == 0) {
    // Special flags support:  Some customers programmed the flags field
    // incorrectly and set the upper bits instead of the lower bits.  Allow
    // both to be supported.  Bugzid 9314.
    flags = (INSTALL_CODE_SPECIAL_FLAGS_MASK & installCodeFlags) >> 9;
  }

  // Test if data is acceptable
  if (sizeof(tokenFlagsToSizeArray) <= flags) {
    return INSTALL_CODE_NOT_CONFIGURED;
  }
  // Get len according to customer flag
  *len = tokenFlagsToSizeArray[flags];

  // read size num of bytes to installCode
  memcpy(installCode, (uint8_t*)&installCodeData[INSTALL_CODE_FLAGS_LEN], *len);

  // Read CRC, stored in LSByte format
  *crc = BYTES_TO_UINT16(installCodeData[INSTALL_CODE_DATA_FULL_LEN - 2],
                         installCodeData[INSTALL_CODE_DATA_FULL_LEN - 1]);

  return INSTALL_CODE_SUCCESS;
}

int8_t getEui64(uint8_t *eui64)
{
  static const uint8_t nullEui[EUI64_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint32_t flashAddress = (USERDATA_BASE | MFG_CUSTOM_EUI_64_LOCATION);

  // Return with unique device ID in case custom EUID is empty
  if (!memcmp((uint8_t*)flashAddress, nullEui, EUI64_LEN)) {
    uint32_t low = (DEVINFO->UNIQUEL);
    uint32_t high = (DEVINFO->UNIQUEH);

    eui64[0] = UINT32_TO_BYTE0(low);
    eui64[1] = UINT32_TO_BYTE1(low);
    eui64[2] = UINT32_TO_BYTE2(low);
    eui64[3] = UINT32_TO_BYTE3(low);
    eui64[4] = UINT32_TO_BYTE0(high);
    eui64[5] = UINT32_TO_BYTE1(high);
    eui64[6] = UINT32_TO_BYTE2(high);
    eui64[7] = UINT32_TO_BYTE3(high);
    return EUI64_DEVICE_ID;
  } else {
    memcpy(eui64, (uint8_t*)flashAddress, EUI64_LEN);
    return EUI64_CUSTOM;
  }
}
