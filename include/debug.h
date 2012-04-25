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


#ifndef __debug_h_OK__
#define __debug_h_OK__

#ifdef __EVENT_INTERRUPTS 
#define IF_INTERRUPT(A) { A; }
#else
#define IF_INTERRUPT(A) { }
#endif

#if DBG

#define VNet_DEBUG_LOUD               0x00000001
#define VNet_DEBUG_VERY_LOUD          0x00000002  
#define VNet_DEBUG_OID_LOUD		      0x00000010  
#define VNet_DEBUG_Tx			      0x00000020
#define VNet_DEBUG_Tx_VERBOSE	      0x00000040
#define VNet_DEBUG_Rx			      0x00000080  
#define VNet_DEBUG_Rx_VERBOSE	      0x00000100  
#define VNet_DEBUG_ERRORS		      0x00000200  
#define VNet_DEBUG_XP			      0x00000400
#define VNet_DEBUG_XP_LOUD		      0x00000800
#define VNet_DEBUG_SNIFFER			  0x00001000


//
// Macros for debug levels
//

#define IF_LOUD(A)					IF_VNetDEBUG( VNet_DEBUG_LOUD ) { A; }
#define IF_VERY_LOUD(A)				IF_VNetDEBUG( VNet_DEBUG_VERY_LOUD ) { A; }
#define IF_OID_LOUD(A)				IF_VNetDEBUG( VNet_DEBUG_OID_LOUD ) { A; }
#define IF_DEBUG_TX(A)				IF_VNetDEBUG( VNet_DEBUG_Tx ) { A; }
#define IF_DEBUG_TX_VERBOSE(A)		IF_VNetDEBUG( VNet_DEBUG_Tx_VERBOSE ) { A; }
#define IF_DEBUG_RX(A)				IF_VNetDEBUG( VNet_DEBUG_Rx ) { A; }
#define IF_DEBUG_RX_VERBOSE(A)		IF_VNetDEBUG( VNet_DEBUG_Rx_VERBOSE ) { A; }
#define IF_DEBUG_ERRORS(A)			IF_VNetDEBUG( VNet_DEBUG_ERRORS ) { A; }
#define IF_XP_LOUD(A)				IF_VNetDEBUG( VNet_DEBUG_XP ) { A; }
#define IF_XP_VLOUD(A)				IF_VNetDEBUG( VNet_DEBUG_XP_LOUD ) { A; }
#define IF_SNIFFER_LOUD(A)			IF_VNetDEBUG( VNet_DEBUG_SNIFFER ) { A; }

#else

//
// In free builds, no messages
//
#define IF_LOUD(A)					{}
#define IF_VERY_LOUD(A)				{}
#define IF_OID_LOUD(A)              {}
#define IF_DEBUG_TX(A)              {}
#define IF_DEBUG_TX_VERBOSE(A)      {}
#define IF_DEBUG_RX(A)              {}
#define IF_DEBUG_RX_VERBOSE(A)      {}
#define IF_DEBUG_ERRORS(A)          {}
#define IF_XP_LOUD(A)				{}
#define IF_XP_VLOUD(A)				{}
#define IF_SNIFFER_LOUD(A)			{}
#endif

#endif
