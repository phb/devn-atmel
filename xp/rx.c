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
/**   Functions used for Rx												  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "vnet.h"
#include "rx.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    GetChannelFromBeaconOrProbe                                                                       
//
//    Gets the Channel from a Beacon or Probe Response
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
GetChannelFromBeaconOrProbe (PUCHAR pBeaconOrProbeFrame, PUCHAR pChannel)
{
  UCHAR SSIDLen, SRatesLen;

  SSIDLen = *((PUCHAR) (pBeaconOrProbeFrame) + SSID_ELLEMENT_LENGTH_OFFSET);

  if (SSIDLen + SUP_RATES_ELLEMENT_LENGTH_REL_OFFSET < MIN_BEACON_PROBE_BYTES)
    {
      SRatesLen =
	*((PUCHAR) (pBeaconOrProbeFrame) + SSIDLen +
	  SUP_RATES_ELLEMENT_LENGTH_REL_OFFSET);
    }
  else
    return FALSE;

  if (SSIDLen + SRatesLen + DS_PARAM_SET_ELLEMENT_REL_OFFSET <
      MIN_BEACON_PROBE_BYTES)
    *pChannel =
      *((PUCHAR) (pBeaconOrProbeFrame) + SSIDLen + SRatesLen +
	DS_PARAM_SET_ELLEMENT_REL_OFFSET);
  else
    return FALSE;

  if (*pChannel >= 1 && *pChannel <= 14)
    return TRUE;
  else
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    GetSSIDfromBeaconOrProbe                                                                  
//
//    Gets the SSID and SSID length from a Beacon or Probe Response
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
GetSSIDfromBeaconOrProbe (PUCHAR pSSID, PUCHAR pSSIDlength,
			  PUCHAR pBeaconOrProbeFrame)
{
  UCHAR i;

  *pSSIDlength =
    *((PUCHAR) pBeaconOrProbeFrame + SSID_ELLEMENT_LENGTH_OFFSET);

  if (*pSSIDlength > MAX_SSID_LENGTH)
    return FALSE;

  for (i = 0; i < *pSSIDlength; i++)
    {
      *((PUCHAR) pSSID + i) =
	*((PUCHAR) pBeaconOrProbeFrame + SSID_ELLEMENT_OFFSET + i);
    }

  return TRUE;
}


//////////////////////////////////////////////////
//      
//      Rx Descriptor Manipulation Functions
//
//////////////////////////////////////////////////

void
GetRxDescFlags (PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR flags)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_FLAGS_OFFSET));
  CardReadUchar (Adapter, DR, flags);
}


void
SetRxDescFlags (PVNet_ADAPTER Adapter, USHORT Descriptor, UCHAR flags)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_FLAGS_OFFSET));
  CardWriteUchar (Adapter, DR, flags);
}


void
GetRxDescStatus (PVNet_ADAPTER Adapter, USHORT Descriptor, PUCHAR status)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_STATUS_OFFSET));
  CardReadUchar (Adapter, DR, status);
}

void
GetRxDescMsduSize (PVNet_ADAPTER Adapter, USHORT Descriptor, PUSHORT msdusize)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_MSDU_SIZE_OFFSET));
  CardReadUshort (Adapter, DR, msdusize);
}


void
GetRxDescMsduPos (PVNet_ADAPTER Adapter, USHORT Descriptor, PUSHORT msdupos)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_MSDU_POS_OFFSET));
  CardReadUshort (Adapter, DR, msdupos);
}


void
GetRxDescRssi (PVNet_ADAPTER Adapter, USHORT Descriptor, PCHAR rssi)
{
  int tmpRSSI = 0;

  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_RSSI_OFFSET));
  CardReadUchar (Adapter, DR, (PUCHAR) & tmpRSSI);

#if defined (REV_E)
  tmpRSSI = (4 * tmpRSSI);

  if (tmpRSSI > 100)
    tmpRSSI = 100;

#elif defined (PCI) || defined (R504) || defined (R504_C) || defined (R504_2958)
  tmpRSSI = -1 * (((98 - tmpRSSI) * 117) / 100);

  if (tmpRSSI < -90)
    tmpRSSI = -90;

#elif defined (RFMD) || defined (RFMD_3COM)
  tmpRSSI = (Adapter->LinkQuality / 2) + (tmpRSSI * 3);


  if (tmpRSSI > 100)
    tmpRSSI = 100;
#endif

  *rssi = (CHAR) tmpRSSI;

}


void
GetRxDescLinkQuality (PVNet_ADAPTER Adapter, USHORT Descriptor,
		      PUCHAR LinkQuality)
{
  CardWriteUshort (Adapter, AR,
		   (USHORT) (Adapter->HostInfo.RxDescPos +
			     (SIZE_OF_RXDESC * Descriptor) +
			     RX_DESC_LINK_QUALITY_OFFSET));
  CardReadUchar (Adapter, DR, LinkQuality);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    StoreBSSinfo                                                                                                               
//
//      Stores in an array all the AP information from a beacon or probe          
//  response frame. If a BSS already exists in the list the entry is 
//  updated if we are processing a Probe Response.                                                                                                                
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOLEAN
StoreBSSinfo (PVNet_ADAPTER Adapter, PUCHAR BSSID, PUCHAR pBeaconOrPrope,
	      UCHAR Channel, BOOLEAN IsProbe)
{
  int i, index = 0;
  BOOLEAN Valid;
  USHORT CapabilityInformation;
  BOOLEAN BSSexists;
  CHAR tmpRSSI;

  BSSexists = FALSE;
  for (i = 0; i < Adapter->BSSListEntries; i++)
    {
      if (!MemCompare (BSSID, Adapter->BSSinfo[i].BSSID, 6))
	{
	  index = i;
	  BSSexists = TRUE;
	}
    }

  if (!BSSexists)
    {
      if (Adapter->BSSListEntries == MAX_BSS_ENTRIES)
	{
	  IF_LOUD (DbgPrint ("** AP list is full ***\n");
	    )return FALSE;
	}

      index = Adapter->BSSListEntries;
    }
  else				//BSS exists in list
    {
      // If we process a probe and an entry from this BSS exists 
      // we will update the BSS entry with the info from this BSS.
      // If we process a beacon we will only update RSSI if the new 
      // beacon has better RSSI.

      if (!IsProbe)
	{
	  GetRxDescRssi (Adapter, Adapter->RxDescHead, &tmpRSSI);
	  if (tmpRSSI > Adapter->BSSinfo[index].RSSI)
	    {
	      Adapter->BSSinfo[index].RSSI = tmpRSSI;
	    }

	  return TRUE;
	}
    }

  Adapter->BSSinfo[index].Channel = Channel;

  Valid = GetSSIDfromBeaconOrProbe (Adapter->BSSinfo[index].SSID,
				    &Adapter->BSSinfo[index].SSIDsize,
				    (PUCHAR) pBeaconOrPrope);

  if (!Valid)
    {
      IF_DEBUG_ERRORS (DbgPrint
		       ("*** Invalid Beacon or Probe Response Frame (S)***\n");
	)return FALSE;
    }

  Adapter->BSSinfo[index].BeaconPeriod =
    *(PUSHORT) (pBeaconOrPrope + BEACON_INTERVAL_OFFSET);

  CapabilityInformation =
    *(pBeaconOrPrope + CAPABILITY_INFO_IN_BEACON_OFFSET);


  if (CapabilityInformation & C80211_MGMT_CAPABILITY_Privacy)
    {
      Adapter->BSSinfo[index].UsingWEP = 1;
    }
  else
    {
      Adapter->BSSinfo[index].UsingWEP = 0;
    }

  if (CapabilityInformation & C80211_MGMT_CAPABILITY_IBSS)
    Adapter->BSSinfo[index].BSStype = BSS_TYPE_AD_HOC;
  else if (CapabilityInformation & C80211_MGMT_CAPABILITY_ESS)
    Adapter->BSSinfo[index].BSStype = BSS_TYPE_INFRASTRUCTURE;


  if (CapabilityInformation & C80211_MGMT_CAPABILITY_ShortPreamble)
    Adapter->BSSinfo[index].PreambleType = SHORT_PREAMBLE;
  else
    Adapter->BSSinfo[index].PreambleType = LONG_PREAMBLE;


  VNetMoveMemory (Adapter->BSSinfo[index].BSSID, BSSID, 6);

  GetRxDescRssi (Adapter, Adapter->RxDescHead, &tmpRSSI);

  if (BSSexists)
    {
      if (tmpRSSI > Adapter->BSSinfo[index].RSSI)
	{
	  Adapter->BSSinfo[index].RSSI = tmpRSSI;
	}
    }
  else
    {
      Adapter->BSSinfo[index].RSSI = tmpRSSI;
    }

  if (!BSSexists)
    {
      Adapter->BSSListEntries++;
    }
  return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    MgmtFrameRxProcessing                                                                                               
//
//        Processes the Rx MGMT frames and implements the necessary actions needed 
//        until the station becomes associated.
//                                                                                                                                                
//              -Supports ANY ssid                                                                                                
//              -Sets time-outs for the mgmt frames                                                               
//              -Fullly supports WEP (challenge text/authentication transaction   
//               sequence numbers).                                                                                                     
//              -"Decides" actions according to the received status codes                 
//              -Supports Reassocioation Response frames                                                        
//              -Supports Deauthentication & Disassociation Frames                                
//      -Supports Prefeared BSSID                                                                                 
//                                                                                                                                        
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
MgmtFrameRxProcessing (PVNet_ADAPTER Adapter,
		       UCHAR FrameSubtype, USHORT FrameLength)
{
  UCHAR AuthenticationFrame[261];	//max auth frame size
  UCHAR AssociationRspnsFrame[16];
  UCHAR BeaconOrProbeRespFrame[MIN_BEACON_PROBE_BYTES];	//60 is enough for the fields we are intereted in (59 bytes is up to DS Param Set)
  USHORT StatusCode;
  int i;
  UCHAR ChallengeTextLength;
  //BOOLEAN                                                       CompareSSIDs = TRUE;
  USHORT BeaconOrProbeRespFrameBytesToGet;
  USHORT AuthenticationFrameBytesToGet;
  USHORT AssociationRespFrameBytesToGet;
  USHORT CapabilityInformation;
  UCHAR SourceAddress[6];
  BOOLEAN MgmtFrameFromCurrentBSS;
  USHORT AssocID;
  UCHAR SupportedRatesLength;
  UCHAR SupportedRates[OPER_RATE_SIZE];
  BOOLEAN IsProbe;
  UCHAR Address3[6];
  BOOLEAN PreambleChanged = FALSE;
  BOOLEAN ChannelChanged = FALSE;
  UCHAR Channel;
  CHAR tmpRSSI;
  int BSSindex;


  VNetMoveMemory (SourceAddress, &Adapter->RxBuf[MGMT_FRAME_SA_OFFSET], 6);
  VNetMoveMemory (Address3, &Adapter->RxBuf[DATA_FRAME_ADHOC_ADR3_OFFSET], 6);

  MgmtFrameFromCurrentBSS = TRUE;

  if (Adapter->OperatingMode == AD_HOC_MODE)
    {
      for (i = 0; i < 6; i++)
	if (Address3[i] != Adapter->CurrentBSSID[i])
	  {
	    MgmtFrameFromCurrentBSS = FALSE;
	    break;
	  }
    }
  else
    {
      for (i = 0; i < 6; i++)
	if (SourceAddress[i] != Adapter->CurrentBSSID[i])
	  {
	    MgmtFrameFromCurrentBSS = FALSE;
	    break;
	  }
    }


  switch (FrameSubtype)
    {
    case C80211_SUBTYPE_MGMT_BEACON:
    case C80211_SUBTYPE_MGMT_ProbeResponse:

      if (FrameSubtype == C80211_SUBTYPE_MGMT_ProbeResponse)
	IsProbe = TRUE;
      else
	IsProbe = FALSE;

      if (FrameLength < 16)
	{
	  IF_DEBUG_ERRORS (DbgPrint
			   ("*** Too short (%d) Beacon or Probe Response ***\n",
			    FrameLength);
	    )break;
	}

      BeaconOrProbeRespFrameBytesToGet =
	(FrameLength <
	 MIN_BEACON_PROBE_BYTES) ? FrameLength : MIN_BEACON_PROBE_BYTES;

      VNetMoveMemory ((PUCHAR) BeaconOrProbeRespFrame,
		      (PUCHAR) & Adapter->RxBuf[MGMT_FRAME_BODY_OFFSET],
		      BeaconOrProbeRespFrameBytesToGet);

      CapabilityInformation =
	*(BeaconOrProbeRespFrame + CAPABILITY_INFO_IN_BEACON_OFFSET);
      if (!GetChannelFromBeaconOrProbe
	  ((PUCHAR) BeaconOrProbeRespFrame, &Channel))
	{
	  IF_DEBUG_ERRORS (DbgPrint
			   ("*** Invalid Beacon or Probe Response Frame (Ch)***\n");
	    )break;
	}

      if (FrameSubtype == C80211_SUBTYPE_MGMT_BEACON)
	Adapter->Stats.MatchingBeacons++;

      if (Adapter->StationState == STATION_STATE_READY)
	{
	  if (MgmtFrameFromCurrentBSS)
	    {
	      if (!IsProbe)
		Adapter->PeriodicBeaconsCnt++;

	      if (Adapter->AutoPreambleDetection)
		{
		  if ((Adapter->PreambleType == LONG_PREAMBLE)
		      && (CapabilityInformation &
			  C80211_MGMT_CAPABILITY_ShortPreamble))
		    {
		      IF_LOUD (DbgPrint
			       ("*** AUTO SWITCH to SHORT preamble ***\n");
			)Adapter->PreambleType = SHORT_PREAMBLE;
		      PreambleChanged = TRUE;
		    }
		  else
		    if ((Adapter->PreambleType == SHORT_PREAMBLE)
			&& !(CapabilityInformation &
			     C80211_MGMT_CAPABILITY_ShortPreamble))
		    {
		      IF_LOUD (DbgPrint
			       ("*** AUTO SWITCH to LONG preamble ***\n");
			)Adapter->PreambleType = LONG_PREAMBLE;
		      PreambleChanged = TRUE;
		    }
		}

	      if (Adapter->Channel != Channel)
		{
		  IF_LOUD (DbgPrint ("*** CHANNEL CHANGED ***\n");
		    )ChannelChanged = TRUE;
		  Adapter->Channel = Channel;
		  SetChannel (Adapter, Adapter->Channel);
		}

	      if (PreambleChanged || ChannelChanged)
		{
		  if (PreambleChanged)
		    {
		      SetPreambleType (Adapter, Adapter->PreambleType);
		    }

		  Adapter->StationIsAssociated = FALSE;
		  Adapter->StationState = STATION_STATE_JOINNING;

		  if (Adapter->OperatingMode == INFRASTRUCTURE_MODE)
		    Join (Adapter, BSS_TYPE_INFRASTRUCTURE);
		  else
		    Join (Adapter, BSS_TYPE_AD_HOC);
		}
	    }

	  GetRxDescRssi (Adapter, Adapter->RxDescHead, &tmpRSSI);

	  if (tmpRSSI)
	    {
	      if ((tmpRSSI + Adapter->Rssi) % 2)
		Adapter->Rssi = ((tmpRSSI + Adapter->Rssi) / 2) + 1;
	      else
		Adapter->Rssi = ((tmpRSSI + Adapter->Rssi) / 2);
	    }
	}
      else if (Adapter->StationState == STATION_STATE_SCANNING)
	{
	  if ((CapabilityInformation & 0x0001))
	    StoreBSSinfo (Adapter, SourceAddress,
			  (PUCHAR) BeaconOrProbeRespFrame, Channel, IsProbe);
	  else
	    StoreBSSinfo (Adapter, Address3, (PUCHAR) BeaconOrProbeRespFrame,
			  Channel, IsProbe);
	}

      break;

    case C80211_SUBTYPE_MGMT_Authentication:

      AuthenticationFrameBytesToGet = (FrameLength < 261) ? FrameLength : 261;

      VNetMoveMemory (AuthenticationFrame,
		      &Adapter->RxBuf[MGMT_FRAME_BODY_OFFSET],
		      AuthenticationFrameBytesToGet);

      StatusCode = AuthenticationFrame[STATUS_IN_AUTHENTICATION_OFFSET];


      if (Adapter->StationState == STATION_STATE_AUTHENTICATING)
	{
	  IF_LOUD (DbgPrint ("C80211_SUBTYPE_MGMT_Authentication...\n ");
	    )if (StatusCode == C80211_MGMT_SC_Success)
	    {
	      if (Adapter->WepInfo.EncryptionLevel != 0 && (Adapter->WepInfo.AuthenticationType == C80211_MGMT_AAN_SHAREDKEY))	//WEP
		{
		  if (AuthenticationFrame
		      [TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET] ==
		      Adapter->ExpectedAuthentTransactionSeqNum)
		    {
		      if (AuthenticationFrame
			  [TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET] == 0x0002)
			{
			  IF_LOUD (DbgPrint
				   ("AUTHENTICATION with TrSeqNo: %d (ELID %d)\n",
				    AuthenticationFrame
				    [TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET],
				    AuthenticationFrame
				    [CHALL_TEXT_EL_ID_IN_AUTHENTICATION_OFFSET]);
			    )if (AuthenticationFrame
				 [CHALL_TEXT_EL_ID_IN_AUTHENTICATION_OFFSET]
				 == C80211_MGMT_ElementID_ChallengeText)
			    {
			      ChallengeTextLength =
				AuthenticationFrame
				[CHALL_TEXT_LENGTH_IN_AUTHENTICATION_OFFSET];
			      IF_LOUD (DbgPrint
				       ("Challenge Text Length = %d\n",
					ChallengeTextLength);
				)VNET_ASSERT (ChallengeTextLength <= 253);

			      SendAuthenticationRequest (Adapter,
							 &AuthenticationFrame
							 [CHALL_TEXT_IN_AUTHENTICATION_OFFSET],
							 ChallengeTextLength);
			    }
			}
		      else
			if (AuthenticationFrame
			    [TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET] == 0x0004)
			{
			  IF_LOUD (DbgPrint
				   ("AUTHENTICATION with TrSeqNo: %d\n",
				    AuthenticationFrame
				    [TRANS_SEQ_NO_IN_AUTHENTICATION_OFFSET]);
			    )IF_LOUD (DbgPrint ("### AUTHENTICATED ###\n");
			    )if (!Adapter->StationWasAssociated)
			    {
			      Adapter->StationState =
				STATION_STATE_ASSOCIATING;
			      SendAssociationRequest (Adapter);
			    }
			  else
			    {
			      Adapter->StationState =
				STATION_STATE_REASSOCIATING;
			      SendReAssociationRequest (Adapter);
			    }

			}

		    }
		}
	      else
		{
		  IF_LOUD (DbgPrint ("### AUTHENTICATED ###\n");
		    )if (!Adapter->StationWasAssociated)
		    {
		      Adapter->StationState = STATION_STATE_ASSOCIATING;
		      SendAssociationRequest (Adapter);
		    }
		  else
		    {
		      Adapter->StationState = STATION_STATE_REASSOCIATING;
		      SendReAssociationRequest (Adapter);
		    }
		}
	    }
	  else
	    {
	      Adapter->MgmtErrorCode = StatusCode;
	      DisplayMgmtFrameStatus (StatusCode);

	      if (StatusCode == C80211_MGMT_SC_AuthAlgNotSupported)
		{
		  IF_LOUD (DbgPrint
			   ("####### Authentication Algorithm Not Supported #######\n");
		    )if (Adapter->AutoSwitchAuthAlgorithm
			 && Adapter->AuthenticationRequestRetryCnt <
			 MAX_AUTHENTICATION_RETRIES)
		    {
		      IF_LOUD (DbgPrint
			       (">>>>>>> Retry with other Authentication Type...\n");
			)Adapter->StationState = STATION_STATE_AUTHENTICATING;

		      if (Adapter->WepInfo.AuthenticationType ==
			  C80211_MGMT_AAN_SHAREDKEY)
			Adapter->WepInfo.AuthenticationType =
			  C80211_MGMT_AAN_OPENSYSTEM;
		      else if (Adapter->WepInfo.AuthenticationType ==
			       C80211_MGMT_AAN_OPENSYSTEM)
			Adapter->WepInfo.AuthenticationType =
			  C80211_MGMT_AAN_SHAREDKEY;

		      Adapter->AuthenticationRequestRetryCnt++;
		      SetMgmtTimer (Adapter);

		      Adapter->CurrentAuthentTransactionSeqNum = 0x0001;
		      SendAuthenticationRequest (Adapter, NULL, 0);
		    }
		  else
		    {
		      if (Adapter->ConnectToAnyBSS)
			{
			  //
			  //BL: (4.0.2.28) invalidate list entry...
			  //
			  IF_LOUD (DbgPrint
				   ("**** Try Another BSS-AU (channel |= 0x80 for BSS #%d)...\n",
				    Adapter->CurrentBSSindex);
			    )Adapter->
			    BSSinfo[(int) (Adapter->CurrentBSSindex)].
			    Channel |= 0x80;

			  BSSindex = RetrieveBSS (Adapter, NULL);
			  if (BSSindex != -1)
			    {
			      IF_LOUD (DbgPrint
				       ("**** Try BSS #%d in channel %d...\n",
					BSSindex,
					Adapter->BSSinfo[BSSindex].Channel);
				)JoinSpecificBSS (Adapter, BSSindex, FALSE);
			    }
			  else
			    {
			      Adapter->AuthenticationRequestRetryCnt = 0;
			      Adapter->StationState =
				STATION_STATE_MGMT_ERROR;
			      Adapter->StationIsAssociated = FALSE;
			    }
			}
		      else
			{
			  Adapter->AuthenticationRequestRetryCnt = 0;
			  Adapter->StationState = STATION_STATE_MGMT_ERROR;
			  Adapter->StationIsAssociated = FALSE;
			}
		    }
		}
	    }
	}
      break;

    case C80211_SUBTYPE_MGMT_ASS_RESPONSE:
    case C80211_SUBTYPE_MGMT_REASS_RESPONSE:

      if (Adapter->StationState == STATION_STATE_ASSOCIATING
	  || Adapter->StationState == STATION_STATE_REASSOCIATING)
	{
	  AssociationRespFrameBytesToGet =
	    (FrameLength < 16) ? FrameLength : 16;
	  VNetMoveMemory (AssociationRspnsFrame,
			  &Adapter->RxBuf[MGMT_FRAME_BODY_OFFSET],
			  AssociationRespFrameBytesToGet);

	  StatusCode =
	    AssociationRspnsFrame[STATUS_IN_ASSOCIATION_RESP_OFFSET];
	  AssocID = AssociationRspnsFrame[ASS_ID_IN_ASSOCIATION_RESP_OFFSET];
	  SupportedRatesLength =
	    AssociationRspnsFrame
	    [SUP_RATES_LENGTH_IN_ASSOCIATION_RESP_OFFSET];

	  if (SupportedRatesLength > OPER_RATE_SIZE)
	    {			//for safety...
	      SupportedRatesLength = OPER_RATE_SIZE;
	      IF_DEBUG_ERRORS (DbgPrint
			       ("*** Invalid Supported Rates Length ***\n");
	    )}

	  VNetMoveMemory (SupportedRates,
			  &AssociationRspnsFrame
			  [SUP_RATES_IN_ASSOCIATION_RESP_OFFSET],
			  SupportedRatesLength);


	  if (StatusCode == C80211_MGMT_SC_Success)
	    {
	      if (FrameSubtype == C80211_SUBTYPE_MGMT_ASS_RESPONSE)
		Adapter->AssociationRequestRetryCnt = 0;
	      else
		Adapter->ReAssociationRequestRetryCnt = 0;

	      if (MemCompare (Adapter->CurrentBSSID, Adapter->LastBSSID, 6))	//if associated to new BSS keep it in LastBSSID
		{
		  VNetMoveMemory (Adapter->LastBSSID, Adapter->CurrentBSSID,
				  6);
		  IndicateConnectionStatus (Adapter, TRUE);
		}

	      Adapter->StationWasAssociated = TRUE;

	      IF_LOUD (DbgPrint
		       ("Association Success (Assoc-ID = 0x%X - Supported Rates = %02X-%02X-%02X-%02X)\n",
			AssocID, SupportedRates[0], SupportedRates[1],
			SupportedRates[2], SupportedRates[3]);
		)
	      AssocID = AssocID & 0x3fff;
	      SetAssociationID (Adapter, AssocID);
	      SetSupportedRates (Adapter, SupportedRates,
				 SupportedRatesLength);

	      SetPowerMgmtMode (Adapter, Adapter->PowerMgmtMode);

	      Adapter->PeriodicBeaconsCnt = 0;

	      Adapter->StationIsAssociated = TRUE;
	      Adapter->StationState = STATION_STATE_READY;
	    }
	  else
	    {
	      Adapter->MgmtErrorCode = StatusCode;
	      DisplayMgmtFrameStatus (StatusCode);

	      if (FrameSubtype == C80211_SUBTYPE_MGMT_ASS_RESPONSE)
		{
		  if ((StatusCode != C80211_MGMT_SC_AssDeniedBSSRate) &&
		      (StatusCode != C80211_MGMT_SC_SupportCapabilities) &&
		      (Adapter->AssociationRequestRetryCnt <
		       MAX_ASSOCIATION_RETRIES))
		    {
		      SetMgmtTimer (Adapter);
		      Adapter->AssociationRequestRetryCnt++;
		      SendAssociationRequest (Adapter);
		    }
		  else
		    {
		      Adapter->StationState = STATION_STATE_MGMT_ERROR;
		      Adapter->StationIsAssociated = FALSE;
		    }
		}
	      else
		{
		  if ((StatusCode != C80211_MGMT_SC_AssDeniedBSSRate) &&
		      (StatusCode != C80211_MGMT_SC_SupportCapabilities) &&
		      (Adapter->ReAssociationRequestRetryCnt <
		       MAX_ASSOCIATION_RETRIES))
		    {
		      SetMgmtTimer (Adapter);
		      Adapter->ReAssociationRequestRetryCnt++;
		      SendReAssociationRequest (Adapter);
		    }
		  else
		    {
		      Adapter->StationState = STATION_STATE_MGMT_ERROR;
		      Adapter->StationIsAssociated = FALSE;
		    }
		}
	      if (Adapter->StationState == STATION_STATE_MGMT_ERROR)
		{
		  if (Adapter->ConnectToAnyBSS)
		    {
		      //
		      //BL: (4.0.2.28) invalidate list entry...
		      //
		      IF_LOUD (DbgPrint
			       ("**** Try Another BSS-AS (channel |= 0x80) for BSS #%d...\n",
				Adapter->CurrentBSSindex);
			)Adapter->BSSinfo[(int) (Adapter->CurrentBSSindex)].
			Channel |= 0x80;

		      BSSindex = RetrieveBSS (Adapter, NULL);
		      if (BSSindex != -1)
			{
			  IF_LOUD (DbgPrint
				   ("**** Try BSS #%d in channel %d...\n",
				    BSSindex,
				    Adapter->BSSinfo[BSSindex].Channel);
			    )JoinSpecificBSS (Adapter, BSSindex, FALSE);
			}
		    }
		}
	    }
	}

      break;

    case C80211_SUBTYPE_MGMT_DISASSOSIATION:
      if (Adapter->StationIsAssociated
	  && (Adapter->OperatingMode == INFRASTRUCTURE_MODE)
	  && MgmtFrameFromCurrentBSS)
	{

	  IF_LOUD (DbgPrint ("C80211_SUBTYPE_MGMT_DISASSOSIATION...\n");
	    )Adapter->StationWasAssociated = FALSE;
	  Adapter->StationIsAssociated = FALSE;

	  Adapter->StationState = STATION_STATE_JOINNING;

	  Join (Adapter, BSS_TYPE_INFRASTRUCTURE);
	}
      break;

    case C80211_SUBTYPE_MGMT_Deauthentication:
      if (Adapter->OperatingMode == INFRASTRUCTURE_MODE
	  && MgmtFrameFromCurrentBSS)
	{
	  IF_LOUD (DbgPrint ("C80211_SUBTYPE_MGMT_Deauthentication...\n");
	    )Adapter->StationWasAssociated = FALSE;

	  Adapter->StationState = STATION_STATE_JOINNING;

	  Join (Adapter, BSS_TYPE_INFRASTRUCTURE);
	}

      break;
    }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    RxInit                                                                                                                     
//
//    This function initializes Rx related parameters in the Adapter      
//        object and enables Rx operation on the MAC through the coresponding 
//        field of the IFACE.                                                                                             
//                                                                        
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


VOID
RxInit (PVNet_ADAPTER Adapter)
{
  UCHAR configuration;

  IF_VERY_LOUD (DbgPrint ("Initializing Rx Section....\n");
    )
    //
    //      Initialize Rx Descriptors counters and pointers
    //
    Adapter->RxDescHead = 0;

  Adapter->RxBuffStart = Adapter->HostInfo.RxBuffPos;
  Adapter->RxBuffEnd =
    Adapter->HostInfo.RxBuffPos + Adapter->HostInfo.RxBuffSize;


  Adapter->Receptions = 0;

  //
  //      Enable Rx Operation on the card.
  //
  GetFuncCtrl (Adapter, &configuration);
  configuration |= FUNC_CTRL_RxENABLE;
  SetFuncCtrl (Adapter, &configuration);

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    RxCompleteDPC                                                                                                               
//
//              Indicates any received packets to the NDIS wrapper. Any errored   
//              frames or Frames that are not of type DATA are dropped by the     
//              driver, and not indicated to the NDIS                                                     
//              wrapper.                                                                                                                  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
RxCompleteDPC (PVNet_ADAPTER Adapter)
{

  ULONG IndicateLen;
  ULONG PacketLength;
  UCHAR status;
  USHORT msduSize;
  UCHAR flags;
  UCHAR FrameType = 0, FrameSubtype;
  BOOLEAN RxFrameToBeIndicated = FALSE;
  BOOLEAN Address4Used = FALSE;
  UCHAR loopcnt;

#ifdef RX_CRC
  ULONG crc = 0;
  ULONG swcrc = 0;
  UCHAR MoreFragments = 0;
  UCHAR PacketFragmentNo;
  USHORT PacketSequenceNo;
#endif

  ULONG RxPacketLoc;
  UCHAR SourceAddr[6];
  UCHAR StartOfEthPacket;

  Adapter->IndicateReceiveDone = FALSE;

  GetRxDescFlags (Adapter, Adapter->RxDescHead, &flags);
	
  loopcnt = 0;
  while (flags == RX_DESC_FLAG_VALID && flags != 0xff
	 && loopcnt < Adapter->HostInfo.RxDescCount)
    {
      loopcnt++;
      Adapter->Receptions++;

      GetRxDescStatus (Adapter, Adapter->RxDescHead, &status);
      GetRxDescMsduSize (Adapter, Adapter->RxDescHead, &msduSize);

#ifdef RX_CRC
      // 
      msduSize -= (USHORT) 4;	// The 4 extra CRC bytes that F/W gives to Host are only necessery
      // for comparison with the CRC that Host calculates
      // The real packet length is msduSize - 4
      //
#endif

#if (!defined RFMD) && (!defined RFMD_3COM)
      GetRxDescLinkQuality (Adapter, Adapter->RxDescHead,
			    &Adapter->LinkQuality);
#endif

      if ((status == RX_STATUS_SUCCESS))	// RX with SUCCESS
	{

/*	  IF_DEBUG_RX (DbgPrint
		       ("Rx #%d (Status = 0x%X - Bytes = %d\n",
			Adapter->Receptions, status, msduSize);
	    )*/
	    //
	    // Get packets location in RxPacketLoc
	    //
	    CardMemToHostMem16 (Adapter, (PUCHAR) & RxPacketLoc,
				(Adapter->HostInfo.RxDescPos +
				 (SIZE_OF_RXDESC * Adapter->RxDescHead) +
				 RX_DESC_MSDU_POS_OFFSET), 2);

