/***************************************************************************************
	Copyright 2000-2001 ATMEL Corporation.
	
	This file is part of atmel wireless lan drivers.

    Atmel wireless lan drivers is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Atmel wireless lan drivers is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Atmel wireless lan drivers; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

**************************************************************************************/

#ifndef __frame_h_OK__
#define __frame_h_OK__

/////////////////////////////////////////////////////////////////////////////
//	FRAME CONTROL BIT 0
//
#define C80211_PROTOCOL_VERSION					0x00
//	FRAME TYPE
#define C80211_TYPE_MASK						0x0C	// 00001100
#define C80211_TYPE_MGMT						0x00	// 00000000
#define C80211_TYPE_CTRL						0x04	// 00000100
#define C80211_TYPE_DATA						0x08	// 00001000
#define C80211_TYPE_RSRV						0x0C	// 00001100

// FRAME SUBTYPE
#define C80211_SUBTYPE_MASK						0xF0
// Management subtypes
#define C80211_SUBTYPE_MGMT_ASS_REQUEST			0x00
#define C80211_SUBTYPE_MGMT_ASS_RESPONSE		0x10
#define	C80211_SUBTYPE_MGMT_REASS_REQUEST		0x20
#define C80211_SUBTYPE_MGMT_REASS_RESPONSE		0x30
#define C80211_SUBTYPE_MGMT_ProbeRequest		0x40
#define C80211_SUBTYPE_MGMT_ProbeResponse		0x50
#define C80211_SUBTYPE_MGMT_BEACON				0x80
#define C80211_SUBTYPE_MGMT_ATIM				0x90
#define C80211_SUBTYPE_MGMT_DISASSOSIATION		0xA0
#define C80211_SUBTYPE_MGMT_Authentication		0xB0
#define C80211_SUBTYPE_MGMT_Deauthentication	0xC0
// Control subtypes
#define C80211_SUBTYPE_CTRL_PSPOLL				0xA0
#define C80211_SUBTYPE_CTRL_RTS					0xB0
#define C80211_SUBTYPE_CTRL_CTS					0xC0
#define C80211_SUBTYPE_CTRL_ACK					0xD0
#define C80211_SUBTYPE_CTRL_CFEND				0xE0
#define C80211_SUBTYPE_CTRL_CFEND_CFACK			0xF0
// Data subtypes
#define C80211_SUBTYPE_DATA_DATA				0x00
#define C80211_SUBTYPE_DATA_CFACK				0x10
#define C80211_SUBTYPE_DATA_CFPOLL				0x20
#define C80211_SUBTYPE_DATA_CFACK_CFPOLL		0x30
#define C80211_SUBTYPE_DATA_NULL				0x40
#define C80211_SUBTYPE_DATA_CFACK_ND			0x50
#define C80211_SUBTYPE_DATA_CFPOLL_ND			0x60
#define C80211_SUBTYPE_DATA_CFACK_CFPOLL_ND		0x70


/////////////////////////////////////////////////////////////////////////////
//	FRAME CONTROL BIT 1
//
#define C80211_FRMCTRL_TO_DS					0x01
// All Data type frames from STA->AP
#define C80211_FRMCTRL_FROM_DS					0x02
//	TO_DS-FROM_DS
//	0		0		DATA frames STA->STA in IBSS, MGMT, CTRL frames
//	1		0		DATA frames to DS
//	0		1		DATA frames exiting the DS
//	1		1		WDS frames AP->AP
#define C80211_FRMCTRL_MORE_FRAG				0x04
// More Fragments.
// Set to 1 in DATA and MGMT frames which have another fragment of the current MSDU or MMPDU
#define C80211_FRMCTRL_RETRY					0x08
// Set to 1 in DATA and MGMT retransmitted frames
#define C80211_FRMCTRL_PWR_MGMT					0x10
// Power management mode. 
//	Always 0 when transmitting from AP
#define C80211_FRMCTRL_MORE_DATA				0x20
//	Directed Frames
//	from AP->(STA in power save mode) for MSDU / MMPDU frames
//	from (CF-Pollable STA)->AP for MSDU frames
//	Multicast
//	From AP when additional multicast frames during this beacon interval
#define C80211_FRMCTRL_WEP						0x40
//	Valid only in MSDU using WEP and MMPDU SubType Authentication
#define C80211_FRMCTRL_ORDER					0x80
//	Set in MSDUs/fragments transferred using the Strictly-Ordered service class


