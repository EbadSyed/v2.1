#include COMMON_HEADER
#include "mpsi_handlers.h"
#include "mpsi_ble_transport_server.h"
#include "command_interpreter.h"

void cliSendBleTestData(int abc, char **buf)
{
  sendBleTestData(ciGetUnsigned(buf[1]));
}

void cliResponseYes(int abc, char **buf)
{
  bleMpsiTransportConfirmResponse(true);
}

void cliResponseNo(int abc, char **buf)
{
  bleMpsiTransportConfirmResponse(false);
}

void cliStartAdvertise(int abc, char **buf)
{
  bleStartAdvertisement();
}

void cliStopAdvertise(int abc, char **buf)
{
  bleStopAdvertisement();
}

void cliAllowAllConnections(int abc, char **buf)
{
  uint8_t timeoutMin = ciGetUnsigned(buf[1]);
  bleSetTimeoutForAcceptingAllConnections(timeoutMin);
}

void cliDisconnectRemoteClient(int abc, char **buf)
{
  bleDisconnect();
}

void cliTest(int abc, char **buf)
{
  printf("Test OK!\n");
}
