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


#ifndef __stdinclude_h_OK__
#define __stdinclude_h_OK__
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/siginfo.h>
#include <sys/syspage.h>
#include <sys/neutrino.h>
#include <sys/dcmd_io-net.h>
#include <sys/slogcodes.h>

#include <hw/pci.h>
#include <hw/inout.h>
#include <hw/pci_devices.h>
#include <sys/mman.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <sys/io-net.h>
#include <drvr/mdi.h>
#include <drvr/eth.h>
#include <drvr/support.h>

#include <vnet.h>

#define OUT_OF_RANGE_TIME_OUT				2000
#define ASSOCIATION_RESPONSE_TIME_OUT		1000
#define JOIN_TIME_OUT						1000

#define MAX_AUTHENTICATION_RETRIES			3
#define MAX_ASSOCIATION_RETRIES 			3

//
// Offsets of some fields in frames
//
#define MGMT_FRAME_SA_OFFSET						10
#define DATA_FRAME_ADHOC_SA_OFFSET					10
#define DATA_FRAME_INFR_SA_OFFSET					16
#define DATA_FRAME_ADHOC_ADR3_OFFSET				16

#define BEACON_INTERVAL_OFFSET						8			//12+2
#define SSID_ELLEMENT_OFFSET						14			//12+2
#define SSID_ELLEMENT_LENGTH_OFFSET 				13			//12+1
#define SUP_RATES_ELLEMENT_LENGTH_REL_OFFSET		15			//12+2+1
#define DS_PARAM_SET_ELLEMENT_REL_OFFSET			18			//12+2+2+2
#define BSSID_HEADER_OFFSET 						16
#define MGMT_FRAME_BODY_OFFSET						24

#define WIRELESS_HEADER_ADDRESS3_OFFSET 			16
#define CAPABILITY_INFO_IN_BEACON_OFFSET			10			
#define STATUS_IN_AUTHENTICATION_OFFSET 			4			
#define TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET		2			
#define CHALL_TEXT_EL_ID_IN_AUTHENTICATION_OFFSET	6			
#define CHALL_TEXT_LENGTH_IN_AUTHENTICATION_OFFSET	7			
#define CHALL_TEXT_IN_AUTHENTICATION_OFFSET 		8

#define STATUS_IN_ASSOCIATION_RESP_OFFSET			2			
#define ASS_ID_IN_ASSOCIATION_RESP_OFFSET			4
#define SUP_RATES_LENGTH_IN_ASSOCIATION_RESP_OFFSET 7
#define SUP_RATES_IN_ASSOCIATION_RESP_OFFSET		8

//////////////////////////////////
//	Functions in card.c
//////////////////////////////////

BOOLEAN
CardReset(PVNet_ADAPTER Adapter);

BOOLEAN 
CardGetHostInfo(PVNet_ADAPTER Adapter);

BOOLEAN 
CardStartUp(PVNet_ADAPTER Adapter);

VOID
CardStop(PVNet_ADAPTER Adapter);


//////////////////////////////////
//	Functions in command.c
//////////////////////////////////

UCHAR 
SendCommand(	PVNet_ADAPTER Adapter, 
				UCHAR CmdId, 
				PUCHAR pParameters);			

BOOLEAN
SetChannel(PVNet_ADAPTER Adapter, UCHAR Channel);

BOOLEAN
SetDataFramesTxRateMode(PVNet_ADAPTER Adapter, UCHAR RateIndex);

BOOLEAN
SetRtsThreshold(PVNet_ADAPTER Adapter, USHORT Threshold);

BOOLEAN
SetFragmentationThreshold(PVNet_ADAPTER Adapter, USHORT Threshold);

BOOLEAN
SetPreambleType(PVNet_ADAPTER Adapter, UCHAR PreambleType);

BOOLEAN
SetCtrlFramesTxRate(PVNet_ADAPTER Adapter);

BOOLEAN
SetMgmtFramesTxRate(PVNet_ADAPTER Adapter);

BOOLEAN 
GetStatistics(PVNet_ADAPTER Adapter, PUCHAR Statistics);

BOOLEAN
SetPowerMgmtMode(PVNet_ADAPTER Adapter, UCHAR PSmode);

BOOLEAN
SetAssociationID(PVNet_ADAPTER Adapter, USHORT AssocID);

BOOLEAN
SetSupportedRates(PVNet_ADAPTER Adapter, PUCHAR SupportedRates, UCHAR SupRatesLength);

UCHAR
Join(PVNet_ADAPTER Adapter, UCHAR Type);

UCHAR
Start(PVNet_ADAPTER Adapter, UCHAR Type);

