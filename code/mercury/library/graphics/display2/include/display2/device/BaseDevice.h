#ifndef DISPLAY2_BASE_DEVICE_H
#define DISPLAY2_BASE_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display2/Context/Context.h"

#include <Core/Threads/Mutex.h>
#include <System/Serialise/FilePath.h>
#include <Btl/Containers/ChainedHashMap.h>
#include <Btl/Allocators/Pool.h>
#include <Btl/Containers/Set.h>

namespace Display2
{

enum E_ResourceList
{
	E_ResourceList_VertexBuffer,
	E_ResourceList_IndexBuffer,
	E_ResourceList_VertexShader,
	E_ResourceList_PixelShader,
	E_ResourceList_Surface,
	E_ResourceList_Texture,
	E_ResourceList_RenderTexture2D,
	E_ResourceList_RenderTextureCube,
	E_ResourceList_RenderBuffer,
	E_ResourceList_Unknown,
};

struct C_ResourceInfo
{
	C_ResourceInfo();

	U64												m_Time;
	Util::C_String									m_Name;
};

struct C_ResourceSort;

class C_ResourceList
{
public:

	C_ResourceList();
	~C_ResourceList();

	typedef Btl::ChainedHashMap<void*, C_ResourceInfo*, Btl::HashKeyOpDefault<void*>, Btl::DynamicPool<> > ResourceInfoMap;

	S32 GetResourceCount() const;
	ResourceInfoMap::Iterator GetResourceBegin();
	ResourceInfoMap::Iterator GetResourceEnd();

	void InsertResource(void* pResource);
	void RemoveResource(void* pResource);

	C_ResourceInfo* GetResourceInfo(void* pResource);
	void SetResourceName(void* pResource, const Util::C_String& name);

	void LockResources();
	void UnlockResources();

private:

	MT::Mutex			m_Lock;
	ResourceInfoMap		*m_pList;
};

typedef C_ResourceList::ResourceInfoMap::Iterator C_ResourceListIt;

class C_ResourceTracker
{
public:

	C_ResourceTracker();
	~C_ResourceTracker();
	
	S32 GetVertexBufferCount() const;
	C_ResourceListIt GetVertexBufferBegin();
	C_ResourceListIt GetVertexBufferEnd();
	void InsertVertexBuffer(C_VertexBuffer* pVertexBuffer);
	void RemoveVertexBuffer(C_VertexBuffer* pVertexBuffer);
	C_ResourceInfo* GetVertexBufferResourceInfo(C_VertexBuffer* pVertexBuffer);

	S32 GetIndexBufferCount() const;
	C_ResourceListIt GetIndexBufferBegin();
	C_ResourceListIt GetIndexBufferEnd();
	void InsertIndexBuffer(C_IndexBuffer* pIndexBuffer);
	void RemoveIndexBuffer(C_IndexBuffer* pIndexBuffer);
	C_ResourceInfo* GetIndexBufferResourceInfo(C_IndexBuffer* pIndexBuffer);

	S32 GetVertexShaderCount() const;
	C_ResourceListIt GetVertexShaderBegin();
	C_ResourceListIt GetVertexShaderEnd();
	void InsertVertexShader(C_VertexShader* pVertexShader);
	void RemoveVertexShader(C_VertexShader* pVertexShader);
	C_ResourceInfo* GetVertexShaderResourceInfo(C_VertexShader* pVertexShader);
	void SetVertexShaderName(C_VertexShader* pVertexShader, const Util::C_String& name);

	S32 GetPixelShaderCount() const;
	C_ResourceListIt GetPixelShaderBegin();
	C_ResourceListIt GetPixelShaderEnd();
	void InsertPixelShader(C_PixelShader* pPixelShader);
	void RemovePixelShader(C_PixelShader* pPixelShader);
	C_ResourceInfo* GetPixelShaderResourceInfo(C_PixelShader* pPixelShader);
	void SetPixelShaderName(C_PixelShader* pPixelShader, const Util::C_String& name);

	S32 GetSurfaceCount() const;
	C_ResourceListIt GetSurfaceBegin();
	C_ResourceListIt GetSurfaceEnd();
	void InsertSurface(C_Surface* pSurface);
	void RemoveSurface(C_Surface* pSurface);
	C_ResourceInfo* GetSurfaceResourceInfo(C_Surface* pSurface);
	void SetSurfaceName(C_Surface* pSurface, const Util::C_String& name);

	S32 GetTextureCount() const;
	C_ResourceListIt GetTextureBegin();
	C_ResourceListIt GetTextureEnd();
	void InsertTexture(C_Texture* pTexture);
	void RemoveTexture(C_Texture* pTexture);
	C_ResourceInfo* GetTextureResourceInfo(C_Texture* pTexture);
	void SetTextureName(C_Texture* pTexture, const Util::C_String& name);

	S32 GetRenderTexture2DCount() const;
	C_ResourceListIt GetRenderTexture2DBegin();
	C_ResourceListIt GetRenderTexture2DEnd();
	void InsertRenderTexture2D(C_RenderTexture2D* pRenderTexture2D);
	void RemoveRenderTexture2D(C_RenderTexture2D* pRenderTexture2D);
	C_ResourceInfo* GetRenderTexture2DResourceInfo(C_RenderTexture2D* pRenderTexture2D);
	void SetRenderTexture2DName(C_RenderTexture2D* pRenderTexture2D, const Util::C_String& name);

