/*
*********************************************************************************************************
*                                            EXAMPLE CODE
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
*                                         BSP CONFIGURATION
*
*                                      CONFIGURATION TEMPLATE FILE
*
* Filename      : bsp_cfg.h
* Programmer(s) : Micrium
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*********************************************************************************************************
*                                               MODULE
*********************************************************************************************************
*********************************************************************************************************
*/

#ifndef  _BSP_CFG_H_
#define  _BSP_CFG_H_


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <bsp_opt_def.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                     CLOCK CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/


#define  BSP_HF_CLK_SEL                                     BSP_HF_CLK_HFXO

#define  BSP_LF_CLK_SEL                                     BSP_LF_CLK_LFXO


/*
*********************************************************************************************************
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif                                                          /* End of bsp_cfg.h module include.                  */
