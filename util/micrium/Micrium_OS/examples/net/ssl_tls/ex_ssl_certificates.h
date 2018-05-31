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
*                       SAMPLE CERTIFICATES AND PRIVATE KEYS FOR SSL/TLS PORTS
*
* File : ex_ssl_certificates.h
*********************************************************************************************************
* Note(s) : (1) The way to format PEM Certificates & Keys and their lenghts may vary from one port to
*               another. This file serves the purpose of abstracting this.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                               DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/

#if 0 /* Define one */
#define  POLARSSL
#define  CUSTOM
#define  VALICORE
#define  GEOTRUST
#endif

#define  POLARSSL

#ifdef POLARSSL
#define  SSL_CA_CERT          SSL_CA_Cert_PolarSSL
#define  SSL_SERVER_CERT      SSL_ServerCert_PolarSSL
#define  SSL_SERVER_KEY       SSL_ServerKey_PolarSSL
#define  SSL_CLIENT_CERT      SSL_ClientCert_PolarSSL
#define  SSL_CLIENT_KEY       SSL_ClientKey_PolarSSL
#endif

#ifdef CUSTOM
#define  SSL_CA_CERT          SSL_CA_Cert_Symantec
#define  SSL_SERVER_CERT      SSL_ServerCert_Custom
#define  SSL_SERVER_KEY       SSL_ServerKey_Custom
#define  SSL_CLIENT_CERT      SSL_ClientCert_Custom
#define  SSL_CLIENT_KEY       SSL_ClientKey_Custom
#endif

extern  const  CPU_CHAR       SSL_CA_Cert_PolarSSL[];
extern  const  CPU_CHAR       SSL_ServerCert_PolarSSL[];
extern  const  CPU_CHAR       SSL_ServerKey_PolarSSL[];
extern  const  CPU_CHAR       SSL_ClientCert_PolarSSL[];
extern  const  CPU_CHAR       SSL_ClientKey_PolarSSL[];

extern  const  CPU_CHAR       SSL_CA_Cert_Symantec[];           /* From Amazon                                          */
extern  const  CPU_CHAR       SSL_ServerCert_Custom[];
extern  const  CPU_CHAR       SSL_ServerKey_Custom[];
extern  const  CPU_CHAR       SSL_ClientCert_Custom[];
extern  const  CPU_CHAR       SSL_ClientKey_Custom[];

extern  const  CPU_CHAR       SSL_CA_Cert_GeoTrust[];
extern  const  CPU_CHAR       SSL_CA_Cert_GlobalSign[];
extern  const  CPU_CHAR       SSL_CA_Cert_DST[];
extern  const  CPU_CHAR       SSL_CA_Cert_GoDaddy[];
extern  const  CPU_CHAR       SSL_CA_Cert_Thawte[];
extern  const  CPU_CHAR       SSL_CA_Cert_Equifax[];

extern  const  CPU_INT32U     SSL_CA_Cert_Len;
extern  const  CPU_INT32U     SSL_ServerCertLen;
extern  const  CPU_INT32U     SSL_ServerKeyLen;
extern  const  CPU_INT32U     SSL_ClientCertLen;
extern  const  CPU_INT32U     SSL_ClientKeyLen;