/////////////////////////////////////////////////////////////////////////////
//	DURATION/ID
//
//	a)	In C80211_SUBTYPE_CTRL_PSPOLL AID (Association Identity) in 14 LSB
//		with bits 15-16 set to 1
//	b)	On ContentionFree period set to 32768 (0x8000)
//	c)	On all other frames set according to each frame type
//


/////////////////////////////////////////////////////////////////////////////
//	Addresses
//
//	BSSID
//	In infrastructure BSS is the MAC of the STA-AP
//	In IBSS a locally administered IEEE MAC address formed from a 46 bit number,
//		setting the MULTICAST_BIT to 0 and the LOCAL_ADMIN_BIT to 1.
//		See also Unsigned64::generate_BSSID()
//	A Broadcast BSSID can be used in C80211_SUBTYPE_MGMT_ProbeRequest
//
//	DA - Destination Address: Final recipient
//	SA - Source Address: Initiating Transmitter
//	RA - Receiving Address: Immediate recipient
//	TA - Transmitting Address: Immediate transmitter
//



/////////////////////////////////////////////////////////////////////////////
//	SequenceControl
//
#define C80211_SEQCTRL_FRAGMENT_MASK			0x000F
#define C80211_SEQCTRL_SEQUENCE_MASK			0xFFF0
//	Sequence number
//	Used in MSDU/MMPDU starting at 0 to modulo(4096). Constant in retransmissions.
//	Fragment number
//	Used in MSDU/MMPDU starting at 0 at first or only fragment. Constant in retransmissions.


/////////////////////////////////////////////////////////////////////////////
//	FrameBody
//	FCS
//


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//	Frame Formats
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//	802.11 General Frame format and DATA frame format
//
typedef struct _s80211GFF {
	UCHAR	FrameControl[2];
	USHORT	DurationID;
//	during the Contention Free period Set to 32768 
//	During the Contention period:
//		if Address1 is multicast set to 0
//		if Address1 is unicast:
//			if MORE_FRAG bit is 0 set to ACK + SIFS (msec)
//			else set to DurationOfNextFrag + 2*ACK + 3*SIFS (msec)
//	All stations process DurationID<32768 to update NAV according to CF rules.
	UCHAR	uAddress1[6];
	UCHAR	uAddress2[6];
	UCHAR	uAddress3[6];
	USHORT	SequenceControl;
	UCHAR	uAddress4[6];
	UCHAR	Body[1508];
	ULONG	FCS;
}s80211GFF;
//	FromDS-ToDS		Address1	Address2	Address3	Address4
//	0		0		DA			SA			BSSID		N/A
//	0		1		DA			BSSID		SA			N/A
//	1		0		BSSID		SA			DA			N/A
//	1		1		RA			TA			DA			SA
//
//	Frame Body is zero-bytes in
//	C80211_SUBTYPE_DATA_NULL
//	C80211_SUBTYPE_DATA_CFACK_ND
//	C80211_SUBTYPE_DATA_CFPOLL_ND
//	C80211_SUBTYPE_DATA_CFACK_CFPOLL_ND
//
typedef struct _s80211Header {
	UCHAR	FrameControl[2];
	USHORT	DurationID;
//	during the Contention Free period Set to 32768 
//	During the Contention period:
//		if Address1 is multicast set to 0
//		if Address1 is unicast:
//			if MORE_FRAG bit is 0 set to ACK + SIFS (msec)
//			else set to DurationOfNextFrag + 2*ACK + 3*SIFS (msec)
//	All stations process DurationID<32768 to update NAV according to CF rules.
	UCHAR	uAddress1[6];
	UCHAR	uAddress2[6];
	UCHAR	uAddress3[6];
	USHORT	SequenceControl;
	UCHAR	uAddress4[6];
//	UCHAR	Body[8];
} s80211Header;



