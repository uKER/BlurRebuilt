#include "wrap32/com/stdafx.h"
#include <core/core.h>
#include <core/utility/singleton.h>
#include <wrap32/common/wrap32_common.h>
#include <wrap32/common/wrap32_xgraphics_args.h>
#include <wrap32/interface/wrap32_instance.h>

#pragma warning(push)
//Disable convertion from x to y of greater size warning as we move from 32->64 bit.
#pragma warning(disable:4311)
#pragma warning(disable:4312)

void TestJanBanana()
{
	printf("janbanana\n");
}

XBOXAPI
VOID 
WINAPI 
XGTileTextureLevel64(
    UINT             Width,        // Width of the texture (base level) in texels (not blocks)
    UINT             Height,       // Height of the texture (base level) in texels (not blocks)
    UINT             Level,        // Mip level of the destination
    DWORD            GpuFormat,    // Gpu format of the texture
    DWORD            Flags,        // Tiling flags (border, non-packed, etc.)
    VOID*            pDestination, // Address of the first texel in the destination (tiled) level
    CONST POINT*     pPoint,       // Offset in the destination level (in texels) to place the tiled image
    CONST VOID*      pSource,      // Address of the source (linear) image
    UINT             RowPitch,     // Distance in bytes between one row of the source image and the next
    CONST RECT*      pRect,         // Rect within the source image to copy (in texels)

	CONST VOID*		pSourceBaseAddress,
	UINT			srcSize,
	VOID*			pDestinationBaseAddress,
	UINT			dstSize)
{
	assert(pSourceBaseAddress == pSource);
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = sizeof(Wrap32_XGTileTextureLevel64Arguments) + 
		sizeof(POINT) +
		srcSize +
		sizeof(RECT)+
		dstSize;
	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);

	Wrap32_XGTileTextureLevel64Arguments* pArgs;
	{
		UINT offset = 0;
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return;

		pArgs =	(Wrap32_XGTileTextureLevel64Arguments*)pSharedMemory;
		offset += sizeof(Wrap32_XGTileTextureLevel64Arguments);

		pArgs->Width = Width;
		pArgs->Height = Height;
		pArgs->Level = Level;
		pArgs->GpuFormat = GpuFormat;
		pArgs->Flags = Flags;
		pArgs->RowPitch = RowPitch;
		pArgs->srcSize = srcSize;
		pArgs->dstSize = dstSize;

		//Copy the _entire_ texture.
		pArgs->pDestinationBaseAddress = offset;
		memcpy(pSharedMemory+offset, pDestinationBaseAddress, dstSize);
		offset += dstSize;

		//Calculate offset of pDestination relative to pDestinationBaseAddress
		UINT destinationRelativeOffset = (UINT)((U8*)pDestination - (U8*)pDestinationBaseAddress);
		//Store this offset.
		pArgs->pDestination = pArgs->pDestinationBaseAddress + destinationRelativeOffset;

		if (pPoint)
		{
			pArgs->pPoint = offset;
			memcpy(pSharedMemory+offset, pPoint, sizeof(POINT));
			offset += sizeof(POINT);
		}
		else
			pArgs->pPoint = -1;

		//Calculate offset of pDestination relative to pDestinationBaseAddress
		UINT sourceRelativeOffset = (UINT)((const U8*)pSource - (const U8*)pSourceBaseAddress);

		pArgs->pSource = offset + sourceRelativeOffset;
		memcpy(pSharedMemory+offset, pSource, srcSize);
		offset += srcSize;

		if (pRect)
		{
			pArgs->pRect = offset;
			memcpy(pSharedMemory+offset, pRect, sizeof(RECT));
		}
		else
			pArgs->pRect = -1;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return;
	}

	HRESULT hr = 
		C_Wrap32Instance::Instance().GetServerInstance()->XGTileTextureLevel(
			(UINT)pid, 0);

	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return; 
		pArgs =	(Wrap32_XGTileTextureLevel64Arguments*)pSharedMemory;

		//Copy the _entire_ texture.
		memcpy(pDestinationBaseAddress, pSharedMemory+pArgs->pDestinationBaseAddress, dstSize);

		BOOL result = C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
	}

	C_Wrap32Utility::DestroySharedMemory();
}

