#ifndef DEBUG_SYSTEMS_H
#define DEBUG_SYSTEMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/DebugLog.h>
#include <system/Debug/Log/subscribers/SubscriberCommon.h>

namespace Render { namespace Font {class C_FontRenderer;}};

namespace DebugSystems
{
	const int DEBUG_LOG_BUFFER_COUNT	= 800;
	const int DEBUG_LOG_BUFFER_LENTH	= 80;

	typedef Util::C_FixedString<DEBUG_LOG_BUFFER_LENTH>	MessageString;

	struct C_DebugHUDLogMessages
	{
		MessageString				m_Msg[DEBUG_LOG_BUFFER_COUNT];
		U8							m_Level[DEBUG_LOG_BUFFER_COUNT];
		U16							m_FirstLogMessage;
		U16							m_NextLogMessage;
	};

	class C_DebugHUDSubscriber : public Debug::SubscriberCommon
	{
		Render::Font::C_FontRenderer	*m_pFontRenderer;
		
		//Ring buffer
		C_DebugHUDLogMessages			m_LogMessages;
		
		MT::Mutex						m_Mutex;

		//Log Render
		S16								m_ScreenMaxMessageCount;
		bool							m_containsMessages;
		Maths::C_TimeS					m_MessageTimeout;
		F32								m_ScrollBack;
		F32								m_ScrollTimeout;
		F32								m_ScrollReturnSpeed;
		
		S32								GetLogMessagesCount() const;
	public:
		C_DebugHUDSubscriber();
		~C_DebugHUDSubscriber();

		void							Render();
		void							Update(const Maths::C_TimeS& timeStep);
		
		void							ScrollUp();
		void							ScrollDown();

		virtual void					Log(Debug::LogEvent *pEvent);
		virtual void					LogString(const Util::C_String &string) {};
		//virtual void					SetXMLParameter(const char *pName, const char *pValue);
		//virtual bool					Start();

	private:
		int								AddMessageJnr(const char *str, DebugLevel::Level level, bool splitLine);

	};
}
#endif //DEBUG_SYSTEMS_H
