#ifndef SLTHREAD_CONFIG_H
#define SLTHREAD_CONFIG_H

// The 4 digit version: A.B.C.D
#define EMBER_MAJOR_VERSION  2
#define EMBER_MINOR_VERSION  5
#define EMBER_PATCH_VERSION  0
#define EMBER_SPECIAL_VERSION  0

// 2 bytes
#define EMBER_BUILD_NUMBER 84

#define EMBER_FULL_VERSION (((uint16_t)EMBER_MAJOR_VERSION << 12)  \
                            | ((uint16_t)EMBER_MINOR_VERSION << 8) \
                            | ((uint16_t)EMBER_PATCH_VERSION << 4) \
                            | ((uint16_t)EMBER_SPECIAL_VERSION))

// 4 bytes
#define EMBER_CHANGE_NUMBER 181248

// 1 byte
#ifdef EMBER_WAKEUP_STACK
#define EMBER_VERSION_TYPE EMBER_VERSION_TYPE_LEGACY
#else
#define EMBER_VERSION_TYPE EMBER_VERSION_TYPE_INTERNAL
#endif

// 2 bytes
#define EMBER_MANAGEMENT_VERSION TMSP_VERSION

// alias used in the HAL
#define SOFTWARE_VERSION EMBER_FULL_VERSION

#endif // SLTHREAD_CONFIG_H
