/***************************************************************************//**
 * @file net_core.c
 * @brief Network core initialization using ethernet interface
 * @version 5.3.5
 *******************************************************************************
 * # License
 * <b>Copyright 2017 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#include  <rtos_description.h>

#include  "../../description.h"

#include <stdbool.h>

#include  <net_cfg.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_str.h>
#include  <rtos/common/include/lib_mem.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/rtos_utils.h>

#include  <rtos/net/include/net.h>
#include  <rtos/net/include/net_type.h>
#include  <rtos/net/include/net_if.h>
#include  <rtos/net/include/net_ascii.h>
#include  <rtos/net/include/dhcp_client.h>
#include  <rtos/net/include/dhcp_client_types.h>
#include  <rtos/net/include/net_ipv4.h>
#include  <rtos/net/include/net_ipv6.h>
#include  <rtos/net/include/net_if_ether.h>

#ifdef  RTOS_MODULE_NET_IF_LOOPBACK_AVAIL
#include  <rtos/net/include/net_if_loopback.h>
#endif

#include "em_device.h"

/***************************************************************************//**
 * @brief logging: This example outputs information to the console via 
 *                 the function printf() via a macro called EX_TRACE().
 *                 This can be modified or disabled if printf() is not supported
 ******************************************************************************/
#ifndef EX_TRACE
#include  <stdio.h>
#define  EX_TRACE(...)                                      printf(__VA_ARGS__)
#endif

// -----------------------------------------------------------------------------
// DEFINES

#if (defined(RTOS_MODULE_NET_IF_ETHER_AVAIL) \
  || defined(RTOS_MODULE_NET_IF_WIFI_AVAIL))
#define  EX_NET_CORE_IF_EXT_EN
#endif

// -----------------------------------------------------------------------------
// Global Variables

#if (RTOS_CFG_EXTERNALIZE_OPTIONAL_CFG == DEF_ENABLED)
extern  const   NET_INIT_CFG  Net_InitCfgDflt;
#endif

NET_IF_NBR ifnNbr;

// -----------------------------------------------------------------------------
// Local Function Prototypes

#ifdef  EX_NET_CORE_IF_EXT_EN
static  void  Ex_IPv6_AddrCfgResult          (NET_IF_NBR                  if_nbr,
                                              NET_IPv6_CFG_ADDR_TYPE      addr_type,
                                              const  NET_IPv6_ADDR              *p_addr_cfgd,
                                              NET_IPv6_ADDR_CFG_STATUS    addr_cfg_status);

static  void  Ex_DHCPc_SetupResult           (NET_IF_NBR                  if_nbr,
                                              DHCPc_STATUS                status,
                                              NET_IPv4_ADDR               addr,
                                              NET_IPv4_ADDR               mask,
                                              NET_IPv4_ADDR               gateway,
                                              RTOS_ERR                    err);

static  void  Ex_IPv4_LinkLocalAddrCfgResult (NET_IF_NBR                  if_nbr,
                                              NET_IPv4_ADDR               link_local_addr,
                                              NET_IPv4_LINK_LOCAL_STATUS  status,
                                              RTOS_ERR                    err);

#if ((NET_IPv4_CFG_EN == DEF_ENABLED) \
  && (NET_DHCP_CLIENT_CFG_MODULE_EN == DEF_ENABLED))
static  void  Ex_DHCPc_Hook                  (NET_IF_NBR                  if_nbr,
                                              DHCPc_STATUS                status,
                                              NET_IPv4_ADDR               addr,
                                              NET_IPv4_ADDR               mask,
                                              NET_IPv4_ADDR               gateway,
                                              RTOS_ERR                    err);
#endif
#endif

static void  Ex_MacAddrGet(CPU_CHAR * addr_mac_str);

// -----------------------------------------------------------------------------
// Local Global Variables

