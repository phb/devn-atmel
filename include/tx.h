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



#ifndef __tx_h_OK__
#define __tx_h_OK__


#define	VNET_TX_STATUS_SUCCESS		0
#define	VNET_TX_STATUS_RESOURCES	1

//
// TX descriptor
//
typedef struct __TXDESC{
   ULONG		NextDescriptor;

   USHORT   	TxStartOfFrame;
   USHORT   	TxLength;
   
   UCHAR    	TxState;
   UCHAR    	TxStatus;
   UCHAR    	RetryCount;

   UCHAR    	TxRate;

#ifdef ATMEL_WLAN
    UCHAR		KeyIndex;
    UCHAR		ChipherType;
    UCHAR		ChipherLength;
    UCHAR		Reserved1;
    UCHAR		Reserved2;
#else
   ULONG    	TxTime;
   UCHAR		Reserved;
#endif

   UCHAR		PacketType;
   USHORT		HostTxLength;

} TXDESC;
//#endif

#define SIZE_OF_TXDESC			sizeof(TXDESC)

#define TX_DESC_NEXT_OFFSET				0
#define TX_DESC_POS_OFFSET				4
#define TX_DESC_SIZE_OFFSET				6
#define TX_DESC_FLAGS_OFFSET			8
#define TX_DESC_STATUS_OFFSET			9
#define TX_DESC_RETRY_OFFSET			10
#define TX_DESC_RATE_OFFSET				11
#ifdef ATMEL_WLAN
#define TX_DESC_KEY_INDEX_OFFSET    12
#define TX_DESC_CIPHER_TYPE_OFFSET    13
#define TX_DESC_CIPHER_LENGTH_OFFSET    14
#endif
#define TX_DESC_PACKET_TYPE_OFFSET		17
#define TX_DESC_HOST_LENGTH_OFFSET		18



///////////////////////////////////////////////////////
// Host-MAC interface
///////////////////////////////////////////////////////

#define TX_STATUS_SUCCESS	0x00

#define TX_FIRM_OWN			(UCHAR)0x80
#define TX_DONE				(UCHAR)0x40


#define TX_ERROR			(UCHAR)0x01


//#define TXDESC_OFFSET(field) ( (UINT) FIELD_OFFSET(TXDESC,field) )
//#define TXDESC_SIZE(field)    sizeof( ((TXDESC *)0)->field )


#define TX_PACKET_TYPE_DATA		0x01
#define TX_PACKET_TYPE_MGMT		0x02

#endif
