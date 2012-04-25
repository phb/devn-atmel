/* atmelqnx.c AKA "The file which got corrupted halfway through the project" */

#include <sys/pccard.h>

#include <vnet.h>
#include <tx.h>
#include <rx.h>
#include <interrupt.h>

/* exported symbol for io-net */
io_net_dll_entry_t io_net_dll_entry = 
{
	2, 
	atmel_init, 
	NULL 
};

/* callback functions for io-net to use to interface with driver */
io_net_registrant_funcs_t atmelFuncs = 
{
	8,
	NULL, 
	atmel_rx_down, //rx_down 
	atmel_tx_done, //tx_done
	atmel_shutdown1,
	atmel_shutdown2,
	atmel_dl_advert,
	atmel_devctl, 
	atmel_flush, 
	NULL
};

/* io-net configuration structure, used during io-net registration */ 
io_net_registrant_t atmel_entry = 
{
	_REG_PRODUCER_UP,
	"devn-atmel",
	"en",
	NULL,
	NULL,
	&atmelFuncs,
	0
};


int str2opt(void *hdl,char *dev,char *name,char *val) 
{
	atmel_options_t *options = (atmel_options_t *)hdl;
	int len;
	int i,y;
	int hex=0;
	char *Key=NULL;

	IF_VERY_LOUD(DbgPrint("str2opt: %s:: %s:%s",dev,name,val);)
	if(strcmp(name,"ssid") == 0) {
		strncpy(options->ssid,val,MAX_SSID_LENGTH);
		options->ssid[MAX_SSID_LENGTH] = '\0';
	} else if(strncmp(name,"key",3) == 0) {
			len = strlen(val);
			switch(len) {
				case 13:
				case 26:
				case 28:
					options->EncryptionLevel = WEP_128BIT;
					break;
				case 5:
				case 10:
				case 12:
					options->EncryptionLevel = WEP_64BIT;
					break;
			default:
				DbgPrint("Invalid WEP Key length, it must be either 10/26 hex characters (key1=0x12345) or 5/13 ascii caracters (key1=12345)\n");
				options->EncryptionLevel = WEP_DISABLED;
				return -1;
			}
			if(memcmp(val,"0x",2)==0 || len == 10 || len == 12 || len == 26 || len == 28) {
				hex  =  1;
			}
			if(memcmp(val,"0x",2) == 0 ) {
				len -= 2;
				val += 2;
			}
			if(strcmp(name,"key1") == 0) {
				Key = options->WepKey1;
			}else if(strcmp(name,"key2") == 0) {
				Key = options->WepKey2;
			}else if(strcmp(name,"key3") == 0) {
				Key = options->WepKey3;
			}else if(strcmp(name,"key4") == 0) {
				Key = options->WepKey4;
			}
	
			if(hex && Key) {
				for(i=0,y=0;i<len;i+=2,y++)
				{
					Key[y] = AsciiToByte(val[i],val[i+1]);	
				}
				Key[y] = '\0';
				len = y;
			} else {
				strncpy(Key,val,len);
			}
		}	

return EOK;
}


#define ATMEL_OFFSET(a) offsetof(atmel_options_t,a)

static drvr_subopt_tbl_t atmelOptionsTable[] = {
	{"ssid",0,0,str2opt},
	{"channel",ATMEL_OFFSET(channel),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"opmode",ATMEL_OFFSET(OpMode),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"default_key",ATMEL_OFFSET(WepKeyToUse),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"key1",0,0,str2opt},
	{"key2",0,0,str2opt},
	{"key3",0,0,str2opt},
	{"key4",0,0,str2opt},
	{"wepmode",ATMEL_OFFSET(WepMode),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"authtype",ATMEL_OFFSET(AuthenticationType),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"preambletype",ATMEL_OFFSET(PreambleType),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"pwrmgmtmode",ATMEL_OFFSET(PwrMgmtMode),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"beaconperiod",ATMEL_OFFSET(BeaconPeriod),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"radioon",ATMEL_OFFSET(RadioOn),DRVR_OPT_FLAG_VAL_UINT32,0},
	{"encryption",ATMEL_OFFSET(EncryptionLevel),DRVR_OPT_FLAG_VAL_UINT32,0}
	
};

