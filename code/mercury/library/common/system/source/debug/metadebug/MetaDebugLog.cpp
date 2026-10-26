#include "SystemPch.h"

#include <system/serialise/I_File.h>
#include <system/serialise/BinarySerialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>

#include <system/Debug/Log/DebugLog.h>
#include <system/debug/log/Filter.h>
#include <system/debug/log/Format.h>

#include <system/debug/metadebug/MetaDebug.h>
#include <system/debug/metadebug/MetaDebugLog.h>

#include <system/debug/metadebug/MetaDebugRender.h>


namespace MetaDebug
{

extern Debug::Logger &g_Log;

// 	C_MetaLogSubscriber

C_MetaLogSubscriber::C_MetaLogSubscriber( C_Log* pMetaLog )
	: m_pMetaLog(pMetaLog)
{
}

C_MetaLogSubscriber::~C_MetaLogSubscriber()
{
}

void C_MetaLogSubscriber::Log(Debug::LogEvent *pEvent)
{
	if (!m_pFilter->Filter(pEvent))
		return;

	if(m_pMetaLog == NULL)
		return;

	Util::C_String message = m_pFormatter->Format(pEvent);

	const char* str = message.Chars();

	m_pMetaLog->LogMessage(str, pEvent->level);

}


// C_LogMessage

const Util::C_String& C_LogMessage::GetMessage() const
{
	return m_Message;
}

DebugLevel::Level C_LogMessage::GetDebugLevel() const
{
	return m_DebugLevel;
}

// C_Log

C_Log::C_Log()
{

}

C_Log::C_Log( const Util::C_String& name, const DebugLevel::Level debugLevel )
	: C_MetaNode(name, debugLevel)
{

}

C_Log::~C_Log()
{

}

E_Type C_Log::GetType() const 
{
	return E_Type_Log;
}

S32 C_Log::GetMessageCount() const
{
	return m_MessageList.Size();
}

const C_LogMessage* C_Log::GetMessageByIndex(S32 i) const
{
	return &m_MessageList[i];
}

void C_Log::LogMessage( const Util::C_String& message, const DebugLevel::Level debugLevel )
{
	m_Mutex.Wait();
	m_MessageList.PushBack();
	C_LogMessage& newMessage = m_MessageList[ m_MessageList.Size() - 1 ];
	newMessage.m_Message = message;
	newMessage.m_DebugLevel = debugLevel;
	m_Mutex.Release();
}

void C_Log::ReadData(  Sys::I_File* pFile )
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

	ar >> m_MessageList;
}

void C_Log::WriteData(  Sys::I_File* pFile ) const
{
	Sys::BinarySerialiserAutoEndian ar(*pFile);

	ar << m_MessageList;
}

void C_Log::DumpData(const Util::C_String& prefix ) const
{
	const S32 messageCount = GetMessageCount();
	for(S32 i=0; i<messageCount; i++)
	{
		const C_LogMessage* pMessage = GetMessageByIndex(i);
		LOGDEBUG(g_Log, "%s   [%d]{%s}%s\n", prefix.Chars(), i, Debug::g_pszDebugLevelNames[pMessage->GetDebugLevel()], pMessage->GetMessage().Chars());
	}
}

} // namespace MetaDebug

