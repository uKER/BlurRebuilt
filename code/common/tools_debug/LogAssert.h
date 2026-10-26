#ifndef TU_LOG_ASSERT_H
#define TU_LOG_ASSERT_H

#include <string>
#include "DebugManager.h"

static char g_LogAssertAtring[] = "Assertion failed: %s, file %s, line %d\n";

namespace Debug
{
	class LogAssert
	{
	public:
		LogAssert()
		{
		#ifdef _DEBUG
			m_debugManager.SetDebugEnabledFlag(true);
			m_debugManager.SetConsoleEnabledFlag(true);
			m_debugManager.SetLogFileEnabledFlag(true);
		#else 
			m_debugManager.SetDebugEnabledFlag(true);
			m_debugManager.SetConsoleEnabledFlag(false);
			m_debugManager.SetLogFileEnabledFlag(true);
		#endif
		}

		~LogAssert()
		{
			m_debugManager.Destroy();
		}

		void Assert (
			const char *logFile,
			const char *expr,
			const char *filename,
			unsigned lineno)
		{
			m_debugManager.Create(logFile);

			//Write assert buffer to debug manager.
			m_debugManager << "Assertion failed: " << expr << ", file " << filename << ", line " << lineno << "\n";
		}

	private:
		Tu::DebugManager m_debugManager;
	};
}

#define DEBUG_LOG_ASSERT(exp) if (!(exp)) {Debug::LogAssert* pLogAssert = new Debug::LogAssert; pLogAssert->Assert("C:/assert.log",#exp, __FILE__, __LINE__); delete pLogAssert; _asm int 3}

#endif
