#ifndef DISPLAY2_GCM_DEVICE_H
#define DISPLAY2_GCM_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Device/BaseDevice.h"
#include "Display2/Buffer/Buffer.h"
#include "Display2/Memory/Memory.h"
#include <System/Memory/MemoryBuffer.h>

#if !CFG_GOLD
//#define TRACK_TILED_MEMORY 1
#endif

namespace Display2
{

#define GPU_ERROR_SUCCESS					0
#define GPU_ERROR_OUT_OF_MEMORY				1

typedef Btl::Set<const C_TileInfo*> C_TileInfoList;

class C_TileInfoManager
{
public:

	~C_TileInfoManager();

	void InsertTiledMemory(const C_TileInfo* pInfo);
	void RemoveTiledMemory(const C_TileInfo* pInfo);
	bool IsTiledMemory(const U32 size, const U32 offset, const E_Location location);
	void DumpTiledMemory();

private:

	C_TileInfoList							m_List;
	MT::Mutex								m_Lock;
};

struct C_ThingyManager
{
	bool Create(const U32 indexCount, const U32 memorySize);
	void Destroy();

	U32 AllocIndex();
	void FreeIndex(const U32 index);

	U32 AllocMemory(const U32 size);
	void FreeMemory(const U32 size, const U32 address);

	#if !CFG_GOLD
	void Dump();
	#endif
	
	Btl::Vector<bool>				m_Indices;
	Btl::Vector<U8>					m_Memory;
	Sys::MemoryBuffer				m_Tracker;
	
	#if !CFG_GOLD
	Btl::Vector<Util::C_String32>	m_Names;
	#endif
};

struct C_TileManager : public C_ThingyManager
{
	U32 AllocIndex();

	U32 AllocMemory(const U32 sizeInBytes);
	void FreeMemory(const U32 sizeInBytes, const U32 startInTiles);
};

struct C_ZcullManager : public C_ThingyManager
{
	U32 AllocIndex();

	U32 AllocMemory(const U32 sizeInBytes);
	void FreeMemory(const U32 sizeInBytes, const U32 startInZcull);
};

class C_DeviceParams
{
public:

	C_DeviceParams();
	~C_DeviceParams();

public:

	U32										m_MainHeapSize;
	U32										m_LocalHeapSize;
	U32										m_CmdBufHeapSize;
	U32										m_MainDeviceHeapSize;
	U32										m_LocalDeviceHeapSize;
};

class C_Device : public C_BaseDevice
{
public:

	C_Device();
	~C_Device();

	void GetParams(C_DeviceParams& params);
	void SetParams(const C_DeviceParams& params);

	U32 IsCreated() const;
	bool Create();
	bool CreateVolatile();
	void DestroyVolatile();
	void Destroy();

	C_Context& GetContext();
	const C_Context& GetContext() const;
	
	C_ContextNoCheck& GetContextNoCheck();
	const C_ContextNoCheck& GetContextNoCheck() const;
	
	C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack();
	const C_ContextNoCheckNoTrack& GetContextNoCheckNoTrack() const;

	C_MemoryManager* GetMainHeap();
	C_MemoryManager* GetLocalHeap();
	C_MemoryManager* GetCmdBufHeap();

