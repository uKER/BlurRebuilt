/*
 *
 */

using System.Runtime.InteropServices;

using Bizarre.Artery;

//////////////////////////////////////////////////////////////////////////////
/// @enum	SNRESULT
/// @brief		All functions return a 32-bit error or return code called SNRESULT., it can be tested with
///				\link ApiBase.h::SN_SUCCEEDED SN_SUCCEEDED \endlink or \link ApiBase.h::SN_FAILED SN_FAILED \endlink
/// @details	Possible values and general meanings (note: where functions have errors representing specific failures the value\n
///					and corresponding cause is listen in the function documentation)\n
/// SN_S_OK - The operation completed successfully.\n
/// SN_S_PENDING - The operation is pending completion.\n
/// SN_S_CONNECTED - The local user is already connected to the target.\n
/// SN_E_NOT_IMPL - Function is not implemented.\n
/// SN_E_TM_NOT_RUNNING - Initialisation of DLL failed because the TM could not be started.\n
/// SN_E_BAD_TARGET - Specified target does not exist\n
/// SN_E_NOT_CONNECTED - Function failed because target was not connected\n
/// SN_E_COMMS_ERR - Function failed because of a communications error with the target\n
/// \link ApiBase.h::SN_E_TM_COMMS_ERR SN_E_TM_COMMS_ERR \endlink - Function failed because of a communications error with the Target Manager.  \n
/// SN_E_TIMEOUT - Timeout occurred during the execution of the function\n
/// SN_E_HOST_NOT_FOUND - During a connect operation, the host could not be found\n
/// SN_E_TARGET_IN_USE - Connect operation failed because the specified target was in use\n
/// SN_E_LOAD_ELF_FAILED - Failed to load ELF file\n
/// SN_E_BAD_UNIT - Specified unit is not valid\n
/// SN_E_OUT_OF_MEM - Internal memory allocation failure\n
/// \link ApiBase.h::SN_E_NOT_LISTED SN_E_NOT_LISTED \endlink - Specified client is not in list for either unit update notification or TTY update notification. \n
/// SN_E_TM_VERSION - Version of Target Manager is incorrect. Contact SN Systems Support for the latest version (www.snsys.com)\n
/// SN_E_DLL_NOT_INITIALISED - The DLL has not been successfully initialised. Call SNXXXInitTargetComms()\n
/// SN_E_TARGET_RUNNING - Operation could not be completed because specified unit is running\n
/// SN_E_BAD_MEMSPACE - Specified memory space was not valid\n
/// SN_E_NO_TARGETS - Operation failed because there are no targets in the Target Manager\n
/// SN_E_NO_SEL - Pick Target operation failed because no target was selected *or* the user pressed cancel\n
/// SN_E_BAD_PARAM - An invalid parameter was passed to the function\n
/// SN_E_BUSY - An command is still pending completion. You must wait for this command to complete before issuing another\n
/// SN_E_DECI_ERROR - The Debug Agent returned an error\n
/// SN_E_EXISTING_CALLBACK - A callback function is already registered for this target\n
/// SN_E_INSUFFICIENT_DATA - Insufficient parameters for command\n
/// SN_E_DATA_TOO_LONG - Too much data passed to command (often due to restrictions on communications)\n
/// SN_E_DEPRECATED - Command no longer supported\n
/// SN_E_BAD_ALIGN - Addresses or buffer sizes incorrectly aligned\n

public enum SNRESULT
{

	//////////////////////////////////////////////////////////////////////////////
	///
	///  @brief			Macro defined for testing the success of a function
	///  @details		Returns TRUE if snr contains a success code\n
	///                 Returns FALSE if snr contains a failure code.
	///   

	//#define SN_SUCCEEDED(snr)   ((SNRESULT)(snr) >= 0)            

	//////////////////////////////////////////////////////////////////////////////
	///
	///  @brief			Macro defined for testing the success of a function
	///
	///  @details	       Returns TRUE if snr contains a failure code.
	///                    Returns FALSE if snr contains a success code.
	///

	//#define SN_FAILED(snr)      ((SNRESULT)(snr) < 0)

	////////////////////////////////////////////////////////////////////////////////
	// ERROR CODES
	////////////////////////////////////////////////////////////////////////////////

	/// SN_S_OK
	/// @brief The operation completed successfully.
	SN_S_OK = 0,

	/// SN_S_PENDING
	/// @brief The operation is pending completion.
	SN_S_PENDING = 1,

	/// SN_S_CONNECTED
	/// @brief The local user is already connected to the target.
	SN_S_CONNECTED = 2,

	/// SN_S_NO_MSG
	/// @brief No message in queue
	SN_S_NO_MSG = 3,

	/// SN_E_NOT_IMPL
	/// @brief Function is not implemented.
	SN_E_NOT_IMPL = -1,


	/// SN_E_TM_NOT_RUNNING
	/// @brief Initialisation of DLL failed because the TM could not be started.
	SN_E_TM_NOT_RUNNING = -2,

	/// SN_E_BAD_TARGET
	/// @brief Specified target does not exist
	SN_E_BAD_TARGET = -3,

	/// SN_E_NOT_CONNECTED
	/// @brief Function failed because target was not connected
	SN_E_NOT_CONNECTED = -4,

	/// SN_E_COMMS_ERR
	/// @brief Function failed because of a communications error with the target
	SN_E_COMMS_ERR = -5,

	/// SN_E_TM_COMMS_ERR
	/// @brief		Function failed because of a communications error with the Target Manager. \n
	///				When this error occurs, you should call SNXXXCloseTargetComms() and then 
	/// SNXXXInitTargetComms() to reinitialise target communications
	SN_E_TM_COMMS_ERR = -6,

	/// SN_E_TIMEOUT
	/// @brief Timeout occurred during the execution of the function
	SN_E_TIMEOUT = -7,

	/// SN_E_HOST_NOT_FOUND
	/// @brief During a connect operation, the host could not be found
	SN_E_HOST_NOT_FOUND = -8,

	/// SN_E_TARGET_IN_USE
	/// @brief Connect operation failed because the specified target was in use
	SN_E_TARGET_IN_USE = -9,

	/// SN_E_LOAD_ELF_FAILED
	/// @brief Failed to load ELF file
	SN_E_LOAD_ELF_FAILED = -10,

	/// SN_E_BAD_UNIT
	/// @brief Specified unit is not valid
	SN_E_BAD_UNIT = -11,

	/// SN_E_OUT_OF_MEM
	/// @brief Internal memory allocation failure
	SN_E_OUT_OF_MEM = -12,

	/// SN_E_NOT_LISTED
	/// @brief		Specified client is not in list for either unit update notification or TTY update notification. \n
	///				Can be received in response to CancelUPDNotfication() or CancelTTYNotification()
	SN_E_NOT_LISTED = -13,

	/// SN_E_TM_VERSION
	/// @brief  Version of Target Manager is incorrect. Contact SN Systems Support for the latest version (www.snsys.com)
	SN_E_TM_VERSION = -14,

	/// SN_E_DLL_NOT_INITIALISED
	/// @brief The DLL has not been successfully initialised. Call SNXXXInitTargetComms()
	SN_E_DLL_NOT_INITIALISED = -15,

	/// SN_E_TARGET_RUNNING
	/// @brief Operation could not be completed because specified unit is running
	SN_E_TARGET_RUNNING = -17,

	/// SN_E_BAD_MEMSPACE
	/// @brief Specified memory space was not valid
	SN_E_BAD_MEMSPACE = -18,

	/// SN_E_NO_TARGETS
	/// @brief Operation failed because there are no targets in the Target Manager
	SN_E_NO_TARGETS = -19,

	/// SN_E_NO_SEL
	/// @brief  Pick Target operation failed because no target was selected *or* the user 
	/// pressed cancel
	SN_E_NO_SEL = -20,

	/// SN_E_BAD_PARAM
	/// @brief An invalid parameter was passed to the function
	SN_E_BAD_PARAM = -21,

	/// SN_E_BUSY
	/// @brief   An command is still pending completion. You must wait for this command to 
	/// complete before issuing another
	SN_E_BUSY = -22,

	/// SN_E_DECI_ERROR
	/// @brief	The Debug Agent returned an error
	SN_E_DECI_ERROR = -23,

	/// SN_E_EXISTING_CALLBACK
	/// @brief   A callback function is already registered for this target
	SN_E_EXISTING_CALLBACK = -24,

	/// SN_E_INSUFFICIENT_DATA
	/// @brief   Insufficient parameters for command
	SN_E_INSUFFICIENT_DATA = -25,

	/// SN_E_DATA_TOO_LONG
	/// @brief   Too much data passed to command (often due to restrictions on communications)
	SN_E_DATA_TOO_LONG = -26,

	/// SN_E_DEPRECATED
	/// @brief	 Command no longer supported
	SN_E_DEPRECATED = -27,

	/// SN_E_BAD_ALIGN
	/// @brief	 Addresses or buffer sizes incorrectly aligned
	SN_E_BAD_ALIGN = -28,

	//SN_E_ERROR = 0x80000000
};

/// @enum		ResetType
/// @brief		Console reset type
public enum ResetType : ulong
{
	SNPS3TM_RESETP_SOFT_RESET	= 0x0000000000000000ul,
	SNPS3TM_RESETP_HARD_RESET	= 0x0000000000000001ul,		//perform a hard reset (default = soft)
	SNPS3TM_RESETP_QUICK_RESET	= 0x0000000000000002ul,		//perform a quick reset
	SNPS3TM_RESETP_RESET_EX		= 0x8000000000000000ul,		//perform the reset using separate off/on commands (all other flags are honored)
	SNPS3TM_RESETP_ALL_MASK		= SNPS3TM_RESETP_SOFT_RESET | SNPS3TM_RESETP_HARD_RESET | SNPS3TM_RESETP_QUICK_RESET 
													| SNPS3TM_RESETP_RESET_EX,
};

/// @enum		System Paramters
/// @brief		For specifying the target model
public enum SystemParams : ulong
{
	SNPS3TM_SYSTEMP_TARGET_MODEL		= 0x00ff000000000000ul,
	SNPS3TM_SYSTEMP_TARGET_MODEL_60GB	= 0x0001000000000000ul,
	SNPS3TM_SYSTEMP_TARGET_MODEL_20GB	= 0x0002000000000000ul,
};

///@enum		Load Parameters
///@brief		For specifying different flags when loading a process.
public enum LoadParams : uint
{
	SNPS3_LOAD_FLAG_ENABLE_DEBUGGING		= 0x00000001,
	SNPS3_LOAD_FLAG_USE_ELF_PRIORITY		= 0x00000100,
	SNPS3_LOAD_FLAG_USE_ELF_STACKSIZE		= 0x00000200,
	SNPS3_LOAD_FLAG_DO_NOT_CALL_AUTH		= 0x00001000,
	SNPS3_LOAD_FLAG_WAIT_BD_MOUNTED			= 0x00002000,
	SNPS3_LOAD_FLAG_PPU_NOT_DEBUG			= 0x00010000,
	SNPS3_LOAD_FLAG_SPU_NOT_DEBUG			= 0x00020000,
	SNPS3_LOAD_FLAG_IGNORE_DEFAULTS			= 0x80000000,
}

/// @enum		Boot Parameters
public enum BootParams : ulong
{
	SNPS3TM_BOOTP_SYSTEM_MODE			= 0x0000000000000011ul,
	SNPS3TM_BOOTP_RELEASE_MODE			= 0x0000000000000001ul,
	SNPS3TM_BOOTP_DEBUG_MODE			= 0x0000000000000010ul,

	SNPS3TM_BOOTP_BOOT_MODE_MASK		= 0x0000000000000011ul,

	SNPS3TM_BOOTP_MEM_192MB				= 0x0000000000000002ul,	//limit available memory to console setting (default = 384mb, tool)
	SNPS3TM_BOOTP_BD_EMU_ON				= 0x0000000000000004ul,	//emulate BD drive from HDD (default = use BD drive)
	SNPS3TM_BOOTP_TRANSFER_RATE_BD		= 0x0000000000000008ul,	//match transfer rate of BD drive (default = HDD native)
	SNPS3TM_BOOTP_BD_EMU_USB			= 0x0000000000000020ul,	
	SNPS3TM_BOOTP_HOSTFS_MASK			= 0x0000000000000040ul,	//gameport = set, devport = not set
	SNPS3TM_BOOTP_NIC_MASK				= 0x0000000000000080ul,	//network interface: dual = set, single = not set.

	SNPS3_BOOT_DEFAULT_MASK				= SNPS3TM_BOOTP_BOOT_MODE_MASK | SNPS3TM_BOOTP_MEM_192MB
											| SNPS3TM_BOOTP_BD_EMU_ON | SNPS3TM_BOOTP_TRANSFER_RATE_BD
											| SNPS3TM_BOOTP_BD_EMU_USB | SNPS3TM_BOOTP_HOSTFS_MASK
											| SNPS3TM_BOOTP_NIC_MASK,
};

/// @enum		ECONNECTSTATUS
/// @brief		This enumerated type defines the different states that a connection can be in.
public enum ECONNECTSTATUS
{
	/// Connected State
	CS_CONNECTED, 
	/// Attempting to connect.     
	CS_CONNECTING,
	/// Not connected, available for use.
	CS_NOT_CONNECTED,
	/// In use by another user.
	CS_IN_USE,
	/// Unavailable, could be offline.
	CS_UNAVAILABLE,
};

/// @enum		EUNITSTATUS
/// @brief		This enumerated type defines the different states that a processor unit can be in
public enum EUNITSTATUS : long
{
	/// Unknown state.
	US_UNKNOWN,             
	/// Unit is executing.						(PPU ONLY)
	US_RUNNING,             
	/// Unit has stopped execution.				(PPU ONLY)
	US_STOPPED,             
	/// A signal occurred on the unit.			(NO LONGER USED)
	US_SIGNALLED,           
	/// Unit is in the process of resetting.
	US_RESETTING,           
	/// Unit not present							(NO LONGER USED)
	US_MISSING,           
	/// Unit has completed a reset.
	US_RESET,             
	/// Unit is not connected 
	US_NOT_CONNECTED,    
	/// Unit is connected
	US_CONNECTED, 
	/// Notification of unit status change.
	US_UNIT_STATUS_CHANGE   
};

/// @enum		POWER_STATE
/// @brief		Modified from #defines in ps3tmapi, enumerates the different power states
public enum POWER_STATE : long
{
	SNPS3_POWER_STATE_OFF = 0,
	SNPS3_POWER_STATE_ON = 1,
	SNPS3_POWER_STATE_SUSPENDED = 2,
	SNPS3_POWER_STATE_UNKNOWN = 4,
	SNPS3_POWER_STATE_SWITCHING_ON = 5
};

/// @enum		Process Status
/// @brief		Defines the different states a process can be in
public enum ESNPS3PROCESSSTATUS : uint
{
	/// Process is being created
	ESNPS3_PS_CREATING = 1,
	/// Process is ready (a PU thread exists in the process)
	ESNPS3_PS_READY = 2,
	/// The process has exited but has not reported the exit status yet
	ESNPS3_PS_EXITED = 3
};

/// @enum   SNPS3_TM_TIMEOUT
/// @brief	Target manager timeout type
public enum SNPS3_TM_TIMEOUT : uint
{
	/// Default timeout, this timeout will be used for all API functions which contact the target
	DEFAULT_TIMEOUT = 0,
	/// Wait between initiating reset and attempting to connect to the target
	RESET_TIMEOUT,
	/// Connect timeout, maximum amount of time spent attempting to connect (multiple connection attempts will occur in this time)
	CONNECT_TIMEOUT,
	/// Timeout for loading process on target
	LOAD_TIMEOUT,
	/// Timeout for connecting to target to get its status
	GET_STATUS_TIMEOUT,
	/// Maximum time following reset where if target is not connected reset has failed
	RECONNECT_TIMEOUT,
	/// Timeout for functions which contact the target via the game port (generally file serving)
	GAMEPORT_TIMEOUT,
	/// Timeout when processes exited using 'game exit' option in debugger
	GAMEEXIT_TIMEOUT
};

/// @struct		TMAPI_TCPIP_CONNECT_PROP
/// @brief		TCP/IP connection properties.
[StructLayout(LayoutKind.Sequential)]
public unsafe struct TMAPI_TCPIP_CONNECT_PROP {
	/// IP Address
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 255)]
	public char[] szIPAddress;
	/// Port number
	public uint uPort;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 260;
	}
};

/// @struct		TMAPI_TARGET_TYPE
/// @brief		This structure contains the parameters that define a target type. It is used in the function 
///				\link PS3tmapi.h::SNPS3ListTargetTypes SNPS3ListTargetTypes\endlink to obtain information about supported target types.
[StructLayout(LayoutKind.Sequential)]
public unsafe struct TMAPI_TARGET_TYPE {
	/// Type
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 64)]
	public char[]	szType;
	/// Description
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 256)]
	public char[]	szDescription;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 320;
	}
};

/// @struct	SNPS3PROCESSINFO_HDR
///	@brief	Process information header
[StructLayout(LayoutKind.Sequential)]
public unsafe struct SNPS3PROCESSINFO_HDR 
{
	/// Process status (see \link PS3tmapi.h::ESNPS3PROCESSSTATUS ESNPS3PROCESSSTATUS \endlink).
	public ESNPS3PROCESSSTATUS uStatus;			
	/// Number of PPU threads.
	public uint	uNumPPUThreads;		
	/// Number of SPU threads.
	public uint	uNumSPUThreads;		
	/// Parent process ID.
	public uint	uParentProcessID;	
	/// Maximum memory size for process.
	public ulong	uMaxMemorySize;		
	/// Path to the loaded elf
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 512)]
	public char[]	szPath;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 536;
	}
};

/// @struct SNPS3PROCESSINFO
/// @brief Contains the parameters that define a process
[StructLayout(LayoutKind.Sequential)]
public unsafe struct SNPS3PROCESSINFO
{
	//struct header containing main process info body
	[MarshalAs(UnmanagedType.Struct)]
	public SNPS3PROCESSINFO_HDR Hdr;

	//public ulong[] ThreadIDs;
}

/// @struct		SNPS3PadData
///	@brief		Pad playback data
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3PadData {
	/// First 32 bits of timestamp
	public uint ulTimeHi;
	/// Second 32 bits of timestamp
	public uint ulTimeLo;
	public uint ulReserved0;
	public uint ulReserved1;
	/// Controller port
	public char  ucPort;
	/// Status of controller port
	public char  ucPortStatus;
	/// Length of button data
	public char  ucLength;
	public char  ucReserved2;
	public uint ulReserved3;
	/// Button data
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 24)]
	public ushort[] arrButtons;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 72;
	}
};

