#include "Core/StdAfx.h"

//#include "XtWin/Core.h"
//#include "WaveBundlerDll.h"
//
//extern void AsyncOpNewMessageFn(int msgType, Util::C_String& str, void* pContext);
//extern void AsyncOpSetProgressFn(F32 fProgress, void* pContext);
//
//const char* WaveBundlerDll::_pchFuncNames[eNumFunctions] =
//{
//	"QueryWaveFile",
//	"BuildBankFromXmlInMemory",
//};
//
//const char* pchQueryWavFileResultStrings[eNumQueryWavResults] =
//{
//	"Dll out of date",							
//	"Dll out of date",
//	"Succeeded",
//	"Could not load dll",
//	"Could not find function in dll",
//	"Structure Size Mismatch",
//	"Could Not Open File",
//	"Could Not Get Format",
//	"Could Not Get Loop Info",
//};
//
//#include "Xt/XtTool.h"
//
//bool Dll::_bDoneDllNotFoundMessage = false;
//namespace Wb { XtString GetWindowsErrorMessage(int errorCode); }
//
////-----------------------------------------------------------------------------
//// Name: LoadDll()
//// Desc: 
////-----------------------------------------------------------------------------
//bool Dll::LoadDll()
//{
//	if (Loaded())
//		return true;
//
//	_hModDll = LoadLibrary(Filename());
//	
//	if (_hModDll)
//		return true;
//
//	// only print error once
//	if (!_bDoneDllNotFoundMessage)
//	{
//		Util::C_String str = Util::C_String().Printf("Could not load %s", Filename());
//		XtString error;
//
//		if (GetLastError() != ERROR_ALREADY_EXISTS)
//		{
//			error = Wb::GetWindowsErrorMessage(GetLastError());
//		}
//
//		if (error != "")
//		{
//			str += Util::C_String("\n\nWindows error message : ");
//			str += error.Text();
//		}
//
//		MessageBox(NULL, str, "Ouch", MB_OK);
//		_bDoneDllNotFoundMessage = true;
//	}
//
//	return false;
//}
//
////-----------------------------------------------------------------------------
//// Name: GetFn()
//// Desc: Get a function
////-----------------------------------------------------------------------------
//FARPROC Dll::GetFn(int index)
//{
//	if (_fns.Size() <= index)
//	{
//		_fns.Resize(index+1);
//		//_fns[index].fn = 0;
//	}
//	if (!_fns[index].fn)
//	{
//		FARPROC p = GetProcAddress(_hModDll, FunctionName(index));
//		
//		if (p == NULL) 
//			return 0;
//
//		_fns[index].fn = p;
//	}
//	return _fns[index].fn;
//}
//
////-----------------------------------------------------------------------------
//// Name: QueryWaveFileInfo()
//// Desc: Call the function int the Wavebundler dll to open the wavefile and
//// query its details
////-----------------------------------------------------------------------------
//eQueryWaveFileResult WaveBundlerDll::QueryWaveFile(WaveFileInfo& waveFileInfo)
//{
//	if (LoadDll()==false)
//		return eFailedDllLoad;
//
//	FuncQueryWaveFile func = (FuncQueryWaveFile)GetFn(eFunc_QueryWaveFile);
//
//	if (!func)
//		return eFailedFunctionLookup;
//
//	return func(waveFileInfo, sizeof(WaveFileInfo));
//}
//
////-----------------------------------------------------------------------------
//// Name: QueryWaveFileInfo()
//// Desc: Call the function int the Wavebundler dll to open the wavefile and
//// query its details
////-----------------------------------------------------------------------------
//const char* LookupQueryWavResultString(eQueryWaveFileResult eResult)
//{
//	return pchQueryWavFileResultStrings[eResult];
//}
//
////-----------------------------------------------------------------------------
//// Name: BuildBankFromXmlInMemory()
//// Desc: build a bank given an xml file in memory
////-----------------------------------------------------------------------------
//bool WaveBundlerDll::BuildBankFromXmlInMemory(BuildBankParams& params)
//{
//	if (LoadDll()==false)
//		return false;
//
//	FuncBuildBankFromXmlInMemory func = (FuncBuildBankFromXmlInMemory)GetFn(eFunc_BuildBank);
//
//	if (!func)
//		return false;
//
//	if (!_pContext)
//		return false;
//
//	params.newMsgFunc = AsyncOpNewMessageFn;
//	params.setProgressFunc = AsyncOpSetProgressFn;
//	params.pContext = _pContext;
//	
//	if (func(params, sizeof(BuildBankParams)) == false)
//		return false;
//
//	return true;
//}
