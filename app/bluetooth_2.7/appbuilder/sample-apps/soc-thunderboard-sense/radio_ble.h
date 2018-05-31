#ifndef __RADIO_BLE_H_
#define __RADIO_BLE_H_

#include <stdint.h>

void RADIO_bleStackInit                (void);
void RADIO_bleStackLoopIteration       (void);
void RADIO_bleChipInit                 (void);
void RADIO_readImuData(bool imuIntIsActive);
const void *RADIO_bleChipGetDCDCConfig (void);

#endif
