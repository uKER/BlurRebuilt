// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdCore/bdCore.h>
#include <bdNet/bdNet.h>

#include <malloc.h>

#include <eekernel.h>
#include <sif.h>
#include <sifdev.h>

#include <libeenet.h>

#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include <netcnfif.h>
#include <libeenet/eenetctl.h>


/*  --------- DEFINE FOR IOP MODULES -------- */

#if 1
#define HDD_ETHERNET
#endif
#if 0
#define HDD_ETHERNET_WITH_PPPOE
#endif
#if 0
#define USB_ETHERNET
#endif
#if 0
#define USB_ETHERNET_WITH_PPPOE
#endif
#if 0
#define MODEM
#endif

#if 1
#define UP_INTERFACE_AUTO
#endif

/* config name */
#if defined(USB_ETHERNET)
#define USR_CONF_NAME     "Combination4"
#elif defined(USB_ETHERNET_WITH_PPPOE)
#define USR_CONF_NAME     "Combination5"
#elif defined(HDD_ETHERNET)
#define USR_CONF_NAME     "Combination6"
#elif defined(HDD_ETHERNET_WITH_PPPOE)
#define USR_CONF_NAME     "Combination7"
#elif defined(MODEM)
#define USR_CONF_NAME     ""
#endif


#include <sys/types.h>
#include <eekernel.h>
#if defined(USE_ERX)
#include <liberx.h>
#include <libgcc_export.h>
#include <libc_export.h>
#endif 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <sifrpc.h>
#include <sifdev.h>

#if defined(HDD_ETHERNET) || defined(HDD_ETHERNET_WITH_PPPOE)
//#include "poweroff.h"
#endif

#include <libeenet.h>
#include <eenetctl.h>

/* ent_cnf sifrpc num */
#define ENT_CNF_SIFRPC_NUM 0x0a31108e

/* code */
#define ENT_CNF_SIFRPC_LOAD_CONFIG      1
#define ENT_CNF_SIFRPC_SET_CONFIG       2
#define ENT_CNF_SIFRPC_SET_CONFIG_ADDR  3

/* utility macro */
#define ee_rpc_size(size) ((size + 15) & ~15)
#define iop_rpc_size(size) ((size + 3) & ~3)

#ifdef __cplusplus
extern "C" {
#endif

	void *sceEENetCnfGetErxEntries(void);

	int ent_cnf_init(void);
	int ent_cnf_load_config(const char *fname, const char *usr_name);
	int ent_cnf_set_config(void);
	int ent_cnf_set_config_addr(u_int addr);

#ifdef __cplusplus
}
#endif

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#if defined(USE_ERX)
#define LIBERX_THREAD_PRIORITY      10
#define LIBERX_MEMORY_SIZE          (8*1024*1024)
#define LIBERX_MAX_MODULES          32

static int libcdvd_erxid, libscf_erxid;
static int libeenet_erxid;
static int eenetctl_erxid;
static int ent_cnf_erxid;
#endif

//defines for eenet 
#define EENET_MEMSIZE	512*1024	/* 512KB */
#define EENET_TPL		32
#define EENET_APP_PRIO	48

static int eenetctl_mid;
static int ent_cnf_mid;
static int ent_devm_mid;
static int ent_dev9_mid;
static int net_cnf_mid;

#if defined(HDD_ETHERNET) || defined(HDD_ETHERNET_WITH_PPPOE)
#define EENET_IFNAME "smap0"
static int ent_smap_mid;
#if defined(USE_ERX)
static int ent_smap_erxid;
#endif
#endif
#if defined(USB_ETHERNET) || defined(USB_ETHERNET_WITH_PPPOE)
#define EENET_IFNAME "eth0"
static int ent_eth_mid;
#if defined(USE_ERX)
static int ent_eth_erxid;
#endif
#endif
#if defined(MODEM)
#define EENET_IFNAME "ppp0"
static int ent_ppp_mid;
#if defined(USE_ERX)
static int ent_ppp_erxid;
#endif
static int modem_mid;
#endif

//#define return -1; while(1)
#define BUFSIZE 1460


static int sema_id;
static int event_flag = 0;
#define Ev_Attach         0x01
#define Ev_UpCompleted    0x02
#define Ev_DownCompleted  0x04
#define Ev_DetachCompleted 0x08
#define Ev_Error 0x10

