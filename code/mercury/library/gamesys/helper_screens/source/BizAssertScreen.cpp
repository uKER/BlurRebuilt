
#include <core/core.h>


#ifdef PLATFORM_PC
#define _WIN32_WINNT 0x0501 
#include <winsock2.h>
#include <windows.h>
#endif

#include <Core/Containers.h>
#include <Core/Threads/Thread.h>

#include <system/debugLog.h>
#include "system/automation/Automation.h"

#include <Display.h>
#include <framework/application/UserControls.h>
#include <Input/DeviceManager.h>
#include <Input/GameInput.h>
#include <System/Debug/log/DebugLog.h>

#include <Render/font/Font.h>

#include "BizAssertScreen.h"

//#define ENABLE_WRITING_CRASH_DUMPS

namespace helper_screens
{

#ifndef CFG_GOLD

Debug::Logger &gLog = Debug::GetLogger("mercury.system.assert");

Debug::Bool g_ShowAssertScreenWithDebugger("Debug.ShowAssertScreenWhenDebuggerAttached", false, Debug::Persist());
Debug::Bool g_IgnoreAsserts("Debug.IgnoreAsserts", false, Debug::Persist());

MT::Lock g_AssertLock;
bool g_isInAssert(false);

#endif

void RenderAssert(Render::Font::C_FontRenderer* fontRenderer, BizAssertKind kind, const char* expr, const char* msg, const char* file, int line)
{
#ifndef CFG_GOLD
	fontRenderer->SetScaleX(24.0f);
	fontRenderer->SetScaleY(32.0f);
	fontRenderer->SetShadowEnable(false);

	// TODO: Handle if we're inside a tiling section

	//displayDevice = ; // ->GetD3DDevice();

	Display::Device()->BeginScene();

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil, Maths::RGBA(0, 0, 0, 255));

	Maths::RGBA titleColour;

	switch (kind)
	{
	case kBizAssert_Warning:
		titleColour = Maths::RGBA(255, 165, 0, 255);
		break;

	case kBizAssert_Notice:
		titleColour = Maths::RGBA(0, 255, 0, 255);
		break;

	case kBizAssert_Panic:
		titleColour = Maths::RGBA(160, 32, 240, 255);
		break;

	case kBizAssert_Assert:
	default:
		titleColour = Maths::RGBA(255, 0, 0, 255);
	}

	fontRenderer->SetColourTop(titleColour);
	fontRenderer->SetColourBottom(titleColour);

	fontRenderer->SetScaleX(48.f);
	fontRenderer->SetScaleY(48.f);
	fontRenderer->Print(Maths::HVec2(512.f, 134.4f), GetBizAssertKindText(kind), Render::Font::E_Justification_Centre);

	fontRenderer->SetScaleX(24.f);
	fontRenderer->SetScaleY(24.f);

	fontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, 255));
	fontRenderer->SetColourBottom(Maths::RGBA(255, 255, 255, 255));

	if (!CoreDebug::IsDebuggerAttached())
	{
#if defined(PLATFORM_PS3)
		fontRenderer->Print(Maths::HVec2(512.f, 204.8f), "Press (X) to continue", Render::Font::E_Justification_Centre);
#else
		fontRenderer->Print(Maths::HVec2(512.f, 204.8f), "Press (A) to continue", Render::Font::E_Justification_Centre);

#ifdef ENABLE_WRITING_CRASH_DUMPS
#endif
#endif
	}

	if (file)
	{
		const char* pszFilename = file + strlen(file) - 1;

		for (; (pszFilename > file) && (*(pszFilename-1) != '\\') &&(*(pszFilename-1) != '/'); pszFilename--);

		Util::C_String text, tempBuf;

		tempBuf.CopyFrom(file, pszFilename - file);

		text = "Dir: " + tempBuf;
		fontRenderer->Print(Maths::HVec2(512.f, 281.6f), text, Render::Font::E_Justification_Centre);

		text.Printf("File: %s  Line: %d", pszFilename, line);
		fontRenderer->Print(Maths::HVec2(512.f, 368.f), text, Render::Font::E_Justification_Centre);
	}

	float yPos = 448.f;

	if ((msg != NULL) && (msg[0] != '\0'))
	{
		fontRenderer->SetColourTop(Maths::RGBA(184, 134, 11, 255));
		fontRenderer->SetColourBottom(Maths::RGBA(184, 134, 1, 255));

		fontRenderer->Print(Maths::HVec2(512.f, yPos), msg, Render::Font::E_Justification_Centre);

		yPos += 64.f;
	}

	if ((expr != NULL) && (expr[0] != '\0'))
	{
		fontRenderer->SetColourTop(Maths::RGBA(255, 255, 0, 255));
		fontRenderer->SetColourBottom(Maths::RGBA(255, 255, 0, 255));

		fontRenderer->Print(Maths::HVec2(512.f, yPos), expr, Render::Font::E_Justification_Centre);
	}

	// Display screen
	Display::Device()->EndScene();
	Display::Device()->Present();
