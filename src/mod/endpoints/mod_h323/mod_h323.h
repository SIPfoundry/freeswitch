/* 
 * H323 endpoint interface for Freeswitch Modular Media Switching Software Library /
 * Soft-Switch Application
 *
 * Version: MPL 1.1
 *
 * Copyright (c) 2010 Ilnitskiy Mixim (max.h323@gmail.com)
 * Copyright (c) 2010 Georgiewskiy Yuriy (bottleman@icf.org.ru)
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Contributor(s):
 * 
 * 
 * 
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 *
 * mod_h323.h -- H323 endpoint
 *
 *	Version 0.0.57
*/

#if defined(__GNUC__) && defined(HAVE_VISIBILITY)
#pragma GCC visibility push(default)
#endif

#include <ptlib.h>
#include <h323.h>
#include <h323neg.h>
#include <h323pdu.h>
#include <h323caps.h>
#include <ptclib/delaychan.h>
#include <h323t38.h>
#include "t38proto.h"
#include "t38.h"
#include <mediafmt.h>
#include <list>


#if defined(__GNUC__) && defined(HAVE_VISIBILITY)
#pragma GCC visibility pop
#endif

#undef strcasecmp
#undef strncasecmp

#define HAVE_APR
#include <switch.h>
#include <switch_version.h>
#define MODNAME "mod_h323"
#define OpalT38_IFP_COR       GetOpalT38_IFP_COR()
#define OpalT38_IFP_PRE       GetOpalT38_IFP_PRE()

const char* const GetDirections[H323Channel::NumDirections+1] = {
	"IsBidirectional",
	"IsTransmitter",
	"IsReceiver",
	"NumDirections"
};

const char * const PayloadTypesNames[RTP_DataFrame::LastKnownPayloadType] = {
	"PCMU",
	"FS1016",
	"G721",
	"GSM",
	"G7231",
	"DVI4_8k",
	"DVI4_16k",
	"LPC",
	"PCMA",
	"G722",
	"L16_Stereo",
	"L16_Mono",
	"G723",
	"CN",
	"MPA",
	"G728",
	"DVI4_11k",
	"DVI4_22k",
	"G729",
	"CiscoCN",
	NULL, NULL, NULL, NULL, NULL,
	"CelB",
	"JPEG",
	NULL, NULL, NULL, NULL,
	"H261",
	"MPV",
	"MP2T",
	"H263"
};



const char* const GetAnswerCallResponse[H323Connection::NumAnswerCallResponses+1]={
	"AnswerCallNow",
	"AnswerCallDenied",
	"AnswerCallPending",
	"AnswerCallDeferred",
	"AnswerCallAlertWithMedia",
	"AnswerCallDeferredWithMedia",
	"AnswerCallDeniedByInvalidCID",
	"AnswerCallNowWithAlert",
	"NumAnswerCallResponses"
};

const char* const GetMainTypes[H323Capability::e_NumMainTypes+1] = {
	"Audio",
	"Video",
	"Data",
	"UserInput",
	"ExtendVideo",
	"GenericControl",
	"ConferenceControl",
	"NumMainTypes"	
};

extern void SetT38_IFP_PRE();
class OpalMediaFormat;
class H245_T38FaxProfile;
class OpalT38Protocol; 
extern const OpalMediaFormat & GetOpalT38_IFP_COR();
extern const OpalMediaFormat & GetOpalT38_IFP_PRE();

typedef enum {
	TFLAG_IO = (1 << 0),
	TFLAG_INBOUND = (1 << 1),
	TFLAG_OUTBOUND = (1 << 2),
	TFLAG_READING = (1 << 3),
	TFLAG_WRITING = (1 << 4),
	TFLAG_BYE = (1 << 5),
	TFLAG_VOICE = (1 << 6),
	TFLAG_RTP_READY = (1 << 7),
	TFLAG_CODEC_READY = (1 << 8),
	TFLAG_TRANSPORT = (1 << 9),
	TFLAG_ANSWER = (1 << 10),
	TFLAG_VAD_IN = (1 << 11),
	TFLAG_VAD_OUT = (1 << 12),
	TFLAG_VAD = (1 << 13),
	TFLAG_DO_CAND = (1 << 14),
	TFLAG_DO_DESC = (1 << 15),
	TFLAG_LANADDR = (1 << 16),
	TFLAG_AUTO = (1 << 17),
	TFLAG_DTMF = (1 << 18),
	TFLAG_TIMER = (1 << 19),
	TFLAG_TERM = (1 << 20),
	TFLAG_TRANSPORT_ACCEPT = (1 << 21),
	TFLAG_READY = (1 << 22),
} TFLAGS;