XBOXAPI
VOID 
WINAPI  
XGTileSurface64(
    VOID*        pDestination, // Base address of the destination (tiled) image
    UINT         Width,        // Width of the destination image
    UINT         Height,       // Height of the destination image
    CONST POINT* pPoint,       // Offset in the destination image to place the tiled rectangle
    CONST VOID*  pSource,      // Base address of the source (linear) image
    UINT         RowPitch,     // Distance in bytes between one row of the source image and the next
    CONST RECT*  pRect,        // Rectangle within the source image to copy
    UINT         TexelPitch,    // Size of an image texel/block in bytes

	CONST VOID*		pSourceBaseAddress,
	UINT			srcSize,
	VOID*			pDestinationBaseAddress,
	UINT			dstSize)
{
	assert(pSourceBaseAddress == pSource);
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = sizeof(Wrap32_XGTileSurface64Arguments) + 
		dstSize + 
		sizeof(POINT) +
		srcSize +
		sizeof(RECT);

	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);

	Wrap32_XGTileSurface64Arguments* pArgs;
	{
		UINT offset = 0;
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return;

		pArgs =	(Wrap32_XGTileSurface64Arguments*)pSharedMemory;
		offset += sizeof(Wrap32_XGTileSurface64Arguments);

		pArgs->Width = Width;
		pArgs->Height = Height;
		pArgs->RowPitch = RowPitch;
		pArgs->TexelPitch = TexelPitch;
		pArgs->srcSize = srcSize;
		pArgs->dstSize = dstSize;

		//Copy the _entire_ texture.
		pArgs->pDestinationBaseAddress = offset;
		memcpy(pSharedMemory+offset, pDestinationBaseAddress, dstSize);
		offset += dstSize;

		//Calculate offset of pDestination relative to pDestinationBaseAddress
		UINT destinationRelativeOffset = (UINT)((U8*)pDestinationBaseAddress - (U8*)pDestination);
		//Store this offset.
		pArgs->pDestination = pArgs->pDestinationBaseAddress + destinationRelativeOffset;

		if (pPoint)
		{
			pArgs->pPoint = offset;
			memcpy(pSharedMemory+offset, pPoint, sizeof(POINT));
			offset += sizeof(POINT);
		}
		else
			pArgs->pPoint = -1;

		pArgs->pSource = offset;
		memcpy(pSharedMemory+offset, pSource, srcSize);
		offset += srcSize;

		if (pRect)
		{
			pArgs->pRect = offset;
			memcpy(pSharedMemory+offset, pRect, sizeof(RECT));
		}
		else
			pArgs->pRect = -1;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return;
	}

	HRESULT hr = 
		C_Wrap32Instance::Instance().GetServerInstance()->XGTileSurface(
			(UINT)pid, 0);

	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return; 
		pArgs =	(Wrap32_XGTileSurface64Arguments*)pSharedMemory;

		//Copy the _entire_ texture.
		memcpy(pDestinationBaseAddress, pSharedMemory+pArgs->pDestinationBaseAddress, dstSize);

		BOOL result = C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
	}

	C_Wrap32Utility::DestroySharedMemory();
}

