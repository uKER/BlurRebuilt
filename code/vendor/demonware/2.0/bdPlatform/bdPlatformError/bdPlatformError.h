// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: 

#ifndef BD_PLATFORM_ERROR_H
#define BD_PLATFORM_ERROR_H

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>
#include <bdPlatform/bdPlatformPatterns/bdLinkable.h>

#define BD_COMPILE_ASSERT(expr, msg) typedef bdNChar8 bdCompileAssert_##msg[(expr) ? 1 : -1]

extern bdBool g_assertFalse;

void BD_CALL bdHandleAssert(const bdBool expr,
							const bdNChar8 *const exprString,
							const bdNChar8 *const file,
							const bdNChar8 *const function,
							const bdUInt line,
							const bdNChar8 *const format,
							...);

#if defined BD_COMPILER_MSVC7 || defined BD_COMPILER_MSVC_XBOX

// Compiler does not support variadic macros

#include <bdPlatform/bdPlatformLog/bdMessageProxy.h>

#define BD_ASSERT \
	const bdMessageProxy( BD_LOG_ERROR, __FILE__ ,__FUNCTION__, __LINE__, "err/" ).handleAssert

#elif defined BD_COMPILER_CODEWARRIOR

// Compiler does support variadic macros

#define BD_ASSERT(expr, ...) \
	bdHandleAssert( expr, #expr, __FILE__, __FUNCTION__, __LINE__,  __VA_ARGS__ )

#else // !BD_COMPILER_CODEWARRIOR

// Compiler does support variadic macros
// Note ## is not required on win32 or Xbox platforms

#define BD_ASSERT(expr, msg, ...) \
	bdHandleAssert( expr, #expr, __FILE__, __FUNCTION__, __LINE__, msg, ## __VA_ARGS__ )

#endif 

#endif // BD_PLATFORM_ERROR_H