struct mod_h323_globals {
	int trace_level;
	char *codec_string;
	char *context;
	char *dialplan;
	int use_rtp_timer;
	char *rtp_timer_name;
	int ptime_override_value;
};

extern struct mod_h323_globals mod_h323_globals;

class FSH323Connection;
class FSH323_ExternalRTPChannel;

typedef struct {
	unsigned int flags;
	switch_timer_t read_timer;
	switch_codec_t read_codec;
	switch_codec_t write_codec;
	switch_frame_t read_frame;

	switch_timer_t vid_read_timer;
	switch_codec_t vid_read_codec;
	switch_codec_t vid_write_codec;
	switch_rtp_t *rtp_session;
	switch_mutex_t *flag_mutex;
	switch_mutex_t *h323_mutex;
	switch_mutex_t *h323_io_mutex;

	FSH323Connection *me;
	bool			active_connection;
	char			*token;
} h323_private_t;

#define DECLARE_CALLBACK0(name)                           \
    static switch_status_t name(switch_core_session_t *session) {       \
        h323_private_t *tech_pvt = (h323_private_t *) switch_core_session_get_private(session); \
	FSH323Connection *me = (tech_pvt && tech_pvt->me != NULL) ? tech_pvt->me : NULL; \
        return me != NULL ? me->name() : SWITCH_STATUS_FALSE; } \
switch_status_t name()

#define DECLARE_CALLBACK1(name, type1, name1)                           \
    static switch_status_t name(switch_core_session_t *session, type1 name1) { \
        h323_private_t *tech_pvt = (h323_private_t *) switch_core_session_get_private(session); \
	FSH323Connection *me = (tech_pvt && tech_pvt->me != NULL) ? tech_pvt->me : NULL; \
        return me != NULL ? me->name(name1) : SWITCH_STATUS_FALSE; } \
switch_status_t name(type1 name1)

#define DECLARE_CALLBACK3(name, type1, name1, type2, name2, type3, name3) \
    static switch_status_t name(switch_core_session_t *session, type1 name1, type2 name2, type3 name3) { \
        h323_private_t *tech_pvt = (h323_private_t *) switch_core_session_get_private(session); \
	FSH323Connection *me = (tech_pvt && tech_pvt->me != NULL) ? tech_pvt->me : NULL; \
        return me != NULL ? me->name(name1, name2, name3) : SWITCH_STATUS_FALSE; } \
switch_status_t name(type1 name1, type2 name2, type3 name3)

class FSH323EndPoint;
class FSProcess:public PLibraryProcess { 
	PCLASSINFO(FSProcess, PLibraryProcess);

  public:
	FSProcess();
	~FSProcess();

	bool Initialise(switch_loadable_module_interface_t *iface);

	void Main()
		{ }
	     FSH323EndPoint & GetH323EndPoint() const {
		return *m_h323endpoint;
  } protected:
	      FSH323EndPoint * m_h323endpoint;
};

struct FSListener {
	FSListener()
		{ }
	PString name;
	H323ListenerTCP *listenAddress;
	PString localUserName;
	PString gatekeeper;
};
class FSGkRegThread;

class OpalMediaFormat;
class FSH323EndPoint:public H323EndPoint {

	PCLASSINFO(FSH323EndPoint, H323EndPoint);
  public:
	FSH323EndPoint();
	~FSH323EndPoint();


	 /* Create a connection that uses the specified call. */
	virtual H323Connection *CreateConnection(unsigned callReference, void *userData, H323Transport * transport, H323SignalPDU * setupPDU);
	virtual bool OnSetGatewayPrefixes(PStringList & prefixes) const;

	bool Initialise(switch_loadable_module_interface_t *iface);