static  const  NET_IF_ETHER_CFG  Ex_NetIF_CfgDflt_Ether = {
  .HW_AddrStr                    = "00:00:00:00:00:00",

  .IPv4.Static.Addr              = DEF_NULL,
  .IPv4.Static.Mask              = DEF_NULL,
  .IPv4.Static.Gateway           = DEF_NULL,

  .IPv4.DHCPc.En                 = DEF_YES,
  .IPv4.DHCPc.Cfg                = DHCPc_CFG_DFLT,
  .IPv4.DHCPc.OnCompleteHook     = Ex_DHCPc_SetupResult,

  .IPv4.LinkLocal.En             = DEF_NO,
  .IPv4.LinkLocal.OnCompleteHook = Ex_IPv4_LinkLocalAddrCfgResult,

  .IPv6.Static.Addr              = DEF_NULL,
  .IPv6.Static.PrefixLen         = 0,
  .IPv6.Static.DAD_En            = DEF_NO,
  .IPv6.AutoCfg.En               = DEF_YES,
  .IPv6.AutoCfg.DAD_En           = DEF_NO,
  .IPv6.Hook                     = Ex_IPv6_AddrCfgResult
};

/***************************************************************************//**
 * @brief Initialize the Network TCP-IP core and add an Ethernet Interface.
 * @note Simplest initialization without any overwrite of default configuration
 ******************************************************************************/
