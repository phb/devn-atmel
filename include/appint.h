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


#ifndef __appint_h_OK__
#define __appint_h_OK__

#include "vnet.h"

#define SHORT_WEP_KEY_SIZE				5
#define LONG_WEP_KEY_SIZE				13

#define WEP_DISABLED					0
#define WEP_64bit						1
#define WEP_128bit						2

///////////////////////////////////////////////////////////////////////////
//	SUPPORT_APP_INFO 
///////////////////////////////////////////////////////////////////////////

//
//	This structure is exchanged betwwen the driver and the 
//	monitor application. 
//

typedef struct __DEVICE_CONFIGURATION
{
	UCHAR			OperatingMode;					//AD_HOC_MODE or INFRASTRUCTURE_MODE
	UCHAR			Channel;						
	UCHAR			SSID[MAX_SSID_LENGTH];			
	UCHAR			SSIDlength;
	UCHAR			TxRate;							//0-4
	
        UCHAR                   PowerMgmtMode;
        UCHAR                   InternationalRoaming;
	UCHAR			RadioIsOn;					// Radio Is On - Off

	UCHAR			PreambleType;					// LONG_PREAMBLE or SHORT_PREAMBLE
	
	USHORT			FragmentationThreshold;			
	USHORT			RtsCtsThreshold;

	UCHAR			BSSID[6];

	UCHAR			StationState;
	CHAR			Rssi;
	UCHAR			LinkQuality;
        UCHAR                   RegDomain;
        UCHAR                   ChannelVector[14];
	ULONG			MgmtErrorCode;			


}DEVICE_CONFIGURATION, *PDEVICE_CONFIGURATION;


// For the MgmtErrorCode we use not only the standard reason codes defined in 802.11 but 
// also the following:

#define MGMT_ERROR_WEP_REQUIRED				0xa0
#define MGMT_ERROR_WEP_NOT_REQUIRED			0xa1

//
// VERSION_INFO exchanged between driver/application 
//
typedef struct __VERSION_INFO{
	UCHAR	DriverMajorVersion;
	UCHAR	DriverMinorVersion;
	UCHAR	DriverSubVersion;
	USHORT	DriverBuild;

	USHORT	FwMajorVersion;
	USHORT	FwMinorVersion;
	USHORT	FwSubVersion;
	USHORT	FwBuild;

}VERSION_INFO;

//
// WEP_INFO is exchanged between driver/application 
//


#define MAX_ENCRYPTION_KEYS				4
#define MAX_ENCRYPTION_KEY_SIZE				40

typedef struct __WEP_INFO
{
	UCHAR						WepKeyToUse;
	UCHAR						WepMode;
	USHORT						AuthenticationType;
	UCHAR						EncryptionLevel;					//0:Disabled,1:64bit,2:128 bit
#ifdef ATMEL_WLAN
  UCHAR	GroupKeyIndex;
  UCHAR	PairwiseKeyIndex;
  UCHAR   PairwiseKeyLength;
  UCHAR	GroupKeyLength;
  UCHAR	PairwiseCipherSuite;
  UCHAR	GroupCipherSuite;
  UCHAR   BSSID[6];
  UCHAR	KeyIndex;
  UCHAR	KeyLength;
  UCHAR	SetKey;
  UCHAR	KeyMaterial[MAX_ENCRYPTION_KEYS][MAX_ENCRYPTION_KEY_SIZE];
  UCHAR	KeyRSC[MAX_ENCRYPTION_KEYS][8];
#else
	UCHAR						WepKey1[LONG_WEP_KEY_SIZE];
	UCHAR						WepKey2[LONG_WEP_KEY_SIZE];
	UCHAR						WepKey3[LONG_WEP_KEY_SIZE];
	UCHAR						WepKey4[LONG_WEP_KEY_SIZE];
#endif
} WEP_INFO;


#define MAX_BSS_ENTRIES		64	

typedef struct __BSS_INFO{
	UCHAR	Channel;
	UCHAR	SSID[MAX_SSID_LENGTH];
	UCHAR	SSIDsize;
	UCHAR	BSSID[6];
	CHAR	RSSI;
	UCHAR	UsingWEP;
	UCHAR	PreambleType;
	USHORT  BeaconPeriod;
	UCHAR	BSStype;
}BSS_INFO;

typedef struct __IBSS_INFO{
	UCHAR	MACaddress[6];
}IBSS_INFO;


typedef struct __STATISTICS{
	ULONG	TxDataPacketsOk;
	ULONG	TxDataPacketsError;
	ULONG	TxMgmtPacketsOk;
	ULONG	TxMgmtPacketsError;
	ULONG	RxDataPacketsOk;
	ULONG	RxDataPacketsError;
	ULONG	RxMgmtPacketsOk;
	ULONG	RxMgmtPacketsError;
	ULONG	RxLost;
	ULONG	TxPacketsRejectedNotReady;
	ULONG	TxPacketsRejectedResources;
	ULONG	MatchingBeacons;
	

}STATISTICS;

typedef UCHAR VALID_CHANNELS[14];

#define SITE_SURVEY_IDLE				0
#define SITE_SURVEY_IN_PROGRESS			1
#define SITE_SURVEY_COMPLETED			2

#endif