int atmel_init(void *dll_hdl, dispatch_t *dpp, io_net_self_t *ion, char *options) 
{
	Nic_t *nic;
	atmel_handle_t *handle;
	struct sigevent event;
	pthread_attr_t	pattr;
	struct sched_param	param;
	drvr_subopt_tbl_chain_t link;
	int ret;
	
	nic = nic_create_dev(sizeof(atmel_handle_t));
	handle = nic->ext;
	memset(handle,0x0,sizeof(atmel_handle_t));
	handle->Adapter.extra = nic;
	handle->ion = ion;

	//setup a few defaults
	handle->options.nic.promiscuous = -1;
	handle->options.nic.nomulticast = -1;
	handle->options.nic.receive = 30;
	handle->options.channel = 4;
	handle->options.TxRate = 3;
	handle->options.RTSThreshold=2347;
	handle->options.FragThreshold=2346;
	handle->options.OpMode = INFRASTRUCTURE_MODE;
	handle->options.WepKeyToUse = 0;
	strcpy(handle->options.WepKey1,"0000000000000\0");
	strcpy(handle->options.WepKey2,"0000000000000\0");
	strcpy(handle->options.WepKey3,"0000000000000\0");
	strcpy(handle->options.WepKey4,"0000000000000\0");
	strcpy(handle->options.ssid,"");
	handle->options.WepMode = WEP_MODE_MANDATORY;
	handle->options.EncryptionLevel = WEP_DISABLED;
	handle->options.AuthenticationType = C80211_MGMT_AAN_OPENSYSTEM;
	handle->options.PreambleType = LONG_PREAMBLE;
	handle->options.PwrMgmtMode = 0;
	handle->options.BeaconPeriod = 100;
	handle->options.RadioOn = 1;

	link.next = NULL;
	link.table = atmelOptionsTable;
	link.table_size = sizeof(atmelOptionsTable) / sizeof(drvr_subopt_tbl_t);
	if((ret = nic_parse_subopts(&handle->options,"devn-atmel",options,&link)) != EOK) {
		DbgPrint("Failure parsing commandline options");
		errno=ret;
		goto err1;
	}

	if(handle->options.nic.drvr.ioport == 0 || handle->options.nic.drvr.irq == 0) {
		DbgPrint("devn-atmel: You must specify ioport and irq, or start it using pccard-launch\n");
		goto err1;
	}

	/* Probe & Connect to device! */
	handle->baseio = mmap_device_io(VNET_IO_LENGTH,handle->options.nic.drvr.ioport);
	if((void *)handle->baseio == MAP_FAILED) {
		DbgPrint("devn-atmel: mmap_device_io failed!\n");
		goto err2;
	}


	handle->Adapter.IsUp = TRUE;
	handle->Adapter.IoBase = handle->baseio;
	
	InitAdapter(&handle->Adapter);
	
	/* Setup Interrupt/Timer thread and start it */

	handle->chid = ChannelCreate(_NTO_CHF_DISCONNECT | _NTO_CHF_UNBLOCK);
	if(handle->chid == -1) {
		DbgPrint("devn-atmel: Can't create channel");
		goto err3;	
	}

	handle->coid = ConnectAttach(0,0,handle->chid,_NTO_SIDE_CHANNEL,0);
	if(handle->coid == -1) {
		DbgPrint("devn-atmel: Can't create connection");
		goto err4;
	}

	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = handle->coid;
	event.sigev_code = NIC_INTERRUPT_EVENT;
	event.sigev_priority = NIC_PRIORITY;

	#ifdef __EVENT_INTERRUPTS
	handle->iid = InterruptAttachEvent(handle->options.nic.drvr.irq,&event,_NTO_INTR_FLAGS_TRK_MSK);
	#else
		mlock(handle,sizeof(atmel_handle_t));
		handle->iid = InterruptAttach(handle->options.nic.drvr.irq,&irq_handler,handle,sizeof(atmel_handle_t),_NTO_INTR_FLAGS_TRK_MSK);
	#endif
	if(handle->iid == -1) {
		DbgPrint("devn-atmel: Can't attach Interrupt");
		goto err5;
	}
	/* timer */

	event.sigev_code = NIC_TIMER_EVENT;
	if(timer_create(CLOCK_REALTIME,&event,&handle->Adapter.MgmtTimer) == -1) {
		goto err6;
	}

	DbgPrint("devn-atmel: Using IRQ=%d and baseIO=0x%x",handle->options.nic.drvr.irq,handle->baseio);

	pthread_attr_init(&pattr);
	pthread_attr_setschedpolicy(&pattr,SCHED_RR);
	param.sched_priority = 20;
	pthread_attr_setschedparam(&pattr,&param);	
	pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);

	pthread_mutex_init(&handle->mutex,NULL);
	pthread_mutex_lock(&handle->mutex);

	SetParameters(handle);


	if(!InitAndStartCard(&handle->Adapter)) {
		DbgPrint("devn-atmel: Can't start driver!\n");
		errno = ENODEV;
		goto err7;
	}

	//Init powermanagement
	atmel_powermgmt_init(nic);
	atmel_entry.flags |= _REG_POWER_MNGR;
	atmel_entry.pmd_attrp = &handle->pmd;
	atmel_entry.func_hdl = nic;
	if((*ion->reg)(dll_hdl,&atmel_entry,&handle->reg_hdlp,&handle->cell,&handle->endpoint)< 0) {	
		//Sets errno?
		goto err7;
	}

	nic->media = NIC_MEDIA_802_11;
	nic->lan = handle->endpoint;
	nic->flags = NIC_FLAG_WIRELESS;
	nic->mac_length = 6;
	memcpy(&nic->permanent_address,handle->Adapter.PermanentAddress,6);
	memcpy(&nic->current_address,handle->Adapter.CurrentAddress,6);

	QQ_InitQueue(&handle->rx_queue.Container,MAX_RX_PACKET_DESC_COUNT);
	atmel_fetchnpkt(nic,TRUE); //fill the receive pkt queue.

	pthread_create(&handle->tid,&pattr,atmel_event_handler,nic);

	EnableInterrupts(&handle->Adapter);
	pthread_mutex_unlock(&handle->mutex);
	atmel_dl_advert(handle->reg_hdlp,atmel_entry.func_hdl);