	S32 GetRenderTextureCubeCount() const;
	C_ResourceListIt GetRenderTextureCubeBegin();
	C_ResourceListIt GetRenderTextureCubeEnd();
	void InsertRenderTextureCube(C_RenderTextureCube* pRenderTextureCube);
	void RemoveRenderTextureCube(C_RenderTextureCube* pRenderTextureCube);
	C_ResourceInfo* GetRenderTextureCubeResourceInfo(C_RenderTextureCube* pRenderTextureCube);
	void SetRenderTextureCubeName(C_RenderTextureCube* pRenderTextureCube, const Util::C_String& name);

	S32 GetRenderBufferCount() const;
	C_ResourceListIt GetRenderBufferBegin();
	C_ResourceListIt GetRenderBufferEnd();
	void InsertRenderBuffer(C_RenderBuffer* pRenderBuffer);
	void RemoveRenderBuffer(C_RenderBuffer* pRenderBuffer);
	C_ResourceInfo* GetRenderBufferResourceInfo(C_RenderBuffer* pRenderBuffer);
	void SetRenderBufferName(C_RenderBuffer* pRenderBuffer, const Util::C_String& name);

	void DumpVertexBuffers(const S32 start = 0, const S32 count = -1);
	void DumpIndexBuffers(const S32 start = 0, const S32 count = -1);
	void DumpVertexShaders(const S32 start = 0, const S32 count = -1);
	void DumpPixelShaders(const S32 start = 0, const S32 count = -1);
	void DumpSurfaces(const S32 start = 0, const S32 count = -1);
	void DumpTextures(const S32 start = 0, const S32 count = -1);
	void DumpRenderTexture2Ds(const S32 start = 0, const S32 count = -1);
	void DumpRenderTextureCubes(const S32 start = 0, const S32 count = -1);
	void DumpRenderBuffers(const S32 start = 0, const S32 count = -1);
	void DumpResources();

	void CheckVertexBufferTrashing(const U32 size, const void* pAddress, const void* pResource);
	void CheckIndexBufferTrashing(const U32 size, const void* pAddress, const void* pResource);
	void CheckSurfaceTrashing(const U32 size, const void* pAddress, const void* pResource);
	void CheckTextureTrashing(const U32 size, const void* pAddress, const void* pResource);
	void CheckResourceTrashing(const U32 size, const void* pAddress, const void* pResource);

	bool FindVertexBufferFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo);
	bool FindIndexBufferFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo);
	bool FindTextureFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo);
	bool FindSurfaceFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo);
	bool FindResourceFromAddress(const void* pAddress, void** ppResource, void** pResourceInfo, E_ResourceType& resourceType);

private:

	C_ResourceList& GetResources(const E_ResourceList resourceType);
	void LockResources(const E_ResourceList resourceType, Btl::Vector<C_ResourceSort>& sortList);
	void UnlockResources(const E_ResourceList resourceType, Btl::Vector<C_ResourceSort>& sortList);

public:

	C_ResourceList									m_VertexBufferList;
	C_ResourceList									m_IndexBufferList;
	C_ResourceList									m_VertexShaderList;
	C_ResourceList									m_PixelShaderList;
	C_ResourceList									m_SurfaceList;
	C_ResourceList									m_TextureList;
	C_ResourceList									m_RenderTexture2DList;
	C_ResourceList									m_RenderTextureCubeList;
	C_ResourceList									m_RenderBufferList;

private:
	
	C_ResourceList									m_UnknownList;
	E_ResourceList									m_LockedType;
};

struct C_ProtectedMemory
{
	C_ProtectedMemory();

	U32												m_Size;
	const U8*										m_pAddress;
	Util::C_String32								m_Name;
};

class C_ProtectedMemoryManager
{
public:

	C_ProtectedMemoryManager();
	~C_ProtectedMemoryManager();

	void InsertMemory(const U32 size, const U8* pAddress, const char* name);
	void RemoveMemory(const U32 size, const U8* pAddress);
	void CheckMemory(const U32 size, const U8* pAddress);

private:

	C_ProtectedMemory* FindItem(const U32 size, const U8* pAddress);

private:

	Btl::Set<C_ProtectedMemory*>					*m_pList;
	MT::Mutex										m_Lock;
};

#if TRACK_DISPLAY2_RESOURCES
class C_BaseDevice : public C_ResourceTracker
#else
class C_BaseDevice
#endif
{
public:

	C_BaseDevice();
	~C_BaseDevice();

	void LockMutex();
	void UnlockMutex();

	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	void InsertProtectedMemory(const U32 size, const void* pAddress, const char* name);
	void RemoveProtectedMemory(const U32 size, const void* pAddress);
	void CheckProtectedMemory(const U32 size, const void* pAddress);
	#endif

protected:
	
	#if TRACK_DISPLAY2_PROTECTED_MEMORY
	C_ProtectedMemoryManager						m_ProtectedMemory;
	#endif

	MT::Mutex										m_Mutex;
};

} // namespace Display2

#include "Display2/Device/BaseDevice.inl"

#endif // DISPLAY2_BASE_DEVICE_H