XBOXAPI
UINT 
WINAPI 
XGGetMipTailLevelOffset(
    UINT  Width,                // Width of the texture
    UINT  Height,               // Height of the texture
    UINT  Depth,                // Depth of the texture
    UINT  Level,                // Mip level of the texture
    DWORD GpuFormat,            // Gpu format of the texture
    BOOL  Tiled,                // TRUE if the texture is tiled, FALSE if linear
    BOOL  Border                // TRUE if the texture has a border, FALSE if it does not
    )
{
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = sizeof(Wrap32_XGGetMipTailLevelOffsetArguments);
	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);
	
	Wrap32_XGGetMipTailLevelOffsetArguments* pArgs;
	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return 0;
		pArgs =	(Wrap32_XGGetMipTailLevelOffsetArguments*)pSharedMemory;

		pArgs->Width = Width;
		pArgs->Height = Height;
		pArgs->Depth = Depth;
		pArgs->Level = Level;
		pArgs->GpuFormat = GpuFormat;
		pArgs->Tiled = Tiled;
		pArgs->Border = Border;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return 0;
	}

	UINT result = C_Wrap32Instance::Instance().GetServerInstance()->XGGetMipTailLevelOffset(
		(UINT)pid, 0);

	C_Wrap32Utility::DestroySharedMemory();

	return result;
}

XBOXAPI
UINT 
WINAPI 
XGGetMipTailBaseLevel(
    UINT Width,                 // Width of the texture
    UINT Height,                // Height of the texture
    BOOL Border                 // TRUE if the texture has a border, FALSE if it does not
    )
{
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = sizeof(Wrap32_XGGetMipTailBaseLevelArguments);
	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);
	
	Wrap32_XGGetMipTailBaseLevelArguments* pArgs;
	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return 0;
		pArgs =	(Wrap32_XGGetMipTailBaseLevelArguments*)pSharedMemory;

		pArgs->Width = Width;
		pArgs->Height = Height;
		pArgs->Border = Border;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return 0;
	}

	UINT result = C_Wrap32Instance::Instance().GetServerInstance()->XGGetMipTailBaseLevel(
		(UINT)pid, 0);

	C_Wrap32Utility::DestroySharedMemory();

	return result;
}

XBOXAPI
VOID 
WINAPI
XGGetTextureDesc(
    D3DBaseTexture* pTexture,   // Texture to describe
    UINT            Level,      // Mip level to describe
    XGTEXTURE_DESC* pDesc       // Structure to be filled with texture description
    )
{
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = 
		sizeof(Wrap32_XGGetTextureDescArguments)+
		sizeof(D3DBaseTexture)+
		sizeof(XGTEXTURE_DESC);
	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);
	
	Wrap32_XGGetTextureDescArguments* pArgs;
	{
		UINT offset = 0;
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return;
		pArgs =	(Wrap32_XGGetTextureDescArguments*)pSharedMemory;
		offset += sizeof(Wrap32_XGGetTextureDescArguments);

		pArgs->Level		= Level;

		if (pTexture)
		{
			pArgs->pTexture = offset;
			memcpy(pSharedMemory+offset, pTexture, sizeof(D3DBaseTexture));
			offset += sizeof(D3DBaseTexture);
		}
		else
			pArgs->pTexture = -1;

		if (pDesc)
		{
			pArgs->pDesc = offset;
			memcpy(pSharedMemory+offset, pDesc, sizeof(XGTEXTURE_DESC));
			offset += sizeof(XGTEXTURE_DESC);
		}
		else
			pArgs->pDesc = -1;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return;
	}

	C_Wrap32Instance::Instance().GetServerInstance()->XGGetTextureDesc(
		(UINT)pid, 0);

	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return; 
		pArgs =	(Wrap32_XGGetTextureDescArguments*)pSharedMemory;

		if (pTexture)
			memcpy(pTexture, pSharedMemory+pArgs->pTexture,	sizeof(D3DBaseTexture));

		if (pDesc)
			memcpy(pDesc, pSharedMemory+pArgs->pDesc,	sizeof(XGTEXTURE_DESC));

		BOOL result = C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
	}

	C_Wrap32Utility::DestroySharedMemory();
}