return EOK;

err7:
pthread_attr_destroy(&pattr);
pthread_mutex_unlock(&handle->mutex);
pthread_mutex_destroy(&handle->mutex);
CardStop(&handle->Adapter);
err6:
timer_delete(handle->Adapter.MgmtTimer);
err5:
InterruptDetach(handle->iid);
err4:
ConnectDetach(handle->coid);
err3:
ChannelDestroy(handle->chid);
err2:
munmap_device_io(handle->baseio,VNET_IO_LENGTH);
err1:
free(nic->ext);
free(nic);
return ENODEV;
}

int atmel_dl_advert(int registrant_hdl, void *func_hdl)
{
	  npkt_t                  *npkt;
		net_buf_t               *nb;
    net_iov_t               *iov;
    io_net_msg_dl_advert_t  *ap;
		Nic_t *nic = (Nic_t *)func_hdl;
		atmel_handle_t *handle = (atmel_handle_t * )nic->ext;

	DbgPrint("devn-atmel: ->> atmel_dl_advert");

    // 1) Allocate a packet; we'll use this for communications
    //    with io-net.
    if ((npkt = handle->ion->alloc_up_npkt (sizeof (*nb) + sizeof (*iov),
                                (void **) &nb)) == NULL) {
        return (0);
    }

    // 2) Allocate room for the advertisement message.
    if ((ap = handle->ion->alloc (sizeof (*ap), 0)) == NULL) {
        handle->ion->free (npkt);
        return (0);
    }

    // 3) Set up the packet into the queue.
    TAILQ_INSERT_HEAD (&npkt -> buffers, nb, ptrs);

    iov = (net_iov_t *) (nb + 1);

    nb -> niov = 1;
    nb -> net_iov = iov;
    iov -> iov_base = ap;
    iov -> iov_len = sizeof (*ap);

    // 4) Generate the info for the advertisement message.
    memset (ap, 0x00, sizeof (*ap));
    ap -> type          = _IO_NET_MSG_DL_ADVERT;
    ap -> iflags        = (IFF_SIMPLEX | IFF_BROADCAST |
                           IFF_RUNNING);
    ap -> mtu_min       = 0;
    ap -> mtu_max       = 1514;
    ap -> mtu_preferred = 1514;
    sprintf (ap -> up_type, "en%d", handle->endpoint);
    strcpy (ap -> dl.sdl_data, ap -> up_type);

    ap -> dl.sdl_len = sizeof (struct sockaddr_dl);
    ap -> dl.sdl_family = AF_LINK;
    ap -> dl.sdl_index  = handle->endpoint;
    ap -> dl.sdl_type = IFT_ETHER; //or 80211??

    // Not terminated:
    ap -> dl.sdl_nlen = strlen (ap -> dl.sdl_data); 
    ap -> dl.sdl_alen = 6;
    memcpy (ap -> dl.sdl_data + ap -> dl.sdl_nlen,
            handle->Adapter.CurrentAddress, 6);

    // 5) Bind the advertisement message to the packet; note
    //    the use of the _NPKT_MSG flag to indicate to the 
    //    upper modules that this is a message intended for
    //    them. It isn't just a "regular" packet.
    npkt -> org_data = ap;
    npkt -> flags |= _NPKT_MSG | ATMEL_TEMP_PACKET;
    npkt -> iface = 0;
    npkt -> framelen = sizeof (*ap);
		npkt -> tot_iov = 1;

    if (handle->ion->reg_tx_done (handle->reg_hdlp, npkt, NULL) == -1) {
        handle->ion->free (ap);
        handle->ion->free (npkt);
        return (0);
    }

    // 6) Complete the transaction.
    if(handle->ion->tx_up (handle->reg_hdlp, npkt, 0, 0, 
    handle->cell, handle->endpoint, 0) == 0) {                  
    handle->ion->tx_done (handle->reg_hdlp, npkt);       
    }                                                
return 0;
}

