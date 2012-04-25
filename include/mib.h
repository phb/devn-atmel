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


#ifndef __mib_h_OK__
#define __mib_h_OK__

#define	DEF_WEP_SIZE		4
#define	WEP_MAP_SIZE		10
#define	OPER_RATE_SIZE		4
#define	NUM_GROUP_ADDRESSES	4

#define MIB_MacAddr					0
#define MIB_ChannelID				8
#define MIB_RtsThreshold			12
#define MIB_ShortRetryLimit			14
#define MIB_TxRate					16
#define MIB_Promiscous				17


#define MIB_CHANNEL				0x00
#define MIB_RAND_SEED			0x04
#define MIB_MAC_ADDR			0x0c
#define MIB_BSS_ID				0x12


#define	Local_Mib_Type				0x01
#define	Mac_Address_Mib_Type		0x02
#define	Mac_Mib_Type				0x03
#define	Statistics_Mib_Type			0x04
#define	Mac_Mgmt_Mib_Type			0x05
#define	Mac_Wep_Mib_Type			0x06
#define	Phy_Mib_Type				0x07
#define Multi_Domain_MIB			0x08

typedef struct __MAC_MIB
{
	ULONG	MaxTransmitMSDULifeTime;	// 1..0xFFFFFFFF
	ULONG	MaxReceiveLifeTime;			// 1..0xFFFFFFFF	
	USHORT	FragmentationThreshold;		// 256..2346
	USHORT	RtsThreshold;
	
	USHORT	CwMin;						// 31
	USHORT	CwMax;						// 1023

	UCHAR	ShortRetryLimit;			// Retries for packets without RTS/CTS. Default = 7
	UCHAR	LongRetryLimit;				// Retries for packets with RTS/CTS. Default = 4
	UCHAR	ScanType;
	UCHAR   ScanChannel; 
	
	USHORT  ProbeDelay;
	USHORT  MinChannelTime;
	USHORT	MaxChannelTime;

	UCHAR	DesiredSSID[MAX_SSID_LENGTH];
	MACADDR	DesiredBSSID;
	UCHAR	DesiredBSSType;
	UCHAR	Reserved[3];
} MAC_MIB;

#define MAC_MIB_FRAG_THRESHOLD_POS					8
#define MAC_MIB_RTS_THRESHOLD_POS					10
#define MAC_MIB_SHORT_RETRY_LIMIT_POS				16



typedef struct __MAC_MGMT_MIB
{
	USHORT	BeaconPeriod;				// 1..0xFFFF
	USHORT	CFPMaxDuration;				// 0..0xFFFF

	USHORT	MediumOccupancyLimit;		// 0..1000 def 100. 
	USHORT	StationID;

	USHORT	ATIMWindow;
	UCHAR	CFPMode;					
	BOOLEAN	PrivacyOptionImplemented;	// FALSE
	UCHAR	DTIMPeriod;					// 1..255
	UCHAR	CFPPeriod;					// 0..255						
	MACADDR	CurrentBSSID;
	UCHAR	CurrentSSID[MAX_SSID_LENGTH];
	UCHAR	CurrentBSSType;
	UCHAR	Power_Mgmt_Mode;			
	UCHAR	ListenInterval[2];		
#ifdef INT_ROAM
	BOOLEAN	MutiDomainCapabilityImplemented;
	BOOLEAN	MultiDomainCapabilityEnabled;
	UCHAR	CountryString[3];
	UCHAR	Reserved[3];
#endif
} MAC_MGMT_MIB;

#define MAC_MGMT_MIB_BEACON_PER_POS				0
#define MAC_MGMT_MIB_STATION_ID_POS				6
#define MAC_MGMT_MIB_CUR_PRIVACY_POS			11
#define MAC_MGMT_MIB_CUR_BSSID_POS				14
#define MAC_MGMT_MIB_PS_MODE_POS				53
#define	MAC_MGMT_MIB_LISTEN_INTERVAL_POS		54
#define	MAC_MGMT_MIB_MULTI_DOMAIN_IMPLEMENTED	56
#define	MAC_MGMT_MIB_MULTI_DOMAIN_ENABLED		57

