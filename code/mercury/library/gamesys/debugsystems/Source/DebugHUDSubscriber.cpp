#include "DebugSystemsPch.h"

#include "DebugSystems/DebugHUDSubscriber.h"
#include "DebugSystems/DebugSystemsCrashInfo.h"
#include <Render/font/Font.h>
#include <Render/Debug/DebugRender.h>
#include <system/debug/log/DebugLog.h>
#include <system/Debug/log/Filter.h>
#include <system/Debug/log/Format.h>

Debug::Float			g_DebugHudSubscriberWindowHeight( "GameSys.DebugHUD.Subscriber.WindowHeight", 128.0f );
Debug::Float			g_DebugHudSubscriberWindowWidth( "GameSys.DebugHUD.Subscriber.WindowWidth", 1024.0f );
Debug::Float			g_DebugHudSubscriberFirstMessageTimeout("GameSys.DebugHUD.Subscriber.FirstMessageTimeout", 5);
Debug::Float			g_DebugHudSubscriberMessageTimeout("GameSys.DebugHUD.Subscriber.MessageTimeout", 1);
Debug::Float			g_DebugHudScrollTimeout("GameSys.DebugHUD.Subscriber.ScrollTimeout", 3);
Debug::Float			g_DebugHudScrollReturnAccel("GameSys.DebugHUD.Subscriber.ScrollReturnAccel", 20.0f);
Debug::Float			g_DebugHudScrollReturnMaxAccel("GameSys.DebugHUD.Subscriber.ScrollReturnMaxAccel", 2000.0f);
Debug::Bool				g_DebugHudRender("GameSys.DebugHUD.Subscriber.Render", true, Debug::Persist() );
Debug::Bool				g_DebugHudHideInSpeedBuild("GameSys.DebugHUD.Subscriber.HideInSpeedBuild", false, Debug::Persist() );
Debug::String			g_DebugHudLevelFilter("GameSys.DebugHUD.Subscriber.LevelFilter", "undefined|Fatal|Error|Warn|Info|Debug");
Debug::Int				g_DebugHudErrorLevel("GameSys.DebugHUD.Subscriber.ErrorLevel", DebugLevel::Info);
Debug::Bool				g_DebugHudShowStreamPane("GameSys.DebugHUD.Render.Streams", true );