int atmel_devctl(void *func_hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret)
{
	Nic_t *nic = (Nic_t *)func_hdl;
	atmel_handle_t *handle = (atmel_handle_t *)nic->ext;
	int status = EOK;
	int promisc;
	nic_wifi_dcmd_t *wifi_info;
	DEVICE_CONFIGURATION DevConfig;


	pthread_mutex_lock(&handle->mutex);
	DisableInterrupts(&handle->Adapter);
	UpdateSupportAppInfo(&handle->Adapter,&DevConfig);

	switch(dcmd)
	{
		case DCMD_IO_NET_NICINFO:
			nic->flags = 0;
			nic->nstats.gstats.xmit_ok = handle->Adapter.Stats.TxDataPacketsOk;
			nic->nstats.gstats.xmit_error = handle->Adapter.Stats.TxDataPacketsError;
			nic->nstats.gstats.rcv_ok = handle->Adapter.Stats.RxDataPacketsOk;
			nic->nstats.gstats.rcv_error = handle->Adapter.Stats.TxDataPacketsError;
			//setup wlan specific shiznas.
//			nic->nstats.un.wstats.
			switch(handle->Adapter.DataFramesTxRate) {
				case 0: 
					nic->media_rate = 1e3;
					break;
				case 1: 
					nic->media_rate = 2e3;
					break;
				case 2: 
					nic->media_rate = 5.5e3;
					break;
				case 3: 
					nic->media_rate = 11e3;
					break;
				default: 
					nic->media_rate = 0;
					break;
			}

			nic->flags |= NIC_FLAG_PIO|NIC_FLAG_BROADCAST|NIC_FLAG_WIRELESS;
			if(handle->Adapter.StationState != STATION_STATE_READY) {
				nic->flags |= NIC_FLAG_LINK_DOWN;
				nic->media_rate = 0;
			}

			nic->mac_length = 6;
			nic->media = NIC_MEDIA_802_11;
			nic->mtu = 1514;
			nic->lan = nic->node = 0;


			memcpy(data,nic,min(size,sizeof(Nic_t)));		
			break;
		case DCMD_IO_NET_TXRX_COUNT:
			memcpy(data,&handle->rxtxstats,sizeof(handle->rxtxstats));
			break;
		case DCMD_IO_NET_PROMISCUOUS:
			promisc = *(int *)data;
			if(promisc) 
				status=ENOTSUP;
			else 
//				SetPromiscuousMode(&handle->Adapter,PROM_MODE_OFF);
				break;
		case DCMD_IO_NET_WIFI:
			wifi_info = (nic_wifi_dcmd_t *)data;
			status = handle_wlan(nic,wifi_info);
			break;
		default:	
			status=ENOTSUP;
			break;
	}
	EnableInterrupts(&handle->Adapter);
	pthread_mutex_unlock(&handle->mutex);
	return status;
}

#ifndef __EVENT_INTERRUPTS 
const struct sigevent *irq_handler(void *area,int id) {
atmel_handle_t *handle = area;
UCHAR InterruptStatus;
INTERRUPT_TYPE InterruptType;
int i=0;

if(!handle->Adapter.IsUp) return 0;

DisableInterrupts(&handle->Adapter);
ProcessInterrupt(&handle->Adapter);
while(i < 100) 
{
	InterruptStatus = GetInterruptStatus (&handle->Adapter);
	InterruptType = GetInterruptType (InterruptStatus);
	i++;
}
InterruptUnmask(handle->options.nic.drvr.irq,handle->iid);
EnableInterrupts(&handle->Adapter);

return 0;
}
#endif

