//
//
//
#include "stdafx.h"


#ifdef CFG_DEBUG
//#define ENABLE_MEMORY_TRACKING
#endif

#if defined( ENABLE_MEMORY_TRACKING )

#define MEMORY_TRACKER_TAG_ALLOCATION 1
#define MEMORY_TRACKER_CALLSTACK 1
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 1
#define MEMORY_TRACKER_BLOCK_LOCATIONS 1

#else

#define MEMORY_TRACKER_TAG_ALLOCATION 0
#define MEMORY_TRACKER_CALLSTACK 0
#define MEMORY_TRACKER_ENUMERATE_ALLOCATIONS 0
#define MEMORY_TRACKER_BLOCK_LOCATIONS 0

#endif

#include <minmax.h>
#include <gdiplus.h>

#include <system/memory/MemoryTrackerDefines.h>
#include <system/memory/MemoryTracker.h>
#include <system/memory/AllocatorFunctions.h>
#include <system/debug/log/DebugLog.h>
#include <wx/config.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <System/Perf/ProfilerBackend.h>
#include <system/artery/Artery.h>

void* operator new[]( size_t size ) { return Memory::TrackedAlignedAlloc(size, 16); }
void operator delete[]( void* ptr ) { Memory::TrackedAlignedFree(ptr); }
void * operator new (size_t size) { return Memory::TrackedAlignedAlloc(size,16); }
void operator delete(void *mem) { Memory::TrackedAlignedFree(mem); }

#include <System/DebugLog.h>
#include <system/debug/log/Filter.h>
#include <system/debug/log/Format.h>
#include <System/Debug/Log/Subscribers/ConsoleSubscriber.h>

#include <animation/tools/CompressedCurveUtils.h>
#include "../PreviewState.h"
#include "../Db.h"

#include "Frame.h"

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
#if !defined(__WXMSW__) && !defined(__WXPM__)
    #include "MotionWorks.xpm"
#endif

// ----------------------------------------------------------------------------
// private classes
// ----------------------------------------------------------------------------

namespace Debug
{

class WxLogSubscriber: public Debug::SubscriberCommon
{
public:
	DECLARE_SUBSCRIBER(WxLogSubscriber);

	WxLogSubscriber()
	{
		m_pFormatter->SetFormatString(Util::C_String("%c"));
	}

	virtual void Log(Debug::LogEvent* pEvent)
	{
		if (m_pFilter->Filter(pEvent))
		{
			Util::C_String msg = m_pFormatter->Format(pEvent);

			switch (pEvent->level)
			{
				case DebugLevel::None:
					wxLogInfo(msg.Chars());
					break;
				case DebugLevel::Fatal:
					//wxLogFatalError(msg.Chars());
					wxLogError(msg.Chars());
					break;
				case DebugLevel::Error:
					wxLogError(msg.Chars());
					break;
				case DebugLevel::Warn:
					wxLogWarning(msg.Chars());
					break;
				case DebugLevel::Info:
					wxLogInfo(msg.Chars());
					break;
				case DebugLevel::Debug:
					wxLogInfo(msg.Chars());
					break;
			}
		}

	}

	virtual void LogString(const Util::C_String &string)
	{
	}

};

IMPLEMENT_SUBSCRIBER(WxLogSubscriber, "WxLogSubscriber");

}



// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    // override base class virtuals
    // ----------------------------

    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();
    
    virtual int OnExit();
    
private:
	C_AnimationGraphEditor* m_animGraphEditor;
	
	ULONG_PTR	m_gdiplusToken;
};



// Create a new application object: this macro will allow wxWidgets to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also implements the accessor function
// wxGetApp() which will return the reference of the right type (i.e. MyApp and
// not wxApp)
IMPLEMENT_APP(MyApp)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