	switch_status_t ReadConfig(int reload);

	void StartGkClient(int retry, PString * gkAddress, PString * gkIdentifer, PString * gkInterface);
	void StopGkClient();

	switch_endpoint_interface_t *GetSwitchInterface() const {
		return m_freeswitch;
	} 
	FSH323Connection *FSMakeCall(const PString & dest, void *userData);
	list < FSListener > m_listeners;
	int m_ai;
	int m_pi;
  protected:
	PStringList m_gkPrefixes;
	switch_endpoint_interface_t *m_freeswitch;
	PString m_gkAddress;
	PString m_gkIdentifer;
	PString m_endpointname;
	PString m_gkInterface;
	bool m_faststart;
	bool m_h245tunneling;
	bool m_h245insetup;
	bool m_dtmfinband;
	int m_gkretry;
	FSGkRegThread *m_thread;
	bool m_stop_gk;
	bool m_fax_old_asn;
};


class FSGkRegThread:public PThread {
	PCLASSINFO(FSGkRegThread, PThread);
  public:
	FSGkRegThread(FSH323EndPoint * endpoint, PString * gkAddress, PString * gkIdentifer, PString * gkInterface, int retry = 0)
  :	PThread(10000), m_ep(endpoint), m_retry(retry), m_gkAddress(gkAddress), m_gkIdentifer(gkIdentifer), m_gkInterface(gkInterface) {
	} void Main() {
		m_ep->StartGkClient(m_retry, m_gkAddress, m_gkIdentifer, m_gkInterface);
	}
  protected:
	FSH323EndPoint * m_ep;
	int m_retry;
	PString *m_gkAddress;
	PString *m_gkIdentifer;
	PString *m_gkInterface;
};


class FSH323Connection:public H323Connection {
	PCLASSINFO(FSH323Connection, H323Connection)

  public:
	FSH323Connection(FSH323EndPoint & endpoint,
					 H323Transport * transport,
					 unsigned callReference, switch_caller_profile_t *outbound_profile,
					 switch_core_session_t *fsSession, switch_channel_t *fsChannel);
	~FSH323Connection();
	virtual void AttachSignalChannel(
      const PString & token,
      H323Transport * channel,
      PBoolean answeringCall
    );
	virtual H323Channel *CreateRealTimeLogicalChannel(const H323Capability & capability,
							H323Channel::Directions dir,
							unsigned sessionID, const H245_H2250LogicalChannelParameters * param, RTP_QOS * rtpqos = NULL);
	virtual PBoolean OnStartLogicalChannel(H323Channel & channel);
	virtual PBoolean OnCreateLogicalChannel(const H323Capability & capability, H323Channel::Directions dir, unsigned &errorCode);
	virtual bool OnReceivedSignalSetup(const H323SignalPDU & setupPDU);
	virtual bool OnReceivedCallProceeding(const H323SignalPDU & pdu);
	virtual void OnReceivedReleaseComplete(const H323SignalPDU & pdu);
	virtual bool OnReceivedProgress(const H323SignalPDU &);
	virtual bool OnSendCallProceeding(H323SignalPDU & callProceedingPDU);
	virtual bool OnSendReleaseComplete(H323SignalPDU & pdu);
	virtual PBoolean OpenLogicalChannel(const H323Capability & capability, unsigned sessionID, H323Channel::Directions dir);
	void setRemoteAddress(const char *remoteIP, WORD remotePort);
	virtual void OnSetLocalCapabilities();
	virtual bool OnAlerting(const H323SignalPDU & alertingPDU, const PString & user);
	virtual void AnsweringCall(AnswerCallResponse response);
	virtual void OnEstablished();
	virtual void OnModeChanged(const H245_ModeDescription & newMode);
	virtual bool OnRequestModeChange(const H245_RequestMode & pdu,
                                         H245_RequestModeAck & ack,
                                         H245_RequestModeReject & reject,
                                         PINDEX & selectedMode);
	virtual bool OnSendSignalSetup(H323SignalPDU & setupPDU);
	bool SetLocalCapabilities();
	bool decodeCapability(const H323Capability & capability, const char **dataFormat, int *payload = 0, PString * capabName = 0);
	virtual H323Connection::AnswerCallResponse OnAnswerCall(const PString & caller, const H323SignalPDU & signalPDU, H323SignalPDU & connectPDU);
	virtual bool OnReceivedCapabilitySet(const H323Capabilities & remoteCaps,
	const H245_MultiplexCapability * muxCap, H245_TerminalCapabilitySetReject & reject);
	switch_core_session_t *GetSession() const {
		return m_fsSession;
	} 
	FSH323EndPoint* GetEndPoint() const{
		return m_endpoint;
	}
	virtual void SendUserInputTone(char tone, unsigned duration = 0, unsigned logicalChannel = 0, unsigned rtpTimestamp = 0);
	virtual void OnUserInputTone(char, unsigned, unsigned, unsigned);
	virtual void OnUserInputString(const PString & value);
	void CleanUpOnCall();
    