/////////////////////////////////////////////////////////////////////////////
//	802.11 Control Frame formats
//
//	RTS (Request To Send) Frame Format
typedef struct _s80211_RTS {
	UCHAR	FrameControl[2];
	USHORT	DurationID;	// Transmitting duration + CTS + ACK + 3 * SIFS (msecs)
	UCHAR	RA[6];
	UCHAR	TA[6];
	ULONG	FCS;
} s80211_RTS;
//	CTS (Clear To Send) Frame Format
typedef struct _s80211_CTS {
	UCHAR	FrameControl[2];
	USHORT	DurationID;		// DurationIDOf(RTS) - CTS - SIFS (msecs)
	UCHAR	RA[6];			// Copied from TA of RTS
	ULONG	FCS;
} s80211_CTS;
//	ACK (Acknowledgment) Frame Format
typedef struct _s80211_ACK {
	UCHAR	FrameControl[2];
	USHORT	DurationID;
//	Set to 0 if C80211_FRMCTRL_MORE_FRAG was 0 in prev directed DATA / MGMT
//	else set to DurationIDOf(Dir.DATA/MGMT) - ACK - SIFS (msecs)
//	Set to 1 if C80211_FRMCTRL_MORE_FRAG was 0 in prev directed DATA / MGMT
	UCHAR	RA[6];			// Copied from TA of Directed DATA, MGMT, PS-POLL
	ULONG	FCS;
} s80211_ACK;
//	PS-Poll (Power Save Poll) Frame Format
//	All Stations update their NAV (Network Allocation Vector) settings
typedef struct _s80211_PSPoll {
	UCHAR	FrameControl[2];
	USHORT	AID;		// The ID of the Association (bits 15-16 set to 1)
	UCHAR	BSSID[6];	// The MAC Address of the AP-STA
	UCHAR	TA[6];
	ULONG	FCS;
} s80211_PSPoll;
//	CF-End (Contention Free-End) Frame Format
typedef struct _s80211_CFEnd {
	UCHAR	FrameControl[2];
	USHORT	DurationID;	// Set to 0
	UCHAR	RA[6];
	UCHAR	BSSID[6];		// The MAC Address of the AP-STA
	ULONG	FCS;
} s80211_CFEnd;
//	CF-End + CF-Ack Frame Format
typedef struct _s80211_CFEndCFAck {
	UCHAR	FrameControl[2];
	USHORT	DurationID;	// Set to 0
	UCHAR	RA[6];
	UCHAR	BSSID[6];		// The MAC Address of the AP-STA
	ULONG	FCS;
} s80211_CFEndCFAck;



/////////////////////////////////////////////////////////////////////////////
//	802.11 Management Frame formats
//
// Information Elements (used below)
typedef struct _sElementFormat {
	UCHAR ElementID;
	UCHAR Length;
	UCHAR Information[256];	// a pointer because it is a variable length field
} sElementFormat;
typedef struct _sFHParameterSet {
	UCHAR	ElementID;
	UCHAR	Length;
	USHORT	DwellTime;		// Kms
	UCHAR	HopSet;
	UCHAR	HopPattern;
	UCHAR	HopIndex;
} sFHParameterSet;
typedef struct _sDSParameterSet {
	UCHAR	ElementID;
	UCHAR	Length;
	UCHAR	CurrentChannel;
} sDSParameterSet;
typedef struct _sCFParameterSet {
	UCHAR	ElementID;
	UCHAR	Length;
	UCHAR	Count;
	UCHAR	Period;
	USHORT	MaxDuration;	// Kms
	USHORT	DurRemaining;	// Kms
} sCFParameterSet;
typedef struct _sTIM {
	UCHAR	ElementID;
	UCHAR	Length;
	UCHAR	Count;
	UCHAR	Period;
	UCHAR	BitmapControl;
	UCHAR*	PartialVirtualBitmap;
} sTIM;
typedef struct _sIBSSParameterSet {
	UCHAR	ElementID;
	UCHAR	Length;
	USHORT	ATIMWindowLength;	// Kms
} sIBSSParameterSet;