#endif
}

bool BizAssertHandler(BizAssertKind kind, const char* expr, const char* msg, const char* file, int line)
{
#ifndef CFG_GOLD
	g_AssertLock.Enter();

	// Check if we've recursively called the Assert handler
	if (g_isInAssert)
	{
		LOGWARN(gLog, "*** DETECTED RECURSIVE ASSERT - Dropping out");
		g_AssertLock.Exit();
		return true;
	}

	g_isInAssert = true;

	LOGERROR(gLog, "%s(%i): %s%s%s%s%s", file, line,
			GetBizAssertKindText(kind),
			*expr ? " : " : "",	expr,
			*msg ? " : " : "",	msg
		);

	// TODO: We need to keep a log of the last couple of asserts that were skipped (can store file/line info) in case people skip some then crash the game

	// TODO: Add way to save crash dump out at this point for debugging

	if (Automation::IsAutomationInProgress())
	{
		Automation::Assert(expr, file, line, "", msg);
	}

	if (g_IgnoreAsserts)
	{
		LOGWARN(gLog, "******************************");
		LOGWARN(gLog, " IGNORE ASSERTS IS ENABLED!!!!");
		LOGWARN(gLog, "******************************");
		
		g_isInAssert = false;
		g_AssertLock.Exit();
		return false;
	}

	bool forceDebuggerToBreak = (kind == kBizAssert_Assert) || (kind == kBizAssert_Panic);
	bool displayScreen = g_ShowAssertScreenWithDebugger || (!CoreDebug::IsDebuggerAttached()) || (kind == kBizAssert_Notice) || (kind == kBizAssert_Warning);

	// 1 - Assert/Panic + Debugger - Break (no show)
	// 2 - Assert/Panic - Screen (Break if we attach)
	// 3 - Other - Screen (Don't break)

	Render::Font::C_FontRenderer* fontRenderer = NULL;

	// Display Assert screen
	if (displayScreen && Render::Font::FontSystem())
	{
		// TODO: Try and get the D3D lock - if not immediately we spin for 10secs trying to get it! If not, we have a deadlock

		if (!Display::Device()->AcquireThreadOwnership(2 * 1000))
		{
			LOGERROR(gLog, "BizAssertHandler - Deadlock - Couldn't aquire ownership of Device to render screen");

			g_isInAssert = false;
			g_AssertLock.Exit();
			return true;
		}

		fontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
		if (fontRenderer)
		{
			RenderAssert(fontRenderer, kind, expr, msg, file, line);
		}

		// If no debugger attached, we wait for controller input before we carry on
		//if (!forceDebuggerToBreak)
		{
			Input::C_DeviceManager::GetInstance()->Update();
			Input::C_GameInputManager::GetInstance()->Update();

			while (!Input::GetGameInput(0)->GetButtonPressedDebounced(Framework::E_Button_Debug_ButtonA))
			{
				Input::C_DeviceManager::GetInstance()->Update();
				Input::C_GameInputManager::GetInstance()->Update();

				// If someone attaches a debugger then we stop waiting for a button press
				if (CoreDebug::IsDebuggerAttached() && forceDebuggerToBreak)
				{
					LOGERROR(gLog, "%s(%i): %s%s%s%s%s", file, line,
							GetBizAssertKindText(kind),
							*expr ? " : " : "",	expr,
							*msg ? " : " : "",	msg
						);

					delete fontRenderer;

					Display::Device()->ReleaseThreadOwnership();

					g_isInAssert = false;
					g_AssertLock.Exit();
					return forceDebuggerToBreak;
				}

				MT::Thread::Sleep(13);

				if (fontRenderer)
					RenderAssert(fontRenderer, kind, expr, msg, file, line);

#ifdef ENABLE_WRITING_CRASH_DUMPS
#endif

#ifdef PLATFORM_PC
				// Process windows messages while the screen is up else it'll go white and unreadable
				MSG msg;
				while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);

					if (msg.message == WM_QUIT)
					{
						PostQuitMessage(msg.wParam);
						Display::Device()->ReleaseThreadOwnership();
						goto drop_out;
					}
				}
#endif
			}

			forceDebuggerToBreak = false;
		}

		Display::Device()->ReleaseThreadOwnership();
	}
	else
		forceDebuggerToBreak = true;