	S32 GetVertexDeclarationCount() const;
	C_VertexDeclaration* GetVertexDeclaration(const S32 index);
	C_VertexDeclaration* FindVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);
	
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexFormat& vertexFormat);
	bool CreateVertexDeclaration(const C_VertexFormat& vertexFormat, C_VertexDeclaration* pVertexDeclaration);
	C_VertexDeclaration* CreateVertexDeclaration(const C_VertexElement* pVertexElements);
	bool CreateVertexDeclaration(const C_VertexElement* pVertexElements, C_VertexDeclaration* pVertexDeclaration);
	
	void DestroyVertexDeclaration(C_VertexDeclaration* pVertexDeclaration);
	void Destroy(C_VertexDeclaration* pVertexDeclaration);
	void DestroyVertexDeclarations();

	bool CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* pVertexBuffer, void* pAddress = NULL);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);
	void Destroy(C_VertexBuffer* pVertexBuffer);

	bool CreateIndexBuffer(U32 size, U32 flags, E_IndexFormat format, C_IndexBuffer* pIndexBuffer, void* pAddress = NULL);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);
	void Destroy(C_IndexBuffer* pIndexBuffer);

	bool CreateVertexShader(U32 size, const U8* pData, U32 flags, C_VertexShader* pVertexShader, void* pAddress = NULL);
	void DestroyVertexShader(C_VertexShader* pVertexShader);
	void Destroy(C_VertexShader* pVertexShader);

	bool CreatePixelShader(U32 size, const U8* pData, U32 flags, C_PixelShader* pPixelShader, void* pAddress = NULL);
	void DestroyPixelShader(C_PixelShader* pPixelShader);
	void Destroy(C_PixelShader* pPixelShader);

	bool CreateColourSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress = NULL, U32 addressSize = 0);
	void DestroySurface(C_Surface* pSurface);
	void Destroy(C_Surface* pSurface);

	bool CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	bool CreateTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat format, C_Texture* pTexture, void* pAddress = NULL, U32 addressSize = 0);
	void DestroyTexture(C_Texture* pTexture);
	void Destroy(C_Texture* pTexture);

	bool CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTexture2D(C_RenderTexture2D* pRenderTexture);
	void Destroy(C_RenderTexture2D* pRenderTexture);

	bool CreateRenderTexture2DIndex(const U32 index, const U32 width, const U32 height, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTexture2DIndex(const U32 index, C_RenderTexture2D* pRenderTexture);

	bool CreateRenderTextureCube(const U32 width, const U32 levels, U32 flags, const E_TextureFormat textureFormat, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTextureCube* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTextureCube(C_RenderTextureCube* pRenderTexture);
	void Destroy(C_RenderTextureCube* pRenderTexture);
	
	bool CreateSharedTiledArea(C_SharedTiledArea& tiledArea, const U32 width, const U32 height, U32 flags, const E_SurfaceFormat surfaceFormat, const E_Multisample multisample, const U32 buffers, const char* pName);
	void DestroySharedTiledArea(C_SharedTiledArea& tiledArea);
	bool CreateColourTile(C_TileInfo& tileInfo, U32 size, U8 location, U32 offset, U32 pitch, E_Multisample multisample, const char* pName);
	bool CreateDepthTile(C_TileInfo& tileInfo, U32 size, U8 location, U32 offset, U32 pitch, E_Multisample multisample, const char* pName);
	void DestroyTile(C_TileInfo& tileInfo);

	bool CreateRenderBuffer(C_RenderBuffer* pRenderBuffer, C_RenderBufferSetup& renderBufferSetup);
	void DestroyRenderBuffer(C_RenderBuffer* pRenderBuffer);
	void Destroy(C_RenderBuffer* pRenderBuffer);

	bool AllocResourceMemory(const E_ResourceType eResourceType, U32& flags, const U32 size, U8** ppAddress);
	bool AllocMemory(const U32 size, const U32 align, const U32 flags, U8** ppAddress, const E_MemoryType type = E_MemoryType_Unknown);
	void FreeMemory(const U32 size, U8* pAddress, const E_MemoryType type = E_MemoryType_Unknown, const U32 flags = 0);
	bool IsProtectedMemory(const U32 size, U8* pAddress);

	const C_AlphaTestState& GetAlphaTestState() const;
	const C_BlendState& GetBlendState() const;
	const C_ColourState& GetColourState() const;
	const C_CullState& GetCullState() const;
	const C_DepthState& GetDepthState() const;
	const C_FillState& GetFillState() const;
	const C_SamplerState& GetSamplerState(const U32 index) const;
	const C_ScissorState& GetScissorState() const;
	const C_StencilState& GetStencilState() const;
	const C_VertexSamplerState& GetVertexSamplerState(const U32 index) const;

	void SetAlphaTestState(const C_AlphaTestState& state);
	void SetBlendState(const C_BlendState& state);
	void SetColourState(const C_ColourState& state);
	void SetCullState(const C_CullState& state);
	void SetDepthState(const C_DepthState& state);
	void SetFillState(const C_FillState& state);
	void SetSamplerState(const U32 index, const C_SamplerState& state);
	void SetScissorState(const C_ScissorState& state);
	void SetStencilState(const C_StencilState& state);
	void SetVertexSamplerState(const U32 index, const C_VertexSamplerState& state);

	void SetBlendMode(const E_BlendMode blendMode);
	void SetCullMode(const E_CullMode cullMode);
	void SetFillMode(const E_FillMode fillMode);
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);
	void SetPixelShader(const C_PixelShader* pPixelShader);
	void SetPixelShader(const C_PixelShader* pPixelShader, U32 offset);
	void SetPixelShaderParamF(const C_PixelShaderParam& shaderParam, const F32* pData, const U32 registerCount = 1);
	void SetTexture(const U32 index, const C_Texture* pTexture);
	void SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	void SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration);
	void SetVertexShader(const C_VertexShader* pVertexShader);
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShaderParamF(const C_VertexShaderParam& shaderParam, const F32* pData);
	
	void DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);

	void BlockUntilIdle();

	U32 GetLastError() const;

	void AllowOutOfMemory(const bool allowOutOfMemory);

	void DumpMemoryHeaps();
	void ShowMemoryHeaps();

	bool CreateSurface(U32 width, U32 height, U32 flags, E_SurfaceFormat format, E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, void* pAddress = NULL, const U32 addressSize = 0);

	bool SetTileInfo(C_TileInfo* pTileInfo);
	bool BindTileInfo(C_TileInfo* pTileInfo);
	bool UnbindTileInfo(C_TileInfo* pTileInfo);
	bool AllocTileInfo(C_TileInfo* pTileInfo, const U32 sizeInBytes);
	void FreeTileInfo(C_TileInfo* pTileInfo);
	
	bool BindZcullInfo(C_ZcullInfo* pZcullInfo);
	bool UnbindZcullInfo(C_ZcullInfo* pZcullInfo);
	bool AllocZcullInfo(C_ZcullInfo* pZcullInfo, const U32 sizeInBytes);
	void FreeZcullInfo(C_ZcullInfo* pZcullInfo);

	bool CheckMemoryGuards() const;

	C_TileManager& GetTileManager();
	C_ZcullManager& GetZcullManager();
	
