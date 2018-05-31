/*
 * File: coap-diagnostic.h
 * Description: Thread Diagnostic Functionality
 *
 * Copyright 2016 by Silicon Laboratories. All rights reserved.             *80*
 */

#ifndef SLTHREAD_DIAGNOSTIC_H
#define SLTHREAD_DIAGNOSTIC_H

#define DIAGNOSTIC_GET_URI    "d/dg"
#define DIAGNOSTIC_RESET_URI  "d/dr"
#define DIAGNOSTIC_QUERY_URI  "d/dq"
#define DIAGNOSTIC_ANSWER_URI "d/da"

bool emHandleThreadDiagnosticMessage(const uint8_t *uri,
                                     const uint8_t *payload,
                                     uint16_t payloadLength,
                                     const EmberCoapRequestInfo *info);

bool emParseDiagnosticData(EmberDiagnosticData *data,
                           const uint8_t *payload,
                           uint16_t length);

#endif // SLTHREAD_DIAGNOSTIC_H
