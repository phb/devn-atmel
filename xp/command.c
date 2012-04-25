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
/**   Functions supporting commands interface							  */
/** 																	  */
/**************************************************************************/
/**************************************************************************/

#include "vnet.h"
#include "rx.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SendCommand														  
//
//	  This function submits a command to MAC and returns the status 	  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
SendCommand(	PVNet_ADAPTER Adapter,
				UCHAR CmdId,
				PUCHAR pParameters)
{
	BOOLEAN 			WaitForCompletion	= TRUE;
	ULONG				retry;
	UCHAR				status;
	USHORT				AddressReg;


	if(Adapter->StationState == STATION_STATE_NO_CARD)
		return CMD_STATUS_HOST_ERROR;

	if(!Adapter->CmdPending)
	{
		Adapter->CmdPending = TRUE;
	}
	else
	{
		IF_LOUD(DbgPrint("*** Command Pending ***\n");)
		return CMD_STATUS_BUSY;
	}

	CardReadUshort(Adapter, AR, &AddressReg);

	switch(CmdId)
	{
		case CMD_Set_MIB_Vars:
			HostMemToCardMem16(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, pParameters, sizeof(GET_SET_MIB_STRUCT));
			break;
		case CMD_Get_MIB_Vars:
			HostMemToCardMem16(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, pParameters, sizeof(GET_SET_MIB_STRUCT));
			break;
		case CMD_Start:
			WaitForCompletion = FALSE;
			HostMemToCardMem16(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, pParameters, sizeof(sSTART));
			break;
		case CMD_Join:
			WaitForCompletion = FALSE;
			HostMemToCardMem16(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, pParameters, sizeof(sJOIN));
			break;
		case CMD_Scan:
			WaitForCompletion = FALSE;
			HostMemToCardMem16(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, pParameters, sizeof(sSCAN));
			break;
#if (defined RFMD) || (defined RFMD_3COM)  
		case CMD_EnableRadio:
			WaitForCompletion = TRUE;
			break;
		case CMD_DisableRadio:
			WaitForCompletion = TRUE;
			break;
#endif //RFMD - RFMD_3COM

		default:
			{
			IF_DEBUG_ERRORS(DbgPrint("Invalid Command...(0x%X)\n", CmdId);)
			CardWriteUshort(Adapter, AR, AddressReg);
			Adapter->CmdPending = FALSE;
			return CMD_STATUS_HOST_ERROR;
			}
	}


	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, &CmdId, 1);

	status = 0;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, &status, 1);


	if(WaitForCompletion)
	{
		retry = 0;
		do
		{
			CardMemToHostMem8(Adapter, &status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
			retry++;
		}while((status == CMD_STATUS_IDLE || status == CMD_STATUS_IN_PROGRESS) && retry < LOOP_RETRY_LIMIT);

		if(retry == LOOP_RETRY_LIMIT)
		{
			IF_LOUD(DbgPrint("*** COMMAND FAILED (Status = 0x%X)***\n", status);)

			CardWriteUshort(Adapter, AR, AddressReg);
			Adapter->CmdPending = FALSE;
			return CMD_STATUS_HOST_ERROR;
		}
		else
		{
			CardWriteUshort(Adapter, AR, AddressReg);
			Adapter->CmdPending = FALSE;

			if(CmdId == CMD_EnableRadio)
				return status;
			else
				return CMD_STATUS_COMPLETE;
		}
	}

	Adapter->CmdPending = FALSE;
	CardWriteUshort(Adapter, AR, AddressReg);
	return status;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  CommandCompleteDPC
//
//	  Called after a Command Complete interrupt. It checks the Command
//	  ID and responds
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
CommandCompleteDPC(PVNet_ADAPTER Adapter)
{
	UCHAR							Status, Command;
	int 							BSSindex;
#if DBG
	UCHAR							index, len;
#endif
	BOOLEAN 						FastScan;

	CardMemToHostMem8(Adapter, &Status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
	CardMemToHostMem8(Adapter, &Command, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, 1);

	if( Status == CMD_STATUS_COMPLETE ||
		Status == CMD_STATUS_UNKNOWN ||
		Status == CMD_STATUS_INVALID_PARAMETER ||
		Status == CMD_STATUS_FUNCTION_NOT_SUPPORTED ||
		Status == CMD_STATUS_TIME_OUT)
	{
		Adapter->CmdPending = FALSE;
	}

	if(Status == 0)
	{
		IF_LOUD(DbgPrint("*** Command Completed with status zero ***\n");)
		return;
	}

	switch(Command){
		case CMD_Start:

			if(Status == CMD_STATUS_COMPLETE) {
				IF_LOUD(DbgPrint("Start Command SUCCEEDED (0x%X)\n", Status);)
				Adapter->StationState = STATION_STATE_READY;
				Adapter->StationWasAssociated = TRUE;
				Adapter->StationIsAssociated = TRUE;

				IndicateConnectionStatus(Adapter, TRUE);

				GetBSSID(Adapter, Adapter->CurrentBSSID);
			}			
			break;
			
		case CMD_Scan:

			if (Status == CMD_STATUS_COMPLETE) {
				if(Adapter->FastScan) {
					IF_LOUD(DbgPrint("F A S T SCAN Completed...\n");)		
				}
				else {
					IF_LOUD(DbgPrint("SCAN Completed...\n");)		
				}
#ifdef INT_ROAM
				if(Adapter->IntRoamingEnabled && !Adapter->MultiDomainMib.DomainInfoValid) {
					if(!GetMultiDomainMIB(Adapter)) {
						IF_DEBUG_ERRORS(DbgPrint("FAILED to get MultiDomainMIB\n");)
					}
				}
#endif //INT_ROAM
			
					IF_LOUD(					

					DbgPrint("%d Entries in BSS List:\n", Adapter->BSSListEntries);
					DbgPrint("\n----------------------------------------------------\n");
					if(Adapter->BSSListEntries)
					{
						for(index=0; index<Adapter->BSSListEntries; index++)
						{
							DbgPrint("#%d: ", index);
							DbgPrint("Ch: %d - ", Adapter->BSSinfo[index].Channel );
							for(len=0; len<Adapter->BSSinfo[index].SSIDsize; len++)
								DbgPrint("%c", Adapter->BSSinfo[index].SSID[len]);
							DbgPrint(" - WEP is %s", (Adapter->BSSinfo[index].UsingWEP) ? "ON" : "OFF");
							DbgPrint(" - Preamble is %s", (Adapter->BSSinfo[index].PreambleType)  ? "SHORT" : "LONG");
							DbgPrint(" - RSSI %d\n", Adapter->BSSinfo[index].RSSI);
							DbgPrint("\n");
						}
					}
					else
					{
						DbgPrint("**** BSS List is empty ****\n");
					}
					DbgPrint("----------------------------------------------------\n");
			
				)

				FastScan = Adapter->FastScan;

				Adapter->FastScan = FALSE;

				if(Adapter->SiteSurveyState == SITE_SURVEY_IN_PROGRESS)
				{
					IF_LOUD(DbgPrint("This was a Site Survey Scan...\n");)
					Adapter->SiteSurveyState = SITE_SURVEY_COMPLETED;
					if(Adapter->StationIsAssociated)
					{
						IF_LOUD(DbgPrint("***Station Was Associated...\n");)
						
						Adapter->StationState = STATION_STATE_JOINNING;

						if(Adapter->OperatingMode == INFRASTRUCTURE_MODE)
							Join(Adapter, BSS_TYPE_INFRASTRUCTURE);
						else
							Join(Adapter, BSS_TYPE_AD_HOC);
					}
					else
					{
						IF_LOUD(DbgPrint("###Station Was NOT Associated...\n");)
              if((Adapter->MgmtErrorCode = 0xa0)|| (Adapter->MgmtErrorCode = 0xa1)){
                Adapter->StationState = STATION_STATE_MGMT_ERROR;
              }else{
						    Adapter->StationState = STATION_STATE_WAIT_ACTION; 
              }
					}
				}
				else // This was not a Site Survey Scan
				{
					BSSindex = RetrieveBSS(Adapter, NULL);

					if(BSSindex != -1)
					{
						JoinSpecificBSS(Adapter, BSSindex, FALSE);	
					}//BSS index != -1
					else
					{
						//(4.1.2.30): We must not initiate an IBSS with NULL SSID
            Adapter->HwRadioIsOn=TRUE;
            //Symplhrwsa kai edw
#ifndef RFMD_3COM
						if((Adapter->OperatingMode == AD_HOC_MODE) && Adapter->SSID_Size && Adapter->RadioIsOn /*&& Adapter->HwRadioIsOn*/) 
#else
						if((Adapter->OperatingMode == AD_HOC_MODE) && Adapter->SSID_Size /*&& Adapter->RadioIsOn && Adapter->HwRadioIsOn*/) 
#endif
						{
							IF_LOUD(DbgPrint("No BSS found. START a new IBSS\n");)
							Start(Adapter, BSS_TYPE_AD_HOC);
						}
						else
						{
							IF_LOUD(DbgPrint("No BSS found. Re-Scan\n");)
							
							if(!FastScan)
								Adapter->FastScan = TRUE;
							Scan(Adapter, TRUE);
						}
					}
				}// No BSS selected
			}// Status Command Complete
			else
			{
				if(Status == CMD_STATUS_IN_PROGRESS)
				{
					IF_LOUD(DbgPrint("Scan Command in Progress...\n");)
				}
				else
				{
					Adapter->FastScan = FALSE;

					IF_LOUD(DbgPrint("Scan Command Failed (0x%X)...\n", Status);)
					Scan(Adapter, TRUE);
				}
			}
			
			break;

		case CMD_SiteSurvey:
			
			if (Status == CMD_STATUS_COMPLETE)
			{

				if(Adapter->FastScan)
				{
					IF_LOUD(DbgPrint("F A S T Site Survey SCAN Completed...\n");)		
				}
				else
				{
					IF_LOUD(DbgPrint("N O R M A L Site Survey Completed...\n");)		
				}
				Adapter->FastScan = FALSE;

				IF_LOUD(DbgPrint("SiteSurvey Completed...\n");)
				IF_LOUD(					

					DbgPrint("%d Entries in BSS List:\n", Adapter->BSSListEntries);
					DbgPrint("\n----------------------------------------------------\n");
					if(Adapter->BSSListEntries)
					{
						for(index=0; index<Adapter->BSSListEntries; index++)
						{
							DbgPrint("#%d: ", index);
							DbgPrint("Ch: %d - ", Adapter->BSSinfo[index].Channel );
							for(len=0; len<Adapter->BSSinfo[index].SSIDsize; len++)
								DbgPrint("%c", Adapter->BSSinfo[index].SSID[len]);
							DbgPrint(" - WEP is %s", (Adapter->BSSinfo[index].UsingWEP) ? "ON" : "OFF");
							DbgPrint(" - Preamble is %s", (Adapter->BSSinfo[index].PreambleType)  ? "SHORT" : "LONG");
							DbgPrint(" - RSSI %d\n", Adapter->BSSinfo[index].RSSI);
							DbgPrint("\n");
						}
					}
					else
					{
						DbgPrint("**** BSS List is empty ****\n");
					}
					DbgPrint("----------------------------------------------------\n");
				)
				
				Adapter->SiteSurveyState = SITE_SURVEY_COMPLETED;
				if(Adapter->StationIsAssociated)
				{
					IF_LOUD(DbgPrint("***Station Was Associated...\n");)
					Adapter->StationState = STATION_STATE_READY;				
				}
				else
				{
					IF_LOUD(DbgPrint("###Station Was NOT Associated...\n");)
              if((Adapter->MgmtErrorCode = 0xa0)|| (Adapter->MgmtErrorCode = 0xa1)){
                Adapter->StationState = STATION_STATE_MGMT_ERROR;
              }else{
						    Adapter->StationState = STATION_STATE_WAIT_ACTION; 
              }
				}
									
			}
			else
			{
				if(Status == CMD_STATUS_IN_PROGRESS)
				{
					IF_LOUD(DbgPrint("SiteSurvey Command in Progress...\n");)
				}
				else
				{
					Adapter->FastScan = FALSE;

					IF_LOUD(DbgPrint("SiteSurvey Command Failed (0x%X)...\n", Status);) 	
				}
			}
			break;

		case CMD_Join:
		{
			if (Status == CMD_STATUS_COMPLETE)
			{
				IF_LOUD(DbgPrint("Join Command Completed...\n");)	

				if(Adapter->OperatingMode == AD_HOC_MODE)
				{
					Adapter->StationState = STATION_STATE_READY;
					Adapter->StationWasAssociated = TRUE;
					Adapter->StationIsAssociated = TRUE;

					if(MemCompare(Adapter->CurrentBSSID, Adapter->LastBSSID, 6))
					{
						VNetMoveMemory(Adapter->LastBSSID, Adapter->CurrentBSSID, 6);
						IndicateConnectionStatus(Adapter, TRUE);
					}

				}
				else
				{

					Adapter->AuthenticationRequestRetryCnt = 0;
					Adapter->StationState = STATION_STATE_AUTHENTICATING;

					SetMgmtTimer(Adapter);
					Adapter->CurrentAuthentTransactionSeqNum = 0x0001;
					SendAuthenticationRequest(Adapter, NULL, 0);
				}
			}
			else if(Status == CMD_STATUS_IN_PROGRESS){
					IF_LOUD(DbgPrint("Join Command in Progress...\n");)
			}
			else{
				IF_LOUD(DbgPrint("Join Command Failed (0x%X)...\n", Status);)		


				if(Adapter->StationState == STATION_STATE_FORCED_JOINNING)
					Adapter->StationState = STATION_STATE_FORCED_JOIN_FAILURE; //user must do something...The selected BSSID does not exist 
				else {

					IF_LOUD(DbgPrint("Join failed -> Re-Scan...\n");)
			
					VNetMoveMemory(Adapter->DesiredSSID, Adapter->InitialSSID, Adapter->InitialSSIDsize);
					Adapter->SSID_Size = Adapter->InitialSSIDsize;

					Scan(Adapter, TRUE);
					}
				}
		}// join	
		break;
	}
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//				
//	  Scan																  
//																		  
//	  Issues a Scan Command 											  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


BOOLEAN
Scan(PVNet_ADAPTER Adapter, BOOLEAN UseSpecificSSID)   
{
	sSCAN				sScan;
	UCHAR				Broadcast[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	UCHAR				status;

	Adapter->StationState = STATION_STATE_SCANNING;

	Adapter->ApSelected = FALSE;

	VNetMoveMemory(sScan.BSSID, Broadcast, 6);

		IF_LOUD(DbgPrint("WE ARE INSIDE Scan...\n");)
	if(Adapter->FastScan)
	{
#if ((!defined RFMD) && (!defined RFMD_3COM))  
		if(!SetSSIDsize(Adapter, Adapter->SSID_Size))
			return FALSE;
#else
		sScan.SSID_Size = Adapter->SSID_Size;
#endif

		VNetMoveMemory(sScan.SSID, Adapter->DesiredSSID, Adapter->SSID_Size);
		
	}
	else
	{
		Adapter->BSSListEntries = 0;
#if ((!defined RFMD) && (!defined RFMD_3COM))  
		if(!SetSSIDsize(Adapter, 0))
			return FALSE;
#else
		sScan.SSID_Size = 0;
#endif
	}

#if (defined RFMD) || (defined RFMD_3COM)  
	if(UseSpecificSSID)
	{
		sScan.Options = 0;
	}
	else
	{
		sScan.Options = SCAN_OPTIONS_SITE_SURVEY;
	}
#endif //RFMD

#ifdef INT_ROAM

	if(Adapter->IntRoamingEnabled && !Adapter->MultiDomainMib.DomainInfoValid)
	{
		sScan.Options |= SCAN_OPTIONS_INTERNATIONAL_SCAN;
		IF_LOUD(DbgPrint("I N T E R N A T I O N A L   Scan...\n");)
	}
#endif //INT_ROAM

	sScan.ScanType = SCAN_TYPE_ACTIVE;

	if(Adapter->OperatingMode == AD_HOC_MODE)
		sScan.BSStype = BSS_TYPE_AD_HOC;
	else
		sScan.BSStype = BSS_TYPE_INFRASTRUCTURE;

	//BL: (4.0.2.28) for safety since MSbit is used from driver
	sScan.Channel = (Adapter->Channel & 0x7f); 

	if(Adapter->FastScan)
	{
		sScan.MinChannelTime = 10;
		sScan.MaxChannelTime = 50;
	}
	else
	{
		sScan.MinChannelTime = 10;//50; 28.06.2002:20 Laura
		sScan.MaxChannelTime = 120;//250; 28.06.2002:150 Laura
	}

	IF_LOUD(DbgPrint("Start Scanning from Channel %d - ", Adapter->Channel);)

	IF_LOUD(
		int 				i;
		DbgPrint("SSID(%d): ", Adapter->SSID_Size);
		for(i=0;i<Adapter->SSID_Size;i++)
			DbgPrint("%c", Adapter->DesiredSSID[i]);
		DbgPrint("\n");
	)

	status = SendCommand(Adapter, CMD_Scan, (PUCHAR)&sScan);

	return CMD_STATUS_COMPLETE; 
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  Join																  
//
//	  Issues a Join Command 											  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
Join(PVNet_ADAPTER Adapter, UCHAR Type)
{
	sJOIN				sJoin;
	UCHAR				status;

#if ((!defined RFMD) && (!defined RFMD_3COM))  
	//
	// (4.0.2.29): After a Scan with a NULL SSID SSID size must be restored in F/W 
	// otherwise Join fails
	//
	status = SetSSIDsize(Adapter, Adapter->SSID_Size);

	if(status != CMD_STATUS_COMPLETE)
		return status;
#else
	sJoin.SSID_Size = Adapter->SSID_Size;
#endif

	//BL: (4.0.2.28) for safety since MSbit is used from driver
	sJoin.Channel = (Adapter->Channel & 0x7f);
	VNetMoveMemory(sJoin.BSSID, Adapter->CurrentBSSID , 6);
	VNetMoveMemory(sJoin.SSID, (PUCHAR)Adapter->DesiredSSID,  Adapter->SSID_Size);
	sJoin.BSSType = Type;
	sJoin.JoinFailureTimeout = 2000;

	IF_LOUD(
		UCHAR				i;

		DbgPrint("BSSID to Join = %02X-%02X-%02X-%02X-%02X-%02X in Channel %d\n", Adapter->CurrentBSSID[0],Adapter->CurrentBSSID[1],Adapter->CurrentBSSID[2], Adapter->CurrentBSSID[3],Adapter->CurrentBSSID[4],Adapter->CurrentBSSID[5], Adapter->Channel);
		DbgPrint("SSID	to Join: ");
		for(i=0;i<Adapter->SSID_Size;i++)
			DbgPrint("%c", Adapter->DesiredSSID[i]);
		DbgPrint("\n");
	)

	status = SendCommand(Adapter, CMD_Join, (PUCHAR)&sJoin);
		
	return CMD_STATUS_COMPLETE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  Start 															  
//																		  
//	  Issues a Start Command											
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
Start(PVNet_ADAPTER Adapter, UCHAR Type)
{
	sSTART				sStart;
	UCHAR				status;

#if ((!defined RFMD) && (!defined RFMD_3COM))  
	if(!SetSSIDsize(Adapter, Adapter->SSID_Size))
	{
		IF_LOUD(DbgPrint("Failed to set SSID size before START command...\n");)
		return CMD_STATUS_HOST_ERROR;
	}
#else
	sStart.SSID_Size = Adapter->SSID_Size;
#endif

	VNetMoveMemory(sStart.BSSID, Adapter->BSSID, 6);
	VNetMoveMemory(sStart.SSID, Adapter->DesiredSSID, 32);
	sStart.BSSType = Type;
	


	//BL: (4.0.2.28) for safety since MSbit is used from driver
	sStart.Channel = (Adapter->Channel & 0x7f);

	//issue a Start command
	status = SendCommand(Adapter, CMD_Start, (PUCHAR)&sStart);

	return CMD_STATUS_COMPLETE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetChannel														  
//
//	  This function sets the Channel to the coresponding MIB			 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetChannel(PVNet_ADAPTER Adapter, UCHAR Channel)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Phy_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = PHY_MIB_CHANNEL_POS;
	sSetMIB.Data[0] = Channel;

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);
	
	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Channel(0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetDataFramesTxRateMode												  
//
//	  This function enables or disables the Auto Tx Rate feature												  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetDataFramesTxRateMode(PVNet_ADAPTER Adapter, UCHAR RateIndex)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_AUTO_TX_RATE_POS;

	if(RateIndex == 4)
		sSetMIB.Data[0] = 1;
	else
		sSetMIB.Data[0] = 0;

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Tx Rate (0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetRtsThreshold													   
//																		  
//	  This function sets the RTS/CTS Threshold to the coresponding MIB	  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetRtsThreshold(PVNet_ADAPTER Adapter, USHORT Threshold)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Mac_Mib_Type;
	sSetMIB.Size = 2;
	sSetMIB.Index = MAC_MIB_RTS_THRESHOLD_POS;
	VNetMoveMemory(sSetMIB.Data, &Threshold, sSetMIB.Size);

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set RTS Threshold(0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	SetFragmentationThreshold											  
//
//	This function sets the Fragmentation Threshold to the coresponding MIB 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetFragmentationThreshold(PVNet_ADAPTER Adapter, USHORT Threshold)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Mac_Mib_Type;
	sSetMIB.Size = 2;
	sSetMIB.Index = MAC_MIB_FRAG_THRESHOLD_POS;
	VNetMoveMemory(sSetMIB.Data, &Threshold, sSetMIB.Size);

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Fragmentation Threshold(0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetPreambleType													  
//
//	  This function sets the Preamble Type to the coresponding MIB
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetPreambleType(PVNet_ADAPTER Adapter, UCHAR PreambleType)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_PREAMBLE_TYPE;
	sSetMIB.Data[0] = PreambleType;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Preamble Type (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetCtrlFramesTxRate												  
//
//	  This function sets the Tx Rate of the Control frames to the		 
//	  coresponding MIB													 
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetCtrlFramesTxRate(PVNet_ADAPTER Adapter)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;


	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_TX_CONTROL_RATE_POS;
	sSetMIB.Data[0] = CTRL_FRAMES_TX_RATE;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Ctrl Frames Tx Rate (0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetMgmtFramesTxRate												  
//
//	  This function sets the Tx Rate of the Mangement frames to the    
//	  coresponding MIB													  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetMgmtFramesTxRate(PVNet_ADAPTER Adapter)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;


	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_TX_MGMT_RATE_POS;
	sSetMIB.Data[0] = MGMT_FRAMES_TX_RATE;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Mgmt Frames Tx Rate (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  GetStatistics 													 
//
//	  This function gets statistics MIB and copies it to the buffer 	 
//	  address provided														
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
GetStatistics(PVNet_ADAPTER Adapter, PUCHAR Statistics)
{
	GET_SET_MIB_STRUCT		sGetMIB;
	UCHAR					CmdId;
	ULONG					retry;
	UCHAR					status;

	sGetMIB.Type = (UCHAR)Statistics_Mib_Type;
	sGetMIB.Size = (UCHAR)sizeof(STATISTICS_MIB);
	sGetMIB.Index = (UCHAR)0;

	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, (PUCHAR)&sGetMIB, sizeof(GET_SET_MIB_STRUCT));

	CmdId = CMD_Get_MIB_Vars;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, &CmdId, 1);

	status = 0;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, &status, 1);



	retry = 0;
	do
	{
		CardMemToHostMem8(Adapter, &status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
		retry++;
	}while((status == CMD_STATUS_IDLE || status == CMD_STATUS_IN_PROGRESS) && retry < LOOP_RETRY_LIMIT);
	

	if(retry == LOOP_RETRY_LIMIT)
	{
		return FALSE;
	}
	
	CardMemToHostMem16(Adapter, Statistics, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET+4, sizeof(STATISTICS_MIB));

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetPowerMgmtMode													 
//
//	  This function sets the Power Save Mode to the coresponding MIB	  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetPowerMgmtMode(PVNet_ADAPTER Adapter, UCHAR PSmode)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;
	

	if(PSmode == 0) 
	{
		Adapter->ListenInterval = 1;
		PSmode = ACTIVE_MODE;
	}
	else if(PSmode == 1) 
	{
		Adapter->ListenInterval = 2;
		PSmode = PS_MODE;
	}
#ifndef RFMD_3COM

         else if(PSmode == 2)
         {
            Adapter->ListenInterval = 2;
            PSmode = SPS_MODE;
         }
#endif

	else
	{
		IF_DEBUG_ERRORS(DbgPrint("Invalid Power Management Mode (0x%X)\n", PSmode);)
		return FALSE;

	}

	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = MAC_MGMT_MIB_PS_MODE_POS;
	sSetMIB.Data[0] = PSmode;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Power Save Mode (0x%X)...\n",status);)
		return FALSE;	
	}

	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 2;
	sSetMIB.Index = MAC_MGMT_MIB_LISTEN_INTERVAL_POS;
	VNetMoveMemory(sSetMIB.Data, &Adapter->ListenInterval, 2);
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Listen Interval (0x%X)...\n",status);)
		return FALSE;	
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetAssociationID													  
//
//	  This function sets the Association ID to the coresponding MIB 	  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetAssociationID(PVNet_ADAPTER Adapter, USHORT AssocID)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 2;
	sSetMIB.Index = MAC_MGMT_MIB_STATION_ID_POS;

	VNetMoveMemory(sSetMIB.Data, &AssocID, 2);
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Association ID (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetSupportedRates 												  
//
//	  This function sets the Supported Rates found in the Association Rsp 
//	  Frame to the coresponding MIB 									  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetSupportedRates(PVNet_ADAPTER Adapter, PUCHAR SupportedRates, UCHAR SupRatesLength)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Phy_Mib_Type;
	sSetMIB.Size = SupRatesLength;
	sSetMIB.Index = PHY_MIB_RATE_SET_POS;

	VNetMoveMemory(sSetMIB.Data, SupportedRates, SupRatesLength);


	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Supported Rates (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetPromiscuousMode												  
//
//	  This function sets the Promiscuous Mode							  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetPromiscuousMode(PVNet_ADAPTER Adapter, UCHAR Mode)
{	
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_TX_PROMISCUOUS_POS;
	sSetMIB.Data[0] = Mode;

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Promiscuous Mode to %s (0x%X)...\n", (Mode) ? "ON" : "OFF", status);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetMACaddress 													  
//
//	 Sets the MAC address in the appropriate MIB						  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


BOOLEAN
SetMACaddress(PVNet_ADAPTER Adapter, PUCHAR MACaddress)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Mac_Address_Mib_Type;
	sSetMIB.Size = 6;
	sSetMIB.Index = MAC_ADDR_MIB_MAC_ADDR_POS;
	VNetMoveMemory(sSetMIB.Data, MACaddress, sSetMIB.Size);
	
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set MAC address (0x%X)...\n",status);)
		return FALSE;
	}
	
	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  GetBSSID															  
//
//	 Gets the BSSID that the F/W creates randomly after starting an IBSS  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
GetBSSID(PVNet_ADAPTER Adapter, PUCHAR bssid)
{
	GET_SET_MIB_STRUCT		sGetMIB;
	UCHAR					CmdId;
	ULONG					retry;
	UCHAR					status;

	sGetMIB.Type = (UCHAR)Mac_Mgmt_Mib_Type;
	sGetMIB.Size = (UCHAR)sizeof(MACADDR);
	sGetMIB.Index = (UCHAR)MAC_MGMT_MIB_CUR_BSSID_POS;

	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, (PUCHAR)&sGetMIB, sizeof(GET_SET_MIB_STRUCT));

	CmdId = CMD_Get_MIB_Vars;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, &CmdId, 1);

	status = 0;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, &status, 1);



	retry = 0;
	do
	{
		CardMemToHostMem8(Adapter, &status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
		retry++;
	}while((status == CMD_STATUS_IDLE || status == CMD_STATUS_IN_PROGRESS) && retry < LOOP_RETRY_LIMIT);
	

	if(retry == LOOP_RETRY_LIMIT)
	{
		return FALSE;
	}
	
	CardMemToHostMem16(Adapter, bssid, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET+4, 6);

	IF_LOUD(
		UCHAR	i;
		DbgPrint("BSSID = ");
		for(i=0;i<6;i++)
			DbgPrint("%02X ", bssid[i]);
		DbgPrint("\n");
	)
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetBeaconPeriod
//
//	  This function sets the Beacon Period to the coresponding MIB
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetBeaconPeriod(PVNet_ADAPTER Adapter, USHORT BeaconPeriod)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 2;
	sSetMIB.Index = MAC_MGMT_MIB_BEACON_PER_POS;

	VNetMoveMemory(sSetMIB.Data, &BeaconPeriod, 2);
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set BeaconPeriod (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;
}

#if (defined RFMD) || (defined RFMD_3COM)

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  GetRegulatoryDomain															  
//
//	 Gets and Validates the Regulatory Domain
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
GetRegulatoryDomain(PVNet_ADAPTER Adapter, PUCHAR RegDomain)
{
	GET_SET_MIB_STRUCT		sGetMIB;
	UCHAR					CmdId;
	ULONG					retry;
	UCHAR					status;

	sGetMIB.Type = (UCHAR)Phy_Mib_Type;
	sGetMIB.Size = 1;
	sGetMIB.Index = (UCHAR)PHY_MIB_REG_DOMAIN_POS;

	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, (PUCHAR)&sGetMIB, sizeof(GET_SET_MIB_STRUCT));

	CmdId = CMD_Get_MIB_Vars;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, &CmdId, 1);

	status = 0;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, &status, 1);



	retry = 0;
	do
	{
		CardMemToHostMem8(Adapter, &status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
		retry++;
	}while((status == CMD_STATUS_IDLE || status == CMD_STATUS_IN_PROGRESS) && retry < LOOP_RETRY_LIMIT);
	

	if(retry == LOOP_RETRY_LIMIT)
	{
		return FALSE;
	}
	
	CardMemToHostMem8(Adapter, RegDomain, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET+4, 1);

	IF_LOUD(DbgPrint("Regulatory Domain = 0x%X\n", *RegDomain);)
	
	//
	// Validate Regulatory Domain
	// 
	if( (*RegDomain != REG_DOMAIN_FCC) &&
		(*RegDomain != REG_DOMAIN_DOC) &&
		(*RegDomain != REG_DOMAIN_ETSI) &&
		(*RegDomain != REG_DOMAIN_SPAIN) &&
		(*RegDomain != REG_DOMAIN_FRANCE) &&
		(*RegDomain != REG_DOMAIN_MKK) &&
		(*RegDomain != REG_DOMAIN_MKK1) &&
		(*RegDomain != REG_DOMAIN_ISRAEL))
	{
		IF_LOUD(DbgPrint("I N V A L I D  Regulatory Domain (0x%X)...\n", *RegDomain);)
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetRegulatoryDomain															  
//
//	 Sets the Regulatory Domain
//
//  *** UNDOCUMENTED FUNCTION -- FOR TEST ONLY ***
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
SetRegulatoryDomain(PVNet_ADAPTER Adapter, UCHAR RegDomainCode)
{
	GET_SET_MIB_STRUCT		sSetMIB;
	UCHAR					status;

		
	//
	// Validate Regulatory Domain
	// 
	if( (RegDomainCode != REG_DOMAIN_FCC) &&
		(RegDomainCode != REG_DOMAIN_DOC) &&
		(RegDomainCode != REG_DOMAIN_ETSI) &&
		(RegDomainCode != REG_DOMAIN_SPAIN) &&
		(RegDomainCode != REG_DOMAIN_FRANCE) &&
		(RegDomainCode != REG_DOMAIN_MKK) &&
		(RegDomainCode != REG_DOMAIN_MKK1) &&
		(RegDomainCode != REG_DOMAIN_ISRAEL))
	{
		IF_LOUD(DbgPrint("I N V A L I D  Regulatory Domain (0x%X)...\n", RegDomainCode);)
		return FALSE;
	}


	sSetMIB.Type	= Phy_Mib_Type;
	sSetMIB.Size	= 1;
	sSetMIB.Index	= PHY_MIB_REG_DOMAIN_POS;
	sSetMIB.Data[0]	= RegDomainCode;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Reg Domain (0x%X)...\n",status);)
		return FALSE;
		
	}

	return TRUE;

}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  DisableRadio															  
//
//	 Submits a Disable Radio command to turn Radio OFF
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
DisableRadio(PVNet_ADAPTER Adapter)
{
	return SendCommand(Adapter, CMD_DisableRadio, NULL);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  EnableRadio
//
//	 Submits an Enable Radio command to turn Radio ON
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

UCHAR
EnableRadio(PVNet_ADAPTER Adapter)
{
	return SendCommand(Adapter, CMD_EnableRadio, NULL);
}

#else 

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//	  SetSSIDsize														  
//
//	  This function sets the size of the SSID to the coresponding MIB
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN
SetSSIDsize(PVNet_ADAPTER Adapter, UCHAR size)
{
	GET_SET_MIB_STRUCT	sSetMIB;
	UCHAR			status;

	sSetMIB.Type = Local_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = LOCAL_MIB_SSID_SIZE;
	sSetMIB.Data[0] = size;

	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set SSID size (0x%X)...\n",status);)
		return FALSE;
	}

	return TRUE;

}

#endif //RFMD

#ifdef INT_ROAM
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    SetIntRoaming												     
//
//    This function sets the Multi Domain info to the coresponding MIB	  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BOOLEAN
SetIntRoaming(PVNet_ADAPTER Adapter)
{
	GET_SET_MIB_STRUCT		sSetMIB;
	UCHAR				status;

	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = MAC_MGMT_MIB_MULTI_DOMAIN_IMPLEMENTED;
	sSetMIB.Data[0] = Adapter->IntRoamingEnabled;
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Int Roaming Implemented (0x%X)...\n",status);)
		return FALSE;	
	}


	sSetMIB.Type = Mac_Mgmt_Mib_Type;
	sSetMIB.Size = 1;
	sSetMIB.Index = MAC_MGMT_MIB_MULTI_DOMAIN_ENABLED;
	sSetMIB.Data[0] = Adapter->IntRoamingEnabled;
		
	status = SendCommand(Adapter, CMD_Set_MIB_Vars, (PUCHAR)&sSetMIB);

	if(status != CMD_STATUS_COMPLETE){
		IF_DEBUG_ERRORS(DbgPrint("FAILED to set Int Roaming Enabled (0x%X)...\n",status);)
		return FALSE;	
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    GetMultiDomainMIB
//
//   Gets MDOMAIN_MIB
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

BOOLEAN 
GetMultiDomainMIB(PVNet_ADAPTER Adapter)
{
	GET_SET_MIB_STRUCT		sGetMIB;
	ULONG					retry;
	UCHAR					CmdId,status,index;

	sGetMIB.Type = (UCHAR)Multi_Domain_MIB;
	sGetMIB.Size = sizeof(MDOMAIN_MIB);
	sGetMIB.Index = 0;

	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET, (PUCHAR)&sGetMIB, sizeof(GET_SET_MIB_STRUCT));

	CmdId = CMD_Get_MIB_Vars;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_COMMAND_OFFSET, &CmdId, 1);

	status = 0;
	HostMemToCardMem8(Adapter, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, &status, 1);


        for(index=0; index<14; index++){
            Adapter->ChannelVector[index] = 0;
        }

	retry = 0;
	do
	{
		CardMemToHostMem8(Adapter, &status, Adapter->HostInfo.CommandPos + CMD_BLOCK_STATUS_OFFSET, 1);
		retry++;
	}while((status == CMD_STATUS_IDLE || status == CMD_STATUS_IN_PROGRESS) && retry < LOOP_RETRY_LIMIT);
	

	if(retry == LOOP_RETRY_LIMIT)
	{
		return FALSE;
	}
	
	CardMemToHostMem8(Adapter, (PUCHAR)&Adapter->MultiDomainMib, Adapter->HostInfo.CommandPos + CMD_BLOCK_PARAMETERS_OFFSET+4, sizeof(MDOMAIN_MIB));

	IF_LOUD(DbgPrint("Multi Domain Info is %s\n", (Adapter->MultiDomainMib.DomainInfoValid)?"VALID":"INVALID" );)
        for(index=0; index<14; index++){
            if(Adapter->MultiDomainMib.ChannelList[index]){
            Adapter->ChannelVector[index] = 1;
            } else {
            Adapter->ChannelVector[index] = 0;
            }
        }
        

	IF_LOUD(
		if(Adapter->MultiDomainMib.DomainInfoValid)
		{
			for(index=0; index<14; index++)
			{
				//if(Adapter->MultiDomainMib.ChannelList[index])
					DbgPrint("Ch: %d - Power Level: %d ## ChannelVector : %d\n", index+1, Adapter->MultiDomainMib.TxPowerLevel[index], Adapter->ChannelVector[index]); 

			}
		}
		else
			DbgPrint("No International Info available\n");
	)
		
	return TRUE;
}


#endif //INT_ROAM