XBOXAPI
UINT 
WINAPI 
XGSetTextureHeader(
    UINT                    Width,
    UINT                    Height,
    UINT                    Levels,
    DWORD                   Usage,
    D3DFORMAT               Format,
    D3DPOOL                 Pool,
    UINT                    BaseOffset,
    UINT                    MipOffset,
    UINT                    Pitch,
    IDirect3DTexture9*      pTexture,
    UINT*                   pBaseSize,
    UINT*                   pMipSize)
{
	HANDLE currentProcess = GetCurrentProcess();
	DWORD pid = GetProcessId(currentProcess);
	UINT size = 
		sizeof(Wrap32_XGSetTextureHeaderArguments)+
		sizeof(IDirect3DTexture9)+
		sizeof(UINT)+
		sizeof(UINT);
	UINT allocatedSize = C_Wrap32Utility::CreateSharedMemory(size);
	assert(size == allocatedSize);
	
	Wrap32_XGSetTextureHeaderArguments* pArgs;
	{
		UINT offset = 0;
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return FALSE;
		pArgs =	(Wrap32_XGSetTextureHeaderArguments*)pSharedMemory;
		offset += sizeof(Wrap32_XGSetTextureHeaderArguments);

		pArgs->Width		= Width;
		pArgs->Height		= Height;
		pArgs->Levels		= Levels;
		pArgs->Usage		= Usage;
		pArgs->Format		= Format;
		pArgs->Pool			= Pool;
		pArgs->BaseOffset	= BaseOffset;
		pArgs->MipOffset	= MipOffset;
		pArgs->Pitch		= Pitch;

		if (pTexture)
		{
			pArgs->pTexture = offset;
			memcpy(pSharedMemory+offset, pTexture, sizeof(IDirect3DTexture9));
			offset += sizeof(IDirect3DTexture9);
		}
		else
			pArgs->pTexture = -1;

		if (pBaseSize)
		{
			pArgs->pBaseSize = offset;
			memcpy(pSharedMemory+offset, pBaseSize, sizeof(UINT));
			offset += sizeof(UINT);
		}
		else
			pArgs->pBaseSize = -1;

		if (pMipSize)
		{
			pArgs->pMipSize = (WRAP32_SHARED_MEMORY_OFFSET)pMipSize;
			memcpy(pSharedMemory+offset, pMipSize, sizeof(UINT));
		}
		else
			pArgs->pMipSize = -1;

		if (!C_Wrap32Utility::UnlockSharedMemory(pSharedMemory))
			return FALSE;
	}

	UINT sizeBytes = 
		C_Wrap32Instance::Instance().GetServerInstance()->XGSetTextureHeader(
			(UINT)pid, 0);

	{
		U8* pSharedMemory = C_Wrap32Utility::LockSharedMemory();
		if (!pSharedMemory) 
			return FALSE; 
		pArgs =	(Wrap32_XGSetTextureHeaderArguments*)pSharedMemory;

		if (pTexture)
			memcpy(pTexture, pSharedMemory+pArgs->pTexture,	sizeof(IDirect3DTexture9));

		if (pBaseSize)
			*pBaseSize = pArgs->pBaseSize;
		if (pMipSize)
			*pMipSize = pArgs->pMipSize;

		BOOL result = C_Wrap32Utility::UnlockSharedMemory(pSharedMemory);
	}

	C_Wrap32Utility::DestroySharedMemory();
	
	return sizeBytes;
}

VOID WINAPI XGEndianSwapSurface(
    VOID*               pDestination, 
    UINT                RowPitchDst, 
    CONST VOID*         pSource, 
    UINT                RowPitchSrc, 
    UINT                Width, 
    UINT                Height, 
    D3DFORMAT           Format
    )
{
	assert(0);
}

HRESULT WINAPI XGCopySurface(
    VOID*            pDestination,  // Base address of the destination image
    INT              DstRowPitch,   // Distance in bytes between one row of the destination image and the next
    UINT             Width,         // Width of the destination image in texels
    UINT             Height,        // Height of the destination image in texels
    D3DFORMAT        DstFormat,     // Format of the destination image
    CONST POINT*     pPoint,        // Offset in the destination image to place the copied rectangle
    CONST VOID*      pSource,       // Base address of the source image
    INT              SrcRowPitch,   // Distance in bytes between one row of the source image and the next
    D3DFORMAT        SrcFormat,     // Format of the source image
    CONST RECT*      pRect,         // Rectangle within the source image to copy
    DWORD            Flags,         // Conversion flags
    FLOAT            Threshold)     // Threshold for 1 bit components
{
	assert(0);
	return S_OK;
}