/// @struct SNPS3GamePortIPAddressData
///	@brief  This structure holds information about the game port.		 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3GamePortIPAddressData
{
	public uint uReturnValue;
	public uint uIPAddress;
	public uint uSubnetMask;
	public uint uBroadcastAddress;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 16;
	}
};

/// @struct SNPS3CondAttr
///	@brief	Attributes for conditional variable 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3CondAttr
{
	public uint	uAttrPshared;
	public ulong	uKey;
	public uint	uFlags;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 24;
	}
};

/// @struct	SNPS3ConditionalInfo
///	@brief  This structure contains the parameters that define a conditional variable.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3ConditionalInfo
{
	public uint		  uCondID;

	[MarshalAs(UnmanagedType.Struct)]
	SNPS3CondAttr Attribute;

	public uint		  uMutexID;
	public uint		  uWaitThreadsNum;
	public uint		  uWaitAllThreadsNum;
	public ulong[]		  aWaitThreadIDs;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return SNPS3CondAttr.GetSizeOf() + 528;
	}
};

/// @struct		SNPS3ControlKeywordEntry
///	@brief		Debug control keyword information	
/// @details	See "DEBUG_THREAD_CTRL_KEYWORD" in the DBGP specification 

public struct SNPS3ControlKeywordEntry
{
	public uint uMatchConditionFlags;

	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 128)]
	public char[] aKeywordString;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 132;
	}

};

/// @struct		SNPS3DebugThreadControlInfo
///	@brief		Debug thread control information	 
/// @details	See "DEBUG_THREAD_CTRL_KEYWORD" in the DBGP specification 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3DebugThreadControlInfo
{
	public ulong uControlFlags;
	public uint uNumEntries;
	public SNPS3ControlKeywordEntry[] arrCtrlKeywordEntry;
};

/// @struct	SNPS3DirEntry
///	@brief	This structure contains the parameters that define a directory entry. It is used in the
///			function SNPS3GetDirectoryList() to obtain information about a particular directory. (see SNPS3GetDirectoryList())
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3DirEntry
{
	/// The file type. See SNPS3_DIRENT_TYPE_* defines	
	public uint Type;
	/// File access attributes in standard octal format.
	public uint Mode;
	/// The last access time of the file.
	//public tm AccessTime; //TODO: uncomment and implement these
	/// The modified time of the file.
	//public tm ModifiedTime;
	/// The create time of the file.
	//public tm CreateTime;
	/// The file size
	public ulong Size;
	/// The file name
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 256)]
	char[] Name; //SNPS3_DIRENT_NAME_LEN

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 272;
	}
};

/// @struct	SNPS3EventFlagAttr
///	@brief	Event flag attributes	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EventFlagAttr
{
	public uint	uAttrProtocol;
	public uint	uAttrPshared;
	public ulong	uKey;
	public uint	uFlags;
	public uint	uType;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 32;
	}
};

/// @struct	SNPS3EventFlagWaitThread
///	@brief	This structure contains parameters defining a thread waiting on an event flag	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EventFlagWaitThread
{
	public ulong uThreadID;
	public ulong uBitPattern;
	public ulong uMode;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 24;
	}
};

/// @struct	SNPS3EventFlagInfo
///	@brief	This structure contains the parameters that define an event flag.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EventFlagInfo
{
	public uint			uEventFlagID;
	public SNPS3EventFlagAttr Attribute;
	public ulong			uBitPattern;
	public uint			uWaitThreadsNum;
	public uint			uWaitAllThreadsNum;
	public SNPS3EventFlagWaitThread[] aWaitThreads;
};

/// @struct	SNPS3EventQueueAttr
///	@brief	Event queue attributes	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EventQueueAttr
{
	public uint	uAttrProtocol;
	public uint	uType;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 16;
	}
};

/// @struct	SNPS3SysEvent
///	@brief	System event	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3SysEvent
{
	public ulong	uSource;
	public ulong	uData1;
	public ulong	uData2;
	public ulong	uData3;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 32;
	}
};

/// @struct	SNPS3EventQueueInfo
///	@brief	This structure contains the parameters that define an event queue.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EventQueueInfo
{
	public uint		uEventQueueID;
	public SNPS3EventQueueAttr Attribute;
	public ulong		uKey;
	public uint		uQueueSize;
	public uint		uWaitThreadsNum;
	public uint		uWaitAllThreadsNum;
	public uint		uReadableEqueueNum;
	public uint		uReadableAllEqueueNum;
	public ulong[]		aWaitThreadIDs;
	public SNPS3SysEvent[] aEventQueueEntries;
};

/// @struct	SNPS3LWConditionalAttr
///	@brief	Lightweight conditional attributes	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3LWConditionalAttr
{
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[]	aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 8;
	}
};

/// @struct	SNPS3LWConditionalInfo
///	@brief	Lightweight conditional information	 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3LWConditionalInfo
{
	public uint					uLWCondID;
	public SNPS3LWConditionalAttr Attribute;
	public uint					uLWMutexID;
	public uint					uWaitThreadsNum;
	public uint					uWaitAllThreadsNum;
	public ulong[]					aWaitThreadIDs;
};

///@struct	SNPS3LWMutexAttr
///	@brief  Lightweight mutex attributes
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3LWMutexAttr {
	public uint	uAttrProtocol;
	public uint	uAttrRecursive;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 16;
	}
};

/// @struct	SNPS3LWMutexInfo
///	@brief	This structure contains the parameters that define a light weight mutex.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3LWMutexInfo {
	public uint		uLwmutexID;
	public SNPS3LWMutexAttr Attribute;
	public ulong		uOwnerThID;
	public uint		uLockCounter;
	public uint		uWaitThreadsNum;
	public uint		uWaitAllThreadsNum;
	public ulong[]		aWaitThreadIDs;
};

/// @struct	SNPS3MATRange
///	@brief	Information on single range of memory access traps
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MATRange
{
	/// Start address of MAT range (aligned to 4k)
	public uint uStartAddress;
	/// Size of MAT range (aligned to 4k)
	public uint uSize;
	/// Byte array of MAT condition for each 4k page in range (see defines of SNPS3_MAT_COND_*)
	public byte[] aPageConditions;
};

///	@struct	SNPS3PRXSEGMENT
///	@brief	Information about segments within PRX module 
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3PRXSEGMENT {
	public ulong	uBase;
	public ulong	uFileSize;
	public ulong	uMemSize;
	public ulong	uIndex;
	public ulong	uElfType;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 40;
	}
};

/// @struct	SNPS3MODULEINFO_HDR
///	@brief	PRX module info
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MODULEINFO_HDR {
	/// Module name
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 30)]
	public char[] aName;
	/// Module version
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 2)]
	public char[] aVersion;
	/// Module attributes
	public uint	uAttribute;
	/// Address of start entry
	public uint	uStartEntry;
	/// Address of stop entry
	public uint	uStopEntry;
	/// Full filename of PRX module
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 512)]
	public char[] aElfName;
	/// Number of segments in module
	public uint	uNumSegments;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 560;
	}
};

/// @struct	SNPS3MODULEINFO
///	@brief	This structure contains the parameters that define a module. It is used in the function
///			SNPS3GetModuleInfo() to obtain information about a particular module of a particular
///			process.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MODULEINFO {
	public SNPS3MODULEINFO_HDR Hdr;
	public SNPS3PRXSEGMENT[] Segments;
};

///	@struct	SNPS3PRXSEGMENTEX
///	@brief	Extended information about segment within PRX module
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3PRXSEGMENTEX {
	public ulong	uBase;
	public ulong	uFileSize;
	public ulong	uMemSize;
	public ulong	uIndex;
	public ulong	uElfType;
	public ulong	uFlags;
	public ulong	uAlign;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 56;
	}
};

///	@struct	SNPS3MODULEINFOEx
///	@brief Extended module information
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MODULEINFOEX {
	public SNPS3MODULEINFO_HDR Hdr;
	public SNPS3PRXSEGMENTEX[] Segments;
};

///	@struct	SNPS3MSELFINFO
///	@brief	MSELF information
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MSELFINFO
{
	public ulong uMselfFileOffset;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 32)]
	public byte[] aReserved; //subbed byte for UCHAR
};

/// @struct	SNPS3EXTRAMODULEINFO
///	@brief	Extra module information contains GUID of PRX module loaded
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EXTRAMODULEINFO
{
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 5)]
	public uint[] uaPPU_GUID;
	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 20;
	}
};

///	@struct	SNPS3MutexAttr
///	@brief  Mutex attribute data
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MutexAttr {
	public uint	uAttrProtocol;
	public uint	uAttrRecursive;
	public uint	uAttrPshared;
	public uint	uAttrAdaptive;
	public ulong	uKey;
	public uint	uFlags;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 36;
	}
};

///	@struct	SNPS3MutexInfo
///	@brief	This structure contains the parameters that define a mutex.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3MutexInfo {
	public uint		uMutexID;
	public SNPS3MutexAttr Attribute;
	public ulong		uOwnerThID;
	public uint		uLockCounter;
	public uint		uCondRefCounter;
	public uint		uCondID;
	public uint		uWaitThreadsNum;
	public uint		uWaitAllThreadsNum;
	public ulong[]		aWaitThreadIDs;
};

///	@struct	SNPS3PPUThreadStatus
///	@brief	PPU thread status
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3PPUThreadStatus
{
	public ulong u64ThreadID;
	/// Status (see defines SNPS3_PPU_IDLE to SNPS3_PPU_DELETED)
	public uint uThreadStatus;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 12;
	}
};

///	@struct	SNPS3SPUThreadGroupStatus
///	@brief	SPU threadgroup status
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3SPUThreadGroupStatus
{
	public uint uThreadGroupID;
	/// State (see defines SNPS3_SPU_NOT_CONFIGURED to SNPS3_SPU_STOPPED) 
	public uint uThreadGroupStatus;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 8;
	}
};

/// @struct	SNPS3ProcessTreeBranch
///	@brief	Single branch of process tree (one process)
[StructLayout(LayoutKind.Sequential)]
public unsafe struct SNPS3ProcessTreeBranch
{
	public uint uProcessID;
	public uint uProcessStatus;
	public uint uPPUThreadCount;
	public uint uSPUThreadGroupCount;
	public ushort uProcessFlags;
	public ushort uRawSPU;

	public SNPS3PPUThreadStatus* aPPUThreadStatus;
	public SNPS3SPUThreadGroupStatus* aSPUThreadGroupStatus;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 36; //by my cmount anyway, what's the size of a pointer?
	}
};

///	@struct SNPS3RWLockAttr
///	@brief	Read/Write lock attributes
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3RWLockAttr {
	public uint	uAttrProtocol;
	public uint	uAttrPshared;
	public ulong	uKey;
	public uint	uFlags;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[] aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 28;
	}
};

///	@struct		SNPS3RWLockInfo
///	@brief		This structure contains the parameters that define a read-write lock. 
/// @copydoc	SNPS3RWLockInfo_InfoEx
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3RWLockInfo {
	public uint		uRwlockID;
	public SNPS3RWLockAttr Attribute;
	public ulong		uOwnerThID;
	public uint		uRwaitThreadsNum;
	public uint		uRwaitAllThreadsNum;
	public uint		uWwaitThreadsNum;
	public uint		uWaitAllThreadsNum;  // Should be uWwaitAllThreadsNum
	public ulong[]		aWaitThreadIDs;
};

///	@struct	SNPS3SemaphoreAttr
///	@brief	 Semaphore attributes
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3SemaphoreAttr
{
	public uint	uAttrProtocol;
	public uint	uAttrPshared;
	public uint	uKey;
	public uint	uFlags;
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 8)]
	public char[]	aName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 24;
	}
};

///	@struct	SNPSemaphoreInfo
///	@brief	  his structure contains the parameters that define a semaphore.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3SemaphoreInfo
{
	public uint		uSemaphoreID;
	SNPS3SemaphoreAttr Attribute;
	public uint		uMaxVal;
	public uint		uCurVal;
	public uint		uWaitThreadsNum;
	public uint		uWaitAllThreadsNum;
	public ulong[]		aWaitThreadIDs;
};

/// @struct		SNPS3SystemInfo
///	@brief		This structure contains miscellaneous information about the system. (see SNPS3GetSystemInfo())
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3SystemInfo
{
	/// SDK version used to build debug agent
	public uint			uCellSdkVersion;
	/// The timebase frequency of the target
	public ulong			uTimebaseFrequency;
	/// The runtime version of the target SDK
	public uint			uCellRuntimeSdkVersion;
	/// The total system memory of the target
	public uint			uTotalSystemMemory;
	/// The available system memory
	public uint			uAvailableSystemMemory;
	/// Max size of DBGP buffer	
	public uint			uDCMBufferSize;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 28;
	}
};

///	@struct		SNPS3TargetInfo
///	@brief		This structure contains the parameters that define a target. (see SNPS3GetTargetInfo() and SNPS3SetTargetInfo())
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3TargetInfo {
	/// Flags indicating what values returned/to be set (see defines SN_TI_TARGETID to SN_TI_BOOT)
	public uint	nFlags;
	/// Handle to target
	public int	hTarget;
	/// Target name
	public char[] pszName;
	/// Target type
	public char[] pszType;
	/// Target information
	public char[] pszInfo;
	/// Home directory of target
	public char[] pszHomeDir;
	/// File serving directory of target
	public char[] pszFSDir;
	/// Boot flags for target
	public ulong boot;
};

///	@struct	SNPS3UserMemoryStats
///	@brief	User memory statistics
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3UserMemoryStats
{
	public uint uCreatedSharedMemorySize;
	public uint uAttachedSharedMemorySize;
	public uint uProcessLocalMemorySize;
	public uint uProcessLocalTextSize;
	public uint uPRXTextSize;
	public uint uPRXDataSize;
	public uint uMiscMemorySize;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 28;
	}
};

///	@struct	SNPS3VRAMInfo
///	@brief	VRAM information
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3VRAMInfo
{
	/// Address at which to put breakpoint in order to stop the process immediately before/after 'flip'
	public ulong	uBPAddress;
	/// Address of a pointer that points to the image in memory
	public ulong	uTopAddressPointer;
	/// Width of image
	public uint		uWidth;
	/// Height of image
	public uint		uHeight;
	/// Image pitch (as described in CellGCMSurface) - in bytes
	public uint		uPitch;
	/// Image colour settings (0 = X8R8G8B8, 1 = X8B8G8R8, 2 = R16G16B16X16)
	public byte		colour;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 29;
	}
};

/// @struct TTYSTREAM
/// @brief Structure that defines information about a TTY stream returned as part of an array from a call to SNPS3ListTTYStreams()
[StructLayout(LayoutKind.Sequential)]
public struct TTYSTREAM
{
	/// The ID of the stream
	public uint nStreamIdx;
	/// A buffer to hold the textual name of the stream
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 64)]
	char[] szName;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 68;
	}
}

///	@struct	SNPS3EXTRAPROCESSINFO
///	@brief	Extra process information (GUID of elf)
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3EXTRAPROCESSINFO
{
	[MarshalAs(
		UnmanagedType.ByValArray,
		SizeConst = 5)]
	public uint[] uaPPU_GUID;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 20;
	}
};

///	@struct	SNPS3ScatteredWrite
///	@brief	Details of a write (as part of a scattered write)
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3ScatteredWrite
{
	/// Address to write to
	public uint uAddress;
	/// Buffer containing data to write (buffer size is passed to scattered write function)
	public byte[] Data;
};

///  @struct	SNPS3Protocol
///  @brief     Structure for protocol information.  This should be treated as
///             an opaque structure.
[StructLayout(LayoutKind.Sequential)]
public struct SNPS3Protocol
{
	/// Protocol number
	public uint uProtocol;
	/// Port number
	public uint uPort;
	/// LPAR Description
	public uint uLPARDesc;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 12;
	}
};

//A struct containing puRangeCount (uint[]), puBufSize(uint[]) and pBuf(byte[])
public struct MATConditions
{
	public uint puRangeCount;
	public uint puBufSize;
	public byte[] pBuf;
};

//A struct containing ulongs which contain, in order, pNumMutexes, pNumCondVars,
//pNumRWLocks, pNumLWMutexes, pNumEventQueues, pNumSemaphores (all uints, any of which may be null)
public struct SyncPrimitiveCounts
{
	public uint pNumMutexes;
	public uint pNumCondVars;
	public uint pNumRWLocks;
	public uint pNumLWMutexes;
	public uint pNumEventQueues;
	public uint pNumSemaphores;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 24;
	}
};

//boot parameter, boot parameter mask, reset parameter, reset parameter mask, system parameter, system parameter mask
public struct ResetParameters
{
	public ulong puBoot;
	public ulong puBootMask;
	public ulong puReset;
	public ulong puResetMask;
	public ulong puSystem;
	public ulong puSystemMask;

	/// Returns the size of this struct in memory
	public static int GetSizeOf()
	{
		return 48;
	}
};

/// @delegate TMAPI_HandleEventCallback
/// @brief	Event callback function template
public unsafe delegate void TMAPI_HandleEventCallback(
	/// Handle to target (or NULL for system events).
	int hTarget,
	/// Type of event (see event types below).
	uint eventType,		
	/// Event parameter (event type specific).
	uint eventParam,	
	/// Result code (error occurred if != SN_S_OK).
	SNRESULT result,	
	/// Length of data (which may include >1 event).
	uint dataLength,	
	/// Pointer to event data.
	byte* eventData,	
	/// Optional user data.
	void* userData
);

/// @delegate	TMAPI_EnumTargetsCallback
/// @brief		Target enumeration callback functions.
/// @details	The \link PS3tmapi.h::SNPS3EnumerateTargets SNPS3EnumerateTargets \endlink function takes a function pointer to a callback function
///				as an argument. During operation, \link PS3tmapi.h::SNPS3EnumerateTargets SNPS3EnumerateTargets \endlink will call this function
///				repeatedly for each target in the Target Manager, passing an hTarget handle.
///				The callback function returns 0 to continue the enumeration or non-zero to cancel it.
public unsafe delegate int TMAPI_EnumTargetsCallback(int hTarget);