void *atmel_event_handler(void *data)
{
	Nic_t *nic = (Nic_t *)data;
	atmel_handle_t *handle = (atmel_handle_t *)nic->ext;
	int rcvid;
	struct _pulse pulse;
	iov_t	iov;

	SETIOV(&iov,&pulse,sizeof(pulse));
	while(1) {
		rcvid = MsgReceivev(handle->chid,&iov,1,NULL);
		if(rcvid == -1) {
			if(errno == ESRCH)
				pthread_exit(NULL);
			continue;
		}
		if(handle->Adapter.IsUp)
		{
			pthread_mutex_lock(&handle->mutex);
			switch(pulse.code) {
				case NIC_INTERRUPT_EVENT:
					DisableInterrupts(&handle->Adapter); 
					ProcessInterrupt(&handle->Adapter);
					EnableInterrupts(&handle->Adapter);
					InterruptUnmask(handle->options.nic.drvr.irq,handle->iid);
					break;
				case NIC_TIMER_EVENT:
						MgmtTimeOutCallBack(&handle->Adapter);	
					break;
				default:
					if(rcvid)
						MsgReplyv (rcvid,ENOTSUP,&iov,1);
				break;
			}
			pthread_mutex_unlock(&handle->mutex);
		}
	}
}
int atmel_flush(int registrant_hdl,
              void *func_hdl)
{

return 0;
}
int atmel_rx_down(npkt_t *npkt, void *func_hdl)
{
	Nic_t *nic = (Nic_t *)func_hdl;
	atmel_handle_t *handle = (atmel_handle_t *)nic->ext;
	net_buf_t *nb;
	net_iov_t *iov;
	int len;
	USHORT StartOfTxBuffer = 0;
	ULONG	TotalBytes = 0;
	io_net_msg_join_mcast_t *msg;
	int i;
	char *toosmallfragbuffer=0;
	int toosmallfraglen=0;

	IF_LOUD(DbgPrint(">> rx_down");)
	if(handle->Adapter.StationState != STATION_STATE_READY) {
		IF_LOUD(DbgPrint("Station is not ready");)
		handle->ion->tx_done(handle->reg_hdlp,npkt);
		return TX_DOWN_FAILED;
	}
	if(npkt->next) {
		IF_LOUD(DbgPrint("Several packets ain't supported yet!");)
		handle->ion->tx_done(handle->reg_hdlp,npkt);
		return TX_DOWN_FAILED;
	}	

	if(npkt->flags & _NPKT_MSG) {
		nb = TAILQ_FIRST(&npkt->buffers);
		if(nb != NULL && NULL != (msg = (io_net_msg_join_mcast_t*)nb->net_iov->iov_base)) 
		{
			//XXX:TODO:!!!
		}
		IF_LOUD(DbgPrint("Multicast packet!!!");)
		handle->ion->tx_done(handle->reg_hdlp,npkt);
		return TX_DOWN_FAILED;
	}

	pthread_mutex_lock(&handle->mutex);
	len = npkt->framelen;
	TotalBytes=len;
	if(TxResourcesAvailable(&handle->Adapter,len+18,&StartOfTxBuffer) == FALSE)
	{
		//Package lost
		handle->Adapter.Stats.RxLost++;
		pthread_mutex_unlock(&handle->mutex);
		handle->ion->tx_done(handle->reg_hdlp,npkt);
		return TX_DOWN_FAILED;	
	}
#ifdef ATMEL_WLAN
  if(handle->Adapter.OperatingMode == INFRASTRUCTURE_MODE){ //In ESS the final DA is always the AP
            handle->Adapter.IsBroadcast = FALSE;
  }else {
    if ( (*((PUCHAR)(TAILQ_FIRST(&npkt->buffers)->net_iov->iov_base))) & 0x01 ){
      handle->Adapter.IsBroadcast = TRUE;
    }else {
      handle->Adapter.IsBroadcast = FALSE;
    }
  }
#endif
	for (nb = TAILQ_FIRST(&npkt->buffers);nb;nb = TAILQ_NEXT (nb, ptrs)) {
   	for (i = 0, iov = nb -> net_iov; i < nb -> niov; i++, iov++) {
			//i'm paranoid, this function will break if first fragment is smaller then 12 bytes
			if((i == 0 && iov->iov_len < 15) || toosmallfragbuffer) {
				//Let's assemble it manually as we don't got the full header. :(
				if(!toosmallfragbuffer) 
					toosmallfragbuffer = (char*) malloc(npkt->framelen);

				IF_LOUD(DbgPrint("Too small fragment, assembling by copying\n");)
				memcpy(toosmallfragbuffer+toosmallfraglen,iov->iov_base,iov->iov_len);
				toosmallfraglen += iov->iov_len;
			} else {
				IF_VERY_LOUD(DbgPrint("TxEthernetPacket %d bytes",iov->iov_len);)
				TxEthernetPacket(&handle->Adapter,iov->iov_base,iov->iov_len,&TotalBytes,StartOfTxBuffer, (i == 0) ? TRUE : FALSE);
			}
    }
	}
	if(toosmallfragbuffer) {
		TxEthernetPacket(&handle->Adapter,toosmallfragbuffer,npkt->framelen,&TotalBytes,StartOfTxBuffer,1);
		free(toosmallfragbuffer);
	}

	IF_VERY_LOUD(DbgPrint("TotalBytes = %d,npkt->framelen = %d",TotalBytes,npkt->framelen);)
	TxUpdateDescriptor(&handle->Adapter,TotalBytes,StartOfTxBuffer,TRUE);

	handle->rxtxstats.tx_bytes+=TotalBytes;
	pthread_mutex_unlock(&handle->mutex);
	//free packet here???
	handle->ion->tx_done(handle->reg_hdlp,npkt);
	IF_LOUD(DbgPrint("<< Rx_down");)
	return TX_DOWN_OK;
}

int atmel_tx_done(npkt_t *npkt, void *done_hdl, void *func_hdl)
{
	Nic_t *nic = (Nic_t *)func_hdl;
	atmel_handle_t *handle = (atmel_handle_t *)nic->ext;

	//move packet back to queue or free it...
	if(npkt->flags & ATMEL_TEMP_PACKET) {
		handle->ion->free(npkt->org_data);
		handle->ion->free(npkt);
	} else {
		QQ_PushHead(&handle->rx_queue.Container,npkt);
	}
	return 0;
}


int atmel_shutdown1(int registrant_hdl, void *func_hdl)
{
atmel_flush(registrant_hdl,func_hdl);
return EOK;
}
int atmel_shutdown2(int registrant_hdl, void *func_hdl)
{
Nic_t *nic = (Nic_t *)func_hdl;
atmel_handle_t *handle = (atmel_handle_t *)nic->ext;

pthread_mutex_lock(&handle->mutex);
DisableInterrupts(&handle->Adapter);
timer_delete(handle->Adapter.MgmtTimer);
CardStop(&handle->Adapter);
handle->Adapter.IsUp = FALSE;
pthread_mutex_unlock(&handle->mutex);
InterruptDetach(handle->iid);
#ifndef __EVENT_INTERRUPTS
munlock(handle,sizeof(atmel_handle_t));
#endif

ConnectDetach(handle->coid);
ChannelDestroy(handle->chid);
pthread_join(handle->tid,NULL);

pthread_mutex_destroy(&handle->mutex);
munmap_device_io(handle->baseio,VNET_IO_LENGTH);
free(nic->ext);
free(nic);
return 0;
}

