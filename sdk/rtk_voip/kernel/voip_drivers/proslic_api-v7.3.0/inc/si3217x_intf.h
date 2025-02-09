/*
** Copyright (c) 2007-2013 by Silicon Laboratories
**
** $Id: si3217x_intf.h 4194 2013-12-12 20:49:53Z cdp $
**
** Si3217x_Intf.h
** Si3217x ProSLIC interface header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the ProSLIC driver.
**
** Dependancies:
** proslic_datatypes.h, Si3217x_registers.h, ProSLIC.h
**
*/
#ifndef SI3217X_INTF_H
#define SI3217X_INTF_H



/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_Reset
**
** Description: 
** Resets the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_Reset (proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruption to 
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_ShutdownChannel (proslicChanType_ptr hProslic);

/*
** Function: PROSLIC_Init_MultiBOM
**
** Description: 
** Initializes the ProSLIC w/ selected general parameters
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** size:     number of channels
** preset:   General configuratin preset
**
** Return:
** none
*/
#ifdef SIVOICE_MULTI_BOM_SUPPORT
int Si3217x_Init_MultiBOM (proslicChanType_ptr *hProslic,int size,int preset);
#endif

/*
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_Init (proslicChanType_ptr *hProslic,int size);

/*
** Function: PROSLIC_Reinit
**
** Description: 
** Soft reset and initialization
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_Reinit (proslicChanType_ptr hProslic,int size);
/*
** Function: PROSLIC_VerifyControlInterface
**
** Description: 
** Verify SPI port read capabilities
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_VerifyControlInterface (proslicChanType_ptr hProslic);

uInt8 Si3217x_ReadReg (proslicChanType_ptr hProslic,uInt8 addr);
int Si3217x_WriteReg (proslicChanType_ptr hProslic,uInt8 addr,uInt8 data);
ramData Si3217x_ReadRAM (proslicChanType_ptr hProslic,uInt16 addr);
int Si3217x_WriteRAM (proslicChanType_ptr hProslic,uInt16 addr, ramData data);

/*
** Function: ProSLIC_PrintDebugData
**
** Description: 
** Register and RAM dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_PrintDebugData (proslicChanType_ptr hProslic);

/*
** Function: ProSLIC_PrintDebugReg
**
** Description: 
** Register dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_PrintDebugReg (proslicChanType_ptr hProslic);

/*
** Function: ProSLIC_PrintDebugRAM
**
** Description: 
** RAM dump utility
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_PrintDebugRAM (proslicChanType_ptr hProslic);

/*
** Function: Si3217x_PowerUpConverter
**
** Description: 
** Powers all DC/DC converters sequentially with delay to minimize
** peak power draw on VDC.
**
** Returns:
** int (error)
**
*/
int Si3217x_PowerUpConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3217x_PowerDownConverter
**
** Description: 
** Power down DCDC converter (selected channel only)
**
** Returns:
** int (error)
**
*/
int Si3217x_PowerDownConverter(proslicChanType_ptr hProslic);

/*
** Function: Si3217x_Calibrate
**
** Description: 
** Generic calibration function for Si3217x
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object,
** size:     maximum number of channels
** calr:     array of CALRx register values
** maxTime:  cal timeout (in ms) 
**
** Return:
** int 
*/
int Si3217x_Calibrate (proslicChanType_ptr *hProslic, int size, uInt8 *calr, int maxTime);

/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** none
*/
int Si3217x_Cal (proslicChanType_ptr *hProslic, int size);

/*
** Function: PROSLIC_LoadRegTables
**
** Description: 
** Loads registers and ram in the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pRamTable: pointer to ram values to load
** pRegTable: pointer to register values to load
** 
**
** Return:
** none
*/
int Si3217x_LoadRegTables (proslicChanType_ptr *hProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable,int size);

/*
** Function: PROSLIC_LoadPatch
**
** Description: 
** Loads patch to the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pPatch: pointer to patch data
**
** Return:
** none
*/
int Si3217x_LoadPatch (proslicChanType_ptr hProslic, const proslicPatch *pPatch);

/*
** Function: PROSLIC_VerifyPatch
**
** Description: 
** Verifies patch to the ProSLIC
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** pPatch: pointer to patch data
**
** Return:
** none
*/
int Si3217x_VerifyPatch (proslicChanType_ptr hProslic, const proslicPatch *pPatch);

/*
** Function: PROSLIC_EnableInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** 
** Return:
** 
*/
int Si3217x_EnableInterrupts (proslicChanType_ptr hProslic);
int Si3217x_DisableInterrupts (proslicChanType_ptr hProslic);


/*
** Function: PROSLIC_SetLoopbackMode
**
** Description: 
** Set loopback test mode
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** 
** Return:
** 
*/
int Si3217x_SetLoopbackMode (proslicChanType_ptr hProslic, ProslicLoopbackModes newMode);


/*
** Function: PROSLIC_SetMuteStatus
**
** Description: 
** Set mute(s)
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** 
** Return:
** 
*/
int Si3217x_SetMuteStatus (proslicChanType_ptr hProslic, ProslicMuteModes muteEn);



/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_RingSetup
**
** Description: 
** configure ringing
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pRingSetup: pointer to ringing config structure
**
** Return:
** none
*/
int Si3217x_RingSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ToneGenSetup
**
** Description: 
** configure tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pTone: pointer to tones config structure
**
** Return:
** none
*/
int Si3217x_ToneGenSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_FSKSetup
**
** Description: 
** configure fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pFsk: pointer to fsk config structure
**
** Return:
** none
*/
int Si3217x_FSKSetup (proslicChanType *pProslic, int preset);

/*
 * Function: Si3217x_ModifyStartBits
 * 
 * Description: To change the FSK start/stop bits field.
 * Returns RC_NONE if OK.
 */

int Si3217x_ModifyCIDStartBits(proslicChanType_ptr pProslic, uInt8 enable_startStop);

/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDTMFDec: pointer to dtmf decoder config structure
**
** Return:
** none
*/
int Si3217x_DTMFDecodeSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pCountryData: pointer to country config structure
**
** Return:
** none
*/
int Si3217x_SetProfile (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedence synthesis
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pZynth: pointer to zsynth config structure
**
** Return:
** none
*/
int Si3217x_ZsynthSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pCI: pointer to ringing config structure
**
** Return:
** none
*/
int Si3217x_GciCISetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pModemDet: pointer to modem det config structure
**
** Return:
** none
*/
int Si3217x_ModemDetSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** preset:   impedance preset to scale
**
** Return:
** none
*/
int Si3217x_TXAudioGainSetup (proslicChanType *pProslic, int preset);
int Si3217x_RXAudioGainSetup (proslicChanType *pProslic, int preset);
#define Si3217x_AudioGainSetup ProSLIC_AudioGainSetup

/*
** Function: PROSLIC_TXAudioGainScale
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pProslic:  pointer to Proslic object
** preset:    pointer to audio gains config structure
** pga_scale: pga_scaling constant
** eq_scale:  equalizer scaling constant
**
** Return:
** none
*/
int Si3217x_TXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);

/*
** Function: PROSLIC_RXAudioGainScale
**
** Description: 
** configure audio gains
**
** Input Parameters: 
** pProslic:  pointer to Proslic object
** preset:    pointer to audio gains config structure
** pga_scale: pga_scaling constant
** eq_scale:  equalizer scaling constant
**
** Return:
** none
*/
int Si3217x_RXAudioGainScale (proslicChanType *pProslic, int preset, uInt32 pga_scale, uInt32 eq_scale);

/*
** Function: PROSLIC_HybridSetup
**
** Description: 
** configure Proslic hybrid
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pHybridCfg: pointer to ringing config structure
**
** Return:
** none
*/
int Si3217x_HybridSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_AudioEQSetup
**
** Description: 
** configure audio equalizers
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pAudioEQ: pointer to ringing config structure
**
** Return:
** none
*/
int Si3217x_AudioEQSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDcFeed: pointer to dc feed config structure
**
** Return:
** none
*/
int Si3217x_DCFeedSetup (proslicChanType *pProslic, int preset);
int Si3217x_DCFeedSetupCfg (proslicChanType *pProslic, ProSLIC_DCfeed_Cfg *cfg, int preset);

/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pGpio: pointer to gpio config structure
**
** Return:
** none
*/
int Si3217x_GPIOSetup (proslicChanType *pProslic);

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPcm: pointer to pcm config structure
**
** Return:
** none
*/
int Si3217x_PCMSetup(proslicChanType *pProslic, int preset);
int Si3217x_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount);

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Enables interrupts
**
** Input Parameters: 
** hProslic: pointer to Proslic object
** pIntData: pointer to interrupt info retrieved
** 
** Return:
** 
*/
int Si3217x_GetInterrupts (proslicChanType_ptr hProslic, proslicIntType *pIntData);