/// @delegate	TMAPI_EnumTargetsExCallback
/// @brief		Target enumeration callback functions.
/// @details	The \link PS3tmapi.h::SNPS3EnumerateTargetsEx SNPS3EnumerateTargetsEx \endlink function takes a pointer to a callback function as an
///				argument. During operation, \link PS3tmapi.h::SNPS3EnumerateTargetsEx SNPS3EnumerateTargetsEx \endlink will call this function
///				repeatedly for each target in the Target Manager, passing an hTarget handle.
///				The callback function returns 0 to continue the enumeration or non-zero to cancel it.
public unsafe delegate int TMAPI_EnumTargetsExCallback(int hTarget, void* pArg);

///	 @delegate	SNPS3CustomProtocolCallback
///  @brief     Callback function for registered protocols.  There can be at
///             most one function for each registered protocol.  The pData
///             pointers to a temporary buffer of data of length uLength.
///             This callback will only be invoked when SNPS3Kick is called.

public unsafe delegate void SNPS3CustomProtocolCallback(int hTarget, byte* pData, uint uLength, void* pUser);

namespace Bizarre.Test
{
	public static unsafe class PS3Wrapper
	{
		//-------------------------------------------------------------------------------
		//---- Function result storage and access ---------------------------------------
		//-------------------------------------------------------------------------------
		private static SNRESULT mostRecentSnResult = SNRESULT.SN_S_OK;
		public static SNRESULT GetLastSnResult()
		{
			return mostRecentSnResult;
		}

