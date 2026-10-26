#include <core/core.h>
#include "wrap32/com/stdafx.h"
#include "wrap32/dll/wrap32_dll.h"
#include "wrap32/common/Wrap32_common.h"
#include "wrap32/common/Wrap32_xgraphics_args.h"


SERVER_API VOID Wrap32_XGTileTextureLevel(UINT callingPID, UINT sharedMemoryID)
{
	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return;

	//Initialise arguments.
	Wrap32_XGTileTextureLevel64Arguments& args = 
		*((Wrap32_XGTileTextureLevel64Arguments*)pSharedMemory);

    VOID*					pDestination;
    POINT*                  pPoint;
    VOID*                   pSource;
	RECT*					pRect;

	C_Wrap32Utility::SetPointerFromOffset(
		pDestination,
		pSharedMemory,
		args.pDestination);

	C_Wrap32Utility::SetPointerFromOffset(
		pPoint,
		pSharedMemory,
		args.pPoint);

	C_Wrap32Utility::SetPointerFromOffset(
		pSource,
		pSharedMemory,
		args.pSource);

	C_Wrap32Utility::SetPointerFromOffset(
		pRect,
		pSharedMemory,
		args.pRect);

	XGTileTextureLevel(
		args.Width,
		args.Height,
		args.Level,
		args.GpuFormat,
		args.Flags,
		pDestination,
		pPoint,
		pSource,
		args.RowPitch,
		pRect);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
}

SERVER_API VOID Wrap32_XGTileSurface(UINT callingPID, UINT sharedMemoryID)
{
	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return;

	//Initialise arguments.
	Wrap32_XGTileSurface64Arguments& args = 
		*((Wrap32_XGTileSurface64Arguments*)pSharedMemory);

    VOID*					pDestination;
    POINT*                  pPoint;
    VOID*                   pSource;
	RECT*					pRect;

	C_Wrap32Utility::SetPointerFromOffset(
		pDestination,
		pSharedMemory,
		args.pDestination);

	C_Wrap32Utility::SetPointerFromOffset(
		pPoint,
		pSharedMemory,
		args.pPoint);

	C_Wrap32Utility::SetPointerFromOffset(
		pSource,
		pSharedMemory,
		args.pSource);

	C_Wrap32Utility::SetPointerFromOffset(
		pRect,
		pSharedMemory,
		args.pRect);

	XGTileSurface(
		pDestination,
		args.Width,
		args.Height,
		pPoint,
		pSource,
		args.RowPitch,
		pRect,
		args.TexelPitch);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
}

SERVER_API UINT Wrap32_XGGetMipTailLevelOffset(UINT callingPID, UINT sharedMemoryID)
{
	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return 0;

	//Initialise arguments.
	Wrap32_XGGetMipTailLevelOffsetArguments& args = 
		*((Wrap32_XGGetMipTailLevelOffsetArguments*)pSharedMemory);

	UINT result = XGGetMipTailLevelOffset(
		args.Width,
		args.Height,
		args.Depth,
		args.Level,
		args.GpuFormat,
		args.Tiled,
		args.Border);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);

	return result;
}

SERVER_API UINT Wrap32_XGGetMipTailBaseLevel(UINT callingPID, UINT sharedMemoryID)
{
	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return 0;

	//Initialise arguments.
	Wrap32_XGGetMipTailBaseLevelArguments& args = 
		*((Wrap32_XGGetMipTailBaseLevelArguments*)pSharedMemory);

	UINT result = XGGetMipTailBaseLevel(
		args.Width,
		args.Height,
		args.Border);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);

	return result;
}

SERVER_API VOID Wrap32_XGGetTextureDesc(UINT callingPID, UINT sharedMemoryID)
{
	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return;

	//Initialise arguments.
	Wrap32_XGGetTextureDescArguments& args = 
		*((Wrap32_XGGetTextureDescArguments*)pSharedMemory);

    IDirect3DTexture9*      pTexture;
	XGTEXTURE_DESC*			pDesc;

	C_Wrap32Utility::SetPointerFromOffset(
		pTexture,
		pSharedMemory,
		args.pTexture);

	C_Wrap32Utility::SetPointerFromOffset(
		pDesc,
		pSharedMemory,
		args.pDesc);

	XGGetTextureDesc(
		pTexture,
		args.Level,
		pDesc);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
}

SERVER_API UINT Wrap32_XGSetTextureHeader(UINT callingPID, UINT sharedMemoryID)
{
	OutputDebugString("I'm here\n");
	printf("I'm here\n");

	U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
	if (!pSharedMemory)
		return 0;

	//Initialise arguments.
	Wrap32_XGSetTextureHeaderArguments& args = 
		*((Wrap32_XGSetTextureHeaderArguments*)pSharedMemory);

    IDirect3DTexture9*      pTexture;
    UINT*                   pBaseSize;
    UINT*                   pMipSize;

	C_Wrap32Utility::SetPointerFromOffset(
		pTexture,
		pSharedMemory,
		args.pTexture);

	C_Wrap32Utility::SetPointerFromOffset(
		pBaseSize,
		pSharedMemory,
		args.pBaseSize);

	C_Wrap32Utility::SetPointerFromOffset(
		pMipSize,
		pSharedMemory,
		args.pMipSize);

	UINT sizebytes = XGSetTextureHeader(
		args.Width,
		args.Height,
		args.Levels,
		args.Usage,
		args.Format,
		args.Pool,
		args.BaseOffset,
		args.MipOffset,
		args.Pitch,
		pTexture,
		pBaseSize,
		pMipSize);

	C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
	return sizebytes;
}