/*
** Function: PROSLIC_ReadHookStatus
**
** Description: 
** Determine hook status
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pHookStat: current hook status
**
** Return:
** none
*/
int Si3217x_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat);

/*
** Function: PROSLIC_WriteLinefeed
**
** Description: 
** Sets linefeed state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** newLinefeed: new linefeed state
**
** Return:
** none
*/
int Si3217x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed);

/*
** Function: PROSLIC_SetLinefeedBroadcast
**
** Description: 
** Sets linefeed state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** newLinefeed: new linefeed state
**
** Return:
** none
*/
int Si3217x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed);

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** abrupt: set this to 1 for abrupt pol rev
** newPolRevState: new pol rev state
**
** Return:
** none
*/
int Si3217x_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState);

/*
** Function: PROSLIC_GPIOControl
**
** Description: 
** Sets gpio of the proslic
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pGpioData: pointer to gpio status
** read: set to 1 to read status, 0 to write
**
** Return:
** none
*/
int Si3217x_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read);

/*
** Function: ProSLIC_MWISetup
**
** Description: 
** Modify default MWI amplitude and switch debounce parameters
**
** Input Parameters: 
** pProslic:		pointer to Proslic object
** vpk_mag:			peak flash voltgage (vpk) - passing a 0 results
**                  in no change to VBATH_NEON
** lcmrmask_mwi:	LCR mask time (ms) after MWI state switch - passing
**                  a 0 results in no change to LCRMASK_MWI
**
** Return:
** none
*/
int Si3217x_MWISetup (proslicChanType *pProslic,uInt16 vpk_mag,uInt16 lcrmask_mwi);