#ifdef PLATFORM_PC
drop_out:
	delete fontRenderer;
#endif

	g_isInAssert = false;
	g_AssertLock.Exit();
	return forceDebuggerToBreak;
#else
	return false;
#endif
}

class LogFatalSubscriber : public Debug::SubscriberCommon
{
public:
	DECLARE_SUBSCRIBER(LogFatalSubscriber);

	LogFatalSubscriber() {}
	~LogFatalSubscriber() {}

	virtual void Log(Debug::LogEvent *pEvent)
	{
		if ((pEvent->level == ::DebugLevel::Fatal) && helper_screens::BizAssertHandler(kBizAssert_Panic, "LOGFATAL", pEvent->msg.Chars(), pEvent->location.m_FileName.Chars(), pEvent->location.m_LineNumber))
		{
			BIZ_HALT();
		}
	}

	virtual void LogString(const Util::C_String &string)
	{
	}
};

IMPLEMENT_SUBSCRIBER(LogFatalSubscriber, "logfatal");

#if defined(BIZ_ASSERT_ENABLED) || defined(BIZ_ASSERT_SPEED_ENABLED)

// Wrapper for CoreAssert to use the BIZ_ASSERT visual handler to put asserts on the screen
bool CoreAssertWrapper(const char *pszFunction, const char *pszExpression, const char *pszFilename, int iLineNumber)
{
	if (helper_screens::BizAssertHandler(kBizAssert_Assert, pszExpression, "", pszFilename, iLineNumber))
	{
		BIZ_HALT();
	}

	return true;
}

#endif

#ifndef CFG_GOLD
LogFatalSubscriber* m_pLogFatalSubscriber = NULL;
#endif

void InitialiseVisualAssertScreen(bool enableLogFatalCapture)
{
#if defined(BIZ_ASSERT_ENABLED) || defined(BIZ_ASSERT_SPEED_ENABLED)
	::g_pCoreAssertFunction = CoreAssertWrapper;
	
	SetBizAssertHandler(helper_screens::BizAssertHandler);
#endif

#ifndef CFG_GOLD
	if (enableLogFatalCapture)
	{
		m_pLogFatalSubscriber = new LogFatalSubscriber();
		Debug::GetRootLogger().AddSubscriber(m_pLogFatalSubscriber);
	}
#endif
}

void ShutdownVisualAssertScreen()
{
#ifndef CFG_GOLD
	if (m_pLogFatalSubscriber)
	{
		Debug::GetRootLogger().RemoveSubscriber(m_pLogFatalSubscriber);
		delete m_pLogFatalSubscriber;

		m_pLogFatalSubscriber = NULL;
	}
#endif
}

}