//if we have a unused packet in the rx queue, use that, else allocate a new temporary packet.
//if the last parameter is true, the create and fill a queue with packets.
npkt_t *atmel_fetchnpkt(Nic_t *nic,int fillqueue) 
{
	atmel_handle_t *handle = (atmel_handle_t * ) nic->ext;	
	npkt_t *pkt;
	int i;

	if(fillqueue) {
		if(!QQ_Empty(&handle->rx_queue.Container)) {
			IF_LOUD(DbgPrint("atmel_fetchnpkt was called with fillqueue=true with a nonempty queue!!!");)
			return NULL; 
		}
		for(i=0;i<handle->options.nic.receive;i++) {
			pkt = newpkt(handle);
			QQ_PushHead(&handle->rx_queue.Container,pkt);
		}
	}
	if(!(pkt = QQ_PopHead(&handle->rx_queue.Container)) )
	{
		IF_VERY_LOUD(DbgPrint("No packets in rx_queue, allocating a temporary pkt");)
		pkt = newpkt(handle);
		pkt->flags |=ATMEL_TEMP_PACKET; 
	}
return pkt;
}

npkt_t *newpkt(atmel_handle_t *handle) 
{
	npkt_t *npkt;
	net_buf_t *nb;
	net_iov_t *iov;
	void *data;


	npkt = handle->ion->alloc_up_npkt(sizeof(*nb) + sizeof(*iov),(void **)&nb);
	if(npkt == NULL) {
		IF_LOUD(DbgPrint("Can't allocate a new package, errno=%d",errno);)
		return NULL;
	}
	data = handle->ion->alloc(MAX_WIRELESS_FRAME_SIZE,0);
	if(data == NULL) {
		IF_LOUD(DbgPrint("Can't allocate npkt_t data, errno=%d",errno);)
		handle->ion->free(npkt);
		return NULL;
	}
	TAILQ_INSERT_HEAD(&npkt->buffers,nb,ptrs);
	iov = (net_iov_t *)(nb+1);
	nb->niov=1;
	nb->net_iov = iov;
	iov->iov_base = data;
	iov->iov_phys = (paddr_t)handle->ion->mphys(iov->iov_base); //XXX: Do we have to do this? they want the phys or virtual address higher up in the hieriarchy?
	iov->iov_len = MAX_WIRELESS_FRAME_SIZE;
	npkt->org_data = data;
	npkt->next = NULL;
	npkt->tot_iov = 1;

	return npkt;
}

void SetParameters(atmel_handle_t *handle) {
	int i;
	VNet_ADAPTER *Adapter = &handle->Adapter;	
	atmel_options_t *options = &handle->options;

	Adapter->Channel = options->channel;
	Adapter->DataFramesTxRate = options->TxRate;
	Adapter->OperatingMode = options->OpMode;
	Adapter->PreambleType = options->PreambleType;
	Adapter->PowerMgmtMode = options->PwrMgmtMode;
	Adapter->IntRoamingEnabled=0;

	VNetMoveMemory(Adapter->DesiredSSID,options->ssid,MAX_SSID_LENGTH);
	VNetMoveMemory(Adapter->InitialSSID,options->ssid,MAX_SSID_LENGTH);

	Adapter->InitialSSIDsize = Adapter->SSID_Size = strlen(Adapter->DesiredSSID);
	if(Adapter->InitialSSIDsize == 0) 
		Adapter->ConnectToAnyBSS = TRUE;
	else
		Adapter->ConnectToAnyBSS = FALSE;

	Adapter->RadioIsOn = TRUE; //options->RadioOn;

	//RTSThreshold
	Adapter->RtsThreshold = options->RTSThreshold;
	Adapter->FragmentationThreshold = options->FragThreshold;
	
	Adapter->WepInfo.WepKeyToUse = options->WepKeyToUse;
	Adapter->WepInfo.EncryptionLevel = options->EncryptionLevel;
	Adapter->WepInfo.WepMode = options->WepMode;
	Adapter->WepInfo.AuthenticationType = options->AuthenticationType;
	switch(Adapter->WepInfo.EncryptionLevel)
	{
		case 0 :
			Adapter->WepInfo.KeyLength =0;
			Adapter->WepInfo.PairwiseCipherSuite = CIPHER_SUITE_NONE;
			break;
		case 1 :
			Adapter->WepInfo.KeyLength =5;
			Adapter->WepInfo.PairwiseCipherSuite = CIPHER_SUITE_WEP_64;
			break;
		case 2 :
			Adapter->WepInfo.KeyLength =13;
			Adapter->WepInfo.PairwiseCipherSuite = CIPHER_SUITE_WEP_128;
			break;
	}
	Adapter->DefaultBeaconPeriod = Adapter->BeaconPeriod = options->BeaconPeriod;	

  for (i = 0; i < 4;i++)
    {
				char *WepKey = NULL; 
				switch(i) {
					case 0: 
						WepKey = handle->options.WepKey1;
						break;	
					case 1: 
						WepKey = handle->options.WepKey2;
						break;	
					case 2: 
						WepKey = handle->options.WepKey3;
						break;	
					case 3: 
						WepKey = handle->options.WepKey4;
						break;	
				}
				memcpy(Adapter->WepInfo.KeyMaterial[i],WepKey,LONG_WEP_KEY_SIZE);
    }
		//?????
		Adapter->WepInfo.KeyMaterial[0][39] = 1;
}

