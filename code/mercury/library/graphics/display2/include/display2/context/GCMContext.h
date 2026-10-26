#ifndef DISPLAY2_GCM_CONTEXT_H
#define DISPLAY2_GCM_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Context/GCMContextSPU.h"

namespace Display2
{

//-----------------------------------------------------------------------------
// Macros

#define GPU_U8_4_TO_U32(r, g, b, a)				((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff)

//-----------------------------------------------------------------------------
// Enums

enum E_BlendMode
{
	E_BlendMode_Solid							= 0,
	E_BlendMode_SolidAlphaTest,
	E_BlendMode_Alpha,
	E_BlendMode_AlphaPreMultiply,
	E_BlendMode_Additive,
	E_BlendMode_AlphaAdditive,
	E_BlendMode_AlphaSubtractive,
	E_BlendMode_Subtractive,
	E_BlendMode_Multiply,
	E_BlendMode_Count,
};

enum E_Label
{
	E_Label_DebugFinish							= 64,
	E_Label_SyncCpuGpu							= 65,
	E_Label_BlockUntilIdle						= 66,
	E_Label_RingBuffer1							= 67,
	E_Label_RingBuffer2							= 68,
	E_Label_CpuGpuSyncWait						= 69,
	E_Label_BlockUntilIdleWait					= 70,
	E_Label_BinkTexturesWait					= 71,
	E_Label_CommandFence						= 72,
	E_Label_TextureFence						= 73,
	E_Label_RasterFence							= 74,
	E_Label_LastMarkerId						= 75,
	E_Label_LastMarkerEa						= 76,
};

//-----------------------------------------------------------------------------
// Functions

U32 AddressToOffset(void* pAddress);

E_SurfaceFormat GetSurfaceFormatFromTexture(const E_TextureFormat format);

E_SurfaceFormat GetSurfaceFromTextureFormat(const E_TextureFormat format);

void CpuSetLabel(const U32 index, const U32 value);

void GpuSetLabelOnCommandEnd(const U32 index, const U32 value);

void GpuSetLabelOnTextureEnd(const U32 index, const U32 value);

void GpuSetLabelOnRasterEnd(const U32 index, const U32 value);

void CpuWaitOnLabel(const U32 index, const U32 value);

void GpuWaitOnLabel(const U32 index, const U32 value);

//------------------------------------------------------------------------------
// Structs

struct C_BlendMode
{
	C_BlendMode();

	E_Bool										m_AlphaTestEnable;
	C_AlphaTestState							m_AlphaTestState;
	E_Bool										m_BlendEnable;
	C_BlendState								m_BlendState;
};

struct C_VideoMode
{
	C_VideoMode();

	U32											m_Width;
	U32											m_Height;
	bool										m_WideScreen;
	bool										m_HiDef;
};

#define INVALID_TILE_INDEX						15
#define INVALID_TILE_MEMORY						2048

enum E_TileFlags
{
	E_TileFlags_UserIndex						= 0x00000001,
	E_TileFlags_UserMemory						= 0x00000002,
};



struct C_TileInfo
{
	C_TileInfo();

	void SetToDefaults();

	bool										enable;
	bool										indirect;
	U32											flags;

	U8 											index;
	U16											base;
	U32											bank;

	U8 											location;
	U32 										offset;
	U32 										size;
	U32 										pitch;
	U8											comp;

	Util::C_String32							name;
};

struct C_SharedTiledArea
{
	C_SharedTiledArea();

	U8*				GetAddress(U32 buffer) { return pAddress + size * buffer; }
	U32				GetSize() { return size; }

	U32				totalSize;
	U32				size;
	U8*				pAddress;
	C_TileInfo		tileInfo;
};

#define INVALID_ZCULL_INDEX						8
#define INVALID_ZCULL_MEMORY					3145729

enum E_ZcullFlags
{
	E_ZcullFlags_UserIndex						= 0x00000001,
	E_ZcullFlags_UserMemory						= 0x00000002,
};

struct C_ZcullInfo
{
	C_ZcullInfo();

	void SetToDefaults();

	bool										enable;
	U32											flags;

	U32 										index;
	U32 										cullStart;

	U32											size;
	U32 										offset;
	U32 										width;
	U32 										height;
	U32 										aaFormat;

	U32 										zFormat;
	U32 										zcullDir;
	U32 										zcullFormat;
	U32 										sFunc;
	U32 										sRef;
	U32 										sMask;
};

struct C_RenderBufferSetup
{
	C_RenderBufferSetup();

	U32											m_ColourMemorySize;
	U8*											m_pColourMemory;
	U32											m_ColourMemoryUsed;

