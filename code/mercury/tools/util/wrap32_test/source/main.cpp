#include "stdafx.h"

#include "wrap32/com/stdafx.h"
#include <core/core.h>

#include <win32/vs2005/d3d9.h>
#include <win32/vs2005/xgraphics.h>
#include <win32/vs2005/xcompress.h>
//#include <compress_lib/CompressLib.h>

#include <core/containers/string.h>
#include <system/debuglog.h>
#include <system/misc/StringUtil.h>
#include <system/shell/ShellExec.h>

#include <wrap32/interface/wrap32_interface.h>

void TestJanBanana();

static Debug::Logger& g_Log = Debug::GetLogger("tools.wrap32_dll");

void RegisterCOM()
{
	// get the path that this executable is in
	char szFilePath[_MAX_PATH];
	GetModuleFileName(NULL, szFilePath, _MAX_PATH);

	Util::C_String sExePath = Util::ExtractDirectory(szFilePath) + "wrap32_test_com.exe";

	if (Sys::ShellExec::ExecuteCommandAndWait(
		sExePath.Chars(), 
		"/regserver", 
		"",
		Sys::ShellExec::E_LaunchState_Minimise,
		&g_Log))
	{
		LOGERROR( g_Log, "Failed to register COM server for Wrap32.\n");
	}
}

BOOL TestXGraphics()
{
	{
		DWORD tiledSize = 
			XGSetTextureHeader(
			128, 
			128, 
			1, 
			0, 
			D3DFMT_DXT1,
			0,
			0, 
			XGHEADER_CONTIGUOUS_MIP_OFFSET, 
			0, 
			0, 
			NULL,
			NULL);
		int btb = 0;
	}

	int width = 640;
	int height = 480;
	int nLevels = 5;
	D3DFORMAT format = D3DFMT_DXT5;
	IDirect3DTexture9 header;

	DWORD tiledSize = 
		XGSetTextureHeader(
		width, 
		height, 
		nLevels, 
		0, 
		format,
		0,
		0, 
		XGHEADER_CONTIGUOUS_MIP_OFFSET, 
		0, 
		&header, 
		NULL,
		NULL); 

	XGTEXTURE_DESC desc;
	XGGetTextureDesc(
		&header,
		nLevels,
		&desc);

	int border = 1;
	UINT mipSize = XGGetMipTailBaseLevel(
		width,
		height,
		border);

	tiledSize = 32768;
	char* pSrcData = new char[tiledSize];
	char* pDest = new char[tiledSize];

	int mipOffset = 8192;

	int level = 0;
	XGTileTextureLevel64(
		128,
		128,
		1, 
		18,
		0, 
		pDest + mipOffset, 
		NULL,
		pSrcData,
		desc.WidthInBlocks * desc.BytesPerBlock,
		NULL,
		
		pSrcData,
		2048,
		pDest,
		tiledSize);

	UINT result;

	result = XGGetMipTailLevelOffset(
		width,
		height,
		1,
		8,
		XGGetGpuFormat(format),
		true,
		true);
/*
	XGTileSurface64(
		pDest,
		width,
		height,
		0,
		pSrcData,
		desc.WidthInBlocks * desc.BytesPerBlock,
		0,
		desc.BytesPerBlock,
		
		pSrcData,
		tiledSize,
		pDest,
		tiledSize);
*/

	delete[] pSrcData;
	delete[] pDest;

	return true;
}

int main(int argc, char* argv[])
{
	RegisterCOM();

	TestJanBanana();

	TestXGraphics();

	return 0;
}

