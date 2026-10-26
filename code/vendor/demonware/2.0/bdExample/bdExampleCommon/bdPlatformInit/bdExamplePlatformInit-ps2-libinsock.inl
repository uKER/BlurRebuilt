// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdCore.h>
#include <bdNet/bdNet.h>

#include <eekernel.h>
#include <sifrpc.h>
#include <libmc.h>
#include <netcnfif.h>
#include <libnet.h>
#include <libmrpc.h>

#include <inet/inet.h>
#include <inet/inetctl.h>
#include <netcnf.h>

#define BD_LOG_LEVEL	"bdExamplePlatformInit-ps2"

#define WORKAREA_SIZE (0x1000)
#define MAX_LEN			(256)

#define FNAME "mc0:/BWNETCNF/BWNETCNF"
#define NET_USR_NAME "Combination1"


static sceNetcnfifArg_t if_arg;

sceSifMClientData gInsockClientData;

static bool modules_loaded = false;

u_char gInsockSocketBuffer[2048] __attribute__((aligned(64)));
static int if_id[sceLIBNET_MAX_INTERFACE];

bdFastArray<int> loadedModules;

typedef struct sceNetCnf
{
	int data;				/* return value or other data (0x004) */
	int src_addr;			/* source address (0x008) */
	int dst_addr;			/* destination address (0x00c) */
	int size;				/* data size (0x010) */
	int type;				/* (0x014) */
	u_char hwaddr[6];		/* (0x01a) */
	char p0;				/* (0x01b) */
	char p1;				/* (0x01c) */
	int device_stat;		/* (0x02c) */
	int no_decode;			/* no_decode (0x020) */
	int no_decode2;			/* no_decode (0x024) */
	int no_decode3;			/* no_decode (0x024) */
	int p4;					/* (0x030) */
	char sys_name[MAX_LEN];		/* (0x130) */
	char usr_name[MAX_LEN];		/* (0x230) */
	char usr_name2[MAX_LEN];	/* (0x330) */
	char dir_name[MAX_LEN];		/* (0x430) */
	char dir_name2[MAX_LEN];	/* (0x530) */
	char dir_name3[MAX_LEN];	/* (0x630) */
} sceNetCnf_t;


bdBool loadModule(const bdNChar8 *const modulePath,
				  const bdNChar8 *const moduleName,
				  const bdInt args,
				  const bdNChar8 *const argp)
{
	bdNChar8 fullModulePath[512];
	const bdUInt maxPathLength = sizeof(fullModulePath);	
	const bdBool fromCd = bdStrncmp(modulePath, "cdrom", 5) == 0;

	if(fromCd)
	{
		// append ";1"
		bdSnprintf(fullModulePath, maxPathLength, "%s%s;1", modulePath, moduleName);
		// make everything after the "cdrom" uppercase
		bdStrupr(fullModulePath + 5);		
	}
	else
	{
		bdSnprintf(fullModulePath, maxPathLength, "%s%s", modulePath, moduleName);
	}

	// ensure the sring is null terminated (even if this makes it incorrect)
	fullModulePath[maxPathLength - 1] = '\0';	

	// try to load the module
	const bdInt result = sceSifLoadModule(fullModulePath, args, argp);
	const bdBool ok = (result >= 0);
	if (!ok)
	{
		bdPrintf("BD_ERROR: Can't load %s with args \"%s\"\n", fullModulePath, argp);
	}
	else
	{
		loadedModules.pushBack(result);
	}
	return ok;
}


