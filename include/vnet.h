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

#ifndef __VNet_h_OK__
#define __VNet_h_OK__

#define MAX_SSID_LENGTH 							32

#include <stdint.h> //hackish
typedef  char				CHAR;
typedef  uint8_t   	UCHAR;
typedef  uint16_t  	USHORT;
typedef  unsigned long   	ULONG;
typedef  uint32_t    	UINT;

typedef  char * 				PCHAR;
typedef  uint8_t * 		PUCHAR;
typedef  uint16_t *		PUSHORT;
typedef  unsigned long * 		PULONG;
typedef  char	 		BOOLEAN;
typedef void			VOID;
typedef  void *			PVOID;

#ifdef VARIANT_REV_D
#define INT_ROAM 1
#define REV_D 1
#define RFMD 1
#elif defined (VARIANT_REV_E)
#define REV_E 1
#define INT_ROAM 1
#define RFMD 1
#elif defined (VARIANT_R504)
#define R504 1
#define RFMD 1
#elif defined (VARIANT_R504_2958)
#define R504_2958 1
#define RFMD 1
#elif defined (VARIANT_R504A_2958)
#define R504A_2958 1
#define RFMD 1
#elif defined (VARIANT_PCI) 
#define PCI 1
#define RFMD 1
#elif defined (VARIANT_RFMD)
#define INT_ROAM 1
#define RFMD 1
#elif defined (VARIANT_RFMD_3COM)
#define RFMD_3COM 1
#endif


#include "debug.h"
#include "appint.h"
#include "at76c502.h"
#include "frame.h"

#include <time.h> 
#include <signal.h>
#define FAST_VNET_MAJOR_VERSION 		1
#define FAST_VNET_MINOR_VERSION 		1
#define FAST_VNET_SUB_VERSION			0
#define FAST_VNET_BUILD 			0

#define PRODUCT_NAME						"PCMCIA Wireless Network Adapter"
#define MAX_DRIVER_DESC_LENGTH				100

#ifdef PCI
#define ATMEL_PCI_VENDOR_ID 			0x1114
#define ATMEL_PCI_DEVICE_ID 			0x0506
#endif

#define AUTHENTICATION_RESPONSE_TIME_OUT	1000

///////////////////////////////////////////////////////////////////////////
// General definitions
///////////////////////////////////////////////////////////////////////////


#define VNet_MAX_FRAME_SIZE 				1500
#define VNet_HEADER_SIZE					14
#define MAX_WIRELESS_FRAME_SIZE 			1550 // 1500 + 14 + 18 + ...
#define VNet_MAX_LOOKAHEAD					VNet_MAX_FRAME_SIZE 
#define VNet_LENGTH_OF_ADDRESS				6
#define DEFAULT_MULTICASTLISTMAX			32

#ifdef SNIFFER
#define MAX_RX_PACKET_SIZE					2000
#endif

#define LOOP_RETRY_LIMIT					500000

#define PERIODIC_TIMER_TOUT 				2000

#define MAX_DEVICE_ID_LENGTH				50

#define ACTIVE_MODE     1
#define PS_MODE                 2
#ifndef RFMD_3COM
#define SPS_MODE                3
#endif


#define MIN_BEACON_PROBE_BYTES				60

///////////////////////////////////////////////////////////////////////////
// 802.11 related definitions
///////////////////////////////////////////////////////////////////////////

//
// Regulatory Domains
//

#define REG_DOMAIN_FCC		0x10	//Channels	1-11	USA
#define REG_DOMAIN_DOC		0x20	//Channel	1-11	Canada
#define REG_DOMAIN_ETSI		0x30	//Channel	1-13	Europe (ex Spain/France)
#define REG_DOMAIN_SPAIN	0x31	//Channel	10-11	Spain
#define REG_DOMAIN_FRANCE	0x32	//Channel	10-13	France
#define REG_DOMAIN_MKK		0x40	//Channel	14		Japan
#define REG_DOMAIN_MKK1		0x41	//Channel	1-14	Japan(MKK1)
//#define REG_DOMAIN_ISRAEL	0x50	//Channel	3-9	ISRAEL


#define AD_HOC_MODE 						0
#define INFRASTRUCTURE_MODE 				1
#define AUTO_MODE							2

#define BSS_TYPE_AD_HOC 					1
#define BSS_TYPE_INFRASTRUCTURE 			2

#define SCAN_TYPE_ACTIVE					0
#define SCAN_TYPE_PASSIVE					1


#define LONG_PREAMBLE						0
#define SHORT_PREAMBLE						1
#define AUTO_PREAMBLE						2