namespace Maths
{
	extern const F32 g_Const_TanSeries[16];
	extern const F32 g_Const_ArcSinSeries[16];
	extern const F32 g_Const_ArcTanSeries[16];
/*
	const F32 g_Const_TanSeries[16] =
	{
		1.f,
		0.333333f,
		0.133333f,
		0.0539683f,
		0.0218695f,
		0.00886324f,
		0.00359213f,
		0.00145583f,
		0.000590027f,
		0.000239129f,
		9.69154e-005f,
		3.92783e-005f,
		1.59189e-005f,
		6.45169e-006f,
		2.61477e-006f,
		0.f
	};
*/

}

// 'Main program' equivalent: the program execution "starts" here
bool MyApp::OnInit()
{
    // call the base class initialization method, currently it only parses a
    // few common command-line options but it could be do more in the future
    if ( !wxApp::OnInit() )
        return false;

#if 0
	::OutputDebugString("Tan\n");
	for (U32 i = 0; i < 16; ++i)
	{
		::OutputDebugString(wxString::Format("%.25ff,\n", Maths::g_Const_TanSeries[i]).c_str());
	}
	::OutputDebugString("ArcSin\n");
	for (U32 i = 0; i < 16; ++i)
	{
		::OutputDebugString(wxString::Format("%.25ff,\n", Maths::g_Const_ArcSinSeries[i]).c_str());
	}
	::OutputDebugString("ArcTan\n");
	for (U32 i = 0; i < 32; ++i)
	{
		::OutputDebugString(wxString::Format("%.25ff,\n", Maths::g_Const_ArcTanSeries[i]).c_str());
	}
#endif
	wxInitAllImageHandlers();
        
	Gdiplus::GdiplusStartupInput	gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
    
	Debug::GetRootLogger().AddSubscriber( new Debug::ConsoleSubscriber() );
	Debug::GetRootLogger().AddSubscriber( new Debug::WxLogSubscriber );
	Debug::GetRootLogger().SetLevel(::DebugLevel::Debug);

	Aftk::C_Global::Create();

	ArteryStartup(true, "motionworks");
	
	Debug::NetworkStart();
	
	// Enable callstack capturing (slow).
	//MEMORY_TRACKER_ENABLE_CALLSTACK_CAPTURE;
	
	// Initialise Perfview service.
	Perf::InitialiseNetworkConnection();

	// Copy DB template into user's documents folder.
	wxFileName dataDir = wxFileName::DirName( wxStandardPaths::Get().GetUserDataDir() );
	LOGINFO( Debug::GetLogger("motionworks"), "User data dir: %s", dataDir.GetFullPath() );
	wxString dbDir = dataDir.GetPathWithSep() + "db";
		
	C_XtDatabase::CopyDbTemplate( "./db", dbDir.c_str() );
	
	// Database.
	C_XtDatabase::Instance().Open( dbDir.c_str() );
	
	// State.
	m_animGraphEditor = new C_AnimationGraphEditor( C_XtDatabase::Instance() );
	gClipPreviewState = new C_ClipPreviewState( C_XtDatabase::Instance(), *m_animGraphEditor );
	
	// Create config for writing registry settings.
	wxConfig::Set( new wxConfig( _T("Motionworks"), _T("Bizarre Creations") ) );
		
    // create the main application window
	gFrame = new C_Frame(_T("MotionWorks"), C_XtDatabase::Instance(), *m_animGraphEditor );

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    gFrame->Show(true);
    
    
	//WSACleanup();

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned false here, the
    // application would exit immediately.
    return true;
}

int MyApp::OnExit()
{
	gFrame = 0;

	delete gClipPreviewState;
	gClipPreviewState = 0;
	
	delete m_animGraphEditor;
	m_animGraphEditor = NULL;
	
	C_XtDatabase::Instance().Commit();
	C_XtDatabase::Instance().Close();

    ArteryCleanup();
    
	Gdiplus::GdiplusShutdown(m_gdiplusToken);   

	return wxApp::OnExit();
}

void RefreshDisplayWindow()
{
	if ( gFrame )
	{
		gFrame->RequestDisplayUpdate();
	}

}