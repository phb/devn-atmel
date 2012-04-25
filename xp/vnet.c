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
/** 																	  */ 
/** FastVNET (PCMCIA) NDIS Miniport Driver								  */ 
/** 																	  */
/**   VNet Functions													  */
/** 																	  */
/**************************************************************************/
/**************************************************************************/

#include "vnet.h"
#include "rx.h"
#include "interrupt.h"

 
UCHAR	SNAP_RFC1024[6] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_ETHERNETII[6] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xF8};
UCHAR	STATIC_ENTRIES_ETHERTYPE_8137[] = {0x81, 0x37};
UCHAR	STATIC_ENTRIES_ETHERTYPE_80F3[] = {0x80, 0xF3};

UCHAR	BasicRates[2][4] = {{0x82,0x84,0x0b,0x16},{0x82,0x84,0x8b,0x96}};

#if defined (RFMD) || defined (RFMD_3COM)
UCHAR	RegDomainTable[8] = {	REG_DOMAIN_FCC,
								REG_DOMAIN_DOC,
								REG_DOMAIN_ETSI,
								REG_DOMAIN_SPAIN,
								REG_DOMAIN_FRANCE,
								REG_DOMAIN_MKK,
								REG_DOMAIN_MKK1,
								REG_DOMAIN_ISRAEL };

#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  ConfigureWEP														  
//
//	  Sets in MIBs all the WEP related parameters						 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


BOOLEAN 
ConfigureWEP(PVNet_ADAPTER Adapter)
{
	GET_SET_MIB_STRUCT		sSetMIB;
	UCHAR				status;
	UCHAR				WepIsOn = (Adapter->WepInfo.EncryptionLevel != 0) ? 1 : 0;
	UCHAR				ExcludeUnEncrypted = (Adapter->WepInfo.WepMode == WEP_MODE_MANDATORY) ? 1 : 0;
	UCHAR				KeyLength;
  	int 				i;
#ifdef ATMEL_WLAN
    int         j;
  UCHAR                         index;
  UCHAR				BytesToSet, offset;
  UCHAR                         InWpaMode = FALSE;
  BOOLEAN                       SetKey ;
  MAC_ENCRYPTION_MIB 		MacEncryptionMib;
#else 
  MAC_WEP_MIB	 		MacWepMib;
#endif	// ATMEL_WLAN

#if defined (RFMD) || defined (RFMD_3COM)
	UCHAR				MaxWepKeySize=13;
#else
	UCHAR				MaxWepKeySize=5;
#endif

	// override Authentication Type to OpenSystem when WEP is disabled

	if(Adapter->WepInfo.EncryptionLevel == WEP_DISABLED) {
		Adapter->WepInfo.AuthenticationType = C80211_MGMT_AAN_OPENSYSTEM;
#ifdef ATMEL_WLAN
    Adapter->PairwiseKeyAvailable = FALSE;
    Adapter->GroupKeyAvailable = FALSE;
#endif
  }

	IF_LOUD(DbgPrint("Configuring WEP (%s-%s-%s)\n", (WepIsOn) ? "Enabled" : "Disabled", 
													(ExcludeUnEncrypted) ? "Mandatory" : "Optional",
													(Adapter->WepInfo.AuthenticationType == C80211_MGMT_AAN_SHAREDKEY) ? "Shared Key" : "Open System");)
	

	IF_LOUD(DbgPrint("WEP Key to use: %d -%d\n", Adapter->WepInfo.WepKeyToUse, Adapter->WepInfo.EncryptionLevel);)

	if(Adapter->WepInfo.EncryptionLevel == WEP_64bit) {
		KeyLength = 5;
      IF_LOUD(DbgPrint("WEP 64-BIT\n");)
#ifdef ATMEL_WLAN
    MacEncryptionMib.PrivacyInvoked = 1;
#endif
	}
	else if(Adapter->WepInfo.EncryptionLevel == WEP_128bit) {
		KeyLength = 13;
      IF_LOUD(DbgPrint("WEP 128-BIT\n");)
#ifdef ATMEL_WLAN
    MacEncryptionMib.PrivacyInvoked = 1;
#endif
        } else {
		KeyLength = 0;
      IF_LOUD(DbgPrint("NO WEP\n");)
#ifdef ATMEL_WLAN
    MacEncryptionMib.PrivacyInvoked = 0;
#endif
        }
	

#ifdef ATMEL_WLAN
	SetKey = (MacEncryptionMib.PrivacyInvoked ? TRUE : FALSE);
  Adapter->PairwiseKeyAvailable = FALSE;
  Adapter->GroupKeyAvailable = FALSE;
#endif

	if(KeyLength) {
#ifndef ATMEL_WLAN
		IF_LOUD(
			DbgPrint("WEP Key #1: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.WepKey1[i]);
			DbgPrint("\n");
			DbgPrint("WEP Key #2: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.WepKey2[i]);
			DbgPrint("\n");

			DbgPrint("WEP Key #3: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.WepKey3[i]);
			DbgPrint("\n");

			DbgPrint("WEP Key #4: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.WepKey4[i]);
			DbgPrint("\n");
		)

#else
		IF_LOUD(
			DbgPrint("WEP Key #1: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.KeyMaterial[0][i]);
			DbgPrint("\n");
			DbgPrint("WEP Key #2: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.KeyMaterial[1][i]);
			DbgPrint("\n");
			DbgPrint("WEP Key #3: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.KeyMaterial[2][i]);
			DbgPrint("\n");

			DbgPrint("WEP Key #4: ");
			for(i=0;i<KeyLength;i++)
				DbgPrint("%02X ", Adapter->WepInfo.KeyMaterial[3][i]);
			DbgPrint("\n");
		)
#endif		// ATMEL_WLAN

	}

#ifdef ATMEL_WLAN
	index = Adapter->WepInfo.WepKeyToUse;
	MacEncryptionMib.ExcludeUnencrypted = ExcludeUnEncrypted;
  Adapter->WepInfo.GroupCipherSuite = 0;
  for (i = 0; i < 4; i++){
      if( Adapter->WepInfo.KeyMaterial[i][39] == 1 ){
        if(i == index) {
          MacEncryptionMib.CipherDefaultKeyID = index;
          Adapter->PairwiseKeyAvailable = TRUE;
        } else {
              MacEncryptionMib.CipherDefaultGroupKeyID = i;
              Adapter->GroupKeyAvailable = TRUE;
              Adapter->WepInfo.GroupCipherSuite = Adapter->WepInfo.PairwiseCipherSuite;
        }
      }
  }
	
	if((Adapter->PairwiseKeyAvailable && !Adapter->GroupKeyAvailable)) {
			MacEncryptionMib.CipherDefaultGroupKeyID = MacEncryptionMib.CipherDefaultKeyID;
	} else if(!Adapter->PairwiseKeyAvailable && Adapter->GroupKeyAvailable) {
			MacEncryptionMib.CipherDefaultKeyID = MacEncryptionMib.CipherDefaultGroupKeyID;
	} else {
		MacEncryptionMib.CipherDefaultKeyID = index;
		MacEncryptionMib.CipherDefaultGroupKeyID = index;
	}	
        
	VNetMoveMemory(MacEncryptionMib.ReceiverAddress, Adapter->CurrentBSSID, 6);
        
        BytesToSet = ENCRYPTION_PARAMETERS_BYTES;
        offset = (MAX_ENCRYPTION_KEYS * MAX_ENCRYPTION_KEY_SIZE);


	sSetMIB.Type = Mac_Wep_Mib_Type;
	sSetMIB.Size = BytesToSet;
	sSetMIB.Index = offset; 
	VNetMoveMemory(sSetMIB.Data, ((PUCHAR)&MacEncryptionMib)+offset, BytesToSet);
        
        status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);
#else

	MacWepMib.ExcludeUnencrypted = ExcludeUnEncrypted;
	
	MacWepMib.WEPDefaultKeyID = Adapter->WepInfo.WepKeyToUse;

#if defined (RFMD) || defined (RFMD_3COM)
	MacWepMib.EncryptionLevel = Adapter->WepInfo.EncryptionLevel;
#endif
	VNetMoveMemory((PUCHAR)MacWepMib.WEPDefaultKeyValue, Adapter->WepInfo.WepKey1, KeyLength);
	VNetMoveMemory((PUCHAR)MacWepMib.WEPDefaultKeyValue + MaxWepKeySize, Adapter->WepInfo.WepKey2, KeyLength);
	VNetMoveMemory((PUCHAR)MacWepMib.WEPDefaultKeyValue + (MaxWepKeySize * 2), Adapter->WepInfo.WepKey3, KeyLength);
	VNetMoveMemory((PUCHAR)MacWepMib.WEPDefaultKeyValue + (MaxWepKeySize * 3), Adapter->WepInfo.WepKey4, KeyLength);

	sSetMIB.Type = Mac_Wep_Mib_Type;
	sSetMIB.Size = sizeof(MAC_WEP_MIB);
	sSetMIB.Index = 0;
	VNetMoveMemory(sSetMIB.Data, &MacWepMib, sizeof(MAC_WEP_MIB));

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);
#endif	// ATMEL_WLAN

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set WEP settings (0x%X)...\n",status);)
		return FALSE;
	}

#ifdef ATMEL_WLAN

        if(SetKey) {
            //KeyRSC. In the case of WEP encryption we can ignore it(set it to zero).

            VNetZeroMemory(Adapter->WepInfo.KeyRSC[0], 32);

            sSetMIB.Type = Mac_Wep_Mib_Type;
            sSetMIB.Size = 6;
            sSetMIB.Index = ENCRYPTION_KEY_RSC_BYTES + (index * 8); // * 8 since the array is 4*8 although we use 6 bytes
            VNetMoveMemory(sSetMIB.Data, (PUCHAR)(&Adapter->WepInfo.KeyRSC[index][0]), 6);
            
            status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);
            if(status != CMD_STATUS_COMPLETE){
                return FALSE;
            }
            
            
            //Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-1] = 0x01;
  for (i = 0; i < 4; i++){
      if( Adapter->WepInfo.KeyMaterial[i][39] == 1 ){
        if(i == index) {
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-1] |= 0x02;
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-1] |= 0x04;
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-2] = Adapter->WepInfo.PairwiseCipherSuite;
        } else {
                    Adapter->WepInfo.KeyMaterial[i][MAX_ENCRYPTION_KEY_SIZE-2] = Adapter->WepInfo.GroupCipherSuite;
        }
        SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[i][0], i, MAX_ENCRYPTION_KEY_SIZE);
      }  
            for(j=32;j<MAX_ENCRYPTION_KEY_SIZE; j++) {
                Adapter->WepInfo.KeyMaterial[i][j] = 0;
            }
  }
    
#if 0
            if(1/*KeyIndex & 0x40000000*/) // Pairwise key
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-1] |= 0x02;
    
            if((1/*KeyIndex & 0x80000000*/)) // key that can be used for Tx
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-1] |= 0x04;

            if((1/*KeyIndex & 0x40000000*/)) // Pairwise key
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-2] = Adapter->WepInfo.PairwiseCipherSuite;
            else
                    Adapter->WepInfo.KeyMaterial[index][MAX_ENCRYPTION_KEY_SIZE-2] = Adapter->WepInfo.GroupCipherSuite;
            
            //SetEncryptionKey(&WepInfo.KeyMaterial[index][0], index, MAX_ENCRYPTION_KEY_SIZE);
            SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[0][0], 0, MAX_ENCRYPTION_KEY_SIZE);
            SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[1][0], 1, MAX_ENCRYPTION_KEY_SIZE);
            SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[2][0], 2, MAX_ENCRYPTION_KEY_SIZE);
            SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[3][0], 3, MAX_ENCRYPTION_KEY_SIZE);