namespace DebugSystems
{

#ifndef CFG_GOLD

// Crash structure used to help people debugging get to useful information quickly
CrashInfo_s CrashInfo = { 0 };

#endif

C_DebugHUDSubscriber::C_DebugHUDSubscriber()
{
#ifndef CFG_GOLD
	CrashInfo.debugOutput = &m_LogMessages;
#endif

	m_pFontRenderer = NULL;
	m_LogMessages.m_FirstLogMessage = 0;
	m_LogMessages.m_NextLogMessage = 0;
	m_containsMessages = false;

	m_MessageTimeout = 0.0f;
	m_ScreenMaxMessageCount = 3;
	m_ScrollBack = 0;
	m_ScrollTimeout = 0.0f;
	m_ScrollReturnSpeed = 0.0f;
}

C_DebugHUDSubscriber::~C_DebugHUDSubscriber()
{
	SAFE_DELETE_PTR(m_pFontRenderer);
}

int C_DebugHUDSubscriber::AddMessageJnr(const char *Str, DebugLevel::Level level, bool splitLine)
{
	m_LogMessages.m_Level[m_LogMessages.m_NextLogMessage] = level;

	if (!splitLine)
	{
		m_LogMessages.m_Msg[m_LogMessages.m_NextLogMessage] = Str;
	}
	else
	{
		m_LogMessages.m_Msg[m_LogMessages.m_NextLogMessage] = "->";
		m_LogMessages.m_Msg[m_LogMessages.m_NextLogMessage] += Str;
	}

	if (m_LogMessages.m_NextLogMessage == m_LogMessages.m_FirstLogMessage && m_containsMessages)
	{
		m_LogMessages.m_NextLogMessage = m_LogMessages.m_FirstLogMessage = (m_LogMessages.m_NextLogMessage + 1) % DEBUG_LOG_BUFFER_COUNT;
	}
	else
	{
		m_LogMessages.m_NextLogMessage = (m_LogMessages.m_NextLogMessage + 1) % DEBUG_LOG_BUFFER_COUNT;
		m_containsMessages = true;
	}

	int nLength = strlen(Str);
	if (!splitLine)
		return (nLength <= DEBUG_LOG_BUFFER_LENTH-1) ? 0 : DEBUG_LOG_BUFFER_LENTH-1;
	else
		return (nLength <= DEBUG_LOG_BUFFER_LENTH-3) ? 0 : DEBUG_LOG_BUFFER_LENTH-3;
}

void C_DebugHUDSubscriber::Log( Debug::LogEvent *pEvent )
{
	if (pEvent->level > g_DebugHudErrorLevel)
		return;
	if (!m_pFilter->Filter(pEvent))
		return;
		
	m_Mutex.Wait();

	Util::C_String message = m_pFormatter->Format(pEvent);

	const char* str = message.Chars();
	bool splitLine(false);

	// If the line run's over, split it up
	while (int nInc = AddMessageJnr(str, pEvent->level, splitLine))
	{
		str += nInc;
		splitLine = true;
	}

	m_MessageTimeout = g_DebugHudSubscriberFirstMessageTimeout;

	m_ScreenMaxMessageCount++;
	if(m_ScreenMaxMessageCount > 3)
	{
		m_ScreenMaxMessageCount = 3;
	}

	m_Mutex.Release();
}


const Maths::RGBA g_FontColoursTop[6] = 
{
	Maths::RGBA(0xff, 0xff, 0xff, 0xff),//None	= 0,
	Maths::RGBA(0xff, 0xf0, 0xf0, 0xff),//Fatal	= 1,
	Maths::RGBA(0xff, 0xf0, 0xf0, 0xff),//Error	= 2,
	Maths::RGBA(0xff, 0xf0, 0x80, 0xff),//Warn	= 3,
	Maths::RGBA(0xff, 0xff, 0xff, 0xff),//Info	= 4,
	Maths::RGBA(0x80, 0x80, 0x80, 0xff)//Debug	= 5,
};

const Maths::RGBA g_FontColoursBottom[6] = 
{
	Maths::RGBA(0xff, 0xff, 0xff, 0xff),//None	= 0,
	Maths::RGBA(0xff, 0x00, 0x00, 0xff),//Fatal	= 1,
	Maths::RGBA(0xff, 0x00, 0x00, 0xff),//Error	= 2,
	Maths::RGBA(0xff, 0x80, 0x00, 0xff),//Warn	= 3,
	Maths::RGBA(0x80, 0x80, 0xb0, 0xff),//Info	= 4,
	Maths::RGBA(0x80, 0x80, 0x80, 0xff)//Debug	= 5,
};

const Maths::RGBA g_BarColours[6] = 
{
	Maths::RGBA(0x10, 0x10, 0x10, 0xa0),//None	= 0,
	Maths::RGBA(0x30, 0x10, 0x10, 0xa0),//Fatal	= 1,
	Maths::RGBA(0x30, 0x10, 0x10, 0xa0),//Error	= 2,
	Maths::RGBA(0x20, 0x20, 0x10, 0xa0),//Warn	= 3,
	Maths::RGBA(0x10, 0x10, 0x10, 0xa0),//Info	= 4,
	Maths::RGBA(0x10, 0x10, 0x10, 0xa0)//Debug	= 5,
};


void C_DebugHUDSubscriber::Update(const Maths::C_TimeS& initialTimeStep)
{
	m_Mutex.Wait();
	
	Maths::C_TimeS timeStep = initialTimeStep;
	if( timeStep > (1.0f / 15.0f) )
	{
		timeStep = Maths::C_TimeS(1.0f / 15.0f);
	}


	//Timeout update
	m_MessageTimeout -= timeStep;
	if(m_MessageTimeout < 0)
	{
		m_ScreenMaxMessageCount--;
		if(m_ScreenMaxMessageCount < 0)
		{
			m_ScreenMaxMessageCount = 0;
		}
		m_MessageTimeout = g_DebugHudSubscriberMessageTimeout;
	}

	//scroll update
	m_ScrollTimeout -= timeStep;
	if(m_ScrollTimeout <= 0.0f)
	{
		m_ScrollTimeout = 0.0f;		

		m_ScrollBack -= m_ScrollReturnSpeed * timeStep;
		m_ScrollReturnSpeed += g_DebugHudScrollReturnAccel * timeStep;
		if(m_ScrollReturnSpeed > g_DebugHudScrollReturnMaxAccel)
		{
			m_ScrollReturnSpeed = g_DebugHudScrollReturnMaxAccel;
		}

		if(m_ScrollBack < 0)
		{
			m_ScrollBack = 0.0f;
			m_ScrollReturnSpeed = 0.0f;
		}
	}

	//Update FilterString
	Util::C_String filterString = g_DebugHudLevelFilter.Value();
	m_pFilter->SetFilterString(filterString);

	m_Mutex.Release();
}

void C_DebugHUDSubscriber::ScrollUp()
{
	m_Mutex.Wait();

	m_MessageTimeout = g_DebugHudSubscriberFirstMessageTimeout;

	m_ScrollTimeout = g_DebugHudScrollTimeout;
	m_ScrollReturnSpeed = 0.0f;
	if( m_ScrollBack < GetLogMessagesCount() )
	{
		m_ScrollBack++;
	}

	m_Mutex.Release();
}

void C_DebugHUDSubscriber::ScrollDown()
{
	m_Mutex.Wait();

	m_MessageTimeout = g_DebugHudSubscriberFirstMessageTimeout;

	m_ScrollTimeout = g_DebugHudScrollTimeout;
	m_ScrollReturnSpeed = 0.0f;
	if( m_ScrollBack > 0 )
	{
		m_ScrollBack--;
	}

	m_Mutex.Release();
}

void C_DebugHUDSubscriber::Render()
{
#ifdef CFG_SPEED
	if(g_DebugHudHideInSpeedBuild)
		return;
#endif

	if(!g_DebugHudRender)
		return;
	

	if(!g_DebugHudShowStreamPane)
		return;

	PERF_EVENT("Render DebugHUDSubscriber");
	m_Mutex.Wait();

	if (m_pFontRenderer == NULL)
	{
		//Create the font renderer
		m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		if (m_pFontRenderer == NULL)	// [Blur2PC] the debug font is not in retail data; skip the pane
		{
			m_Mutex.Release();
			return;
		}
		m_pFontRenderer->SetScaleX(16);
		m_pFontRenderer->SetScaleY(24);
	}
	
	F32 windowHeight = g_DebugHudSubscriberWindowHeight;
	F32 wrapWidth = g_DebugHudSubscriberWindowWidth;

	S32 maxMessageCount = m_ScreenMaxMessageCount;
	if(maxMessageCount > 8)
	{
		maxMessageCount = 8;
	}

	
	S32 firstDisplayMessage = GetLogMessagesCount() - maxMessageCount - (S32)m_ScrollBack;
	S32 lastDisplayMessage = GetLogMessagesCount();

	if(firstDisplayMessage < 0)
		firstDisplayMessage = 0;
	
	if(firstDisplayMessage > GetLogMessagesCount())
		firstDisplayMessage = GetLogMessagesCount();

	if(lastDisplayMessage < 0)
		lastDisplayMessage = 0;

	if(lastDisplayMessage > GetLogMessagesCount())
		lastDisplayMessage = GetLogMessagesCount();

	Maths::HVec2 pos(48.0f, 32.0f);
	for(S32 index=firstDisplayMessage; index<lastDisplayMessage; index++)
	{
		int mindex = (index + m_LogMessages.m_FirstLogMessage) % DEBUG_LOG_BUFFER_COUNT;

		const DebugLevel::Level msgLevel = (DebugLevel::Level)m_LogMessages.m_Level[mindex];
		const MessageString& msgString = m_LogMessages.m_Msg[mindex];

		m_pFontRenderer->SetColourTop(g_FontColoursTop[msgLevel]);
		m_pFontRenderer->SetColourBottom(g_FontColoursBottom[msgLevel]);

		Maths::RGBA barCol = g_BarColours[msgLevel];

		Maths::HVec2 dim = m_pFontRenderer->GetDimensions((U32)wrapWidth, msgString.Chars());
		dim.Y( dim.Y() + 4.0f );
		
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		Render::Debug::Immediate()->Draw2dBar(pos, dim, barCol);
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);

		m_pFontRenderer->Print(pos + Maths::HVec2(0.0f, 2.0f), msgString.Chars(), Render::Font::E_Justification_Left, (U32)wrapWidth);
		pos.Y( pos.Y() + dim.Y());
	}
	m_Mutex.Release();
}

S32 C_DebugHUDSubscriber::GetLogMessagesCount() const
{
	if (m_LogMessages.m_FirstLogMessage == 0)
		return m_LogMessages.m_NextLogMessage - m_LogMessages.m_FirstLogMessage;
	else
		return DEBUG_LOG_BUFFER_COUNT;
}
}
