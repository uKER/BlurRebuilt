//CoreDebug.h - contains debug prints, asserts and error handling

#include <core/bizassert.h>

#ifndef NO_MERCURY_ASSERT // Define this to prevent mercury from declaring its own version of assert


#ifdef CFG_SPEED
#ifndef assert
	#define assert(x)
#endif // assert
#else // CFG_SPEED

#if defined( PLATFORM_SPU )

#if !defined(CFG_SPEED) && !defined(JOBQUEUE_KERNEL)

#define ASL__TOKEN_JOIN(X,Y) X##Y 
#define ASL_TOKEN_JOIN(X,Y) ASL__TOKEN_JOIN(X,Y)
#define ASL_UNIQUE_TOKEN ASL_TOKEN_JOIN(__tmp_token,__LINE__)

	extern bool spuAssert();
	#undef assert
	#define assert(X) (void) ((!(bool)!(X)) || spuAssert());
#else // CFG_DEBUG, etc.
	#define assert(X)
#endif // CFG_SPEED

#else // PLATFORM_SPU
	// define our own assert, which calls the coredebug assert callback function
	#ifndef assert
	//#define assert(X) (void) ((!!(X)) || (*::g_pCoreAssertFunction)(__DEBUG_FUNC__, #X, __FILE__, __LINE__))
	/// JASOND: Fix for compiler bug with !!(X) where X is a Scalar_Bool
	#define assert(X) (void) ((!(bool)!(X)) || (*::g_pCoreAssertFunction)("", #X, __FILE__, __LINE__))
	#endif

#endif // PLATFORM_SPU

#endif // CFG_SPEED

#ifdef __LINT__
#undef assert
extern int myexit(int x);
#define assert(X) if (!(X)) myexit(-1)
#define assert(X) (void) ((!(bool)!(X)) || myexit(-1))
#endif

#ifndef __GNUC__
// dont have a pragma once in this file as it is included several times
//#pragma once
#endif
#ifndef CORE_DEBUG_H
#define CORE_DEBUG_H

#include <stdio.h>
#include <core/core.h>
#include <core/utility/newclib.h>

// Using macros to get line numbers etc correct

template<bool> 
struct CompileTimeError;

template<> 
struct CompileTimeError< true > 
{
    static const bool Result = true;
};

typedef bool AssertFunction(const char*,const char*,const char*,int);
extern AssertFunction *g_pCoreAssertFunction;

#define StaticAssert(expression, error_message)	{ CompileTimeError<((expression) != 0)> ASSERT_ERROR_##error_message; (void)ASSERT_ERROR_##error_message; }

// define function location macro
#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __DEBUG_FUNC__ __FUNCSIG__
#endif
#elif defined(__GNUC__)
#define __DEBUG_FUNC__ __PRETTY_FUNCTION__
#else
#define __DEBUG_FUNC__ ""
#endif

#ifdef USER_LEEW
//	#define assert(x)
#endif

#ifdef CFG_SPEED
	#define CORE_FATAL_ERROR(x)
	inline void CORE_PRINT(const char* format, ...)
	{
	}
#else
#if defined( PLATFORM_SPU )
	inline void CORE_PRINT(const char* format, ...)
	{

	}
#else

	#ifdef COMPILER_VC
		void DoErrorString(const char* string, const char* file, const U32 line);
		#include <crtdbg.h>
		#define ALLOW_CRT_CHECK_MEMORY
		#ifdef PLATFORM_PC
			#define CORE_FATAL_ERROR(x) \
			{\
				DoErrorString(x, __FILE__, __LINE__);\
			}
		#else
		#define CORE_FATAL_ERROR(x) \
		{\
			printf("Fatal Error at: %s %d\n%s", __FILE__, __LINE__, x);\
		}
		#endif
		void CORE_PRINT(const char* format, ...);

	#elif defined PLATFORM_PS3
		#define CORE_FATAL_ERROR(x) \
		{\
			printf("Fatal Error at: %s %d\n%s", __FILE__, __LINE__, x);\
		}
		void CORE_PRINT(const char* format, ...);
		
	#else
		void CORE_PRINT(const char* format, ...);

		#define CORE_FATAL_ERROR(x) \
		{\
			printf("Fatal Error at: %s %d\n%s", __FILE__, __LINE__, x);\
		}
	#endif
#endif

#endif

#ifndef CFG_MASTER
#define CHECK_MEMORY_INTEGRITY() { bool rval = CoreDebug::gpMemoryIntegrityFunction(); BIZ_PANIC(rval); }
#else
#define CHECK_MEMORY_INTEGRITY()
#endif

namespace CoreDebug
{

/// assert function callback type
typedef void AssertFunc(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber);

/// assert callback function, uses core_Assert by default
extern AssertFunc *gpAssertFunction;

/// check memory integrity callback
typedef bool CheckMemoryIntegrityFunc(void);

/// check memory integrity callback function, uses CheckMemoryIntegrity by default
extern CheckMemoryIntegrityFunc *gpMemoryIntegrityFunction;

bool CheckMemoryIntegrity(void);

bool IsDebuggerAttached();

};





#endif


#endif // NO_MERCURY_ASSERT