BOOLEAN
Scan(PVNet_ADAPTER Adapter, BOOLEAN UseSpecificSSID);	

VOID
CommandCompleteDPC(PVNet_ADAPTER Adapter);

BOOLEAN
SetPromiscuousMode(PVNet_ADAPTER Adapter, UCHAR Mode);

BOOLEAN
SetMACaddress(PVNet_ADAPTER Adapter, PUCHAR MACaddress);


BOOLEAN
SetMibValues(PVNet_ADAPTER Adapter);

BOOLEAN 
GetBSSID(PVNet_ADAPTER Adapter, PUCHAR bssid);

BOOLEAN
SetBeaconPeriod(PVNet_ADAPTER Adapter, USHORT BeaconPeriod);


#if (defined RFMD) || (defined RFMD_3COM)

BOOLEAN
GetRegulatoryDomain(PVNet_ADAPTER Adapter, PUCHAR RegDomain);

UCHAR
DisableRadio(PVNet_ADAPTER Adapter);

UCHAR
EnableRadio(PVNet_ADAPTER Adapter);

BOOLEAN 
SetRegulatoryDomain(PVNet_ADAPTER Adapter, UCHAR RegDomainCode);

#else

BOOLEAN
SetSSIDsize(PVNet_ADAPTER Adapter, UCHAR size);

#endif

//////////////////////////////////
//	Functions in fastvnet_cs.c
//////////////////////////////////

VOID 
VnetSleep(ULONG stime);

VOID
VNet_netif_wake_queue(PVNet_ADAPTER Adapter);

VOID 
RxIndicatePacket(PVNet_ADAPTER Adapter, PUCHAR pRxBuf, ULONG PacketLength, ULONG IndicateLen);


//////////////////////////////////
//	Functions in flash.c
//////////////////////////////////

VOID 
WriteFlash8(PVNet_ADAPTER Adapter, ULONG Address, UCHAR Data);

VOID 
ReadFlash8(PVNet_ADAPTER Adapter, ULONG Address, PUCHAR pData);


BOOLEAN 
GetFlashSize(PVNet_ADAPTER Adapter);

VOID
GetFlashID(PVNet_ADAPTER Adapter, PUCHAR ManID, PUCHAR DevID);

VOID
EraseFlash(PVNet_ADAPTER Adapter);

BOOLEAN
BlankCheck(PVNet_ADAPTER Adapter);

BOOLEAN
DownloadBufferToFlash(PVNet_ADAPTER Adapter, ULONG bytes, PUCHAR data, ULONG sect_offset);

BOOLEAN
ProgramFlash(PVNet_ADAPTER Adapter);

VOID
GetCurrentCardInfo(PVNet_ADAPTER Adapter);

VOID 
EnterFlashIDMode(PVNet_ADAPTER Adapter); 

//////////////////////////////////
//	Functions in mgmt.c
//////////////////////////////////

BOOLEAN 
SendAuthenticationRequest(PVNet_ADAPTER Adapter, PUCHAR pChallengeText, UCHAR ChallengeTextLength);

BOOLEAN
SendAssociationRequest(PVNet_ADAPTER Adapter);

BOOLEAN
SendReAssociationRequest(PVNet_ADAPTER Adapter);

BOOLEAN 
TransmitMgmtFrame(	PVNet_ADAPTER Adapter, 
					PUCHAR pFrameBuffer, 
					USHORT	FrameSize);

VOID 
MgmtFrameRxProcessing(	PVNet_ADAPTER Adapter, 
							UCHAR FrameSubtype,
							USHORT FrameLength);


VOID
DisplayMgmtFrameStatus(ULONG StatusCode);

BOOLEAN
StoreBSSinfo(PVNet_ADAPTER Adapter, PUCHAR BSSID, PUCHAR pBeaconOrPrope, UCHAR Channel, BOOLEAN IsProbe);

int
RetrieveBSS(PVNet_ADAPTER Adapter, PUCHAR BSSID);

VOID
SelectBSS(PVNet_ADAPTER Adapter, int APindex);

//////////////////////////////////
//	Functions in rx.h
//////////////////////////////////

VOID 
RxInit(PVNet_ADAPTER Adapter);

VOID 
RxCompleteDPC(PVNet_ADAPTER Adapter);

BOOLEAN 
GetChannelFromBeaconOrProbe(PUCHAR pBeaconOrProbeFrame, PUCHAR pChannel);

void 
GetRxDescRssi(PVNet_ADAPTER Adapter, USHORT Descriptor, PCHAR rssi);

//////////////////////////////////
//	Functions in tx.h
//////////////////////////////////

VOID 
TxInit(PVNet_ADAPTER Adapter);

