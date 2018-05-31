#ifndef __USER_COMMAND_CONFIG__
#define __USER_COMMAND_CONFIG__

void cliSendBleTestData(int abc, char **buf);
void cliConnectRemoteServer(int abc, char **buf);
void cliStartDeviceDiscovery(int abc, char **buf);
void cliStopDeviceDiscovery(int abc, char **buf);
void cliMpsiAppInfo(int abc, char **buf);
void cliMpsiBootloadSlot(int abc, char **buf);
void cliMpsiInitiateJoining(int abc, char **buf);
void cliMpsiGetZigbeeJoiningInfo(int abc, char **buf);
void cliMpsiSetZigbeeJoiningInfo(int abc, char **buf);
void cliMpsiGetZigbeeTrustCenterJoiningCredentials(int abc, char **buf);
void cliSendOtaFile(int abc, char **buf);
void cliTest(int abc, char **buf);

#define USER_COMMAND_LIST                                                                                                                                                                                                                                   \
  {                                                                                                                                                                                                                                                         \
    COMMAND_ENTRY("send-data", "u", cliSendBleTestData, "Sends test data over BLE. send-data [0/1]"),                                                                                                                                                       \
    COMMAND_ENTRY("connect", "b", cliConnectRemoteServer, "Connects to specified remote server. connect [jd/tc]"),                                                                                                                                          \
    COMMAND_ENTRY("start-disc", "b", cliStartDeviceDiscovery, "Starts BLE device discovery process. start-disc <param>"),                                                                                                                                   \
    COMMAND_ENTRY("stop-disc", NULL, cliStopDeviceDiscovery, "Stops BLE device discovery process."),                                                                                                                                                        \
    COMMAND_ENTRY("app-info", "bb", cliMpsiAppInfo, "Gets app info. app-info [jd/tc] [ble/zb]"),                                                                                                                                                            \
    COMMAND_ENTRY("boot-slot", "b", cliMpsiBootloadSlot, "Sends the bootload slot command to the joining device. boot-slot <slotId>"),                                                                                                                      \
    COMMAND_ENTRY("init-join", "bbu", cliMpsiInitiateJoining, "BLE: Advertise for <option> minutes. ZB: Attempt to join a network with current parameters. init-join [jd/tc] [ble/zb] <option>"),                                                           \
    COMMAND_ENTRY("get-zb-join-info", NULL, cliMpsiGetZigbeeJoiningInfo, "Gets ZigBee joining device info from the switched multiprotocol device."),                                                                                                        \
    COMMAND_ENTRY("send-zb-join-info", NULL, cliMpsiSetZigbeeJoiningInfo, "Sends ZigBee joining device info to the trust-center."),                                                                                                                         \
    COMMAND_ENTRY("get-zb-tc-join-cred", NULL, cliMpsiGetZigbeeTrustCenterJoiningCredentials, "Gets ZigBee joining credential from the trust-center device."),                                                                                              \
    COMMAND_ENTRY("send-ota-file", "bu", cliSendOtaFile, "Sends the specified OTA file to joining device. It is sent with the specified BLE write operation (write command / write request) and stored to specified slot. send-ota-file [wr/wc] <slotId>"), \
    COMMAND_ENTRY("test", NULL, cliTest, "CLI test."),                                                                                                                                                                                                      \
                                                                                                                                                                                                                                                            \
    COMMAND_ENTRY(NULL, NULL, NULL, NULL)                                                                                                                                                                                                                   \
  }

#endif // __USER_COMMAND_CONFIG__
