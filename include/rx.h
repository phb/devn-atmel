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

#ifndef __rx_h_OK__
#define __rx_h_OK__


#include "mib.h"
#include "command.h"

extern UCHAR	SNAP_RFC1024[6];
extern UCHAR	SNAP_ETHERNETII[6];
extern UCHAR	STATIC_ENTRIES_ETHERTYPE_8137[2];
extern UCHAR	STATIC_ENTRIES_ETHERTYPE_80F3[2];

typedef struct __RXDESC
{
	ULONG		Next;
	USHORT		MsduPos;
	USHORT		MsduSize;

	UCHAR		State;
	UCHAR		Status;
	UCHAR		Rate;
	UCHAR   	Rssi;
	UCHAR		LinkQuality;
	UCHAR		PreambleType;
	USHORT		Duration;
	ULONG		RxTime;

}RXDESC;

#define RX_DESC_FLAG_VALID					0x80
#define RX_DESC_FLAG_CONSUMED				0x40
#define RX_DESC_FLAG_IDLE					0x00

#define RX_STATUS_SUCCESS					0x00

#define RX_DESC_MSDU_POS_OFFSET				4
#define RX_DESC_MSDU_SIZE_OFFSET			6
#define RX_DESC_FLAGS_OFFSET				8
#define RX_DESC_STATUS_OFFSET				9
#define RX_DESC_RSSI_OFFSET					11
#define RX_DESC_LINK_QUALITY_OFFSET			12
#define RX_DESC_PREAMBLE_TYPE_OFFSET		13
#define RX_DESC_DURATION_OFFSET				14
#define RX_DESC_RX_TIME_OFFSET				16


/////////////////////////////////////////////////////////////////////////

#define SIZE_OF_RXDESC					sizeof(RXDESC)

#define RXDESC_OFFSET(field) ( (UINT) FIELD_OFFSET(RXDESC,field) )
#define RXDESC_SIZE(field)    sizeof( ((RXDESC *)0)->field )

		
extern UCHAR	SNAP_RFC1024[6];
extern UCHAR	SNAP_ETHERNETII[6];
extern UCHAR	STATIC_ENTRIES_ETHERTYPE_8137[2];
extern UCHAR	STATIC_ENTRIES_ETHERTYPE_80F3[2];



#endif