//	General Management Frame Format
typedef struct _s80211_MgmtFrm {
	UCHAR	FrameControl[2];
	USHORT	DurationID;
//	during the Contention Free period Set to 32768
//	During the Contention period:
//		if DA is multicast set to 0
//		if DA is unicast:
//			if MORE_FRAG bit is 0 set to ACK + SIFS (msec)
//			else set to DurationOfNextFrag + 2*ACK + 3*SIFS (msec)
//	All stations process DurationID<32768 to update NAV according to CF rules.
	UCHAR	uDA[6];
	UCHAR	uSA[6];
	UCHAR	uBSSID[6];
	USHORT	SequenceControl;
	UCHAR	Body[1508];
	ULONG	FCS;
} s80211_MgmtFrm;

//	Beacon Frame Body
typedef struct _s80211_MGMT_BEACON {
	UCHAR				Timestamp[8];		// TSFTIMER
	USHORT				BeaconInterval;		// Kms between TBTTs (Target Beacon Transmission Times)
	USHORT				CapabilityInformation;
	sElementFormat		SSID;				// Service Set Identity
	sElementFormat		SupportedRates;
	sFHParameterSet		FHParameterSet;		// only present by STA using Frequency Hopping Physical Layers
	sDSParameterSet		DSParameterSet;		// only present by STA using Direct Sequence Physical Layers
	sCFParameterSet		CFParameterSet;		// only present by AP supporting PCF
	sIBSSParameterSet	IBSSParameterSet;	// only present by STA in IBSS
	sTIM				TIM;				// only present by APs
} s80211_MGMT_BEACON;

//	IBSS ATIM (Announcement Traffic Indication Message) Frame Body
//typedef struct _s80211_MGMT_ATIM {
//	// Null Body
//} s80211_MGMT_ATIM;

//	Disassociation Frame Body
typedef struct _s80211_MGMT_DISASSOSIATION {
	USHORT ReasonCode;
} s80211_MGMT_DISASSOSIATION;

//	Association Request Frame Body
typedef struct _s80211_MgmtBody_AssRqst {
	USHORT	CapabilityInformation;
	USHORT	ListenInterval;		// MIB's aListenInterval in Kms
//	Used by AP to determine the lifetime of frames it buffers for a STA
	sElementFormat SSID;		// Service Set Identity
	sElementFormat SupportedRates;
} s80211_MgmtBody_AssRqst;

//	Association Response Frame Body
typedef struct _s80211_MgmtBody_AssRspns {
	USHORT CapabilityInformation;
	USHORT StatusCode;
	USHORT AssociationID;	
	// 1-2007, b15-16 set to 1, 0 used to announce multicast frames in ATIM
	sElementFormat SupportedRates;
} s80211_MgmtBody_AssRspns;

//	Reassociation Request Frame Body
typedef struct _s80211_MGMT_REASS_REQUEST {
	USHORT CapabilityInformation;
	USHORT ListenInterval;		// MIB's aListenInterval in Kms
//	Used by AP to determine the lifetime of frames it buffers for a STA
	UCHAR CurrentAPAddress[6];
	sElementFormat SSID;	// Service Set Identity
	sElementFormat SupportedRates;
} s80211_MGMT_REASS_REQUEST;

//	Reassociation Response Frame Body
typedef struct _s80211_MGMT_REASS_RESPONSE {
	USHORT CapabilityInformation;
	USHORT StatusCode;
	USHORT AssociationID;	
	// 1-2007, b15-16 set to 1, 0 used to announce multicast frames in ATIM
	sElementFormat SupportedRates;
} s80211_MGMT_REASS_RESPONSE;

//	Probe Request Frame Body
typedef struct _s80211_MGMT_PRB_REQUEST {
	sElementFormat SSID;	// Service Set Identity
	sElementFormat SupportedRates;
} s80211_MGMT_PRB_REQUEST;