	DECLARE_CALLBACK0(on_init);
	DECLARE_CALLBACK0(on_routing);
	DECLARE_CALLBACK0(on_execute);
	DECLARE_CALLBACK0(on_exchange_media);
	DECLARE_CALLBACK0(on_soft_execute);
	DECLARE_CALLBACK1(kill_channel, int, sig);
	DECLARE_CALLBACK1(send_dtmf, const switch_dtmf_t *, dtmf);
	DECLARE_CALLBACK1(receive_message, switch_core_session_message_t *, msg);
	DECLARE_CALLBACK1(receive_event, switch_event_t *, event);
	DECLARE_CALLBACK0(state_change);

	DECLARE_CALLBACK3(read_audio_frame, switch_frame_t **, frame, switch_io_flag_t, flags, int, stream_id);
	DECLARE_CALLBACK3(write_audio_frame, switch_frame_t *, frame, switch_io_flag_t, flags, int, stream_id);
	DECLARE_CALLBACK3(read_video_frame, switch_frame_t **, frame, switch_io_flag_t, flag, int, stream_id);
	DECLARE_CALLBACK3(write_video_frame, switch_frame_t *, frame, switch_io_flag_t, flag, int, stream_id);

	bool m_callOnPreAnswer;
	bool m_startRTP;
	bool m_rxChannel;
	bool m_txChannel;
	bool m_ChannelAnswer;
	bool m_ChannelProgress;
	unsigned char m_select_dtmf;
	PSyncPoint m_rxAudioOpened;
	PSyncPoint m_txAudioOpened;
	unsigned m_active_sessionID;
	bool m_active_channel_fax;
	int m_rtp_resetting;
	bool m_isRequst_fax;
	bool m_channel_hangup;
  protected:
	FSH323EndPoint * m_endpoint;
	PString m_remoteAddr;
	int m_remotePort;
	switch_core_session_t *m_fsSession;
	switch_channel_t *m_fsChannel;
	PIPSocket::Address m_RTPlocalIP;
	WORD m_RTPlocalPort;
	unsigned char m_buf[SWITCH_RECOMMENDED_BUFFER_SIZE];
};


class FSH323_ExternalRTPChannel:public H323_ExternalRTPChannel {
	PCLASSINFO(FSH323_ExternalRTPChannel, H323_ExternalRTPChannel);
  public:
	/* Create a new channel. */
	FSH323_ExternalRTPChannel(FSH323Connection & connection, const H323Capability & capability, Directions direction, unsigned sessionID, const PIPSocket::Address & ip, WORD dataPort);
	/* Destructor */
	~FSH323_ExternalRTPChannel();

	virtual PBoolean Start();
	virtual PBoolean OnReceivedAckPDU(const H245_H2250LogicalChannelAckParameters & param);
	virtual PBoolean OnSendingPDU(H245_H2250LogicalChannelParameters & param);
	virtual PBoolean OnReceivedPDU(const H245_H2250LogicalChannelParameters & param, unsigned &errorCode);
	virtual void OnSendOpenAck(H245_H2250LogicalChannelAckParameters & param);