#if (defined RFMD) || (defined RFMD_3COM)

//definitions for CurrentCCAMode
#define ED_ONLY		1
#define CS_ONLY		2
#define ED_AND_CS	4	

//definitions for PHYType
#define FHSS		1
#define DSSS		2
#define IRBASEBAND	3

//definitions for Current Reg Domain
#define FCC			0x10
#define DOC			0x20
#define ETSI		0x30
#define SPAIN		0x31
#define FRANCE		0x32
#define MKK			0x40
#define MKK1			0x41
#define ISRAEL			0x50

typedef struct __PHY_MIB
{	
	ULONG	EDThreshold;
	USHORT	SlotTime;				// 20us
	USHORT	SifsTime;				// 10us
	USHORT	PreampleLength;			// 144 us
	USHORT	PLCPHeaderLength;		// 48us
	USHORT	MPDUMaxLength;			// 4<= x <= (2^13)-1
	USHORT	ChannelID;				// 1..14 
	ULONG	CCAModeSupported;		// 1..7
	UCHAR	OperationalRateSet[OPER_RATE_SIZE];						
	UCHAR	CurrentCCAMode;
	UCHAR	PHYType;
	UCHAR	CurrentRegDomain;
} PHY_MIB;

#define PHY_MIB_CHANNEL_POS					14
#define PHY_MIB_RATE_SET_POS				20
#define PHY_MIB_REG_DOMAIN_POS				26


#else

// 10-10-01
// Channel changed to USHORT from UCHAR and CCAModeSupported changed to UCHAR for alligment purposes
// These changes fix the alligment problem without affecting the f/w-driver interface.

typedef struct __PHY_MIB
{	
	ULONG	EDThreshold;
	USHORT	SlotTime;				// 20us
	USHORT	SifsTime;				// 10us
	USHORT	PreampleLength;			// 144 us
	USHORT	PLCPHeaderLength;		// 48us
	USHORT	MPDUMaxLength;			// 4<= x <= (2^13)-1
	USHORT	ChannelID;				// 1..14
	UCHAR	CCAModeSupported;		// 1..7
	UCHAR	OperationalRateSet[OPER_RATE_SIZE];			// 			
	enum	{ED_ONLY=1,CS_ONLY=2,ED_AND_CS=4}	CurrentCCAMode;	
	enum	{FHSS=1,DSSS=2,IRBASEBAND=3}	PHYType;
	enum	{FCC=0x10,DOC=0x20,ETSI=0x30,SPAIN=0x31,FRANCE=0x32,MKK=0x40,MKK1=0x41,ISRAEL=0x50}	CurrentRegDomain;
} PHY_MIB;

#define PHY_MIB_CHANNEL_POS					14
#define PHY_MIB_RATE_SET_POS				17


#endif

typedef struct __LOCAL_MIB
{

	USHORT	TimeOutDelay;
	BOOLEAN	BeaconEnable;
	UCHAR	AutoTxRate;
	BOOLEAN	IsAP;
	UCHAR	SSID_Size;
	UCHAR   PromiscuousMode;
	UCHAR	BasicRate;
	UCHAR	TxControlRate;
	UCHAR	PreambleType;
	UCHAR   Reserved[2];

} LOCAL_MIB;	
	

#define LOCAL_MIB_AUTO_TX_RATE_POS				3
#define LOCAL_MIB_SSID_SIZE						5
#define LOCAL_MIB_TX_PROMISCUOUS_POS			6
#define LOCAL_MIB_TX_MGMT_RATE_POS				7
#define LOCAL_MIB_TX_CONTROL_RATE_POS			8
#define	LOCAL_MIB_PREAMBLE_TYPE					9

typedef struct __MAC_ADDRESS_MIB
{
	MACADDR	MACAddress;									// Unique MAC address assigned to the STA.
	MACADDR	GroupAddresses[NUM_GROUP_ADDRESSES];		// Multicast address from which the STA will receive frames
	UCHAR	GroupAddressesStatus[NUM_GROUP_ADDRESSES];	// The status column for the above addresses.
	UCHAR	res[3];
} MAC_ADDRESS_MIB;

