// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A place to add custom user settings.

// For the PS2 our default setting is to use a multi threaded socket.
#if (defined(__mips__) || defined(__MIPS__)) && !defined(__psp__)  

	// pick a ps2 network stack
	#define BD_PS2_USE_LIBINSOCK
//	#define BD_PS2_USE_LIBEENET

	// ps2 sockets should be multi-threaded
	#define BD_MULTITHREADED_SOCKET

	// if your memory functions are not thread safe define this
	#define BD_MEMORY_FUNCTIONS_NOT_THREAD_SAFE
#endif // ps2

#if defined (__psp__)

// Uncomment to use ad-hoc broadcast as primary way of communication
#if defined BD_PLATFORM_PSP_ADHOC
//# define BD_SOCKET_DO_BROADCAST
#endif
#endif

// Enable multithreaded sockets on Wii by default.
#if defined(GEKKO)
#define BD_MULTITHREADED_SOCKET
#endif

#define BD_COMPILE_RELAY_SOCKET_ROUTER