/*
** Function: ProSLIC_MWIEnable
**
** Description: 
** Enable MWI feature
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_MWIEnable (proslicChanType *pProslic);

/*
** Function: ProSLIC_MWIDisable
**
** Description: 
** Disable MWI feature
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_MWIDisable (proslicChanType *pProslic);

/*
** Function: ProSLIC_SetMWIState
**
** Description: 
** Set MWI state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** flash_on: 0 = low, 1 = high (VBATH_NEON)
**
** Return:
** none
*/
int Si3217x_SetMWIState (proslicChanType *pProslic,uInt8 flash_on);

/*
** Function: ProSLIC_GetMWIState
**
** Description: 
** Read MWI state
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** 0 - Flash OFF, 1 - Flash ON, RC_MWI_NOT_ENABLED
*/
int Si3217x_GetMWIState (proslicChanType *pProslic);

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** lampOn: 0 = turn lamp off, 1 = turn lamp on
**
** Return:
** none
**
** Use Deprecated.
*/
int Si3217x_MWI (proslicChanType *pProslic,uInt8 lampOn);

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** Initializes and start tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** timerEn: specifies whether to enable the tone generator timers
**
** Return:
** none
*/
int Si3217x_ToneGenStart (proslicChanType *pProslic, uInt8 timerEn);


/*
** Function: PROSLIC_StopTone
**
** Description: 
** Stops tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_ToneGenStop (proslicChanType *pProslic);


/*
** Function: PROSLIC_StartRing
**
** Description: 
** Initializes and start ring generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_RingStart (proslicChanType *pProslic);


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_RingStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_EnableCID (proslicChanType *pProslic);

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_DisableCID (proslicChanType *pProslic);

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** buffer: buffer to send
** numBytes: num of bytes in the buffer
**
** Return:
** none
*/
int Si3217x_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes);

int Si3217x_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *fsk_buf_avail);

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PCMStart (proslicChanType *pProslic);


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PCMStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_DialPulseDetect
**
** Description: 
** implements pulse dial detection and should be called at every off/on hook transistion
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPulsedialCfg: pointer to pulse dial config
** pPulseDialData: pointer to pulse dial state for current channel
**
** Return:
** none
*/
int Si3217x_DialPulseDetect (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData);