#ifdef RX_CRC
	  //
	  // we had decrement msduSize by 4. However we need them for CRC comparison.  
	  //
	  CardMemToHostMem16 (Adapter, Adapter->RxBuf, RxPacketLoc,
			      msduSize + 4);
#else
	  CardMemToHostMem16 (Adapter, Adapter->RxBuf, RxPacketLoc, msduSize);
#endif
	  FrameType = Adapter->RxBuf[0] & C80211_TYPE_MASK;
	  FrameSubtype = Adapter->RxBuf[0] & C80211_SUBTYPE_MASK;

#ifdef RX_CRC

	  //
	  // Get the CRC calculated in the device
	  //
	  VNetMoveMemory (&crc, &Adapter->RxBuf[msduSize], 4);

	  //
	  // Calculate CRC locally (which part of the packet is used for CRC calculation depends on
	  // the WEP usage
	  //
	  if ((Adapter->WepInfo.EncryptionLevel != 0)
	      && ((Adapter->RxBuf[1] & 0x40) != 0))
	    {			//WEP is ON
	      swcrc =
		Calculate_CRC32 ((Adapter->RxBuf + 24), (msduSize - 24),
				 Adapter->CrcTable);
	    }
	  else
	    {			//WEP is OFF
	      swcrc =
		Calculate_CRC32 (Adapter->RxBuf, msduSize, Adapter->CrcTable);
	    }

	  if (crc != swcrc)	//CRC error 
	    {
	      IF_LOUD (DbgPrint
		       ("### Rx Crc Error (%02X%02X)\n", Adapter->RxBuf[22],
			Adapter->RxBuf[23]);
		)Adapter->RxCrcError++;

	      SetRxDescFlags (Adapter, Adapter->RxDescHead,
			      RX_DESC_FLAG_CONSUMED);

	      if (Adapter->RxDescHead < (Adapter->HostInfo.RxDescCount - 1))
		Adapter->RxDescHead++;
	      else
		Adapter->RxDescHead = 0;

	      GetRxDescFlags (Adapter, Adapter->RxDescHead, &flags);
	      continue;
	    }
