/*!
*	\file			appcall.h
*	\brief			
*	\Author		kelbch 
*
*	@(#)	%filespec: appcall.h~6 %
*
*******************************************************************************
*	\par	History
*	\n==== History ============================================================\n
*	date			name		version	 action                                          \n
*	----------------------------------------------------------------------------\n
*******************************************************************************/

#if	!defined( APPCALL_H )
#define	APPCALL_H

typedef enum
{
   E_APPCALL_AUTOMAT_MODE_OFF,      /*!< call object automat mode off */
   E_APPCALL_AUTOMAT_MODE_ON        /*!< call object automat mode on, incoming and outgoing calls shall be handled in simple statemachine */
}E_APPCALL_AUTOMAT_MODE;

/*! \brief exchange structure between CMBS API layer and upper application */
typedef  struct
{
   E_CMBS_IE_TYPE e_IE;          /*! IE type */
   char *         psz_Value;     /*! string value in case of upper -> CMBS API layer */
}ST_APPCALL_PROPERTIES, * PST_APPCALL_PROPERTIES;

/*! \brief CMBS API layer call states */
typedef enum
{
   E_APPCMBS_CALL_CLOSE,         /*!<  line is closed */
   
   E_APPCMBS_CALL_INC_PEND,      /*!<  CMBS target is informed of an incoming call*/
   E_APPCMBS_CALL_INC_RING,      /*!<  CMBS target let the handset ringing */
   
   E_APPCMBS_CALL_OUT_PEND,      /*!<  CMBS-API layer received a outgoing call establishment event */
   E_APPCMBS_CALL_OUT_PEND_DIAL, /*!<  Digits will be collected in CLD array, if Dialtone was switched on.
                                       it is automatically switched off, if call enters this state */
   E_APPCMBS_CALL_OUT_INBAND,    /*!<  The outgoing call is set-up to carry inband signalling, e.g. network tones */
   E_APPCMBS_CALL_OUT_PROC,      /*!<  The outgoing call is proceeding state */
   E_APPCMBS_CALL_OUT_RING,      /*!<  The outgoing call is in ringing state, if not the inband tone is available */
   
   E_APPCMBS_CALL_ACTIVE,        /*!<  The call is in active mode, media shall be transmitted after channel start */
   E_APPCMBS_CALL_RELEASE,       /*!<  The call is in release mode */
   
}E_APPCMBS_CALL;

/*! \brief CBS API layer media state */
typedef enum
{
   E_APPCMBS_MEDIA_CLOSE,        /*!< Media entity is closed */
   E_APPCMBS_MEDIA_PEND,         /*!< Media entity is prepared, codec negotiated and channel ID from Target available */
   E_APPCMBS_MEDIA_ACTIVE        /*!< Media entity is started to stream */
} E_APPCMBS_MEDIA;

/*! \brief  Line node, it contains every important information of the line */
typedef struct
{
   u32   u32_foo;             /*!< foo */  //TODO
} ST_LINE_OBJ, * PST_LINE_OBJ;

/*! \brief  Call node, it contains every important information of the connection */
typedef struct
{
   u32   u32_CallInstance;             /*!< Call Instance to identify the call on CMBS */
   u8    u8_LineId;                    /*!< Line ID*/
   ST_IE_CALLERPARTY  st_CallerParty;  /*!< Caller Party, incoming call CLI, outgoing call Handset number */
   ST_IE_CALLEDPARTY  st_CalledParty;  /*!< Called Party, incoming call ringing mask, outgoing call to be dialled number */
   ST_IE_CALLERPARTY  st_TmpParty;     /*!< further feature, temp party, e.g. call waiting active and the CMBS API layer has to restore connection*/
   char  ch_TmpParty[30];              /*!< buffer of temp party number */
   char  ch_CallerID[30];              /*!< buffer of caller party number */
   char  ch_CalledID[30];              /*!< buffer of called party number */
   u32   u32_ChannelID;                /*!< Channel ID to identify the media connection on CMBS */
   u32	 u32_ChannelParameter;		   /*!< Channel Parameter provides information about the parameter settings, e.g. IOM - used slots */
   E_CMBS_AUDIO_CODEC   e_Codec;       /*!< used codec */
   u8					pu8_CodecsList[CMBS_AUDIO_CODEC_MAX];   /*!< Codecs list */
   u8					u8_CodecsLength;
   E_APPCMBS_CALL       e_Call;        /*!< call state */
   E_APPCMBS_MEDIA      e_Media;       /*!< media entity state */
   bool                 b_Incoming;    /*!< TRUE for incoming calls, FALSE o/w */
} ST_CALL_OBJ, * PST_CALL_OBJ;

                                          /*! \brief max line */
#define     APPCALL_LINEOBJ_MAX        40
                                          /*! \brief max connection for CMBS API layer */
#define     APPCALL_CALLOBJ_MAX        8  

#define     APPCALL_NO_LINE	         0xFF
#define     APPCALL_NO_CALL	         0xFFFF


#if defined( __cplusplus )
extern "C"
{
#endif

void        appcall_Initialize (void);
void        appcall_AutomatMode ( E_APPCALL_AUTOMAT_MODE e_Mode );
u16         appcall_EstablishCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties );
int         appcall_ReleaseCall( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u16 u16_CallId, char * psz_CLI );
int         appcall_AnswerCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u16 u16_CallId, char * psz_CLI );
int         appcall_ProgressCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u16 u16_CallId, char * psz_CLI );
int         appcall_DisplayCall ( PST_APPCALL_PROPERTIES pst_Properties, int n_Properties, u16 u16_CallId, char * psz_CLI );
int         appcall_ResumeCall( u16 u16_CallId, char * psz_CLI );
int         appcall_HoldCall( u16 u16_CallId, char * psz_CLI );

void        appmedia_CallObjTonePlay( char * psz_Value, int bo_On, u16 u16_CallId, char * psz_Cli );
void        appmedia_CallObjMediaStart( u32 u32_CallInstance, u16 u16_CallId, u16 u16_StartSlot, char * psz_CLI );
void        appmedia_CallObjMediaStop( u32 u32_CallInstance, u16 u16_CallId, char * psz_CLI );
void        appmedia_CallObjMediaOffer( u16 u16_CallId, char ch_Audio );

void        appcall_InfoPrint(void);


void        app_PrintCallProgrInfo( E_CMBS_CALL_PROGRESS e_Progress );
void        app_PrintCallInfoType( E_CMBS_CALL_INFO_TYPE e_Type );
//void        app_PrintCallInfo( E_CMBS_CALL_INFO e_Info );

#if defined( __cplusplus )
}
#endif

#endif	//	APPCALL_H
//*/
