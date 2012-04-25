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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** FastVNET (PCMCIA) NDIS Miniport Driver                                */
/**                                                                       */
/**   Functions used for Tx												  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "vnet.h"
#include "tx.h"


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    TxInit                                                                                                                      
//                                                                                                                                            
//    This function initializes Tx related parameters in the Adapter      
//        object and enables Tx operation on the MAC through the coresponding 
//        field of the IFACE.                                                                                             
//                                                                         
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
TxInit (PVNet_ADAPTER Adapter)
{
  UCHAR configuration;
  ULONG next;

  IF_VERY_LOUD (DbgPrint ("Initializing Tx Section....\n");
    )next = (ULONG) 0x80000000;

  SetTxDescFlags (Adapter, 0, 0);
  SetTxDescNext (Adapter, 0, next);
  SetTxDescMsduPos (Adapter, 0, 0);
  SetTxDescMsduSize (Adapter, 0, 0);

  //
  //      Initialize Tx Descriptors counters and pointers
  //
  Adapter->TxDescFree = Adapter->HostInfo.TxDescCount;
  Adapter->TxDescHead = 0;
  Adapter->TxDescTail = 0;
  Adapter->TxDescPrevious = 0;
  //
  //      Initialize TxBuffers counters and pointers.
  //
  Adapter->TxFreeBuffMem = Adapter->HostInfo.TxBuffSize;
  Adapter->TxBuffHead = 0;
  Adapter->TxBuffTail = 0;

  Adapter->TxBuffStart = Adapter->HostInfo.TxBuffPos;
  Adapter->TxBuffEnd =
    Adapter->HostInfo.TxBuffPos + Adapter->HostInfo.TxBuffSize;

  Adapter->CompletedTransmissions = 0;
  Adapter->Transmissions = 0;
  //
  //      Enable Tx Operation on the card.
  //
  GetFuncCtrl (Adapter, &configuration);
  configuration |= FUNC_CTRL_TxENABLE;
  SetFuncCtrl (Adapter, &configuration);
	GetFuncCtrl(Adapter,&configuration);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    TxCompleteDPC                                                                                                               
//
//    Called after a Tx Complete interrupt. It checks the status of the   
//        transmission and completes the send.                                                            
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
TxCompleteDPC (PVNet_ADAPTER Adapter)
{
  UCHAR flags, status;
  USHORT MsduSize;
  UCHAR PacketType;
  UCHAR loopcnt;

	IF_LOUD(DbgPrint("TxCompleteDPC\n");)
  if (Adapter->StationState == STATION_STATE_RESETTING ||
      Adapter->StationState == STATION_STATE_RESET_PENDING)
    {
      return;
    }
  GetTxDescFlags (Adapter, Adapter->TxDescHead, &flags);

  loopcnt = 0;
  while (flags == TX_DONE && flags != 0xff
	 && loopcnt < Adapter->HostInfo.TxDescCount)
    {
      loopcnt++;

      Adapter->CompletedTransmissions++;
			IF_LOUD(DbgPrint("Fetch package @ 0x%x,0x%x\n",Adapter->TxDescHead,Adapter->HostInfo.TxDescPos);)
      GetTxDescStatus (Adapter, Adapter->TxDescHead, &status);

      GetTxDescHostMsduSize (Adapter, Adapter->TxDescHead, &MsduSize);
      GetTxDescPacketType (Adapter, Adapter->TxDescHead, &PacketType);

      Adapter->TxFreeBuffMem += (USHORT) MsduSize;
      VNET_ASSERT (Adapter->TxFreeBuffMem <= Adapter->HostInfo.TxBuffSize);

      if (Adapter->TxBuffHead + MsduSize > Adapter->TxBuffEnd)
	Adapter->TxBuffHead = 0;
      else
	Adapter->TxBuffHead += (USHORT) MsduSize;

      SetTxDescFlags (Adapter, Adapter->TxDescHead, 0);

      if (Adapter->TxDescHead < (Adapter->HostInfo.TxDescCount - 1))
	Adapter->TxDescHead++;
      else
	Adapter->TxDescHead = 0;

      Adapter->TxDescFree++;

      VNET_ASSERT (Adapter->TxDescFree <= Adapter->HostInfo.TxDescCount);


      if (PacketType == TX_PACKET_TYPE_DATA)
	{
	  if (status == TX_STATUS_SUCCESS)
	    Adapter->Stats.TxDataPacketsOk++;
	  else { 
	    Adapter->Stats.TxDataPacketsError++;
		IF_LOUD(	DbgPrint("Data packet errorcode= %d, errorcount: %d\n",status,Adapter->Stats.TxDataPacketsError);)
		}
	  TxIndicateCompletion (Adapter);
	}
      else if (PacketType == TX_PACKET_TYPE_MGMT)
	{
	  if (status == TX_STATUS_SUCCESS)
	    Adapter->Stats.TxMgmtPacketsOk++;
	  else {
	    Adapter->Stats.TxMgmtPacketsError++;
		}
	}
      GetTxDescFlags (Adapter, Adapter->TxDescHead, &flags);
    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    TxEthernetPacket                                                                                                    
//                                                                                                                                            
//    This function gets an Ethernet (802.3) packet (at once or in                
//        fragments, converts it to 802.11 and copies it on the card to be    
//    transmitted and then calls TxUpdateDescriptor to initiate                   
//    thr transmission.                                                                                                   
//                                                                         
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
TxEthernetPacket (PVNet_ADAPTER Adapter,
		  PUCHAR pTxBuf,
		  ULONG CurrentBytes,
		  PULONG TotalBytes,
		  USHORT StartOfTxBuf, BOOLEAN ContainsHeader)
{
  s80211Header MacHeader;

#ifdef ATMEL_WLAN
	USHORT			EtherType;
	UCHAR			KeyID;
        USHORT                  STATIC_ENTRIES_ETHERTYPES[2] = {0x8137, 0x80F3};
        UCHAR	SNAP_RFC1024[6] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
        UCHAR	SNAP_ETHERNETII[6] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xF8};
#endif

  IF_DEBUG_TX_VERBOSE (DbgPrint
		       ("Tx: %d/%d Bytes to 0x%X (header = %d)\n",
			CurrentBytes, *TotalBytes, StartOfTxBuf,
			ContainsHeader);
    )
    //
    // The Wireless Header is 30 bytes. In the Ethernet packet we "cut" the
    // 12 first bytes (containing DA/SA) and put them in the appropriate fields of
    // the Wireless Header. Thus the packet length now is the initilal + 18 (+30-12)
    //
    if (ContainsHeader)
    {
      *TotalBytes += (USHORT) 18;

      MacHeader.FrameControl[0] = C80211_TYPE_DATA;
      MacHeader.DurationID = 0x00;


	    MacHeader.FrameControl[1] = 0x00;

	  if (Adapter->WepInfo.EncryptionLevel != 0){
                if(!(( pTxBuf[12] == 0x88) && ( pTxBuf[13] == 0x8E))){
                        MacHeader.FrameControl[1] |= C80211_FRMCTRL_WEP;
                }
          }

      if (Adapter->OperatingMode == AD_HOC_MODE) {

	  VNetMoveMemory (MacHeader.uAddress1, pTxBuf,
			  VNet_LENGTH_OF_ADDRESS);
	  VNetMoveMemory (MacHeader.uAddress2, Adapter->CurrentAddress,
			  VNet_LENGTH_OF_ADDRESS);
	  VNetMoveMemory (MacHeader.uAddress3, Adapter->BSSID,
			  VNet_LENGTH_OF_ADDRESS);

	}
      else {

	  MacHeader.FrameControl[1] |= C80211_FRMCTRL_TO_DS;

	  VNetMoveMemory (MacHeader.uAddress1, Adapter->CurrentBSSID,
			  VNet_LENGTH_OF_ADDRESS);
	  VNetMoveMemory (MacHeader.uAddress2, Adapter->CurrentAddress,
			  VNet_LENGTH_OF_ADDRESS);
	  VNetMoveMemory (MacHeader.uAddress3, pTxBuf,
			  VNet_LENGTH_OF_ADDRESS);
	  MacHeader.SequenceControl = 0;
	}


#ifdef ATMEL_WLAN
        EtherType = ((USHORT) pTxBuf[12]) << 8;
        EtherType |= (USHORT) pTxBuf[13];
      if((EtherType == (USHORT)STATIC_ENTRIES_ETHERTYPES[0]) || (EtherType == (USHORT)STATIC_ENTRIES_ETHERTYPES[1])) {

          VNetMoveMemory(MacHeader.uAddress4, SNAP_ETHERNETII, 6);
      } else if (EtherType > 1536) {

          VNetMoveMemory(MacHeader.uAddress4, SNAP_RFC1024, 6);
      } else {
          VNetMoveMemory(MacHeader.uAddress4, &pTxBuf[14], 6);
      }
#endif


      //
      // Copy the wireless Header.
      //


      HostMemToCardMem16 (Adapter,
			  Adapter->HostInfo.TxBuffPos + Adapter->TxBuffTail,
			  (PUCHAR) & MacHeader, DATA_FRAME_WS_HEADER_SIZE);
      Adapter->TxBuffTail += DATA_FRAME_WS_HEADER_SIZE;	// Data should start at offset 30
      //
      // The first buffer contains DA/SA in the first 12 bytes.
      // Copy the rest bytes (DA/SA have been copied in the Wireless Header).
      //
      HostMemToCardMem16 (Adapter,
			  Adapter->HostInfo.TxBuffPos + Adapter->TxBuffTail,
			  &pTxBuf[12], CurrentBytes - 12);
      Adapter->TxBuffTail += ((USHORT) CurrentBytes - 12);
    }
  else {
      HostMemToCardMem16 (Adapter,
			  Adapter->HostInfo.TxBuffPos + Adapter->TxBuffTail,
			  pTxBuf, CurrentBytes);
      Adapter->TxBuffTail += (USHORT) CurrentBytes;
  }
  return VNET_TX_STATUS_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//  TxResourcesAvailable                                                                                     
//                                                                                                                  
//  Returns TRUE if there is enough space & descriptors for a frame to be 
//      transmitted and also the starting address that this frame must be copied.
//     If there are no resources available returns FALSE                                             
//                                                                         
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
TxResourcesAvailable (PVNet_ADAPTER Adapter, ULONG Bytes,
		      PUSHORT StartOfTxBuf)
{
  USHORT FreeBytesAtBottom;

  FreeBytesAtBottom = Adapter->HostInfo.TxBuffSize - Adapter->TxBuffTail;

  if (Adapter->TxDescFree == 3)
    {
      return FALSE;
    }

  if (Adapter->TxFreeBuffMem < Bytes)	//no space at all
    {
      return FALSE;
    }
  else				// Free space but where?
  if (FreeBytesAtBottom < Bytes)	//No Space at bottom
    {
      if (Adapter->TxFreeBuffMem - FreeBytesAtBottom >= (USHORT) Bytes)	//Ok at top
	{
	  Adapter->TxBuffTail = 0;
	  *StartOfTxBuf = Adapter->HostInfo.TxBuffPos;
	  return TRUE;
	}
      else			// no space at bottom & at top
	{
	  return FALSE;
	}
    }
  else
    {
      *StartOfTxBuf = Adapter->TxBuffStart + Adapter->TxBuffTail;
      return TRUE;
    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    TxUpdateDescriptor                                                                                                  
//
//    This function updates the Tx descriptors to transmit a frame                
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


VOID
TxUpdateDescriptor (PVNet_ADAPTER Adapter, ULONG TotalBytes,
		    USHORT StartOfTxBuffer, BOOLEAN IsData)
{

#ifdef ATMEL_WLAN
        UCHAR	CipherType, CipherLength;
#endif
  IF_DEBUG_TX_VERBOSE (DbgPrint
		       ("TxUpdateDescriptor %d to 0x%X\n", TotalBytes,
			StartOfTxBuffer);
    )
	IF_LOUD(DbgPrint("slip a package into 0x%x,%x,%d\n",Adapter->TxDescTail,Adapter->HostInfo.TxDescPos,TotalBytes);)
	SetTxDescMsduPos (Adapter, Adapter->TxDescTail, StartOfTxBuffer);
  SetTxDescMsduSize (Adapter, Adapter->TxDescTail, (USHORT) TotalBytes);

#ifndef ATMEL_WLAN
  SetTxDescHostMsduSize (Adapter, Adapter->TxDescTail, (USHORT) TotalBytes);
#endif

  if (IsData)
    SetTxDescPacketType (Adapter, Adapter->TxDescTail, TX_PACKET_TYPE_DATA);
  else
    SetTxDescPacketType (Adapter, Adapter->TxDescTail, TX_PACKET_TYPE_MGMT);

  if (Adapter->DataFramesTxRate != 4)
    SetTxDescRate (Adapter, Adapter->TxDescTail, Adapter->DataFramesTxRate);
  else
    SetTxDescRate (Adapter, Adapter->TxDescTail, 3);

  SetTxDescRetryCount (Adapter, Adapter->TxDescTail, 0);

        // We send the unicast packets with the pairwise key if available, else we send 
	// them using the group key. Also, we send the multicast packets with the group key
	// if available , else if the encryption is WEP we use the pairwise key.

#ifdef ATMEL_WLAN
	if(Adapter->IsBroadcast) {

		CipherType = Adapter->WepInfo.GroupCipherSuite;
		if(Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_WEP_64 || Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_WEP_128)
			CipherLength = 8;
		else if(Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_TKIP)
			CipherLength = 12;
		else {
			CipherType = CIPHER_SUITE_NONE;
			CipherLength = 0;
		}
    
		if( CipherType == CIPHER_SUITE_NONE && 
			(Adapter->WepInfo.PairwiseCipherSuite == CIPHER_SUITE_WEP_64 || 
			 Adapter->WepInfo.PairwiseCipherSuite == CIPHER_SUITE_WEP_128)
		  )
		{
			CipherType = Adapter->WepInfo.PairwiseCipherSuite;
			CipherLength = 8;
		}
		
	}
	else {
		CipherType = Adapter->WepInfo.PairwiseCipherSuite;
		if(Adapter->WepInfo.PairwiseCipherSuite == CIPHER_SUITE_WEP_64 || Adapter->WepInfo.PairwiseCipherSuite == CIPHER_SUITE_WEP_128){
			CipherLength = 8;
    }
		else if(Adapter->WepInfo.PairwiseCipherSuite == CIPHER_SUITE_TKIP)
			CipherLength = 12;
		else {
			CipherType = CIPHER_SUITE_NONE;
			CipherLength = 0;
		}

		if(CipherType == CIPHER_SUITE_NONE)
		{
			CipherType = Adapter->WepInfo.GroupCipherSuite;
			if(Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_WEP_64 || Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_WEP_128)
				CipherLength = 8;
			else
			if(Adapter->WepInfo.GroupCipherSuite == CIPHER_SUITE_TKIP)
				CipherLength = 12;
			else
			{
				CipherType = CIPHER_SUITE_NONE;
				CipherLength = 0;
			}
		}

	}

        SetTxDescCipherType(Adapter, Adapter->TxDescTail, CipherType);
	SetTxDescCipherLength(Adapter, Adapter->TxDescTail, CipherLength);

#endif // ATMEL_WLAN

 SetTxDescNext (Adapter, Adapter->TxDescTail, 0x80000000);
  SetTxDescFlags (Adapter, Adapter->TxDescTail, TX_FIRM_OWN);

 if (Adapter->TxDescPrevious != Adapter->TxDescTail)
    SetTxDescNext (Adapter, Adapter->TxDescPrevious, 0x00);


  Adapter->TxDescPrevious = Adapter->TxDescTail;

  if (Adapter->TxDescTail < (Adapter->HostInfo.TxDescCount - 1))
    Adapter->TxDescTail++;
  else
    Adapter->TxDescTail = 0;


  Adapter->TxDescFree--;

 // Adapter->TxFreeBuffMem -= (USHORT) TotalBytes;

  //VNET_ASSERT (Adapter->TxFreeBuffMem <= Adapter->HostInfo.TxBuffSize);
  Adapter->Transmissions++;
}

//////////////////////////////////////////////////
//
// Tx Descriptors manipulation functions
//
//////////////////////////////////////////////////

void
GetTxDescFlags (PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR flags)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_FLAGS_OFFSET));
  CardReadUchar (Adapter, DR, flags);
}


void
GetTxDescStatus (PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR status)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_STATUS_OFFSET));
  CardReadUchar (Adapter, DR, status);
}


void
SetTxDescFlags (PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR flags)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_FLAGS_OFFSET));
  CardWriteUchar (Adapter, DR, flags);
}



void
SetTxDescRetryCount (PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR cnt)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_RETRY_OFFSET));
  CardWriteUchar (Adapter, DR, cnt);
}

void
SetTxDescNext (PVNet_ADAPTER Adapter, USHORT Descriptor, ULONG next)
{
  HostMemToCardMem16 (Adapter,
		      Adapter->HostInfo.TxDescPos +
		      (SIZE_OF_TXDESC * Descriptor) + TX_DESC_NEXT_OFFSET,
		      (PUCHAR) & next, 4);
}

#ifdef ATMEL_WLAN

void SetTxDescCipherType(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR CipherType)
{
	HostMemToCardMem8(Adapter, (Adapter->HostInfo.TxDescPos + (SIZE_OF_TXDESC * Descriptor) + TX_DESC_CIPHER_TYPE_OFFSET), &CipherType, 1);
}

void SetTxDescCipherLength(PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR CipherLen)
{
	HostMemToCardMem8(Adapter, (Adapter->HostInfo.TxDescPos + (SIZE_OF_TXDESC * Descriptor) + TX_DESC_CIPHER_LENGTH_OFFSET), &CipherLen, 1);
}
 
void GetTxDescKeyIndex(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR Index)
{
	CardMemToHostMem8(Adapter, Index, (Adapter->HostInfo.TxDescPos + (SIZE_OF_TXDESC * Descriptor) + TX_DESC_KEY_INDEX_OFFSET), 1);
}

void GetTxDescCipherType(PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR CipherType)
{
	CardMemToHostMem8(Adapter, CipherType, (Adapter->HostInfo.TxDescPos + (SIZE_OF_TXDESC * Descriptor) + TX_DESC_CIPHER_TYPE_OFFSET), 1);
}
#endif // ATMEL_WLAN

void
SetTxDescMsduPos (PVNet_ADAPTER Adapter, USHORT Descriptor, USHORT msdupos)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_POS_OFFSET));
  CardWriteUshort (Adapter, DR, msdupos);
}

void
GetTxDescMsduSize (PVNet_ADAPTER Adapter, USHORT Descriptor, PUSHORT msdusize)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_SIZE_OFFSET));
  CardReadUshort (Adapter, DR, msdusize);
}


void
SetTxDescMsduSize (PVNet_ADAPTER Adapter, USHORT Descriptor, USHORT msdusize)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_SIZE_OFFSET));
  CardWriteUshort (Adapter, DR, msdusize);
}


void
SetTxDescRate (PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR RateIndex)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_RATE_OFFSET));
  CardWriteUchar (Adapter, DR, RateIndex);
}


void
GetTxDescHostMsduSize (PVNet_ADAPTER Adapter, USHORT Descriptor,
		       PUSHORT msdusize)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_HOST_LENGTH_OFFSET));
  CardReadUshort (Adapter, DR, msdusize);
}


void
SetTxDescHostMsduSize (PVNet_ADAPTER Adapter, USHORT Descriptor,
		       USHORT msdusize)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_HOST_LENGTH_OFFSET));
  CardWriteUshort (Adapter, DR, msdusize);

}


void
GetTxDescPacketType (PVNet_ADAPTER Adapter, USHORT Descriptor,
		     PUCHAR PacketType)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_PACKET_TYPE_OFFSET));
  CardReadUchar (Adapter, DR, PacketType);
}


void
SetTxDescPacketType (PVNet_ADAPTER Adapter, USHORT Descriptor,
		     UCHAR PacketType)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.TxDescPos +
			     (SIZE_OF_TXDESC * Descriptor) +
			     TX_DESC_PACKET_TYPE_OFFSET));
  CardWriteUchar (Adapter, DR, PacketType);
}
