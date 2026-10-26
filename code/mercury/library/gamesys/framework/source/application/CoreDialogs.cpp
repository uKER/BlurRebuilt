#include "FrameworkPch.h"
#include <Core/Threads/Mutex.h>
#include <Core/Threads/Thread.h>
#include <System/Debug/Debug.h>
#include <System/serialise/FileSystemGlobals.h>

#include "Framework/Application/ApplicationBase.h"
#include "Framework/Application/CoreDialogs.h"

#ifdef PLATFORM_PS3

#include <sysutil/sysutil_msgdialog.h>

#define DIALOG_RESPONSE_NONE -1
#define DIALOG_RESPONSE_CLOSE 0

#endif

namespace Framework
{

Debug::Logger &g_CDLog( Debug::GetLogger("system.coredialogs") );

void CriticalFileErrorTestCallback(Debug::VarBase* pVar, void* pContext);

MT::Mutex g_dialogMutex;

PrimaryPadQueryCallback g_primaryPadQueryCallback = NULL;

Debug::Bool g_debugCriticalFileError("Test.DoNotUse.Dangerous.CriticalFileError", false, Debug::Notify(&CriticalFileErrorTestCallback));
Debug::Bool g_debugReadErrorDialog("Test.DoNotUse.Dangerous.EnableReadErrorDialogs", false);

#ifdef PLATFORM_PS3

void CriticalReadErrorDialogClosed(int buttonType, void *userData)
{
}

#endif


#ifdef PLATFORM_PS3

//[Ken] Does not now allow the app to invoke termination to avoid  failing TRC 079
//[R079] DG HG DGP HGP
//The title does not carry out termination processing except when the game termination
//request event is notified by the system.
//Build
//Test
//Explanation:
//For example, the following implementations are forbidden.
//- Providing an original termination method in the application
//- Automatic reboot by the application after a patch is applied
//- Automatic reboot by the application after game data is installed
//Even if the application does not itself initiate termination processing, it is also forbidden to request the user
//to terminate the game (except per the error handling methods described in R115, R124, R125, R132,
//R133, R154). For example, do not request the user to reboot the application after installing game data.

void DoCriticalReadErrorDialogPS3(const char* fileName)
{
	g_dialogMutex.Wait();

	bool isRenderThread = Display::Device()->HaveThreadOwnership();

	// Wait to see if the main-thread is going to process the termination-request( Prefered Method )
	sys_timer_sleep( 5 );

	//1/20 of a second.
	U32 kDelay = 50000;

	//Wait five seconds before displaying a read-error dialog, just incase we're pending a disc ejection
	//notification. This avoids a read error dialog flashing up and disappearing on disc ejection to 
	//make things a little nicer.
	for (U32 i = 0; i < 100; ++i)
	{
		C_ApplicationBase::GetInstance()->PollPlatformCallbacks();
		sys_timer_usleep(kDelay);
	}

	//Steal render loop if in main thread.
	if (isRenderThread)
	{
		Display::Device()->AcquireThreadOwnership();
		bool inScene = Display::Device()->InScene();
		if (inScene) Display::Device()->EndScene();
	}
	
	//Fire up our error dialog
	U32 style = 
		CELL_MSGDIALOG_TYPE_SE_TYPE_ERROR | 
		CELL_MSGDIALOG_TYPE_SE_MUTE_OFF |
		CELL_MSGDIALOG_TYPE_BG_VISIBLE |
		CELL_MSGDIALOG_TYPE_BUTTON_TYPE_NONE |
		CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON |
		CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_OK |
		CELL_MSGDIALOG_TYPE_PROGRESSBAR_NONE;

	Btl::String msg = Sys::GetFileReadErrorString();

	cellMsgDialogOpen2( 
		style, 
		msg.Chars(), 
		&CriticalReadErrorDialogClosed, 
		NULL,
		NULL);

	//We never break out of this loop.
	while(true)
	{
		//Steal render loop if in main thread.
		if (isRenderThread)
		{
			Display::Device()->BeginScene();
			Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth | Display::E_Clear_Stencil);
			Display::Device()->EndScene();
			Display::Device()->Present();
			Display::RingBuffer()->EndFrame();
		}
		else
		{
			sys_timer_usleep(kDelay);
		}

		//Poll for response.
		C_ApplicationBase::GetInstance()->PollPlatformCallbacks();
	}
}

#endif

#ifdef PLATFORM_PC

void DoCriticalReadErrorDialogPC(const char* fileName)
{
	g_dialogMutex.Wait();

	//TODO
	bool isRenderThread = Display::Device()->HaveThreadOwnership();

	g_dialogMutex.Release();

	if (isRenderThread)
	{
		Display::Device()->ReleaseThreadOwnership();
		//Halt application with minimal cleanup immediately.
		C_ApplicationBase::GetInstance()->Shutdown(true);
	}

	//Halt application with minimal cleanup.
	C_ApplicationBase::GetInstance()->RequestShutdown(true);
}

#endif

void CriticalReadErrorDialog(const char* fileName, bool writing, bool isCritical, void* pUserData)
{
	FSERROR(g_CDLog, "CriticalReadErrorDialog: '%s' isCritical=%s, writing=%s", fileName, isCritical ? "true" : "false", writing ? "true" : "false");

	if (writing || !isCritical)
		return;

#ifdef CFG_GOLD
	bool doReadErrorDialog = true;
#else
	bool doReadErrorDialog = g_debugReadErrorDialog;
#endif

	if (!doReadErrorDialog)
	{
		BIZ_ASSERT_FAIL_SPEED("Read error for: %s\nError string %s.\nIn a MASTER build this would trigger a dirty disc notification\n.", fileName, Sys::GetFileReadErrorString().Chars());
		return;
	}

	LOGERROR(g_CDLog, "CriticalReadErrorDialog: '%s'", fileName);

#ifdef ENABLE_FS_SPAM
	DebugBreak();
#endif

#if PLATFORM_PS3
	DoCriticalReadErrorDialogPS3(fileName);
#elif PLATFORM_PC
	DoCriticalReadErrorDialogPC(fileName);
#else
	BIZ_ASSERT_FAIL_SPEED("Unknown platform for read failure dialog");
#endif
}

void SetPrimaryPadQueryCallback(PrimaryPadQueryCallback cb)
{
	g_primaryPadQueryCallback = cb;
}

S32 GetPrimaryPadNo()
{
	if (g_primaryPadQueryCallback)
		return g_primaryPadQueryCallback();
	
	return 0;
}

void CriticalFileErrorTestCallback(Debug::VarBase* pVar, void* pContext)
{
	CriticalReadErrorDialog("thisisatest", false, true, NULL);
}

}; // namespace Framework