#define WaitEvent(event) \
	while(1){ \
	WaitSema(sema_id); \
	if(event_flag & (event)) \
	break; \
	} \

#if defined(USE_ERX)
int InitErxEnvironment(void *pool, int pool_size);
#endif

static void event_handler(const char *ifname, int af, int type){

	//sceBD_INFO("info","event_handler: event happened. af = %d, type = %d\n",
	//	af, type);

	switch(type){
	case sceEENETCTL_IEV_Attach:
		if(strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_Attach;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_Error:
		{
			int pcode, psubcode;

			sceEENetCtlGetErrorCode(ifname, af, &pcode, &psubcode);
			BD_INFO("info","%s(af=%d): error %d/%d\n", ifname, af, pcode, psubcode);
			event_flag |= Ev_Error;
			SignalSema(sema_id);
			break;
		}
	case sceEENETCTL_IEV_UpCompleted:
		event_flag |= Ev_UpCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DownCompleted:
		event_flag |= Ev_DownCompleted;
		SignalSema(sema_id);
		break;
	case sceEENETCTL_IEV_DetachCompleted:
		if(strcmp(ifname, EENET_IFNAME))
			break;
		event_flag |= Ev_DetachCompleted;
		SignalSema(sema_id);
		break;
	}

	return;
}

#define IOP_MAIN_ROOT	"host0:../../../../iop/modules" //"host0:/usr/local/sce/iop/modules/"
#define SETAPP_ROOT	"host0:../../../../conf/neticon/english/"
#define IOP_MOD_DEV9	"dev9.irx"
#define IOP_MOD_USBD	"usbd.irx"
#define IOP_MOD_NETCNF	"netcnf.irx"
#define NETCNF_ICON	SETAPP_ROOT "SYS_NET.ICO"
#define NETCNF_ICONSYS	SETAPP_ROOT "icon.sys"
#define NETCNF_ARG	"icon=" NETCNF_ICON "\0" "iconsys=" NETCNF_ICONSYS
#define NET_DB		"host0:../../../../conf/net/net.db"

#define EENET_IOP_TOP		"host0:../../../../iop/"
#define EENET_IOP_MODULES	EENET_IOP_TOP "modules/"
#define IOP_MOD_EENETCTL	"eenetctl.irx"
#define IOP_MOD_ENT_SMAP	"ent_smap.irx"
#define IOP_MOD_ENT_ETH		"ent_eth.irx"
#define IOP_MOD_ENT_PPP 	"ent_ppp.irx"
#define IOP_MOD_ENT_DEVM	"ent_devm.irx"
//#define IOP_MOD_ENT_CNF		EENET_IOP_TOP "sample/libeenet/ent_cnf/ent_cnf.irx"
#define IOP_MOD_ENT_CNF		"ent_cnf.irx"
#define IOP_MOD_MODEMDRV	""
#define MODEMDRV_ARG		""

/* erx */
#if defined(USE_ERX)
#define EE_TOP				"host0:/usr/local/sce/ee/modules/"
#define EE_MOD_LIBCDVD		EE_TOP "libcdvd.erx"
#define EE_MOD_LIBSCF		EE_TOP "libscf.erx"
#define EENET_EE_TOP		"host0:/usr/local/sce/ee/"
#define EENET_EE_MODULES	EENET_EE_TOP "modules/"
#define EE_MOD_LIBEENET		EENET_EE_MODULES "libeenet.erx"
#define EE_MOD_EENETCTL		EENET_EE_MODULES "eenetctl.erx"
#define EE_MOD_ENT_DUMP		EENET_EE_MODULES "ent_dump.erx"
#define EE_MOD_ENT_SMAP		EENET_EE_MODULES "ent_smap.erx"
#define EE_MOD_ENT_ETH		EENET_EE_MODULES "ent_eth.erx"
#define EE_MOD_ENT_PPP 		EENET_EE_MODULES "ent_ppp.erx"
#define EE_MOD_ENT_CNF		EENET_EE_TOP "sample/libeenet/ent_cnf/ent_cnf.erx"
#endif

void *libeenet_pool;


static int create_sema(int init_count, int max_count){
	struct SemaParam sema_param;
	int sema_id;

	memset(&sema_param, 0, sizeof(struct SemaParam));
	sema_param.initCount = init_count;
	sema_param.maxCount = max_count;
	sema_id = CreateSema(&sema_param);

	return sema_id;
}

bdInt loadModule(const bdNChar8* modulePath, const bdNChar8* moduleName, bdInt argSize, bdNChar8 *args)
{
	bdNChar8 module[2048];
	bdSnprintf(module, 2048, "%s/%s", modulePath, moduleName);
	bdInt moduleID = sceSifLoadModule(module, argSize, args);
	if (moduleID < 0)
	{
		BD_ERR("err","Failed to load module %s\n", module);
	}
	return moduleID;
}

int initAll(const bdNChar8* modulePath)
{
	if((sema_id = create_sema(0, 255)) < 0)
	{
		BD_INFO("info","create_sema() failed.\n");
		return -1;
	}
	int ret = 0;
	bdInt threadPriority = BD_DEFAULT_THREAD_PRIORITY + 48;
	BD_INFO("info","Setting main thread priority to %u\n", threadPriority);
	ret = ChangeThreadPriority(GetThreadId(), threadPriority);
	#if defined(USE_ERX)
	/* init erx library */
	if((liberx_pool = memalign(256, LIBERX_MEMORY_SIZE)) == NULL)
	{
		BD_INFO("info","memalign() failed.\n");
		return -1;
	}
	if(InitErxEnvironment(liberx_pool, LIBERX_MEMORY_SIZE) < 0)
	{
		BD_INFO("info","InitErxEnvironment() failed.\n");
		return -1;
	}

	/* load libcdvd/libscf erx */
	while((libcdvd_erxid = sceErxLoadStartModuleFile(EE_MOD_LIBCDVD, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","libcdvd.erx cannot be loaded.\n");
	while((libscf_erxid = sceErxLoadStartModuleFile(EE_MOD_LIBSCF, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","libscf.erx cannot be loaded.\n");
	#endif

	/* init sif stuff */
	sceSifInitRpc(0);

	/* load iop modules */
	//while((ent_devm_mid = sceSifLoadModule(IOP_MOD_ENT_DEVM, 0, NULL)) < 0)
	if ((ent_devm_mid = loadModule(modulePath, IOP_MOD_ENT_DEVM, 0, NULL)) <0)
	{
		return -1;
	}
	
	if ((net_cnf_mid = loadModule(modulePath,IOP_MOD_NETCNF, sizeof(NETCNF_ARG), NETCNF_ARG)) < 0)
	{
		return -1;
	}
	if ((eenetctl_mid = loadModule(modulePath,IOP_MOD_EENETCTL, 0, NULL)) < 0)
	{
		return -1;
	}

	if ((ent_cnf_mid = loadModule(modulePath,IOP_MOD_ENT_CNF, 0, NULL)) < 0)
	{
		return -1;
	}

	#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	if ((ent_dev9_mid = loadModule(modulePath,IOP_MOD_DEV9, 0, NULL)) < 0)
	{
		return -1;
	}
	if ((ent_smap_mid = loadModule(modulePath,IOP_MOD_ENT_SMAP, 0, NULL)) < 0)
	{
		return -1;
	}
//	PreparePowerOff();
	#endif
	#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	if (loadModule(modulePath,IOP_MOD_USBD, 0, NULL) < 0)
	{
		return -1;
	}
	if((ent_eth_mid = loadModule(modulePath, IOP_MOD_ENT_ETH, 0, NULL)) < 0)
	{
		return -1;
	}
	#endif
	#if defined( MODEM )
	if((ent_ppp_mid = sceSifLoadModule(IOP_MOD_ENT_PPP, 0, NULL)) < 0)
	{
		return -1;
	}
	if((modem_mid = sceSifLoadModule(IOP_MOD_MODEMDRV, sizeof(MODEMDRV_ARG), MODEMDRV_ARG)) < 0)
	{
		return -1;
	}
	#endif

	#if defined(USE_ERX)
	/* load libeenet/eenetctl/ent_cnf erx */
	if((libeenet_erxid = sceErxLoadStartModuleFile(EE_MOD_LIBEENET, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","libeenet.erx cannot be loaded.\n");
	if((eenetctl_erxid = sceErxLoadStartModuleFile(EE_MOD_EENETCTL, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","eenetctl.erx cannot be loaded.\n");
	if((ent_cnf_erxid = sceErxLoadStartModuleFile(EE_MOD_ENT_CNF, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","ent_cnf.erx cannot be loaded.\n");
	#endif

	/* initialize eenet library */
	if((libeenet_pool = memalign(64, EENET_MEMSIZE)) == NULL)
	{
		BD_INFO("info","memalign() failed.\n");
		return -1;
	}
	if((ret = sceEENetInit(libeenet_pool, EENET_MEMSIZE, EENET_TPL, 8192, EENET_APP_PRIO)) < 0)
	{
		BD_INFO("info","sceEENetInit() failed.\n");
		return -1;
	}

	#ifdef DEBUG
	sceEENetSetLogLevel(EENET_LOG_DEBUG);
	#endif

	#ifdef UP_INTERFACE_AUTO
	if((ret = sceEENetCtlInit(8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 1, 0)) < 0)
	{
	#else
	if((ret = sceEENetCtlInit(8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 8192, EENET_APP_PRIO, 1, 1)) < 0)
	{
	#endif
		BD_INFO("info","sceEENetCtlInit() failed.\n");
		return -1;
	}
	if((ret = sceEENetCtlRegisterEventHandler(event_handler)) < 0)
	{
		BD_INFO("info","sceEENetCtlRegisterEventHandler() failed.\n");
		return -1;
	}

	
	if((ret = ent_cnf_init()) < 0)
	{
		BD_INFO("info","ent_cnf_init() failed.\n");
		return -1;
	}

	/* register device to eenet */
	#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	#if defined(USE_ERX)
	while((ent_smap_erxid = sceErxLoadStartModuleFile(EE_MOD_ENT_SMAP, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","ent_smap.erx cannot be loaded.\n");
	#endif
	if((ret = sceEENetDeviceSMAPReg(8192, 8192)) < 0)
	{
		BD_INFO("info","ret of sceEENetDeviceSMAPReg() = %d\n", ret);
		return -1;
	}
	#endif
	#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	#if defined(USE_ERX)
	while((ent_eth_erxid = sceErxLoadStartModuleFile(EE_MOD_ENT_ETH, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","ent_eth.erx cannot be loaded.\n");
	#endif
	if((ret = sceEENetDeviceETHReg(8192, 8192)) < 0){
		BD_INFO("info","ret of sceEENetDeviceETHReg() = %d\n", ret);
		return -1;
	}
	#endif
	#if defined( MODEM )
	#if defined(USE_ERX)
	while((ent_ppp_erxid = sceErxLoadStartModuleFile(EE_MOD_ENT_PPP, 0, NULL, 0, NULL, &ret)) < 0)
	BD_INFO("info","ent_ppp.erx cannot be loaded.\n");
	#endif
	if((ret = sceEENetDevicePPPReg(8192, 8192)) < 0)
	{
		BD_INFO("info","ret of sceEENetDevicePPPReg() = %d\n", ret);
		return -1;
	}
	#endif

	/* wait until target interface is attached. */
	WaitEvent(Ev_Attach);

	if((ret = ent_cnf_load_config(NET_DB, USR_CONF_NAME)) < 0)
	{
		BD_INFO("info","ent_cnf_load_config() failed.\n");
		return -1;
	}
	if((ret = ent_cnf_set_config()) < 0)
	{
		BD_INFO("info","ent_cnf_set_config() failed.\n");
		return -1;
	}

	#ifndef UP_INTERFACE_AUTO
	/* up interface manually */
	if((ret = sceEENetCtlUpInterface(EENET_IFNAME)) < 0)
	{
		BD_INFO("info","sceEENetCtlUpInterface() failed.\n");
		return -1;
	}
	#endif
	/* wait for interface initialization completed */
	WaitEvent(Ev_UpCompleted|Ev_DownCompleted);
	event_flag &= ~Ev_DownCompleted;
	if(event_flag & Ev_Error)
	{
		BD_INFO("info","Failed to initialize network.\n");
		return -1;
	}
	return 0;
}

#define NETBUFSIZE	(512)


static sceSifClientData g_cd;
static int g_rpc_buffer[NETBUFSIZE]  __attribute__((aligned(64)));

static int bind_sifrpc(sceSifClientData *bd, unsigned int request, unsigned int mode)
{
	int i;

	for (;;) {
		if (sceSifBindRpc(bd, request, mode) < 0) {
			BD_INFO("info","bind_sifrpc error\n");
			return (-1);
		}
		if (bd->serve != 0) {
			break;
		}
		i = 0x10000;
		while (--i);
	}
	return (0);
}

static int call_sifrpc(sceSifClientData *bd, unsigned int fno,
					   unsigned int mode, void *send, int ssize, void *receive, int rsize,
					   sceSifEndFunc end_func, void *end_parm)
{
	int i;

	for (;;) {
		if (sceSifCallRpc(bd, fno, mode, send, ssize, receive, rsize, end_func, end_parm) == 0) {
			break;
		}
		i = 0x10000;
		while (--i);
	}
	return (0);
}

int ent_cnf_init(void)
{
	return (bind_sifrpc(&g_cd, ENT_CNF_SIFRPC_NUM, 0));
}

int ent_cnf_load_config(const char *fname, const char *usr_name)
{
	int len;

	strcpy((char *)g_rpc_buffer, fname);
	len = strlen(fname) + 1;
	strcpy((char *)g_rpc_buffer + len, usr_name);
	len += strlen(usr_name) + 1;

	call_sifrpc(
		&g_cd,
		ENT_CNF_SIFRPC_LOAD_CONFIG,
		0,
		(void *)g_rpc_buffer,
		ee_rpc_size(len),
		(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),
		0,
		0
		);
	return (g_rpc_buffer[0]);
}

int ent_cnf_set_config(void)
{
	call_sifrpc(
		&g_cd,
		ENT_CNF_SIFRPC_SET_CONFIG,
		0,
		NULL,
		0,
		(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),
		0,
		0
		);
	return (g_rpc_buffer[0]);
}

int ent_cnf_set_config_addr(u_int addr)
{
	g_rpc_buffer[0] = (int)addr;
	call_sifrpc(
		&g_cd,
		ENT_CNF_SIFRPC_SET_CONFIG_ADDR,
		0,
		(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),
		(void *)g_rpc_buffer,
		ee_rpc_size(sizeof(u_int)),
		0,
		0
		);
	return (g_rpc_buffer[0]);
}


#define MAX_IFNUM 3

static char buf[256];
static char *iftype_str[] = {
	"Unknown",
		"Local Loopback",
		"Ethernet",
		"PPP",
		"PPPoE",
};
static struct sceEENetIfname ifnames[MAX_IFNUM];


void sceEENetShowIfinfo(const char *ifname) {
	int buflen;
	struct sceEENetIfstat ifs;

	/* 1st line - ifname, Vendor & Product */
	BD_INFO("info","%s\t", ifname);
	BD_INFO("info","Vendor:");
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_VENDOR_NAME, buf, &buflen) == 0)
		BD_INFO("info","%s", buf);
	else
		BD_INFO("info","(N/A)");
	BD_INFO("info","  Product:");
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_PRODUCT_NAME, buf, &buflen) == 0)
		BD_INFO("info","%s", buf);
	else
		BD_INFO("info","(N/A)");
	BD_INFO("info","\n");

	/* 2nd line - Link encap & HWaddr */
	BD_INFO("info","\t");
	BD_INFO("info","Link encap:");
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_IFTYPE, buf, &buflen) == 0)
		BD_INFO("info","%s  ", iftype_str[*((int *)buf)]);
	else
		BD_INFO("info","(error)  ");
	buflen = 256;
	
	/* 3rd line - Media type (if available) */
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_MEDIA, buf, &buflen) == 0) 
	{
		BD_INFO("info","\t");
		BD_INFO("info","Media:");
		if (*((u_short *)buf) & sceEENET_MEDIA_10_T)
			BD_INFO("info","10baseT ");
		else if (*((u_short *)buf) & sceEENET_MEDIA_100_TX)
			BD_INFO("info","100baseTX ");
		if (*((u_short *)buf) & sceEENET_MEDIA_HDX)
			BD_INFO("info","half-duplex ");
		else if (*((u_short *)buf) & sceEENET_MEDIA_FDX)
			BD_INFO("info","full-duplex ");
		if (*((u_short *)buf) & sceEENET_MEDIA_FLOW)
			BD_INFO("info","with flow-control ");
		if (*((u_short *)buf) & sceEENET_MEDIA_AVALID) 
		{
			if (*((u_short *)buf) & sceEENET_MEDIA_ACTIVE)
				BD_INFO("info","(active)");
			else
				BD_INFO("info","(no carrier)");
		}
		BD_INFO("info","\n");
	}

	/* 4th line - addr, Bcast & Mask (if assigned) */
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_ADDR, buf, &buflen) == 0) 
	{
		BD_INFO("info","\t");
		BD_INFO("info","inet addr:%s  ", sceEENetInetNtoa(*((struct sceEENetInAddr *)buf)));
		buflen = 256;
		if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_BROADADDR, buf, &buflen) == 0)
			BD_INFO("info","Bcast:%s  ", sceEENetInetNtoa(*((struct sceEENetInAddr *)buf)));
		buflen = 256;
		if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_NETMASK, buf, &buflen) == 0)
			BD_INFO("info","Mask:%s", sceEENetInetNtoa(*((struct sceEENetInAddr *)buf)));
		BD_INFO("info","\n");
	}

	/* 5th line - flags */
	BD_INFO("info","\t");
	buflen = 256;
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_IFFLAGS, buf, &buflen) == 0) 
	{
		if (*((u_short *)buf) & IFF_UP)
			BD_INFO("info","UP ");
		if (*((u_short *)buf) & IFF_BROADCAST)
			BD_INFO("info","BROADCAST ");
		if (*((u_short *)buf) & IFF_LOOPBACK)
			BD_INFO("info","LOOPBACK ");
		if (*((u_short *)buf) & IFF_POINTOPOINT)
			BD_INFO("info","POINTOPOINT ");
		if (*((u_short *)buf) & IFF_RUNNING)
			BD_INFO("info","RUNNING ");
		if (*((u_short *)buf) & IFF_PROMISC)
			BD_INFO("info","PROMISC ");
		if (*((u_short *)buf) & IFF_ALLMULTI)
			BD_INFO("info","ALLMULTI ");
		if (*((u_short *)buf) & IFF_MULTICAST)
			BD_INFO("info","MULTICAST ");
		BD_INFO("info"," ");
	}
	buflen = 256;
	BD_INFO("info","MTU:");
	if (sceEENetGetIfinfo(ifname, sceEENET_IFINFO_MTU, buf, &buflen) == 0)
		BD_INFO("info","%d", *((int *)buf));
	else
		BD_INFO("info","(error)");
	BD_INFO("info","\n");

	/* 6th - 8th line - statistics */
	if (sceEENetGetIfstat(ifname, &ifs) == 0) 
	{
		BD_INFO("info","\tRX packets:%d errors:%d dropped:%d\n",
			ifs.ifs_ipackets, ifs.ifs_ierrors, ifs.ifs_iqdrops);
		BD_INFO("info","\tTX packets:%d errors:%d\n",
			ifs.ifs_opackets, ifs.ifs_oerrors);
		BD_INFO("info","\tcollisions: %d noproto:%d\n",
			ifs.ifs_collisions, ifs.ifs_noproto);
	}
}

void sceEENetShowAllIfinfo(void) {
	int num = MAX_IFNUM;
	struct sceEENetIfname *ifn;

	sceEENetGetIfnames(ifnames, &num);
	for (ifn = ifnames; ifn < ifnames + num; ifn++) 
	{
		sceEENetShowIfinfo(ifn->ifn_name);
		BD_INFO("info","\n");
	}
}

bdPlatformParams::bdPlatformParams()
{
}

bdInt  BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params, bdPlatformParams *platformParams)
{
	int err = 0;

	if (platformParams && platformParams->m_modulePath.getLength() > 1)
	{
		err = initAll(platformParams->m_modulePath);
	}
	else
	{
		err = initAll(EENET_IOP_MODULES);
	}
	if (err == 0)
	{
		sceEENetShowAllIfinfo();
		char buf[256];
		int buflen = 256;
		if (sceEENetGetIfinfo(EENET_IFNAME , sceEENET_IFINFO_ADDR, buf, &buflen) == 0) 
		{
			bdUInt *inAddr = (bdUInt*)buf;
			BD_INFO("info","\t");
			BD_INFO("info","inet addr:%s  ", sceEENetInetNtoa(*((struct sceEENetInAddr *)buf)));
			params.m_localAddresses.pushBack(bdInetAddr(*inAddr));
		}
	}
	return err;
}

void BD_CALL bdExamplePlatformInit::quit()
{
	int ret = 0;
	if((ret = sceEENetCtlDownInterface(EENET_IFNAME)) < 0)
	{
		BD_INFO("info","sceEENetCtlDownInterface() failed.\n");
		return;
	}
	WaitEvent(Ev_DownCompleted);

#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	if((ret = sceEENetDeviceSMAPUnreg()) < 0) 
	{
		BD_INFO("info","sceEENetDeviceSMAPUnreg() failed.\n");
		return;
	}
#if defined(USE_ERX)
	if(0 <= sceErxStopModule(ent_smap_erxid, 0, NULL, &ret))
		sceErxUnloadModule(ent_smap_erxid);
#endif
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	if((ret = sceEENetDeviceETHUnreg()) < 0) 
	{
		BD_INFO("info","sceEENetDeviceETHUnreg() failed.\n");
		return;
	}
#if defined(USE_ERX)
	if(0 <= sceErxStopModule(ent_eth_erxid, 0, NULL, &ret))
		sceErxUnloadModule(ent_eth_erxid);
#endif
#endif
#if defined( MODEM )
	if((ret = sceEENetDevicePPPUnreg()) < 0) 
	{
		BD_INFO("info","sceEENetDevicePPPUnreg() failed.\n");
		return;
	}
#if defined(USE_ERX)
	if(0 <= sceErxStopModule(ent_ppp_erxid, 0, NULL, &ret))
		sceErxUnloadModule(ent_ppp_erxid);
#endif
#endif
	WaitEvent(Ev_DetachCompleted);
	event_flag = 0;
	if((ret = sceEENetCtlUnregisterEventHandler(event_handler)) < 0)
	{
		BD_INFO("info","sceEENetCtlUnRegisterEventHandler() failed.\n");
		return;
	}
	if((ret = sceEENetCtlTerm()) < 0) 
	{
		BD_INFO("info","sceEENetCtlTerm() failed.\n");
		return;
	}
	if((ret = sceEENetTerm()) < 0) 
	{
		BD_INFO("info","sceEENetTerm() failed.\n");
		return;
	}
	free(libeenet_pool);
#if defined(USE_ERX)
	if(0 <= sceErxStopModule(ent_cnf_erxid, 0, NULL, &ret))
		sceErxUnloadModule(ent_cnf_erxid);
	if(0 <= sceErxStopModule(eenetctl_erxid, 0, NULL, &ret))
		sceErxUnloadModule(eenetctl_erxid);
	if(0 <= sceErxStopModule(libeenet_erxid, 0, NULL, &ret))
		sceErxUnloadModule(libeenet_erxid);
#endif

#if defined( HDD_ETHERNET ) || defined( HDD_ETHERNET_WITH_PPPOE )
	sceSifStopModule(ent_smap_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_smap_mid);
#endif
#if defined( USB_ETHERNET ) || defined( USB_ETHERNET_WITH_PPPOE )
	sceSifStopModule(ent_eth_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_eth_mid);
#endif
#if defined( MODEM )
	sceSifStopModule(modem_mid, 0, NULL, &ret);
	sceSifUnloadModule(modem_mid);

	sceSifStopModule(ent_ppp_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_ppp_mid);
#endif

	sceSifStopModule(ent_cnf_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_cnf_mid);

	sceSifStopModule(eenetctl_mid, 0, NULL, &ret);
	sceSifUnloadModule(eenetctl_mid);

	sceSifStopModule(ent_devm_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_devm_mid);

	sceSifStopModule(net_cnf_mid, 0, NULL, &ret);
	sceSifUnloadModule(net_cnf_mid);

	sceSifStopModule(ent_dev9_mid, 0, NULL, &ret);
	sceSifUnloadModule(ent_dev9_mid);

#if defined(USE_ERX)
	if(0 <= sceErxStopModule(libscf_erxid, 0, NULL, &ret))
		sceErxUnloadModule(libscf_erxid);
	if(0 <= sceErxStopModule(libcdvd_erxid, 0, NULL, &ret))
		sceErxUnloadModule(libcdvd_erxid);
	free(liberx_pool);
#endif

	DeleteSema(sema_id);
}

