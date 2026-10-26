#ifndef MERCURY_CORE_BIZASSERT_H
#define MERCURY_CORE_BIZASSERT_H

	#include <Core/Core.h>

	//
	// Bizarre Creations Assert API
	//

	//
	//	Configuration
	//
	//		Asserts are enabled by default in all builds except for SPEED and GOLD.
	//		Asserts are always DISABLED in GOLD builds.
	//
	//		Define BIZ_ASSERT_ENABLED to force enable asserts in all builds except GOLD.
	//		Define BIZ_ASSERT_DISABLED to force disable asserts in all builds.
	//
	//	Usage
	//
	//		Each macro has three variants, a standard assert macro which accepts only an expression, an extend macro which accepts a
	//		custom message in addition to the expression, and an always fail macro which accepts only a message.
	//
	//		BIZ_ASSERT(expr)
	//			Takes a single expression as an argument, if the evaluation of the expression returns false the application assert
	//			handler will be called and the failed expression will be displayed along with the file and line the assert was
	//			declared upon, if the application assert handler returns false the application will also debug break and halt.
	//
	//		BIZ_ASSERT_MSG(expr, msg)
	//			This macro works in a similar fashion to the previous, but takes an additional string as a second argument. This string
	//			is passed to the application assert handler and will be displayed along with the information previously listed.
	//
	//		BIZ_ASSERT_FAIL(msg)
	//			Unlike the previous two macros this one does not accept an expression argument, this is because the macro always calls
	//			into the application assert handler. Like BIZ_ASSERT_MSG you may pass a string to be displayed.
	//
	//		In addition to BIZ_ASSERT there are three other flavours of assert available. Your choice between them should reflect the
	//		severity of the test being asserted, along with whether the situation is recoverable, or if you are merely trying to relay
	//		some information to the user.
	//
	//		BIZ_ASSERT
	//			Should be used to test for programmer error, such as indexing beyond the end of an array, passing a null argument when
	//			the contract to the function specifies that you may not pass null arguments. This should only be used when the failure
	//			case is unexpected, for example, you shouldn't assert the pointer returned from malloc is not equal to null because
	//			it is expected that malloc can fail.
	//
	//		BIZ_NOTICE
	//			Should be used to test for conditions which are expected and significant, but don't necessary indicate an error. For example,
	//			if a configuration file was missing you could use BIZ_NOTICE to inform the user that you'll be using a default configuration
	//			instead; the application is still going to be fully functional, but might not behave quite as the user expects.
	//
	//		BIZ_WARNING
	//			Should be used to test for conditions which are actionable in some way by the user, but beyond the programmers control to prove
	//			a sensible default that doesn't impact functionality. For example, if a scene is missing its collision mesh and there's no sensible
	//			default the game can provide to continue in a fully functional way it could use BIZ_WARNING to inform the user of the problem,
	//			then disable physics for that scene and continue. Another example would be using BIZ_WARNING to test if malloc returned null,
	//			this would display an informational message to the user and allow them to decide whether they want to continue, however, if they
	//			do decide to continue the game should not immediately crash, BIZ_WARNING should be used in addition to robust error handling, not
	//			in place of it.
	//
	//		BIZ_PANIC
	//			In situations where there's no reasonable course of action to continue and where total system failure is inevitable, you may use
	//			BIZ_PANIC to inform the user of the problem. For example, if you were writing a memory heap manager and you detected corruption in
	//			the heap due to someone stomping over memory you could call BIZ_PANIC to indicate this. Crashing after a user continues on from a
	//			BIZ_PANIC is fine as there's no reasonable way you could correct this problem. BIZ_PANIC also creates a stack dump to aid in debugging.
	//

	//
	// * BIZ_ASSERT / BIZ_ASSERT_MSG / BIZ_ASSERT_FAIL *
	//

	#define BIZ_ASSERT(expr)				BIZ_IMPL_ASSERT(kBizAssert_Assert,			(expr),	#expr,	0)
	#define BIZ_ASSERT_MSG(expr, msg, ...)	BIZ_IMPL_ASSERT(kBizAssert_Assert,			(expr),	#expr,	(msg),	## __VA_ARGS__)
	#define BIZ_ASSERT_FAIL(msg, ...)		BIZ_IMPL_ASSERT_FALSE(kBizAssert_Assert,	0,		(msg),	## __VA_ARGS__)

	#define BIZ_ASSERT_SPEED(expr	)				BIZ_IMPL_ASSERT_SPEED(kBizAssert_Assert,		(expr),	#expr,	0)
	#define BIZ_ASSERT_MSG_SPEED(expr, msg, ...)	BIZ_IMPL_ASSERT_SPEED(kBizAssert_Assert,		(expr),	#expr,	(msg),	## __VA_ARGS__)
	#define BIZ_ASSERT_FAIL_SPEED(msg, ...)			BIZ_IMPL_ASSERT_FALSE_SPEED(kBizAssert_Assert,	0,	(msg),	## __VA_ARGS__)

	//
	// * BIZ_NOTICE / BIZ_NOTICE_MSG / BIZ_NOTICE_FAIL *
	//

	#define BIZ_NOTICE(expr)				BIZ_IMPL_ASSERT(kBizAssert_Notice,	(expr),	#expr,	0)
	#define BIZ_NOTICE_MSG(expr, msg, ...)	BIZ_IMPL_ASSERT(kBizAssert_Notice,	(expr),	#expr,	(msg),	## __VA_ARGS__)
	#define BIZ_NOTICE_FAIL(msg, ...)		BIZ_IMPL_ASSERT_FALSE(kBizAssert_Notice,	0,		(msg),	## __VA_ARGS__)

	//
	// * BIZ_WARNING / BIZ_WARNING_MSG / BIZ_WARNING_FAIL *
	//

	#define BIZ_WARNING(expr)				BIZ_IMPL_ASSERT(kBizAssert_Warning,	(expr),	#expr,	0)
	#define BIZ_WARNING_MSG(expr, msg, ...)	BIZ_IMPL_ASSERT(kBizAssert_Warning,	(expr),	#expr,	(msg),	## __VA_ARGS__)
	#define BIZ_WARNING_FAIL(msg, ...)		BIZ_IMPL_ASSERT_FALSE(kBizAssert_Warning,	0,		(msg),	## __VA_ARGS__)

	//
	// * BIZ_PANIC / BIZ_PANIC_MSG / BIZ_PANIC_FAIL *
	//

	#define BIZ_PANIC(expr)					BIZ_IMPL_ASSERT(kBizAssert_Panic,	(expr),	#expr,	0)
	#define BIZ_PANIC_MSG(expr, msg, ...)	BIZ_IMPL_ASSERT(kBizAssert_Panic,	(expr),	#expr,	(msg),	## __VA_ARGS__)
	#define BIZ_PANIC_FAIL(msg, ...)		BIZ_IMPL_ASSERT_FALSE(kBizAssert_Panic,		0,		(msg),	## __VA_ARGS__)

	//
	// * BIZ_HALT *
	//
	// Immediately halt application execution
	//

	#define BIZ_HALT() BIZ_IMPL_HALT()

	//
	// * BizAssertHandler *
	//
	// Interface to allow applications to overload default assert functionality
	//

	enum BizAssertKind
	{
		kBizAssert_Assert,
		kBizAssert_Notice,
		kBizAssert_Warning,
		kBizAssert_Panic
	};

	typedef bool (*BizAssertHandler)(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line);

	BizAssertHandler GetBizAssertHandler();
	void SetBizAssertHandler(BizAssertHandler handler);

	const char* GetBizAssertKindText(BizAssertKind kind);

	//
	// ****** Implementation ******
	//

	#if defined(CFG_GOLD)
		#define BIZ_ASSERT_DISABLED // Always DISABLED in gold
		#undef BIZ_ASSERT_ENABLED
	#else
		#if defined(BIZ_ASSERT_ENABLED) || defined(BIZ_ASSERT_DISABLED)
			#if defined(BIZ_ASSERT_ENABLED) && defined(BIZ_ASSERT_DISABLED)
				#undef BIZ_ASSERT_ENABLED // DISABLED trumps ENABLED
			#endif
		#else
			#if defined(CFG_SPEED)
				#define BIZ_ASSERT_DISABLED // DISABLED by default in speed
			#else
				#define BIZ_ASSERT_ENABLED // ENABLED by default in all other builds
			#endif
		#endif
	#endif

	#if defined(CFG_GOLD)
		#define BIZ_ASSERT_SPEED_DISABLED // Always DISABLED in gold
		#undef BIZ_ASSERT_SPEED_ENABLED
	#else
		#if defined(BIZ_ASSERT_SPEED_ENABLED) || defined(BIZ_ASSERT_SPEED_DISABLED)
			#if defined(BIZ_ASSERT_SPEED_ENABLED) && defined(BIZ_ASSERT_SPEED_DISABLED)
				#undef BIZ_ASSERT_SPEED_ENABLED // DISABLED trumps ENABLED
			#endif
		#else
			#define BIZ_ASSERT_SPEED_ENABLED // ENABLED by default in all other builds
		#endif
	#endif

	// BIZ_IMPL_HALT

	#if defined(BIZ_ASSERT_ENABLED) || !defined(CFG_GOLD)

		#if defined(PLATFORM_PC)
			#define BIZ_IMPL_HALT() do { __debugbreak(); } while(0)
		#elif defined(PLATFORM_SPU)
			extern bool spuAssert();
			#define BIZ_IMPL_HALT() do { spuAssert(); } while(0)
		#elif defined(PLATFORM_PS3)
			#define BIZ_IMPL_HALT() do { snPause(); } while(0)
		#else
			#define BIZ_IMPL_HALT() do { DebugBreak(); } while(0)
		#endif

	#else

		#define BIZ_IMPL_HALT() do { } while(0)

	#endif

	#if defined(BIZ_ASSERT_ENABLED)

		// BIZ_IMPL_ASSERT

		#if defined(PLATFORM_SPU)
			#define BIZ_IMPL_ASSERT(kind, result, expr, msg, ...) \
				do { if (!(result)) BIZ_HALT(); } while(0)
			
			#define BIZ_IMPL_ASSERT_FALSE(kind, expr, msg, ...) \
				do { BIZ_HALT(); } while(0)
		#else
			#define BIZ_IMPL_ASSERT(kind, result, expr, msg, ...) \
				do { if (!(result) && BizAssertReport((kind), (expr), __FILE__, __LINE__, (msg), ## __VA_ARGS__)) BIZ_HALT(); } while(0)
			
			#define BIZ_IMPL_ASSERT_FALSE(kind, expr, msg, ...) \
				do { if (BizAssertReport((kind), (expr), __FILE__, __LINE__, (msg), ## __VA_ARGS__)) BIZ_HALT(); } while(0)
		#endif

	#else // !BIZ_ASSERT_ENABLED

		#define BIZ_IMPL_ASSERT(result, exprText, msg, ...)	do { } while(0)
		#define BIZ_IMPL_ASSERT_FALSE(kind, expr, msg, ...)	do { } while(0)

	#endif // !BIZ_ASSERT_ENABLED

	#if defined(BIZ_ASSERT_SPEED_ENABLED)

		#if defined(PLATFORM_SPU)
			#define BIZ_IMPL_ASSERT_SPEED(kind, result, expr, msg, ...) \
				do { if (!(result)) BIZ_HALT(); } while(0)
			
			#define BIZ_IMPL_ASSERT_FALSE_SPEED(kind, expr, msg, ...) \
				do { BIZ_HALT(); } while(0)
		#else
			#define BIZ_IMPL_ASSERT_SPEED(kind, result, expr, msg, ...) \
				do { if (!(result) && BizAssertReport((kind), (expr), __FILE__, __LINE__, (msg), ## __VA_ARGS__)) BIZ_HALT(); } while(0)

			#define BIZ_IMPL_ASSERT_FALSE_SPEED(kind, expr, msg, ...) \
				do { if (BizAssertReport((kind), (expr), __FILE__, __LINE__, (msg), ## __VA_ARGS__)) BIZ_HALT(); } while(0)
		#endif

	#else

		#define BIZ_IMPL_ASSERT_SPEED(result, exprText, msg, ...)	do { } while(0)
		#define BIZ_IMPL_ASSERT_FALSE_SPEED(kind, expr, msg, ...)	do { } while(0)

	#endif

	// BizAssertReport

	bool BizAssertReport(BizAssertKind kind, const char* expr, const char* file, int line, const char* msg, ...);

#endif //MERCURY_CORE_BIZASSERT_H