int handle_wlan(Nic_t *nic,nic_wifi_dcmd_t *winfo) {
int status = EOK;
atmel_handle_t *handle = (atmel_handle_t *)nic->ext;
BSS_INFO *CurBssInfo = &handle->Adapter.BSSinfo[(int)handle->Adapter.CurrentBSSindex];


	switch(winfo->subcmd & NIC_WIFI_SUBCMD_MASK) {
		case NIC_WIFI_SUBCMD_SSID:
			if(winfo->subcmd & DEVDIR_TO) //set new ssid to use
			{ //XXX:TODO
			}
			if(winfo->subcmd & DEVDIR_FROM) { 
				strncpy(winfo->un.ssid,CurBssInfo->SSID,CurBssInfo->SSIDsize);
			}
		status = EOK;
		break;
		case NIC_WIFI_SUBCMD_BSS_TYPE:
			if(winfo->subcmd & DEVDIR_FROM) {
				switch(handle->Adapter.BSSinfo[(int)handle->Adapter.CurrentBSSindex].BSStype)
				{
					case BSS_TYPE_AD_HOC:
						winfo->un.bss_type = NIC_WIFI_BSS_TYPE_ADHOC;
					break;
					case BSS_TYPE_INFRASTRUCTURE:
						winfo->un.bss_type = NIC_WIFI_BSS_TYPE_AP;
					break;
					default:
						winfo->un.bss_type = NIC_WIFI_BSS_TYPE_AP;
				} 
			status = EOK;
			}
			if(winfo->subcmd & DEVDIR_TO) {
				status = ENOTSUP;
			}
		break;
		case NIC_WIFI_SUBCMD_STATION_NAME: //FOR SNMP
			status = ENOTSUP;
		break;
		case NIC_WIFI_SUBCMD_CHANNEL:
			winfo->un.channel = handle->Adapter.Channel;
			status = EOK;
		case NIC_WIFI_SUBCMD_AUTH_TYPE:
			if(winfo->subcmd & DEVDIR_TO) {

			}
			if(winfo->subcmd & DEVDIR_FROM) {
				switch(handle->Adapter.WepInfo.AuthenticationType){
					case C80211_MGMT_AAN_OPENSYSTEM:
						winfo->un.auth_type = NIC_WIFI_AUTH_TYPE_OPEN;
						break;
					case C80211_MGMT_AAN_SHAREDKEY:
						winfo->un.auth_type = NIC_WIFI_AUTH_TYPE_SHARED_KEY;
						break;
					default: 
						winfo->un.auth_type = NIC_WIFI_AUTH_TYPE_OPEN;
				}
			status = EOK;
			}
		break;
		case NIC_WIFI_SUBCMD_CRYPTO_TYPE:
			if(winfo->subcmd & DEVDIR_TO) {

			}
			if(winfo->subcmd & DEVDIR_FROM) {
				switch(handle->Adapter.WepInfo.EncryptionLevel)
				{
					case 	WEP_DISABLED:
						winfo->un.crypto_type = NIC_WIFI_CRYPTO_TYPE_NONE;
						break;
				//XXX: TODO! ADD MORE!
					default: 
						winfo->un.crypto_type = NIC_WIFI_CRYPTO_TYPE_NONE;
				}
			status = EOK;
			}
		break;
		case NIC_WIFI_SUBCMD_CRYPTO_DATA:
			if(winfo->subcmd & DEVDIR_TO) {

			}
			if(winfo->subcmd & DEVDIR_FROM) {

			}
		break;
		case NIC_WIFI_SUBCMD_CRYPTO_CFG:
			if(winfo->subcmd & DEVDIR_TO) {

			}
			if(winfo->subcmd & DEVDIR_FROM) {

			}
		break;
		case NIC_WIFI_SUBCMD_SIGNAL_INFO:
			if(winfo->subcmd & DEVDIR_TO) {
				status = ENOTSUP;
				break;
			}
			if(winfo->subcmd & DEVDIR_FROM) {

			/*			winfo->un.signal_info.radio_freq = 
			winfo->un.signal_info.tx_rate =
			winfo->un.signal_info.quality = 
			winfo->un.signal_info.signal_level = 
			winfo->un.signal_info.noise_level = 	
			*/
			}
		break;
		case NIC_WIFI_SUBCMD_RTS_THRESH:
			if(winfo->subcmd & DEVDIR_TO) {
				status = ENOTSUP;
				break;
			}
			if(winfo->subcmd & DEVDIR_FROM) {
				winfo->un.rts_thresh = handle->Adapter.RtsThreshold;
			}
		break;
		case NIC_WIFI_SUBCMD_FRAG_THRESH:
			if(winfo->subcmd & DEVDIR_TO) {
				status = ENOTSUP;
				break;
			}
			if(winfo->subcmd & DEVDIR_FROM) {
			winfo->un.frag_thresh = handle->Adapter.FragmentationThreshold;
			}
		break;
		case NIC_WIFI_SUBCMD_SCAN_NETWORK:
			//rescan!
		break;
		case NIC_WIFI_SUBCMD_BSSID:
			memcpy(&winfo->un.bssid_cfg.macaddr,handle->Adapter.BSSinfo[(int)handle->Adapter.CurrentBSSindex].BSSID,6);
			winfo->un.bssid_cfg.channel = 	handle->Adapter.BSSinfo[(int)handle->Adapter.CurrentBSSindex].Channel;
		break;
		case NIC_WIFI_SUBCMD_CONFIGURE: //...?
		break;
		default: 
		status = ENOTSUP;
	}

	return status;
}