#endif //RX_CRC

	  if ((FrameType ==
	       C80211_TYPE_DATA)
	      /*&& (Adapter->StationState == STATION_STATE_READY) */ )
	    {			// Data Frame & Station Ready
#ifdef RX_CRC
	      //
	      // In case of CRC calculation on driver the packet reasembly must also be
	      // implemented on driver
	      //
	      MoreFragments = (Adapter->RxBuf[1] & 0x04);
	      PacketFragmentNo = (Adapter->RxBuf[22] & 0x0f);
	      PacketSequenceNo =
		(Adapter->RxBuf[23] << 8) | (Adapter->RxBuf[22] & 0xf0);

	      if (Adapter->OperatingMode == AD_HOC_MODE)
		VNetMoveMemory (SourceAddr, &Adapter->RxBuf[10], 6);
	      else
		VNetMoveMemory (SourceAddr, &Adapter->RxBuf[16], 6);


	      if (MoreFragments == 0 && PacketFragmentNo == 0)
		{		// This is a non-fragmented packet. Process it imediately
		  RxFrameToBeIndicated = TRUE;
		}
	      else if (MoreFragments != 0 && PacketFragmentNo == 0)
		{		// This is the first Fragment of the packet

		  IF_DEBUG_RX (DbgPrint
			       (">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Start of FRAGMENT FragNo = %d - SeqNo = %d - msduSize = %d\n",
				PacketFragmentNo, PacketSequenceNo, msduSize);
		    )VNetMoveMemory (Adapter->FragPacket.SourceAddress,
				     SourceAddr, 6);
		  Adapter->FragPacket.Sequence = PacketSequenceNo;
		  Adapter->FragPacket.FragmentNumber = 1;
		  VNetMoveMemory ((Adapter->FragPacket.WirelessPacket),
				  Adapter->RxBuf, msduSize);

		  Adapter->FragPacket.CurrentLen = msduSize;
		}
	      else if (MoreFragments != 0 &&
		       Adapter->FragPacket.FragmentNumber == PacketFragmentNo
		       && Adapter->FragPacket.Sequence == PacketSequenceNo
		       && !MemCompare (Adapter->FragPacket.SourceAddress,
				       SourceAddr, 6))
		{		// This is a packet fragment
		  // Copy the fragment

		  IF_DEBUG_RX (DbgPrint
			       ("Fragment FragNo = %d - SeqNo = %d - msduSize = %d\n",
				PacketFragmentNo, PacketSequenceNo, msduSize);
		    )VNetMoveMemory ((Adapter->FragPacket.WirelessPacket +
				      Adapter->FragPacket.CurrentLen),
				     (Adapter->RxBuf + 24), msduSize - 24);

		  Adapter->FragPacket.CurrentLen += (msduSize - 24);

		  Adapter->FragPacket.FragmentNumber++;
		}
	      else if (MoreFragments == 0 &&
		       Adapter->FragPacket.FragmentNumber == PacketFragmentNo
		       && Adapter->FragPacket.Sequence == PacketSequenceNo
		       && !MemCompare (Adapter->FragPacket.SourceAddress,
				       SourceAddr, 6))
		{		// This is the last fragment of the packet. This will make the packet complete.
		  // Copy the last fragment

		  IF_DEBUG_RX (DbgPrint
			       ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<End of FRAGMENT FragNo = %d - SeqNo = %d - msduSize = %d\n",
				PacketFragmentNo, PacketSequenceNo, msduSize);
		    )VNetMoveMemory ((Adapter->FragPacket.WirelessPacket +
				      Adapter->FragPacket.CurrentLen),
				     (Adapter->RxBuf + 24), (msduSize - 24));

		  Adapter->FragPacket.CurrentLen += (msduSize - 24);

		  //
		  // msduSize must keep the whole packet size
		  //
		  msduSize = Adapter->FragPacket.CurrentLen;

		  // move the whole thing to RxBuf for further processing

		  VNetMoveMemory (Adapter->RxBuf,
				  Adapter->FragPacket.WirelessPacket,
				  Adapter->FragPacket.CurrentLen);

		  //
		  // Clear the Adapter->FragPacket fields
		  //
		  Adapter->FragPacket.Sequence = 0;
		  Adapter->FragPacket.CurrentLen = 0;
		  VNetZeroMemory (Adapter->FragPacket.SourceAddress, 6);
		  Adapter->FragPacket.FragmentNumber = 1;
		  //
		  // Set the flag for further processing
		  //
		  RxFrameToBeIndicated = TRUE;
		}