#define AUTO_SWITCH_AUTH_ALGORITHM			2

// WEP key values
#define WEP_DISABLED                    0
#define WEP_64BIT                               1
#define WEP_128BIT                              2

#define WEP_MODE_OPTIONAL					0
#define WEP_MODE_MANDATORY					1


#define LISTEN_INTERVAL 					200

#define DATA_FRAME_WS_HEADER_SIZE			30


#define CTRL_FRAMES_TX_RATE 				1
#define MGMT_FRAMES_TX_RATE 				1

#define PROM_MODE_OFF						0x0
#define PROM_MODE_UNKNOWN					0x1
#define PROM_MODE_CRC_FAILED				0x2
#define PROM_MODE_DUPLICATED				0x4
#define PROM_MODE_MGMT						0x8
#define PROM_MODE_CTRL						0x10
#define PROM_MODE_BAD_PROTOCOL				0x20


#define CARD_TYPE_PARALLEL_FLASH			0x1
#define CARD_TYPE_SPI_FLASH 				0x2
#define CARD_TYPE_EEPROM					0x3



///////////////////////////////////////////////////////////////////////////
// FLASH programming related definitions
///////////////////////////////////////////////////////////////////////////

#define BANK_SIZE							0x10000

#define ATMEL_ID							0x1f
#define ST_ID								0x20

#define ST_M29W010B 						0x23 

#define ATMEL_49LV002						0x07
#define ATMEL_49LV002T						0x08
#define ATMEL_49LV001						0x05
#define ATMEL_49LV001T						0x04
#define ATMEL_29LV512						0x3d
#define ATMEL_29LV010						0x35
#define ATMEL_29LV020						0xba
#define ATMEL_29LV040						0xc4


#define CIS_BYTES							1024
#define HW_CONFIG_BYTES 					0x700
#define HW_CONFIG_OFFSET					0x1C000
///////////////////////////////////////////////////////////////////////////
// type definitions
///////////////////////////////////////////////////////////////////////////

typedef UCHAR MACADDR[VNet_LENGTH_OF_ADDRESS];


///////////////////////////////////////////////////////////////////////////
//	IFACE
///////////////////////////////////////////////////////////////////////////

//
// interface host driver/firmware structure IFACE
//
typedef struct __IFACE
{
	UCHAR	volatile IntStatus;
	UCHAR	volatile IntMask;
	UCHAR	volatile LockoutHost;
	UCHAR	volatile LockoutMac;

	USHORT	TxBuffPos;
	USHORT	TxBuffSize;
	USHORT	TxDescPos;
	USHORT	TxDescCount;

	USHORT	RxBuffPos;
	USHORT	RxBuffSize;
	USHORT	RxDescPos;
	USHORT	RxDescCount;

	USHORT	BuildVer;
	USHORT	CommandPos; 	

	USHORT	MajorVer;
	USHORT	MinorVer;

	USHORT	FuncCtrl;
	USHORT	MacStatusPos;	//This field is undocumented for ATMEL use ONLY
#if (defined RFMD) || (defined RFMD_3COM)
	USHORT  GenericIRQType;
	UCHAR	Reserved[2];
#endif
} IFACE, *PIFACE;

#define IFACE_INT_STATUS_OFFSET 		0
#define IFACE_INT_MASK_OFFSET			1
#define IFACE_LOCKOUT_HOST_OFFSET		2
#define IFACE_LOCKOUT_MAC_OFFSET		3
#define IFACE_FUNC_CTRL_OFFSET			28
#define IFACE_MAC_STAT_OFFSET			30
#define IFACE_GENERIC_INT_TYPE_OFFSET	32
//
// IFACE MACROS & definitions
//
//

// FuncCtrl field: 
//
#define FUNC_CTRL_TxENABLE					(UCHAR)0x10
#define FUNC_CTRL_RxENABLE					(UCHAR)0x20
#define FUNC_CTRL_INIT_COMPLETE 			(UCHAR)0x01




//
// Station States definitions
//
#define STATION_STATE_INITIALIZING			0x00
#define STATION_STATE_SCANNING				0x01
#define STATION_STATE_JOINNING				0x02
#define STATION_STATE_AUTHENTICATING		0x03
#define STATION_STATE_ASSOCIATING			0x04
#define STATION_STATE_READY 				0x05
#define STATION_STATE_OUT_OF_RANGE			0x06
#define STATION_STATE_DISASSOCIATING		0x07
#define STATION_STATE_DIAUTHENTICATING		0x08
#define STATION_STATE_REASSOCIATING 		0x09
#define STATION_STATE_FORCED_JOINNING		0x0a
#define STATION_STATE_RECONFIGURING 		0x0b
#define STATION_STATE_FW_UPGRADE			0x0c
#define STATION_STATE_FORCED_JOIN_FAILURE	0x0d
#define STATION_STATE_RESETTING 			0x0e
#define STATION_STATE_RESET_PENDING 		0x0f
#define STATION_STATE_SNIFFER_MODE			0x10
#define STATION_STATE_WAIT_ACTION			0x11 //under XP we need this state to allow XP to initiate a new scan
#define STATION_STATE_TEST_MODE 			0x12
#define STATION_STATE_RADIO_OFF 			0x13