VOID UpdateSupportAppInfo(PVNet_ADAPTER Adapter,PDEVICE_CONFIGURATION pDevConfig) 
{
	int i;

  pDevConfig->StationState = Adapter->StationState;
  pDevConfig->OperatingMode = Adapter->OperatingMode;
  pDevConfig->Channel = (Adapter->Channel & 0x7f);


  VNetMoveMemory (pDevConfig->SSID, Adapter->DesiredSSID, Adapter->SSID_Size);
  pDevConfig->SSIDlength = Adapter->SSID_Size;

  VNetMoveMemory (pDevConfig->BSSID, Adapter->CurrentBSSID, 6);
  pDevConfig->InternationalRoaming = Adapter->IntRoamingEnabled;
  if (pDevConfig->InternationalRoaming)
  {
	  for (i = 0; i < 14; i++)
		{
		  pDevConfig->ChannelVector[i] = Adapter->ChannelVector[i];
		}
  }

  pDevConfig->MgmtErrorCode = Adapter->MgmtErrorCode;
  pDevConfig->RegDomain = Adapter->RegDomain;
  pDevConfig->PreambleType = Adapter->PreambleType;
  pDevConfig->PowerMgmtMode = Adapter->PowerMgmtMode;
  pDevConfig->RadioIsOn = Adapter->RadioIsOn;

  pDevConfig->TxRate = Adapter->DataFramesTxRate;
  pDevConfig->FragmentationThreshold = Adapter->FragmentationThreshold;
  pDevConfig->RtsCtsThreshold = Adapter->RtsThreshold;
	if(Adapter->RadioIsOn){
	  pDevConfig->Rssi = Adapter->Rssi;
	  pDevConfig->LinkQuality = 40 - Adapter->LinkQuality;
	}else{
	  pDevConfig->Rssi = 0;
	  pDevConfig->LinkQuality = 0;
	}
  if ((Adapter->StationState != 0x05) || (Adapter->OperatingMode == AD_HOC_MODE)){
	  pDevConfig->Rssi = 0;
	  pDevConfig->LinkQuality = 0;
  }
}


/** Powermanagement **/

int atmel_setpower(void *data,pm_power_mode_t mode,unsigned flags) 
{
	Nic_t *nic = (Nic_t *)data;
	atmel_handle_t *handle = nic->ext;
	int status= EOK;

	pthread_mutex_lock(&handle->mutex);
	DisableInterrupts(&handle->Adapter);
	switch(mode) {
		case PM_MODE_ACTIVE:
			if(handle->pmd.pm_attr.cur_mode == PM_MODE_ACTIVE) {
				goto exit_setpower;
			}
/*		b44_LM_InitializeAdapter(pDevice);
		b44_LM_EnableInterrupt(pDevice);
		MM_ATOMIC_SET(&pUmDevice->asleep,0);
*/
		break;
		case PM_MODE_OFF:
			if(handle->pmd.pm_attr.cur_mode == PM_MODE_OFF) {
				goto exit_setpower;
			}
/*			b44_LM_DisableInterrupt(&pUmDevice->lm_dev);
			b44_LM_Halt(pDevice);
			bcm4400_free_remaining_rx_bufs(pUmDevice);
			bcm4400_freemem(pUmDevice);
			b44_LM_PowerDownPhy(pDevice);
			MM_ATOMIC_SET(&pUmDevice->asleep,1);*/
		break;
		default:
			status = EINVAL;
			goto exit_setpower;
	}


exit_setpower:
	EnableInterrupts(&handle->Adapter);
	pthread_mutex_unlock(&handle->mutex);
return status;
}

void atmel_powermgmt_init(Nic_t *nic) 
{
atmel_handle_t *handle = nic->ext;
pmd_attr_t *dev_pmd = &handle->pmd;
static pm_power_mode_t dev_modes[] = {PM_MODE_OFF,PM_MODE_ACTIVE};


pmd_attr_init(dev_pmd);
pmd_attr_setmodes(dev_pmd,PM_MODE_ACTIVE,dev_modes,2);
pmd_attr_setpower(dev_pmd,atmel_setpower,nic);

}