VOID WINAPI XGTileSurface(
    VOID*        pDestination, // Base address of the destination (tiled) image
    UINT         Width,        // Width of the destination image
    UINT         Height,       // Height of the destination image
    CONST POINT* pPoint,       // Offset in the destination image to place the tiled rectangle
    CONST VOID*  pSource,      // Base address of the source (linear) image
    UINT         RowPitch,     // Distance in bytes between one row of the source image and the next
    CONST RECT*  pRect,        // Rectangle within the source image to copy
    UINT         TexelPitch    // Size of an image texel/block in bytes
    )
{
	assert(0);
}

VOID WINAPI XGTileVolume(
    VOID*            pDestination, // Base address of the destination (tiled) volume
    UINT             Width,        // Width of the destination volume
    UINT             Height,       // Height of the destination volume
    UINT             Depth,        // Depth of the destination volume
    CONST XGPOINT3D* pPoint,       // Offset in the destination volume to place the tiled box
    CONST VOID*      pSource,      // Base address of the source (linear) volume
    UINT             RowPitch,     // Distance in bytes between one row of the source volume and the next
    UINT             SlicePitch,   // Distance in bytes between one slice of the source volume and the next
    CONST D3DBOX*    pBox,         // Box within the source volume to copy
    UINT             TexelPitch    // Size of a volume texel/block in bytes
    )
{
	assert(0);
}

UINT WINAPI XGSetVolumeTextureHeader(
    UINT                     Width,
    UINT                     Height,
    UINT                     Depth,
    UINT                     Levels,
    DWORD                    Usage,
    D3DFORMAT                Format,
    D3DPOOL                  Pool,
    UINT                     BaseOffset,
    UINT                     MipOffset,
    IDirect3DVolumeTexture9* pVolumeTexture,
    UINT*                    pBaseSize,
    UINT*                    pMipSize
    )
{
	assert(0);
	return 0;
}

VOID WINAPI XGGetBlockDimensions(
    DWORD GpuFormat,            // Gpu format of the compression block
    UINT* pWidth,               // Out: Width of a block
    UINT* pHeight               // Out: Height of a block
    )
{
	assert(0);
}

UINT WINAPI XGSetCubeTextureHeader(
    UINT                    EdgeLength,
    UINT                    Levels,
    DWORD                   Usage,
    D3DFORMAT               Format,
    D3DPOOL                 Pool,
    UINT                    BaseOffset,
    UINT                    MipOffset,
    IDirect3DCubeTexture9*  pCubeTexture,
    UINT*                   pBaseSize,
    UINT*                   pMipSize
    )
{
	assert(0);
	return 0;
}

VOID WINAPI XGUntileSurface(
    VOID*        pDestination, // Base address of the destination (linear) image
    UINT         RowPitch,     // Distance in bytes between one row of the destination image and the next
    CONST POINT* pPoint,       // Offset in the destination image to place the untiled rectangle
    CONST VOID*  pSource,      // Base address of the source (tiled) image
    UINT         Width,        // Width of the source image
    UINT         Height,       // Height of the source image
    CONST RECT*  pRect,        // Rectangle within the source image to copy
    UINT         TexelPitch    // Size of an image texel/block in bytes
    )
{
	assert(0);
}

D3DRESOURCETYPE             WINAPI D3DResource_GetType(D3DResource *pThis)
{
	assert(0);
	return D3DRESOURCETYPE(0);
}

DWORD                       WINAPI D3DBaseTexture_GetLevelCount(D3DBaseTexture *pThis)
{
	assert(0);
	return 0;
}


#pragma warning(pop)