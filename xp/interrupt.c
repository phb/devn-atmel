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
/**   Interrupt Related Functions										  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "vnet.h"
#include "interrupt.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    DisableInterrupts                                                                                               
//
//    Disables interrupts on the card
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
DisableInterrupts (PVNet_ADAPTER Adapter)
{
  USHORT gcr;

  CardReadUshort (Adapter, GCR, &gcr);
  gcr &= ~GCR_ENINT;
  CardWriteUshort (Adapter, GCR, gcr);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    EnableInterrupts                                                                                                
//
//    Enables interrupts on the card
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
EnableInterrupts (PVNet_ADAPTER Adapter)
{
  USHORT gcr;
  CardReadUshort (Adapter, GCR, &gcr);
  gcr |= GCR_ENINT;
  CardWriteUshort (Adapter, GCR, gcr);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    AcknowledgeInterrupt                                                                                                    
//
//    Acknowledges an interrupt
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
AcknowledgeInterrupt (PVNet_ADAPTER Adapter)
{
  USHORT gcr;

  CardReadUshort (Adapter, GCR, &gcr);
  gcr |= GCR_ACKINT;
  CardWriteUshort (Adapter, GCR, gcr);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    GetInterruptType                                                                                                
//
//    Returns the first INTERRUPT_TYPE set in an InterruptStatus
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

INTERRUPT_TYPE
GetInterruptType (UCHAR IntStatus)
{

  return (IntStatus & ISR_OUT_OF_RANGE) ? OUT_OF_RANGE : (IntStatus & ISR_RxCOMPLETE) ? RxCOMPLETE : (IntStatus & ISR_TxCOMPLETE) ? TxCOMPLETE : (IntStatus & ISR_RxFRAMELOST) ? RxFRAMELOST : (IntStatus & ISR_FATAL_ERROR) ? FATAL_ERROR : (IntStatus & ISR_COMMAND_COMPLETE) ? COMMAND_COMPLETE : (IntStatus & ISR_IBSS_MERGE) ?	// (4.1.2.30): IBSS merge
    IBSS_MERGE : (IntStatus & GENERIC_IRQ) ? GENERIC_IRQ : UNKNOWN;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
//    ProcessInterrupt                                                                                                
//
//        This is the real interrupt handling....  
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

VOID
ProcessInterrupt (PVNet_ADAPTER Adapter)
{
  UCHAR InterruptStatus;
  INTERRUPT_TYPE InterruptType;
  USHORT GenericIntType;

  InterruptStatus = GetInterruptStatus (Adapter);
  InterruptType = GetInterruptType (InterruptStatus);

  while (InterruptType != UNKNOWN)
    {
      switch (InterruptType)
	{
	case ISR_OUT_OF_RANGE:
	  Adapter->InterruptStatus &= ~(ISR_OUT_OF_RANGE);
	  ResetInterruptStatusBit (Adapter, ISR_OUT_OF_RANGE);

	  IF_INTERRUPT(IF_LOUD (DbgPrint ("######### OUT OF RANGE ###########\n");))
if (Adapter->OperatingMode == INFRASTRUCTURE_MODE
		 && Adapter->StationState == STATION_STATE_READY)
	    {
	      //
	      // Use the SSID that was initially selected
	      //
	      VNetMoveMemory (Adapter->DesiredSSID, Adapter->InitialSSID,
			      Adapter->InitialSSIDsize);
	      Adapter->SSID_Size = Adapter->InitialSSIDsize;
	      Adapter->InitialSSID[Adapter->InitialSSIDsize] = '\0';
IF_INTERRUPT(
	      IF_LOUD (DbgPrint ("Restore SSID to %s (%d bytes)\n",
			Adapter->InitialSSID, Adapter->InitialSSIDsize);)
		IF_LOUD (DbgPrint ("Re-Scan...\n");)
)
		IndicateConnectionStatus (Adapter, FALSE);
	      Adapter->StationIsAssociated = FALSE;
	      Scan (Adapter, TRUE);
	    }
	  break;

	case COMMAND_COMPLETE:
	  Adapter->InterruptStatus &= ~(ISR_COMMAND_COMPLETE);
	  ResetInterruptStatusBit (Adapter, ISR_COMMAND_COMPLETE);
	  CommandCompleteDPC (Adapter);
	  break;

	case TxCOMPLETE:
	  Adapter->InterruptStatus &= ~(ISR_TxCOMPLETE);
	  ResetInterruptStatusBit (Adapter, ISR_TxCOMPLETE);
	  TxCompleteDPC (Adapter);
	  break;

	case RxCOMPLETE:
	  Adapter->InterruptStatus &= ~(ISR_RxCOMPLETE);
	  ResetInterruptStatusBit (Adapter, ISR_RxCOMPLETE);
	  RxCompleteDPC (Adapter);
	  break;

	case IBSS_MERGE:	// (4.1.2.30): IBSS merge
	  Adapter->InterruptStatus &= ~(ISR_IBSS_MERGE);
	  ResetInterruptStatusBit (Adapter, ISR_IBSS_MERGE);

	  IF_INTERRUPT(IF_LOUD (DbgPrint ("\n>>>>>>>>>> IBSS MERGE <<<<<<<<<<\n");))

		GetBSSID (Adapter, Adapter->CurrentBSSID);

	  break;

	case GENERIC_IRQ:
	  Adapter->InterruptStatus &= ~(ISR_GENERIC_IRQ);
	  ResetInterruptStatusBit (Adapter, ISR_GENERIC_IRQ);

	  IF_INTERRUPT(IF_LOUD (DbgPrint ("\n>>>>>>>>>> GENERIC_IRQ <<<<<<<<<<\n");))
		GetGenericIrqType (Adapter, &GenericIntType);

	  if (GenericIntType == GENERIC_IRQ_RADIO_ON)
	    {
	      IF_INTERRUPT(IF_LOUD (DbgPrint ("---->H/W Radio ON indication\n");)
		)if (!Adapter->HwRadioIsOn && Adapter->RadioIsOn)
		{
		  Adapter->HwRadioIsOn = TRUE;
#if !(defined (PCI))
		  ResetAdapter (Adapter, TRUE);
#endif
		}
	      else
		Adapter->HwRadioIsOn = TRUE;
	    }
	  else if (GenericIntType == GENERIC_IRQ_RADIO_OFF)
	    {
	      IF_INTERRUPT(IF_LOUD (DbgPrint ("---->H/W Radio OFF indication\n");))
				Adapter->HwRadioIsOn = FALSE;
	      //      Adapter->RadioIsOn = FALSE;
	      Adapter->StationState = STATION_STATE_RADIO_OFF;
	      IndicateConnectionStatus (Adapter, FALSE);
	    }
	  else
	    {
	      IF_INTERRUPT(IF_LOUD (DbgPrint ("*** Unknown Generic Interrupt ***\n");))
			}

	  break;

	case RxFRAMELOST:
	  Adapter->InterruptStatus &= ~(ISR_RxFRAMELOST);
	  ResetInterruptStatusBit (Adapter, ISR_RxFRAMELOST);
	  Adapter->Stats.RxLost++;
	  RxCompleteDPC (Adapter);
	  break;
	case FATAL_ERROR:
	  Adapter->InterruptStatus &= ~(ISR_FATAL_ERROR);
	  ResetInterruptStatusBit (Adapter, ISR_FATAL_ERROR);
	  IF_INTERRUPT(IF_DEBUG_ERRORS (DbgPrint ("########################### F A T A L   E R R O R  ( %x )\n");))
ResetAdapter (Adapter, TRUE);

	  break;
	default:
		IF_INTERRUPT(IF_DEBUG_ERRORS (DbgPrint ("DEFAULT\n");))
		break;
	}
      // Get any new interrupts AND ACK
      InterruptStatus = GetInterruptStatus (Adapter);
      // Save the interrupt reasons
      Adapter->InterruptStatus |= InterruptStatus;

      //
      // Get next interrupt to process
      //
      InterruptType = GetInterruptType (Adapter->InterruptStatus);
    }
}


//////////////////////////////////////////////////
//
// Functions for handling the interrupt related 
// fields of IFACE
//
//////////////////////////////////////////////////

UCHAR
GetInterruptStatus (PVNet_ADAPTER Adapter)
{
  UCHAR LockoutH, Status, tmpUC;

PollLockoutHost:

  do
    {
      GetLockoutHost (Adapter, &LockoutH);
    }
  while (LockoutH);

  tmpUC = 1;
  SetLockoutMac (Adapter, &tmpUC);

  GetLockoutHost (Adapter, &LockoutH);

  if (LockoutH)
    {
      tmpUC = 0;
      SetLockoutMac (Adapter, &tmpUC);
      goto PollLockoutHost;
    }

  GetIntStatus (Adapter, &Status);

  tmpUC = 0;
  SetLockoutMac (Adapter, &tmpUC);

  if (Status != ISR_EMPTY)
    AcknowledgeInterrupt (Adapter);

  return Status;
}

UCHAR
ResetInterruptStatusBit (PVNet_ADAPTER Adapter, UCHAR mask)
{
  UCHAR LockoutH, Status, tmpUC;

PollLockoutHost:

  do
    {
      GetLockoutHost (Adapter, &LockoutH);
    }
  while (LockoutH);


  tmpUC = 1;
  SetLockoutMac (Adapter, &tmpUC);

  GetLockoutHost (Adapter, &LockoutH);

  if (LockoutH)
    {
      tmpUC = 0;
      SetLockoutMac (Adapter, &tmpUC);
      goto PollLockoutHost;
    }

  GetIntStatus (Adapter, &Status);
  Status ^= mask;

  SetIntStatus (Adapter, &Status);

  tmpUC = 0;
  SetLockoutMac (Adapter, &tmpUC);

  return Status;
}

VOID
SetInterruptMask (PVNet_ADAPTER Adapter, UCHAR Mask)
{
  UCHAR LockoutH;		//, Status;      
  UCHAR tmpUC;

PollLockoutHost:

  do
    {
      GetLockoutHost (Adapter, &LockoutH);
    }
  while (LockoutH);


  tmpUC = 1;
  SetLockoutMac (Adapter, &tmpUC);
  GetLockoutHost (Adapter, &LockoutH);

  if (LockoutH)
    {
      tmpUC = 0;
      SetLockoutMac (Adapter, &tmpUC);
      goto PollLockoutHost;
    }

  SetIntMask (Adapter, &Mask);

  tmpUC = 0;
  SetLockoutMac (Adapter, &tmpUC);
}
