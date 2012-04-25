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


#ifndef __command_h_OK__
#define __command_h_OK__

#ifndef ATMEL_WLAN
#define CMD_BLK_PARM_SIZE	76
#define MIB_CMD_DATA_SIZE	72
#else
#define CMD_BLK_PARM_SIZE	216
#define MIB_MAX_DATA_BYTES	212
#endif	//ATMEL_WLAN


//
// Command IDs
//

#define 	CMD_Set_MIB_Vars		0x01
#define 	CMD_Get_MIB_Vars		0x02
#define 	CMD_Scan				0x03
#define 	CMD_Join				0x04
#define 	CMD_Start				0x05
#define 	CMD_EnableRadio 		0x06
#define 	CMD_DisableRadio		0x07
#define 	CMD_SiteSurvey			0x0B
#ifdef	ATMEL_WLAN
#define 	CMD_SynthReprogram		0x0C
#endif

//
//Command Status
//
#define 	CMD_STATUS_IDLE 						0x00
#define 	CMD_STATUS_COMPLETE 					0x01
#define 	CMD_STATUS_UNKNOWN						0x02
#define 	CMD_STATUS_INVALID_PARAMETER			0x03
#define 	CMD_STATUS_FUNCTION_NOT_SUPPORTED		0x04
#define 	CMD_STATUS_TIME_OUT 					0x07
#define 	CMD_STATUS_IN_PROGRESS					0x08
#define		CMD_STATUS_REJECTED_RADIO_OFF			0x09
//
// Host defined command status
//
#define 	CMD_STATUS_HOST_ERROR					0xFF
#define		CMD_STATUS_BUSY							0xFE

typedef struct __CMD_BLOCK_STRUCT{
	UCHAR			Command;
	UCHAR			Status;
	UCHAR			Error_Offset;
	UCHAR			Reserved;
	UCHAR			Parameters[CMD_BLK_PARM_SIZE];

} CMD_BLOCK_STRUCT;

#define CMD_BLOCK_COMMAND_OFFSET		0
#define CMD_BLOCK_STATUS_OFFSET 		1
#define CMD_BLOCK_PARAMETERS_OFFSET 	4

typedef struct __GET_SET_MIB_STRUCT {
	UCHAR	Type;
	UCHAR	Size;
	UCHAR	Index;
	UCHAR	Reserved;
#ifndef ATMEL_WLAN
	UCHAR	Data[72];
#else	
	UCHAR	Data[MIB_MAX_DATA_BYTES];
#endif
	
} GET_SET_MIB_STRUCT;

#ifdef ATMEL_WLAN
#define GET_SET_MIB_DATA_OFFSET			4
#define SCAN_OPTIONS_FAST_SCAN			0x01
#endif	// ATMEL_WLAN

#if defined (RFMD) || defined (RFMD_3COM)

#define SCAN_OPTIONS_SITE_SURVEY				0x80
#define SCAN_OPTIONS_INTERNATIONAL_SCAN			0x40  //INT_ROAM

typedef struct __sSCAN{
	UCHAR			BSSID[6];
	UCHAR			SSID[MAX_SSID_LENGTH];
	UCHAR			ScanType;
	UCHAR			Channel;
	USHORT			BSStype;
	USHORT			MinChannelTime;
	USHORT			MaxChannelTime;
	UCHAR			Options;
	UCHAR			SSID_Size;
}sSCAN;


typedef struct __sSTART{
	UCHAR	BSSID[6];
	UCHAR	SSID[MAX_SSID_LENGTH];
	UCHAR	BSSType;				
	UCHAR	Channel;
	UCHAR	SSID_Size;
	UCHAR	Reserved[3];
}sSTART;

typedef struct __sJOIN{
	UCHAR	BSSID[6];
	UCHAR	SSID[MAX_SSID_LENGTH];
	UCHAR	BSSType;				
	UCHAR	Channel;
	USHORT	JoinFailureTimeout;
	UCHAR	SSID_Size;
	UCHAR	Reserved;
}sJOIN;

#else

typedef struct __sSCAN{
	UCHAR			BSSID[6];
	UCHAR			SSID[MAX_SSID_LENGTH];
	UCHAR			ScanType;
	UCHAR			Channel;
	USHORT			BSStype;
	USHORT			MinChannelTime;
	USHORT			MaxChannelTime;
}sSCAN;


typedef struct __sSTART{
	UCHAR	BSSID[6];
	UCHAR	SSID[MAX_SSID_LENGTH];
	UCHAR	BSSType;				
	UCHAR	Channel;
}sSTART;

typedef struct __sJOIN{
	UCHAR	BSSID[6];
	UCHAR	SSID[MAX_SSID_LENGTH];
	UCHAR	BSSType;				
	UCHAR	Channel;
	USHORT	JoinFailureTimeout;
}sJOIN;

#endif //RFMD - RFMD_3COM

#endif
