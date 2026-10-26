#include "wrap32/com/stdafx.h"
#include "wrap32/com/Wrap32.h"

#include "wrap32/dll/wrap32_dll.h"

#define SHMEMSIZE 4096 


STDMETHODIMP C_Wrap32::XGTileTextureLevel(UINT callingPID, UINT sharedMemoryID)
{
	Wrap32_XGTileTextureLevel(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGTileSurface(UINT callingPID, UINT sharedMemoryID)
{
	Wrap32_XGTileSurface(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGGetMipTailLevelOffset(UINT callingPID, UINT sharedMemoryID, UINT* result)
{
	*result = Wrap32_XGGetMipTailLevelOffset(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGGetMipTailBaseLevel(UINT callingPID, UINT sharedMemoryID, UINT* result)
{
	*result = Wrap32_XGGetMipTailBaseLevel(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGGetTextureDesc(UINT callingPID, UINT sharedMemoryID)
{
	Wrap32_XGGetTextureDesc(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGSetTextureHeader(UINT callingPID, UINT sharedMemoryID, UINT* result)
{
	*result = Wrap32_XGSetTextureHeader(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGEndianSwapSurface(UINT callingPID, UINT sharedMemoryID)
{
//	Wrap32_XGEndianSwapSurface(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGCopySurface(UINT callingPID, UINT sharedMemoryID)
{
//	Wrap32_XGCopySurface(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGTileVolume(UINT callingPID, UINT sharedMemoryID)
{
//	Wrap32_XGTileVolume(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGSetVolumeTextureHeader(UINT callingPID, UINT sharedMemoryID)
{
//	Wrap32_XGSetVolumeTextureHeader(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGGetBlockDimensions(UINT callingPID, UINT sharedMemoryID)
{
//	Wrap32_XGGetBlockDimensions(callingPID, sharedMemoryID);
	return S_OK;
}

STDMETHODIMP C_Wrap32::XGSetCubeTextureHeader(UINT callingPID, UINT sharedMemoryID)
{
	return S_OK;
}