bdBool loadAllModules(const bdNChar8 *const modulePath)
{
	if (modules_loaded)
	{
		return true;
	}
	bdBool ok = true;
	// setup the netcnf.irx parameters
	bdNChar8 netcnfArgs[256];
	bdMemset(netcnfArgs, BD_NULL, sizeof(netcnfArgs));

	bdUInt netcnfArgsSize = bdSnprintf(netcnfArgs, sizeof(netcnfArgs), "-no_check_capacity");
	netcnfArgsSize++;
	netcnfArgsSize += bdSnprintf(netcnfArgs + netcnfArgsSize, sizeof(netcnfArgs) - netcnfArgsSize, "icon=%sconf/neticon/english/SYS_NET.ICO", modulePath);
	netcnfArgsSize++;
	netcnfArgsSize += bdSnprintf(netcnfArgs + netcnfArgsSize, sizeof(netcnfArgs) - netcnfArgsSize, "iconsys=%sconf/neticon/english/icon.sys", modulePath);
	netcnfArgsSize++;

	netcnfArgs[sizeof(netcnfArgs)-1] = '\0';

	//setup the netcnfif.irx parameters
	bdNChar8 netcnfifArgs[256];
	bdMemset(netcnfifArgs, BD_NULL, sizeof(netcnfifArgs));

	bdUInt netcnfifArgsSize = bdSnprintf(netcnfifArgs, sizeof(netcnfifArgs), "thpri=123");
	netcnfifArgsSize++;
	netcnfifArgsSize += bdSnprintf(netcnfifArgs + netcnfifArgsSize, sizeof(netcnfifArgs) - netcnfifArgsSize, "thstack=8KB");
	netcnfifArgsSize++;

	netcnfifArgs[sizeof(netcnfifArgs)-1] = '\0';

	//setup the inetctl.irx parameters
	bdNChar8 inetctlArgs[256];
	bdMemset(inetctlArgs, BD_NULL, sizeof(inetctlArgs));
	bdUInt inetctlArgsSize = bdSnprintf(inetctlArgs, sizeof(inetctlArgs), "-no_auto");
	inetctlArgsSize++;

	inetctlArgs[sizeof(inetctlArgs)-1] = '\0';

	ok = ok && loadModule(modulePath, "sio2man.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "mcman.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "mcserv.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "netcnf.irx", netcnfArgsSize, netcnfArgs);
	ok = ok && loadModule(modulePath, "netcnfif.irx", netcnfifArgsSize, netcnfifArgs);
	ok = ok && loadModule(modulePath, "inet.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "inetctl.irx", inetctlArgsSize, inetctlArgs);
	ok = ok && loadModule(modulePath, "ppp.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "pppoe.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "msifrpc.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "libnet.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "dev9.irx", 0, BD_NULL);
	ok = ok && loadModule(modulePath, "smap.irx", 0, BD_NULL);
	modules_loaded = ok;
	return ok;
}

void unloadAllModules()
{
	int res;
	for (bdUInt i = 0; i < loadedModules.getSize(); i++)
	{
		int tmp;
		res = sceSifStopModule(loadedModules[i],0,NULL, &tmp);
		printf ("Stop modules: %i\n",tmp);
		res = sceSifUnloadModule(loadedModules[i]);
	}
	loadedModules.clear();
}

static int load_entry(char *fname, int type, char *usr_name, sceNetcnfifData_t *data)
{
	FlushCache(0);
	sceNetcnfifLoadEntry(fname, type, usr_name, data);
	while (sceNetcnfifCheck());
	sceNetcnfifGetResult(&if_arg);
	if (if_arg.data < 0) {
		scePrintf("type(%d):sceNetcnfifLoadEntry (%d)\n", type, if_arg.data);
		return (if_arg.data);
	}
	sceNetcnfifDataDump(data);
	return (0);
}

static int set_env(int type, sceNetcnfifData_t *data)
{
	int addr, id;

	sceNetcnfifGetAddr();
	while (sceNetcnfifCheck());
	sceNetcnfifGetResult(&if_arg);
	addr = if_arg.addr;

	id = sceNetcnfifSendIOP((unsigned int)data, (unsigned int)addr, (unsigned int)sizeof(sceNetcnfifData_t));
	while (sceNetcnfifDmaCheck(id));

	sceNetcnfifSetEnv(type);
	while (sceNetcnfifCheck());
	sceNetcnfifGetResult(&if_arg);
	if (if_arg.data < 0)
	{
		scePrintf("type(%d):sceNetcnfifSetEnv (%d)\n", type, if_arg.data);
		return (0);
	}
	return (if_arg.addr);
}

bdPlatformParams::bdPlatformParams()
{
}

bdInt  BD_CALL bdExamplePlatformInit::init(bdNetStartParams &params, bdPlatformParams *platformParams)
{
	bdBool success;
	static sceNetcnfifData_t data __attribute__((aligned(64)));
	static sceInetAddress_t myaddr;
	int r, addr;

	r = ChangeThreadPriority(GetThreadId(), 2);

	sceSifInitRpc(0);
	//while (!sceSifRebootIop(IOPRP));
	//while (!sceSifSyncIop());
	sceSifInitRpc(0);
	sceSifLoadFileReset();
	sceFsReset();

	if (platformParams && platformParams->m_modulePath.getLength() > 1)
	{
		BD_INFO(BD_LOG_LEVEL, "Loading modules from path %s", platformParams->m_modulePath.getBuffer());
		success = loadAllModules(platformParams->m_modulePath.getBuffer());
	}
	else
	{
		BD_INFO(BD_LOG_LEVEL, "No module path specified, using default");
		success = loadAllModules("host0:../../../../iop/modules/");
	}
	
	if (!success)
	{
		BD_ERR(BD_LOG_LEVEL, "Loading modules failed");
		return -1;
	}
	sceNetcnfifInit();
	sceMcInit();
	sceSifMInitRpc(0);

	if ((r = sceLibnetInitialize(&gInsockClientData, sizeof(gInsockSocketBuffer), sceLIBNET_STACKSIZE, sceLIBNET_PRIORITY)) < 0) 
	{

		BD_ERR(BD_LOG_LEVEL, "sceLibnetInitialize failed: (%d)\n", r);
		return -1;
	}

	if ((r = sceLibnetRegisterHandler(&gInsockClientData, gInsockSocketBuffer)) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "sceLibnetRegisterHandler failed: (%d)\n", r); 
		return -1;
	}

	sceNetcnfifAllocWorkarea(WORKAREA_SIZE);
	while (sceNetcnfifCheck());
	r = sceNetcnfifGetResult(&if_arg);
	if (if_arg.data < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceNetcnfifAllocWorkarea failed: (%d)\n", r); 
		return -1;
	}

	sceNetcnfifSetFNoDecode(sceNetcnfifArg_f_no_decode_off);

	sceNetcnfifDataInit(&data);
	if (load_entry(FNAME, sceNetcnfifArg_type_net, NET_USR_NAME, &data) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "Loading config data %s failed: (%d)\n", FNAME); 
		return -1;
	}

	if ((addr = set_env(sceNetcnfifArg_type_net, &data)) == 0) 
	{
		return -1;
	}

	if ((r = sceLibnetSetConfiguration(&gInsockClientData, gInsockSocketBuffer, addr)) < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceLibnetSetConfiguration failed (%d)\n", r);
		return -1;
	}

	if ((r = sceLibnetWaitGetInterfaceID(&gInsockClientData, gInsockSocketBuffer, if_id, sceLIBNET_MAX_INTERFACE)) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "sceLibnetGetInterfaceID failed (%d)\n", r);
		return -1;
	}

	//	ievHandlerParam.func = ievHandler;
	//    sceInetCtlRegisterEventHandler(&ievHandlerParam);

	if ((r = sceInetCtlUpInterface(&gInsockClientData, gInsockSocketBuffer, 0)) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "sceInetCtlUpInterface failed (%d)\n", r);
		return -1;
	}
	if ((r = sceLibnetWaitGetAddress(&gInsockClientData, gInsockSocketBuffer, if_id, sceLIBNET_MAX_INTERFACE, &myaddr, 0)) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "sceLibnetWaitGetAddress failed (%d)\n", r);
		return -1;
	}

	{
		//bdChar8 tmpBuf[sizeof(sceInetAddress_t)];
		sceInetAddress sceAddress;
		r = sceInetInterfaceControl(&gInsockClientData, gInsockSocketBuffer, if_id[0], sceInetCC_GetAddress, &sceAddress, sizeof(sceAddress));
		const bdNChar8 ad1 = sceAddress.data[3];
		const bdNChar8 ad2 = sceAddress.data[2];
		const bdNChar8 ad3 = sceAddress.data[1];
		const bdNChar8 ad4 = sceAddress.data[0];
		bdNChar8 addressString[BD_INET_ADDR_STRING_SIZE];
		bdSnprintf(addressString, sizeof(addressString),"%u.%u.%u.%u", ad1, ad2, ad3, ad4);
		const bdInetAddr address(addressString);
		params.m_localAddresses.pushBack(address);
	}
	return 0;
}

void BD_CALL bdExamplePlatformInit::quit()
{
	int r;

	if ((r = sceLibnetUnregisterHandler(&gInsockClientData, gInsockSocketBuffer)) < 0) 
	{
		BD_ERR(BD_LOG_LEVEL, "sceLibnetUnregisterHandler failed: (%d)\n", r); 
	}
	if ((r = sceInetCtlDownInterface(&gInsockClientData, gInsockSocketBuffer, 0)) < 0)
	{
		BD_ERR(BD_LOG_LEVEL, "sceInetCtlDownInterface failed (%d)\n", r);
	}
		
	r = sceLibnetTerminate(&gInsockClientData);
	if (r < 0)
	{
		BD_ERR("err", "sceLibnetTerminate() failed, %i\n", r);
	}
	sceSifMExitRpc();
	sceMcEnd();
	r = sceNetcnfifTerm();
	if (r < 0)
	{
		BD_ERR("err", "sceNetcnfifTerm() failed, %i\n", r);
	}
	//unloadAllModules();
}

