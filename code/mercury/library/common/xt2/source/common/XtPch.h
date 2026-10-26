#ifndef XT_STDAFX_H
#define XT_STDAFX_H

#include <core/Maths.h>

// START HACK - Waiting for proper core headers...
#if !defined(PLATFORM_PC) && !defined(PLATFORM_PS3)
#ifdef _MSC_VER
#define PLATFORM_PC
#else
#ifndef PLATFORM_PS3
#define PLATFORM_PS3
#endif
#endif
#endif
// END HACK

#if defined(XT_TOOL)
	#if defined(PLATFORM_PC)

		//#define WIN32_LEAN_AND_MEAN

		#define NO_MERCURY_ASSERT

		#define WINVER			0x0501
		#define _WIN32_WINNT	0x0501

		#include <winsock2.h>
		#include <windows.h>
		#include <assert.h>
		#include <string.h>
		#include <ctype.h>
		#include <stdio.h>
		#include <stdlib.h>
		#include <math.h>

		#pragma warning(disable:4355)	// This used in initialiser list
		#pragma warning(disable:4706)	// Assignment within conditional expression
		#pragma warning(disable:4200)	// Zero sized array in struct
		#pragma warning(disable:4345)	// An object of POD type constructed with an initializer of the form () will be default-initialized
		#pragma warning(disable:4512)	// Assignment operator could not be generated

	#else
	#error XtTool not implemented for this platform
	#endif
#elif defined(XT_GAME)
	#if defined(PLATFORM_PS3)

		#include <assert.h>
		#include <string.h>
		#include <stdint.h>
		#include <cell/atomic.h>
		#include <stdio.h>
		#include <stdlib.h>

		#define _stricmp strcasecmp
		#define snPause() __asm__ volatile ( "tw 31,1,1" )

	#elif defined(PLATFORM_PC)

		#include <assert.h>
		#include <string.h>
		#include <winsock2.h>
		#include <stdio.h>
		#include <stdlib.h>

		#pragma warning(disable:4355)	// This used in initialiser list
		#pragma warning(disable:4706)	// Assignment within conditional expression

	#else
	#error XtGame not implemented for this platform
	#endif
#else
#error XT_GAME or XT_TOOL must be defined
#endif

#endif // XT_STDAFX_H