#else
// no CRC and Re-assembly in driver                             
	      RxFrameToBeIndicated = TRUE;
#endif //RX_CRC
	      //
	      // Determine Packet Type
	      //
	      if ((!MemCompare (&Adapter->RxBuf[24], SNAP_ETHERNETII, 6) &&
		   !MemCompare (&Adapter->RxBuf[30],
				STATIC_ENTRIES_ETHERTYPE_8137, 2))
		  || (Adapter->RxBuf[24] == 0xaa && Adapter->RxBuf[25] == 0xaa
		      && MemCompare (&Adapter->RxBuf[30],
				     STATIC_ENTRIES_ETHERTYPE_8137, 2)
		      && MemCompare (&Adapter->RxBuf[30],
				     STATIC_ENTRIES_ETHERTYPE_80F3, 2)))
		{		// IP packet
		  Address4Used = FALSE;
		}
	      else
		{		// non-IP packet
		  Address4Used = TRUE;
		}

	    }
	  else if (FrameType == C80211_TYPE_MGMT)
	    {
	      RxFrameToBeIndicated = FALSE;
	      Adapter->Stats.RxMgmtPacketsOk++;
	      MgmtFrameRxProcessing (Adapter, FrameSubtype, msduSize);
	    }
	  else
	    RxFrameToBeIndicated = FALSE;


	  if (RxFrameToBeIndicated)
	    {

	      //
	      // Source Address resides at offset 10 or 16 according to OpMode
	      // Destination Address is alway at offset 4 and we do not overwrite it
	      //
	      if (Adapter->OperatingMode == AD_HOC_MODE)
		VNetMoveMemory (SourceAddr, &Adapter->RxBuf[10], 6);
	      else
		VNetMoveMemory (SourceAddr, &Adapter->RxBuf[16], 6);

	      if (Address4Used)	//non-IP packet
		{
		  //
		  // We will add 14 bytes (DA(6)+SA(6)+Length(2)) and since
		  // the Addr4 is used, 802.3 FrameBody starts at an offset of +24 in the 802.11
		  // Frame. We must start creating the 802.3 header at an offset of +10 (10+14 = 24) 
		  //
		  StartOfEthPacket = 10;

		  PacketLength = msduSize - 10;

		  RxPacketLoc += 10;
		  //
		  //DA
		  //
		  VNetMoveMemory (&Adapter->RxBuf[StartOfEthPacket],
				  &Adapter->RxBuf[4], 6);
		  //
		  //SA
		  //
		  VNetMoveMemory (&Adapter->RxBuf[StartOfEthPacket + 6],
				  SourceAddr, 6);
		  //
		  // Length
		  //
		  Adapter->RxBuf[StartOfEthPacket + 12] =
		    (UCHAR) ((msduSize - 24) >> 8);
		  Adapter->RxBuf[StartOfEthPacket + 13] =
		    (UCHAR) (msduSize - 24);
		}
	      else		//IP packet
		{
		  //
		  // We will add 12 bytes (DA(6)+SA(6)) and since the Addr4 is NOT used,
		  // 802.3 FrameBody starts at an offset of +30 in the 802.11 Frame. 
		  // We must start creating the 802.3 header at an offset of +18 (18 + 12 = 30)
		  //
		  StartOfEthPacket = 18;

		  PacketLength = msduSize - 18;

		  RxPacketLoc += 18;

		  //
		  //DA
		  //
		  VNetMoveMemory (&Adapter->RxBuf[StartOfEthPacket],
				  &Adapter->RxBuf[4], 6);
		  //
		  //SA
		  //
		  VNetMoveMemory (&Adapter->RxBuf[StartOfEthPacket + 6],
				  SourceAddr, 6);
		}

	      Adapter->RxPacketLen = PacketLength;

	      // Lookahead amount to indicate                 
/*
				IndicateLen = (PacketLength > Adapter->CurrentLookAhead) ?
							   Adapter->CurrentLookAhead :
							   PacketLength;
*/
	      IndicateLen = PacketLength;

	      RxIndicatePacket (Adapter, (Adapter->RxBuf + StartOfEthPacket),
				PacketLength, IndicateLen);
	      Adapter->Stats.RxDataPacketsOk++;

	      Adapter->IndicateReceiveDone = TRUE;
	    }
	}
      else			// Reception with status != SUCCESS
	{
	  if (FrameType == C80211_TYPE_DATA)
	    Adapter->Stats.RxDataPacketsError++;
	  else if (FrameType == C80211_TYPE_MGMT)
	    Adapter->Stats.RxMgmtPacketsError++;
	}

      //
      // Release Descriptor
      //

      SetRxDescFlags (Adapter, Adapter->RxDescHead, RX_DESC_FLAG_CONSUMED);

      if (Adapter->RxDescHead < (Adapter->HostInfo.RxDescCount - 1))
	Adapter->RxDescHead++;
      else
	Adapter->RxDescHead = 0;

      GetRxDescFlags (Adapter, Adapter->RxDescHead, &flags);
    }


  //
  // Indicate ReceiveComplete
  //
  if (Adapter->IndicateReceiveDone)
    Adapter->IndicateReceiveDone = FALSE;
}


#ifdef RX_CRC
// CRC
VOID
init_CRCtable (PVNet_ADAPTER Adapter)
{
  ULONG crc;
  int i, j;

  for (i = 0; i < 256; i++)
    {
      crc = i;
      for (j = 8; j > 0; j--)
	{
	  if (crc & 1)
	    crc = (crc >> 1) ^ 0xedb88320;
	  else
	    crc >>= 1;
	}
      Adapter->CrcTable[i] = crc;
    }
}

ULONG
Calculate_CRC32 (PUCHAR p, ULONG FrameLength, PULONG crctab)
{
  ULONG crcv, t;
  ULONG nr = 0;

  crcv = 0xffffffff;
  while (nr < FrameLength)
    {
      t = (crcv ^ p[nr]) & 0xff;
      crcv = ((crcv >> 8) & 0x00ffffff) ^ crctab[t];
      nr++;
    }
  crcv = (crcv ^ 0xffffffff);

  return (ULONG) crcv;
}
#endif
