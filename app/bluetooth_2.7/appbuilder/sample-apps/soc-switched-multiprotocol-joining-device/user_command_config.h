#ifndef __USER_COMMAND_CONFIG__
#define __USER_COMMAND_CONFIG__

// Add slot manager command list if applicable
#if defined (SILABS_AF_PLUGIN_SLOT_MANAGER)
#include "slot-manager-cli.h"
#else
#define SLOT_MANAGER_COMMAND_LIST COMMAND_SEPARATOR("")
#endif

void cliSendBleTestData(int abc, char **buf);
void cliResponseYes(int abc, char **buf);
void cliResponseNo(int abc, char **buf);
void cliStartAdvertise(int abc, char **buf);
void cliStopAdvertise(int abc, char **buf);
void cliAllowAllConnections(int abc, char **buf);
void cliDisconnectRemoteClient(int abc, char **buf);
void cliPrintManufData(int abc, char **buf);
void cliTest(int abc, char **buf);

#define USER_COMMAND_LIST                                                                                                                           \
  {                                                                                                                                                 \
    COMMAND_ENTRY("send-data", "u", cliSendBleTestData, "Sends test data over BLE. send-data [0/1]"),                                               \
    COMMAND_ENTRY("y", NULL, cliResponseYes, "Yes!"),                                                                                               \
    COMMAND_ENTRY("n", NULL, cliResponseNo, "No!"),                                                                                                 \
    COMMAND_ENTRY("start-adv", NULL, cliStartAdvertise, "Start advertisement."),                                                                    \
    COMMAND_ENTRY("stop-adv", NULL, cliStopAdvertise, "Stop advertisement."),                                                                       \
    COMMAND_ENTRY("allow-all-conn", "b", cliAllowAllConnections, "Allow non-bonded connections for <min> number of minutes. allow-all-conn <min>"), \
    COMMAND_ENTRY("disconnect", NULL, cliDisconnectRemoteClient, "Disconnects from remote client."),                                                \
    COMMAND_ENTRY("print-manuf", "b", cliPrintManufData, "Print out manufacturing data. [eui64/install]"),                                          \
    COMMAND_ENTRY("test", NULL, cliTest, "CLI test."),                                                                                              \
                                                                                                                                                    \
    SLOT_MANAGER_COMMAND_LIST,                                                                                                                      \
                                                                                                                                                    \
    COMMAND_ENTRY(NULL, NULL, NULL, NULL)                                                                                                           \
  }

#endif // __USER_COMMAND_CONFIG__
