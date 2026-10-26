#include "SystemPch.h"

// generic storage for debug metadata
#include <system/serialise/I_File.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/MetaDebugAttributeList.h>
#include <system/debug/metadebug/MetaDebugLog.h>
#include <system/debug/metadebug/MetaDebugRender.h>
#include <system/debug/metadebug/DefaultMetaDebugNotify.h>

namespace MetaDebug
{

extern Debug::Logger &g_Log;

C_DefaultMetaDebugNotify::C_DefaultMetaDebugNotify( DebugLevel::Level filterLevel, const S32 logMessageCount, const char* logMessage, const char* logHeader, const char* logFooter, bool storeLogs )
	: m_FilterLevel(filterLevel)
	, m_LogMessageCount(logMessageCount)
	, m_LogMessage(logMessage)
	, m_LogHeader(logHeader)
	, m_LogFooter(logFooter)
	, m_StoreLogs(storeLogs)
{
}

const Util::C_String C_DefaultMetaDebugNotify::FormatString(C_MetaNode* pNode, const Util::C_String& src) const
{
	return src.Replace("%n", pNode->GetFullName());
}

// callback to do something with debug data. returns whether node should be stored or not
bool C_DefaultMetaDebugNotify::OnLoad(C_MetaNode* pNode)
{
	switch(pNode->GetType())
	{
		case E_Type_Node:
			break;

		case E_Type_AttributeList:
			break;

		case E_Type_DebugRender:
			break;

		case E_Type_Log:
			if(m_LogMessageCount != 0)
			{
				C_Log* pLog = (C_Log*)pNode;

				S32 messageCount = pLog->GetMessageCount();

				if(m_LogMessageCount != -1)
				{
					messageCount = Maths::Min(messageCount, m_LogMessageCount);
				}

				if(messageCount > 0)
				{
					Util::C_String logHeader = FormatString(pNode, m_LogHeader);
					Util::C_String logFooter = FormatString(pNode, m_LogFooter);
					Util::C_String logMessage = FormatString(pNode, m_LogMessage);

					LOGINFO(g_Log, "%s", logHeader.Chars() );

					for(S32 i=0; i<messageCount; i++)
					{
						const C_LogMessage* pLogMessage = pLog->GetMessageByIndex(i);

						Util::C_String message = logMessage.Replace( "%c", pLogMessage->GetMessage().Chars() );

						switch(pLogMessage->GetDebugLevel())
						{
							case DebugLevel::Debug:
								LOGDEBUG( g_Log, "%s", message.Chars() );
								break;
							case DebugLevel::Info:
								LOGINFO( g_Log, "%s", message.Chars() );
								break;
							case DebugLevel::Warn:
								LOGWARN( g_Log, "%s", message.Chars() );
								break;
							case DebugLevel::Error:
								LOGERROR( g_Log, "%s", message.Chars() );
								break;
							case DebugLevel::Fatal:
								LOGERROR( g_Log, "%s", message.Chars() );
								break;
						}
					}

					LOGINFO(g_Log, "%s", logFooter.Chars() );
				}
			}

			return m_StoreLogs;
			break;
	}

	if(pNode->GetDebugLevel() <= m_FilterLevel)
		return true;
	
	return false;
}

} // namespace MetaDebug