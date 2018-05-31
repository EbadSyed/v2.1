/**************************************************************************//**
 * Copyright 2016 by Silicon Laboratories Inc. All rights reserved.
 *
 * http://developer.silabs.com/legal/version/v11/Silicon_Labs_Software_License_Agreement.txt
 *****************************************************************************/

#ifndef __CSLIB_HWCONFIG_H__
#define __CSLIB_HWCONFIG_H__

#include "em_device.h"
#include "em_csen.h"

/// @brief Defines the size of the sensor node array.
/// Also defines volatile arrays that have a one-to-one correspondence
/// to the number of sensors in the project.
/// @note Minimum value is 1, maximum value is the number of capacitive
/// sensing-enabled pins on the device
#define DEF_NUM_SENSORS                           4

/// @brief Cross reference between sensor number ordering and APORT ordinal
/// ordering.  For example, if a slider is using APORT 3, 0, 4, 5, the DMA
/// engine returns data as 0, 3, 4, 5, but it has to be reordered so the numbers
/// are presented as 3, 0, 4, 5.  MUX_VALUE_ARRAY in this case is 1, 0, 2, 3
#define MUX_VALUE_ARRAY 0, 1, 2, 3

/// @brief Per-channel active threshold settings.  When consecutive conversions
/// for a channel rise above this threshold, the sensor will be qualified as active.
/// @note Minimum threshold used is @ref INACTIVE_THRESHOLD_ARRAY value,
/// maximum value is 100
#define ACTIVE_THRESHOLD_ARRAY 70, 70, 70, 70

/// @brief Per-channel inactive threshold settings.  When consecutive conversions
/// for a channel fall below this threshold, the sensor will be qualified as inactive.
/// @note Minimum threshold used is 1, maximum value is @ref ACTIVE_THRESHOLD_ARRAY
#define INACTIVE_THRESHOLD_ARRAY 30, 30, 30, 30

/// @brief Per-channel expected touch delta.  This value describes the difference
/// in capacitive sensing output codes between the inactive/baseline of the sensor,
/// and the output of the sensor when active(touched).
/// @note These values should be defined in terms of X/16, or X>>4, as they are stored
/// in a packed byte array
#define AVERAGE_TOUCH_DELTA_ARRAY 2048 >> 4, 2048 >> 4, 2048 >> 4, 2048 >> 4

#define CSEN_ACTIVEMODE_DEFAULT                                         \
  {                                                                     \
    csenSampleModeScan,         /* Sample one input and stop. */        \
    csenTrigSelTimer,           /* Use start bit to trigger. */         \
    true,                       /* Enable DMA. */                       \
    false,                      /* Average the accumulated result. */   \
    csenAccMode8,               /* Accumulate 1 sample. */              \
    csenEMASampleW1,            /* Disable the EMA. */                  \
    csenCmpModeDisabled,        /* Disable the comparator. */           \
    0,                          /* Comparator threshold not used. */    \
    csenSingleSelDefault,       /* Disconnect the single input. */      \
    0x000F,                     /* Enable inputs 1,3,5,7.  */           \
    0,                          /* Disable inputs 32 to 63. */          \
    false,                      /* Do not ground inactive inputs. */    \
    csenConvSelDM,             /* Use the DM mode. */                   \
    csenSARRes10,               /* Set SAR resolution to 10 bits. */    \
    csenDMRes16,                /* Set DM resolution to 10 bits. */     \
    4,                          /* Set DM conv/cycle to default. */     \
    6,                          /* Set DM cycles to default. */         \
    128,                         /* Set DM initial delta to default. */ \
    true,                       /* Use DM auto delta reduction. */      \
    csenResetPhaseSel0,         /* Use shortest reset phase time. */    \
    csenDriveSelFull,           /* Use full output current. */          \
    csenGainSel8X,              /* Use highest converter gain. */       \
  }

#define CSEN_SLEEPMODE_DEFAULT                                         \
  {                                                                    \
    csenSampleModeBonded,       /* Sample bonded channels. */          \
    csenTrigSelTimer,           /* Use start bit to trigger. */        \
    false,                      /* Disable DMA. */                     \
    true,                       /* Average the accumulated result. */  \
    csenAccMode8,               /* Accumulate 1 sample. */             \
    csenEMASampleW8,            /* Set EMA to W8.   */                 \
    csenCmpModeEMAWindow,       /* Enable EMA comparator. */           \
    255,                        /* Set wake value +/-EMA. */           \
    csenSingleSelDefault,       /* Disconnect the single input. */     \
    0x000F,                     /* Disable inputs 0 to 31. */          \
    0,                          /* Disable inputs 32 to 63. */         \
    false,                      /* Do not ground inactive inputs. */   \
    csenConvSelSAR,             /* Use the SAR converter. */           \
    csenSARRes10,               /* Set SAR resolution to 10 bits. */   \
    csenDMRes10,                /* Set DM resolution to 10 bits. */    \
    1,                          /* Set DM conv/cycle to default. */    \
    6,                          /* Set DM cycles to default. */        \
    128,                        /* Set DM initial delta to default. */ \
    true,                      /* Use DM auto delta reduction. */      \
    csenResetPhaseSel0,         /* Use shortest reset phase time. */   \
    csenDriveSelFull,           /* Use full output current. */         \
    csenGainSel8X,              /* Use highest converter gain. */      \
  }

/// @brief Array of cross-references between sensor number and APORT number
extern const uint8_t CSLIB_muxValues[];

/** @} (end cslib_HWconfig) */

#endif // __CSLIB_HWCONFIG_H__