  private:
	FSH323Connection * m_conn;
	const H323Capability *m_capability;
	switch_core_session_t *m_fsSession;
	switch_channel_t *m_fsChannel;
	OpalMediaFormat *m_format;
	PString m_RTPremoteIP;
	WORD m_RTPremotePort;
	PString m_RTPlocalIP;
	WORD m_RTPlocalPort;
	BYTE payloadCode;
	unsigned m_sessionID;
	int m_rtp_resetting;
};

class BaseG7231Capab:public H323AudioCapability {
	PCLASSINFO(BaseG7231Capab, H323AudioCapability);
  public:
	BaseG7231Capab(const char *fname, bool annexA = true)
  :	H323AudioCapability(7, 4), m_name(fname), m_aa(annexA) {
	} 
	virtual PObject *Clone() const {
		return new BaseG7231Capab(*this);
	} 
	virtual unsigned GetSubType() const {
		return H245_AudioCapability::e_g7231;
	} 
	virtual PString GetFormatName() const {
		return m_name;
	} 
	virtual H323Codec *CreateCodec(H323Codec::Direction direction) const {
		return 0;
	} 
	virtual Comparison Compare(const PObject & obj) const {
		Comparison res = H323AudioCapability::Compare(obj);
		if         (res != EqualTo)
			           return res;
		bool aa = static_cast < const BaseG7231Capab & >(obj).m_aa;
		if    (aa && !m_aa)
			      return LessThan;
		if    (m_aa && !aa)
			      return GreaterThan;
		      return EqualTo;
	} 
	virtual bool OnSendingPDU(H245_AudioCapability & pdu, unsigned packetSize) const {
		pdu.SetTag(GetSubType());
		H245_AudioCapability_g7231 & g7231 = pdu;
		g7231.m_maxAl_sduAudioFrames = packetSize;
		g7231.m_silenceSuppression = m_aa;
		return true;
	}
	virtual bool OnReceivedPDU(const H245_AudioCapability & pdu, unsigned &packetSize) {
		if (pdu.GetTag() != H245_AudioCapability::e_g7231)
			return false;
		const H245_AudioCapability_g7231 & g7231 = pdu;
		packetSize = g7231.m_maxAl_sduAudioFrames;
		m_aa = (g7231.m_silenceSuppression != 0);
		return true;
	}

  protected:
	const char *m_name;
	bool m_aa;
};

class BaseG729Capab:public H323AudioCapability {
	PCLASSINFO(BaseG729Capab, H323AudioCapability);
  public:
	BaseG729Capab(const char *fname, unsigned type = H245_AudioCapability::e_g729)
  :	H323AudioCapability(24, 6), m_name(fname), m_type(type) {
	} 
	virtual PObject *Clone() const
	{
		return new BaseG729Capab(*this);
	}
	virtual unsigned GetSubType() const {
		return m_type;
	}
	virtual PString GetFormatName() const {
		return m_name;
	}
	virtual H323Codec *CreateCodec(H323Codec::Direction direction) const {
		return 0;
	} 
  protected:
	const char *m_name;
	unsigned m_type;
};

class BaseGSM0610Cap:public H323AudioCapability {
	PCLASSINFO(BaseGSM0610Cap, H323AudioCapability);

  public:

	BaseGSM0610Cap(const char *fname, unsigned type = H245_AudioCapability::e_gsmFullRate)
  :	H323AudioCapability(24, 2), m_name(fname), m_type(type), m_comfortNoise(0), m_scrambled(0) {
	} 
	virtual PObject *Clone() const {
		return new BaseGSM0610Cap(*this);
	}
	virtual H323Codec *CreateCodec(H323Codec::Direction direction) const {
		return 0;
	}
	virtual unsigned GetSubType() const {
		return H245_AudioCapability::e_gsmFullRate;
	}
	virtual PString GetFormatName() const {
		return m_name;
	}
	virtual bool OnSendingPDU(H245_AudioCapability & pdu, unsigned packetSize) const {
		pdu.SetTag(H245_AudioCapability::e_gsmFullRate);
		H245_GSMAudioCapability & gsm = pdu;
		gsm.m_audioUnitSize = packetSize * 33;
		gsm.m_comfortNoise = m_comfortNoise;
		gsm.m_scrambled = m_scrambled;
		return true;
	}
	virtual bool OnReceivedPDU(const H245_AudioCapability & pdu, unsigned &packetSize) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"==============>BaseGSM0610Cap::OnReceivedPDU [%p]\n", this);
		if (pdu.GetTag() != H245_AudioCapability::e_gsmFullRate)
			return false;
		const H245_GSMAudioCapability & gsm = pdu;
		packetSize = (gsm.m_audioUnitSize + 32) / 33;
		m_comfortNoise = gsm.m_comfortNoise;
		m_scrambled = gsm.m_scrambled;
		return true;
	}

  protected:
	const char *m_name;
	int m_comfortNoise;
	int m_scrambled;
	unsigned m_type;
};


