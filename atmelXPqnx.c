#include <vnet.h>

//We define this in vnet_qnx.h based on which directory we are compiling in.
#if defined(REV_D)
#include "fwr_d.h"
#elif defined(REV_E)
#include "fwr_e.h"
#elif defined(R504)
#include "fwr504.h"
#elif defined(R504_2958)
#include "fwr504_2958.h"
#elif defined(R504A_2958)
#include "fwr504a_2958.h"
#elif defined(PCI)
#include "fwr506.h"
#elif defined(RFMD)
#include "fwr.h"
#elif defined(RFMD_3COM)
#include "fwr_3COM.h"
#endif //REV_D

VOID CardReadUshort(PVNet_ADAPTER Adapter, UCHAR Offset, PUSHORT pData)
{
	*pData = in16(Adapter->IoBase + (Offset));
}


VOID CardWriteUshort(PVNet_ADAPTER Adapter, UCHAR Offset, USHORT Data)
{
	out16((Adapter->IoBase + Offset),Data);
	if (Offset == AR) {
		USHORT i;
		USHORT temp;
		for(i=0;i<10;i++) {
			CardReadUshort(Adapter, Offset,&temp);
			if (temp == (USHORT) Data )
				break;
			out16((Adapter->IoBase + Offset),Data);
		}
		if(temp != (USHORT)Data) IF_LOUD(DbgPrint("FAILED TO SET AR REG\n"));
	}
}


VOID CardWriteUchar(PVNet_ADAPTER Adapter, UCHAR Offset, UCHAR Data)
{
	out8((Adapter->IoBase + Offset),Data);
}

VOID CardReadUchar(PVNet_ADAPTER Adapter, UCHAR Offset, PUCHAR pData)
{
	*(pData) = in8(Adapter->IoBase + Offset);
}


void SetMgmtTimer(PVNet_ADAPTER Adapter)
{
	struct itimerspec tvalue;

	if(timer_getexpstatus(Adapter->MgmtTimer)!= 0) {
		/* Initialze timer */
		tvalue.it_value.tv_sec = 2; 
		tvalue.it_interval.tv_sec = tvalue.it_interval.tv_nsec =0;
		tvalue.it_value.tv_nsec = 56 * 1000000;

		timer_settime(Adapter->MgmtTimer,0,&tvalue,0); //2.56 seconds.
	}
}
VOID VnetSleep(ULONG stime)
{
usleep(stime);
}

BOOLEAN GetDefaultFw(PVNet_ADAPTER Adapter)
{
	Nic_t *nic = (Nic_t *)Adapter->extra;
	atmel_handle_t *handle = (atmel_handle_t *)nic->ext;
	ULONG	SoFw = sizeof(BasicFW);
	
	IF_LOUD(DbgPrint("--=+>GetFwFile BasicFW : %d, \n",SoFw);)

	Adapter->MappedFirmwareBuffer = (PUCHAR)handle->ion->alloc(SoFw,NULL); //XXX: FREE THIS SOMEWHERE

	IF_LOUD(DbgPrint("Fw Mapped in : 0x%08X\n",Adapter->MappedFirmwareBuffer);)
	VNetZeroMemory(Adapter->MappedFirmwareBuffer, SoFw);
	VNetMoveMemory(Adapter->MappedFirmwareBuffer, BasicFW, SoFw);
	Adapter->FirmwareFileLength = SoFw;
	IF_LOUD(DbgPrint("<=- Out of GetFwFile\n");)
	return TRUE;
}

VOID
RxIndicatePacket(PVNet_ADAPTER Adapter, PUCHAR pRxBuf, ULONG PacketLength, ULONG IndicateLen)
{
	npkt_t									*npkt;
	net_buf_t								*nb;
	Nic_t 									*nic =	(Nic_t *)Adapter->extra;
	atmel_handle_t					*handle = (atmel_handle_t *)nic->ext;

	IF_LOUD(DbgPrint(">> RxIndicatePacket PacketLen = %d, IndicateLen = %d",PacketLength,IndicateLen);)

	pthread_mutex_lock(&handle->mutex);

	npkt = atmel_fetchnpkt(nic,FALSE);
	nb = (net_buf_t *)TAILQ_FIRST(&npkt->buffers);

	//is this a ethernet packet or 80211B ???
	VNetMoveMemory(nb->net_iov->iov_base,pRxBuf,PacketLength);
	//Reset the packet...
	npkt->next = NULL;
	npkt->num_complete = 1;
	npkt->req_complete = 0;
	npkt->ref_cnt = 1;
	npkt->flags = _NPKT_UP | _NPKT_NOT_TXED;
	npkt->tot_iov = 1;
	nb->net_iov->iov_len = PacketLength;
	npkt->framelen = PacketLength;
	npkt->cell = handle->cell;
	npkt->iface = 0;
	npkt->endpoint = handle->endpoint;

	handle->rxtxstats.rx_bytes += PacketLength;
		

	if( (npkt = handle->ion->tx_up_start(handle->reg_hdlp,npkt,0,0,handle->cell,handle->endpoint,0,&handle))) {
		IF_LOUD(DbgPrint("packet NOT acceped by IO-NET!!");)
	}
	pthread_mutex_unlock(&handle->mutex);
IF_LOUD(DbgPrint("<< RxIndicatePacket\n");)
}

VOID IndicateConnectionStatus(PVNet_ADAPTER Adapter, BOOLEAN Connected)
{
	if(Connected==TRUE){
	}else{
	}
}

VOID 
TxIndicateCompletion(PVNet_ADAPTER Adapter)
{
}
