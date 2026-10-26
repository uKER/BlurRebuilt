#pragma once

SERVER_API VOID Wrap32_XGTileTextureLevel(UINT callingPID, UINT sharedMemoryID);

SERVER_API VOID Wrap32_XGTileSurface(UINT callingPID, UINT sharedMemoryID);

SERVER_API UINT Wrap32_XGGetMipTailLevelOffset(UINT callingPID, UINT sharedMemoryID);

SERVER_API UINT Wrap32_XGGetMipTailBaseLevel(UINT callingPID, UINT sharedMemoryID);

SERVER_API VOID Wrap32_XGGetTextureDesc(UINT callingPID, UINT sharedMemoryID);

SERVER_API UINT Wrap32_XGSetTextureHeader(UINT callingPID, UINT sharedMemoryID);