#endif

    }
    else {
        for(i=0; i<MAX_ENCRYPTION_KEY_SIZE; i++) {
            Adapter->WepInfo.KeyMaterial[0][i] = 0;
            Adapter->WepInfo.KeyMaterial[1][i] = 0;
            Adapter->WepInfo.KeyMaterial[2][i] = 0;
            Adapter->WepInfo.KeyMaterial[3][i] = 0;
        }
            
        SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[0][0], 0, MAX_ENCRYPTION_KEY_SIZE);
        SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[1][0], 1, MAX_ENCRYPTION_KEY_SIZE);
        SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[2][0], 2, MAX_ENCRYPTION_KEY_SIZE);
        SetEncryptionKey(Adapter, &Adapter->WepInfo.KeyMaterial[3][0], 3, MAX_ENCRYPTION_KEY_SIZE);
    }


#endif // ATMEL_WLAN

	return TRUE;
}

#ifdef ATMEL_WLAN
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetEncryptionKey 
//	  Sets Encryption Key						 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOLEAN
SetEncryptionKey(PVNet_ADAPTER Adapter, PUCHAR Key, UCHAR index, ULONG length)
{
	GET_SET_MIB_STRUCT		sSetMIB;
	UCHAR				status;

	//////////////////////////////
	//	MAC_ENCRYPTION_MIB 
	//	Keys
	//////////////////////////////

	sSetMIB.Type = Mac_Wep_Mib_Type;
	sSetMIB.Size = (UCHAR)length;
	sSetMIB.Index = (index*MAX_ENCRYPTION_KEY_SIZE);
	VNetMoveMemory(sSetMIB.Data, Key, length);

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);
        if(status != CMD_STATUS_COMPLETE){
            return FALSE;
	}
        	
	return TRUE;
}