	U32											m_DepthMemorySize;
	U8*											m_pDepthMemory;
	U32											m_DepthMemoryUsed;
};

struct C_RenderBuffer
{
	C_RenderBuffer();

	void SetToDefaults();

	bool ColourBufferEnabled(const U32 i) const;
	bool DepthStencilBufferEnabled() const;

	const Util::C_String32& GetName() const;

	U32 GetWidth() const;
	U32 GetHeight() const;
	U32 GetLevels() const;
	U32 GetFlags() const;
	E_Multisample GetMultisample() const;

	U32 GetColourFlags(const U32 i) const;
	E_SurfaceFormat GetColourSurfaceFormat(const U32 i) const;
	E_TextureFormat GetColourTextureFormat(const U32 i) const;

	U32 GetDepthStencilFlags() const;
	E_SurfaceFormat GetDepthStencilSurfaceFormat() const;
	E_TextureFormat GetDepthStencilTextureFormat() const;

	C_Surface* GetColourSurface(const U32 i);
	const C_Surface* GetColourSurface(const U32 i) const;
	C_Texture* GetColourTexture(const U32 i);
	const C_Texture* GetColourTexture(const U32 i) const;

	C_Surface* GetDepthStencilSurface();
	const C_Surface* GetDepthStencilSurface() const;
	C_Texture* GetDepthStencilTexture();
	const C_Texture* GetDepthStencilTexture() const;

	void SetName(const char* name);

	void SetWidth(const U32 width);
	void SetHeight(const U32 height);
	void SetLevels(const U32 levels);
	void SetFlags(const U32 flags);
	void SetMultisample(const E_Multisample multisample);
	
	void SetColourFlags(const U32 i, const U32 flags);
	void SetColourOffset(const U32 i, const U32 offset);
	void SetColourAddress(const U32 i, U8* pAddress);
	void SetColourFormat(const U32 i, const E_TextureFormat textureFormat);
	void SetColourSurfaceFormat(const U32 i, const E_SurfaceFormat surfaceFormat);
	void SetColourTextureFormat(const U32 i, const E_TextureFormat textureFormat);
	void SetColourTileEnable(const U32 i, const bool enable, const bool indirect = false);
		
	void SetDepthStencilFlags(const U32 flags);
	void SetDepthStencilOffset(const U32 offset);
	void SetDepthStencilAddress(U8* pAddress);
	void SetDepthStencilFormat(const E_TextureFormat textureFormat);
	void SetDepthStencilSurfaceFormat(const E_SurfaceFormat surfaceFormat);
	void SetDepthStencilTextureFormat(const E_TextureFormat textureFormat);
	void SetDepthStencilTileEnable(const bool enable, const bool indirect = false);
	void SetDepthStencilZcullEnable(const bool enable);
	
	void ChangeResolution(const U32 width, const U32 height, const E_Multisample multisample);
	void RestoreResolution();
	void UpdateResolution();

	void OverlapColourBuffer(const U32 i, const U32 size, U8* pAddress);
	void OverlapDepthStencilBuffer(const U32 size, U8* pAddress);

	// For compatibility
	bool IsCreated() const;
	C_Surface& GetSurface();
	const C_Surface& GetSurface() const;
	C_Texture& GetTexture();
	const C_Texture& GetTexture() const;
	
	Util::C_String32							m_Name;
	U32											m_Width;
	U32											m_Height;
	U32											m_Levels;
	U32											m_Flags;
	E_Multisample								m_Multisample;
	
	U32											m_ColourFlags[4];
	
	U32											m_ColourSize[4];
	U32											m_ColourOffset[4];
	U8*											m_pColourAddress[4];
	
	E_SurfaceFormat								m_ColourSurfaceFormat[4];
	E_TextureFormat								m_ColourTextureFormat[4];
	C_TileInfo									m_ColourTileInfo[4];
	C_RenderTexture2D							m_ColourBuffer[4];
	
	U32											m_DepthStencilFlags;
	
	U32											m_DepthStencilSize;
	U32											m_DepthStencilOffset;
	U8*											m_pDepthStencilAddress;

	E_SurfaceFormat								m_DepthStencilSurfaceFormat;
	E_TextureFormat								m_DepthStencilTextureFormat;
	C_TileInfo									m_DepthStencilTileInfo;
	C_ZcullInfo									m_DepthStencilZcullInfo;
	C_RenderTexture2D							m_DepthStencilBuffer;

	U32											m_OldWidth;
	U32											m_OldHeight;
	E_Multisample								m_OldMultisample;
};

} // namespace Display2

#include "Display2/Context/GCMContext.inl"

#endif // DISPLAY2_GCM_CONTEXT_H
