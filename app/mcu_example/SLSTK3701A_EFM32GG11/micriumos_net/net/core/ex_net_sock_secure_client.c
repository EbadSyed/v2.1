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
*                                        NETWORK CORE EXAMPLE
*                                           TLS/SSL CLIENT

*
* File : ex_net_sock_secure.c
* Note(s) : (1) This example show how to connect a client to a server using TLS/SSL
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#include <rtos_description.h>

#ifdef  RTOS_MODULE_NET_SSL_TLS_AVAIL


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/net/include/net_secure_mocana.h>
#include  <rtos/net/include/net_sock.h>
#include  <rtos/net/include/net_app.h>
#include  <rtos/net/include/net_ascii.h>
#include  <rtos/net/include/net_util.h>

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/lib_str.h>
#include  <rtos/common/include/lib_mem.h>
#include  <rtos/common/include/rtos_err.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

                                                            /* CA certificate:                                          */
                                                            /*      (1) Can be obtained on CA website.                  */
                                                            /*      (2) Can be generated using a tool such as OpenSSL.  */
CPU_CHAR *p_cert =
                "MIIDVDCCAjygAwIBAgIDAjRWMA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT"
                "MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i"
                "YWwgQ0EwHhcNMDIwNTIxMDQwMDAwWhcNMjIwNTIxMDQwMDAwWjBCMQswCQYDVQQG"
                "EwJVUzEWMBQGA1UEChMNR2VvVHJ1c3QgSW5jLjEbMBkGA1UEAxMSR2VvVHJ1c3Qg"
                "R2xvYmFsIENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA2swYYzD9"
                "9BcjGlZ+W988bDjkcbd4kdS8odhM+KhDtgPpTSEHCIjaWC9mOSm9BXiLnTjoBbdq"
                "fnGk5sRgprDvgOSJKA+eJdbtg/OtppHHmMlCGDUUna2YRpIuT8rxh0PBFpVXLVDv"
                "iS2Aelet8u5fa9IAjbkU+BQVNdnARqN7csiRv8lVK83Qlz6cJmTM386DGXHKTubU"
                "1XupGc1V3sjs0l44U+VcT4wt/lAjNvxm5suOpDkZALeVAjmRCw7+OC7RHQWa9k0+"
                "bw8HHa8sHo9gOeL6NlMTOdReJivbPagUvTLrGAMoUgRx5aszPeE4uwc2hGKceeoW"
                "MPRfwCvocWvk+QIDAQABo1MwUTAPBgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTA"
                "ephojYn7qwVkDBF9qn1luMrMTjAfBgNVHSMEGDAWgBTAephojYn7qwVkDBF9qn1l"
                "uMrMTjANBgkqhkiG9w0BAQUFAAOCAQEANeMpauUvXVSOKVCUn5kaFOSPeCpilKIn"
                "Z57QzxpeR+nBsqTP3UEaBU6bS+5Kb1VSsyShNwrrZHYqLizz/Tt1kL/6cdjHPTfS"
                "tQWVYrmm3ok9Nns4d0iXrKYgjy6myQzCsplFAMfOEVEiIuCl6rYVSAlk6l5PdPcF"
                "PseKUgzbFbS9bZvlxrFUaKnjaZC2mqUPuLk/IH2uSrW4nOQdtqvmlKXBx4Ot2/Un"
                "hw4EbNX/3aBd7YdStysVAq45pmp06drE57xNNB6pXE0zX5IJL4hmXXeXxx12E6nV"
                "5fEWCRE11azbJHFwLJhWC9kXtNHjUStedejV0NxPNO3CBWaAocvmMw==";


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  CPU_BOOLEAN  Ex_ClientCertTrustCallBackFnct (void                             *p_cert_dn,
                                                     NET_SOCK_SECURE_UNTRUSTED_REASON  reason);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                    Ex_Net_SockSecureClientConnect()
