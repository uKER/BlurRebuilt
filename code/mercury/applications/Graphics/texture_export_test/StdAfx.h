#ifndef STDAFX_H
#define STDAFX_H

#ifdef PLATFORM_PC

#pragma warning (disable:4996)

#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <windows.h>
	#include <time.h>
#elif defined(PLATFORM_XENON)
	#include <xtl.h>
	#include <stdio.h>
	#include <stdlib.h>
#elif defined(PLATFORM_PS3)
	#ifndef PLATFORM_SPU
		#include <limits.h>
		#include <stdio.h>
		#include <stdlib.h>
		#include <sys/timer.h>
		#include <sysutil/sysutil_sysparam.h>
		#include <cell/pad.h>
	#endif
#endif

#include <list>
#include <Core/Core.h>
#include <Display/Core/Includes.h>
#include <Core/Maths.h>
#include <Core/Utility/Utility.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Display.h>
#include <memory.h>
#include <stdio.h>
#include <limits.h>
#include "Samples/System/Memory.h"
#include "Samples/System/Debug.h"


#endif // STDAFX_H