#define MAC_ADDR_MIB_MAC_ADDR_POS					0


#ifdef ATMEL_WLAN
#define MAX_ENCRYPTION_KEYS                             4
#define MAX_ENCRYPTION_KEY_SIZE                         40
#define ENCRYPTION_KEY_RSC_BYTES                        (MAX_ENCRYPTION_KEYS*MAX_ENCRYPTION_KEY_SIZE) +20
#define RECEIVER_ADDRESS_OFFSET                         MAX_ENCRYPTION_KEYS*MAX_ENCRYPTION_KEY_SIZE
#define ENCRYPTION_PARAMETERS_BYTES                     11
#endif	//ATMEL_WLAN



#ifdef ATMEL_WLAN
typedef struct __MAC_ENCRYPTION_MIB
{
 UCHAR   CipherDefaultKeyValue[MAX_ENCRYPTION_KEYS][MAX_ENCRYPTION_KEY_SIZE];
 UCHAR   ReceiverAddress[6];
 BOOLEAN PrivacyInvoked;
 UCHAR   CipherDefaultKeyID;
 UCHAR   CipherDefaultGroupKeyID;
 UCHAR   ExcludeUnencrypted;
 UCHAR   EncryptionType;
 UCHAR   reserved;
 ULONG   WEPICVErrorCount;
 ULONG   WEPExcludedCount;
 UCHAR   KeyRSC[4][8];

} MAC_ENCRYPTION_MIB;

#else
typedef struct __MAC_WEP_MIB
{
	BOOLEAN	PrivacyInvoked;			
	UCHAR	WEPDefaultKeyID;		// 0..3
	UCHAR	reserved;
	UCHAR	ExcludeUnencrypted;

	ULONG	WEPICVErrorCount;
	ULONG	WEPExcludedCount;

	UCHAR	WEPDefaultKeyValue[4][13];
#if (defined RFMD) || (defined RFMD_3COM)
	UCHAR	EncryptionLevel;
	UCHAR	Reserved[3];
#endif

} MAC_WEP_MIB;	
#endif	//ATMEL_WLAN

#define WEP_MIB_PRIVACY_POS					0
#define WEP_MIB_DEF_KEY_POS					1
#define WEP_MIB_EXCLUDE_UNENCR_POS			3
#define WEP_MIB_DEF_KEY_VAL_POS				12
#define WEP_MIB_ENCRYPTION_LEVEL			64


#define CIPHER_SUITE_NONE 0
#define CIPHER_SUITE_WEP_64 1
#define CIPHER_SUITE_TKIP 2
#define CIPHER_SUITE_AES  3
#define CIPHER_SUITE_CCX  4
#define CIPHER_SUITE_WEP_128  5


#ifdef INT_ROAM

typedef struct __MDOMAIN_MIB
{
    ULONG	DomainInfoValid;
    UCHAR	TxPowerLevel[14];
    UCHAR	ChannelList[14];
} MDOMAIN_MIB;

#endif //INT_ROAM

typedef struct __STATISTICS_MIB
{
	ULONG  UnicastPacketsTx;
	ULONG  BroadcastPacketsTx;
	ULONG  MulticastPacketsTx;
	ULONG  BeaconsTx;
	ULONG  AckPacketsTx;
	ULONG  RTSPacketsTx;
	ULONG  CTSPacketsTx;
	// Rx Packets
	ULONG  UnicastPacketsRx;
	ULONG  BroadcastPacketsRx;
	ULONG  MulticastPacketsRx;
	ULONG  BeaconsRx;
	ULONG  AckPacketsRx;
	ULONG  RTSPacketsRx;
	ULONG  CTSPacketsRx;
	// failure
	ULONG	ACKFailureCount;
	ULONG	CTSFailureCount;
} STATISTICS_MIB;

#endif