#endif // ATMEL_WLAN



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  ResetAdapter														  
//
//	  Causes a H/W reset and re-initializes whatever is necessery for	  
//	  the device to start working normaly after the reset
//	  It will initiate a SCAN after Reset according the InitiateScan parameter						
//																		  
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
ResetAdapter(PVNet_ADAPTER Adapter, BOOLEAN InitiateScan)
{
	USHORT AddressReg;

	if(Adapter->StationState == STATION_STATE_NO_CARD)
		return FALSE;


	CardReadUshort(Adapter, AR, &AddressReg);

	DisableInterrupts(Adapter); 		//will be enabled again in CardStartUp

	Adapter->BSSListEntries = 0;

	Adapter->AuthenticationRequestRetryCnt		= 0;
	Adapter->AssociationRequestRetryCnt 		= 0;
	Adapter->ReAssociationRequestRetryCnt		= 0;

	Adapter->InterruptMask						= ISR_TxCOMPLETE | ISR_RxCOMPLETE | ISR_FATAL_ERROR | ISR_RxFRAMELOST | ISR_COMMAND_COMPLETE | ISR_OUT_OF_RANGE | ISR_IBSS_MERGE | ISR_GENERIC_IRQ;
	Adapter->CurrentAuthentTransactionSeqNum	= 0x0001;
	Adapter->ExpectedAuthentTransactionSeqNum	= 0x0002;

	Adapter->StationState					= STATION_STATE_INITIALIZING;
	
	Adapter->StationIsAssociated				= FALSE;

	Adapter->ApSelected 						= FALSE;

#ifdef INT_ROAM
	VNetZeroMemory((PUCHAR)&Adapter->MultiDomainMib, sizeof(MDOMAIN_MIB));
#endif

#if defined (RFMD) || defined (RFMD_3COM)
	if(Adapter->CardType == CARD_TYPE_EEPROM)
		CopyCodeToInternalMemory(Adapter);
#endif

	while(1)
	{
		if(!CardReset(Adapter)) 		// Resets card and waits for MAC to be initialized
			break;
		
		if(!CardGetHostInfo(Adapter))	// Get HOSTINFO struct from MAC
			break;
		
		if(!CardStartUp(Adapter))
			break;

		if(!SetMibValues(Adapter))
			break;
				
		if(InitiateScan)
		{
			Adapter->StationState = STATION_STATE_SCANNING;
			Scan(Adapter, TRUE);
		}
		CardWriteUshort(Adapter, AR, AddressReg);
		return TRUE;
	}

	CardWriteUshort(Adapter, AR, AddressReg);
	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  InitAdapter														  
//																		  
//	  Initializes members of our Adapter Object 					
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID InitAdapter(PVNet_ADAPTER Adapter)
{
	//int 		i = 0;
	//
	// Initialize parameters.
	//
	Adapter->StationState						= STATION_STATE_INITIALIZING;

	VNetZeroMemory(Adapter->BSSID,6);

	VNetZeroMemory(Adapter->LastBSSID, 6);
	Adapter->CurrentBSSID[0] = 0xFF;		//Initialize it to something invalid....

	VNetZeroMemory((PUCHAR)&Adapter->Stats, sizeof(STATISTICS));
	
	Adapter->MulticastListMax					= DEFAULT_MULTICASTLISTMAX;
	Adapter->CurrentLookAhead					= MAX_WIRELESS_FRAME_SIZE;
	Adapter->InterruptMask						= ISR_TxCOMPLETE | ISR_RxCOMPLETE | ISR_FATAL_ERROR | ISR_RxFRAMELOST | ISR_COMMAND_COMPLETE | ISR_OUT_OF_RANGE | ISR_IBSS_MERGE | ISR_GENERIC_IRQ;

	Adapter->PendingTxPackets					= 0;

	// No resource have been assigned yet

	Adapter->IOregistered						= FALSE;

	Adapter->StationWasAssociated				= FALSE;
	Adapter->StationIsAssociated				= FALSE;

	Adapter->CurrentAuthentTransactionSeqNum	= 0x0001;
	Adapter->ExpectedAuthentTransactionSeqNum	= 0x0002;

	Adapter->ApSelected 						= FALSE;

	Adapter->BSSListEntries 					= 0;

	Adapter->AuthenticationRequestRetryCnt		= 0;
	Adapter->AssociationRequestRetryCnt 		= 0;
	Adapter->ReAssociationRequestRetryCnt		= 0;

	Adapter->MiniportResetRequest				= FALSE;
	Adapter->MiniportResetPending				= FALSE;

	Adapter->SiteSurveyState					= SITE_SURVEY_IDLE;

	Adapter->IntRoamingEnabled					= 0;

	Adapter->UseWzcs							= 0;

	Adapter->InSnifferMode						= FALSE;

	Adapter->PeriodicBeaconsCnt 				= 0;		
//
//(4.1.2.30): Under XP we no longer use a boolean var (Adapter->WepIsConfigured)
//	but a NDIS_802_11_WEP_STATUS var (Adapter->XP_WepStatus) in order to support
// all possible states (KeyAbsent, Enabled, Disabled, NotSupported)
//
// KeyAbsent seems to cause problems with 802.1x so we use only WepDisabled
	Adapter->DeviceIsUp = TRUE;
#ifdef RX_CRC
	init_CRCtable(Adapter);
#endif

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  ChangeConfiguration												  
//
//	  Is called when the application has changed some configuration 	  
//	  parameters & adopts them. Depending to the parameters that have	  
//	  changed ant the current station state & mode it may reset the card, 
//	  or re-scan or just set some MIBs									  
//																		  
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
ChangeConfiguration(PVNet_ADAPTER Adapter, PDEVICE_CONFIGURATION pDevConfig)
{
	UCHAR				i;
	BOOLEAN 			SSIDmatch,AdapterMustBeReset = FALSE;
	UCHAR				bytes;
	//UCHAR				Status;

	//
	// Channel
	//
	if( (Adapter->Channel != pDevConfig->Channel) &&  (Adapter->OperatingMode == AD_HOC_MODE) )
	{

		IF_LOUD(DbgPrint("Changing From Channel %d to %d\n", Adapter->Channel, pDevConfig->Channel);)
		Adapter->Channel = pDevConfig->Channel;
		SetChannel(Adapter, pDevConfig->Channel);
		AdapterMustBeReset = TRUE; //in AdHoc after changing Channel reset
	}
	//
	// Data Frames Tx Rate
	//
	if(Adapter->DataFramesTxRate != pDevConfig->TxRate)
	{
		IF_LOUD(DbgPrint("Change SpeedIndex from %d to %d\n",Adapter->DataFramesTxRate,pDevConfig->TxRate);)
		Adapter->DataFramesTxRate = pDevConfig->TxRate;
		
		SetDataFramesTxRateMode(Adapter, pDevConfig->TxRate);
	}

	//
	//	International Roaming
	//

#ifdef INT_ROAM

	if(Adapter->IntRoamingEnabled != pDevConfig->InternationalRoaming)
	{
		IF_LOUD(DbgPrint("Changing International Roaming : %d to %d\n",Adapter->IntRoamingEnabled, pDevConfig->InternationalRoaming);)
      if(Adapter->OperatingMode == INFRASTRUCTURE_MODE){
		    Adapter->IntRoamingEnabled = pDevConfig->InternationalRoaming;
      }else {
		    Adapter->IntRoamingEnabled = 0;
      }

                SetIntRoaming(Adapter);
                Adapter->StationState = STATION_STATE_SCANNING;
                Scan(Adapter, TRUE);
	}

#endif


	//
	//	RtsThreshold
	//

	if(Adapter->RtsThreshold != pDevConfig->RtsCtsThreshold)
	{
		IF_LOUD(DbgPrint("Changing RTS/CTS Threshold from %d to %d\n",Adapter->RtsThreshold, pDevConfig->RtsCtsThreshold);)
		Adapter->RtsThreshold = pDevConfig->RtsCtsThreshold;

		SetRtsThreshold(Adapter, pDevConfig->RtsCtsThreshold);
	}


	//
	//	Fragmentation Threshold->>
	//
	if(Adapter->FragmentationThreshold != pDevConfig->FragmentationThreshold)
	{
		IF_LOUD(DbgPrint("Changing FragmentationThreshold from %d to %d\n", Adapter->FragmentationThreshold, pDevConfig->FragmentationThreshold);)

		if ((pDevConfig->FragmentationThreshold)%2){
		    pDevConfig->FragmentationThreshold -= 1;
    		
                }
        	
                Adapter->FragmentationThreshold = pDevConfig->FragmentationThreshold ;
    		
    	        SetFragmentationThreshold(Adapter, pDevConfig->FragmentationThreshold );
                
               
	}
	//
	// Preamble
	//
	if(Adapter->PreambleType != pDevConfig->PreambleType || Adapter->AutoPreambleDetection)
	{
		if(pDevConfig->PreambleType == AUTO_PREAMBLE && !Adapter->AutoPreambleDetection)
		{
			IF_LOUD(DbgPrint("AUTO Preamble detection enabled...\n");)
			Adapter->PreambleType = LONG_PREAMBLE;
			Adapter->AutoPreambleDetection = TRUE;

			SetPreambleType(Adapter, Adapter->PreambleType);
		}
		else if(pDevConfig->PreambleType != AUTO_PREAMBLE)
		{
			Adapter->AutoPreambleDetection = FALSE;

			IF_LOUD(DbgPrint("Changing Preamble Type from %s to %s\n", (Adapter->PreambleType == LONG_PREAMBLE) ? "LONG" : "SHORT", (pDevConfig->PreambleType == AUTO_PREAMBLE) ? "AUTO" : (pDevConfig->PreambleType == LONG_PREAMBLE) ? "LONG" :"SHORT");)

			Adapter->PreambleType = pDevConfig->PreambleType;
			
			SetPreambleType(Adapter, Adapter->PreambleType);
		}
	}
	
#if !defined(RFMD_3COM)        
	//
	//	RADIO MODE
	//
	if(Adapter->RadioIsOn != pDevConfig->RadioIsOn)
	{
                Adapter->RadioIsOn = pDevConfig->RadioIsOn;
		if(Adapter->RadioIsOn == TRUE)
		{
			IF_LOUD(DbgPrint("Changing RADIO Mode -> ON ...\n");)
                        EnableRadio(Adapter);
                        ConfigureWEP(Adapter);
                        if(Adapter->OperatingMode == INFRASTRUCTURE_MODE)
                                 Join(Adapter, BSS_TYPE_INFRASTRUCTURE);
                            else
                                     Join(Adapter, BSS_TYPE_AD_HOC);
                        //ResetAdapter(Adapter, TRUE);
		}
		else
		{
			IF_LOUD(DbgPrint("Changing Radio Mode -> OFF ...\n");)
		        Adapter->StationState	= STATION_STATE_RADIO_OFF;
		        Adapter->RejectPendingTxPackets = TRUE;
            DisableRadio(Adapter);
		}

	}
#endif

	//
	// SSID
	//

        if(Adapter->RadioIsOn == TRUE){
            SSIDmatch = TRUE;

            // (4.0.2.28) Whenever NULL SSID is selected force a SCAN

                if((Adapter->InitialSSIDsize != pDevConfig->SSIDlength) || (pDevConfig->SSIDlength == 0))
                {
                        IF_LOUD(DbgPrint("### SSID size changed (or NULL SSID) ### (%d-%d)\n", Adapter->InitialSSIDsize, pDevConfig->SSIDlength);)
                        SSIDmatch = FALSE;
                }
                else
                {
                        bytes = (Adapter->InitialSSIDsize > pDevConfig->SSIDlength) ? Adapter->InitialSSIDsize : pDevConfig->SSIDlength;
                        for(i=0;i<bytes;i++)
                        {
                                if(Adapter->InitialSSID[i] != pDevConfig->SSID[i])
                                {
                                        IF_LOUD(DbgPrint("### SSID Changed ###\n");)
                                        SSIDmatch = FALSE;
                                        break;
                                }
                        }
                }

                if(!SSIDmatch)
                {
                        IF_LOUD(DbgPrint("Changing SSID...\n");)
                        
                        AdapterMustBeReset = TRUE;

                        VNetMoveMemory(Adapter->DesiredSSID, pDevConfig->SSID, pDevConfig->SSIDlength);
                        Adapter->SSID_Size = pDevConfig->SSIDlength;
                        
                        VNetMoveMemory(Adapter->InitialSSID, pDevConfig->SSID, pDevConfig->SSIDlength);
                        Adapter->InitialSSIDsize = pDevConfig->SSIDlength;

                        if( Adapter->SSID_Size == 0 )
                                Adapter->ConnectToAnyBSS = TRUE;
                        else
                                Adapter->ConnectToAnyBSS = FALSE;

    #if ((!defined RFMD) && (!defined RFMD_3COM))
                        SetSSIDsize(Adapter, Adapter->SSID_Size);
    #endif

                }
            //
            //	OPERATING MODE
            //
            if(Adapter->OperatingMode != pDevConfig->OperatingMode)
            {
                    AdapterMustBeReset		= TRUE;
                    Adapter->StationState	= STATION_STATE_INITIALIZING;
                    if(Adapter->OperatingMode == AD_HOC_MODE)
                    {
                            IF_LOUD(DbgPrint("Changing Operating Mode to INFRASTRUCTURE...\n");)
                            Adapter->OperatingMode = pDevConfig->OperatingMode;
                    }
                    else
                    {
                            IF_LOUD(DbgPrint("Changing Operating Mode to AD-HOC in channel %d...\n", pDevConfig->Channel);)
                            Adapter->OperatingMode = pDevConfig->OperatingMode;
                            //restore channel
                            IF_LOUD(DbgPrint("Changing From Channel %d to %d\n", Adapter->Channel, pDevConfig->Channel);)
                            Adapter->Channel = pDevConfig->Channel; 
                            if(Adapter->IntRoamingEnabled){
                              Adapter->IntRoamingEnabled = 0;
                            }
                    }

            }

            //
            // Power Mgmt Mode
            //
            if(Adapter->PowerMgmtMode != pDevConfig->PowerMgmtMode)
            {
                    //	
                    //(4.1.2.30): While in AdHoc we MUST NOT enable PS. The selected Power Mgmt
                    // mode is now tested after setting the Operating Mode in Infrastructure.
                    //
                    if(Adapter->OperatingMode == INFRASTRUCTURE_MODE)
                    {
                            IF_LOUD(DbgPrint("Changing Power Mgmt Mode from %d to %d\n", Adapter->PowerMgmtMode, pDevConfig->PowerMgmtMode);)

                            if(Adapter->PowerMgmtMode) //from PS->normal needs Join
                                    Join(Adapter, BSS_TYPE_INFRASTRUCTURE);
                            else
                                    SetPowerMgmtMode(Adapter, pDevConfig->PowerMgmtMode);

                            Adapter->PowerMgmtMode = pDevConfig->PowerMgmtMode;
                    }
                    else
                    {
                            if(Adapter->PowerMgmtMode)
                            {
                                    IF_LOUD(DbgPrint("Power Save selection is ignored in AdHoc Mode...\n");)
                    
                                    AdapterMustBeReset = TRUE; //from PS->normal needs reset
                                    Adapter->PowerMgmtMode = 0;
                            }
                    }
            }
        }


	//
	//BL: (4.0.2.28) reset the 'black list'...
	//
	for(i=0; i<Adapter->BSSListEntries;i++)
		Adapter->BSSinfo[i].Channel = (Adapter->BSSinfo[i].Channel & 0x7f);
	
	if(AdapterMustBeReset)
	{
		Adapter->StationIsAssociated = FALSE;
		Adapter->RejectPendingTxPackets = TRUE;

		Adapter->StationState = STATION_STATE_SCANNING;
		Scan(Adapter, TRUE);
	
//		ResetAdapter(Adapter, TRUE);
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  InitAndStartCard
//
//	  Initializes the card and starts operation. Is called from the 	  
//	  Initialize handler												  
//																		  
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN InitAndStartCard(PVNet_ADAPTER Adapter)
{
	BOOLEAN 				ParallelFlash;
	USHORT					temp;
	USHORT					gcr;


	CardReadUshort(Adapter, GCR, &gcr);

#ifndef PCI
	CardWriteUshort(Adapter, GCR, (USHORT)(gcr|0x0060));
	CardReadUshort(Adapter, GCR, &gcr);
	CardWriteUshort(Adapter, GCR, (USHORT)(gcr&0xFF9F));
#endif

	VnetSleep(500000);				

	CardReadUshort(Adapter, MR2, &temp);

	if (!temp) {
		ParallelFlash = FALSE;
		Adapter->CardType = CARD_TYPE_EEPROM;

		CopyCodeToInternalMemory(Adapter);
	}
	else {
		CardReadUshort(Adapter, MR4, &temp);
		if (temp) {
			Adapter->CardType = CARD_TYPE_SPI_FLASH;
			IF_LOUD(DbgPrint("SPI FLASH\n");)
			ParallelFlash = FALSE;
		}
		else {
			Adapter->CardType = CARD_TYPE_PARALLEL_FLASH;
			IF_LOUD(DbgPrint("PARALLEL FLASH\n");)
			ParallelFlash = TRUE;
		}
	}

	while(1) {
		if(!CardGetHostInfo(Adapter))	// waits for MAC to be initialized and gets IFACE struct from MAC
			break;

		if(!CardStartUp(Adapter))
			break;

		//
		// MAC Address
		//
		GetMACaddress(Adapter);
		if(!Adapter->NetAddressOverride)
			VNetMoveMemory(Adapter->CurrentAddress, Adapter->PermanentAddress, 6);

#if defined (RFMD) || defined (RFMD_3COM)
		//
		//RegDomain: (4.0.2.28) Get the regulatory domain of this card
		// if the domain is invalid select by default the MKK1
		//
	
                if(!GetRegulatoryDomain(Adapter, &Adapter->RegDomain)) 
                {
                        IF_LOUD(DbgPrint("FAILED to get Regulatory Domain. MKK1 is selected (Default)\n");)
                        Adapter->RegDomain = REG_DOMAIN_MKK1;
                }
#else
		IF_LOUD(DbgPrint("Default Regulatory Domain assigned (MKK1)\n");)
		Adapter->RegDomain = REG_DOMAIN_MKK1;
#endif	//RFMD

		//
		//RegDomain: (4.0.2.28) Check if the currently selected channel is valid for 
		// the selected domain. If not, this function will select the first valid channel
		// for this domain
		//
		ValidateChannelForDomain(Adapter, &Adapter->Channel);

		if(!SetMibValues(Adapter))
		{
			break;
		}

		Adapter->StationState = STATION_STATE_SCANNING;
		Scan(Adapter, TRUE);
		return TRUE;
	}

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  CopyCodeToInternalMemory											  
//
//	  Downloads F/W to the internal memory								  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID CopyCodeToInternalMemory(PVNet_ADAPTER Adapter)
{
	UINT					i;
	USHORT					gcr;

	IF_LOUD(DbgPrint("Copy Code To Internal Memory (** EEPROM **)\n");)
	// Download firmware
	IF_LOUD(DbgPrint("*** Downloading Firmware to internal memory ***\n");)

	Adapter->WorkingWithDefaultFW = TRUE;
	GetDefaultFw(Adapter);

	CardStop(Adapter);

	if(Adapter->FirmwareFileLength <= 24576)
	{
		CardWriteUshort(Adapter, BSR, BSS_IRAM);
		CardWriteUshort(Adapter, AR, (USHORT)0);

		for (i=0;i<Adapter->FirmwareFileLength;i++)
			CardWriteUchar(Adapter, DR, Adapter->MappedFirmwareBuffer[i]);

		// Remap 
		CardReadUshort(Adapter, GCR, &gcr);
		CardWriteUshort(Adapter, GCR, (USHORT)(gcr|GCR_REMAP));
		CardReadUshort(Adapter, GCR, &gcr);
	}
	else //scatter loading
	{
		IF_LOUD(DbgPrint("**** SCATTER LOADING ****\n");)
#ifdef R504A_2958
		CardReadUshort(Adapter, GCR, &gcr);
		CardWriteUshort(Adapter, GCR, (USHORT)(gcr|0xc00));
		CardReadUshort(Adapter, GCR, &gcr);

		CardWriteUshort(Adapter, BSR, 0x2002);
                HostMemToCardMem8(Adapter, 0, Adapter->MappedFirmwareBuffer, 0x8000); 

#else
		// Remap 
		CardReadUshort(Adapter, GCR, &gcr);
		CardWriteUshort(Adapter, GCR, (USHORT)(gcr|GCR_REMAP));
		CardReadUshort(Adapter, GCR, &gcr);

		CardWriteUshort(Adapter, BSR, BSS_IRAM);
		CardWriteUshort(Adapter, AR, (USHORT)0);

		for (i=0;i<0x6000;i++)
			CardWriteUchar(Adapter, DR, Adapter->MappedFirmwareBuffer[i]);
#endif

                ///Michali edw symplhrwsa 111
#ifdef R504A_2958
		CardWriteUshort(Adapter, BSR, 0x2001);
                HostMemToCardMem8(Adapter, 0x8000, (Adapter->MappedFirmwareBuffer+0x8000), Adapter->FirmwareFileLength-0x8000); 
		CardWriteUshort(Adapter, BSR, 0x2000);
		CardReadUshort(Adapter, GCR, &gcr);
		CardWriteUshort(Adapter, GCR, (USHORT)(gcr&0xf7ff));
		CardReadUshort(Adapter, GCR, &gcr);
#else
		CardWriteUshort(Adapter, BSR, 0x2ff);
		CardWriteUshort(Adapter, AR, (USHORT)0x8000);

		for (i=0;i<Adapter->FirmwareFileLength-0x6000;i++)
			CardWriteUchar(Adapter, DR, Adapter->MappedFirmwareBuffer[0x6000+i]);

#endif
	}
	//
	// get ARM from reset state. Code will start running
	//
	CardWriteUshort(Adapter, GCR, (USHORT)(gcr&0xFFBF));
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  GetMACaddress 													  
//
//	 Gets (reads) the MAC address from the FLASH. if the MAC address is   
//	 not present, the default MAC address is assigned and the application 
//	 will prompt the user to upgrade his F/W							  
//																		  
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
GetMACaddress(PVNet_ADAPTER Adapter)
{
	UCHAR					DefaultMAC[] = {0x00,0x04, 0x25, 0x00, 0x00, 0x00};
	GET_SET_MIB_STRUCT		sGetMIB;
	UCHAR					status = 0;

	VnetSleep(500000);

	if (Adapter->CardType == CARD_TYPE_PARALLEL_FLASH) {

		CardWriteUshort(Adapter,  BSR, 1);	
		CardMemToHostMem8(Adapter, Adapter->PermanentAddress, 0xc000, 6);
		CardWriteUshort(Adapter,  BSR, BSS_SRAM);
	}
	else 
	{

		sGetMIB.Type = (UCHAR)Mac_Address_Mib_Type;
		sGetMIB.Size = (UCHAR)6;
		sGetMIB.Index = (UCHAR)0;

		status = SendCommand(Adapter, CMD_Get_MIB_Vars, (PUCHAR)&sGetMIB);

		if(status != CMD_STATUS_COMPLETE)
		{
			IF_DEBUG_ERRORS(DbgPrint("******** FAILED to Get Mac Address (0x%X)...\n",status);)
			VNetMoveMemory(Adapter->PermanentAddress, DefaultMAC, 6);
		}

		CardMemToHostMem16(Adapter, sGetMIB.Data, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET + 4, 6);
		VNetMoveMemory(Adapter->PermanentAddress, sGetMIB.Data, 6);
	}

	if(Adapter->PermanentAddress[0] == 0xFF)
	{
		IF_DEBUG_ERRORS(
		DbgPrint("*** Invalid MAC address. UPGRADE Firmware ****");
		DbgPrint("Assigning default MAC address...\n");)
		Adapter->InvalidMACaddress = TRUE;
		VNetMoveMemory(Adapter->PermanentAddress, DefaultMAC, 6);
	}
	else 
		Adapter->InvalidMACaddress = FALSE;


	IF_LOUD(
		int 					i;

		DbgPrint("MAC Address: ");
		for(i=0;i<6;i++)
			DbgPrint("%02X ", Adapter->PermanentAddress[i]);
		DbgPrint("\n");
	)
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  AsciiToByte														  
//
//	  This function gets two ASCII values and returns a byte with the	  
//	  4 MSbits set to the value that coresponds to the first ASCII and	  
//	  the 4 LSbits set to the value that coresponds to the second ASCII.  
//	  This function is required because the WEP keys are get as a 10	  
//	  characters string and we need to extract the 5 HEX Bytes of the KEY 
//	  There are no range checks 										  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
AsciiToByte(UCHAR Ascii1, UCHAR Ascii2)
{
	UCHAR digit1= 0;
	UCHAR digit2= 0;

	if(Ascii1 >= 0x30 && Ascii1 <= 0x39)
			digit1 = Ascii1 - 0x30;
	else if(Ascii1 >= 0x41 && Ascii1 <= 0x46)
			digit1 = Ascii1 - 0x31 - 6;


	if(Ascii2 >= 0x30 && Ascii2 <= 0x39)
			digit2 = Ascii2 - 0x30;
	else if(Ascii2 >= 0x41 && Ascii2 <= 0x46)
			digit2 = Ascii2 - 0x31 - 6;


	return (digit1<<4) + digit2;
}

VOID
ByteToAscii(UCHAR Byte, PUCHAR Ascii1, PUCHAR Ascii2)
{
	if( (( Byte & 0xf0 ) >> 4) <=9 )
		*Ascii1 = (( Byte & 0xf0 ) >> 4) + 0x30;
	else
		*Ascii1 = (( Byte & 0xf0 ) >> 4) + 0x57;

	if( ( Byte & 0x0f )  <=9 )
		*Ascii2 = ( Byte & 0x0f ) + 0x30;
	else
		*Ascii2 = ( Byte & 0x0f ) + 0x57;	
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  MgmtTimeOutCallBack												  
//																		  
//	  This is the OS indipendent callback routine of a mgmt timer set when
//	  a management frame is being transmited.							  
//																		   
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID MgmtTimeOutCallBack(PVNet_ADAPTER Adapter)
{
	BOOLEAN 	TakeAction = FALSE;
	int 		BSSindex;

//
// (4.0.2.28) if we must go in STATION_STATE_MGMT_ERROR we decide if we will do 
// a SCAN, JOIN or nothing (ie just stay in STATION_STATE_MGMT_ERROR) according to the
// OS (XP or not) and to the SSID (NULL or not)
//
	if(Adapter->StationState == STATION_STATE_AUTHENTICATING )
	{
		IF_LOUD(DbgPrint("#### Authentication Request Timed Out (%d)...\n", Adapter->AuthenticationRequestRetryCnt);)
		
		if(Adapter->AuthenticationRequestRetryCnt >= MAX_AUTHENTICATION_RETRIES){
			Adapter->StationState				   = STATION_STATE_MGMT_ERROR;
			Adapter->StationIsAssociated		   = FALSE;
			Adapter->AuthenticationRequestRetryCnt = 0;

			TakeAction = TRUE;
		}
		else
		{
			IF_LOUD(DbgPrint("Retry to Authenticate...\n");)
			Adapter->AuthenticationRequestRetryCnt++;
			Adapter->CurrentAuthentTransactionSeqNum = 0x0001;
			SetMgmtTimer(Adapter);
			SendAuthenticationRequest(Adapter, NULL, 0);
		}

	}
	else
	if(Adapter->StationState == STATION_STATE_ASSOCIATING)
	{
		IF_LOUD(DbgPrint("#### Association Request Timed Out...\n");)
		
		if(Adapter->AssociationRequestRetryCnt == MAX_ASSOCIATION_RETRIES){
			Adapter->StationState				= STATION_STATE_MGMT_ERROR;
			Adapter->StationIsAssociated		= FALSE;
			Adapter->AssociationRequestRetryCnt = 0;

			TakeAction = TRUE;
		}
		else
		{
			IF_LOUD(DbgPrint("Retry to Associate...\n");)
			Adapter->AssociationRequestRetryCnt++;
			SetMgmtTimer(Adapter);
			SendAssociationRequest(Adapter);
		}

	}
	else
	if(Adapter->StationState == STATION_STATE_REASSOCIATING)
	{
		IF_LOUD(DbgPrint("#### ReAssociation Request Timed Out...\n");)
		
		if(Adapter->ReAssociationRequestRetryCnt == MAX_ASSOCIATION_RETRIES){
			Adapter->StationState = STATION_STATE_MGMT_ERROR;
			Adapter->StationIsAssociated		   = FALSE;
			Adapter->ReAssociationRequestRetryCnt = 0;

			TakeAction = TRUE;
		}
		else
		{
			IF_LOUD(DbgPrint("Retry to ReAssociate...\n");)
			Adapter->ReAssociationRequestRetryCnt++;
			SetMgmtTimer(Adapter);
			SendReAssociationRequest(Adapter);
		}

	}

	if(TakeAction)
	{
		if(Adapter->ConnectToAnyBSS)
		{
			//
			//BL: (4.0.2.28) invalidate list entry...
			//
			IF_LOUD(DbgPrint("**** Try Another BSS-T (channel |= 0x80 for BSS #%d)...\n", Adapter->CurrentBSSindex);)
			Adapter->BSSinfo[(int)(Adapter->CurrentBSSindex)].Channel |= 0x80;

			BSSindex = RetrieveBSS(Adapter, NULL);
			if(BSSindex != -1)
			{
				IF_LOUD(DbgPrint("**** Try BSS #%d in Channel %d...\n", BSSindex, Adapter->BSSinfo[BSSindex].Channel);)
				JoinSpecificBSS(Adapter, BSSindex, FALSE);
			}
			else
			{
				IF_LOUD(DbgPrint("**** No BSS to try...\n");)

				if(Adapter->UseWzcs)
					IndicateConnectionStatus(Adapter, FALSE);
				else
					Scan(Adapter, FALSE);
			}
		}
		else
		{
			IF_LOUD(DbgPrint("#### Re Scan after Mgmt Frame Time Out ####\n");)
			Scan(Adapter, TRUE);
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  JoinSpecificBSS												  
//																		  
//	  Joins a specific BSS from the list by index.
//																		   
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
JoinSpecificBSS(PVNet_ADAPTER Adapter, int BSSindex, BOOLEAN ChangeInitialSSID)
{
	int 	i = 0;

	if(BSSindex<0 || BSSindex > MAX_BSS_ENTRIES)
	{
		IF_LOUD(DbgPrint("*** Invalid BSS index ***\n");)
		return FALSE;
	}

	VNetMoveMemory(Adapter->CurrentBSSID, Adapter->BSSinfo[BSSindex].BSSID, 6);
	Adapter->SSID_Size = Adapter->BSSinfo[BSSindex].SSIDsize;
	VNetMoveMemory(Adapter->DesiredSSID, (PUCHAR)Adapter->BSSinfo[BSSindex].SSID, Adapter->SSID_Size);

#if ((!defined RFMD) && (!defined RFMD_3COM))
	SetSSIDsize(Adapter, Adapter->SSID_Size);
#endif

	if(ChangeInitialSSID)
	{
		VNetZeroMemory(Adapter->InitialSSID, MAX_SSID_LENGTH);
		VNetMoveMemory(Adapter->InitialSSID, (PUCHAR)Adapter->BSSinfo[BSSindex].SSID, Adapter->BSSinfo[BSSindex].SSIDsize);
		Adapter->InitialSSIDsize = Adapter->BSSinfo[BSSindex].SSIDsize;
		
		//BL: (4.0.2.28) for safety since MSbit is used from driver
		Adapter->Channel = (Adapter->BSSinfo[BSSindex].Channel & 0x7f); 	
	}

	//When switching to AdHoc turn OFF International Roaming / Power Save if needed

	if((Adapter->BSSinfo[BSSindex].BSStype == BSS_TYPE_AD_HOC) &&
		(Adapter->OperatingMode != AD_HOC_MODE) && 
		(Adapter->IntRoamingEnabled || Adapter->PowerMgmtMode))
	{
#ifdef INT_ROAM
		if(Adapter->IntRoamingEnabled)
		{
			IF_LOUD(DbgPrint("*** Turn OFF Int Roaming (switching to AdHoc) ***\n");)
			Adapter->IntRoamingEnabled = 0;
			SetIntRoaming(Adapter);
		}
#endif
		if(Adapter->PowerMgmtMode)
		{
			IF_LOUD(DbgPrint("*** Turn OFF Power Save (switching to AdHoc) ***\n");)
			Adapter->PowerMgmtMode = 0;
			SetPowerMgmtMode(Adapter, Adapter->PowerMgmtMode);
		}
	}
					
	if(Adapter->BSSinfo[BSSindex].BSStype == BSS_TYPE_AD_HOC)
		Adapter->OperatingMode = AD_HOC_MODE;
	else
	if(Adapter->BSSinfo[BSSindex].BSStype == BSS_TYPE_INFRASTRUCTURE)
		Adapter->OperatingMode = INFRASTRUCTURE_MODE;

	//BL: (4.0.2.28) for safety since MSbit is used from driver
	Adapter->Channel = (Adapter->BSSinfo[BSSindex].Channel & 0x7f); 	

	if(Adapter->AutoPreambleDetection && Adapter->PreambleType != Adapter->BSSinfo[BSSindex].PreambleType)
	{
		IF_LOUD(DbgPrint("Changing Preamble Type from %s to %s\n", (Adapter->PreambleType == LONG_PREAMBLE) ? "LONG" : "SHORT", (Adapter->BSSinfo[BSSindex].PreambleType == LONG_PREAMBLE) ? "LONG" : "SHORT");)
		Adapter->PreambleType = Adapter->BSSinfo[BSSindex].PreambleType;

		SetPreambleType(Adapter, Adapter->PreambleType);
	}
	
	Adapter->BeaconPeriod = Adapter->BSSinfo[BSSindex].BeaconPeriod;
	

	IF_LOUD(
		DbgPrint("*** BSS Selected ***\n");
		DbgPrint("Channel = %d\n", Adapter->BSSinfo[BSSindex].Channel);
		DbgPrint("BSSID = %02X-%02X-%02X-%02X-%02X-%02X\n", Adapter->CurrentBSSID[0],Adapter->CurrentBSSID[1],Adapter->CurrentBSSID[2], Adapter->CurrentBSSID[3],Adapter->CurrentBSSID[4],Adapter->CurrentBSSID[5]);
		DbgPrint("SSID: ");
		for(i=0;i<Adapter->SSID_Size;i++)
			DbgPrint("%c", Adapter->DesiredSSID[i]);
		DbgPrint("\n");
		DbgPrint("Beacon Period = %d\n", Adapter->BeaconPeriod);
		DbgPrint("Preamble Type is %s(%d)\n", (Adapter->PreambleType == 0)?"LONG":"SHORT", Adapter->PreambleType);)


		if(!Adapter->UseWzcs)
		{
			if(Adapter->WepInfo.EncryptionLevel == WEP_DISABLED && Adapter->BSSinfo[BSSindex].UsingWEP)
			{
				Adapter->StationState = STATION_STATE_MGMT_ERROR;
				Adapter->MgmtErrorCode = MGMT_ERROR_WEP_REQUIRED;
				
				Adapter->StationIsAssociated		   = FALSE;
				
				IF_LOUD(DbgPrint("**** WEP must be ebabled for this BSS ***\n");)
				return FALSE;
			}

			if(Adapter->WepInfo.EncryptionLevel != WEP_DISABLED && !Adapter->BSSinfo[BSSindex].UsingWEP)
			{
				Adapter->StationState = STATION_STATE_MGMT_ERROR;
				Adapter->MgmtErrorCode = MGMT_ERROR_WEP_NOT_REQUIRED;

				Adapter->StationIsAssociated		   = FALSE;

				IF_LOUD(DbgPrint("**** WEP must be disabled for this BSS ***\n");)
				return FALSE;
			}
		}
		Adapter->ApSelected = TRUE;

		Adapter->StationState = STATION_STATE_JOINNING;
		
		if(Adapter->OperatingMode == INFRASTRUCTURE_MODE)
			Join(Adapter, BSS_TYPE_INFRASTRUCTURE);
		else 
			Join(Adapter, BSS_TYPE_AD_HOC);


		return TRUE;
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetMibValues
//
//	  Initializes the minimum MIB fields, necessary for the FW to operate
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetMibValues(PVNet_ADAPTER Adapter)
{
	UCHAR			status;

	if(Adapter->RadioIsOn && !Adapter->InSnifferMode)
	{
		status = EnableRadio(Adapter);
		
		if(status == CMD_STATUS_REJECTED_RADIO_OFF)
		{
			IF_LOUD(DbgPrint("*** Radio ON command rejected (H/W Radio selection is OFF\n");)
#ifndef RFMD_3COM                        
			Adapter->RadioIsOn = FALSE;
#endif                        
			Adapter->HwRadioIsOn = FALSE;
		}
		else
		if(status != CMD_STATUS_COMPLETE)
		{
			IF_DEBUG_ERRORS(DbgPrint("Failed to turn Radio ON (0x%X)\n", status);)
			return FALSE;
		}
		else
		{
			IF_LOUD(DbgPrint("Radio ON command completed with SUCCESS (0x%X)...\n", status);)
		}

	}

	//	Data Frames Tx Rate Mode

	status = SetDataFramesTxRateMode(Adapter, Adapter->DataFramesTxRate);

	if(!status)
		return FALSE;

#if ((!defined RFMD) && (!defined RFMD_3COM))
// SSID
	status = SetSSIDsize(Adapter, Adapter->SSID_Size);
	if(!status)
		return FALSE;
#endif

	// Promiscuous
		
	status = SetPromiscuousMode(Adapter, PROM_MODE_OFF);
	if(!status)
		return FALSE;


	// RtsThreshold

	status = SetRtsThreshold(Adapter, Adapter->RtsThreshold);

	if(!status)
		return FALSE;


	//	Fragmentation Threshold

	status = SetFragmentationThreshold(Adapter, Adapter->FragmentationThreshold);

	if(!status)
		return FALSE;

	
	//Preamble	

	status = SetPreambleType(Adapter, Adapter->PreambleType);

	if(!status)
		return FALSE;


	if(!SetMACaddress(Adapter, Adapter->CurrentAddress))
		return FALSE;
	
	
	// PS MODE (always Active until the station becomes associated )
	
	status = SetPowerMgmtMode(Adapter, 0);

	if(!status)
		return FALSE;
	

	status = SetBeaconPeriod(Adapter, Adapter->DefaultBeaconPeriod);

	if(!status)
		return FALSE;

	status = SetSupportedRates(Adapter, &BasicRates[Adapter->BasicRatesIndex][0], 4);

	if(!status)
		return FALSE;

#ifdef INT_ROAM
	//Int Roaming

	status = SetIntRoaming(Adapter);

	if(!status)
		return FALSE;
#endif

	// WEP
	
	status = ConfigureWEP(Adapter);

	if(!status)
	{
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    ValidateChannelForDomain														  
//	
//    If International Roaming is enabled it returns TRUE if the channel is 
//    valid for the domain otherwise it returns FALSE. 															         
//	  If International Roaming is NOT enabled, according to the current 
//	  Regulatory Domain validates the channel. If the current channel is not 
//    valid for the domain, a valid channel is automatically selected and 
//	  returns always TRUE
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN ValidateChannelForDomain(PVNet_ADAPTER Adapter, PUCHAR pChannel)
{
	if(Adapter->IntRoamingEnabled)
	{
#ifdef INT_ROAM
		if(Adapter->MultiDomainMib.DomainInfoValid && 
			Adapter->MultiDomainMib.ChannelList[(*pChannel)-1])
			return TRUE;
		else if(Adapter->MultiDomainMib.DomainInfoValid)		
		{
			IF_LOUD(DbgPrint("*** Channel %d is invalid (802.11d) ***\n", *pChannel);)
			return FALSE;
		}
		else 
			return TRUE;
#else
		IF_LOUD(DbgPrint("*** This Driver build does not support Int Roaming !!!***\n");)
		return FALSE;
#endif
	}
	else
	{
            IF_LOUD(DbgPrint(" Adapter-REGDOMAIN!!! : 0x%X\n", Adapter->RegDomain);)
		switch(Adapter->RegDomain)
		{
		case REG_DOMAIN_FCC: //Channels 1-11
		case REG_DOMAIN_DOC: 
			if ((*pChannel<1) || (*pChannel>11))
			{
				*pChannel = 1;
			}
			break;

		case REG_DOMAIN_ETSI: //Channels 1-13
			if ((*pChannel<1) || (*pChannel>13))
			{
				*pChannel = 1;
			}
			break;

		case REG_DOMAIN_SPAIN: //Channels 10-11
			if ((*pChannel<10) || (*pChannel>11))
			{
				*pChannel = 10;
			}
			break;

		case REG_DOMAIN_FRANCE: //Channels 10-13
			if ((*pChannel<10) || (*pChannel>13)) 
			{
				*pChannel = 10;
			}
			break;

		case REG_DOMAIN_MKK: //Channel 14
			if (*pChannel!=14)
			{
				*pChannel = 14;
			}
			break;

		case REG_DOMAIN_MKK1: //Channels 1-14
			if ((*pChannel<1) || (*pChannel>14)) 
			{
				*pChannel = 1;
			}
			break;

		case REG_DOMAIN_ISRAEL: //Channels 3-9
			if ((*pChannel<3) || (*pChannel>9)) 
			{
				*pChannel = 3;
			}
			break;

		default:
			Adapter->RegDomain = REG_DOMAIN_MKK1; //Channels 1-14
			if ((*pChannel<1) || (*pChannel>14))
			{
				*pChannel = 1;
			}
		}

		return TRUE;
	}
}


VOID PrepareBSSList2View(PVNet_ADAPTER Adapter)
{
	BOOLEAN							SkipEntry = FALSE;
	int								i,j;

	Adapter->BSSListEntries2App = 0;
	for(i=0; i<Adapter->BSSListEntries; i++)
	{
		SkipEntry = FALSE;
		
		if(Adapter->BSSinfo[i].SSIDsize == 0)
			continue;
		else
		{
			for(j=0; j<Adapter->BSSinfo[i].SSIDsize; j++)
			{
				if(Adapter->BSSinfo[i].SSID[j] < 0x20)
				{
					SkipEntry = TRUE;
					break;
				}
			}

			if(!SkipEntry)
			{
				for(j=0; j<Adapter->BSSinfo[i].SSIDsize; j++)
				{
					if(Adapter->BSSinfo[i].SSID[j] != 0x20)
						break;
				}
				if(j == Adapter->BSSinfo[i].SSIDsize)
					SkipEntry = TRUE;
			}

			if(!SkipEntry)
			{
				VNetMoveMemory(Adapter->BSSinfo2App + Adapter->BSSListEntries2App, &Adapter->BSSinfo[i], sizeof(BSS_INFO));
				Adapter->BSSListEntries2App++;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  MemCompare														  
//																		 
//	  Compares a number of bytes of two buffers. Returns 0 if they are 
//	  identical 1 otherwise
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

int MemCompare(PUCHAR pMem1, PUCHAR pMem2, ULONG bytes)
{

	ULONG i;

	for(i=0; i<bytes; i++)
		if(pMem1[i] != pMem2[i])
			break;

	if(i == bytes)	
		return 0;
	else
		return 1;
}

VOID VNetMoveMemory(PVOID pDest, PVOID pSrc, ULONG bytes)
{
	memcpy(pDest, pSrc, bytes);
}


VOID VNetZeroMemory(PUCHAR buf, ULONG bytes)
{
	memset(buf, 0, bytes);
}
