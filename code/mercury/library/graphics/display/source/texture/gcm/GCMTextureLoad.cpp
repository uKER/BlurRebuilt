#include "stdafx.h"
#include "Display/Device/Device.h"

#include <Core/Utility/MemCpy.h>
#include <System/Serialise/I_BinaryStream.h>
#include <System/Serialise/BinarySerialise.h>

namespace Display
{
namespace GCM
{

void AllocateTextureMemory(C_Texture2D *pTexture)
{
	BIZ_ASSERT(pTexture);

	pTexture->SetFlags(pTexture->GetFlags() | Display2::E_Flags_Video); // Preserve what's already there...
	
	BIZ_ASSERT(pTexture->GetSize() > 0);

	U8* pAddress = NULL;
	if(Display2::Device()->AllocMemory(pTexture->GetSize(), ALIGNMENT_TEXTURE, pTexture->GetFlags(), &pAddress, Display2::E_MemoryType_Texture))
	{
		U32 offset = 0;
		S32 result = cellGcmAddressToOffset(pAddress, &offset);
		BIZ_ASSERT(result == CELL_OK);

		pTexture->m_gcmTexture.offset = offset;
		pTexture->m_gcmTexture.location = Display2::FlagsToLocation(pTexture->GetFlags());
	}
	else
	{
		pTexture->SetSize(0);
	}
}

void CreateTexture(C_Texture2D *pTexture)
{
	BIZ_ASSERT(pTexture && pTexture->IsCreated());

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->InsertTexture(pTexture);
	#endif
	
	AllocateTextureMemory(pTexture);

	#if TRACK_DISPLAY2_RESOURCES
	if(!pTexture->IsCreated())
		Display2::Device()->RemoveTexture(pTexture);
	#endif
}

void CloneTexture2D(C_Texture2D *pDst, const C_Texture2D *pSrc, int newWidth, int newHeight, int newLevels, int newSize)
{
	BIZ_ASSERT(pDst);
	BIZ_ASSERT(pSrc);

	U32 flags = pDst->GetFlags() & E_Usage_AlignSizeToBlockSize ? E_Usage_AlignSizeToBlockSize : 0; // Preserve this for the guard blocks...

	if(pDst->IsCreated())
		Display::Device()->Destroy(pDst);

	pDst->SetFlags(flags);

	*pDst = *pSrc;
	
	pDst->m_gcmTexture.width	= newWidth;
	pDst->m_gcmTexture.height	= newHeight;
	pDst->m_gcmTexture.mipmap	= newLevels;

	pDst->SetSize(newSize);

	CreateTexture(pDst);
}

S32 CreateTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream)
{
	BIZ_ASSERT(pTexture);

	U32 flags = pTexture->GetFlags() & E_Usage_AlignSizeToBlockSize ? E_Usage_AlignSizeToBlockSize : 0; // Preserve this for the guard blocks...

	if (pTexture->IsCreated())
		Display::Device()->Destroy(pTexture);

	pTexture->SetFlags(flags);

	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);
	
	// read header
	ar	>> pTexture->m_gcmTexture.width
		>> pTexture->m_gcmTexture.height
		>> pTexture->m_gcmTexture.mipmap
		>> pTexture->m_gcmTexture.format
		>> pTexture->m_gcmTexture.dimension
		>> pTexture->m_gcmTexture.cubemap
		>> pTexture->m_gcmTexture.remap
		>> pTexture->m_gcmTexture.depth
		>> pTexture->m_gcmTexture.pitch
		>> pTexture->m_gcmTexture.offset;

	if(pTexture->m_gcmTexture.cubemap)
		pTexture->m_gcmTexture.depth = 1; // This is coming in wrong from the converter

	pTexture->m_gcmTexture.offset = UINT_MAX; // Make sure this starts out as an invalid offset

	// read raw data size
	S32 iSize;
	ar >> iSize;

	pTexture->SetSize(iSize);

	if(iSize > 0)
	{
		CreateTexture(pTexture);

		#if !CFG_GOLD
		if(pTexture->IsCreated())
			Display2::DebugCheckTexture(&pTexture->m_gcmTexture);
		#endif
		
	}
#if !CFG_GOLD
	else
	{
		LOGERROR(g_Log, "Loading a ps3 texture of size 0 width %i height %i mipmap %i format %i dimension %i cubemap %i remap %i depth %i pitch %i offset %i\n",
			pTexture->m_gcmTexture.width,
			pTexture->m_gcmTexture.height,
			pTexture->m_gcmTexture.mipmap,
			pTexture->m_gcmTexture.format,
			pTexture->m_gcmTexture.dimension,
			pTexture->m_gcmTexture.cubemap,
			pTexture->m_gcmTexture.remap,
			pTexture->m_gcmTexture.depth,
			pTexture->m_gcmTexture.pitch,
			pTexture->m_gcmTexture.offset);
	}
#endif
	
	return iSize;
}

void LoadTextureData2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream, int size)
{
	BIZ_ASSERT(pTexture);

	U8* pAddress = pTexture->GetAddress();

	pStream->Read(pAddress, size);
}

void LoadTexture2D(C_Texture2D *pTexture, Sys::I_BinaryStream *pStream)
{
	BIZ_ASSERT(pTexture);

	S32 size = CreateTexture2D(pTexture, pStream);

	LoadTextureData2D(pTexture, pStream, size);
}

void GPUImageTransfer(void* pDstData, const void* pSrcData, U32 srcSize)
{
	GPU_ASSERT_X(Device()->HaveThreadOwnership());

	U32 srcOffset = 0;
	GPU_ASSERT_X(Display2::IsLocalAddress((void*)pSrcData));
	S32 result = cellGcmAddressToOffset(pSrcData, &srcOffset);
	GPU_ASSERT_X(result == CELL_OK);
	
	U32 dstOffset = 0;
	GPU_ASSERT_X(Display2::IsLocalAddress(pDstData));
	result = cellGcmAddressToOffset(pDstData, &dstOffset);
	GPU_ASSERT_X(result == CELL_OK);
	GPU_ASSERT_X(dstOffset % 16 == 0);

	#if TRACK_TILED_MEMORY
	GPU_ASSERT_X(!Display2::Device()->m_TileInfoManager.IsTiledMemory(srcSize, dstOffset, Display2::E_Location_Local));
	#endif

	U32 size = Btl::AlignUp(srcSize, 128);
	
	GPU_ASSERT_X(!Display2::Device()->IsProtectedMemory(size, (U8*)pDstData));
	
	#if TRACK_TILED_MEMORY
	GPU_ASSERT_X(!Display2::Device()->m_TileInfoManager.IsTiledMemory(size, dstOffset, Display2::E_Location_Local));
	#endif

	cellGcmSetTransferImage(CELL_GCM_TRANSFER_LOCAL_TO_LOCAL, dstOffset, 128, 0, 0, srcOffset, 128, 0, 0, 64, size>>7, 2);
}

void CopyGPUData(void* pDst, const void* pSrc, S32 size)
{
	FastMemCpy(pDst, pSrc, size);

	// flush the gpu cache
	for(int offset=0; offset < size; offset+=128)
		__dcbf((U8*)pDst + offset);
}

}; // namespace GCM
}; // namespace Display
