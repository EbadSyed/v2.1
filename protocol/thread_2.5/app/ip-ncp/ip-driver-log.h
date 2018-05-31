#ifndef IP_DRIVER_LOG_H
#define IP_DRIVER_LOG_H

typedef enum {
  LOG_NCP_TO_IP_STACK,
  LOG_IP_STACK_TO_NCP,
  LOG_APP_TO_NCP,
  LOG_NCP_TO_DRIVER_MGMT,
  LOG_DRIVER_TO_APP,
  LOG_DRIVER_TO_COMM_PROXY_APP,
  LOG_DRIVER_TO_NCP_DATA,
  LOG_DRIVER_TO_NCP_MGMT,
  LOG_RX_ERROR,
  LOG_INIT_EVENT,
  LOG_RX_EVENT,
  LOG_SIGNAL,
} LogEvent;

void ipDriverOpenLogFile(const char *filename);
void ipDriverLogEvent(LogEvent type,
                      const uint8_t *data,
                      uint16_t length,
                      SerialLinkMessageType messageType);
void ipDriverLogStatus(char *format, ...);
void ipDriverLogFlush(void);

#endif // IP_DRIVER_LOG_H