VOID 
TxCompleteDPC(PVNet_ADAPTER Adapter);

VOID
TxUpdateDescriptor(PVNet_ADAPTER Adapter, ULONG TotalBytes, USHORT StartOfTxBuffer, BOOLEAN IsData);

UCHAR 
TxEthernetPacket(PVNet_ADAPTER Adapter, 
					 PUCHAR pTxBuf, 
					 ULONG CurrentBytes, 
					 PULONG TotalBytes, 
					 USHORT StartOfTxBuf,
					 BOOLEAN ContainsHeader);

VOID 
TxIndicateCompletion(PVNet_ADAPTER Adapter);

BOOLEAN 
TxResourcesAvailable(PVNet_ADAPTER Adapter, ULONG Bytes, PUSHORT StartOfTxBuf);

//////////////////////////////////
//	Functions in vnet.c
//////////////////////////////////

BOOLEAN 
ConfigureWEP(PVNet_ADAPTER Adapter);

BOOLEAN
ResetAdapter(PVNet_ADAPTER Adapter, BOOLEAN InitiateScan);

VOID
InitAdapter(PVNet_ADAPTER Adapter);

BOOLEAN
InitAndStartCard(PVNet_ADAPTER Adapter);

VOID 
CopyCodeToInternalMemory(PVNet_ADAPTER Adapter);

BOOLEAN
ChangeConfiguration(PVNet_ADAPTER Adapter, PDEVICE_CONFIGURATION pHostInfo);


VOID
GetMACaddress(PVNet_ADAPTER Adapter);

BOOLEAN 
GetDefaultFw(PVNet_ADAPTER Adapter);

VOID 
ProcessInterrupt(PVNet_ADAPTER Adapter);

VOID 
MgmtTimeOutCallBack(PVNet_ADAPTER Adapter);

UCHAR 
AsciiToByte(UCHAR Ascii1, UCHAR Ascii2);

VOID
ByteToAscii(UCHAR Byte, PUCHAR Ascii1, PUCHAR Ascii2);

BOOLEAN 
JoinSpecificBSS(PVNet_ADAPTER Adapter, int BSSindex, BOOLEAN ChangeInitialSSID);

VOID 
VNetZeroMemory(PUCHAR buf, ULONG bytes);

BOOLEAN
ValidateChannelForDomain(PVNet_ADAPTER Adapter, PUCHAR pChannel);

VOID PrepareBSSList2View(PVNet_ADAPTER Adapter);

#ifdef INT_ROAM
BOOLEAN
SetIntRoaming(PVNet_ADAPTER Adapter);

BOOLEAN 
GetMultiDomainMIB(PVNet_ADAPTER Adapter);

#endif

void MgmtTimer(ULONG a);
void SetMgmtTimer(PVNet_ADAPTER Adapter);

////////
// tx.c
////////
VOID
TxInit(PVNet_ADAPTER Adapter);

VOID
TxCompleteDPC(PVNet_ADAPTER Adapter);

UCHAR
TxEthernetPacket(PVNet_ADAPTER Adapter,
					 PUCHAR pTxBuf,
					 ULONG CurrentBytes,
					 PULONG TotalBytes,
					 USHORT StartOfTxBuf,
					 BOOLEAN ContainsHeader);

BOOLEAN 
TxResourcesAvailable(PVNet_ADAPTER Adapter, ULONG Bytes, PUSHORT StartOfTxBuf);

VOID
TxUpdateDescriptor(PVNet_ADAPTER Adapter, ULONG TotalBytes, USHORT StartOfTxBuffer, BOOLEAN IsData);

VOID
GetTxDescFlags(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR flags);


VOID 
GetTxDescStatus(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR status);

VOID 
SetTxDescFlags(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR flags);

VOID 
SetTxDescRetryCount(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR cnt);

VOID
SetTxDescNext(PVNet_ADAPTER Adapter, USHORT Descriptor, ULONG next);

VOID 
SetTxDescMsduPos(PVNet_ADAPTER Adapter, USHORT Descriptor, USHORT msdupos);

VOID 
GetTxDescMsduSize(PVNet_ADAPTER Adapter, USHORT Descriptor, PUSHORT msdusize);

VOID 
SetTxDescMsduSize(PVNet_ADAPTER Adapter, USHORT Descriptor, USHORT msdusize);

VOID 
SetTxDescRate(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR RateIndex);

VOID 
GetTxDescHostMsduSize(PVNet_ADAPTER Adapter, USHORT Descriptor, PUSHORT msdusize);

VOID 
SetTxDescHostMsduSize(PVNet_ADAPTER Adapter, USHORT Descriptor, USHORT msdusize);

