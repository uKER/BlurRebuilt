#ifndef __SYSTEM_DEBUG_METADEBUGLOG_H__
#define __SYSTEM_DEBUG_METADEBUGLOG_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Utility/string.h>
#include <btl/Containers/Vector.h>
#include <system/Debug/metadebug/MetaDebug.h>
#include <system/DebugLog.h>
#include <system/debug/log/subscribers/SubscriberCommon.h>
#include <system/debug/metadebug/MetaDebug.h>

namespace MetaDebug
{

class C_Log;

// so we can easily plug MetaLog into log system
class C_MetaLogSubscriber : public Debug::SubscriberCommon
{
public:
	C_MetaLogSubscriber( C_Log* pMetaLog );
	~C_MetaLogSubscriber();

	void					Log(Debug::LogEvent *pEvent) OVERRIDE;
	void					LogString(const Util::C_String &string) OVERRIDE {};

public:

	C_Log* m_pMetaLog;
};

class C_LogMessage
{
friend class C_Log;
public:
	const Util::C_String& GetMessage() const;
	DebugLevel::Level GetDebugLevel() const;

	template <typename Archive>
	void Serialise(Archive& ar) const;

	template <typename Archive>
	void Deserialise(Archive& ar);

private:
	Util::C_String m_Message;
	DebugLevel::Level m_DebugLevel;
};


// C_Log
class C_Log : public C_MetaNode
{
	public:
		C_Log();
		C_Log( const Util::C_String& name, const DebugLevel::Level debugLevel );
		~C_Log();

		S32 GetMessageCount() const;
		const C_LogMessage* GetMessageByIndex(S32 i) const;
		inline const C_LogMessage* operator[](S32 index) const
		{
			return GetMessageByIndex(index);
		}

		void LogMessage( const Util::C_String& message, const DebugLevel::Level debugLevel );
		void ReadData(  Sys::I_File* pFile ) OVERRIDE;
		void WriteData(  Sys::I_File* pFile ) const OVERRIDE;

		E_Type GetType() const OVERRIDE;

		void DumpData(const Util::C_String& prefix ) const OVERRIDE;

	private:
		MT::Mutex m_Mutex;
		Ut::Vector<C_LogMessage> m_MessageList;
};


template <typename Archive>
void C_LogMessage::Serialise(Archive& ar) const
{
	ar << (U8)m_DebugLevel;
	ar << m_Message;
}

template <typename Archive>
void C_LogMessage::Deserialise(Archive& ar)
{
	U8 debugLevel;
	ar >> debugLevel;
	m_DebugLevel = (DebugLevel::Level)debugLevel;
	ar >> m_Message;
}


} // namespace MetaDebug


#endif