		//-------------------------------------------------------------------------------
		//---- Imports for functions contained in the ps3tmapi.h header file, via -------
		//---- the unmanaged C++ DLL provided by Sony. No attributes are specified ------
		//---- on the DllImport calls at this time, though this may be necessary in -----
		//---- the future. Default calling convention is stdcall. -----------------------
		//-------------------------------------------------------------------------------

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3AddTarget(char[] pszName, char[] pszType, 
														uint uConnParamSize, byte[] pConnParams, int* pnTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3CancelFileTransfer(int hTarget, uint uTXID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3CancelFTPEvents(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3CancelTargetEvents(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3CancelTTYEvents(int hTarget, uint uStream);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ClearBreakPoint(int hTarget, uint uUnit, uint uProcessID,
														ulong uThreadID, ulong uAddress);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ClearSPULoopPoint(int hTarget, uint uProcessID, ulong uThreadID,
														uint uAddress, bool bCurrentPC);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ClearTTYCache(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3CloseTargetComms();

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Connect(int hTarget, char[] pszApplication); //pszApplication should always be NULL

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Delete(int hTarget, char[] pPath);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3DeleteEx(int hTarget, char[] pPath, uint nMillisecondTimeout);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3DeleteTarget(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Disconnect(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3DownloadDirectory(int hTarget, char[] pSource, char[] pDest, uint* pLastTXID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3DownloadFile(int hTarget, char[] pSource, char[] pDest, uint* pTXID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3EnableAutoStatusUpdate(int hTarget, uint uEnabled, uint* puPrevState);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3EnableVRAMCapture(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3EnumerateTargets(TMAPI_EnumTargetsCallback pfnCallBack);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3EnumerateTargetsEx(TMAPI_EnumTargetsExCallback pfnCallBack, void* pArg);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Exit();

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3FlashTarget(int hTarget, char[] pszUpdaterTool, char[] pszFlashImage);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3FootswitchControl(int hTarget, uint uEnabled);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ForceDisconnect(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetAPIVersion(System.Text.StringBuilder sb); //TODO: is this ok? (was char*[])

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetBreakPoints(int hTarget, uint uUnit, uint uPID,
														ulong u64TID, uint* puBPCount, ulong[] au64BPAddress);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetConditionalVariableInfo(int hTarget, uint uProcessID, uint uCondVarID,
														uint* puBufferSize, SNPS3ConditionalInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetConditionalVariableList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetConnectionInfo(int hTarget, System.IntPtr pConnection);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetConnectStatus(int hTarget, ECONNECTSTATUS* puConnectStatus, System.Text.StringBuilder ppszUsage);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetCoreDumpFlags(int hTarget, ulong* pu64Flags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetCPVersion(int hTarget, ulong* puCPVersion);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetDABR(int hTarget, uint uProcessID, ulong* puAddr);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetDebugThreadControlInfo(int hTarget, uint uProcessID,
														uint* puBufferSize, SNPS3DebugThreadControlInfo[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetDefaultLoadPriority(int hTarget, uint* pPriority);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetDefaultPPUThreadStackSize(int hTarget, uint* puSize);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetDirectoryList(int hTarget, char[] pDirectory, 
														uint* puCount, SNPS3DirEntry[] pDirectoryList);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetErrorQualifier(uint* pQualifier, System.Text.StringBuilder ppszErrorString);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetEventFlagInfo(int hTarget, uint uProcessID, uint uEventFlagID,
														uint* puBufferSize, SNPS3EventFlagInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetEventFlagList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetEventQueueInfo(int hTarget, uint uProcessID, uint uEventQueueID,
														uint* puBufferSize, SNPS3EventQueueInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetEventQueueList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetExtraLoadFlags(int hTarget, ulong* puExtraLoadFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetGamePortDebugIPAddrData(int hTarget, char[] szDeviceName,
														System.IntPtr pData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetGamePortIPAddrData(int hTarget, char[] szDeviceName,
														System.IntPtr pData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetLightWeightConditionalInfo(int hTarget, uint uProcessID, uint uLWCondVarID,
														uint* puBufferSize, SNPS3LWConditionalInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetLightWeightConditionalList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetLightWeightMutexInfo(int hTarget, uint uProcessID,
														uint uLWMutexID, uint* puBufferSize, SNPS3LWMutexInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetLightWeightMutexList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMacAddress(int hTarget, System.Text.StringBuilder ppszMacAddess);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMATConditions(int hTarget, uint uProcessID, uint* puRangeCount, byte[] pRanges,
														uint* puBufSize, byte[] pBuf);

		//[DllImport("ps3tmapi")]
		//private static extern SNRESULT SNPS3GetMATRangePointers(uint uRangeCount, byte[] pBuf, SNPS3MATRange*[] ppRanges);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMATRanges(int hTarget, uint uProcessID, uint* puRangeCount, byte[] pBuf);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMemory64Compressed(int hTarget, uint uProcessID, uint uLevel,
														ulong uAddr, uint uSize, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMemoryCompressed(int hTarget, uint uProcessID, 
														uint uLevel, uint uAddr, uint uSize, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetModuleInfo(int htarget, uint uProcessID, uint uModuleID,
														ulong* puBufferSize, SNPS3MODULEINFO[] pModuleInfo);

		//[DllImport("ps3tmapi")]
		//private static extern SNRESULT SNPS3GetModuleInfoEx(int htarget, uint uProcessID, uint uModuleID, ulong* puBufferSize,
		//                                                SNPS3MODULEINFOEX* pModuleInfo, SNPS3MSELFINFO*[] ppMSELFInfo,
		//                                                SNPS3EXTRAMODULEINFO* pExtraModuleInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetModuleList(int hTarget, uint uProcessID, uint* puCount, uint[] puBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMutexInfo(int hTarget, uint uProcessInfo,
														uint uMutexID, uint* puBufferSize, SNPS3MutexInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetMutexList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetNumTargets(uint* puNumTargets);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetPowerStatus(int hTarget, POWER_STATE* pnStatus);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetProcessTree(int hTarget, uint* puProcessCount, SNPS3ProcessTreeBranch[] pProcessTree);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetRawSPULogicalIDs(int hTarget, uint uProcessID, ulong[] pu64LogicalIDs);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetReadWriteLockInfo(int hTarget, uint uProcessID,
														uint uRWLockID, uint* puBufferSize, SNPS3RWLockInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetReadWriteLockList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetResetParameters(int hTarget, ulong* puBoot, ulong* puBootMask,
														ulong* puReset, ulong* puResetMask, ulong* puSystem, ulong* puSystemMask);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetRSXProfilingFlags(int htarget, ulong* puRSXFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSDKVersion(int hTarget, ulong* puSDKVersion);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSemaphoreInfo(int hTarget, uint uProcessID, uint uSemaphore, uint* puBufferSize,
														SNPS3SemaphoreInfo[] pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSemaphoreList(int hTarget, uint uProcessID, uint* puCount, uint[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSPUThreadGroupInfo(int hTarget, uint uProcessID,
														ulong uThreadGroupID, uint* puBufferSize, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetStatus(int hTarget, uint uUnit, EUNITSTATUS* pnUnitStatus, long* pnReasonCode);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSyncPrimitiveCounts(int hTarget, uint uProcessID, uint* pNumMutexes,
														uint* pNumCondVars, uint* pNumRWLocks, uint* pNumLWMutexes,
														uint* pNumEventQueues, uint* pNumSemaphores);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSyncPrimtivesCountEx(int hTarget, uint uProcessID, uint* puBufferSize, uint[] puCounts);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetSystemInfo(int hTarget, uint uReserved, uint* puMask, System.IntPtr pInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetTargetFromName(char[] pszTgtName, int* phTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetTargetInfo(System.IntPtr pTargetInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetTimeouts(int hTarget, uint* puNoTimeouts, SNPS3_TM_TIMEOUT[] auTimeoutIDs,
														uint[] auTimeoutValues);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetTMVersion(System.Text.StringBuilder sb);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetUserMemoryStats(int hTarget, uint uProcessID, System.IntPtr pStats);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetVirtualMemoryInfo(int hTarget, uint uPID, bool bStatsOnly, uint* puAreaCount,
														uint* puBufferSize, byte* pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetVRAMCaptureFlags(int hTarget, ulong* puVRAMFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3GetVRAMInformation(int hTarget, uint uProcessID,
														System.IntPtr pPrimary, System.IntPtr pSecondary);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3InitTargetComms();

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Kick();

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ListTargetTypes(uint* puNumTypes, TMAPI_TARGET_TYPE[] pTypes);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ListTTYStreams(int hTarget, uint* puNumTTYStreams, TTYSTREAM[] pStreams);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3MakeDirectory(int hTarget, char[] pDirectory, uint uMode);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3PickTarget(System.IntPtr hWndOwner, int* phTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3PowerOff(int hTarget, uint uForce);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3PowerOn(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3PPUThreadInfoEx(int hTarget, uint uProcessID, ulong uThreadID,
														uint* puBufferSize, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessAttach(int hTarget, uint uUnitID, uint uProcessID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessContinue(int hTarget, uint uProcessID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessGetMemory(int hTarget, uint uUnit, uint uProcessID,
														ulong uThreadID, ulong uAddress, int nCount, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessInfo(int hTarget, uint uProcessID,
															uint* puBufferSize, byte[] processInfo);
		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessInfo(int hTarget, uint uProcessID,
															uint* puBufferSize, System.IntPtr processInfo);

		//[DllImport("ps3tmapi")]
		//private static extern SNRESULT SNPS3ProcessInfoEx(int hTarget, uint uProcessID, uint* puBufferSize,
		//                                                    SNPS3PROCESSINFO* pProcessInfo, SNPS3EXTRAPROCESSINFO* pExtra);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessKill(int hTarget, uint uProcessID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessList(int hTarget, uint* puCount, uint[] puBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessLoad(int hTarget, uint uPriority, char[] pszFilename,
													int argc, char[,] argv, int envc, char[,] envv,
													uint* puProcessID, ulong* puThreadID, uint uDebugFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessOfflineFileTrace(int hTarget, char[] pszPath);

		//[DllImport("ps3tmapi")]
		//private static extern SNRESULT SNPS3ProcessScatteredSetMemory(int hTarget, uint uPID, uint uNumWrites,
		//                                            uint uWriteSize, SNPS3ScatteredWrite* pWrites, 
		//                                            uint* puErrorCode, uint* puFailedAddress);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessSetMemory(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID,
													ulong uAddress, int nCount, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ProcessStop(int hTarget, uint uProcessID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterCustomProtocol(int hTarget, uint uProtocol, uint uPort, char* pszLPAR,
													uint uPriority, SNPS3Protocol* pProto, System.IntPtr pfnCallBack,
													System.IntPtr pUser);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterFileTraceHandler(int hTarget, System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterFTPEventHandler(int hTarget, System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterPadCaptureHandler(int hTarget,
																System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterPadPlaybackNotificationHandler(int hTarget,
																System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RegisterTargetEventHandler(int hTarget,
																System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		public static extern SNRESULT SNPS3RegisterTTYEventHandler(int hTarget, uint uStream,
																System.IntPtr pfnCallBack, System.IntPtr pUserData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Rename(int hTarget, char[] pSource, char[] pDestination);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3Reset(int hTarget, ulong uReset);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ResetEx(int hTarget, ulong uBoot, ulong uBootMask, ulong uReset, 
																ulong uResetMask, ulong uSystem, ulong uSystemMask);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3RetryFileTransfer(int hTarget, uint uTXID, uint uForce);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SaveSettings();

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SendCustomProtocolData(int hTarget, System.IntPtr pProto, byte[] pData, uint uLength);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SendPadPlaybackData(int hTarget, System.IntPtr pData);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SendTTY(int hTarget, uint uStream, char[] pszText);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetBootParameter(int hTarget, ulong uBoot, ulong uBootMask);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetBreakPoint(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, ulong uAddress);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetCaseSensitiveFileServing(int hTarget, bool bOn, bool* pbOldSetting);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetConnectionInfo(int hTarget, System.IntPtr pConnection);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetCoreDumpFlags(int hTarget, ulong u64Flags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetDABR(int hTarget, uint uProcessID, ulong uAddr);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetDebugThreadControlInfo(int hTarget, uint uProcessID, System.IntPtr pInfo,
																uint* puMaxEntries);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetDefaultLoadPriority(int hTarget, uint uPriority);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetDefaultPPUThreadStackSize(int hTarget, uint uSize);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetExtraLoadFlags(int hTarget, ulong uFlags, ulong uMask);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetMATConditions(int hTarget, uint uProcessID, uint uRangeCount,
																uint uBufSize, byte[] pBuf);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetRSXProfilingFlags(int hTarget, ulong uRSXFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetSPULoopPoint(int hTarget, uint uProcessID, 
																ulong uThreadID, uint uAddress, bool bCurrentPC);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetSystemParameter(int hTarget, ulong uSystem, ulong uSystemMask);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetTargetInfo(System.IntPtr pTargetInfo);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetTimeouts(int hTarget, uint uNoTimeouts, SNPS3_TM_TIMEOUT[] pTimeoutIDs,
																uint[] puTimeoutValues);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetUseSonyDebugAgent(int hTarget, bool bSonyAgent);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SetVRAMCaptureFlags(int hTarget, ulong uVRAMFlags);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SPUThreadGroupContinue(int hTarget, uint uProcessID, ulong uThreadGroupID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3SPUThreadGroupStop(int hTarget, uint uProcessID, ulong uThreadGroupID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StartFileTrace(int hTarget, uint uPID, uint uSize, char[] pszFileName);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StartPadCapture(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StartPadPlayback(int hTarget);

		//[DllImport("ps3tmapi")]
		//private static extern SNRESULT SNPS3StatTargetFile(int hTarget, char[] pFile, SNPS3DirEntry* pEntry);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StopFileTrace(int hTarget, uint uPID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StopPadCapture(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3StopPadPlayback(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3TerminateGameProcess(int hTarget, uint uProcessID, uint uTimeout);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadContinue(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadGetRegisters(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID,
																uint uNumRegisters, uint[] puNum, byte[] pRegBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadInfo(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID,
																uint* puBufferSize, byte[] pBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadList(int hTarget, uint uProcessID, uint* puNumPPUThreads, ulong* pPPUThreadIDs,
																uint* puNumSPUThreads, ulong pSPUThreadIDs);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadSetRegisters(int htarget, uint uUnit, uint uProcessID, ulong uThreadID,
																uint uNumRegisters, uint[] puNum, byte[] pRegBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3ThreadStop(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3TranslateError(SNRESULT snr, System.Text.StringBuilder ppszErrorString);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3TriggerCoreDump(int hTarget, uint uProcessID, ulong uUserData1,
																ulong uUserData2, ulong uUserData3);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UnRegisterCustomProtocol(int hTarget, System.IntPtr pProto);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UnRegisterFileTraceHandler(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UnRegisterPadCaptureHandler(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UnRegisterPadPlaybackNotificationHandler(int hTarget);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UploadDirectory(int hTarget, char[] pSource, char[] pDestination, uint* pLastTXID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UploadFile(int hTarget, char[] pSource, char[] pDestination, uint* pTXID);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3UserProcessList(int hTarget, uint* puCount, uint[] puBuffer);

		[DllImport("ps3tmapi")]
		private static extern SNRESULT SNPS3VRAMCapture(int hTarget, uint uPID, System.IntPtr pVRAMInfo, char[] czpFileName);

		//-------------------------------------------------------------------------------
		//---- The following are C# functions that implement and wrap the above Dll -----
		//---- imports. Some functions may have minor name changes to reflect -----------
		//---- slightly differing behaviours (such as returning by value instead of -----
		//---- by pointers). All SNRESULTs  returned by the Dll functions may be --------
		//---- accessed by the GetLastSnResult() function. ------------------------------
		//-------------------------------------------------------------------------------

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Add a new target to the Target Manager.
		///
		///             Creates a new PS3 target based on the 
		///             parameters supplied. If the target already exists the 
		///             function will return SN_E_BAD_TARGET.
		///
		///  @param     pszName         Pointer to zero terminated char array containing
		///								target name (e.g. "ps3devtool").
		///  @param     pszType			Pointer to zero terminated char array containing
		///								Target Type (One of the ones returned by
		///								SNPS3ListTargetTypes)
		///  @param     uConnParamSize	Size of connect parameter in bytes (for IP
		///                             targets this is sizeof(TCPIP_CONNECT_PROP)
		///  @param		pConnParams		Pointer to connect parameters block, for
		///								IP targets this is an instance of TCPIP_CONNECT_PROP
		///
		///  @return	The target ID.
		public static int AddTarget(char[] pszName, char[] pszType, uint uConnParamSize, byte[] pConnParams)
		{
			int t = 0; //receives the target id
			int* ptr = &t;
			mostRecentSnResult = SNPS3AddTarget(pszName, pszType, uConnParamSize, pConnParams, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Cancel a current or pending transfer to/from the target.
		///				Note:	This only initiates the cancellation, to check success or failure you need to
		///						monitor \link TargetEvents target events\endlink
		///						(see SNPS3RegisterFTPEventHandler() for more information)
		///
		///  @param     hTarget        Handle to target.
		///  @param		uTXID		   Transaction ID of transfer to cancel
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void CancelFileTransfer(int hTarget, uint uTXID)
		{
			mostRecentSnResult = SNPS3CancelFileTransfer(hTarget, uTXID);
		}

		////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Unregister for file transfer notification events.
		///
		///  @param		hTarget			Handle to target
		public static void CancelFTPEvents(int hTarget)
		{
			mostRecentSnResult = SNPS3CancelFTPEvents(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Unregister the current callback function from receiving events.
		///
		///  @param     hTarget	   Handle to target
		public static void CancelTargetEvents(int hTarget)
		{
			mostRecentSnResult = SNPS3CancelTargetEvents(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Unregister the current callback function from receiving TTY.
		///
		///  @param     hTarget	   Handle to target
		///  @param     uStream	   Stream ID previously registered.
		public static void CancelTTYEvents(int hTarget, uint uStream)
		{
			mostRecentSnResult = SNPS3CancelTTYEvents(hTarget, uStream);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Clear a break point.
		///
		///  @param		hTarget			Handle to a target
		///  @param		uUnit			Unit upon which the process runs
		///  @param     uProcessID		Process ID.
		///  @param     uThreadID		Thread ID.
		///  @param		uAddress		Address of the break point.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ClearBreakPoint(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, ulong uAddress)
		{
			mostRecentSnResult = SNPS3ClearBreakPoint(hTarget, uUnit, uProcessID, uThreadID, uAddress);
		}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Clear loop point from SPU thread (replaces looping instruction with previously stored instruction)
		///				The thread group must be stopped when you set/clear loop points
		///
		/// @param		hTarget				Handle to target
		/// @param		uProcessID			Process ID
		/// @param		uThreadID			SPU thread ID
		/// @param		uAddress			Address to clear loop point from
		/// @param		bCurrentPC			If equals true then address is ignored and looppoint is cleared from current PC address
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ClearSPULoopPoint(int hTarget, uint uProcessID, ulong uThreadID, uint uAddress, bool bCurrentPC)
		{
			mostRecentSnResult = SNPS3ClearSPULoopPoint(hTarget, uProcessID, uThreadID, uAddress, bCurrentPC);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Clear the TTY cache used for the SAVETTY special fileserving command.
		///
		///  @param     hTarget			Handle to target
		public static void ClearTTYCache(int hTarget)
		{
			mostRecentSnResult = SNPS3ClearTTYCache(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Shuts down internal communications (but does not close the 
		///             Target Manager) and frees resources.
		public static void CloseTargetComms()
		{
			mostRecentSnResult = SNPS3CloseTargetComms();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Connect to specified target.
		///
		///  @param     hTarget           Handle to valid target.
		///  @param     pszApplication    Name of connecting application.
		///                               (Reserved for future use, should be NULL).
		public static void Connect(int hTarget)
		{
			mostRecentSnResult = SNPS3Connect(hTarget, null);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Delete file from target
		///
		///  @param     hTarget	   Handle to target
		///  @param     pPath      Path of file to delete
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void Delete(int hTarget, char[] pPath)
		{
			mostRecentSnResult = SNPS3Delete(hTarget, pPath);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Delete file from target with a custom timeout. This is useful when deleting large directory
		///             structures from a single base directory where the targets default timeout may not be long
		///             enough for the target to remove all the files.
		///
		///  @param     hTarget					Handle to target
		///  @param     pPath					Path of file to delete
		///  @param     nMillisecondTimeout     Millisecond timeout time
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void DeleteEx(int hTarget, char[] pPath, uint nMillisecondTimeout)
		{
			mostRecentSnResult = SNPS3DeleteEx(hTarget, pPath, nMillisecondTimeout);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Delete a specified target from the Target Manager.
		///
		///             Warning - this operation is irreversible.
		///
		///  @param     hTarget           Handle to valid target.
		public static void DeleteTarget(int hTarget)
		{
			mostRecentSnResult = SNPS3DeleteTarget(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Disconnect from specified target.
		///
		///  @param     hTarget    Handle to valid target.
		public static void Disconnect(int hTarget)
		{
			mostRecentSnResult = SNPS3Disconnect(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Queue the contents of the directory for download (does not create the containing directory)
		///
		///  @param     hTarget        Handle to target.
		///  @param     pSource        Source.
		///  @param     pDest          Destination
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///				
		///	 @return	Value of pLastTXID.
		public static uint DownloadDirectory(int hTarget, char[] pSource, char[] pDest)
		{
			uint pLastTXID = 0;
			uint* ptr = &pLastTXID;
			mostRecentSnResult = SNPS3DownloadDirectory(hTarget, pSource, pDest, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Queue the given file for download to the host.\n
		///				Note:	This only initiates the download, to check progress, success or failure you need to
		///						monitor target events (see SNPS3RegisterFTPEventHandler() for more information)
		///
		///  @param     hTarget        Handle to target.
		///  @param     pSource        Source.
		///  @param     pDest          Destination.
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///				
		///	 @return	Value of pTXID.
		public static uint DownloadFile(int hTarget, char[] pSource, char[] pDest)
		{
			uint pTXID = 0;
			uint* ptr = &pTXID;
			mostRecentSnResult = SNPS3DownloadFile(hTarget, pSource, pDest, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Enables and disables automatic client notification of unit 
		///             status changes.
		///
		///             Normally, Target Manager will notify all registered clients 
		///             whenever another client modifies memory or register contents. 
		///             It does this by sending a US_STATUS_UPDATE notification for 
		///             that unit.
		///
		///             In some circumstances this may be undesirable, e.g. if the 
		///             TMAPI application is carrying out many memory writes, and the 
		///             ProDG Debugger is connected at the same time, it will 
		///             automatically refresh itself for each update and slow down 
		///             the SDK function. For these cases, notifications can be 
		///             disabled by calling this function.
		///
		///             This feature is enabled by default. Once set it remains set 
		///             for the lifetime of the Target Manager and is reset to 
		///             enabled when restarted. You should restore the previous 
		///             setting at the end of the SDK application.
		///
		///  @param     hTarget        Handle to target.
		///  @param     uEnabled       New state for auto notifications.
		///
		///  @return	Returns the previous state.
		public static uint EnableAutoStatusUpdate(int hTarget, uint uEnabled)
		{
			uint puPrevState = 0;
			uint* ptr = &puPrevState;
			mostRecentSnResult = SNPS3EnableAutoStatusUpdate(hTarget, uEnabled, ptr);
			return *ptr;
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Enable VRAM capture for a target (sets vram capture flag to enabled and gcm debug flag on)
		///
		/// @param		hTarget				Handle to target
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		public static void EnableVRAMCapture(int hTarget)
		{
			mostRecentSnResult = SNPS3EnableVRAMCapture(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Enumerate targets via a callback function.
		///
		///             See prototype of callback function. SNPS3EnumerateTargets calls
		///             the callback function for each target in the system passing an 
		///             HTARGET handle. The callback function can then call SNPS3GetTargetInfo() 
		///             to retrieve relevant information for that target and store it.
		///
		///  @copydoc	SNPS3EnumerateTargets_ExInfo
		///
		///  @param     pfnCallBack    Pointer to callback function.
		public static void EnumerateTargets(TMAPI_EnumTargetsCallback pfnCallBack)
		{
			mostRecentSnResult = SNPS3EnumerateTargets(pfnCallBack);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Enumerate targets via a callback function.
		///
		///             See prototype of callback function. SNPS3EnumerateTargetsEx calls
		///             the callback function for each target in the system passing an 
		///             HTARGET handle. The callback function can then call SNPS3GetTargetInfo() 
		///             to retrieve relevant information for that target and store it. The
		///				callback function should return 0 to continue the enumeration or non-zero
		///				to cancel it
		///
		///  @param     pfnCallBack    Pointer to callback function.
		///  @param     pArg           Parameter passed to callback, void pointer can be cast to anything.
		public static void EnumerateTargetsEx(TMAPI_EnumTargetsExCallback pfnCallBack, void* pArg)
		{
			mostRecentSnResult = SNPS3EnumerateTargetsEx(pfnCallBack, pArg);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Close TM
		public static void Exit()
		{
			mostRecentSnResult = SNPS3Exit();
		}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Flash Target with the specified PS3ToolUpdater.self and PS3UPDAT.PUP
		///
		/// @param		hTarget				Handle to target
		/// @param		pszUpdaterTool		Path to PS3ToolUpdater.self
		/// @param		pszFlashImage		Path to PS3UPDAT.PUP file
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.6.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 1.6.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void FlashTarget(int hTarget, char[] pszUpdaterTool, char[] pszFlashImage)
		{
			mostRecentSnResult = SNPS3FlashTarget(hTarget, pszUpdaterTool, pszFlashImage);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Enable or disable footswitch notifications (disabled by default)
		///
		///  @param     hTarget			Handle to target
		///  @param     uEnabled		Set to 0 to disable notifications, non-zero to enable notifications
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void FootswitchControl(int hTarget, uint uEnabled)
		{
			mostRecentSnResult = SNPS3FootswitchControl(hTarget, uEnabled);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Forcibly disconnect another user from the specified target.
		///
		///  @param     hTarget    Handle to valid target.
		public static void ForceDisconnect(int hTarget)
		{
			mostRecentSnResult = SNPS3ForceDisconnect(hTarget);
		}


		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Returns the version of the API the code was built against.
		///
		///  @param     ppszVersion    Pointer to a const char* to receive the 
		///                            version.
		public static string GetAPIVersion() //TODO: test this [mtaylor]
		{
			System.Text.StringBuilder sb = new System.Text.StringBuilder();
			mostRecentSnResult = SNPS3GetAPIVersion(sb);
			return sb.ToString();
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		List breakpoints set in process
		///				To just get breakpoint count set au64BPAddress to NULL
		///
		/// @param		hTarget			Handle to target
		/// @param		uUnit			ppu/spu
		/// @param		uPID			Process ID
		/// @param		u64TID			Thread ID, this is only used for SPU breakpoints, if the unit is PPU then this is ignored
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Number of break points buffer needs to hold (if buffer big enough this is the number of break points written to buffer)
		public static ulong GetBreakPointsBufferSize(int hTarget, uint uUnit, uint uPID, ulong u64TID)
		{
			uint puBPCount = 0;
			uint* ptr = &puBPCount;
			mostRecentSnResult = SNPS3GetBreakPoints(hTarget, uUnit, uPID, u64TID, ptr, null);
			return *ptr;
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		List breakpoints set in process
		///				To just get breakpoint count set au64BPAddress to NULL
		///
		/// @param		hTarget			Handle to target
		/// @param		uUnit			ppu/spu
		/// @param		uPID			Process ID
		/// @param		u64TID			Thread ID, this is only used for SPU breakpoints, if the unit is PPU then this is ignored
		/// @param		puBPCount		[in] Number of break points buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer containing address of breakpoints.
		public static ulong[] GetBreakPoints(int hTarget, uint uUnit, uint uPID, ulong u64TID, uint puBPCount)
		{
			uint* puBPCount_ptr = &puBPCount;
			ulong[] au64BPAddress = new ulong[puBPCount];
			mostRecentSnResult = SNPS3GetBreakPoints(hTarget, uUnit, uPID, u64TID, puBPCount_ptr, au64BPAddress);
			return au64BPAddress;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given conditional variable. 
		///				Call with pInfo as NULL to get required size of buffer
		///
		///  @param     hTarget			  Handle to target
		///  @param     uProcessID		  Process ID
		///  @param     uCondVarID		  Conditional Variable ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required size of buffer (or if buffer is big enough, bytes written to buffer)
		public static uint GetConditionalVariableInfoBufferSize(int hTarget, uint uProcessID, uint uCondVarID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetConditionalVariableInfo(hTarget, uProcessID, uCondVarID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given conditional variable. 
		///				Call with pInfo as NULL to get required size of buffer
		///
		///  @param     hTarget			  Handle to target
		///  @param     uProcessID		  Process ID
		///  @param     uCondVarID		  Conditional Variable ID
		///  @param     puBufferSize      [in] Size of buffer provided
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer that holds the information about the conditional variable
		//public static SNPS3ConditionalInfo[] GetConditionalVariableInfo(int hTarget, uint uProcessID, uint uCondVarID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		/// 
		///   @brief	Get the list of conditional variables associated with the given process.
		///				Call with pBuffer as NULL to get just count.
		///  
		///   @param     hTarget	Handle to target
		///   @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	  @return	 Number of conditional variables in buffer
		public static uint GetConditionalVariableListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetConditionalVariableList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		/// 
		///   @brief	Get the list of conditional variables associated with the given process.
		///				Call with pBuffer as NULL to get just count.
		///  
		///   @param     hTarget	Handle to target
		///   @param     uProcessID Process ID
		///   @param     puCount    [in] Number of conditional variables buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	  @return	 Buffer that holds the set of conditional variable IDs
		public static uint[] GetConditionalVariableList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetConditionalVariableList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the connection info for a given target.
		///
		///  @param     hTarget			Handle to target
		///  @param     pConnection		Pointer to a buffer to receive the 
		///								\link TMAPI_TCPIP_CONNECT_PROP TMAPI_TCPIP_CONNECT_PROP \endlink structure.
		///
		///  @return	TMAPI_TCPIP_CONNECT_PROP struct containing the info
		public static TMAPI_TCPIP_CONNECT_PROP GetConnectionInfo(int hTarget)
		{
			System.IntPtr infoPtr = Marshal.AllocHGlobal(TMAPI_TCPIP_CONNECT_PROP.GetSizeOf());

			mostRecentSnResult = SNPS3GetConnectionInfo(hTarget, infoPtr);

			TMAPI_TCPIP_CONNECT_PROP toReturn = new TMAPI_TCPIP_CONNECT_PROP { };
			toReturn = (TMAPI_TCPIP_CONNECT_PROP)Marshal.PtrToStructure(infoPtr, typeof(TMAPI_TCPIP_CONNECT_PROP));
			Marshal.FreeHGlobal(infoPtr);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Returns the connection status of the specified target.
		///
		///             If this function succeeds it returns SN_S_OK and 
		///             *pnConnectStatus contains the connection status of the target.
		///
		///             If *pnConnectStatus is CS_IN_USE, then ppszUsage will point to 
		///             a string containing details of the current user. The contents 
		///             of string returned is temporary and must be copied to a 
		///             permanent location.
		///
		///             If *pnConnectStatus is any other value then ppszUsage is NULL; 
		///             you should determine your own strings for these states.
		///
		///  @param     hTarget            Handle to target.
		///
		///	 @return	The connect status as a ECONNECTSTATUS enum.
		public static ECONNECTSTATUS GetConnectStatus(int hTarget)
		{
			ECONNECTSTATUS status = new ECONNECTSTATUS();
			ECONNECTSTATUS* ptr = &status;
			System.Text.StringBuilder sb = new System.Text.StringBuilder();
			mostRecentSnResult = SNPS3GetConnectStatus(hTarget, ptr, sb); //TODO: test StringBuilder
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Returns the connection status of the specified target.
		///
		///             If this function succeeds it returns SN_S_OK and 
		///             *pnConnectStatus contains the connection status of the target.
		///
		///             If *pnConnectStatus is CS_IN_USE, then ppszUsage will point to 
		///             a string containing details of the current user. The contents 
		///             of string returned is temporary and must be copied to a 
		///             permanent location.
		///
		///             If *pnConnectStatus is any other value then ppszUsage is NULL; 
		///             you should determine your own strings for these states.
		///
		///  @param     hTarget            Handle to target.
		///
		///	 @return	Gets the usage string, which may be null.
		//public static string GetConnectStatusUsageString(int hTarget)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets the core dump flags of the target.
		///
		/// @param		hTarget				Handle to target
		/// @param		pu64Flags			Pointer to UINT64 where flags will be written
		///
		/// @return		ulong containing the core dump flags.
		public static ulong GetCoreDumpFlags(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetCoreDumpFlags(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the CP version. Read as 6 hexidecimal digits, where (from left to right); first 2 digits
		///				are major version number, second 2 digits are minor version number, final 2 digits are fix version number.
		///
		///	 @param		hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)		: Any SDK
		///				Reference tool (DECR-1400J / DECR-1400A)	: SDK 2.6.0 - latest version
		///
		///  @return	ulong containing the CP version number
		public static ulong GetCPVersionNumber(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetCPVersion(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Get address of current data access breakpoint
		///				Note: There will be only one data access breakpoint per process
		///
		///	 @param		hTarget			Handle to target
		///	 @param		uProcessID		Process ID
		///	 @param		puAddr			Pointer to UINT64 where data address will be written
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	ulong containing the data address
		public static ulong GetDABR(int hTarget, uint uProcessID)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetDABR(hTarget, uProcessID, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a list of all the debug thread control settings.
		///				To just get buffer size set pBuffer to NULL\n
		///				Buffer will contain instance of \link SNPS3DebugThreadControlInfo SNPS3DebugThreadControlInfo \endlink (buffer size is variable due to variable array arrCtrlKeywordEntry)
		///
		///  @param     hTarget			Handle to target.
		///  @param     uProcessID		Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required buffer size
		public static uint GetDebugThreadControlInfoBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetDebugThreadControlInfo(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a list of all the debug thread control settings.
		///				To just get buffer size set pBuffer to NULL\n
		///				Buffer will contain instance of \link SNPS3DebugThreadControlInfo SNPS3DebugThreadControlInfo \endlink (buffer size is variable due to variable array arrCtrlKeywordEntry)
		///
		///  @param     hTarget			Handle to target.
		///  @param     uProcessID		Process ID
		///  @param     puBufferSize	[in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer containing debug thread control info in the form of a SNPS3DebugThreadControlInfo struct
		//public static SNPS3DebugThreadControlInfo[] GetDebugThreadControlInfo(int hTarget, uint uProcessID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///	 @brief		Gets the default elf load priority for the target.
		///
		///  @param     hTarget			Handle to target.
		///  @param		pPriority		Pointer to UINT where value is stored
		///
		///	 @return	uint containing default load priority
		public static uint GetDefaultLoadPriority(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetDefaultLoadPriority(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the default primary PPU thread stack size for the target.
		///
		///  @param     hTarget        Handle to target.
		///
		///  @return	uint containing default PPU thread stack size
		public static uint GetDefaultPPUThreadStackSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetDefaultPPUThreadStackSize(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a directory listing.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pDirectory     Directory.
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @result	Required buffer size for the directory list
		public static uint GetDirectoryListBufferSize(int hTarget, char[] pDirectory)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetDirectoryList(hTarget, pDirectory, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a directory listing.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pDirectory     Directory.
		///  @param     puCount        [in] Size of buffer
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @result	SNPS3DirEntry struct buffer that holds the directory list
		//public static SNPS3DirEntry[] GetDirectoryList(int hTarget, char[] pDirectory, uint puCount)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Get further information on error, if available
		///
		///  @return	Last error code
		public static uint GetErrorQualifierCode()
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetErrorQualifier(ptr, null);
			return *ptr;
		}

		/////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Get further information on error, if available
		///
		///  @return	Last error address
		public static string GetErrorQualifierAddress()
		{
			System.Text.StringBuilder sb = new System.Text.StringBuilder();
			mostRecentSnResult = SNPS3GetErrorQualifier(null, sb);
			return sb.ToString();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given event flag.
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uEventFlagID		Event flag ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		///
		///  @return	uint holding the required size of the buffer
		public static uint GetEventFlagInfoBufferSize(int hTarget, uint uProcessID, uint uEventFlagID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetEventFlagInfo(hTarget, uProcessID, uEventFlagID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given event flag.
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uEventFlagID		Event flag ID
		///  @param     puBufferSize        [in] Size of buffer [out] Required size of buffer
		///  @param     pInfo				[out] Pointer to a buffer holding the information about the event flag
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		///
		///  @return	SNPS3EventFlagInfo buffer contaning information about the event flag
		//public static SNPS3EventFlagInfo[] GetEventFlagInfo(int hTarget, uint uProcessID, uint uEventFlagID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of event flags associated with the given process.
		///				Set pBuffer to NULL to just get event flag count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		///
		///  @return	Number of event flags in buffer
		public static uint GetEventFlagListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetEventFlagList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of event flags associated with the given process.
		///				Set pBuffer to NULL to just get event flag count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Number of event flags buffer can hold
		///  @param     pBuffer    Buffer to hold the set of event flag IDs
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		///
		///  @return	Buffer that holds the set of event flag IDs
		public static uint[] GetEventFlagList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetEventFlagList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given event queue.
		//				Set pBuffer to NULL to just get required buffer size
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uEventQueueID		Light-Weight Mutex ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Required size of the buffer
		public static uint GetEventQueueInfoBufferSize(int hTarget, uint uProcessID, uint uEventQueueID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetEventQueueInfo(hTarget, uProcessID, uEventQueueID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given event queue.
		//				Set pBuffer to NULL to just get required buffer size
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uEventQueueID		Light-Weight Mutex ID
		///  @param     puBufferSize        [in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	 SNPS3EventQueueInfo struct buffer holding the information about the event queue
		//public static SNPS3EventQueueInfo[] GetEventQueueInfo(int hTarget, uint uProcessID, uint uEventQueueID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of event queues associated with the process.
		///				Set pBuffer to NULL to just get event queue count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Number of event queues
		public static uint GetEventQueueListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetEventQueueList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of event queues associated with the process.
		///				Set pBuffer to NULL to just get event queue count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer holding the set of event queue IDs
		public static uint[] GetEventQueueList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetEventQueueList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the extra load flags passed during an ELF load.
		///
		///  @param		hTarget				Handle to target
		///
		///  @return	ulong containing extra load flags
		public static ulong GetExtraLoadFlags(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetExtraLoadFlags(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		If the target is in single nic mode this returns the same value as SNPS3GetGamePortIPAddrData(). 
		///				If the target is in dual nic mode this returns the IP address used for host file serving if file serving through the game port is 
		///				selected (this IP is set via the XMB, through the Connection Settings for Debug option in debug settings)
		///
		///  @param     hTarget			Handle to target.
		///	 @param		szDeviceName	Name of device target is connected on (if NULL default of "eth0" will be used)
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	SNPS3GamePortIPAddressData struct containing the info
		public static SNPS3GamePortIPAddressData GetGamePortDebugIPAddrData(int hTarget, char[] szDeviceName)
		{
			System.IntPtr infoPtr = Marshal.AllocHGlobal(SNPS3GamePortIPAddressData.GetSizeOf());
			mostRecentSnResult = SNPS3GetGamePortDebugIPAddrData(hTarget, szDeviceName, infoPtr);

			SNPS3GamePortIPAddressData toReturn = new SNPS3GamePortIPAddressData { };
			toReturn = (SNPS3GamePortIPAddressData)Marshal.PtrToStructure(infoPtr, typeof(SNPS3GamePortIPAddressData));
			Marshal.FreeHGlobal(infoPtr);

			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Fetches the IP address data used by the game on the target.
		///
		///  @param     hTarget			Handle to target.
		///	 @param		szDeviceName	Name of device target is connected on (if NULL default of "eth0" will be used)
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	SNPS3GamePortIPAddressData struct containing the info
		public static SNPS3GamePortIPAddressData GetGamePortIPAddrData(int hTarget, char[] szDeviceName)
		{
			System.IntPtr infoPtr = Marshal.AllocHGlobal(SNPS3GamePortIPAddressData.GetSizeOf());
			mostRecentSnResult = SNPS3GetGamePortIPAddrData(hTarget, szDeviceName, infoPtr);

			SNPS3GamePortIPAddressData toReturn = new SNPS3GamePortIPAddressData { };
			toReturn = (SNPS3GamePortIPAddressData)Marshal.PtrToStructure(infoPtr, typeof(SNPS3GamePortIPAddressData));
			Marshal.FreeHGlobal(infoPtr);

			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given light weight conditional variable. 
		///				Call with pInfo as NULL to get required size of buffer	
		///
		///  @param     hTarget       Handle to target
		///  @param     uProcessID    Process ID
		///  @param     uLWCondVarID  ID of the light weight conditional variable
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required buffer size.
		public static uint GetLightWeightConditionalInfoBufferSize(int hTarget, uint uProcessID, uint uLWCondVarID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetLightWeightConditionalInfo(hTarget, uProcessID, uLWCondVarID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given light weight conditional variable. 
		///				Call with pInfo as NULL to get required size of buffer	
		///
		///  @param     hTarget       Handle to target
		///  @param     uProcessID    Process ID
		///  @param     uLWCondVarID  ID of the light weight conditional variable
		///  @param     puBufferSize  [in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	SNPS3LWConditionalInfo struct holding information about the lightweight conditionalnvariable
		//public static SNPS3LWConditionalInfo[] GetLightWeightConditionalInfo(int hTarget, uint uProcessID, uint uLWCondVarID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get list of light weight conditional variables.
		///				Call with pBuffer as NULL to get required size of buffer	
		///
		///  @param     hTarget    Handle to target
		///  @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Number of LW conditional variables in buffer
		public static uint GetLightWeightConditionalListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetLightWeightConditionalList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get list of light weight conditional variables.
		///				Call with pBuffer as NULL to get required size of buffer	
		///
		///  @param     hTarget    Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Number of LW Conditional buffer can hold
		///						   [out] Number of LW Conditional variables in buffer
		///  @param     pBuffer    Buffer to hold the set of light-weight conditional
		///                        variable IDs
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	uint array holding a set of lightweight conditional variable IDs
		public static uint[] GetLightWeightConditionalList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetLightWeightConditionalList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given light-weight mutex
		///				Set pInfo to NULL to just find the required size of the buffer
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uLWMutexID   Light-Weight Mutex ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Required size of the buffer
		public static uint GetLightWeightMutexInfoBufferSize(int hTarget, uint uProcessID, uint uLWMutexID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetLightWeightMutexInfo(hTarget, uProcessID, uLWMutexID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given light-weight mutex
		///				Set pInfo to NULL to just find the required size of the buffer
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uLWMutexID   Light-Weight Mutex ID
		///  @param     puBufferSize [in] Size of the buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	SNPS3LWMutexInfo struct containing the information about the lightweight mutex
		//public static SNPS3LWMutexInfo[] GetLightWeightMutexInfo(int hTarget, uint uProcessID, uint uLWMutexID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of light-weight mutexes associated with the process.
		///				Set pBuffer to NULL to just get the count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Required size of buffer
		public static uint GetLightWeightMutexListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetLightWeightMutexList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of light-weight mutexes associated with the process.
		///				Set pBuffer to NULL to just get the count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Size of buffer
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint array holding the set of lightweight mutex IDs
		public static uint[] GetLightWeightMutexList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetLightWeightMutexList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		///////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Get MAC address of gameport of target
		///
		/// @param		hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.1.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.4.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	@return		The MAC address
		public static string GetMACAddress(int hTarget)
		{
			System.Text.StringBuilder sb = new System.Text.StringBuilder();
			mostRecentSnResult = SNPS3GetMacAddress(hTarget, sb);
			return sb.ToString();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Get current settings of memory access traps for specified address ranges for a process \n
		///				Input buffer is array of \link SNPS3MATRange SNPS3MATRange \endlink structures (with empty aPageConditions arrays)
		///				specifying what memory ranges to get conditions for\n
		///				Output buffer is also array of \link SNPS3MATRange SNPS3MATRange \endlink structures where aPageConditions arrays will
		///				now be filled with requested information\n
		///				See SNPS3_MAT_COND_* defines for possible conditions\n
		///
		///  @param     hTarget			Handle to target
		///  @param     uProcessID		Process ID
		///	 @param		puRangeCount	[in] Number of ranges requested in pRanges buffer
		///								[out]Number of ranges successfully read
		///  @param		pRanges			Pointer to buffer containing ranges to get conditions for
		///  @param		puBufSize		[in] Size of output buffer pBuf in bytes
		///								[out]Required size of output buffer pBuf
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	A struct defined within this class containing puRangeCount (uint), puBufSize(uint) and pBuf(byte[])
		public static MATConditions GetMATConditions(int hTarget, uint uProcessID, uint puRangeCount, byte[] pRanges, uint puBufSize)
		{
			uint* puRangeCount_ptr = &puRangeCount;
			uint* puBufSize_ptr = &puBufSize;
			byte[] pBuf = new byte[puBufSize];
			MATConditions toReturn = new MATConditions();
			mostRecentSnResult = SNPS3GetMATConditions(hTarget, uProcessID, puRangeCount_ptr, pRanges, puBufSize_ptr, pBuf);
			toReturn.puRangeCount = *puRangeCount_ptr;
			toReturn.puBufSize = *puBufSize_ptr;
			toReturn.pBuf = pBuf;
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Gets the range count for pBuf in the GetMATRanges function
		///
		///  @param     hTarget			Handle to target
		///  @param     uProcessID		Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Number of ranges written to buffer/number of ranges buffer needs to hold (if NULL)
		public static uint GetMATRangesBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetMATRanges(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Get list of address ranges for a process where memory access traps can be set. 
		///				Buffer is array of \link SNPS3MATRange SNPS3MATRange \endlink structures where aPageConditions will always be empty\n
		///
		///  @param     hTarget			Handle to target
		///  @param     uProcessID		Process ID
		///  @param		puRangeCount 	[in] Total number of ranges buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer containing the range information
		public static byte[] GetMATRanges(int hTarget, uint uProcessID, uint puRangeCount)
		{
			uint* puRangeCount_ptr = &puRangeCount;
			byte[] toReturn = new byte[puRangeCount * 8]; //TODO: needs testing, see this function in the SDK
			mostRecentSnResult = SNPS3GetMATRanges(hTarget, uProcessID, puRangeCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Gets a range of memory, compressing and decompressing at start/end of communications (improved speed)
		///				Address can be a maximum of 64 bits long.
		///
		///	 @param		hTarget			Handle to target
		///	 @param		uProcessID		Process ID
		///  @param		uLevel			Compression level - see SNPS3_COMPRESSION_LEVEL_* - can be value between 1 and 9, 1 being best speed
		///								9 being best compression, default value (-1) will request compromise between compression and speed
		///	 @param		uAddr			Address of memory to read from
		///  @param		uSize			Size of data to read
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.9.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer containing data
		public static byte[] GetMemory64Compressed(int hTarget, uint uProcessID, uint uLevel, ulong uAddr, uint uSize)
		{
			byte[] toReturn = new byte[uSize];
			mostRecentSnResult = SNPS3GetMemory64Compressed(hTarget, uProcessID, uLevel, uAddr, uSize, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Gets a range of memory, compressing and decompressing at start/end of communications (improved speed)
		///				Address can be a maximum of 32 bits, for more use SNPS3GetMemory64Compressed
		///
		///	 @param		hTarget			Handle to target
		///	 @param		uProcessID		Process ID
		///  @param		uLevel			Compression level - see SNPS3_COMPRESSION_LEVEL_* - can be value between 1 and 9, 1 being best speed
		///								9 being best compression, default value (-1) will request compromise between compression and speed
		///	 @param		uAddr			Address of memory to read from
		///  @param		uSize			Size of data to read
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.9.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer containing the data
		public static byte[] GetMemoryCompressed(int hTarget, uint uProcessID, uint uLevel, uint uAddr, uint uSize)
		{
			byte[] toReturn = new byte[uSize];
			mostRecentSnResult = SNPS3GetMemoryCompressed(hTarget, uProcessID, uLevel, uAddr, uSize, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Gets required puBufferSize
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		ID of the process.
		///  @param     uModuleID		ID of the module.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required size of buffer		
		public static ulong GetModuleInfoBufferSize(int hTarget, uint uProcessID, uint uModuleID)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetModuleInfo(hTarget, uProcessID, uModuleID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Fetches module information for a given module of a given process.
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		ID of the process.
		///  @param     uModuleID		ID of the module.
		///  @param		puBufferSize	[in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer holding the module info as a SNPS3MODULEINFO struct array
		//public static SNPS3MODULEINFO[] GetModuleInfo(int hTarget, uint uProcessID, uint uModuleID, ulong puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the number of modules loaded by the process.
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		ID of the process to which the module belongs.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Module count
		public static uint GetModuleListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetModuleList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Fetches a list of modules loaded by a given process.
		///				Set puBuffer to NULL to just set *puCount to be the number of modules loaded by the process.
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		ID of the process to which the module belongs.
		///  @param     puCount			[in] Number of module IDs buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer holding the information
		public static uint[] GetModuleList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetModuleList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required buffer size for GetMutexInfo
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uMutexID     Mutex ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Required buffer size
		public static uint GetMutexInfoBufferSize(int hTarget, uint uProcessID, uint uMutexID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetMutexInfo(hTarget, uProcessID, uMutexID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given mutex.
		///				Set pBuffer to NULL to just get required buffer size
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uMutexID     Mutex ID
		///  @param     puBufferSize [in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer containing information about the mutex
		//public static SNPS3MutexInfo[] GetMutexInfo(int hTarget, uint uProcessID, uint uMutexID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the number of mutexes associated with the process
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Number of mutexes in buffer
		public static uint GetMutexListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetMutexList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of mutexes associated with the process.
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Number of mutexes buffer can hold
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Buffer holding the set of mutex IDs
		public static uint[] GetMutexList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetMutexList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Returns the number of targets that are currently set up in the Target Manager. This
		///				function can be used to determine the number of \link SNPS3TargetInfo SNPS3TargetInfo \endlink structures
		///				required for a SNPS3EnumerateTargets call.
		///
		///  @return	The number of targets
		public static uint GetNumTargets()
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetNumTargets(ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Retrieves the current power status of the specified target.\n
		///				SNPS3_POWER_STATE_OFF, SNPS3_POWER_STATE_ON, SNPS3_POWER_STATE_SUSPENDED,
		///				SNPS3_POWER_STATE_UNKNOWN, SNPS3_POWER_STATE_SWITCHING_ON
		///
		///  @param     hTarget        Handle to target.
		///
		///  @return	Enumaerated long holding power status (POWER_STATE)
		public static POWER_STATE GetPowerStatus(int hTarget)
		{
			POWER_STATE l = 0;
			POWER_STATE* ptr = &l;
			mostRecentSnResult = SNPS3GetPowerStatus(hTarget, ptr);
			return l; //TODO: test this [mtaylor]
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets the number of instances the SNPS3ProcessTreeBranch buffer needs to hold
		///
		/// @param		hTarget			Handle to target
		///
		/// @version    Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		/// @return		Number of instances buffer needs to hold
		public static uint GetProcessTreeBufferSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetProcessTree(hTarget, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Retrieves the process tree (information on processes, their 
		///				PPU and SPU threads)
		///				Only instances of \link SNPS3ProcessTreeBranch SNPS3ProcessTreeBranch \endlink struct are stored in buffer
		///				pProcessTree points to. Data that aPPUThreadStatus and aSPUThreadGroupStatus
		///				is stored in temporary memory and will be destroyed on next TMAPI function call
		///
		/// @param		hTarget			Handle to target
		/// @param		puProcessCount	[in] Number of instances of \link SNPS3ProcessTreeBranch SNPS3ProcessTreeBranch \endlink buffer can hold
		///
		/// @version    Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		/// @return		Buffer holding process tree information as an array of SNPS3ProcessTreeBranch structs
		//public static SNPS3ProcessTreeBranch[] GetProcessTree(int hTarget, uint puProcessCount)
		//{
		//}

		////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Get RAW SPU Logical IDs\n
		///				Note: Will return a value for all Physical SPUs event those not allocated to RAW SPUs
		///
		///  @param     hTarget			Handle to target
		///  @param     uProcessID		Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Array of 8 64 bit uints holding the IDs
		public static ulong[] GetRawSPULogicalIDs(int hTarget, uint uProcessID)
		{
			ulong[] toReturn = new ulong[8];
			mostRecentSnResult = SNPS3GetRawSPULogicalIDs(hTarget, uProcessID, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required buffer size
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uRWLockID    Read-Write Lock ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required buffer size
		public static uint GetReadWriteLockInfoBufferSize(int hTarget, uint uProcessID, uint uRWLockID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetReadWriteLockInfo(hTarget, uProcessID, uRWLockID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given read-write lock.
		///				Set pInfo to NULL to just get required buffer size
		///
		///  @param     hTarget	     Handle to target
		///  @param     uProcessID   Process ID
		///  @param     uRWLockID    Read-Write Lock ID
		///  @param     puBufferSize [in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer of SNPS3RWLockInfo structs
		//public static SNPS3RWLockInfo[] GetReadWriteLockInfo(int hTarget, uint uProcessID, uint uRWLockID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets lock count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Number of read-write locks in buffer
		public static uint GetReadWriteLockListBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetReadWriteLockList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of read-write lock list associated with the given process.
		///				Set pBuffer to NULL to just get lock count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Number of IDs buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Array of uints containing the set of read-write lock IDs
		public static uint[] GetReadWriteLockList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetReadWriteLockList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Query the current reset parameter settings.
		///
		///				Boot and System parameters are the parameters which will be used
		///				on the next reset (having already been masked). The boot and system
		///				mask parameters are the masks which were used on the last call to 
		///				SNPS3ResetEx(). The reset parameter and masks are those used in the last
		///				reset
		///
		///  @param     hTarget      Handle to a valid target.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	A struct defined in this class containing all reset parameters
		public static ResetParameters GetResetParameters(int hTarget)
		{
			ulong i, j, k, l, m, n;
			i = j = k = l = m = n = 0;
			ulong* bootPtr = &i; ulong* bootMaskPtr = &j; ulong* resetPtr = &k; 
			ulong* resetMaskPtr = &l; ulong* systemPtr = &m; ulong* systemMaskPtr = &n;
			ResetParameters toReturn = new ResetParameters();
			mostRecentSnResult = SNPS3GetResetParameters(hTarget, bootPtr, bootMaskPtr, resetPtr, resetMaskPtr, systemPtr, systemMaskPtr);
			toReturn.puBoot = *bootPtr;
			toReturn.puBootMask = *bootMaskPtr;
			toReturn.puReset = *resetPtr;
			toReturn.puResetMask = *resetMaskPtr;
			toReturn.puSystem = *systemPtr;
			toReturn.puSystemMask = *systemMaskPtr;
			return toReturn;
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets RSX profiling flags\n
		///				For RSX profiling flags see SNPS3_HUD_RSX_USE_*
		///
		/// @param		hTarget				Handle to target
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.4.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	ulong holding RSX profiling flags
		public static ulong GetRSXProfilingFlags(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetRSXProfilingFlags(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the SDK version of the target from Target Manager. In 6 byte hexidecimal form, first 2 bytes
		///				are major version number, second 2 bytes are minor version number,
		///				final 2 bytes are fix version number.
		///  @copydoc	SNPS3GetSDKVersion_ExInfo()
		///
		///	 @param		hTarget			Handle to target
		///
		///  @return	ulong containing the sdk version number
		public static ulong GetSDKVersion(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetSDKVersion(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get required buffer size
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uSemaphoreID		Semaphore ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint containing required size of buffer
		public static uint GetSemaphoreInfoBufferSize(int hTarget, uint uProcessID, uint uSemaphoreID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetSemaphoreInfo(hTarget, uProcessID, uSemaphoreID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about the given semaphore.
		///
		///  @param     hTarget				Handle to target
		///  @param     uProcessID			Process ID
		///  @param     uSemaphoreID		Semaphore ID
		///  @param     puBufferSize        [in] Size of buffer [out] Required size of buffer
		///  @param     pInfo				[out] Pointer to a buffer holding the information about the semaphore
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	SNPS3emaphoreInfo struct array holding information about the semaphore
		//public static SNPS3SemaphoreInfo[] GetSemaphoreInfo(int hTarget, uint uProcessID, uint uSemaphoreID, uint puBufferSize)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the semaphore count
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Number of semaphores in the buffer
		public static uint GetSemaphoreCountBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetSemaphoreList(hTarget, uProcessID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get the list of semaphores associated with the given process.
		///
		///  @param     hTarget	   Handle to target
		///  @param     uProcessID Process ID
		///  @param     puCount    [in] Number of semaphores buffer can hold
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint array of semaphore IDs
		public static uint[] GetSemaphoreList(int hTarget, uint uProcessID, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3GetSemaphoreList(hTarget, uProcessID, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required size of puBufferSize
		///
		///  @param     hTarget				Handle to a valid target.
		///	 @param		uProcessID			Process ID.
		///	 @param		uThreadGroupID		64-bit thread group ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint containing required buffer size
		public static uint GetSPUThreadGroupInfoBufferSize(int hTarget, uint uProcessID, ulong uThreadGroupID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetSPUThreadGroupInfo(hTarget, uProcessID, uThreadGroupID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get information about an SPU thread group.
		///				Buffer contains instance of \link SNPS3_SPU_THREADGROUP_INFO SNPS3_SPU_THREADGROUP_INFO \endlink followed by an array
		///				of 32 bit SPU thread IDs in the group and finally the threadgroup name
		///
		///  @param     hTarget				Handle to a valid target.
		///	 @param		uProcessID			Process ID.
		///	 @param		uThreadGroupID		64-bit thread group ID.
		///  @param		puBufferSize		[in] Size of pBuffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	byte array containing the information
		public static byte[] GetSPUThreadGroupInfo(int hTarget, uint uProcessID, ulong uThreadGroupID, uint puBufferSize)
		{
			uint* puBufferSize_ptr = &puBufferSize;
			byte[] toReturn = new byte[puBufferSize];
			mostRecentSnResult = SNPS3GetSPUThreadGroupInfo(hTarget, uProcessID, uThreadGroupID, puBufferSize_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Retrieves current status of the specified processor unit on 
		///             a given target. Check definition of EUNITSTATUS in \link ApiBase.h ApiBase.h \endlink for
		///			    details on which statuses will be used for which units 
		///
		///  @param     hTarget         Handle to target.
		///  @param     uUnit           Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param     pnUnitStatus    Pointer to variable for status.
		///                             (see EUNITSTATUS in \link ApiBase.h ApiBase.h \endlink).
		///  @param     pnReasonCode    Not used for PS3, should be NULL.
		///
		///  @return	A long variable for the status, enumerated as a EUNITSTATUS enum
		public static EUNITSTATUS GetStatus(int hTarget, uint Uunit)
		{
			EUNITSTATUS l = EUNITSTATUS.US_UNKNOWN;
			EUNITSTATUS* ptr = &l;
			mostRecentSnResult = SNPS3GetStatus(hTarget, Uunit, ptr, null);
			return l;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a count of all the synchronization primitives owned by the given process.
		///
		///  @param     hTarget			Handle to target.
		///  @param     uProcessID		Process ID
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.8.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	A struct defined in this class containing the specified primitives.
		public static SyncPrimitiveCounts GetSyncPrimitiveCounts(int hTarget, uint uProcessID)
		{
			uint i, j, k, l, m, n;
			i = j = k = l = m = n = 0;
			uint* mutPtr = &i; uint* condPtr = &j; uint* locksPtr = &k;
			uint* LWMutexesPtr = &l; uint* eventPtr = &m; uint* semaphoresPtr = &n;
			SyncPrimitiveCounts toReturn = new SyncPrimitiveCounts();
			mostRecentSnResult = SNPS3GetSyncPrimitiveCounts(hTarget, uProcessID, mutPtr,
													condPtr, locksPtr, LWMutexesPtr, eventPtr, semaphoresPtr);
			toReturn.pNumMutexes = *mutPtr;
			toReturn.pNumCondVars = *condPtr;
			toReturn.pNumRWLocks = *locksPtr;
			toReturn.pNumLWMutexes = *LWMutexesPtr;
			toReturn.pNumEventQueues = *eventPtr;
			toReturn.pNumSemaphores = *semaphoresPtr;
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get a summary of the number of synchronization primitives in use.
		///				Set puCounts to get total number of sync primitive types written to puBufferSize
		///
		///  @param     hTarget        Handle to target.
		///  @param     uProcessID     Process ID
		///  @param     puBufferSize   [in] Maximum number of sync primitive counts to write to buffer
		///                            [out] Number of counts written to buffer (unless puCounts is NULL, where it will be maximum possible number of counts)
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.8.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	An array of UINTs to store the number of each kind of primitive.\n
		///							   Values are:\n
		///							   puCounts[0] = Mutex count\n
		///							   puCounts[1] = Conditional variable count\n
		///							   puCounts[2] = Read/Write lock count\n
		///							   puCounts[3] = Lightweight mutex count\n
		///							   puCounts[4] = Event queue count\n
		///							   puCounts[5] = Semaphore count\n
		///							   puCounts[6] = Lightweight conditional variable count\n
		///							   puCounts[7] = Event flag count (sdk 2.7.0 and above)\n
		///							   You can use \link PS3tmapi.h::SyncPrimitiveType SyncPrimitiveType \endlink as an index, be aware that values always 
		///							   come in the same order, e.g if puBufferSize = 2 puCounts will always contain mutex and conditional variable counts
		public static uint[] GetSyncPrimitiveCountsEx(int hTarget, uint uProcessID, uint puBufferSize)
		{
			uint* puBufferSize_ptr = &puBufferSize;
			uint[] toReturn = new uint[8]; //gone with the maximum number of counts, could resize based on returned puBufferSize?
			mostRecentSnResult = SNPS3GetSyncPrimtivesCountEx(hTarget, uProcessID, puBufferSize_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Populates the \link SNPS3SystemInfo SNPS3SystemInfo \endlink. Which fields have been initialized is indicated by
		///				the mask, which must be obtained by calling GetSystemInfoMask
		///
		///  @param     hTarget	   Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	SNPS3SystemInfo struct containing the data
		//public static SNPS3SystemInfo GetSystemInfo(int hTarget)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Indicates which fields have been initialised in the result returned by a recent call to GetSystemInfo
		///
		///  @param     hTarget	   Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint mask
		//public static uint GetSystemInfoMask(int hTarget)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Takes a string containing the name of a target (e.g. 
		///             "ps3devtool") and fills the supplied handle with the 
		///             corresponding target ID.
		///
		///             This handle can then be used in subsequent operations.
		///
		///  @param     pszTgtName    String containing the name of the target.
		///
		///  @return    int holding the target from the name
		///
		public static int GetTargetFromName(char[] pszTgtName)
		{
			int i = 0;
			int* ptr = &i;
			mostRecentSnResult = SNPS3GetTargetFromName(pszTgtName, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Retrieves all information for a target specified by hTarget 
		///             member of \link SNPS3TargetInfo SNPS3TargetInfo \endlink structure.
		///
		///             SN_TI_TARGETID flag in nFlags member *must* be set. All 
		///             members are filled in when structure returns and setting of 
		///             other flags has no effect. Contents of structure returned are 
		///             temporary and must be copied to a permanent location.
		///
		///  @param     pTargetInfo    SNPS3TargetInfo struct to pass in
		///
		///  @return	SNPS3TargetInfo struct out
		//public static SNPS3TargetInfo GetTargetInfo(SNPS3TargetInfo pTargetInfo)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets the number of timeouts
		///
		/// @param		hTarget				Handle to target
		///
		/// @return		uint containing the number of timeouts
		public static uint GetTimeoutsBufferSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3GetTimeouts(hTarget, ptr, null, null);
			return *ptr;
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets the TM timeouts. Values in uaTimeoutIDs and uaTimeoutValues arrays correspond
		///				e.g if pTimeoutIDs[2] = CONNECT_TIMEOUT, puTimeoutValues[2] contains connect timeout value
		///
		/// @param		hTarget				Handle to target
		/// @param		puNoTimeouts		UINT containing number of timeouts
		///
		/// @return		A 2 dimensional array of uints containing timeout IDs (array[0][*]) and timeout values (array[1][*])
		//public static uint[][] GetTimeouts(int hTarget, uint puNoTimeouts)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Returns the version of the Target Manager currently connected to.
		///
		///  @return	string containing the version number
		public static string GetTMVersion()
		{
			System.Text.StringBuilder sb = new System.Text.StringBuilder();
			mostRecentSnResult = SNPS3GetTMVersion(sb);
			return sb.ToString();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief    Get the memory usage statistics for a process
		///
		///  @param	   hTarget			Handle to target
		///	 @param	   uProcessID		ID of process to get statistics for
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///	
		///	 @return	SNPS3UserMemoryStats struct
		//public static SNPS3UserMemoryStats GetUserMemoryStats(int hTarget, uint uProcessID)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Gets VRAM capture flags
		///
		/// @param		hTarget				Handle to target
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	A ulong holding the flags
		public static ulong GetVRAMCaptureFlags(int hTarget)
		{
			ulong l = 0;
			ulong* ptr = &l;
			mostRecentSnResult = SNPS3GetVRAMCaptureFlags(hTarget, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets VRAM information for given process
		///				Secondary VRAM information is optional, set pointer to NULL to ignore
		///
		///  @param		hTarget			Handle to target
		///	 @param		uProcessID		Process ID
		///	 @param		ignoreSecondaryInfo	Pass null to pSecondary, ignoring it (since it's optional)
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Array of size 2 holding primary (array[0]) and secondary(array[1]) SNPS3VRAMInfo structs.
		///				If ignoreSecondaryInfo is true, the array will be of size 1
		//public static SNPS3VRAMInfo[] GetVRAMInformation(int hTarget, uint uProcessID, bool ignoreSecondaryInfo)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Initialises target communications and launches Target Manager.
		///
		///  This routine must be called by each of your application threads before functions in 
		///  the DLL can be used by that thread. It sets up any necessary data structures and establishes 
		///  communication with the Target Manager.
		///
		///  When the application has finished using the DLL, SNPS3CloseTargetComms should be called.
		public static void InitTargetComms()
		{
			mostRecentSnResult = SNPS3InitTargetComms();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Kick the message queue. This is the PS3's version of "do work"
		///				for all callback functions and must be called regularly if
		///				you want your callback functions to be called. I found this out
		///				after 3 days. That was a fun 3 days.
		public static void Kick()
		{
			mostRecentSnResult = SNPS3Kick();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required buffer size for the list of target types.
		///
		///  @return    The required size of the buffer
		public static ulong ListTargetTypesBufferSize()
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3ListTargetTypes(ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     List available target types.
		///
		///  @param     puNumTypes      Pointer to unsigned int containing number of
		///								instances of \link TMAPI_TARGET_TYPE TMAPI_TARGET_TYPE \endlink to go in pTypes buffer
		///
		///  @return    A TMAPI_TARGET_TYPE struct array containing the list of target types.
		//public static TMAPI_TARGET_TYPE[] ListTargetTypes(uint puNumTypes)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the number of TTY streams.
		///
		///  @param     hTarget            Handle to target.
		///
		///  @return	uint containing the number of TTY streams
		public static uint ListTTYStreamsBufferSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3ListTTYStreams(hTarget, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     List all available TTY streams for a particular target.
		///
		///             To obtain a list of TTY Streams, call this function passing 
		///             NULL for pStreams to get the number of TTY streams. Allocate 
		///             a block of \link TTYSTREAM TTYSTREAM \endlink structures and call this function again 
		///             passing in a pointer to this buffer.
		///
		///  @param     hTarget            Handle to target.
		///  @param     puNumTTYStreams    Number of streams
		///
		///  @return	TTYSTREAM struct array detailing the TTY streams
		//public static TTYSTREAM[] ListTTYStreams(int hTarget, uint puNumTTYStreams)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Make the directory on the target.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pDirectory     Target file.
		///  @param     uMode          Access mode, used as the second parameter to CellFsMkDir (see SDK)
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void MakeDirectory(int hTarget, char[] pDirectory, uint uMode)
		{
			mostRecentSnResult = SNPS3MakeDirectory(hTarget, pDirectory, uMode);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Displays a dialog centered in the window passed or to the 
		///             desktop if NULL is passed in which displays a list of the 
		///             targets available in the Target Manager.
		///
		///             The user can choose a target which is returned through the 
		///             buffer passed in.
		///
		///             If NO targets are available, the dialog will not be displayed 
		///             and the function will return SN_E_NO_TARGETS.
		///
		///             If the user presses cancel or does not select a target before
		///             clicking OK, the function will return SN_E_NO_SEL.
		///
		///             If the function succeeds, the function will return SN_S_OK 
		///             and the contents of the buffer pointed to by phTarget will be 
		///             set to the handle for the selected target.
		///
		///             This handle can then be used in subsequent operations.
		///             
		///				Note that while this function can take in a HWND owner on the
		///				SNPS3 side of things, I can't figure out how to do that at the
		///				moment. It's desktop or nothing, I'm afraid :( [mtaylor]
		///
		///  @return	Target handle
		public static int PickTarget()
		{
			int i = 0;
			int* ptr = &i;
			mostRecentSnResult = SNPS3PickTarget(System.IntPtr.Zero, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Powers off the specified target.
		///
		///  @param     hTarget        Handle to target.
		///  @param     uForce         Flag to specify whether the target should 
		///                            forced off (0 == False).
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.4.0 - latest version (must have WOL enabled)\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void PowerOff(int hTarget, uint uForce)
		{
			mostRecentSnResult = SNPS3PowerOff(hTarget, uForce);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Powers on the specified target.
		///
		///  @param     hTarget        Handle to target.
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.4.0 - latest version (must have WOL enabled)\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void PowerOn(int hTarget)
		{
			mostRecentSnResult = SNPS3PowerOn(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required buffer size
		///
		///  @param     hTarget			Handle to a valid target.
		///	 @param		uProcessID		Process ID.
		///	 @param		uThreadID		64-bit Thread ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required size of buffer
		public static uint PPUThreadInfoExBufferSize(int hTarget, uint uProcessID, ulong uThreadID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3PPUThreadInfoEx(hTarget, uProcessID, uThreadID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Requests extended information about a PPU thread. 
		///				The buffer will contain an \link SNPS3_PPU_THREAD_INFO_EX SNPS3_PPU_THREAD_INFO_EX \endlink structure followed by the thread name. 
		///				To just get required buffer size set pBuffer to NULL\n
		///				To get standard thread information use SNPS3ThreadInfo() with unit set to PS3_UI_CPU
		///
		///  @param     hTarget			Handle to a valid target.
		///	 @param		uProcessID		Process ID.
		///	 @param		uThreadID		64-bit Thread ID.
		///  @param		puBufferSize	[in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer containing PPU thread info
		public static byte[] PPUThreadInfoEx(int hTarget, uint uProcessID, ulong uThreadID, uint puBufferSize)
		{
			uint* puBufferSize_ptr = &puBufferSize;
			byte[] toReturn = new byte[puBufferSize];
			mostRecentSnResult = SNPS3PPUThreadInfoEx(hTarget, uProcessID, uThreadID, puBufferSize_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Attach to a process\n
		///				Note: Currently can only attach to a PPU process (which will give you access to 
		///				any SPU children), thus any unit ID other than PS3_UI_CPU will cause the function to fail
		///
		///  @param     hTarget	    Handle to target
		///  @param     uUnitID     Unit upon which the process is running (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink)
		///  @param     uProcessID  Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ProcessAttach(int hTarget, uint uUnitID, uint uProcessID)
		{
			mostRecentSnResult = SNPS3ProcessAttach(hTarget, uUnitID, uProcessID);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Continues all threads from a specified process.
		///
		///  @param     hTarget			Handle to target.
		///  @param		uProcessID		Process ID.
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ProcessContinue(int hTarget, uint uProcessID)
		{
			mostRecentSnResult = SNPS3ProcessContinue(hTarget, uProcessID);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets a range of memory for the specified processor unit on the target specified by
		///				hTarget.
		///
		///  @param     hTarget		Handle to target.
		///  @param     uUnit		Processor unit (see \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	Thread ID\n
		///							- For PPU, thread ID ignored\n
		///							- For SPU, specify the thread group ID\n
		///  @param     uAddress	Address to read from.
		///  @param     nCount		Count in bytes to read.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	Byte array containing memory
		public static byte[] ProcessGetMemory(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, ulong uAddress, int nCount)
		{
			byte[] toReturn = new byte[nCount];
			mostRecentSnResult = SNPS3ProcessGetMemory(hTarget, uUnit, uProcessID, uThreadID, uAddress, nCount, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets the required buffer size
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		Process ID.
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	The required buffer size
		public static uint ProcessInfoBufferSize(int hTarget, uint uProcessID)
		{
			uint i = 0;
			SNPS3ProcessInfo(hTarget, uProcessID, &i, System.IntPtr.Zero);
			return i;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Fetches process information for a given process id.
		///
		///  @param     hTarget			Handle to target
		///  @param		uProcessID		Process ID.
		///  @param		puBufferSize	[in] Size of buffer
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	A SNPS3PROCESSINFO struct containing process info
		public static SNPS3PROCESSINFO ProcessInfo(int hTarget, uint uProcessID, uint puBufferSize)
		{
			byte[] buffer = new byte[puBufferSize];
			mostRecentSnResult = SNPS3ProcessInfo(hTarget, uProcessID, &puBufferSize, buffer);
			GCHandle handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
			SNPS3PROCESSINFO toReturn = (SNPS3PROCESSINFO)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(SNPS3PROCESSINFO));
			handle.Free();
			return toReturn; //TODO: test thread IDs
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets process count
		///
		///  @param     hTarget			Handle to target
		/// 
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Process count (and by extension, required buffer size for SNPS3ProcessList)
		public static uint ProcessListBufferSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3ProcessList(hTarget, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Fetches a list of processes running on the specified target.
		///
		///  @param     hTarget			Handle to target
		///  @param		puCount			[in] Maximum number of IDs buffer can hold (should probably be 64)
		/// 
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	uint array listing processes running on specified target
		public static uint[] ProcessList(int hTarget, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount * sizeof(uint)];
			mostRecentSnResult = SNPS3ProcessList(hTarget, puCount_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Loads an ELF file, returning the process id and thread id for the process.
		///
		///  @param     hTarget			Handle to target.
		///  @param		uPriority		Process priority (e.g. SNPS3_DEF_PROCESS_PRI).
		///  @param		pszFilename		Filename of ELF to download, /app_home/ will automatically be added to the start of this path
		///								The maximum size of the path is 1024 bytes
		///  @param		argc			Size of list of arguments to be passed to 
		///                             main function.
		///  @param		argv			Argument list to be passed to main function. Total max size is 4096 bytes
		///  @param		envc			Size of list of environment variables.
		///  @param		envv			List of environment variables to be set for
		///                             the process in format:
		///                                 VAR=value
		///								Total max size os 4096 bytes
		///  @param		uDebugFlags		Debug flags. See SNPS3_LOAD_FLAG_*
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.6.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	The uint process ID.
		public static uint ProcessLoad(int hTarget, uint uPriority, char[] pszFilename, int argc, char[,] argv,
												int envc, char[,] envv, uint uDebugFlags)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3ProcessLoad(hTarget, uPriority, pszFilename, argc, argv, envc, envv, ptr, null, uDebugFlags);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Get all file trace events from file created by SNPS3StartFileTrace() function\n
		///				Events from file will be passed to callback registered by SNPS3RegisterFileTraceHandler()
		///
		///  @param		hTarget			Handle to target
		///	 @param		pszPath			Path of file trace
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ProcessOfflineFileTrace(int hTarget, char[] pszPath)
		{
			mostRecentSnResult = SNPS3ProcessOfflineFileTrace(hTarget, pszPath);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets a range of memory for the specified processor unit on the target specified by
		///				hTarget.
		///
		///  @param     hTarget		Handle to target.
		///  @param     uUnit		Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	Thread ID.
		///							- For PPU, specify -1.
		///							- For SPU, specify the thread group ID.
		///  @param     uAddress	Address to write to.
		///  @param     nCount		Count in bytes to write.
		///  @param     pBuffer		Buffer containing data to write.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ProcessSetMemory(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, ulong uAddress, int nCount, byte[] pBuffer)
		{
			mostRecentSnResult = SNPS3ProcessSetMemory(hTarget, uUnit, uProcessID, uThreadID, uAddress, nCount, pBuffer);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Stops all threads from a specified process.
		///
		///  @param     hTarget			Handle to target.
		///  @param		uProcessID		Process ID.
		///
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void ProcessStop(int hTarget, uint uProcessID)
		{
			mostRecentSnResult = SNPS3ProcessStop(hTarget, uProcessID);
		}

		///////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Register file trace event handler, trace events will be passed to handler during a file trace
		///				or after calling SNPS3ProcessOfflineFileTrace
		///
		///  @param		hTarget			Handle to target
		///  @param		pfnCallBack		Pointer to callback function
		///  @param		pUserData		Optional pointer to user data
		///
		///  Callback must be in the form of:
		///
		/// \code
		///  __stdcall void Callback(
		///          HTARGET hTarget,       // Handle to target
		///          UINT Type,	            // SN_EVENT_FILE_TRACE
		///          UINT Param,            // Reserved - currently 0
		///          SNRESULT eResult,      // Success or failure of data fetch
		///          UINT Length,           // Length of data in event including header
		///          BYTE* Data,            // Pointer to event data
		///          void* UserData);       // Optional user data
		/// \endcode
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///				
		///  @return	A bool indicating the success of the registering
		public static bool RegisterFileTraceHandler(int hTarget, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterFileTraceHandler(hTarget, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Register a callback function that will be called when a 
		///             file transfer event occurs. See the '\link TargetEvents PLAYSTATION(R)3 Target Events \endlink'
		///             definitions above for details of the possible events.
		///
		///  @param		hTarget	      Handle to a target
		///  @param     pfnCallBack   Pointer to callback
		///  @param		pUserData     Optional user defined data.
		///
		///  Callback must be in the form of:
		///
		///  \code
		///  void Callback(
		///          HTARGET hTarget,		// Handle to target
		///          UINT Type,				// SN_EVENT_FTP
		///		     UINT Param,			// Reserved - will be 0
		///          UINT Length,			// Length of data in event including header
		///                                 // Count * sizeof(TMAPI_FT_NOTIFICATION)
		///			 SNRESULT eResult,		// Success or failure of data fetch
		///          BYTE* Data,			// Pointer to event data
		///                                 // (TMAPI_FT_NOTIFICATION*)
		///          void* UserData);		// Optional user data
		///	 \endcode
		///
		///	 @return	A bool indicating the success of the registering
		public static bool RegisterFTPEventHandler(int hTarget, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterFTPEventHandler(hTarget, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Register a pad capture callback function
		///  @details	The callback function behaves as other TMAPI callback functions do.\n
		///				Data will an instance of the \link SNPS3PadData SNPS3PadData \endlink structure sent each time pad data is sent by the target\n
		///				The length field in the callback will contain the number of instances of the \link SNPS3PadData SNPS3PadData \endlink structure in the buffer.\n
		///				SNPS3StartPadCapture() must have been called to receive data on the callback
		///
		///  @param     hTarget			Handle to target
		///  @param		pfnCallBack		Pointer to callback function
		///  @param		pUserData		Pointer to optional user data to be passed to callback function
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	A bool indicating the success of the registering
		public static bool RegisterPadCaptureHandler(int hTarget, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterPadCaptureHandler(hTarget, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Register a pad playback notification callback function
		///  @copydoc	SNPS3RegisterPadPlaybackNotificationHandler_ExInfo()
		///
		///  @param     hTarget			Handle to target
		///  @param		pfnCallBack		Pointer to callback function
		///  @param		pUserData		Pointer to optional user data to be passed to callback function
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	A bool indicating the success of the registering
		public static bool RegisterPadPlaybackNotificationHandler(int hTarget, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterPadPlaybackNotificationHandler(hTarget, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Register a callback function that will be called when a 
		///             target event occurs (e.g. processes/threads created/destroyed, 
		///             exceptions, etc). See the '\link TargetEvents  PLAYSTATION(R)3 Target Events\endlink'  
		///             definitions above for details of the possible events.
		///
		///  @param		hTarget	      Handle to a target
		///  @param     pfnCallBack   Pointer to callback
		///  @param		pUserData     Optional user defined data.
		///
		///  Callback must be in the form of:
		///
		///  void Callback(
		///          HTARGET hTarget,		// Handle to target
		///          UINT Type,				// TMAPI_EVENT_TYPE_TARGET
		///		     UINT Param,			// Reserved - will be 0
		///          UINT Length,			// Length of data in event including header
		///			 SNRESULT eResult,		// Success or failure of data fetch
		///          BYTE* Data,			// Pointer to event data
		///          void* UserData);		// Optional user data
		///
		///	 @return	A bool indicating the success of the registering
		public static bool RegisterTargetEventHandler(int hTarget, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterTargetEventHandler(hTarget, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Register a callback function that will be called when TTY 
		///             output is available from a target.
		///
		///  @param		hTarget			Handle to a target
		///  @param     uStream			TTY stream to receive TTY events on or SNPS3_TTY_ALL_STREAMS for all streams
		///  @param     pfnCallBack		Pointer to callback
		///  @param		pUserData		Optional user defined data.
		///
		///  Callback must be in the form of:
		///
		/// \code
		///  __stdcall void Callback(
		///          HTARGET hTarget,       // Handle to target
		///          UINT Type,	            // TMAPI_EVENT_TYPE_TTY
		///          UINT Param,            // Stream ID
		///          SNRESULT eResult,      // Success or failure of data fetch
		///          UINT Length,           // Length of data in event including header
		///          BYTE* Data,            // Pointer to event data
		///          void* UserData);       // Optional user data
		/// \endcode
		///
		///	 @return	A bool indicating the success of the registering
		public static bool RegisterTTYEventHandler(int hTarget, uint uStream, TMAPI_HandleEventCallback pfnCallBack, System.IntPtr pUserData)
		{
			bool success = false; //TODO: if someone could tell me what the hell pUserData is for, I may be able to handle it here? [mtaylor]
			mostRecentSnResult = SNPS3RegisterTTYEventHandler(hTarget, uStream, Marshal.GetFunctionPointerForDelegate(pfnCallBack), pUserData);
			if (!(mostRecentSnResult == SNRESULT.SN_E_BAD_PARAM) && !(mostRecentSnResult == SNRESULT.SN_E_EXISTING_CALLBACK))
			{
				success = true;

			}
			return success;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Rename a file on the target.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pSource        Target file.
		///  @param     pDestination   New name.
		///	
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void Rename(int hTarget, char[] pSource, char[] pDestination)
		{
			mostRecentSnResult = SNPS3Rename(hTarget, pSource, pDestination);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Resets specified target allowing you to specify boot parameters.
		///
		///  @param     hTarget     Handle to a valid target.
		///  @param     uReset	    PS3 reset parameter, SNPS3TM_RESETP_SOFT_RESET or
		///                         SNPS3TM_RESETP_HARD_RESET.
		public static void Reset(int hTarget, ulong uReset)
		{
			mostRecentSnResult = SNPS3Reset(hTarget, uReset);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Resets specified target allowing you to specify boot, reset and system parameters.
		///
		///  @param     hTarget     Handle to a valid target.
		///  @param     uBoot	    PS3 boot parameter, or SNPS3_DEF_BOOT_PARAM.
		///  @param     uBootMask   Mask for boot parameters, or SNPS3_BOOT_DEFAULT_MASK
		///  @param     uReset      PS3 reset parameters (SNPS3TM_RESETP_*)
		///  @param     uResetMask  Mask for reset parameters, or SNPS3TM_RESETP_ALL_MASK
		///  @param     uSystem     PS3 system parameters (SNPS3TM_SYSTEMP_*)
		///  @param     uSystemMask Mask for system parameters (typicall SNPS3TM_SYSTEMP_TARGET_MODEL)
		public static void ResetEx(int hTarget, ulong uBoot, ulong uBootMask,
												ulong uReset, ulong uResetMask,
												ulong uSystem, ulong uSystemMask)
		{
			mostRecentSnResult = SNPS3ResetEx(hTarget, uBoot, uBootMask, 
												uReset, uResetMask, uSystem, uSystemMask);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Retry the file tansfer with the given id.
		///				(see SNPS3RegisterFTPEventHandler() for more information)
		///
		///  @param     hTarget        Handle to target.
		///  @param		uTXID		   Transaction ID of transfer to continue
		///  @param     uForce		   Non-zero to bypass check for modifications
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		public static void RetryFileTransfer(int hTarget, uint uTXID, uint uForce)
		{
			mostRecentSnResult = SNPS3RetryFileTransfer(hTarget, uTXID, uForce);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Save TM settings as they are at the moment
		///
		public static void SaveSettings()
		{
			mostRecentSnResult = SNPS3SaveSettings();
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Send the data to the target using the given protocol.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pProto         Protocol information.
		///  @param     pData          The data to send.
		///  @param     uLength        The length of the data to send.
		//public static void SendCustomProtocolData(int hTarget, SNPS3Protocol pProto, byte[] pData, uint uLength)
		//{
		//}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Send pad capture data to the target for playback
		///  @copydoc	SNPS3SendPadPlaybackData_ExInfo()
		///
		///  @param     hTarget			Handle to target
		///  @param		pData			Pointer to instance of \link SNPS3PadData SNPS3PadData \endlink structure to send to the target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		//public static void SendPadPlaybackData(int hTarget, SNPS3PadData pData)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Send TTY data to the given target TTY stream.
		///
		///  @param     hTarget	   Handle to target.
		///  @param     uStream	   Stream ID.
		///  @param     pszText    Text.
		///
		public static void SendTTY(int hTarget, uint uStream, char[] pszText)
		{
			mostRecentSnResult = SNPS3SendTTY(hTarget, uStream, pszText);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets the boot parameter of the given target
		///
		///  @param     hTarget      Handle to a valid target.
		///  @param     uBoot	     PS3 boot parameter.
		///  @param     uBootMask    PS3 boot mask, SNPS3_BOOT_DEFAULT_MASK.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetBootParameter(int hTarget, ulong uBoot, ulong uBootMask)
		{
			mostRecentSnResult = SNPS3SetBootParameter(hTarget, uBoot, uBootMask);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Set a break point.
		///
		///  @param		hTarget			Handle to a target
		///  @param		uUnit			Unit upon which the process runs
		///  @param     uProcessID		Process ID.
		///  @param     uThreadID		Thread ID.
		///  @param		uAddress		Address of the break point.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetBreakPoint(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, ulong uAddress)
		{
			mostRecentSnResult = SNPS3SetBreakPoint(hTarget, uUnit, uProcessID, uThreadID, uAddress);
		}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Set file serving case sensitivity for PC (not target, which always has case sensitive)
		///				fileserving
		///
		/// @param		hTarget				Handle to target
		/// @param		bOn					Turn case sensitivity on
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		/// @return		The previous case sensitivity setting
		///
		public static bool SetCaseSensitivityServing(int htarget, bool bOn)
		{
			bool b = false;
			bool* ptr = &b;
			mostRecentSnResult = SNPS3SetCaseSensitiveFileServing(htarget, bOn, ptr);
			return *ptr;
		}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Set connection parameters for a specific target
		///
		/// @param		hTarget				Handle to target
		/// @param		pConnection			Pointer to \link TMAPI_TCPIP_CONNECT_PROP TMAPI_TCPIP_CONNECT_PROP \endlink containing information
		///									on connection
		///
		/// @return		\link ApiBase::SNRESULT SNRESULT \endlink:\n
		///				SN_E_BAD_PARAM		Pointer to structure was null\n
		///				SN_E_BAD_TARGET		hTarget is invalid handle\n
		///				SN_S_OK				Success
		//public static void SetConnectionInfo(int hTarget, TMAPI_TCPIP_CONNECT_PROP pConnection)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////////
		///	
		/// @brief		Sets the core dump flags of the target.
		///
		/// @param		hTarget				Handle to target
		/// @param		u64Flags			Flags to set (SNPS3TM_CORE_DUMP_TO_*)
		///
		public static void SetCoreDumpFlags(int hTarget, ulong u64Flags)
		{
			mostRecentSnResult = SNPS3SetCoreDumpFlags(hTarget, u64Flags);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Set data access breakpoint
		///				Note: Only one data access breakpoint can be set per process
		///
		///	 @param		hTarget			Handle to target
		///	 @param		uProcessID		Process ID
		///	 @param		uAddr			Address of data to set breakpoint on
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetDABR(int hTarget, uint uProcessID, ulong uAddr)
		{
			mostRecentSnResult = SNPS3SetDABR(hTarget, uProcessID, uAddr);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Set the debug thread control settings for a process
		///
		///  @param     hTarget			Handle to target.
		///  @param     uProcessID		Process ID
		///  @param     pInfo			Instance of \link SNPS3DebugThreadControlInfo SNPS3DebugThreadControlInfo \endlink containing new settings
		///  @param		puMaxEntries	UINT to store maximum number of control keyword entries
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		//public static void SetDebugThreadControlInfo(int hTarget, uint uProcessID, SNPS3DebugThreadControlInfo pInfo, uint puMaxEntries)
		//{
		//}

		//////////////////////////////////////////////////////////////////////////////
		///
		///	 @brief		Sets the default elf load priority for the target.
		///
		///  @param     hTarget			Handle to target.
		///  @param		uPriority		Value to set default priority to 
		///
		public static void SetDefaultLoadPriority(int hTarget, uint uPriority)
		{
			mostRecentSnResult = SNPS3SetDefaultLoadPriority(hTarget, uPriority);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets the default primary PPU thread stack size for the target.
		///
		///  @param     hTarget        Handle to target.
		///  @param     uSize          Size of the stack (should be one of the 
		///                            predefined constants, see SNPS3_ELF_STACK_SIZE_*).
		///
		public static void SetDefaultPPUThreadStackSize(int hTarget, uint uSize)
		{
			mostRecentSnResult = SNPS3SetDefaultPPUThreadStackSize(hTarget, uSize);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Update the additional load flags located on the Target Properties, Load Options
		//				page.
		///
		///  @param		hTarget				Handle to target 
		///	 @param		uFlags				New flags to set, see SNPS3_XLF_*
		///  @param		uMask				Mask that indicates which values have been set/unset (use SNPS3_XLF_ALL_FLAG_MASK for all flags)
		///
		public static void SetExtraLoadFlags(int hTarget, ulong uFlags, ulong uMask)
		{
			mostRecentSnResult = SNPS3SetExtraLoadFlags(hTarget, uFlags, uMask);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Set memory access trap conditions for a process\n
		///				Buffer is array of \link SNPS3MATRange SNPS3MATRange \endlink structures\n
		///				Note: Page size is 4k so start address must be 4k aligned and size must be a multiple of 4k\n
		///				Page condition array contains 1 byte per page\n
		///				See SNPS3_MAT_COND_* defines for possible conditions\n
		///
		///  @param     hTarget	    Handle to target
		///  @param     uProcessID  Process ID
		///  @param		uRangeCount	Number of ranges defined in buffer
		///  @param		uBufSize	Size of pBuf
		///  @param		pBuf		Buffer containing conditions
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetMATConditions(int hTarget, uint uprocessID, uint uRangeCount, uint uBufSize, byte[] pBuf)
		{
			mostRecentSnResult = SNPS3SetMATConditions(hTarget, uprocessID, uRangeCount, uBufSize, pBuf);
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Sets RSX profiling flags (note extra load flag SNPS3_XLF_ENABLE_HUD_RSX_TOOLS will need to be specified)\n
		///				For RSX profiling flags see SNPS3_HUD_RSX_USE_*
		///
		/// @param		hTarget				Handle to target
		/// @param		uRSXFlags			RSX profiling flags
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.4.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.4.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetRSXProfilingFlags(int hTarget, ulong uRSXFlags)
		{
			mostRecentSnResult = SNPS3SetRSXProfilingFlags(hTarget, uRSXFlags);
		}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Set loop point in SPU thread (replaces instruction with looping instruction, effectively stalling thread)
		///				SNPS3ClearSPULoopPoint will replace looping instruction with previously stored instruction
		///				The thread group must be stopped when you set/clear loop points
		///
		/// @param		hTarget				Handle to target
		/// @param		uProcessID			Process ID
		/// @param		uThreadID			SPU thread ID
		/// @param		uAddress			Address to set loop point
		/// @param		bCurrentPC			If equals true then address is ignored and looppoint is set at current PC address
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetSPULoopPoint(int hTarget, uint uProcessID, ulong uThreadID, uint uAddress, bool bCurrentPC)
		{
			mostRecentSnResult = SNPS3SetSPULoopPoint(hTarget, uProcessID, uThreadID, uAddress, bCurrentPC);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets the system parameter of the given target.
		///
		///  @param     hTarget			   Handle to a valid target.
		///  @param     uSystem			   PS3 System parameter.
		///  @param     uSystemMask        PS3 System mask.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetSystemParameter(int hTarget, ulong uSystem, ulong uSystemMask)
		{
			mostRecentSnResult = SNPS3SetSystemParameter(hTarget, uSystem, uSystemMask);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets target information based on members of passed in 
		///             \link SNPS3TargetInfo SNPS3TargetInfo \endlink structure.
		///
		///             Only members identified by nFlags member will be set. The 
		///             desired target to change is indicated by the hTarget member 
		///             for which nFlags *must* include the SN_TI_TARGETID flag.
		///				The info field (SN_TI_INFO) and boot parameter field (SN_TI_BOOT)
		///				cannot be set. See SNPS3SetBootParameter() and SNPS3SetConnectionInfo()
		///
		///				Notifications are sent for each field changed, note that the flags in 
		///				notifications start with SN_DTLS_. For more information see the page on
		///				\link TargetEvents PLAYSTATION(R)3 target events \endlink
		///
		///  @param     pTargetInfo    Address of target info structure.
		///
		//public static void SetTargetInfo(SNPS3TargetInfo pTargetInfo)
		//{
		//}

		///////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Sets the various TM timeouts. The IDs in the pTimeoutIDs array
		///				correspond to the values in the puTimeoutValues array\n
		///				e.g pTimeoutIDs[2] = CONNECT_TIMEOUT, so value puTimeoutValues[2] will
		///				be the connect timeout value
		///
		/// @param      hTarget			Handle to target.
		/// @param		uNoTimeouts		Number of timeouts to be changed
		/// @param		pTimeoutIDs		Array of IDs of timeouts to be changed
		/// @param		puTimeoutValues	Array of new values for timeouts (correspond to prev array)
		///
		public static void SetTimeouts(int hTarget, uint uNoTimeouts, SNPS3_TM_TIMEOUT[] pTimeoutIDs, uint[] puTimeoutValues)
		{
			mostRecentSnResult = SNPS3SetTimeouts(hTarget, uNoTimeouts, pTimeoutIDs, puTimeoutValues);
		}

		///////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Sets whether to use the Debug Agent from flash or Target Manager.
		///
		/// @param		hTarget			Handle to target
		/// @param		bSonyAgent		If true use DA from flash, if false use DA from TM
		///
		public static void SetUseSonyDebugAgent(int hTarget, bool bSonyAgent)
		{
			mostRecentSnResult = SNPS3SetUseSonyDebugAgent(hTarget, bSonyAgent);
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Sets VRAM capture flags (SNPS3_ENABLE_VRAM_CAPTURE)
		///
		/// @param		hTarget				Handle to target
		/// @param		uVRAMFlags			Value of VRAM capture flags
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SetVRAMCaptureFlags(int hTarget, ulong uVRAMFlags)
		{
			mostRecentSnResult = SNPS3SetVRAMCaptureFlags(hTarget, uVRAMFlags);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Continues all threads in a specified thread group.
		///
		///  @param     hTarget			Handle to target.
		///  @param		uProcessID		Process ID
		///  @param		uThreadGroupID	Thread group ID.
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SPUThreadGroupContinue(int hTarget, uint uProcessID, ulong uThreadGroupID)
		{
			mostRecentSnResult = SNPS3SPUThreadGroupContinue(hTarget, uProcessID, uThreadGroupID);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Stops all threads in a specified SPU thread group.
		///
		///  @param     hTarget				Handle to target.
		///  @param		uProcessID			Process ID
		///  @param		uThreadGroupID		Thread Group ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void SPUThreadGroupStop(int hTarget, uint uProcessID, ulong uThreadGroupID)
		{
			mostRecentSnResult = SNPS3SPUThreadGroupStop(hTarget, uProcessID, uThreadGroupID);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Start a file trace for the process
		///
		///  @param     hTarget			Handle to target
		///  @param     uPID			Process ID
		///  @param		uSize			Size of the container that will be created for use by the file trace. 1mb is the recommended size
		///  @param		pszFileName		Specifies the location to which the trace file will be written
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StartFileTrace(int hTarget, uint uPID, uint uSize, char[] pszFileName)
		{
			mostRecentSnResult = SNPS3StartFileTrace(hTarget, uPID, uSize, pszFileName);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Start pad capture on the target (recording all information from the control pad)
		///  @details	Once pad capture is started you will need a pad capture callback function registered
		///				(using SNPS3RegisterPadCaptureHandler()). Instances of \link SNPS3PadData SNPS3PadData \endlink will be passed
		///				to the callback each time pad data is sent by the target, see CTRLP spec for details
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StartPadCapture(int hTarget)
		{
			mostRecentSnResult = SNPS3StartPadCapture(hTarget);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Start pad playback on the target
		///  @details	Once pad playback is started you can send pad capture data using SNPS3SendPadPlaybackData()
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StartPadPlayback(int hTarget)
		{
			mostRecentSnResult = SNPS3StartPadPlayback(hTarget);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Stop process file trace
		///
		///  @param     hTarget			Handle to target
		///  @param     uPID			Process ID
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StopFileTrace(int hTarget, uint uPID)
		{
			mostRecentSnResult = SNPS3StopFileTrace(hTarget, uPID);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Stop pad capture on the target 
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StopPadCapture(int hTarget)
		{
			mostRecentSnResult = SNPS3StopPadCapture(hTarget);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Stop pad playback on the target 
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void StopPadPlayback(int hTarget)
		{
			mostRecentSnResult = SNPS3StopPadPlayback(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Terminate the given game process.
		///				See SDK sample in (SDK LOCATION)\\samples\\common\\gcmutil\\exit_callback.cpp for 
		///				how to deal with terminate signal on target
		///
		///  @param     hTarget        Handle to target.
		///  @param     uProcessID     Process ID.
		///  @param     uTimeout       Timeout (ms).
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.2.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.2.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void TerminateGameProcess(int htarget, uint uProcessID, uint uTimeout)
		{
			mostRecentSnResult = SNPS3TerminateGameProcess(htarget, uProcessID, uTimeout);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Restarts specified thread.
		///				Note: SPU mode only included for backwards compatibility and only works
		///				if thread group is stopped, instead use SNPS3ClearSPULoopPoint() with bCurrentPC set to true
		///
		///  @param     hTarget     Handle to a valid target.
		///  @param     uUnit		Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	64-bit Thread ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void ThreadContinue(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID)
		{
			mostRecentSnResult = SNPS3ThreadContinue(hTarget, uUnit, uProcessID, uThreadID);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Reads an array of registers from the specified thread.
		///
		///  @copydoc	SNPS3ThreadGetRegisters_ExInfo()
		///  @param     hTarget     Handle to a valid target.
		///  @param     uUnit		Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	64-bit Thread ID.
		///  @param		uNumRegisters 
		///							Number of registers to get.
		///  @param		puNum		Buffer holding array of register numbers to get.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	byte array containing values returned from SNPS3ThreadGetRegisters
		public static byte[] ThreadGetRegisters(int hTarget, uint uUnit, uint uprocessID, ulong uThreadID, uint uNumRegisters, uint[] puNum)
		{
			byte[] toReturn = new byte[uNumRegisters];
			mostRecentSnResult = SNPS3ThreadGetRegisters(hTarget, uUnit, uprocessID, uThreadID, uNumRegisters, puNum, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets required buffer size
		///
		///  @param     hTarget			Handle to a valid target.
		///	 @param		uUnit			Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///	 @param		uProcessID		Process ID.
		///	 @param		uThreadID		64-bit Thread ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Required buffer size for a call to ThreadInfo
		public static uint ThreadInfoBufferSize(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3ThreadInfo(hTarget, uUnit, uProcessID, uThreadID, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Requests information about a thread. For PPU units the buffer will contain an
		///				\link SNPS3_PPU_THREAD_INFO SNPS3_PPU_THREAD_INFO\endlink structure followed by the thread name. For SPUs the
		///				buffer will contain an \link SNPS3_SPU_THREAD_INFO SNPS3_SPU_THREAD_INFO \endlink structure followed by the file
		///				name and thread name.\n
		///				Note: You can get extended PPU thread info using SNPS3PPUThreadInfoEx()
		///
		///  @param     hTarget			Handle to a valid target.
		///	 @param		uUnit			Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///	 @param		uProcessID		Process ID.
		///	 @param		uThreadID		64-bit Thread ID.
		///  @param		puBufferSize	[in] Size of buffer
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	Buffer containing the information
		public static byte[] ThreadInfo(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, uint puBufferSize)
		{
			uint* puBufferSize_ptr = &puBufferSize;
			byte[] toReturn = new byte[puBufferSize];
			mostRecentSnResult = SNPS3ThreadInfo(hTarget, uUnit, uProcessID, uThreadID, puBufferSize_ptr, toReturn);
			return toReturn;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Sets the registers for a thread.
		///
		///  @param     hTarget     Handle to a valid target.
		///  @param     uUnit		Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	64-bit Thread ID.
		///  @param		uNumRegisters
		///                         Number of registers to set.
		///  @param		puNum		Buffer holding array of register
		///                         numbers to set.
		///  @param		pRegBuffer	Buffer containing values of registers to set.
		///							Each register is of size SNPS3_REGLEN.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void ThreadSetRegisters(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID, uint uNumRegisters,
																				uint[] puNum, byte[] pRegBuffer)
		{
			mostRecentSnResult = SNPS3ThreadSetRegisters(hTarget, uUnit, uProcessID, uThreadID, uNumRegisters, puNum, pRegBuffer);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Stops specified thread.
		///				Note: SPU mode only included for backwards compatibility and only works
		///				if thread group is stopped, instead use SNPS3SetSPULoopPoint() with bCurrentPC set to true
		///
		///  @param     hTarget     Handle to a valid target.
		///  @param     uUnit		Processor unit (see enum \link PS3tmapi.h::ESNPS3UNIT ESNPS3UNIT \endlink).
		///  @param		uProcessID	Process ID.
		///  @param		uThreadID	64-bit Thread ID.
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void ThreadStop(int hTarget, uint uUnit, uint uProcessID, ulong uThreadID)
		{
			mostRecentSnResult = SNPS3ThreadStop(hTarget, uUnit, uProcessID, uThreadID);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Returns a textual description of an SN error code.
		///
		///  @param     snr              SN error code.
		///
		///  @return	Error string
		//public static string TranslateError(SNRESULT snr)
		//{
		//}

		/////////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief	    Trigger a core dump of the given process.
		///
		/// @param		hTarget				Handle to target
		/// @param		uProcessID			Process ID
		/// @param		uUserData1			User Data
		/// @param		uUserData2			User Data
		/// @param		uUserData3			User Data
		///
		/// @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void TriggerCoreDump(int hTarget, uint uProcessID, ulong uUserData1, ulong uUserData2, ulong uUserData3)
		{
			mostRecentSnResult = SNPS3TriggerCoreDump(hTarget, uProcessID, uUserData1, uUserData2, uUserData3);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Unregister the protocol.
		///
		///  @param     hTarget        Handle to target.
		///  @param     pProto         Protocol information.
		///
		//public static void UnRegisterCustomProtocol(int hTarget, SNPS3Protocol pProto)
		//{
		//}

		///////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		Unregister file trace event callback. Callbacks are registered using the function SNPS3RegisterFileTraceHandler
		///
		///  @param		hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		/// 
		public static void UnRegisterFileTraceHandler(int hTarget)
		{
			mostRecentSnResult = SNPS3UnRegisterFileTraceHandler(hTarget);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		UnRegister a pad capture callback function
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void UnRegisterPadCaptureHandler(int hTarget)
		{
			mostRecentSnResult = SNPS3UnRegisterPadCaptureHandler(hTarget);
		}

		////////////////////////////////////////////////////////////////////////////////////
		///
		///  @brief		UnRegister a pad playback notification callback function
		///
		///  @param     hTarget			Handle to target
		///
		///  @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.5.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.5.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		public static void UnRegisterPadPlaybackNotificationHandler(int hTarget)
		{
			mostRecentSnResult = SNPS3UnRegisterPadPlaybackNotificationHandler(hTarget);
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Upload all files in a directory (uploads the containing directory)
		///
		///  @param     hTarget				  Handle to target.
		///  @param     pSource				  Source.
		///  @param     pDestination          Destination.
		///  @param		pLastTXID			  Pointer to unsigned int where ID of last file to be uploaded will be written
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	ID of the last file to be uploaded
		///	 
		public static uint UploadDirectory(int hTarget, char[] pSource, char[] pDestination)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3UploadDirectory(hTarget, pSource, pDestination, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Upload the given file to the target.
		///				Note:	This only initiates the upload, to check progress, success or failure you need to
		///						monitor \link TargetEvents target events\endlink
		///						(see SNPS3RegisterFTPEventHandler() for more information)
		///
		///  @param     hTarget        Handle to target.
		///  @param     pSource        Local file.
		///  @param     pDestination   Target file.
		///
		///  @version   Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///  @return	uint containing transaction ID
		///  
		public static uint UploadFile(int hTarget, char[] pSource, char[] pDestination)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3UploadFile(hTarget, pSource, pDestination, ptr);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Gets process count
		///
		///  @param     hTarget			Handle to target
		/// 
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	uint containing the process count, a buffer size for UserProcessList
		///	 
		public static uint UserProcessListBufferSize(int hTarget)
		{
			uint i = 0;
			uint* ptr = &i;
			mostRecentSnResult = SNPS3UserProcessList(hTarget, ptr, null);
			return *ptr;
		}

		//////////////////////////////////////////////////////////////////////////////
		///
		///  @brief     Fetches a list of user processes running on the specified target.
		///
		///  @param     hTarget			Handle to target
		///  @param		puCount			[in] Maximum number of IDs buffer can hold
		/// 
		///	 @version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 1.0.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.0.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.6.0 - latest version
		///
		///	 @return	uint array containing the list
		///	 
		public static uint[] UserProcessList(int hTarget, uint puCount)
		{
			uint* puCount_ptr = &puCount;
			uint[] toReturn = new uint[puCount];
			mostRecentSnResult = SNPS3UserProcessList(hTarget, puCount_ptr, toReturn);
			return toReturn;
		}

		/////////////////////////////////////////////////////////////////////////////////
		///
		/// @brief		Perform VRAM capture on process
		///
		/// @param		hTarget				Handle to target
		/// @param		uPID				Process to capture
		/// @param		pVRAMInfo			Instance of SNPS3VRAMInfo containing parameters for capture (if NULL values will be retrieved from target)
		/// @param		czpFileName			Char array containing filename to write capture to
		///
		///	@version	Reference tool (DECR-1000 / DECR-1000A)							: SDK 2.7.0 - latest version\n
		///				Debugging station (DECHA00J / DECHA00A / DECHJ00J / DECHJ00A)	: SDK 2.7.0 - latest version\n
		///				Reference tool (DECR-1400J / DECR-1400A)						: SDK 2.7.0 - latest version
		///
		//public static void VRAMCapture(int hTarget, uint uPID, SNPS3VRAMInfo pVRAMInfo, char[] czpFileName)
		//{
		//}
	}
}