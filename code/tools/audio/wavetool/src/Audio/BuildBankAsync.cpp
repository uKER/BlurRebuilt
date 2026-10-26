#include "Core/StdAfx.h"
#include "XtWin/Core.h"
#include "Application/SorceryApp.h"
#include "Application/SorceryView.h"
#include "Xt/XtTool.h"
#include "Audio/WaveBankPane.h"
#ifndef WAVEUTILS_H
#include "WaveUtils.h"
#endif

//-----------------------------------------------------------------------------
// Name: MessageBoxPrintFn
// Desc: used to print out a message when no asyncop exists
//-----------------------------------------------------------------------------
void MessageBoxPrintFn(int msgType, Util::C_String& str, void* pContext)
{
	if (pContext==NULL)
		return;

	SorceryApp* pApp = (SorceryApp*)pContext;
	
	MessageBox(pApp->_window, _T(str.Chars()), _T("Bob"), MB_OK | MB_ICONERROR);
}


int giNumWarnings = 0;

//-----------------------------------------------------------------------------
// Name: NewAsyncOpMessageFn
// Desc: the callback which the dll will call to add messages to the async op window
//-----------------------------------------------------------------------------
void AsyncOpNewMessageFn(int msgType, Util::C_String& str, void* pContext)
{
	AsyncOp* pOp = (AsyncOp*)pContext;

	if (msgType == kAoiWarning)
		giNumWarnings++;

	if (pOp)
		pOp->NewMessage((AsyncOpIcon)msgType, XtString(str.Chars()));
	//else
	//	MessageBox(NULL, _T("NULL AsyncOp"), _T("Bob"), MB_OK | MB_ICONERROR);
}

//F32 fLastProgress=0.f;
//-----------------------------------------------------------------------------
// Name: NewAsyncOpMessageFn
// Desc: the callback which the dll will call to add messages to the async op window
//-----------------------------------------------------------------------------
void AsyncOpSetProgressFn(F32 fProgress, void* pContext)
{
	AsyncOp* pOp = (AsyncOp*)pContext;

	//if (fProgress < fLastProgress)
	//	OutputDebugString("HOTDAMN\n");
	//fLastProgress = fProgress;
	//OutputDebugString(Util::C_String().Printf("%.3f\n", fProgress).Chars());

	if (pOp)
		pOp->SetProgress(fProgress);
	//else
	//	MessageBox(NULL, _T("NULL AsyncOp"), _T("Bob"), MB_OK | MB_ICONERROR);
}


// set this filter to build a subset of the banks. eg setting to "weapons" will build all banks in folder "weapons". 
// or setting to "weapons.shotgun" will build only the record "shotgun" in the "weapons" folder
Util::C_String g_strBankBuildFilter="";		

int gBuildMode=eBuildAllBanks;

AsyncOp* gpAsyncOp = 0;