VOID 
GetTxDescPacketType(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR PacketType);


VOID
SetTxDescPacketType(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR PacketType);

////////
// inlines.c
////////

VOID
CardMemToHostMem16(PVNet_ADAPTER Adapter, PUCHAR pDestBuf, ULONG SramAd, ULONG Bytes);

VOID
HostMemToCardMem16(PVNet_ADAPTER Adapter, ULONG SramAd, PUCHAR pSrcBuf, ULONG Bytes);

VOID 
SetFuncCtrl(PVNet_ADAPTER Adapter, PUCHAR FuncCtrl);

VOID 
SetLockoutMac(PVNet_ADAPTER Adapter, PUCHAR pData);

VOID 
SetIntStatus(PVNet_ADAPTER Adapter, PUCHAR IntStatus);

VOID 
SetIntMask(PVNet_ADAPTER Adapter, PUCHAR IntMask);

VOID 
GetLockoutHost(PVNet_ADAPTER Adapter, PUCHAR pData);


VOID 
GetLockoutMac(PVNet_ADAPTER Adapter, PUCHAR pData);

VOID 
GetIntStatus(PVNet_ADAPTER Adapter, PUCHAR IntStatus);

VOID
GetFuncCtrl(PVNet_ADAPTER Adapter, PUCHAR FuncCtrl);

VOID 
GetGenericIrqType(PVNet_ADAPTER Adapter, PUSHORT GenericIntType);

int 
MemCompare(PUCHAR pMem1, PUCHAR pMem2, ULONG bytes);

VOID 
IndicateConnectionStatus(PVNet_ADAPTER Adapter, BOOLEAN Connected);

VOID 
CardReadUshort(PVNet_ADAPTER Adapter, UCHAR Offset, PUSHORT pData);

VOID
CardWriteUshort(PVNet_ADAPTER Adapter, UCHAR Offset, USHORT Data);

VOID 
CardWriteUchar(PVNet_ADAPTER Adapter, UCHAR Offset, UCHAR Data);

VOID 
CardReadUchar(PVNet_ADAPTER Adapter, UCHAR Offset, PUCHAR pData);

VOID 
CardMemToHostMem8(PVNet_ADAPTER Adapter, PUCHAR pDestBuf, ULONG SramAd, ULONG Bytes);

VOID 
HostMemToCardMem8(PVNet_ADAPTER Adapter, ULONG SramAd, PUCHAR pSrcBuf, ULONG Bytes);

///////
// interrupt.c
///////

VOID 
DisableInterrupts(PVNet_ADAPTER Adapter);

VOID 
EnableInterrupts(PVNet_ADAPTER Adapter);

VOID 
AcknowledgeInterrupt(PVNet_ADAPTER Adapter);

UCHAR 
GetInterruptStatus(PVNet_ADAPTER Adapter);

UCHAR
ResetInterruptStatusBit(PVNet_ADAPTER Adapter, UCHAR mask);

VOID
SetInterruptMask(PVNet_ADAPTER Adapter, UCHAR Mask);


VOID 
ProcessInterrupt(PVNet_ADAPTER Adapter);
////////
// rx.c
////////


VOID 
RxCompleteDPC(PVNet_ADAPTER Adapter);

#ifdef RX_CRC
VOID 
init_CRCtable(PVNet_ADAPTER Adapter);

ULONG 
Calculate_CRC32(PUCHAR p, ULONG FrameLength, PULONG crctab);

#endif //RX_CRC

////////////////
// interrupt.c
///////////////
VOID
ProcessInterrupt(PVNet_ADAPTER Adapter);

VOID
VNetMoveMemory(PVOID pDest, PVOID pSrc, ULONG bytes);

VOID
VNetZeroMemory(PUCHAR buf, ULONG bytes);


VOID CardReadUshort(PVNet_ADAPTER Adapter, UCHAR Offset, PUSHORT pData);


#endif


#ifdef ATMEL_WLAN
BOOLEAN
SetEncryptionKey(PVNet_ADAPTER Adapter, PUCHAR Key, UCHAR index, ULONG length);
void SetTxDescKeyIndex(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR Index);
void SetTxDescCipherType(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR CipherType);
void SetTxDescCipherLength(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR CipherLen);
void GetTxDescKeyIndex(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR Index);
void GetTxDescCipherType(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR CipherType);
typedef UCHAR	UINT8;
typedef USHORT	UINT16;
void WPA_addMICToPacket(UINT8 *packet,UINT16 msgLength,UINT8 *micKey);
UINT8 WPA_validateMic(UINT8 *packet,UINT16 msgLength,UINT8 *micKey);
#endif // ATMEL_WLAN


