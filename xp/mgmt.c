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
/**   Functions Handling Mgmt Frames									  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#include "vnet.h"
#include "tx.h"

extern UCHAR BasicRates[2][4];

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    SendAuthenticationRequest                                                                               
//
//    This function formats an Authentication Request Frame and calls     
//    TransmitMgmtFrame() to transmit it.                                              
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOLEAN
SendAuthenticationRequest (PVNet_ADAPTER Adapter, PUCHAR pChallengeText,
			   UCHAR ChallengeTextLength)
{
  USHORT FrameLen;
  s80211_MgmtFrm MgmtFrame;
  s80211_MgmtBody_Authentication *AuthFrameBody =
    (s80211_MgmtBody_Authentication *) MgmtFrame.Body;

  //
  // Format Frame Header 
  //
  MgmtFrame.FrameControl[0] =
    C80211_PROTOCOL_VERSION | C80211_TYPE_MGMT |
    C80211_SUBTYPE_MGMT_Authentication;

  //
  // Support WEP (NO WEP for authentication frames with TrSeqNo 1)
  //

  if (Adapter->CurrentAuthentTransactionSeqNum == 1
      || Adapter->WepInfo.EncryptionLevel == 0)
    MgmtFrame.FrameControl[1] = 0x00;
  else if (Adapter->WepInfo.EncryptionLevel != 0)
    MgmtFrame.FrameControl[1] = C80211_FRMCTRL_WEP;

  MgmtFrame.DurationID = 0x8000;

  VNetMoveMemory (MgmtFrame.uDA, Adapter->CurrentBSSID, 6);
  VNetMoveMemory (MgmtFrame.uSA, Adapter->CurrentAddress, 6);
  VNetMoveMemory (MgmtFrame.uBSSID, Adapter->CurrentBSSID, 6);

  MgmtFrame.SequenceControl = 0x0000;

  //
  // Frame Body
  //
  if (Adapter->WepInfo.EncryptionLevel == 0)	// NO WEP
    AuthFrameBody->AuthenticationAlgorithmNumber = C80211_MGMT_AAN_OPENSYSTEM;
  else
    AuthFrameBody->AuthenticationAlgorithmNumber =
      Adapter->WepInfo.AuthenticationType;

  //
  // Set AuthenticationTransactionSequenceNumber
  //
  AuthFrameBody->AuthenticationTransactionSequenceNumber =
    Adapter->CurrentAuthentTransactionSeqNum;

  IF_LOUD (DbgPrint
	   ("Sending Authentication Request with Transaction Sequence Number:%d...\n",
	    Adapter->CurrentAuthentTransactionSeqNum);
    )Adapter->ExpectedAuthentTransactionSeqNum =
    Adapter->CurrentAuthentTransactionSeqNum + 1;

  Adapter->CurrentAuthentTransactionSeqNum += 2;


  AuthFrameBody->StatusCode = 0x0000;

  if (ChallengeTextLength != 0)
    {
      AuthFrameBody->ChallengeText.ElementID =
	C80211_MGMT_ElementID_ChallengeText;
      AuthFrameBody->ChallengeText.Length = ChallengeTextLength;

      VNetMoveMemory (&MgmtFrame.Body[8], pChallengeText,
		      ChallengeTextLength);

      FrameLen = 24 + 6 + 2 + ChallengeTextLength;	//Header + Body + element Header + element length
    }
  else
    FrameLen = 24 + 6;		//Header + Body

  //
  //send frame & return status
  //
  return TransmitMgmtFrame (Adapter, (UCHAR *) & MgmtFrame,
			    (USHORT) FrameLen);
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    SendAssociationRequest                                                                                  
//
//    This function formats an Association Request Frame and calls                
//    TransmitMgmtFrame() to transmit it.                                     
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SendAssociationRequest (PVNet_ADAPTER Adapter)
{
  USHORT FrameLen;
  s80211_MgmtFrm MgmtFrame;
  s80211_MgmtBody_AssRqst *AssReqFrameBody =
    (s80211_MgmtBody_AssRqst *) MgmtFrame.Body;

  IF_LOUD (USHORT i;
	   DbgPrint ("SSID to Associate:");
	   for (i = 0; i < Adapter->SSID_Size; i++)
	   DbgPrint ("%c", Adapter->DesiredSSID[i]); DbgPrint ("\n");)

    //
    // Create Association Request Frame Header
    //
    MgmtFrame.FrameControl[0] =
      C80211_PROTOCOL_VERSION | C80211_TYPE_MGMT |
      C80211_SUBTYPE_MGMT_ASS_REQUEST;
  MgmtFrame.FrameControl[1] = 0x00;
  MgmtFrame.DurationID = 0x8000;

  VNetMoveMemory (MgmtFrame.uDA, Adapter->CurrentBSSID, 6);
  VNetMoveMemory (MgmtFrame.uSA, Adapter->CurrentAddress, 6);
  VNetMoveMemory (MgmtFrame.uBSSID, Adapter->CurrentBSSID, 6);

  MgmtFrame.SequenceControl = 0x0000;

  //
  // Capability Information
  //
  if (Adapter->WepInfo.EncryptionLevel == 0)	//NO WEP 
    AssReqFrameBody->CapabilityInformation =
      (USHORT) C80211_MGMT_CAPABILITY_ESS;
  else
    AssReqFrameBody->CapabilityInformation =
      (USHORT) (C80211_MGMT_CAPABILITY_ESS | C80211_MGMT_CAPABILITY_Privacy);


  if (Adapter->PreambleType == SHORT_PREAMBLE)
    AssReqFrameBody->CapabilityInformation |=
      C80211_MGMT_CAPABILITY_ShortPreamble;

  //Listen Interval
  AssReqFrameBody->ListenInterval =
    Adapter->ListenInterval * Adapter->BeaconPeriod;


  MgmtFrame.Body[4] = C80211_MGMT_ElementID_SSID;
  MgmtFrame.Body[5] = Adapter->SSID_Size;
  VNetMoveMemory (&MgmtFrame.Body[6], Adapter->DesiredSSID,
		  Adapter->SSID_Size);
  MgmtFrame.Body[6 + Adapter->SSID_Size] =
    C80211_MGMT_ElementID_SupportedRates;
  MgmtFrame.Body[6 + Adapter->SSID_Size + 1] = 4;
  VNetMoveMemory (MgmtFrame.Body + 6 + Adapter->SSID_Size + 2,
		  (PVOID) & BasicRates[0][0], 4);


  FrameLen = 24 + 2 + 2 + 2 + Adapter->SSID_Size + 2 + 4;

  IF_LOUD (DbgPrint ("Sending Association Request...\n");
    )
    //
    // send Association Request frame
    //
    return TransmitMgmtFrame (Adapter, (PUCHAR) & MgmtFrame,
			      (USHORT) FrameLen);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    SendReAssociationRequest                                                                  
//
//    This function formats a ReAssociation Request Frame and calls               
//    TransmitMgmtFrame() to transmit it. Based on SendAssociationRequest 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SendReAssociationRequest (PVNet_ADAPTER Adapter)
{
  USHORT FrameLen;		//,i;
  s80211_MgmtFrm MgmtFrame;
  s80211_MGMT_REASS_REQUEST *pReAssReqFrameBody =
    (s80211_MGMT_REASS_REQUEST *) MgmtFrame.Body;

  IF_LOUD (DbgPrint ("ReAssociating....\n");
    )
    //
    //Create ReAssociation Request Frame Header
    //
    MgmtFrame.FrameControl[0] =
    C80211_PROTOCOL_VERSION | C80211_TYPE_MGMT |
    C80211_SUBTYPE_MGMT_REASS_REQUEST;
  MgmtFrame.FrameControl[1] = 0x00;
  MgmtFrame.DurationID = 0x8000;

  VNetMoveMemory (MgmtFrame.uDA, Adapter->CurrentBSSID, 6);
  VNetMoveMemory (MgmtFrame.uSA, Adapter->CurrentAddress, 6);
  VNetMoveMemory (MgmtFrame.uBSSID, Adapter->CurrentBSSID, 6);

  MgmtFrame.SequenceControl = 0x0000;

  //
  //Capability Information
  //
  if (Adapter->WepInfo.EncryptionLevel == 0)	//NO WEP 
    pReAssReqFrameBody->CapabilityInformation =
      (USHORT) C80211_MGMT_CAPABILITY_ESS;
  else
    pReAssReqFrameBody->CapabilityInformation =
      (USHORT) (C80211_MGMT_CAPABILITY_ESS | C80211_MGMT_CAPABILITY_Privacy);

  if (Adapter->PreambleType == SHORT_PREAMBLE)
    pReAssReqFrameBody->CapabilityInformation |=
      C80211_MGMT_CAPABILITY_ShortPreamble;

  //
  //Listen Interval
  //
  pReAssReqFrameBody->ListenInterval =
    Adapter->ListenInterval * Adapter->BeaconPeriod;

  VNetMoveMemory (&MgmtFrame.Body[4], Adapter->CurrentBSSID, 6);

  MgmtFrame.Body[10] = C80211_MGMT_ElementID_SSID;
  MgmtFrame.Body[11] = Adapter->SSID_Size;
  VNetMoveMemory (&MgmtFrame.Body[12], Adapter->DesiredSSID,
		  Adapter->SSID_Size);
  MgmtFrame.Body[12 + Adapter->SSID_Size] =
    C80211_MGMT_ElementID_SupportedRates;
  MgmtFrame.Body[12 + Adapter->SSID_Size + 1] = 4;
  VNetMoveMemory (MgmtFrame.Body + 12 + Adapter->SSID_Size + 2,
		  (PVOID) & BasicRates[0][0], 4);

  FrameLen = 24 + 2 + 2 + 6 + 2 + Adapter->SSID_Size + 2 + 4;

  IF_LOUD (DbgPrint ("Sending ReAssociation Request...\n");
    )return TransmitMgmtFrame (Adapter, (PUCHAR) & MgmtFrame,
			       (USHORT) FrameLen);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    TransmitMgmtFrame                                                                                               
//
//    This function submits to MAC a formated MGMT frame for transmission 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOLEAN
TransmitMgmtFrame (PVNet_ADAPTER Adapter,
		   PUCHAR pFrameBuffer, USHORT FrameSize)
{

  USHORT StartOfTxBuffer;

  Adapter->MgmtErrorCode = C80211_MGMT_SC_Success;

  if (!TxResourcesAvailable (Adapter, FrameSize, &StartOfTxBuffer))
    return FALSE;

  HostMemToCardMem16 (Adapter, StartOfTxBuffer, pFrameBuffer, FrameSize);

  Adapter->TxBuffTail += FrameSize;

  Adapter->Transmissions++;

  TxUpdateDescriptor (Adapter, FrameSize, StartOfTxBuffer, FALSE);

  return TRUE;


}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    RetrieveBSS                                                                                                                 
//
//      if a BSSID is specified during the call of this function it returns   
//  the index in the array of the coresponding AP. if no BSSID is                 
//  specified it returns the index of the AP in the list having the same  
//  SSID and the best RSSI. If SSID is NULL and we have not previously fail
//  to connect to this BSS, it returns the index of the AP with the best RSSI.                                                                                    
//  If no AP exists or no AP matches the requirements it returns -1.      
//                                                                                                                                                
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int
RetrieveBSS (PVNet_ADAPTER Adapter, PUCHAR BSSID)
{
  UCHAR i;
  CHAR MaxRssi;
  UCHAR MaxRssiIndex;
  BOOLEAN SpecificSSID = TRUE;
  BOOLEAN APfound = FALSE;

  if (Adapter->BSSListEntries == 0)
    {
      IF_LOUD (DbgPrint (" NO APs in the List...\n");
	)return -1;
    }

  if (BSSID == NULL)
    {
      //
      // Check if a specific SSID is used
      //
      if (Adapter->ConnectToAnyBSS)
	SpecificSSID = FALSE;

      MaxRssi = -128;
      MaxRssiIndex = 0;
      Adapter->CurrentBSSindex = 0;

      for (i = 0; i < Adapter->BSSListEntries; i++)
	{
	  if (SpecificSSID)
	    {
	      if (Adapter->SSID_Size == Adapter->BSSinfo[i].SSIDsize)
		{
		  if (!MemCompare
		      (Adapter->DesiredSSID, Adapter->BSSinfo[i].SSID,
		       Adapter->SSID_Size))
		    {
		      if (((Adapter->OperatingMode == AD_HOC_MODE && Adapter->BSSinfo[i].BSStype == BSS_TYPE_AD_HOC) || (Adapter->OperatingMode == INFRASTRUCTURE_MODE && Adapter->BSSinfo[i].BSStype == BSS_TYPE_INFRASTRUCTURE && ValidateChannelForDomain (Adapter, &Adapter->BSSinfo[i].Channel))))	//INT_ROAM
			{
			  APfound = TRUE;

			  if (Adapter->BSSinfo[i].RSSI >= MaxRssi)
			    {
			      IF_LOUD (DbgPrint
				       ("Max Rssi was %d at index %d-->%d at index %d\n",
					MaxRssi, MaxRssiIndex,
					Adapter->BSSinfo[i].RSSI, i);
				)MaxRssi = Adapter->BSSinfo[i].RSSI;
			      MaxRssiIndex = i;
			    }
			}
		    }
		}
	    }
	  else
	    {
	      //
	      // Select a BSS with the max-RSSI but of the same type and of the same WEP mode
	      // and that it is not marked as 'bad' (i.e. we had previously failed to connect to
	      // this BSS with the settings that we currently use)
	      //
	      if (((Adapter->OperatingMode == AD_HOC_MODE && Adapter->BSSinfo[i].BSStype == BSS_TYPE_AD_HOC) || (Adapter->OperatingMode == INFRASTRUCTURE_MODE && Adapter->BSSinfo[i].BSStype == BSS_TYPE_INFRASTRUCTURE)) && ((Adapter->WepInfo.EncryptionLevel == WEP_DISABLED && !Adapter->BSSinfo[i].UsingWEP) || (Adapter->WepInfo.EncryptionLevel != WEP_DISABLED && Adapter->BSSinfo[i].UsingWEP)) && !(Adapter->BSSinfo[i].Channel & 0x80))	//BL: (4.0.2.28) Is this entry marked as 'bad'?
		{
		  APfound = TRUE;
		  if (Adapter->BSSinfo[i].RSSI >= MaxRssi)
		    {
		      MaxRssi = Adapter->BSSinfo[i].RSSI;
		      MaxRssiIndex = i;
		      Adapter->CurrentBSSindex = i;
		    }
		}
	    }
	}

      if (APfound)
	{
	  IF_LOUD (DbgPrint
		   ("Max Rssi is %d in index = %d\n", MaxRssi, MaxRssiIndex);
	    )return MaxRssiIndex;
	}
    }
  else				//Specific BSSID
    {
      for (i = 0; i < Adapter->BSSListEntries; i++)
	{
	  if (!MemCompare (BSSID, Adapter->BSSinfo[i].BSSID, 6))
	    return i;
	}
    }

  return -1;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//  SelectBSS
//                                                                                                                        
//      After a Site Survey the user selected AP index is passed to this          
//      function to Join it.                                                                                              
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
SelectBSS (PVNet_ADAPTER Adapter, int BSSindex)
{
  if (BSSindex == 0xff)
    {
      IF_LOUD (DbgPrint ("Site Survey Stoped. NO AP SELECTED\n");
	)if (Adapter->OperatingMode == INFRASTRUCTURE_MODE)
	{
	  if (Adapter->StationIsAssociated)
	    {
	      IF_LOUD (int i;
		       DbgPrint ("SSID: ");
		       for (i = 0; i < Adapter->SSID_Size; i++)
		       DbgPrint ("%c", Adapter->DesiredSSID[i]);
		       DbgPrint ("\n");
		       DbgPrint ("BSSID: ");
		       for (i = 0; i < 6; i++)
		       DbgPrint ("%02X ", Adapter->CurrentBSSID[i]);
		       DbgPrint ("\n");)

		Adapter->ApSelected = TRUE;

	      Adapter->StationState = STATION_STATE_JOINNING;
	      Join (Adapter, BSS_TYPE_INFRASTRUCTURE);
	    }
	  else
	    Scan (Adapter, TRUE);
	}
      else			//Ad-Hoc
	{
	  Adapter->StationState = STATION_STATE_JOINNING;
	  Join (Adapter, BSS_TYPE_AD_HOC);
	}

    }
  else
    {
      IF_LOUD (DbgPrint ("Site Survey Stoped. BSS #%d selected\n", BSSindex);
	)if (BSSindex != -1)
	{
	  JoinSpecificBSS (Adapter, BSSindex, TRUE);
	}			//BSS index != -1


    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    DisplayMgmtFrameStatus                                                                                      
//
//      if a BSSID is specified during the call of this function it returns   
//  the index in the array of the coresponding AP. if no BSSID is                 
//  specified it returns the index of the AP in the list having the same  
//  SSID and the best RSSI. If SSID is any it simply returns the index of 
//  the AP with the best RSSI.                                                                                    
//  If no AP exists or no AP matches the requirements it returns -1.      
//                                                                                                                                                
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


VOID
DisplayMgmtFrameStatus (ULONG StatusCode)
{
  switch (StatusCode)
    {
    case C80211_MGMT_SC_Unspecified:
      IF_LOUD (DbgPrint ("Unspecified error in Management Frame...\n");
	)break;
    case C80211_MGMT_SC_AssDenied:
      IF_LOUD (DbgPrint ("Association Denied...\n");
	)break;
    case C80211_MGMT_SC_ReassDenied:
      IF_LOUD (DbgPrint ("Re-Association Denied...\n");
	)break;
    case C80211_MGMT_SC_AssDeniedHandleAP:
      IF_LOUD (DbgPrint
	       ("Association Denied(Access Point can not handle any more stations)...\n");
	)break;
    case C80211_MGMT_SC_AssDeniedBSSRate:
      IF_LOUD (DbgPrint
	       ("Association Denied (Supported Rates mismatch)...\n");
	)break;
    case C80211_MGMT_SC_SupportCapabilities:
      IF_LOUD (DbgPrint
	       ("SupportCapabilities error in Association Request...\n");
	)break;
    case C80211_MGMT_SC_AuthAlgNotSupported:
      IF_LOUD (DbgPrint ("Authentication Algorithm not supported...\n");
	)break;
    case C80211_MGMT_SC_AuthTransSeqNumError:
      IF_LOUD (DbgPrint
	       ("Bad Authentication transaction sequence number...\n");
	)break;
    case C80211_MGMT_SC_AuthRejectChallenge:
      IF_LOUD (DbgPrint ("Authentication challenge text rejection...\n");
	)break;
    case C80211_MGMT_SC_AuthRejectTimeout:
      IF_LOUD (DbgPrint ("Authentication Timeout...\n");
	)break;
    default:
      {
	IF_LOUD (DbgPrint ("Error 0x%X\n", StatusCode);
      )}
    }
}