class FSH323_T38Capability : public H323_T38Capability
{
    PCLASSINFO(FSH323_T38Capability, H323_T38Capability);
  public:
    FSH323_T38Capability(const OpalMediaFormat &_mediaFormat)
      : H323_T38Capability(e_UDP),
        mediaFormat(_mediaFormat) {
	}
    virtual PObject * Clone() const {
		return new FSH323_T38Capability(*this);
	}
    virtual PString GetFormatName() const { 
		return mediaFormat; 
	}
    virtual H323Channel * CreateChannel(
      H323Connection & connection,
      H323Channel::Directions dir,
      unsigned sessionID,
      const H245_H2250LogicalChannelParameters * param    ) const;
  protected:
    const OpalMediaFormat &mediaFormat;
};

class FSH323_T38CapabilityCor : public FSH323_T38Capability {
  public:
    FSH323_T38CapabilityCor() : FSH323_T38Capability(OpalT38_IFP_COR) {}
};

class FSH323_T38CapabilityPre : public FSH323_T38Capability {
  public:
    FSH323_T38CapabilityPre() : FSH323_T38Capability(OpalT38_IFP_PRE) {}
};


H323Channel * FSH323_T38Capability::CreateChannel(
    H323Connection & connection,
    H323Channel::Directions direction,
    unsigned int sessionID,
    const H245_H2250LogicalChannelParameters * params) const
{
 switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,"FSH323_T38Capability::CreateChannel %p  sessionID= %u direction=%s [%p]\n"
    , &connection
    , sessionID
    , GetDirections[direction]
    , this);

  return connection.CreateRealTimeLogicalChannel(*this, direction, sessionID, params);
}


#define DEFINE_H323_CAPAB(cls,base,param,name) \
class cls : public base { \
  public: \
    cls() : base(name,param) { } \
}; \
H323_REGISTER_CAPABILITY(cls,name) \


#define DEFINE_H323_CAPAB_m(cls,base,name) \
class cls : public base { \
  public: \
    cls() : base(name) { } \
}; \
H323_REGISTER_CAPABILITY(cls,name) \


DEFINE_H323_CAPAB(FS_G7231_5, BaseG7231Capab, false, OPAL_G7231_5k3 "{sw}")
DEFINE_H323_CAPAB(FS_G7231_6, BaseG7231Capab, false, OPAL_G7231_6k3 "{sw}")
DEFINE_H323_CAPAB(FS_G7231A_5, BaseG7231Capab, true, OPAL_G7231A_5k3 "{sw}")
DEFINE_H323_CAPAB(FS_G7231A_6, BaseG7231Capab, true, OPAL_G7231A_6k3 "{sw}")
DEFINE_H323_CAPAB(FS_G729, BaseG729Capab, H245_AudioCapability::e_g729, OPAL_G729 "{sw}")
DEFINE_H323_CAPAB(FS_G729A, BaseG729Capab, H245_AudioCapability::e_g729AnnexA, OPAL_G729A "{sw}")
DEFINE_H323_CAPAB(FS_G729B, BaseG729Capab, H245_AudioCapability::e_g729wAnnexB, OPAL_G729B "{sw}")
DEFINE_H323_CAPAB(FS_G729AB, BaseG729Capab, H245_AudioCapability::e_g729AnnexAwAnnexB, OPAL_G729AB "{sw}")
DEFINE_H323_CAPAB(FS_GSM, BaseGSM0610Cap, H245_AudioCapability::e_gsmFullRate, OPAL_GSM0610 "{sw}")

static FSProcess *h323_process = NULL;
