#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/slog.h>
#include <hw/inout.h>
#include <hw/pci.h>
#include <gulliver.h> //for endianness
#include <drvr/support.h>
#include <drvr/eth.h>
#include <drvr/mdi.h>
#include <sys/io-net.h>
#include <net/if_types.h>
#include <net/if_dl.h>
#include <net/if.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/dcmd_io-net.h>
#include <sys/neutrino.h>
#include <atomic.h>
#include <sys/slogcodes.h>
#include <assert.h>
//powermanagement
#include <sys/pm.h>

#include "b44queue.h"

#define TRUE 1
#define FALSE 0 

#define IF_VNetDEBUG(f) if(1) 

#define VNET_IO_LENGTH 0x20
#define NIC_INTERRUPT_EVENT 0
#define NIC_TIMER_EVENT 1
#define NIC_PRIORITY            21
#define ATMEL_TEMP_PACKET 0x79c00000

#define __NO_VERSION__
#if defined(PCMCIA_DEBUG)||defined(PCI_DEBUG)

#define DbgPrint(fmt, arg...) slogf(_SLOGC_NETWORK_NEXTFREE,_SLOG_WARNING,fmt,##arg)
#else
#define DbgPrint(fmt,arg...) 
#endif

typedef struct {
	nic_options_t nic;

	//atmel wlan specific options
	uint32_t mtu;
	char ssid[MAX_SSID_LENGTH+1];
	uint32_t channel;
	uint32_t TxRate;
	uint32_t OpMode;
	uint32_t WepKeyToUse;
	char WepKey1[(LONG_WEP_KEY_SIZE*2)+1];
	char WepKey2[(LONG_WEP_KEY_SIZE*2)+1];
	char WepKey3[(LONG_WEP_KEY_SIZE*2)+1];
	char WepKey4[(LONG_WEP_KEY_SIZE*2)+1];
	uint32_t WepMode;
	uint32_t EncryptionLevel;
	uint32_t AuthenticationType;
	uint32_t PreambleType;
	uint32_t PwrMgmtMode;
	uint32_t BeaconPeriod;
	uint32_t RadioOn;
	uint32_t RTSThreshold;
	uint32_t FragThreshold;
}atmel_options_t;

#define MAX_RX_PACKET_DESC_COUNT 512 
DECLARE_QUEUE_TYPE(RX_PKT_Q,MAX_RX_PACKET_DESC_COUNT+1);

typedef struct {
VNet_ADAPTER Adapter;

atmel_options_t options;

pmd_attr_t pmd;
pthread_mutex_t mutex;
uintptr_t baseio;
io_net_self_t *ion;
int reg_hdlp;
NicTxRxCount_t rxtxstats;
uint16_t	cell;
uint16_t	endpoint; //position inside io-net...
RX_PKT_Q	rx_queue;
int iid; //Interrupt ID
int coid; //Connection ID for pulses 
int chid; //Channel ID   - || - 
int tid; //thread ID (event handler)
}atmel_handle_t;

int atmel_init (void *dll_hdl,
             dispatch_t *dpp,
             io_net_self_t *ion,
             char *options);
int atmel_devctl(void *func_hdl, int dcmd, void *data, size_t size, union _io_net_dcmd_ret_cred *ret);
int atmel_init (void *dll_hdl, dispatch_t *dpp, io_net_self_t *ion, char *options);
int atmel_master_shutdown(void *dll_hdl);
int atmel_flush (int registrant_hdl, void *func_hdl);
int atmel_rx_down(npkt_t *npkt, void *func_hdl);
int atmel_tx_done(npkt_t *npkt, void *done_hdl, void *func_hdl);
int atmel_shutdown1 (int registrant_hdl, void *func_hdl);
int atmel_shutdown2 (int registrant_hdl, void *func_hdl);
int atmel_dl_advert(int registrant_hdl, void *func_hdl);
int str2opt(void *hdl,char *dev,char *name,char *val);
void *atmel_event_handler(void*);
void SetParameters(atmel_handle_t *);
void print_usage();
int handle_wlan(Nic_t *nic,nic_wifi_dcmd_t *winfo);
npkt_t *atmel_fetchnpkt(Nic_t *nic,int fillqueue);
npkt_t *newpkt(atmel_handle_t *);
#ifndef __EVENT_INTERRUPTS
const struct sigevent *irq_handler(void *,int);
#endif

int atmel_setpower(void *data,pm_power_mode_t mode,unsigned flags);
void atmel_powermgmt_init(Nic_t *nic);
VOID UpdateSupportAppInfo(PVNet_ADAPTER Adapter,PDEVICE_CONFIGURATION pDevConfig);
