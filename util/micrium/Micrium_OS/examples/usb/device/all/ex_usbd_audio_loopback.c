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
*                                         USB DEVICE EXAMPLE
*
*                                           USB Audio class
*
*                                              Loopback
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                     DEPENDENCIES & AVAIL CHECK(S)
*********************************************************************************************************
*********************************************************************************************************
*/

#include <rtos_description.h>

#if (defined(RTOS_MODULE_USB_DEV_AUDIO_AVAIL))


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*********************************************************************************************************
*/

#include  <rtos/cpu/include/cpu.h>
#include  <rtos/common/include/rtos_utils.h>
#include  <rtos/common/include/rtos_err.h>
#include  <rtos/common/include/lib_ascii.h>

#include  <rtos/kernel/include/os.h>

#include  <rtos/usb/include/device/usbd_core.h>

#include  <rtos/usb/include/device/usbd_audio.h>
#include  "ex_usbd_audio_drv_simulation.h"

#include  <ex_description.h>


/*
*********************************************************************************************************
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*********************************************************************************************************
*/

static  void  Ex_USBD_Audio_LoopConn   (CPU_INT08U            dev_nbr,
                                        CPU_INT08U            cfg_nbr,
                                        CPU_INT08U            terminal_id,
                                        USBD_AUDIO_AS_HANDLE  as_handle);

static  void  Ex_USBD_Audio_LoopDisconn(CPU_INT08U            dev_nbr,
                                        CPU_INT08U            cfg_nbr,
                                        CPU_INT08U            terminal_id,
                                        USBD_AUDIO_AS_HANDLE  as_handle);


/*
*********************************************************************************************************
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*********************************************************************************************************
*/

const  USBD_AUDIO_EVENT_FNCTS Ex_USBD_Audio_LoopEventFncts = {
    Ex_USBD_Audio_LoopConn,
    Ex_USBD_Audio_LoopDisconn
};


/*
*********************************************************************************************************
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*********************************************************************************************************
*/

extern  CPU_INT08U  Ex_USBD_AudioMic_IT_ID;
extern  CPU_INT08U  Ex_USBD_AudioMic_OT_USB_IN_ID;
extern  CPU_INT08U  Ex_USBD_AudioMic_FU_ID;
extern  CPU_INT08U  Ex_USBD_AudioSpeaker_IT_USB_OUT_ID;
extern  CPU_INT08U  Ex_USBD_AudioSpeaker_OT_ID;
extern  CPU_INT08U  Ex_USBD_AudioSpeaker_FU_ID;


/*
*********************************************************************************************************
*********************************************************************************************************
*                                        CONFIGURATION ERRORS
*********************************************************************************************************
*********************************************************************************************************
*/

#if    ((USBD_AUDIO_CFG_PLAYBACK_EN != DEF_ENABLED) || \
        (USBD_AUDIO_CFG_RECORD_EN   != DEF_ENABLED))
#error  "USBD_AUDIO_CFG_PLAYBACK_EN and USBD_AUDIO_CFG_RECORD_EN MUST be set to DEF_ENABLED for the audio loopback demo."
#endif


/*
*********************************************************************************************************
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        Ex_USBD_Audio_Init()
*
* Description : Adds a vendor interface to the device and creates a task to handle the loopback
*               functionnalities.
*
* Argument(s) : dev_nbr         Device number.
*
*               config_nbr_fs   Full-Speed configuration number.
*
*               config_nbr_hs   High-Speed configuration number.
*
* Return(s)   : None.
*
* Note(s)     : (1) This function builds a simple audio topology that emulates a headset. However,
*                   the microphone is fed by the speaker output. See below.
*
*                   (a) Speaker:
*
*                   +-----------+       +-----------+       +-----------+
*                   |  Input    |       |  Feature  |       |  Output   |
*                   | Terminal  | ----> |   Unit    | ----> | Terminal  |
*                   |           |       | Mute/Vol  |       |           |
*                   +-----------+       +-----------+       +-----------+
*                         ^                                       |
*                         |                                       |
*                Fed by USB streaming                    Normally connected to
*                    from host.                          speaker. In this case,
*                                                        used to feed microhpone.
*
*                   (b) Microphone
*
*                   +-----------+       +-----------+       +-----------+
*                   |  Input    |       |  Feature  |       |  Output   |
*                   | Terminal  | ----> |   Unit    | ----> | Terminal  |
*                   |           |       | Mute/Vol  |       |           |
*                   +-----------+       +-----------+       +-----------+
*                         ^                                       |
*                         |                                       |
*               Normally fed by                           Bound to an audio
*               microphone. In this case,                streaming interface
*               fed by speaker.                           that outputs data
*                                                           through USB.
*********************************************************************************************************
*/