//	Probe Response Frame Body
typedef struct _s80211_MGMT_PRB_RESPONSE {
	UCHAR	Timestamp[8];		// TSFTIMER
	USHORT	BeaconInterval;	// Kms between TBTTs (Target Beacon Transmission Times)
	USHORT	CapabilityInformation;
	sElementFormat SSID;	// Service Set Identity
	sElementFormat SupportedRates;
	sFHParameterSet FHParameterSet;		// only present by STA using Frequency Hopping Physical Layers
	sDSParameterSet DSParameterSet;		// only present by STA using Direct Sequence Physical Layers
	sCFParameterSet CFParameterSet;		// only present by AP supporting PCF
	sIBSSParameterSet IBSSParameterSet;	// only present by STA in IBSS
} s80211_MGMT_PRB_RESPONSE;

//	Authentication Frame Body
typedef struct _s80211_MgmtBody_Authentication {
	USHORT AuthenticationAlgorithmNumber;
#define C80211_MGMT_AAN_OPENSYSTEM		0x0000
#define C80211_MGMT_AAN_SHAREDKEY		0x0001
	USHORT AuthenticationTransactionSequenceNumber;
	USHORT StatusCode;				// see 802.11 p.57
	sElementFormat ChallengeText;	// see 802.11 p.57
} s80211_MgmtBody_Authentication;

//	Deauthentication Frame Body
typedef struct _s80211_MGMTBody_DEAUTHENTICATION {
	USHORT ReasonCode;
} s80211_MGMTBody_DEAUTHENTICATION;

//	Authentication Algorithm Number constants
#define C80211_MGMT_AAN_OpenSystem				0x0000
#define C80211_MGMT_AAN_SharedKey				0x0001

//	CapabilityInformation constants
#define C80211_MGMT_CAPABILITY_ESS				0x0001	// see 802.11 p.58
#define C80211_MGMT_CAPABILITY_IBSS				0x0002	//      - " -
#define C80211_MGMT_CAPABILITY_CFPollable		0x0004	//      - " -
#define C80211_MGMT_CAPABILITY_CFPollRequest	0x0008	//      - " -
#define C80211_MGMT_CAPABILITY_Privacy			0x0010	//      - " -

#define C80211_MGMT_CAPABILITY_ShortPreamble	0x0020

//	ReasonCode constant, see 802.11 p.60
#define C80211_MGMT_RC_Unspecified			1
#define C80211_MGMT_RC_AuthPrevNoValid		2
#define C80211_MGMT_RC_DeAuthLeaving		3
#define C80211_MGMT_RC_DisassInactivity		4
#define C80211_MGMT_RC_DisassAPHandle		5
#define C80211_MGMT_RC_AuthClass2Frm		6
#define C80211_MGMT_RC_AssClass3Frm			7
#define C80211_MGMT_RC_DisassLeaving		8
#define C80211_MGMT_RC_AssNotAuth			9

//	StatusCode constants, see 802.11 p.62
#define C80211_MGMT_SC_Success				0
#define C80211_MGMT_SC_Unspecified			1
#define C80211_MGMT_SC_SupportCapabilities	10
#define C80211_MGMT_SC_ReassDenied			11
#define C80211_MGMT_SC_AssDenied			12
#define C80211_MGMT_SC_AuthAlgNotSupported	13
#define C80211_MGMT_SC_AuthTransSeqNumError	14
#define C80211_MGMT_SC_AuthRejectChallenge	15
#define C80211_MGMT_SC_AuthRejectTimeout	16
#define C80211_MGMT_SC_AssDeniedHandleAP	17
#define C80211_MGMT_SC_AssDeniedBSSRate		18

//	sElementFormat ElementID
#define C80211_MGMT_ElementID_SSID				0
//	Length 0-32, 0 indicates the broadcast SSID
#define C80211_MGMT_ElementID_SupportedRates	1
//	Length 1-8, used by STAs to avoid association
#define C80211_MGMT_ElementID_FHParameterSet	2
//	Length 5
#define C80211_MGMT_ElementID_DSParameterSet	3
#define C80211_MGMT_ElementID_CFParameterSet	4
#define C80211_MGMT_ElementID_TIM				5
#define C80211_MGMT_ElementID_IBSSParameterSet	6
#define C80211_MGMT_ElementID_ChallengeText		16

#endif
