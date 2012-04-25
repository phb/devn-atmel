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

#ifndef __at76c502_h_OK__
#define __at76c502_h_OK__

//
// FastVNet registers currently used
//
#define GCR					0x00	//	(SIR0)	General Configuration Register		(GCR)
#define BSR					0x02	//	(SIR1)	Bank Switching Select Register		(BSR)
#define	AR					0x04
#define	DR					0x08
#define MR1					0x12	//	Mirror Register 1					
#define MR2					0x14	//	Mirror Register 2					

#define MR3					0x16	//	Mirror Register 3					
#define MR4					0x18	//	Mirror Register 4					

#define GPR1				0x0c
#define GPR2				0x0e
#define GPR3				0x10
//
// Constants for the GCR register.
//
#define GCR_REMAP				0x0400		// Remap internal SRAM to 0
#define GCR_SWRES				0x0080		// BIU reset (ARM and PAI are NOT reset) 
#define GCR_CORES				0x0060		// Core Reset (ARM and PAI are reset)
#define GCR_ENINT				0x0002		// Enable Interrupts	
#define GCR_ACKINT				0x0008		// Acknowledge Interrupts

#ifdef R504A_2958
#define BSS_SRAM				0x2000		// AMBA module selection --> SRAM
#else
#define BSS_SRAM				0x0200		// AMBA module selection --> SRAM
#endif
#define BSS_IRAM				0x0100		// AMBA module selection --> IRAM
//
// Constants for the MR registers.
//
#define MAC_INIT_COMPLETE	0x0001        // MAC init has been completed
#define MAC_BOOT_COMPLETE	0x0010        // MAC boot has been completed
#define MAC_INIT_OK			0x0002        // MAC boot has been completed

#endif
