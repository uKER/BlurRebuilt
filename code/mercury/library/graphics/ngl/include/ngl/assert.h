#ifndef ASSERT_H__INCLUDED
#define ASSERT_H__INCLUDED

#if defined(_MSC_VER) && (_MSC_VER > 1000)
	#pragma once
#endif

#include "ngl/defs.h"

#if defined(ACT_TOOLS_DEBUG_QA)
#define ACT_TOOLS_DEBUG
#endif

#if defined(_DEBUG) && !defined(ACT_TOOLS_DEBUG)
#define ACT_TOOLS_DEBUG
#endif

#if defined (_M_IX86)
	#if defined(_MSC_VER) && (_MSC_VER > 1000)
		#define ActToolsDebugBreak()  __debugbreak()
	#else
		#define ActToolsDebugBreak() __asm { int 3 }
	#endif
#else
		
#endif

#if defined(ACT_TOOLS_DEBUG)

namespace act_tools_assert
{
	
	bool IsIgnored( const char* file, int line, const char* expression );
	void Ignore( const char* file, int line, const char* expression );
	int Message( const char* file, int line, const char* expression, bool show );

}


#define ACT_TOOLS_ASSERT( expr ) assert( expr )

#define ACT_TOOLS_ASSERTE( expr )                                            \
		((!(expr) &&                                                         \
		  !act_tools_assert::IsIgnored(__FILE__, __LINE__, #expr) &&         \
		  (act_tools_assert::Message(__FILE__, __LINE__, #expr, true))) ?    \
			ActToolsDebugBreak() : ACT_TOOLS_NOOP)                           \

#ifdef ACT_TOOLS_DEBUG_QA
	#define ACT_TOOLS_QA_ASSERT     ACT_TOOLS_ASSERT
	#define ACT_TOOLS_QA_ASSERTE    ACT_TOOLS_ASSERTE
#else
	#define ACT_TOOLS_QA_ASSERT     ACT_TOOLS_NOOP
	#define ACT_TOOLS_QA_ASSERTE    ACT_TOOLS_NOOP
#endif // ACT_TOOLS_QA

#define ACT_TOOLS_DEBUG_UNUSEDPTR  0xCDCDCDCDCDCDCDCD

#else // ACT_TOOLS_RELEASE?

#define ACT_TOOLS_ASSERT( expr )         ACT_TOOLS_NOOP
#define ACT_TOOLS_ASSERTE( expr )        ACT_TOOLS_NOOP
#define ACT_TOOLS_QA_ASSERT( expr )      ACT_TOOLS_NOOP
#define ACT_TOOLS_QA_ASSERTE( expr )     ACT_TOOLS_NOOP

#endif // ACT_TOOLS_DEBUG

#endif //ASSERT_H__INCLUDED