//errors: error code field is valid
#define STATION_STATE_NO_CARD				0xf0
#define STATION_STATE_MGMT_ERROR			0xf1


//
// Regulatory Domains
//

#define REG_DOMAIN_FCC		0x10	//Channels	1-11
#define REG_DOMAIN_DOC		0x20	//Channel	11
#define REG_DOMAIN_ETSI 	0x30	//Channel	1-13
#define REG_DOMAIN_SPAIN	0x31	//Channel	10-11
#define REG_DOMAIN_FRANCE	0x32	//Channel	10-13
#define REG_DOMAIN_MKK		0x40	//Channel	14
#define REG_DOMAIN_MKK1 	0x41	//Channel	1-14
#define REG_DOMAIN_ISRAEL 	0x50	//Channel	3-7

#ifdef RX_CRC
////////////////////////////////////////////////////////////////////////////
//
// The FRAGMENTED_PACKET structure is used for the reassembly of the
// fragmented 802.11 packets in the driver
//
////////////////////////////////////////////////////////////////////////////
typedef struct __FRAGMENTED_PACKET{
	USHORT	Sequence;
	USHORT	CurrentLen;
	UCHAR	SourceAddress[6];
	UCHAR	FragmentNumber;
	UCHAR	WirelessPacket[MAX_WIRELESS_FRAME_SIZE];
}FRAGMENTED_PACKET, *PFRAGMENTED_PACKET;
#endif

#include "mib.h"

