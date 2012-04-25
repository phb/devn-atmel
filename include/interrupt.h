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


#ifndef __interrupt_h_OK__
#define __interrupt_h_OK__

//
// Constants for the Interrupt Status
//
// Indicate the cause of an interrupt.
//
#define ISR_EMPTY				(UCHAR)0x00        // no bits set in ISR
#define ISR_TxCOMPLETE			(UCHAR)0x01        // packet transmitted
#define ISR_RxCOMPLETE			(UCHAR)0x02        // packet received
#define ISR_RxFRAMELOST			(UCHAR)0x04        // Rx Frame lost
#define ISR_FATAL_ERROR			(UCHAR)0x08        // Fatal error
#define ISR_COMMAND_COMPLETE	(UCHAR)0x10        // command completed
#define ISR_OUT_OF_RANGE		(UCHAR)0x20        // command completed
#define ISR_IBSS_MERGE			(UCHAR)0x40        // (4.1.2.30): IBSS merge
#define ISR_GENERIC_IRQ			(UCHAR)0x80		   // In IFACE field GenericIRQType you can see the reasons/s for this IRQ
//
// Type of an interrupt.
//

typedef enum {
    TxCOMPLETE			= 0x01,
    RxCOMPLETE			= 0x02,
    RxFRAMELOST			= 0x04,
	FATAL_ERROR			= 0x08,
	COMMAND_COMPLETE	= 0x10,
	OUT_OF_RANGE		= 0x20,
	IBSS_MERGE			= 0x40,
    GENERIC_IRQ			= 0x80,
	UNKNOWN				= 0x100
} INTERRUPT_TYPE;

//
// Types of Generiq IRQ in IFACE
//
#define GENERIC_IRQ_RADIO_ON	0x0001
#define GENERIC_IRQ_RADIO_OFF   0x0002 

#endif