void  Ex_USBD_Audio_Init (CPU_INT08U  dev_nbr,
                          CPU_INT08U  config_nbr_fs,
                          CPU_INT08U  config_nbr_hs)
{
    CPU_INT08U                audio_nbr;
    RTOS_ERR                  err;
    USBD_AUDIO_AS_IF_HANDLE   mic_record_as_if_handle;
    USBD_AUDIO_AS_IF_HANDLE   speaker_playback_as_if_handle;
    CPU_INT32U                as_sam_freq_tbl[2u];
    CPU_INT16U                fu_log_ch_ctrl_tbl[3u];
    USBD_AUDIO_QTY_CFG        audio_qty_cfg;
    USBD_AUDIO_OT_CFG         ot_cfg;
    USBD_AUDIO_STREAM_CFG     stream_cfg;
    USBD_AUDIO_IT_CFG         it_cfg;
    USBD_AUDIO_FU_CFG         fu_cfg;
    USBD_AUDIO_AS_IF_CFG      as_if_cfg;
    USBD_AUDIO_AS_ALT_CFG     as_if_alt_cfg;
    USBD_AUDIO_AS_ALT_CFG    *p_as_if_alt_cfg_tbl[1u];


                                                                /* ----------------- INIT AUDIO CLASS ----------------- */
    audio_qty_cfg.ClassInstanceQty  = 1u;
    audio_qty_cfg.ConfigQty         = 2u;
    audio_qty_cfg.IT_Qty            = 2u;
    audio_qty_cfg.OT_Qty            = 2u;
    audio_qty_cfg.FU_Qty            = 2u;
    audio_qty_cfg.MU_Qty            = 1u;
    audio_qty_cfg.SU_Qty            = 1u;
    audio_qty_cfg.IF_AltQty         = 2u;
    audio_qty_cfg.AS_IF_PlaybackQty = 1u;
    audio_qty_cfg.AS_IF_RecordQty   = 1u;

    USBD_Audio_Init(&audio_qty_cfg, &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* ---------- CREATE AN AUDIO CLASS INSTANCE ---------- */
    audio_nbr = USBD_Audio_Add(DEF_NULL,
                               6u,
                              &Ex_USBD_Audio_LoopDrvCommonAPI_Simulation,
                              &Ex_USBD_Audio_LoopEventFncts,
                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* --- ADD AUDIO CLASS INSTANCE TO CONFIGURATION(S) --- */
    USBD_Audio_ConfigAdd(audio_nbr,
                         dev_nbr,
                         config_nbr_fs,
                        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    if (config_nbr_hs != USBD_CONFIG_NBR_NONE) {
        USBD_Audio_ConfigAdd(audio_nbr,
                             dev_nbr,
                             config_nbr_hs,
                            &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    }

                                                                /* ---------- CREATE AUDIO FUNCTION ENTITIES ---------- */
                                                                /* -------------------- MICROPHONE -------------------- */
                                                                /* Add input terminal.                                  */
    it_cfg.TerminalType  =  USBD_AUDIO_TERMINAL_TYPE_MIC;
    it_cfg.LogChNbr      =  USBD_AUDIO_MONO;
    it_cfg.LogChCfg      =  USBD_AUDIO_SPATIAL_LOCATION_LEFT_FRONT;
    it_cfg.CopyProtEn    =  DEF_DISABLED;
    it_cfg.CopyProtLevel =  USBD_AUDIO_CPL_NONE;
    it_cfg.StrPtr        = "IT Microphone";

    Ex_USBD_AudioMic_IT_ID = USBD_Audio_IT_Add(audio_nbr,
                                              &it_cfg,
                                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Add output terminal.                                 */
    ot_cfg.TerminalType =  USBD_AUDIO_TERMINAL_TYPE_USB_STREAMING;
    ot_cfg.CopyProtEn   =  DEF_DISABLED;
    ot_cfg.StrPtr       = "OT USB IN";

    Ex_USBD_AudioMic_OT_USB_IN_ID = USBD_Audio_OT_Add(audio_nbr,
                                                     &ot_cfg,
                                                      DEF_NULL,
                                                     &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Add feature unit for mute and volume ctrl.           */
    fu_log_ch_ctrl_tbl[0u] = (USBD_AUDIO_FU_CTRL_MUTE |         /* Master ch.                                           */
                              USBD_AUDIO_FU_CTRL_VOL);
    fu_log_ch_ctrl_tbl[1u] =  USBD_AUDIO_FU_CTRL_NONE;          /* Log ch #1.                                           */
    fu_log_ch_ctrl_tbl[2u] =  USBD_AUDIO_FU_CTRL_NONE;          /* Log ch #2.                                           */

    fu_cfg.LogChNbr     =  USBD_AUDIO_MONO;
    fu_cfg.LogChCtrlTbl =  fu_log_ch_ctrl_tbl;
    fu_cfg.StrPtr       = "FU Microphone";
    Ex_USBD_AudioMic_FU_ID = USBD_Audio_FU_Add(audio_nbr,
                                              &fu_cfg,
                                              &Ex_USBD_Audio_DrvFU_API_Simulation,
                                              &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* --------------------- SPEAKER ---------------------- */
                                                                /* Add input terminal.                                  */
    it_cfg.TerminalType  =  USBD_AUDIO_TERMINAL_TYPE_USB_STREAMING;
    it_cfg.LogChNbr      =  USBD_AUDIO_MONO;
    it_cfg.LogChCfg      =  USBD_AUDIO_SPATIAL_LOCATION_LEFT_FRONT;
    it_cfg.CopyProtEn    =  DEF_DISABLED;
    it_cfg.CopyProtLevel =  USBD_AUDIO_CPL_NONE;
    it_cfg.StrPtr        = "IT USB OUT";

    Ex_USBD_AudioSpeaker_IT_USB_OUT_ID = USBD_Audio_IT_Add(audio_nbr,
                                                          &it_cfg,
                                                          &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Add output terminal.                                 */
    ot_cfg.TerminalType =  USBD_AUDIO_TERMINAL_TYPE_SPEAKER;
    ot_cfg.CopyProtEn   =  DEF_DISABLED;
    ot_cfg.StrPtr       = "OT Speaker";

    Ex_USBD_AudioSpeaker_OT_ID = USBD_Audio_OT_Add(audio_nbr,
                                                  &ot_cfg,
                                                   DEF_NULL,
                                                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Add feature unit for mute and volume ctrl.           */
    fu_log_ch_ctrl_tbl[0u] = (USBD_AUDIO_FU_CTRL_MUTE |         /* Master ch.                                           */
                              USBD_AUDIO_FU_CTRL_VOL);
    fu_log_ch_ctrl_tbl[1u] =  USBD_AUDIO_FU_CTRL_NONE;          /* Log ch #1.                                           */
    fu_log_ch_ctrl_tbl[2u] =  USBD_AUDIO_FU_CTRL_NONE;          /* Log ch #2.                                           */

    fu_cfg.LogChNbr     =  USBD_AUDIO_MONO;
    fu_cfg.LogChCtrlTbl =  fu_log_ch_ctrl_tbl;
    fu_cfg.StrPtr       = "FU Speaker";
    Ex_USBD_AudioSpeaker_FU_ID = USBD_Audio_FU_Add(audio_nbr,
                                                  &fu_cfg,
                                                  &Ex_USBD_Audio_DrvFU_API_Simulation,
                                                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* ------------- BIND TERMINALS AND UNITS ------------- */
                                                                /* -------------------- MICROPHONE -------------------- */
                                                                /* Input terminal has no association.                   */
    USBD_Audio_IT_Assoc(audio_nbr,
                        Ex_USBD_AudioMic_IT_ID,
                        USBD_AUDIO_TERMINAL_NO_ASSOCIATION,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Associate output terminal to feature unit.           */
    USBD_Audio_OT_Assoc(audio_nbr,
                        Ex_USBD_AudioMic_OT_USB_IN_ID,
                        Ex_USBD_AudioMic_FU_ID,
                        USBD_AUDIO_TERMINAL_NO_ASSOCIATION,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Associate feature unit to input terminal.            */
    USBD_Audio_FU_Assoc(audio_nbr,
                        Ex_USBD_AudioMic_FU_ID,
                        Ex_USBD_AudioMic_IT_ID,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* --------------------- SPEAKER ---------------------- */
                                                                /* Input terminal has no association.                   */
    USBD_Audio_IT_Assoc(audio_nbr,
                        Ex_USBD_AudioSpeaker_IT_USB_OUT_ID,
                        USBD_AUDIO_TERMINAL_NO_ASSOCIATION,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Associate output terminal to feature unit.           */
    USBD_Audio_OT_Assoc(audio_nbr,
                        Ex_USBD_AudioSpeaker_OT_ID,
                        Ex_USBD_AudioSpeaker_FU_ID,
                        USBD_AUDIO_TERMINAL_NO_ASSOCIATION,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Associate feature unit to input terminal.            */
    USBD_Audio_FU_Assoc(audio_nbr,
                        Ex_USBD_AudioSpeaker_FU_ID,
                        Ex_USBD_AudioSpeaker_IT_USB_OUT_ID,
                       &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);


                                                                /* --------- CREATE AUDIO STREAMING INTERFACE --------- */
                                                                /* -------------------- MICROPHONE -------------------- */
                                                                /* Stream configurations.                               */
    stream_cfg.MaxBufNbr    = 12u;
    stream_cfg.CorrPeriodMs = 4u;

                                                                /* Create one operationnal alternate setting.           */
    as_sam_freq_tbl[0u] = USBD_AUDIO_FMT_TYPE_I_SAMFREQ_48KHZ;

    as_if_alt_cfg.Dly                = USBD_AUDIO_DATA_DLY_1_MS;
    as_if_alt_cfg.FmtTag             = USBD_AUDIO_DATA_FMT_TYPE_I_PCM;
    as_if_alt_cfg.NbrCh              = USBD_AUDIO_MONO;
    as_if_alt_cfg.SubframeSize       = USBD_AUDIO_FMT_TYPE_I_SUBFRAME_SIZE_2;
    as_if_alt_cfg.BitRes             = USBD_AUDIO_FMT_TYPE_I_BIT_RESOLUTION_16;
    as_if_alt_cfg.NbrSamplingFreq    = 1u;                      /* Use discrete sampling freq tbl.                      */
    as_if_alt_cfg.LowerSamplingFreq  = USBD_AUDIO_FMT_TYPE_I_SAMFREQ_NONE;
    as_if_alt_cfg.UpperSamplingFreq  = USBD_AUDIO_FMT_TYPE_I_SAMFREQ_NONE;
    as_if_alt_cfg.SamplingFreqTblPtr = as_sam_freq_tbl;
    as_if_alt_cfg.EP_DirIn           = DEF_YES;
    as_if_alt_cfg.EP_SynchType       = USBD_EP_TYPE_SYNC_ASYNC;
    as_if_alt_cfg.EP_Attrib          = USBD_AUDIO_AS_EP_CTRL_SAMPLING_FREQ;
    as_if_alt_cfg.EP_LockDlyUnits    = USBD_AUDIO_AS_EP_LOCK_DLY_UND;
    as_if_alt_cfg.EP_LockDly         = 0u;
    as_if_alt_cfg.EP_SynchRefresh    = 0u;

    p_as_if_alt_cfg_tbl[0u]          = &as_if_alt_cfg;
    as_if_cfg.AS_CfgPtrTbl           =  p_as_if_alt_cfg_tbl;
    as_if_cfg.AS_CfgAltSettingNbr    =  1u;

    mic_record_as_if_handle = USBD_Audio_AS_IF_Cfg(DEF_NULL,
                                                  &stream_cfg,
                                                  &as_if_cfg,
                                                  &Ex_USBD_Audio_MicDrvAS_API_Simulation,
                                                   Ex_USBD_AudioMic_OT_USB_IN_ID,
                                                   DEF_NULL,
                                                  &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                    /* Add AS IF to configuration(s).                       */
    USBD_Audio_AS_IF_Add(audio_nbr,
                         config_nbr_fs,
                         mic_record_as_if_handle,
                        &as_if_cfg,
                        "Microphone AudioStreaming IF",
                        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    if (config_nbr_hs != USBD_CONFIG_NBR_NONE) {
        USBD_Audio_AS_IF_Add(audio_nbr,
                             config_nbr_hs,
                             mic_record_as_if_handle,
                            &as_if_cfg,
                            "Microphone AudioStreaming IF",
                            &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    }


                                                                /* --------------------- SPEAKER ---------------------- */
    stream_cfg.MaxBufNbr    = 12u;
    stream_cfg.CorrPeriodMs = 4u;

                                                                /* Create one operationnal alternate setting.           */
    as_sam_freq_tbl[0u] = USBD_AUDIO_FMT_TYPE_I_SAMFREQ_48KHZ;

    as_if_alt_cfg.Dly                = 1u;
    as_if_alt_cfg.FmtTag             = USBD_AUDIO_DATA_FMT_TYPE_I_PCM;
    as_if_alt_cfg.NbrCh              = USBD_AUDIO_MONO;
    as_if_alt_cfg.SubframeSize       = USBD_AUDIO_FMT_TYPE_I_SUBFRAME_SIZE_2;
    as_if_alt_cfg.BitRes             = USBD_AUDIO_FMT_TYPE_I_BIT_RESOLUTION_16;
    as_if_alt_cfg.NbrSamplingFreq    = 1u;                      /* Use discrete sampling freq tbl.                      */
    as_if_alt_cfg.LowerSamplingFreq  = 0u;
    as_if_alt_cfg.UpperSamplingFreq  = 0u;
    as_if_alt_cfg.SamplingFreqTblPtr = as_sam_freq_tbl;
    as_if_alt_cfg.EP_DirIn           = DEF_NO;
    as_if_alt_cfg.EP_SynchType       = USBD_EP_TYPE_SYNC_ASYNC;
    as_if_alt_cfg.EP_Attrib          = USBD_AUDIO_AS_EP_CTRL_SAMPLING_FREQ;
    as_if_alt_cfg.EP_LockDlyUnits    = USBD_AUDIO_AS_EP_LOCK_DLY_UND;
    as_if_alt_cfg.EP_LockDly         = 0u;
    as_if_alt_cfg.EP_SynchRefresh    = 2u;

    p_as_if_alt_cfg_tbl[0u]       = &as_if_alt_cfg;
    as_if_cfg.AS_CfgPtrTbl        =  p_as_if_alt_cfg_tbl;
    as_if_cfg.AS_CfgAltSettingNbr =  1u;

    speaker_playback_as_if_handle =  USBD_Audio_AS_IF_Cfg(DEF_NULL,
                                                         &stream_cfg,
                                                         &as_if_cfg,
                                                         &Ex_USBD_Audio_SpeakerDrvAS_API_Simulation,
                                                          Ex_USBD_AudioSpeaker_IT_USB_OUT_ID,
                                                          DEF_NULL,
                                                         &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

                                                                /* Add AS IF to configuration(s).                       */
    USBD_Audio_AS_IF_Add(audio_nbr,
                         config_nbr_fs,
                         speaker_playback_as_if_handle,
                        &as_if_cfg,
                        "Speaker AudioStreaming IF",
                        &err);
    APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);

    if (config_nbr_hs != USBD_CONFIG_NBR_NONE) {
        USBD_Audio_AS_IF_Add(audio_nbr,
                             config_nbr_hs,
                             speaker_playback_as_if_handle,
                            &as_if_cfg,
                            "Speaker AudioStreaming IF",
                            &err);
        APP_RTOS_ASSERT_CRITICAL(err.Code == RTOS_ERR_NONE, ;);
    }
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
*                                         App_USBD_Audio_Conn()
*
* Description : Inform the application about audio device connection to host. The host has completed the
*               device's enumeration and the device is ready for communication.
*
* Argument(s) : dev_nbr         Device number.
*
*               cfg_nbr         Configuration number.
*
*               terminal_id     Terminal ID.
*
*               as_handle       AudioStreaming interface handle.
*
* Return(s)   : None.
*
* Note(s)     : (1) When calling this function, you may call the function USBD_Audio_AS_IF_StatGet() to
*                   get audio statistics for a given AudioStreaming interface.
*                   The audio statistics structure can be consulted during debug operations in a watch
*                   window for instance. This structure collects long-term statistics for a given
*                   AudioStreaming interface, that is each time the corresponding stream is open
*                   and used by the host.
*********************************************************************************************************
*/

static  void  Ex_USBD_Audio_LoopConn (CPU_INT08U           dev_nbr,
                                      CPU_INT08U            cfg_nbr,
                                      CPU_INT08U            terminal_id,
                                      USBD_AUDIO_AS_HANDLE  as_handle)
{
    PP_UNUSED_PARAM(dev_nbr);
    PP_UNUSED_PARAM(cfg_nbr);
    PP_UNUSED_PARAM(terminal_id);
    PP_UNUSED_PARAM(as_handle);

    /* TODO: Make any operation needed when audio devcie is connected.*/
}

/*
*********************************************************************************************************
*                                       App_USBD_Audio_Disconn()
*
* Description : Inform the application about audio device configuration being inactive because of
*               device disconnection from host or host selecting another device configuration.
*
* Argument(s) : dev_nbr         Device number.
*
*               cfg_nbr         Configuration number.
*
*               terminal_id     Terminal ID.
*
*               as_handle       AudioStreaming interface handle.
*
* Return(s)   : None.
*
* Note(s)     : None.
*********************************************************************************************************
*/

static  void  Ex_USBD_Audio_LoopDisconn (CPU_INT08U            dev_nbr,
                                         CPU_INT08U            cfg_nbr,
                                         CPU_INT08U            terminal_id,
                                         USBD_AUDIO_AS_HANDLE  as_handle)
{
    PP_UNUSED_PARAM(dev_nbr);
    PP_UNUSED_PARAM(cfg_nbr);
    PP_UNUSED_PARAM(terminal_id);
    PP_UNUSED_PARAM(as_handle);

    /* TODO: Make any operation needed when audio devcie is disconnected.*/
}


/*
*********************************************************************************************************
*********************************************************************************************************
*                                   DEPENDENCIES & AVAIL CHECK(S) END
*********************************************************************************************************
*********************************************************************************************************
*/

#endif  /* RTOS_MODULE_USB_DEV_AUDIO_AVAIL */