//-----------------------------------------------------------------------------
// Name: BuildBanksSub
// Desc: function called from async thread to build bank(s)
//-----------------------------------------------------------------------------
bool BuildBanksSub(AsyncOp* op, SorceryApp* app)
{
	StopWatch sw;

	op->SetStayOpen();

	if (!app)
		return false;

	giNumWarnings = 0;
	//WaveBundlerDll::Get().SetAsyncOpContext(op);
	gpAsyncOp = op;

	BankBuilder::Get().SetMessagePrintCallback(AsyncOpNewMessageFn, op);
	BankBuilder::Get().SetProgressUpdateCallback(AsyncOpSetProgressFn, op);
	ProjectSettings::Get().SetMessagePrintCallback(AsyncOpNewMessageFn, op);
	//ProjectSettings::Get().SetMessagePrintCallback(MessageBoxPrintFn, app);
	
	if (!ProjectSettings::Get().Update(app->_db))
	{
		op->NewMessage(kAoiError, XtString("Build Failed"));
		return false;
	}

	if (!ProjectSettings::Get().Validate())
	{
		op->NewMessage(kAoiError, XtString("Build Failed"));
		return false;
	}

	// set the output dirs and which platforms to build
	for (int i=0; i<NumPlatforms; i++)
	{
		BankBuilder::Get().PlatformSettings(i).SetOutputPath(ProjectSettings::Get().PlatformSettings(i).OutputPath());
		BankBuilder::Get().PlatformSettings(i).SetBuildEnabled(ProjectSettings::Get().PlatformSettings(i).BuildEnabled());
		BankBuilder::Get().PlatformSettings(i).SetCopyToKit(ProjectSettings::Get().PlatformSettings(i).NeedToCopyToKit());
		BankBuilder::Get().PlatformSettings(i).SetDeploymentPath(ProjectSettings::Get().PlatformSettings(i).DeploymentPath());
		BankBuilder::Get().PlatformSettings(i).SetStreamingDeploymentPath(ProjectSettings::Get().PlatformSettings(i).StreamingDeploymentPath());
	}

	bool bOk=false;
	WaveBankMdiWindow* pWindow=0;

	PresetManager::Get().PopulateFromDom(app->_db);

	switch (gBuildMode)
	{
		case eBuildCurrentBank:
		{
			op->SetTitle("Building Current Bank...");
			SorceryView* pView = app->_window.GetActiveView();
			if (pView)
			{
				if (pView->_type == kSvtWaveBank)
				{
					pWindow = (WaveBankMdiWindow*)pView->_window;
					if (pWindow)
					{
						bOk = BankBuilder::Get().Build(pWindow->Bank());
					}
				}
			}
		}
		break;
		case eBuildAllBanks:
			op->SetTitle("Building All Banks...");
			bOk = BankBuilder::Get().BuildAll(app->_db);
		break;
		case eBuildSubsetOfBanks:
		{
			Util::C_String str;
			str.Printf("Building Bank(s) in folder '%s'", g_strBankBuildFilter.Chars());
			op->SetTitle(str.Replace(".", "\\").Chars());
			bOk = BankBuilder::Get().BuildAll(app->_db, g_strBankBuildFilter);
		}
		break;
		default:
			bOk = false;
			op->NewMessage(kAoiError, XtString("Unknown build mode!"));
		break;
	}

	if (bOk)
	{
		if (BankBuilder::Get().NumBanksBuilt())
		{
			Util::C_String strWarn = Util::C_String().Printf(" with %d warnings", giNumWarnings);
			op->NewMessage(kAoiInfo, XtString(Util::C_String().Printf("Completed in %s with %d warnings", Wb::FormatTime(sw.Time()).Chars(), giNumWarnings).Chars())); 
		}
		else
		{
			op->NewMessage(kAoiError, "No banks found to build!");
		}
	}
	else
	{
		if (gBuildMode == eBuildCurrentBank && !pWindow)
			op->NewMessage(kAoiError, "No bank selected!");
		else
			op->NewMessage(kAoiError, "Failed to build bank(s).");
	}

	//WaveBundlerDll::Get().SetAsyncOpContext(NULL);
	gpAsyncOp = 0;

	ProjectSettings::Get().SetMessagePrintCallback(MessageBoxPrintFn, app);

	return bOk;
}

//-----------------------------------------------------------------------------
// Name: BuildAllBanks
// Desc: kicks off the async thread to build subset of all banks 
//-----------------------------------------------------------------------------
void BuildBanksSubset(SorceryApp& app, const char* pchFilter)
{
	gBuildMode = eBuildSubsetOfBanks;
	g_strBankBuildFilter = pchFilter;
	AsyncInvoke(app._window, BuildBanksSub, &app);
}

//-----------------------------------------------------------------------------
// Name: BuildAllBanks
// Desc: kicks off the async thread to build all banks 
//-----------------------------------------------------------------------------
void BuildAllBanks(SorceryApp& app)
{
	gBuildMode = eBuildAllBanks;
	AsyncInvoke(app._window, BuildBanksSub, &app);
}

//-----------------------------------------------------------------------------
// Name: BuildCurrentBank
// Desc: kicks off the async thread to build current bank
//-----------------------------------------------------------------------------
void BuildCurrentBank(SorceryApp& app)
{
	gBuildMode = eBuildCurrentBank;
	AsyncInvoke(app._window, BuildBanksSub, &app);
}




































