typedef struct __VNet_ADAPTER {
	//
	// I/O 
	//
	UINT						IoBaseAddr;
	ULONG						IoPortsLen;
	ULONG						IoBase;   
	BOOLEAN 					IOregistered;
	//
	// Interrupt
	//
	UCHAR						InterruptStatus;
	UCHAR						InterruptType; 
	UCHAR						InterruptMask;
	//
	// NDIS
	//
	UCHAR						BusType;
	UINT						MulticastListMax;
	ULONG						PacketFilter;
	ULONG						CurrentLookAhead;
	UCHAR						Lookahead[MAX_WIRELESS_FRAME_SIZE];
	UCHAR						PermanentAddress[VNet_LENGTH_OF_ADDRESS];
	UCHAR						CurrentAddress[VNet_LENGTH_OF_ADDRESS];

	//
	// Statistics 
	//
	STATISTICS					Stats;	

	//
	// TX
	//
	USHORT						TxDescFree; 	
	USHORT						TxDescHead;
	USHORT						TxDescTail;
	USHORT						TxDescPrevious;

	USHORT						TxFreeBuffMem;
	USHORT						TxBuffHead;

	USHORT						TxBuffTail;
	USHORT						TxBuffEnd;
	USHORT						TxBuffStart;
	ULONG						Transmissions;
	ULONG						CompletedTransmissions;

	ULONG						PendingTxPackets;
	//
	//	RX
	//
	USHORT						RxDescHead;
	USHORT						RxBuffEnd;
	USHORT						RxBuffStart;
	ULONG						Receptions;
	ULONG						RxPacketLen;
	BOOLEAN 					IndicateReceiveDone;

	UCHAR						RxBuf[MAX_WIRELESS_FRAME_SIZE];
#ifdef RX_CRC
	ULONG						RxCrcError;
	ULONG						CrcTable[256];
	FRAGMENTED_PACKET			FragPacket;
#endif 
	ULONG						MiniportResets;
	//
	// parameters
	//
	UCHAR						Channel;
	UCHAR						OperatingMode;
	USHORT						RtsThreshold;
	USHORT						FragmentationThreshold;
	UCHAR						DataFramesTxRate;
	IFACE						HostInfo;
	USHORT						HostInfoBase;
	UCHAR						PreambleType;
	CHAR						Rssi;
	UCHAR						LinkQuality;
	UCHAR						DriverDesc[MAX_DRIVER_DESC_LENGTH];
	UCHAR						DriverDescSize;
	BOOLEAN 					AutoPreambleDetection;
	BOOLEAN 					AutoSwitchAuthAlgorithm;
	UCHAR						BasicRatesIndex;
	BOOLEAN 					NetAddressOverride;

/////////////////////////////////////////////////
	UCHAR						InitialChannel;
	USHORT						InitialRtsThreshold;
	USHORT						InitialFragmentationThreshold;
	UCHAR						InitialDataFramesTxRate;
	UCHAR						InitialPreambleType;
	UCHAR						InitialPowerMgmtMode;
	BOOLEAN 					InitialRadioIsOn;
	UCHAR						InitialIntRoamingEnabled;

/////////////////////////////////////////////////
	//
	// State/Control vars
	// 
	UCHAR						StationState;
	USHORT						CurrentAuthentTransactionSeqNum;
	USHORT						ExpectedAuthentTransactionSeqNum;	
	BOOLEAN 					StationWasAssociated;
	BOOLEAN 					StationIsAssociated;
	BOOLEAN						MiniportResetRequest;
	BOOLEAN						MiniportResetPending;
	UCHAR						SiteSurveyState;
	UCHAR						CardType;
	UCHAR						IndicateConnectState;
	BOOLEAN 					RejectPendingTxPackets;
	BOOLEAN 					WepIsConfigured;
	BOOLEAN 					ConnectToAnyBSS;
	CHAR						CurrentBSSindex;
	BOOLEAN 					FastScan;
	BOOLEAN 					CmdPending;
	BOOLEAN 					RadioIsOn;
	BOOLEAN 					HwRadioIsOn;

	UCHAR						RegDomainIndex;
	UCHAR						UseWzcs;
	UCHAR						WasUsingWzcs; // when entering non-Net modes we force UseWzcs to 0
											  // and we use WasUsingWzcs to restore it to the last value
	BOOLEAN						InSnifferMode;
	BOOLEAN						DeviceIsUp;

	//
	// 802.11 
	//
	MACADDR 					CurrentBSSID;
	MACADDR 					LastBSSID;
        UCHAR                                           SelectedSSID[MAX_SSID_LENGTH];
	UCHAR						DesiredSSID[MAX_SSID_LENGTH];
	UCHAR						SSID_Size;
	UCHAR						InitialSSIDsize;
	UCHAR						InitialSSID[MAX_SSID_LENGTH];
	UCHAR						BSSID[6];
	UCHAR						AuthenticationRequestRetryCnt;
	UCHAR						AssociationRequestRetryCnt;
	UCHAR						ReAssociationRequestRetryCnt;
	WEP_INFO					WepInfo;
#ifdef ATMEL_WLAN
  	BOOLEAN					        PairwiseKeyAvailable;
	BOOLEAN         				GroupKeyAvailable;
	BOOLEAN         				IsBroadcast;
#endif
	BOOLEAN 					ApSelected;
	UCHAR						PowerMgmtMode;
	USHORT						DefaultBeaconPeriod;
	USHORT						BeaconPeriod;
	USHORT						ListenInterval;
	BSS_INFO					BSSinfo[MAX_BSS_ENTRIES];
	BSS_INFO					BSSinfo2App[MAX_BSS_ENTRIES];
	UCHAR						BSSListEntries;
        UCHAR                                           BSSListEntries2App;
	ULONG						MgmtErrorCode;			
	UCHAR						PeriodicBeaconsCnt;
	UCHAR						IntRoamingEnabled;
//	VALID_CHANNELS				        ValidChannels;
        UCHAR                                           ChannelVector[14];
	UCHAR						RegDomain;
#ifdef INT_ROAM
	MDOMAIN_MIB					MultiDomainMib;
#endif

	//
	//Timers
	//
	timer_t					MgmtTimer;
	void *extra;
	UCHAR						IsUp;
	//
	// application support
	//
	VERSION_INFO				VersionInfo;
	UCHAR						DeviceID[MAX_DEVICE_ID_LENGTH];
	UCHAR						DeviceIDsize;
	//
	// DFU
	//
	UCHAR						VendorID;
	UCHAR						FlashID;
	ULONG						FlashTotalBytes;
	ULONG						FirmwareFileLength;
	PUCHAR						MappedFirmwareBuffer;
	BOOLEAN 					WorkingWithDefaultFW;

	PUCHAR						pCIS;
	PUCHAR						pHardConfig;
	BOOLEAN 					InvalidMACaddress;
	//
	//Debug
	//
} VNet_ADAPTER, * PVNet_ADAPTER;

#include "vnet_qnx.h"

#define VNET_ASSERT(A)	{}
#include "stdinclude.h"
#endif