/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pDigit: digit read
**
** Return:
** none
*/
int Si3217x_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit);

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PLLFreeRunStart (proslicChanType *pProslic);

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PLLFreeRunStop (proslicChanType *pProslic);

/*
** Function: PROSLIC_GetPLLFreeRunStatus
**
** Description: 
** Read FREERUN status
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** RC_PLL_FREERUN_ACTIVE (FREERUN active), otherwise 0
*/
int Si3217x_GetPLLFreeRunStatus (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
**
** Input Parameters: 
** pProslic: pointer to Proslic object
** pPulseCfg: pointer to pulse metering config structure
**
** Return:
** none
*/
int Si3217x_PulseMeterSetup (proslicChanType *pProslic, int preset);

/*
** Function: PROSLIC_PulseMeterEnable
**
** Description: 
** Enable pulse meter generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PulseMeterEnable (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterDisable
**
** Description: 
** Disable pulse meter generator
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PulseMeterDisable (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PulseMeterStart (proslicChanType *pProslic);

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si3217x_PulseMeterStop (proslicChanType *pProslic);


/*
** Function: PROSLIC_LBCal
**
** Description: 
** Execute longitudinal balance calibration
**
** Input Parameters: 
** hProslic: pointer to array of Proslic objects
** 
** Return:
** 
*/
int Si3217x_LBCal (proslicChanType_ptr *pProslic, int size);


int Si3217x_GetLBCalResult (proslicChanType *pProslic,int32 *result1,int32 *result2,int32 *result3,int32 *result4);
int Si3217x_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result);
int Si3217x_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4);
int Si3217x_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result);

/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3217x_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset);

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3217x_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);


/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3217x_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3217x_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX path gain.
*/
int Si3217x_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX path gain.
*/
int Si3217x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);


/*
** Function: PROSLIC_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3217x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor);


/*
** Function: PROSLIC_PSTNCheck
**
** Description: 
** Continuous monitor of ilong to detect hot pstn line 
*/
int Si3217x_PSTNCheck(proslicChanType *pProslic, proslicPSTNCheckObjType *pstnCheckObj);


/*
** Function: PROSLIC_PSTNCheck
**
** Description: 
** Continuous monitor of ilong to detect hot pstn line 
*/
int Si3217x_DiffPSTNCheck(proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pstnCheckObj);

/*
** Function: PROSLIC_SetPowersaveMode
**
** Description: 
** Enable or disable powersave mode 
*/
int Si3217x_SetPowersaveMode(proslicChanType *pProslic, int pwrsave);

/*
** Function: PROSLIC_ReadMADCScaled
**
** Description: 
** Read MADC (or other sensed voltages/currents) and
** return scaled value in int32 format
*/
int32 Si3217x_ReadMADCScaled(proslicChanType_ptr pProslic, uInt16 addr, int32 scale);

/*
** Function: PROSLIC_SetDAAEnable
**
** Description: 
** Enable FXO channel (Si32178 only)
*/
int Si3217x_SetDAAEnable(proslicChanType *pProslic, int enable);

#ifdef REALTEK_PATCH_FOR_SILAB
int Si3217x_GetLinefeedStatus (proslicChanType *pProslic,uInt8 *newLinefeed);
void Si3217x_Set_Ring_Cadence_ON(proslicChanType *pProslic, unsigned short msec, unsigned char timer_enable);
void Si3217x_Set_Ring_Cadence_OFF(proslicChanType *pProslic, unsigned short msec, unsigned char timer_enable);
void Si3217x_Set_Impendance_Silicon(proslicChanType *pProslic, unsigned short country, unsigned short impd /*reserve*/);
void Si3217x_Set_Impendance(proslicChanType *pProslic, unsigned short preset);
void Si3217x_SendNTTCAR(proslicChanType *pProslic);
unsigned int Si3217x_SendNTTCAR_check(unsigned int chid, proslicChanType *pProslic, unsigned long time_out);
int SI3217x_SetUserMode(proslicChanType *pProslic, int on);
void SI3217X_Set_SO_DTx_Loopback(proslicChanType *pProslic, unsigned int enable);
#endif
#endif

