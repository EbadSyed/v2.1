/*
 *********************************************************************************************************
 *                                             EXAMPLE CODE
 *********************************************************************************************************
 * Licensing terms:
 *   This file is provided as an example on how to use Micrium products. It has not necessarily been
 *   tested under every possible condition and is only offered as a reference, without any guarantee.
 *
 *   Please feel free to use any application code labeled as 'EXAMPLE CODE' in your application products.
 *   Example code may be used as is, in whole or in part, or may be used as a reference only. This file
 *   can be modified as required.
 *
 *   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
 *
 *   You can contact us at: http://www.micrium.com
 *
 *   Please help us continue to provide the Embedded community with the finest software available.
 *
 *   Your honesty is greatly appreciated.
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *
 *                                   EXAMPLE NETWORK CORE INITIALISATION
 *                                        ETHERNET & WiFi INTERFACE
 *
 * File : net_core.h
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                               MODULE
 *********************************************************************************************************
 *********************************************************************************************************
 */

#ifndef  EX_NET_CORE_H
#define  EX_NET_CORE_H

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                            INCLUDE FILES
 *********************************************************************************************************
 *********************************************************************************************************
 */

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                         FUNCTION PROTOTYPES
 *********************************************************************************************************
 *********************************************************************************************************
 */

void  Ex_Net_CoreInit       (void);

void  Ex_Net_CoreStartIF    (void);

void  Ex_Net_CoreStartEther (void);

void  Ex_Net_CoreStartWiFi  (void);

/*
 *********************************************************************************************************
 *********************************************************************************************************
 *                                             MODULE END
 *********************************************************************************************************
 *********************************************************************************************************
 */

#endif /* EX_NET_CORE_H */