private:

	void InitBlendModes();

	bool CreateMemory();
	void DestroyMemory();

public:

	#if TRACK_TILED_MEMORY
	C_TileInfoManager										m_TileInfoManager;
	#endif

private:

	C_TileManager											m_TileManager;
	C_ZcullManager											m_ZcullManager;

	S32														m_Created;

	U32														m_LastError;

	bool													m_AllowOutOfMemory;

	U32														m_MainHeapSize;
	C_MemoryManager											m_MainHeap;

	U32														m_LocalHeapSize;
	C_MemoryManager											m_LocalHeap;

	U32														m_CmdBufHeapSize;
	U8*														m_pCmdBufHeapAddress;
	C_MemoryManager											m_CmdBufHeap;

	U32														m_MainDeviceHeapSize;
	U8*														m_pMainDeviceHeapAddress;
	C_MemoryManager											m_MainDeviceHeap;

	U32														m_LocalDeviceHeapSize;
	U8*														m_pLocalDeviceHeapAddress;
	C_MemoryManager											m_LocalDeviceHeap;

	C_BlendMode												m_BlendModes[E_BlendMode_Count];

	C_Context												m_Context;
	C_ContextNoCheck										m_ContextNoCheck;
	C_ContextNoCheckNoTrack									m_ContextNoCheckNoTrack;
	C_ContextState											m_ContextState;
	
	Btl::Vector<C_VertexDeclaration*>						m_VertexDeclarationList;
};

class C_DeviceManagerParams
{
public:
	
	C_DeviceManagerParams();
	~C_DeviceManagerParams();

public:

	U32		m_cbSize;
	U32		m_ioSize;
};

class C_DeviceManager
{
public:

	C_DeviceManager();
	~C_DeviceManager();

	bool IsCreated();
	bool Create(const C_DeviceManagerParams& params);
	void Destroy();

	void SetupDefaultDeviceParams(C_DeviceParams& params);

	bool GetVideoMode(C_VideoMode* pVideoMode);

	U32 GetCBSize() const;
	U32 GetIOSize() const;

private:

	void SetupDebugging();

private:

	U32						m_cbSize;
	U32						m_ioSize;
	void*					m_ioAddress;
};

} // namespace Display2

#include "Display2/Device/GCMDevice.inl"

#endif // DISPLAY2_GCM_DEVICE_H
