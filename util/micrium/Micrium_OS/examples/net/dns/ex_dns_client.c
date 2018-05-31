/*
*********************************************************************************************************
*                                             EXAMPLE CODE
*********************************************************************************************************
* Licensing:
*   The licensor of this EXAMPLE CODE is Silicon Laboratories Inc.
*
*   Silicon Laboratories Inc. grants you a personal, worldwide, royalty-free, fully paid-up license to
*   use, copy, modify and distribute the EXAMPLE CODE software, or portions thereof, in any of your
*   products.
*
*   Your use of this EXAMPLE CODE is at your own risk. This EXAMPLE CODE does not come with any
*   warranties, and the licensor disclaims all implied warranties concerning performance, accuracy,
*   non-infringement, merchantability and fitness for your application.
*
*   The EXAMPLE CODE is provided "AS IS" and does not come with any support.
*
*   You can find user manuals, API references, release notes and more at: https://doc.micrium.com
*
*   You can contact us at: https://www.micrium.com
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                          NETWORK CORE EXAMPLE
*                                               DNS CLIENT
*
* File : ex_dns_client.c
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos_description.h>

#if (defined(RTOS_MODULE_NET_AVAIL))

#include  <net_cfg.h>

#if (NET_DNS_CLIENT_CFG_MODULE_EN == DEF_ENABLED)


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/net/include/net_ip.h>
#include  <rtos/net/include/net_type.h>
#include  <rtos/net/include/dns_client.h>
#include  <rtos/net/include/net_ascii.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/rtos_err.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                       Ex_DNSc_GetHostname()
*
* Description : This function resolve the provided hostname and returns the IP address of the host.
*
* Argument(s) : p_hostname  Pointer to a string that contains the hostname to be resolved.
*
*               p_addr_str  Pointer to a string that will receive the IP address.
*
* Return(s)   : None.
*
* Note(s)     : (1) Prior to do any call to DNSc the module must be initialized via the Net_Init()
*                   function.
*********************************************************************************************************
*/

void  Ex_DNSc_GetHostname (CPU_CHAR  *p_hostname,
                           CPU_CHAR  *p_addr_str)
{
    NET_IP_ADDR_OBJ  addrs[2u];
    CPU_INT08U       addr_nbr = 2u;
    CPU_INT08U       ix;
    RTOS_ERR         err;


    DNSc_GetHost(p_hostname, addrs, &addr_nbr, DNSc_FLAG_NONE, DEF_NULL, &err);
    if (err.Code != RTOS_ERR_NONE) {
        return;
    }

    for (ix = 0u; ix < addr_nbr; ix++) {

        if (addrs[ix].AddrLen == NET_IPv4_ADDR_LEN) {
#if (NET_IPv4_CFG_EN == DEF_ENABLED)
            NET_IPv4_ADDR *p_addr = (NET_IPv4_ADDR *)&addrs[ix].Addr.Array[0];


            NetASCII_IPv4_to_Str(*p_addr, p_addr_str, NET_ASCII_LEN_MAX_ADDR_IP, &err);
#endif
        } else {
#if (NET_IPv6_CFG_EN == DEF_ENABLED)
            NET_IPv6_ADDR *p_addr = (NET_IPv6_ADDR *)&addrs[ix].Addr.Array[0];


            NetASCII_IPv6_to_Str(p_addr, p_addr_str, DEF_NO, DEF_YES, &err);
#endif
        }
    }
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* NET_DNS_CLIENT_CFG_MODULE_EN */
#endif  /* RTOS_MODULE_NET_AVAIL        */
