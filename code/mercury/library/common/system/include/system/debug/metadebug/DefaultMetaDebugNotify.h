#ifndef __SYSTEM_DEBUG_DEFAULTMETADEBUGNOTIFY_H__
#define __SYSTEM_DEBUG_DEFAULTMETADEBUGNOTIFY_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Debug/MetaDebug/MetaDebug.h>

namespace MetaDebug
{
	// C_DefaultMetaDebugNotify
	class C_DefaultMetaDebugNotify : public I_MetaDebugNotify
	{
		public:
			// filterLevel - any nodes with a debug level greater than this will not be stored
			// logOutputCount - writes log output to the game debug log when encountered for the first "logOutputCount" messages. If -1 is specified all messages will be output. 0 surpresses log messages.
			// logHeader - a message to print before logs are output. Special character %N will print the name of this MetaNode
			// storeLogs - whether to store debug log nodes at all
			C_DefaultMetaDebugNotify( DebugLevel::Level filterLevel=DebugLevel::Info, const S32 logMessageCount=0, const char* logMessage="", const char* logHeader="", const char* logFooter="", bool storeLogs=false );

			// callback to do something with debug data. returns whether node should be stored or not
			bool OnLoad(C_MetaNode* pNode) OVERRIDE;

		private:
			const Util::C_String FormatString(C_MetaNode* pNode, const Util::C_String& src) const;

		private:

			DebugLevel::Level m_FilterLevel;
			S32 m_LogMessageCount;
			bool m_StoreLogs;
			Util::C_String m_LogMessage;
			Util::C_String m_LogHeader;
			Util::C_String m_LogFooter;
	};

} // namespace MetaDebug

#endif