void  Ex_Net_CoreInit(void)
{
  RTOS_ERR    err;

  // INITIALIZE NETWORK TASKS & OBJECTS
  Net_Init(&err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

#ifdef  RTOS_MODULE_NET_IF_LOOPBACK_AVAIL
  // ADD LOOPBACK INTERFACE
  NetIF_Loopback_Init(DEF_NULL, &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
#endif

  // ADD ETHERNET INTERFACE
  NetIF_Ether_Add("eth0",
                  DEF_NULL,
                  DEF_NULL,
                  &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
}

/***************************************************************************//**
 * @brief Starts network interface(s).
 * @note (1) If the Ethernet module is available and the "eth0" was registered 
 *           and added, this function will start that network interface.
 *       (2) If the WiFi module is available and the "wifi0" was registered
 *           and added, this function will start that network interface.
 ******************************************************************************/
void  Ex_Net_CoreStartIF(void)
{
  NET_IF_NBR         if_nbr;
  RTOS_ERR           err;
  NET_IF_ETHER_CFG   if_cfg_ether;
  CPU_CHAR           addr_mac_str[NET_IF_802x_ADDR_SIZE_STR];
  
  // INTERFACE SETTINGS
  // Keep Default settings that enables DHCP client for...
  // IPv4 and SLAAC for IPv6.
  if_cfg_ether = Ex_NetIF_CfgDflt_Ether;
  Ex_MacAddrGet(addr_mac_str);
  if_cfg_ether.HW_AddrStr = addr_mac_str;

  // RECOVER INTERFACE NUMBER FROM INTEFACE NAME
  if_nbr = NetIF_NbrGetFromName("eth0");
  APP_RTOS_ASSERT_CRITICAL((if_nbr != NET_IF_NBR_NONE),; );

  // START ETHERNET INTERFACE
  NetIF_Ether_Start(if_nbr, &if_cfg_ether, &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

#if (NET_IF_CFG_WAIT_SETUP_READY_EN == DEF_ENABLED)
  NET_IF_APP_INFO if_app_info;
  // WAIT FOR INTEFACE SETUP TO FINISH
  printf("eth0: Waiting for interface to be ready\n");
  NetIF_WaitSetupReady(if_nbr, &if_app_info, 0, &err);
  printf("eth0: interface ready\n");
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
#endif
}

/***************************************************************************//**
 * @brief Start an Ethernet Interface.
 * @note (1) In this example no NET_IF_ETHER_CFG structure is passed to 
 *           the start function.
 *           Therefore, no address setup is done by the start function
 *           and it must be done explicitly with DHCP client,
 *           IPv4 and IPv6 API calls, and after the call to NetIF_Ether_Start().
 ******************************************************************************/
#ifdef  RTOS_MODULE_NET_IF_ETHER_AVAIL
void  Ex_Net_CoreStartEther(void)
{
  NET_IF_NBR      if_nbr;
  RTOS_ERR        err;
#if (NET_IPv4_CFG_EN == DEF_ENABLED)
  NET_IPv4_ADDR   addr_ipv4;
  NET_IPv4_ADDR   msk_ipv4;
  NET_IPv4_ADDR   gateway_ipv4;
#endif
#if  (NET_IPv6_CFG_EN == DEF_ENABLED)
  CPU_BOOLEAN     cfg_result  = DEF_FAIL;
  NET_FLAGS       ipv6_flags;
  NET_IPv6_ADDR   addr_ipv6;
#endif

  // RECOVER INTERFACE NUMBER FROM INTEFACE NAME
  if_nbr = NetIF_NbrGetFromName("eth0");
  APP_RTOS_ASSERT_CRITICAL((if_nbr != NET_IF_NBR_NONE),; );

  // START ETHERNET INTERFACE
  NetIF_Ether_Start(if_nbr, DEF_NULL, &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

#if (NET_IPv4_CFG_EN == DEF_ENABLED)
  // ENABLE DHCP PROCESS ON NETWORK INTEFACE
#if (NET_DHCP_CLIENT_CFG_MODULE_EN == DEF_ENABLED)
  DHCPc_IF_Add(if_nbr, DEF_NULL, Ex_DHCPc_Hook, &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
#endif

  // CONFIGURE IPV4 STATIC ADDRESSES
  // TODO Update IPv4 Address, Mask and Gateway below...
  // following your network requirements.

  // Convert Host IPv4 string address to 32 bit address.
  NetASCII_Str_to_IP("10.10.10.64",
                     &addr_ipv4,
                     NET_IPv4_ADDR_SIZE,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

  // Convert IPv4 mask string to 32 bit address.
  NetASCII_Str_to_IP("255.255.255.0",
                     &msk_ipv4,
                     NET_IPv4_ADDR_SIZE,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

  // Convert Gateway string address to 32 bit address.
  NetASCII_Str_to_IP("10.10.10.1",
                     &gateway_ipv4,
                     NET_IPv4_ADDR_SIZE,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

  // Add a statically-configured IPv4 host address,   ...
  // ... subnet mask, & default gateway to the        ...
  // ... interface.
  NetIPv4_CfgAddrAdd(if_nbr,
                     addr_ipv4,
                     msk_ipv4,
                     gateway_ipv4,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
#endif // NET_IPv4_CFG_EN

#if  (NET_IPv6_CFG_EN == DEF_ENABLED)

  // Set hook function to received addr cfg result.
  NetIPv6_AddrSubscribe(&Ex_IPv6_AddrCfgResult, &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

#if  (NET_IPv6_CFG_ADDR_AUTO_CFG_EN == DEF_ENABLED)
  // ENABLE IPV6 STATELESS AUTO-CONFIGURATION

  // Enable and Start Auto-Configuration process.
  cfg_result = NetIPv6_AddrAutoCfgEn(if_nbr,
                                     DEF_YES,
                                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
#endif
  // CONFIGURE IPV6 STATIC LINK LOCAL ADDRESS
  // DHCPv6c is not yet available.

  // TODO Update IPv6 Address below following your network requirements.

  // Convert IPv6 string address to 128 bit address.
  NetASCII_Str_to_IP("fe80::1111:1111",
                     &addr_ipv6,
                     NET_IPv6_ADDR_SIZE,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

  ipv6_flags = 0;
  // Enable DAD with Address Configuration.
  DEF_BIT_SET(ipv6_flags, NET_IPv6_FLAG_DAD_EN);

  // Add a statically-configured IPv6 host address to ...
  // ... the interface.
  NetIPv6_CfgAddrAdd(if_nbr,
                     &addr_ipv6,
                     64,
                     ipv6_flags,
                     &err);
  APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );

  PP_UNUSED_PARAM(cfg_result);
#endif // NET_IPv6_CFG_EN
}
#endif

/***************************************************************************//**
 *  @brief Hook function to received 
 *         any IPv6 address configuration process result.
 *
 *  @param if_nbr Network Interface number on 
 *                 which address configuration occurred.
 *
 *  @param addr_type IPv6 address type:
 *                                  NET_IPv6_CFG_ADDR_TYPE_STATIC
 *                                  NET_IPv6_CFG_ADDR_TYPE_AUTO_CFG_LINK_LOCAL
 *                                  NET_IPv6_CFG_ADDR_TYPE_AUTO_CFG_GLOBAL
 *
 *  @param p_addr_cfgd Pointer to IPv6 address configured, if any.
 *                     DEF_NULL, otherwise.
 *
 *  @param addr_cfg_status Result status of the IPv6 address 
 *                         configuration process:
 *                                  NET_IPv6_ADDR_CFG_STATUS_SUCCEED
 *                                  NET_IPv6_ADDR_CFG_STATUS_FAIL
 *                                  NET_IPv6_ADDR_CFG_STATUS_DUPLICATE
 ******************************************************************************/
#ifdef  EX_NET_CORE_IF_EXT_EN
static  void  Ex_IPv6_AddrCfgResult(NET_IF_NBR                 if_nbr,
                                    NET_IPv6_CFG_ADDR_TYPE     addr_type,
                                    const  NET_IPv6_ADDR             *p_addr_cfgd,
                                    NET_IPv6_ADDR_CFG_STATUS   addr_cfg_status)
{
  CPU_CHAR   ip_string[NET_ASCII_LEN_MAX_ADDR_IPv6];
  RTOS_ERR   err;

  PP_UNUSED_PARAM(if_nbr);

  if (p_addr_cfgd != DEF_NULL) {
    NetASCII_IPv6_to_Str((NET_IPv6_ADDR *)p_addr_cfgd, ip_string, DEF_NO, DEF_NO, &err);
    APP_RTOS_ASSERT_CRITICAL((err.Code == RTOS_ERR_NONE),; );
    EX_TRACE("IPv6 Address Link Local: %s\n", ip_string);
  }

  switch (addr_type) {
    case NET_IPv6_CFG_ADDR_TYPE_STATIC:
      switch (addr_cfg_status) {
        case NET_IPv6_ADDR_CFG_STATUS_SUCCEED:
          EX_TRACE("IPv6 Address Static: %s, configured successfully\n", ip_string);
          break;

        case NET_IPv6_ADDR_CFG_STATUS_DUPLICATE:
          EX_TRACE("IPv6 Address Static already exists on the network\n");
          break;

        default:
          EX_TRACE("IPv6 Address Static configuration failed.\n");
          break;
      }
      break;

    case NET_IPv6_CFG_ADDR_TYPE_AUTO_CFG_LINK_LOCAL:
      switch (addr_cfg_status) {
        case NET_IPv6_ADDR_CFG_STATUS_SUCCEED:
          EX_TRACE("IPv6 Address Link Local: %s, configured successfully\n", ip_string);
          break;

        case NET_IPv6_ADDR_CFG_STATUS_DUPLICATE:
          EX_TRACE("IPv6 Address Link Local already exists on the network\n");
          break;

        default:
          printf("IPv6 Address Link Local configuration failed.\n");
          break;
      }
      break;

    case NET_IPv6_CFG_ADDR_TYPE_AUTO_CFG_GLOBAL:
      switch (addr_cfg_status) {
        case NET_IPv6_ADDR_CFG_STATUS_SUCCEED:
          EX_TRACE("IPv6 Address Global: %s, configured successfully\n", ip_string);
          break;

        case NET_IPv6_ADDR_CFG_STATUS_DUPLICATE:
          EX_TRACE("IPv6 Address Global already exists on the network\n");
          break;

        default:
          EX_TRACE("IPv6 Address Global configuration failed.\n");
          break;
      }
      break;

    default:
      break;
  }
}
#endif

/***************************************************************************//**
 *  @brief Hook function called when DHCP process 
 *         has been complete on an network interface.
 *
 *  @param if_nbr Network Interface number on which DHCP process occurred.
 *
 *  @param status DHCP process status:
 *                               DHCPc_STATUS_SUCCESS
 *                               DHCPc_STATUS_FAIL_ADDR_USED
 *                               DHCPc_STATUS_FAIL_OFFER_DECLINE
 *                               DHCPc_STATUS_FAIL_NAK_RX
 *                               DHCPc_STATUS_FAIL_NO_SERVER
 *                               DHCPc_STATUS_FAIL_ERR_FAULT
 *
 *  @param addr IPv4 address configured, if any.
 *              Else, NET_IPv4_ADDR_NONE.
 *
 *  @param mask IPv4 mask configured, if any.
 *              Else, NET_IPv4_ADDR_NONE.
 *
 *  @param gateway IPv4 gateway configured, if any.
 *                 Else, NET_IPv4_ADDR_NONE.
 *
 *  @param err Error object in case an error occurred.
 *             Else, RTOS_ERR_NONE.
 ******************************************************************************/
#ifdef  EX_NET_CORE_IF_EXT_EN
static  void  Ex_DHCPc_SetupResult(NET_IF_NBR     if_nbr,
                                   DHCPc_STATUS   status,
                                   NET_IPv4_ADDR  addr,
                                   NET_IPv4_ADDR  mask,
                                   NET_IPv4_ADDR  gateway,
                                   RTOS_ERR       err)
{
  CPU_CHAR   addr_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  CPU_CHAR   mask_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  CPU_CHAR   gateway_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  RTOS_ERR   local_err;

  PP_UNUSED_PARAM(if_nbr);

  switch (status) {
    case DHCPc_STATUS_SUCCESS:
      NetASCII_IPv4_to_Str(addr, addr_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      NetASCII_IPv4_to_Str(mask, mask_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      NetASCII_IPv4_to_Str(gateway, gateway_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      EX_TRACE("DHCP client process succeeded.\n");
      EX_TRACE("IP address configured : %s .\n", addr_string);
      EX_TRACE("IP address mask       : %s .\n", mask_string);
      EX_TRACE("IP address gateway    : %s .\n", gateway_string);
      break;

    case DHCPc_STATUS_FAIL_ADDR_USED:
      EX_TRACE("DHCP client process failed: Address already used on network.\n");
      break;

    case DHCPc_STATUS_FAIL_OFFER_DECLINE:
      EX_TRACE("DHCP client process failed: DHCP client declined the offer.\n");
      break;

    case DHCPc_STATUS_FAIL_NAK_RX:
      EX_TRACE("DHCP client process failed: The DHCP server refused the client.\n");
      break;

    case DHCPc_STATUS_FAIL_NO_SERVER:
      EX_TRACE("DHCP client process failed: No DHCP server was detected on the network.\n");
      break;

    case DHCPc_STATUS_FAIL_ERR_FAULT:
#if ((RTOS_ERR_CFG_EXT_EN == DEF_ENABLED) \
      && (RTOS_ERR_CFG_STR_EN == DEF_ENABLED))
      EX_TRACE("DHCP client process failed: Fatal error %s.\n", err.CodeText);
#else
      EX_TRACE("DHCP client process failed: Fatal error %i.\n", err.Code);
#endif
      break;

    default:
      EX_TRACE("DHCP client invalid result.\n");
      break;
  }
}
#endif

/***************************************************************************//**
 *  @brief Hook function called when IPv4 Link local process 
 *         has been complete on an network interface.
 *
 *  @param if_nbr Network Interface number on which 
 *                IPv4 link local process occurred.
 *
 *  @param link_local_addr IPv4 link local address, if configured.
 *                         Else, NET_IPv4_ADDR_NONE.
 *
 *  @param status IPv4 Link Local process status:
 *                                       NET_IPv4_LINK_LOCAL_STATUS_NONE
 *                                       NET_IPv4_LINK_LOCAL_STATUS_SUCCEEDED
 *                                       NET_IPv4_LINK_LOCAL_STATUS_FAILED
 *
 *  @param err Error object in case an error occurred.
 *             Else, RTOS_ERR_NONE.
 *  @note IPv4 link local address configuration will only be started 
 *        if its enabled in the
 *                   NET_IF_ETHER_CFG object, but also if no other 
 *                   IPv4 valid addresses are configured on the network interface.
 *                   Therefore, it will occurred if :
 *                       (a)  no IPv4 static address is set up and DHCP process is disabled.
 *                       (b)  the DHCP setup fails and no IPv4 static address is set up.
 ******************************************************************************/
#ifdef  EX_NET_CORE_IF_EXT_EN
static  void  Ex_IPv4_LinkLocalAddrCfgResult(NET_IF_NBR                  if_nbr,
                                             NET_IPv4_ADDR               link_local_addr,
                                             NET_IPv4_LINK_LOCAL_STATUS  status,
                                             RTOS_ERR                    err)
{
  CPU_CHAR  addr_str[NET_ASCII_LEN_MAX_ADDR_IPv4];
  RTOS_ERR  local_err;

  EX_TRACE("On Interface number #%i, \n", if_nbr);

  switch (status) {
    case NET_IPv4_LINK_LOCAL_STATUS_SUCCEEDED:
      if (link_local_addr != NET_IPv4_ADDR_NONE) {
        NetASCII_IPv4_to_Str(link_local_addr, addr_str, DEF_YES, &local_err);
        if (local_err.Code != RTOS_ERR_NONE) {
          return;
        }
      }
      EX_TRACE("IPv4 link local address: %s, was configured successfully!\n", addr_str);
      break;

    case NET_IPv4_LINK_LOCAL_STATUS_FAILED:
#if ((RTOS_ERR_CFG_EXT_EN == DEF_ENABLED) \
      && (RTOS_ERR_CFG_STR_EN == DEF_ENABLED))
      EX_TRACE("IPv4 link local address configuration failed: %s.\n", err.CodeText);
#else
      EX_TRACE("IPv4 link local address configuration failed: %i.\n", err.Code);
#endif
      break;

    default:
      break;
  }
}
#endif

/***************************************************************************//**
 *  @brief Hook function called when DHCP process 
 *         has been complete on an network interface.
 *
 *  @param if_nbr Network Interface number on which DHCP process occurred.
 *
 *  @param status DHCP process status:
 *                               DHCPc_STATUS_SUCCESS
 *                               DHCPc_STATUS_FAIL_ADDR_USED
 *                               DHCPc_STATUS_FAIL_OFFER_DECLINE
 *                               DHCPc_STATUS_FAIL_NAK_RX
 *                               DHCPc_STATUS_FAIL_NO_SERVER
 *                               DHCPc_STATUS_FAIL_ERR_FAULT
 *
 *  @param addr IPv4 address configured, if any.
 *              Else, NET_IPv4_ADDR_NONE.
 *
 *  @param mask IPv4 mask configured, if any.
 *              Else, NET_IPv4_ADDR_NONE.
 *
 *  @param gateway IPv4 gateway configured, if any.
 *                 Else, NET_IPv4_ADDR_NONE.
 *
 *  @param err Error object in case an error occurred.
 *             Else, RTOS_ERR_NONE.
 *
 *  @note This hook is used for the manual network interface setup. Therefore if the DHCP
 *        setup fails, the IPv4 link local address configuration API is called to start the
 *        link local process.
 ******************************************************************************/
#if ((NET_IPv4_CFG_EN               == DEF_ENABLED) \
  && (NET_DHCP_CLIENT_CFG_MODULE_EN == DEF_ENABLED) \
  && (defined(EX_NET_CORE_IF_EXT_EN)))
static  void  Ex_DHCPc_Hook(NET_IF_NBR     if_nbr,
                            DHCPc_STATUS   status,
                            NET_IPv4_ADDR  addr,
                            NET_IPv4_ADDR  mask,
                            NET_IPv4_ADDR  gateway,
                            RTOS_ERR       err)
{
  CPU_CHAR   addr_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  CPU_CHAR   mask_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  CPU_CHAR   gateway_string[NET_ASCII_LEN_MAX_ADDR_IPv4];
  RTOS_ERR   local_err;

  PP_UNUSED_PARAM(if_nbr);

  switch (status) {
    case DHCPc_STATUS_SUCCESS:
      NetASCII_IPv4_to_Str(addr, addr_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      NetASCII_IPv4_to_Str(mask, mask_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      NetASCII_IPv4_to_Str(gateway, gateway_string, DEF_NO, &local_err);
      APP_RTOS_ASSERT_CRITICAL((local_err.Code == RTOS_ERR_NONE),; );

      EX_TRACE("DHCP client process succeeded.\n");
      EX_TRACE("IP address configured : %s .\n", addr_string);
      EX_TRACE("IP address mask       : %s .\n", mask_string);
      EX_TRACE("IP address gateway    : %s .\n", gateway_string);
      break;

    case DHCPc_STATUS_FAIL_ADDR_USED:
      EX_TRACE("DHCP client process failed: Address already used on network.\n");
      break;

    case DHCPc_STATUS_FAIL_OFFER_DECLINE:
      EX_TRACE("DHCP client process failed: DHCP client declined the offer.\n");
      break;

    case DHCPc_STATUS_FAIL_NAK_RX:
      EX_TRACE("DHCP client process failed: The DHCP server refused the client.\n");
      break;

    case DHCPc_STATUS_FAIL_NO_SERVER:
      EX_TRACE("DHCP client process failed: No DHCP server was detected on the network.\n");
      break;

    case DHCPc_STATUS_FAIL_ERR_FAULT:
#if ((RTOS_ERR_CFG_EXT_EN == DEF_ENABLED) \
      && (RTOS_ERR_CFG_STR_EN == DEF_ENABLED))
      EX_TRACE("DHCP client process failed: Fatal error %s.\n", err.CodeText);
#else
      EX_TRACE("DHCP client process failed: Fatal error %i.\n", err.Code);
#endif
      break;

    default:
      EX_TRACE("DHCP client invalid result.\n");
      return;
  }

#if ((NET_IPv4_CFG_EN            == DEF_ENABLED) \
  && (NET_IPv4_CFG_LINK_LOCAL_EN == DEF_ENABLED))
  if (status != DHCPc_STATUS_SUCCESS) {
    NetIPv4_AddrLinkLocalCfg(if_nbr, Ex_IPv4_LinkLocalAddrCfgResult, &local_err);
  }
#endif
}
#endif

static void  Ex_MacAddrGet(CPU_CHAR * addr_mac_str)
{
    CPU_INT08U         addr_mac[NET_IF_802x_HW_ADDR_LEN];
    RTOS_ERR           err;

    if (DEVINFO->EUI48L != 0xFFFFFFFF) {
      addr_mac[0] = DEVINFO->EUI48H >> 8;
      addr_mac[1] = DEVINFO->EUI48H >> 0;
      addr_mac[2] = DEVINFO->EUI48L >> 24;
      addr_mac[3] = DEVINFO->EUI48L >> 16;
      addr_mac[4] = DEVINFO->EUI48L >> 8;
      addr_mac[5] = DEVINFO->EUI48L >> 0;
    } else {
      addr_mac[0] = DEVINFO->UNIQUEH >> 24;
      addr_mac[1] = DEVINFO->UNIQUEH >> 16;
      addr_mac[2] = DEVINFO->UNIQUEH >> 8;
      addr_mac[3] = DEVINFO->UNIQUEL >> 16;
      addr_mac[4] = DEVINFO->UNIQUEL >> 8;
      addr_mac[5] = DEVINFO->UNIQUEL >> 0;
    }

    NetASCII_MAC_to_Str(addr_mac, addr_mac_str, DEF_FALSE, DEF_TRUE, &err);
}