*
* Description : (1) Initialize a client secure socket:
*
*                   (a) Install CA certificate.
*                   (b) Open a TCP socket.
*                   (c) Configure socket's option to be secure.
*                   (d) Connect the socket, establish a secure connection with the server.
*
* Argument(s) : none.
*
* Return(s)   : Socket ID, if successfully connected.
*               -1,        Otherwise.
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_INT16S  Ex_Net_SockSecureClientConnect (void)
{
    NET_SOCK_ID         sock_id;
    NET_SOCK_ADDR_IPv4  addr_server;
    CPU_INT32U          len_addr_server;
    CPU_INT32U          len;
    RTOS_ERR            err;


                                                                /* -------------- INSTALL CA CERTIFICATE -------------- */
    len = Str_Len(p_cert);
    NetSecure_CA_CertIntall(p_cert, len, NET_SOCK_SECURE_CERT_KEY_FMT_PEM, &err);


                                                                /* ------------------ OPEN THE SOCKET ----------------- */
    sock_id = NetApp_SockOpen(NET_SOCK_ADDR_FAMILY_IP_V4,
                              NET_SOCK_TYPE_STREAM,
                              NET_SOCK_PROTOCOL_TCP,
                              3,
                              5,
                              &err);
    if (err.Code != RTOS_ERR_NONE) {
        return (-1);
    }

                                                                /* ------------ CONFIGURE SOCKET AS SECURE ------------ */
   (void)NetSock_CfgSecure(sock_id,                             /* First the socket option secure must be set.          */
                           DEF_YES,
                          &err);
   if (err.Code != RTOS_ERR_NONE) {
       NetApp_SockClose(sock_id, 1, &err);
       return (-1);
   }


  (void)NetSock_CfgSecureClientCommonName(sock_id,              /* Configure the common name of the server ...          */
                                          "domain_name.com",    /* certificate, most of the time it is the Domain name. */
                                          &err);
   if (err.Code != RTOS_ERR_NONE) {
       NetApp_SockClose(sock_id, 1, &err);
       return (-1);
   }

                                                                /* Configure the callback function to call if the ...   */
                                                                /* ... server certificate is not trusted. So the  ...   */
                                                                /* ... connection can be allow even if the        ...   */
                                                                /* ... certificate is not trusted.                      */
   (void)NetSock_CfgSecureClientTrustCallBack(sock_id,
                                             &Ex_ClientCertTrustCallBackFnct,
                                             &err);
    if (err.Code != RTOS_ERR_NONE) {
        NetApp_SockClose(sock_id, 1, &err);
        return (-1);
    }



                                                                /* ------------- ESTABLISH TCP CONNECTION ------------- */
    Mem_Clr((void *)&addr_server, NET_SOCK_ADDR_SIZE);


    addr_server.AddrFamily = NetASCII_Str_to_IP((CPU_CHAR  *)"98.139.211.125",
                                                (void      *)addr_server.Addr,
                                                (CPU_INT08U )sizeof(addr_server.Addr),
                                                            &err);
    addr_server.Port       = NET_UTIL_HOST_TO_NET_16(12345);
    len_addr_server        = sizeof(addr_server);

    (void)NetApp_SockConn(                  sock_id,            /* Connect to server using TLS/SSL.                     */
                          (NET_SOCK_ADDR *)&addr_server,
                                            len_addr_server,
                                            3,
                                            5,
                                            5,
                                           &err);
    if (err.Code != RTOS_ERR_NONE) {
        NetApp_SockClose(sock_id, 1, &err);
        return (-1);
    }

    /* Now all the data transfered on this socket is encrypted.                 */
    /* You just have to use any socket API such as NetApp_Rx() or NetApp_Tx().  */

    return (sock_id);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                   Ex_ClientCertTrustCallBackFnct()
*
* Description : Function called when the server's certificate is not trusted.
*
* Argument(s) : p_cert_dn   Pointer to certificate distinguish name.
*
*               reason      Reason why the certificate is not trusted:
*
*                               NET_SOCK_SECURE_UNTRUSTED_BY_CA
*                               NET_SOCK_SECURE_EXPIRE_DATE
*                               NET_SOCK_SECURE_INVALID_DATE
*                               NET_SOCK_SECURE_SELF_SIGNED
*                               NET_SOCK_SECURE_UNKNOWN
*
*
* Return(s)   : DEF_OK,   The connection can be established even if the certificated is not trusted.
*               DEF_FAIL, Connection refused.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  CPU_BOOLEAN  Ex_ClientCertTrustCallBackFnct (void                             *p_cert_dn,
                                                     NET_SOCK_SECURE_UNTRUSTED_REASON  reason)
{
    return (DEF_OK);
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_NET_SSL_TLS_AVAIL */

