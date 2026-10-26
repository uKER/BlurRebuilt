#include "StdAfx.h"
#include "Display2/Device/BaseDevice.h"
#include "Display2/Device/ResourceSort.h"
#include <System/Memory/MemoryTracker.h>
#include <algorithm>

namespace Display2
{

Debug::Logger& g_ProtectedMemoryLog = Debug::GetLogger("mercury.display2.protectedmemory");

C_ResourceInfo::C_ResourceInfo() :
	m_Time(0),
	m_Name("")
{
}

C_ResourceList::C_ResourceList()
{
	m_pList = new ResourceInfoMap;
}

C_ResourceList::~C_ResourceList()
{
	BIZ_ASSERT(m_pList->Size() == 0);
	delete m_pList;
}

S32 C_ResourceList::GetResourceCount() const
{
	return m_pList->Size();
}

C_ResourceListIt C_ResourceList::GetResourceBegin()
{
	return m_pList->Begin();
}

C_ResourceListIt C_ResourceList::GetResourceEnd()
{
	return m_pList->End();
}

void C_ResourceList::InsertResource(void* pResource)
{
	m_Lock.Wait();

	BIZ_ASSERT(pResource);
	BIZ_ASSERT_MSG(m_pList->Find(pResource) == m_pList->End(), "Resource already in list. Has it been created then not destroyed then created again?");
	(*m_pList)[pResource] = NULL;

	m_Lock.Release();
}

void C_ResourceList::RemoveResource(void* pResource)
{
	m_Lock.Wait();

	BIZ_ASSERT(pResource);
	if(m_pList->Find(pResource) != m_pList->End())
	{
		SAFE_DELETE_PTR((*m_pList)[pResource]);
		m_pList->Erase(pResource);
	}
	
	m_Lock.Release();
}

C_ResourceInfo* C_ResourceList::GetResourceInfo(void* pResource)
{
	m_Lock.Wait();

	BIZ_ASSERT(pResource);
	BIZ_ASSERT_MSG(m_pList->Find(pResource) != m_pList->End(), "Resource not in list. Has it been created?");
	if((*m_pList)[pResource] == NULL)
		(*m_pList)[pResource] = new C_ResourceInfo();
	C_ResourceInfo* pResourceInfo = (*m_pList)[pResource];

	m_Lock.Release();

	return pResourceInfo;
}

void C_ResourceList::SetResourceName(void* pResource, const Util::C_String& name)
{
	C_ResourceInfo* pResourceInfo = GetResourceInfo(pResource);
	pResourceInfo->m_Name = name;
}

void C_ResourceList::LockResources()
{
	m_Lock.Wait();
}

void C_ResourceList::UnlockResources()
{
	m_Lock.Release();
}

C_ResourceTracker::C_ResourceTracker()
{
	m_LockedType = E_ResourceList_Unknown;
}

C_ResourceTracker::~C_ResourceTracker()
{
}

void C_ResourceTracker::InsertVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexbuffers");
	m_VertexBufferList.InsertResource(pVertexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.vertexbuffers");
}

void C_ResourceTracker::RemoveVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexbuffers");
	m_VertexBufferList.RemoveResource(pVertexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.vertexbuffers");
}

C_ResourceInfo* C_ResourceTracker::GetVertexBufferResourceInfo(C_VertexBuffer* pVertexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexbuffers");
	C_ResourceInfo* pResourceInfo = m_VertexBufferList.GetResourceInfo(pVertexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.vertexbuffers");
	return pResourceInfo;
}

void C_ResourceTracker::InsertIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.indexbuffers");
	m_IndexBufferList.InsertResource(pIndexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.indexbuffers");
}

void C_ResourceTracker::RemoveIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.indexbuffers");
	m_IndexBufferList.RemoveResource(pIndexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.indexbuffers");
}

C_ResourceInfo* C_ResourceTracker::GetIndexBufferResourceInfo(C_IndexBuffer* pIndexBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.indexbuffers");
	C_ResourceInfo* pResourceInfo = m_IndexBufferList.GetResourceInfo(pIndexBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.indexbuffers");
	return pResourceInfo;
}

void C_ResourceTracker::InsertVertexShader(C_VertexShader* pVertexShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexshaders");
	m_VertexShaderList.InsertResource(pVertexShader);
	MEMORY_TRACKER_POP("graphics.display.device.vertexshaders");
}

void C_ResourceTracker::RemoveVertexShader(C_VertexShader* pVertexShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexshaders");
	m_VertexShaderList.RemoveResource(pVertexShader);
	MEMORY_TRACKER_POP("graphics.display.device.vertexshaders");
}

C_ResourceInfo* C_ResourceTracker::GetVertexShaderResourceInfo(C_VertexShader* pVertexShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexshaders");
	C_ResourceInfo* pResourceInfo = m_VertexShaderList.GetResourceInfo(pVertexShader);
	MEMORY_TRACKER_POP("graphics.display.device.vertexshaders");
	return pResourceInfo;
}

void C_ResourceTracker::SetVertexShaderName(C_VertexShader* pVertexShader, const Util::C_String& name)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.vertexshaders");
	m_VertexShaderList.SetResourceName(pVertexShader, name);
	MEMORY_TRACKER_POP("graphics.display.device.vertexshaders");
}

void C_ResourceTracker::InsertPixelShader(C_PixelShader* pPixelShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.pixelshaders");
	m_PixelShaderList.InsertResource(pPixelShader);
	MEMORY_TRACKER_POP("graphics.display.device.pixelshaders");
}

void C_ResourceTracker::RemovePixelShader(C_PixelShader* pPixelShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.pixelshaders");
	m_PixelShaderList.RemoveResource(pPixelShader);
	MEMORY_TRACKER_POP("graphics.display.device.pixelshaders");
}

C_ResourceInfo* C_ResourceTracker::GetPixelShaderResourceInfo(C_PixelShader* pPixelShader)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.pixelshaders");
	C_ResourceInfo* pResourceInfo = m_PixelShaderList.GetResourceInfo(pPixelShader);
	MEMORY_TRACKER_POP("graphics.display.device.pixelshaders");
	return pResourceInfo;
}

void C_ResourceTracker::SetPixelShaderName(C_PixelShader* pPixelShader, const Util::C_String& name)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.pixelshaders");
	m_PixelShaderList.SetResourceName(pPixelShader, name);
	MEMORY_TRACKER_POP("graphics.display.device.pixelshaders");
}

void C_ResourceTracker::InsertSurface(C_Surface* pSurface)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.surfaces");
	m_SurfaceList.InsertResource(pSurface);
	MEMORY_TRACKER_POP("graphics.display.device.surfaces");
}

void C_ResourceTracker::RemoveSurface(C_Surface* pSurface)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.surfaces");
	m_SurfaceList.RemoveResource(pSurface);
	MEMORY_TRACKER_POP("graphics.display.device.surfaces");
}

C_ResourceInfo* C_ResourceTracker::GetSurfaceResourceInfo(C_Surface* pSurface)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.surfaces");
	C_ResourceInfo* pResourceInfo = m_SurfaceList.GetResourceInfo(pSurface);
	MEMORY_TRACKER_POP("graphics.display.device.surfaces");
	return pResourceInfo;
}

void C_ResourceTracker::SetSurfaceName(C_Surface* pSurface, const Util::C_String& name)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.surfaces");
	m_SurfaceList.SetResourceName(pSurface, name);
	MEMORY_TRACKER_POP("graphics.display.device.surfaces");
}

void C_ResourceTracker::InsertTexture(C_Texture* pTexture)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.textures");
	m_TextureList.InsertResource(pTexture);
	MEMORY_TRACKER_POP("graphics.display.device.textures");
}

void C_ResourceTracker::RemoveTexture(C_Texture* pTexture)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.textures");
	m_TextureList.RemoveResource(pTexture);
	MEMORY_TRACKER_POP("graphics.display.device.textures");
}

C_ResourceInfo* C_ResourceTracker::GetTextureResourceInfo(C_Texture* pTexture)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.textures");
	C_ResourceInfo* pResourceInfo = m_TextureList.GetResourceInfo(pTexture);
	MEMORY_TRACKER_POP("graphics.display.device.textures");
	return pResourceInfo;
}

void C_ResourceTracker::SetTextureName(C_Texture* pTexture, const Util::C_String& name)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.textures");
	m_TextureList.SetResourceName(pTexture, name);
	MEMORY_TRACKER_POP("graphics.display.device.textures");
}

void C_ResourceTracker::InsertRenderTexture2D(C_RenderTexture2D* pRenderTexture2D)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexture2ds");
	m_RenderTexture2DList.InsertResource(pRenderTexture2D);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexture2ds");
}

void C_ResourceTracker::RemoveRenderTexture2D(C_RenderTexture2D* pRenderTexture2D)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexture2ds");
	m_RenderTexture2DList.RemoveResource(pRenderTexture2D);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexture2ds");
}

C_ResourceInfo* C_ResourceTracker::GetRenderTexture2DResourceInfo(C_RenderTexture2D* pRenderTexture2D)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexture2ds");
	C_ResourceInfo* pResourceInfo = m_RenderTexture2DList.GetResourceInfo(pRenderTexture2D);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexture2ds");
	return pResourceInfo;
}

void C_ResourceTracker::SetRenderTexture2DName(C_RenderTexture2D* pRenderTexture2D, const Util::C_String& name)
{
	BIZ_ASSERT(pRenderTexture2D);
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexture2ds");
	m_RenderTexture2DList.SetResourceName(pRenderTexture2D, name);
#if PLATFORM_PS3 || PLATFORM_PC
	m_TextureList.SetResourceName(&pRenderTexture2D->m_Texture[0], name);
	#if PLATFORM_PS3
	for(S32 i=0; i<pRenderTexture2D->m_Surfaces[0].Size(); i++)
	{
		m_SurfaceList.SetResourceName(&pRenderTexture2D->m_Surfaces[0][i], name);
	}
	#endif
	if(pRenderTexture2D->GetFlags() & 0x40000000) // E_Usage_ReadWrite
	{
		m_TextureList.SetResourceName(&pRenderTexture2D->m_Texture[1], name);
		#if PLATFORM_PS3
		for(S32 i=0; i<pRenderTexture2D->m_Surfaces[1].Size(); i++)
		{
			m_SurfaceList.SetResourceName(&pRenderTexture2D->m_Surfaces[1][i], name);
		}
		#endif
	}
#else
	m_TextureList.SetResourceName(&pRenderTexture2D->m_Texture, name);
	for(S32 i=0; i<pRenderTexture2D->m_Surfaces.Size(); i++)
	{
		m_SurfaceList.SetResourceName(&pRenderTexture2D->m_Surfaces[i], name);
	}
#endif
	MEMORY_TRACKER_POP("graphics.display.device.rendertexture2ds");
}

void C_ResourceTracker::InsertRenderTextureCube(C_RenderTextureCube* pRenderTextureCube)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexturecubes");
	m_RenderTextureCubeList.InsertResource(pRenderTextureCube);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexturecubes");
}

void C_ResourceTracker::RemoveRenderTextureCube(C_RenderTextureCube* pRenderTextureCube)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexturecubes");
	m_RenderTextureCubeList.RemoveResource(pRenderTextureCube);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexturecubes");
}

C_ResourceInfo* C_ResourceTracker::GetRenderTextureCubeResourceInfo(C_RenderTextureCube* pRenderTextureCube)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexturecubes");
	C_ResourceInfo* pResourceInfo = m_RenderTextureCubeList.GetResourceInfo(pRenderTextureCube);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexturecubes");
	return pResourceInfo;
}

void C_ResourceTracker::SetRenderTextureCubeName(C_RenderTextureCube* pRenderTextureCube, const Util::C_String& name)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.rendertexturecubes");
	m_RenderTextureCubeList.SetResourceName(pRenderTextureCube, name);
	MEMORY_TRACKER_POP("graphics.display.device.rendertexturecubes");
}

void C_ResourceTracker::InsertRenderBuffer(C_RenderBuffer* pRenderBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.renderbuffers");
	m_RenderBufferList.InsertResource(pRenderBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.renderbuffers");
}

void C_ResourceTracker::RemoveRenderBuffer(C_RenderBuffer* pRenderBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.renderbuffers");
	m_RenderBufferList.RemoveResource(pRenderBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.renderbuffers");
}

C_ResourceInfo* C_ResourceTracker::GetRenderBufferResourceInfo(C_RenderBuffer* pRenderBuffer)
{
	MEMORY_TRACKER_PUSH("graphics.display.device.renderbuffers");
	C_ResourceInfo* pResourceInfo = m_RenderBufferList.GetResourceInfo(pRenderBuffer);
	MEMORY_TRACKER_POP("graphics.display.device.renderbuffers");
	return pResourceInfo;
}

void C_ResourceTracker::SetRenderBufferName(C_RenderBuffer* pRenderBuffer, const Util::C_String& name)
{
	BIZ_ASSERT(pRenderBuffer);
	MEMORY_TRACKER_PUSH("graphics.display.device.renderbuffers");
	m_RenderBufferList.SetResourceName(pRenderBuffer, name);
	MEMORY_TRACKER_POP("graphics.display.device.renderbuffers");
}

C_ResourceList& C_ResourceTracker::GetResources(const E_ResourceList resourceType)
{
	switch(resourceType)
	{
	case E_ResourceList_VertexBuffer:		return m_VertexBufferList;
	case E_ResourceList_IndexBuffer:		return m_IndexBufferList;
	case E_ResourceList_VertexShader:		return m_VertexShaderList;
	case E_ResourceList_PixelShader:		return m_PixelShaderList;
	case E_ResourceList_Surface: 			return m_SurfaceList;
	case E_ResourceList_Texture: 			return m_TextureList;
	case E_ResourceList_RenderTexture2D:	return m_RenderTexture2DList;
	case E_ResourceList_RenderTextureCube:	return m_RenderTextureCubeList;
	case E_ResourceList_RenderBuffer:		return m_RenderBufferList;
	default:
		BIZ_ASSERT(0);
		return m_UnknownList;
	}
}

void C_ResourceTracker::LockResources(const E_ResourceList resourceType, Btl::Vector<C_ResourceSort>& sortList)
{
	BIZ_ASSERT(resourceType != E_ResourceList_Unknown);
	BIZ_ASSERT(m_LockedType == E_ResourceList_Unknown);

	sortList.Clear();
	
	C_ResourceList& resourceList = GetResources(resourceType);
	resourceList.LockResources();

	C_ResourceListIt it = resourceList.GetResourceBegin();

	while(it != resourceList.GetResourceEnd())
	{
		void* pResource = it->GetKey();

		C_ResourceSort sortItem;
		switch(resourceType)
		{
		case E_ResourceList_VertexBuffer:		sortItem.m_Size = ((C_VertexBuffer*)pResource)->GetSize();						break;
		case E_ResourceList_IndexBuffer:		sortItem.m_Size = ((C_IndexBuffer*)pResource)->GetSize();						break;
		case E_ResourceList_VertexShader:		sortItem.m_Size = ((C_VertexShader*)pResource)->GetSize();						break;
		case E_ResourceList_PixelShader:		sortItem.m_Size = ((C_PixelShader*)pResource)->GetSize();						break;
		case E_ResourceList_Surface: 			sortItem.m_Size = ((C_Surface*)pResource)->GetSize(); 							break;
		case E_ResourceList_Texture: 			sortItem.m_Size = ((C_Texture*)pResource)->GetSize(); 							break;
		case E_ResourceList_RenderTexture2D:	sortItem.m_Size = ((C_RenderTexture2D*)pResource)->GetTexture().GetSize();		break;
		case E_ResourceList_RenderTextureCube:	sortItem.m_Size = ((C_RenderTextureCube*)pResource)->GetTexture().GetSize();	break;
		case E_ResourceList_RenderBuffer:		sortItem.m_Size = 0;															break;
		default:
			sortItem.m_Size = 0;
			BIZ_ASSERT(0);
			break;
		}
		sortItem.m_Iterator = it;
		sortList.PushBack(sortItem);

		it++;
	}

	std::sort(sortList.Begin(), sortList.End());
	
	m_LockedType = resourceType;
}

void C_ResourceTracker::UnlockResources(const E_ResourceList resourceType, Btl::Vector<C_ResourceSort>& sortList)
{
	BIZ_ASSERT(resourceType == m_LockedType);

	sortList.Clear();

	C_ResourceList& resourceList = GetResources(resourceType);
	resourceList.UnlockResources();

	m_LockedType = E_ResourceList_Unknown;
}

void C_ResourceTracker::DumpVertexBuffers(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_VertexBuffer, sortList);

	LOGINFO(g_Log, "VertexBuffers %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size :    Address :            Flags : Name\n");

	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;
		
		C_ResourceSort sortItem = sortList[i];
		C_VertexBuffer* pVertexBuffer = (C_VertexBuffer*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : 0x%.8x : %16s : %s\n",
			i,
			pVertexBuffer->GetSize(),
			pVertexBuffer->GetAddress(),
			GetFlagsAsString(pVertexBuffer->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}

	UnlockResources(E_ResourceList_VertexBuffer, sortList);
}

void C_ResourceTracker::DumpIndexBuffers(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_IndexBuffer, sortList);

	LOGINFO(g_Log, "IndexBuffers %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size : Format :    Address :            Flags : Name\n");

	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;
		
		C_ResourceSort& sortItem = sortList[i];
		C_IndexBuffer* pIndexBuffer = (C_IndexBuffer*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : %6s : 0x%.8x : %16s : %s\n",
			i,
			pIndexBuffer->GetSize(),
			GetIndexFormatAsString(pIndexBuffer->GetFormat()).Chars(),
			pIndexBuffer->GetAddress(),
			GetFlagsAsString(pIndexBuffer->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}
	
	UnlockResources(E_ResourceList_IndexBuffer, sortList);
}

void C_ResourceTracker::DumpVertexShaders(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_VertexShader, sortList);

	LOGINFO(g_Log, "VertexShaders %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size :    Address :            Flags : Name\n");

	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;
		
		C_ResourceSort sortItem = sortList[i];
		C_VertexShader* pVertexShader = (C_VertexShader*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : 0x%.8x : %16s : %s\n",
			i,
			pVertexShader->GetSize(),
			pVertexShader->GetAddress(),
			GetFlagsAsString(pVertexShader->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}

	UnlockResources(E_ResourceList_VertexShader, sortList);
}

void C_ResourceTracker::DumpPixelShaders(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_PixelShader, sortList);

	LOGINFO(g_Log, "PixelShaders %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size :    Address :            Flags : Name\n");

	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;
		
		C_ResourceSort sortItem = sortList[i];
		C_PixelShader* pPixelShader = (C_PixelShader*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : 0x%.8x : %16s : %s\n",
			i,
			pPixelShader->GetSize(),
			pPixelShader->GetAddress(),
			GetFlagsAsString(pPixelShader->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}

	UnlockResources(E_ResourceList_PixelShader, sortList);
}

void C_ResourceTracker::DumpSurfaces(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_Surface, sortList);

	LOGINFO(g_Log, "Surfaces %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size :  Width : Height : Multisample :    Address :                   Format :                    Flags : Name\n");

	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;
		
		C_ResourceSort sortItem = sortList[i];
		C_Surface* pSurface = (C_Surface*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : %6i : %6i : %11s : 0x%.8x : %24s : %24s : %s\n",
			i,
			pSurface->GetSize(),
			pSurface->GetWidth(),
			pSurface->GetHeight(),
			Display2::GetMultisampleAsString((Display2::E_Multisample)pSurface->GetMultisample()).Chars(),
			pSurface->GetAddress(),
			GetSurfaceFormatAsString(pSurface->GetFormat()).Chars(),
			GetFlagsAsString(pSurface->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}

	UnlockResources(E_ResourceList_Surface, sortList);
}

void C_ResourceTracker::DumpTextures(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_Texture, sortList);

	LOGINFO(g_Log, "Textures %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size : Type :  Width : Height :  Depth : Mips :    Address :           Format :                    Flags : Name\n");
	
	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;

		C_ResourceSort& sortItem = sortList[i];
		C_Texture* pTexture = (C_Texture*)sortItem.m_Iterator->GetKey();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : %4s : %6i : %6i : %6i : %4i : 0x%.8x : %16s : %24s : %s\n",
			i,
			pTexture->GetSize(),
			GetTextureTypeAsString(pTexture->GetType()).Chars(),
			pTexture->GetWidth(),
			pTexture->GetHeight(),
			pTexture->GetDepth(),
			pTexture->GetLevels(),
			pTexture->GetAddress(),
			GetTextureFormatAsString(pTexture->GetFormat()).Chars(),
			GetFlagsAsString(pTexture->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}

	UnlockResources(E_ResourceList_Texture, sortList);
}

void C_ResourceTracker::DumpRenderTexture2Ds(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_RenderTexture2D, sortList);

	LOGINFO(g_Log, "RenderTexture2Ds %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size : Type :  Width : Height :  Depth : Mips :    Address :           Format :\n");
	
	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;

		C_ResourceSort& sortItem = sortList[i];
		C_RenderTexture2D* pRenderTexture = (C_RenderTexture2D*)sortItem.m_Iterator->GetKey();
		C_Texture* pTexture = &pRenderTexture->GetTexture();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : %4s : %6i : %6i : %6i : %4i : 0x%.8x : %16s : %32s : %s\n",
			i,
			pTexture->GetSize(),
			GetTextureTypeAsString(pTexture->GetType()).Chars(),
			pTexture->GetWidth(),
			pTexture->GetHeight(),
			pTexture->GetDepth(),
			pTexture->GetLevels(),
			pTexture->GetAddress(),
			GetTextureFormatAsString(pTexture->GetFormat()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "",
			GetFlagsAsString(pTexture->GetFlags()).Chars());
	}

	UnlockResources(E_ResourceList_RenderTexture2D, sortList);
}

void C_ResourceTracker::DumpRenderTextureCubes(const S32 start, const S32 count)
{
	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_RenderTextureCube, sortList);

	LOGINFO(g_Log, "RenderTextureCubes %i\n", sortList.Size());
	LOGINFO(g_Log, "  No :     Size : Type :  Width : Height :  Depth : Mips :    Address :           Format :                    Flags : Name\n");
	
	for(S32 i=start; i<sortList.Size(); i++)
	{
		if(count != -1 && i >= start+count)
			continue;

		C_ResourceSort& sortItem = sortList[i];
		C_RenderTextureCube* pRenderTexture = (C_RenderTextureCube*)sortItem.m_Iterator->GetKey();
		C_Texture* pTexture = &pRenderTexture->GetTexture();
		C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

		LOGINFO(g_Log, "%4i : %8i : %4s : %6i : %6i : %6i : %4i : 0x%.8x : %16s : %24s : %s\n",
			i,
			pTexture->GetSize(),
			GetTextureTypeAsString(pTexture->GetType()).Chars(),
			pTexture->GetWidth(),
			pTexture->GetHeight(),
			pTexture->GetDepth(),
			pTexture->GetLevels(),
			pTexture->GetAddress(),
			GetTextureFormatAsString(pTexture->GetFormat()).Chars(),
			GetFlagsAsString(pTexture->GetFlags()).Chars(),
			pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
	}
	
	UnlockResources(E_ResourceList_RenderTextureCube, sortList);
}

#if PLATFORM_PS3

void FormatString_RenderBufferHeader(char* str, const U32 strLen)
{
	strcpy(str, " i :");
	strcat(str, "     width :");
	strcat(str, "    height :");
	strcat(str, "    levels :");
	strcat(str, "     flags                 :");
	strcat(str, " name");
}

void FormatString_RenderBuffer(char* str, const U32 strLen, C_RenderBuffer* pRenderBuffer)
{
	sprintf_s(str, sizeof(str), "%s: %9i ", 	str, pRenderBuffer->GetWidth());
	sprintf_s(str, sizeof(str), "%s: %9i ", 	str, pRenderBuffer->GetHeight());
	sprintf_s(str, sizeof(str), "%s: %9i ", 	str, pRenderBuffer->GetLevels());
	sprintf_s(str, sizeof(str), "%s: %24s ", 	str, GetFlagsAsString(pRenderBuffer->GetFlags()).Chars());
}

void FormatString_SurfaceHeader(char* str, const U32 strLen)
{
	strcpy(str, " i : type :");
	strcat(str, "      size :");
	strcat(str, "      flags :");
	strcat(str, "    offset :");
	strcat(str, "    address :");
	strcat(str, "     width :");
	strcat(str, "    height :");
	strcat(str, "    format :");
	strcat(str, "        aa :");
	strcat(str, "     pitch :");
	strcat(str, " name");
}

void FormatString_Surface(char* str, const U32 strLen, C_Surface* pSurface)
{
	sprintf_s(str, strLen, "%s: %9i ",		str, pSurface->GetSize());
	sprintf_s(str, strLen, "%s: 0x%.8x ",	str, pSurface->GetFlags());
	sprintf_s(str, strLen, "%s: %9i ",		str, pSurface->GetOffset());
	sprintf_s(str, strLen, "%s: 0x%.8x ",	str, (U32)pSurface->GetAddress());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pSurface->GetWidth());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pSurface->GetHeight());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pSurface->GetFormat());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pSurface->GetMultisample());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pSurface->GetPitch());
}

void FormatString_TextureHeader(char* str, const U32 strLen)
{
	strcpy(str, " i : type :");
	strcat(str, "      size :");
	strcat(str, "      flags :");
	strcat(str, "    offset :");
	strcat(str, "    address :");
	strcat(str, "     width :");
	strcat(str, "    height :");
	strcat(str, "     depth :");
	strcat(str, "    levels :");
	strcat(str, "    format :");
	strcat(str, "     pitch :");
	strcat(str, "      type :");
	strcat(str, " name");
}

void FormatString_Texture(char* str, const U32 strLen, C_Texture* pTexture)
{
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetSize());
	sprintf_s(str, strLen, "%s: 0x%.8x ",	str, pTexture->GetFlags());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetOffset());
	sprintf_s(str, strLen, "%s: 0x%.8x ", 	str, (U32)pTexture->GetAddress());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetWidth());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetHeight());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetDepth());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetLevels());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetFormat());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetPitch());
	sprintf_s(str, strLen, "%s: %9i ", 		str, pTexture->GetType());
}

void FormatString_TileInfoHeader(char* str, const U32 strLen)
{
	strcpy(str, " i : type :");
	strcat(str, "    enable :");
	strcat(str, "     index :");
	strcat(str, "      base :");
	strcat(str, "      bank :");
	strcat(str, "    offset :");
	strcat(str, "  location :");
	strcat(str, " name");
}

void FormatString_TileInfo(char* str, const U32 strLen, C_TileInfo* pTileInfo, bool& dump)
{
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->enable);
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->index);
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->base);
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->bank);
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->offset);
	sprintf_s(str, strLen, "%s: %9i ",		str, pTileInfo->location);

	dump = pTileInfo->index != INVALID_TILE_INDEX ? true : false;
}

void FormatString_ZcullInfoHeader(char* str, const U32 strLen)
{
	strcpy(str, " i : type :");
	strcat(str, "    enable :");
	strcat(str, "     index :");
	strcat(str, "     start :");
	strcat(str, "    offset :");
	strcat(str, " name");
}

void FormatString_ZcullInfo(char* str, const U32 strLen, C_ZcullInfo* pZcullInfo, bool& dump)
{
	sprintf_s(str, strLen, "%s: %9i ", 		str, pZcullInfo->enable);
	sprintf_s(str, strLen, "%s: %9i ", 		str, pZcullInfo->index);
	sprintf_s(str, strLen, "%s: %9i ", 		str, pZcullInfo->cullStart);
	sprintf_s(str, strLen, "%s: %9i ", 		str, pZcullInfo->offset);

	dump = pZcullInfo->index != INVALID_ZCULL_INDEX ? true : false;
}

#endif

void C_ResourceTracker::DumpRenderBuffers(const S32 start, const S32 count)
{
#if PLATFORM_PS3

	Btl::Vector<C_ResourceSort> sortList;
	LockResources(E_ResourceList_RenderBuffer, sortList);

	char str[2048];

	enum E_Output
	{
		E_Output_RenderBuffer = 0,
		E_Output_Surface,
		E_Output_Texture,
		E_Output_TileInfo,
		E_Output_ZcullInfo,
		E_Output_Count,
	};

	for(U32 output=0; output<E_Output_Count; output++)
	{
		strcpy(str, "");
		switch(output)
		{
		case E_Output_RenderBuffer: sprintf_s(str, sizeof(str), "\nRenderBuffers %i", sortList.Size()); break;
		case E_Output_Surface: sprintf_s(str, sizeof(str), "\nSurfaces"); break;
		case E_Output_Texture: sprintf_s(str, sizeof(str), "\nTextures"); break;
		case E_Output_TileInfo: sprintf_s(str, sizeof(str), "\nTileInfos"); break;
		case E_Output_ZcullInfo: sprintf_s(str, sizeof(str), "\nZcullInfos"); break;
		}
		if(strcmp(str, "") != 0)
			LOGINFO(g_Log, str);

		strcpy(str, "");
		switch(output)
		{
		case E_Output_RenderBuffer: sprintf_s(str, sizeof(str), ""); FormatString_RenderBufferHeader(str, sizeof(str)); break;
		case E_Output_Surface: sprintf_s(str, sizeof(str), ""); FormatString_SurfaceHeader(str, sizeof(str)); break;
		case E_Output_Texture: sprintf_s(str, sizeof(str), ""); FormatString_TextureHeader(str, sizeof(str)); break;
		case E_Output_TileInfo: sprintf_s(str, sizeof(str), ""); FormatString_TileInfoHeader(str, sizeof(str)); break;
		case E_Output_ZcullInfo: sprintf_s(str, sizeof(str), ""); FormatString_ZcullInfoHeader(str, sizeof(str)); break;
		}
		if(strcmp(str, "") != 0)
			LOGINFO(g_Log, str);

		for(S32 buffer=start; buffer<sortList.Size(); buffer++)
		{
			C_ResourceSort& sortItem = sortList[buffer];
			C_RenderBuffer* pRenderBuffer = (C_RenderBuffer*)sortItem.m_Iterator->GetKey();
			C_ResourceInfo* pResourceInfo = sortItem.m_Iterator->GetValue();

			if(output == E_Output_RenderBuffer)
			{
				sprintf_s(str, sizeof(str), "%2i ", buffer);

				FormatString_RenderBuffer(str, sizeof(str), pRenderBuffer);

				LOGINFO(g_Log, "%s: %s\n", str, pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
			}
			else
			{
				for(U32 i=0; i<4; i++)
				{
					if(pRenderBuffer->GetColourSurface(i) && output != E_Output_ZcullInfo)
					{
						bool dump = true;

						sprintf_s(str, sizeof(str), "%2i : col%i ", buffer, i);

						switch(output)
						{
						case E_Output_Surface: FormatString_Surface(str, sizeof(str), pRenderBuffer->GetColourSurface(i)); break;
						case E_Output_Texture: FormatString_Texture(str, sizeof(str), pRenderBuffer->GetColourTexture(i)); break;
						case E_Output_TileInfo: FormatString_TileInfo(str, sizeof(str), &pRenderBuffer->m_ColourTileInfo[i], dump); break;
						}

						if(dump)
							LOGINFO(g_Log, "%s: %s\n", str, pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
					}
				}

				if(pRenderBuffer->GetDepthStencilSurface())
				{
					bool dump = true;

					sprintf_s(str, sizeof(str), "%2i : dep0 ", buffer);

					switch(output)
					{
					case E_Output_Surface: FormatString_Surface(str, sizeof(str), pRenderBuffer->GetDepthStencilSurface()); break;
					case E_Output_Texture: FormatString_Texture(str, sizeof(str), pRenderBuffer->GetDepthStencilTexture()); break;
					case E_Output_TileInfo: FormatString_TileInfo(str, sizeof(str), &pRenderBuffer->m_DepthStencilTileInfo, dump); break;
					case E_Output_ZcullInfo: FormatString_ZcullInfo(str, sizeof(str), &pRenderBuffer->m_DepthStencilZcullInfo, dump); break;
					}

					if(dump)
						LOGINFO(g_Log, "%s: %s\n", str, pResourceInfo ? pResourceInfo->m_Name.Chars() : "");
				}
			}
		}
	}
	
	UnlockResources(E_ResourceList_RenderBuffer, sortList);

#endif
}

void C_ResourceTracker::DumpResources()
{
	if(m_VertexBufferList.GetResourceCount() > 0)
		DumpVertexBuffers();
	if(m_IndexBufferList.GetResourceCount() > 0)
		DumpIndexBuffers();
	if(m_VertexShaderList.GetResourceCount() > 0)
		DumpVertexShaders();
	if(m_PixelShaderList.GetResourceCount() > 0)
		DumpPixelShaders();
	if(m_SurfaceList.GetResourceCount() > 0)
		DumpSurfaces();
	if(m_TextureList.GetResourceCount() > 0)
		DumpTextures();
	if(m_RenderTexture2DList.GetResourceCount() > 0)
		DumpRenderTexture2Ds();
	if(m_RenderTextureCubeList.GetResourceCount() > 0)
		DumpRenderTextureCubes();
	if(m_RenderBufferList.GetResourceCount() > 0)
		DumpRenderBuffers();
}

C_ProtectedMemory::C_ProtectedMemory()
{
	m_Size = 0;
	m_pAddress = NULL;
	m_Name = "";
}

C_ProtectedMemoryManager::C_ProtectedMemoryManager()
{
	m_pList = new Btl::Set<C_ProtectedMemory*>;
}

C_ProtectedMemoryManager::~C_ProtectedMemoryManager()
{
	BIZ_ASSERT(m_pList->Size() == 0);
	delete m_pList;
}

C_ProtectedMemory* C_ProtectedMemoryManager::FindItem(const U32 size, const U8* pAddress)
{
	Btl::Set<C_ProtectedMemory*>::Iterator it = m_pList->Begin();
	while(it != m_pList->End())
	{
		C_ProtectedMemory* pItem = *it;

		if(size == pItem->m_Size && pAddress == pItem->m_pAddress)
			return pItem;

		it++;
	}

	return NULL;
}

void C_ProtectedMemoryManager::InsertMemory(const U32 size, const U8* pAddress, const char* name)
{
	LOGDEBUG(g_ProtectedMemoryLog, "Inserting protected memory of size %i address 0x%.8x name %s\n", size, pAddress, name);
	
	m_Lock.Wait();

	C_ProtectedMemory* pItem = FindItem(size, pAddress);
	BIZ_ASSERT_SPEED(!pItem);
	
	pItem = new C_ProtectedMemory();
	pItem->m_Name = name;
	m_pList->Insert(pItem);
	
	pItem->m_Size = size;
	pItem->m_pAddress = pAddress;
	
	m_Lock.Release();
}

void C_ProtectedMemoryManager::RemoveMemory(const U32 size, const U8* pAddress)
{
	LOGDEBUG(g_ProtectedMemoryLog, "Removing protected memory of size %i address 0x%.8x\n", size, pAddress);

	m_Lock.Wait();

	C_ProtectedMemory* pItem = FindItem(size, pAddress);
	BIZ_ASSERT_SPEED(pItem);
	
	m_pList->Erase(pItem);

	SAFE_DELETE_PTR(pItem);
	
	m_Lock.Release();
}

void C_ProtectedMemoryManager::CheckMemory(const U32 size, const U8* pAddress)
{
	m_Lock.Wait();

	Btl::Set<C_ProtectedMemory*>::Iterator it = m_pList->Begin();
	while(it != m_pList->End())
	{
		C_ProtectedMemory* pItem = *it;
		
		BIZ_ASSERT_MSG_SPEED((pAddress >= pItem->m_pAddress+pItem->m_Size) || (pAddress+size <= pItem->m_pAddress), "Accessing memory (address 0x%.8x size %i end 0x%.8x) which is protected (address 0x%.8x size %i end 0x%.8x name %s)", pAddress, size, pAddress+size, pItem->m_pAddress, pItem->m_Size, pItem->m_pAddress+pItem->m_Size, pItem->m_Name.Chars());
		
		it++;
	}
	
	m_Lock.Release();
}

C_BaseDevice::C_BaseDevice()
{
}

C_BaseDevice::~C_BaseDevice()
{
}

void C_BaseDevice::LockMutex()
{
	m_Mutex.Wait();
}

void C_BaseDevice::UnlockMutex()
{
	m_Mutex.Release();
}

#if TRACK_DISPLAY2_PROTECTED_MEMORY
void C_BaseDevice::InsertProtectedMemory(const U32 size, const void* pAddress, const char* name)
{
	m_ProtectedMemory.InsertMemory(size, (const U8*)pAddress, name);
}

void C_BaseDevice::RemoveProtectedMemory(const U32 size, const void* pAddress)
{
	m_ProtectedMemory.RemoveMemory(size, (const U8*)pAddress);
}

void C_BaseDevice::CheckProtectedMemory(const U32 size, const void* pAddress)
{
	m_ProtectedMemory.CheckMemory(size, (const U8*)pAddress);
}
#endif

S32 C_ResourceTracker::GetVertexBufferCount() const
{
	return m_VertexBufferList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetVertexBufferBegin()
{
	return m_VertexBufferList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetVertexBufferEnd()
{
	return m_VertexBufferList.GetResourceEnd();
}

S32 C_ResourceTracker::GetIndexBufferCount() const
{
	return m_IndexBufferList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetIndexBufferBegin()
{
	return m_IndexBufferList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetIndexBufferEnd()
{
	return m_IndexBufferList.GetResourceEnd();
}

S32 C_ResourceTracker::GetVertexShaderCount() const
{
	return m_VertexShaderList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetVertexShaderBegin()
{
	return m_VertexShaderList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetVertexShaderEnd()
{
	return m_VertexShaderList.GetResourceEnd();
}

S32 C_ResourceTracker::GetPixelShaderCount() const
{
	return m_PixelShaderList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetPixelShaderBegin()
{
	return m_PixelShaderList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetPixelShaderEnd()
{
	return m_PixelShaderList.GetResourceEnd();
}

S32 C_ResourceTracker::GetSurfaceCount() const
{
	return m_SurfaceList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetSurfaceBegin()
{
	return m_SurfaceList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetSurfaceEnd()
{
	return m_SurfaceList.GetResourceEnd();
}

S32 C_ResourceTracker::GetTextureCount() const
{
	return m_TextureList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetTextureBegin()
{
	return m_TextureList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetTextureEnd()
{
	return m_TextureList.GetResourceEnd();
}

S32 C_ResourceTracker::GetRenderTexture2DCount() const
{
	return m_RenderTexture2DList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetRenderTexture2DBegin()
{
	return m_RenderTexture2DList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetRenderTexture2DEnd()
{
	return m_RenderTexture2DList.GetResourceEnd();
}

S32 C_ResourceTracker::GetRenderTextureCubeCount() const
{
	return m_RenderTextureCubeList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetRenderTextureCubeBegin()
{
	return m_RenderTextureCubeList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetRenderTextureCubeEnd()
{
	return m_RenderTextureCubeList.GetResourceEnd();
}

S32 C_ResourceTracker::GetRenderBufferCount() const
{
	return m_RenderBufferList.GetResourceCount();
}

C_ResourceListIt C_ResourceTracker::GetRenderBufferBegin()
{
	return m_RenderBufferList.GetResourceBegin();
}

C_ResourceListIt C_ResourceTracker::GetRenderBufferEnd()
{
	return m_RenderBufferList.GetResourceEnd();
}

#define CHECK_FOR_OVERLAP(_srcName, _srcSize, _srcAddress, _dstSize, _dstAddress, _dstName) \
	BIZ_ASSERT_MSG_SPEED \
	( \
		( (U8*)_srcAddress >= (U8*)_dstAddress+_dstSize ) || ( (U8*)_srcAddress+_srcSize <= (U8*)_dstAddress ), \
		"Accessing %s memory (address 0x%.8x _srcSize %i end 0x%.8x) which is protected (address 0x%.8x _srcSize %i address 0x%.8x name %s)", \
		_srcName, _srcAddress, _srcSize, (U8*)_srcAddress+_srcSize, _dstAddress, _dstSize, (U8*)_dstAddress+_dstSize, _dstName \
	);

void C_ResourceTracker::CheckVertexBufferTrashing(const U32 size, const void* pAddress, const void* pResource)
{
	m_VertexBufferList.LockResources();
	
	C_ResourceList::ResourceInfoMap::Iterator it = m_VertexBufferList.GetResourceBegin();
	while(it != m_VertexBufferList.GetResourceEnd())
	{
		C_VertexBuffer* pVertexBuffer = (C_VertexBuffer*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pResource != (void*)pVertexBuffer)
		{
			CHECK_FOR_OVERLAP("vertex buffer", size, pAddress, pVertexBuffer->GetSize(), pVertexBuffer->GetAddress(), pResourceInfo->m_Name.Chars());
		}
	}

	m_VertexBufferList.UnlockResources();
}

void C_ResourceTracker::CheckIndexBufferTrashing(const U32 size, const void* pAddress, const void* pResource)
{
	m_IndexBufferList.LockResources();

	C_ResourceList::ResourceInfoMap::Iterator it = m_IndexBufferList.GetResourceBegin();
	while(it != m_IndexBufferList.GetResourceEnd())
	{
		C_IndexBuffer* pIndexBuffer = (C_IndexBuffer*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pResource != (void*)pIndexBuffer)
		{
			CHECK_FOR_OVERLAP("index buffer", size, pAddress, pIndexBuffer->GetSize(), pIndexBuffer->GetAddress(), pResourceInfo->m_Name.Chars());
		}
	}

	m_IndexBufferList.UnlockResources();
}

void C_ResourceTracker::CheckSurfaceTrashing(const U32 size, const void* pAddress, const void* pResource)
{
	m_SurfaceList.LockResources();

	C_ResourceList::ResourceInfoMap::Iterator it = m_SurfaceList.GetResourceBegin();
	while(it != m_SurfaceList.GetResourceEnd())
	{
		C_Surface* pSurface = (C_Surface*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pResource != (void*)pSurface)
		{
			CHECK_FOR_OVERLAP("surface", size, pAddress, pSurface->GetSize(), pSurface->GetAddress(), pResourceInfo->m_Name.Chars());
		}
	}

	m_SurfaceList.UnlockResources();
}

void C_ResourceTracker::CheckTextureTrashing(const U32 size, const void* pAddress, const void* pResource)
{
	m_TextureList.LockResources();

	C_ResourceList::ResourceInfoMap::Iterator it = m_TextureList.GetResourceBegin();
	while(it != m_TextureList.GetResourceEnd())
	{
		C_Texture* pTexture = (C_Texture*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pResource != (void*)pTexture)
		{
			CHECK_FOR_OVERLAP("texture", size, pAddress, pTexture->GetSize(), pTexture->GetAddress(), pResourceInfo->m_Name.Chars());
		}
	}

	m_TextureList.UnlockResources();
}

void C_ResourceTracker::CheckResourceTrashing(const U32 size, const void* pAddress, const void* pResource)
{
	CheckVertexBufferTrashing(size, pAddress, pResource);
	CheckIndexBufferTrashing(size, pAddress, pResource);
	CheckSurfaceTrashing(size, pAddress, pResource);
	CheckTextureTrashing(size, pAddress, pResource);
}

bool C_ResourceTracker::FindVertexBufferFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo)
{
	m_VertexBufferList.LockResources();
	
	C_ResourceList::ResourceInfoMap::Iterator it = m_VertexBufferList.GetResourceBegin();
	while(it != m_VertexBufferList.GetResourceEnd())
	{
		C_VertexBuffer* pVertexBuffer = (C_VertexBuffer*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pVertexBuffer->GetAddress() == pAddress)
		{
			*ppResource = pVertexBuffer;
			*ppResourceInfo = pResourceInfo;
			m_VertexBufferList.UnlockResources();
			return true;
		}
	}

	m_VertexBufferList.UnlockResources();
	return false;
}

bool C_ResourceTracker::FindIndexBufferFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo)
{
	m_IndexBufferList.LockResources();
	
	C_ResourceList::ResourceInfoMap::Iterator it = m_IndexBufferList.GetResourceBegin();
	while(it != m_IndexBufferList.GetResourceEnd())
	{
		C_IndexBuffer* pIndexBuffer = (C_IndexBuffer*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pIndexBuffer->GetAddress() == pAddress)
		{
			*ppResource = pIndexBuffer;
			*ppResourceInfo = pResourceInfo;
			m_IndexBufferList.UnlockResources();
			return true;
		}
	}

	m_IndexBufferList.UnlockResources();
	return false;
}

bool C_ResourceTracker::FindTextureFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo)
{
	m_TextureList.LockResources();
	
	C_ResourceList::ResourceInfoMap::Iterator it = m_TextureList.GetResourceBegin();
	while(it != m_TextureList.GetResourceEnd())
	{
		C_Texture* pTexture = (C_Texture*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pTexture->GetAddress() == pAddress)
		{
			*ppResource = pTexture;
			*ppResourceInfo = pResourceInfo;
			m_TextureList.UnlockResources();
			return true;
		}
	}

	m_TextureList.UnlockResources();
	return false;
}

bool C_ResourceTracker::FindSurfaceFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo)
{
	m_SurfaceList.LockResources();
	
	C_ResourceList::ResourceInfoMap::Iterator it = m_SurfaceList.GetResourceBegin();
	while(it != m_SurfaceList.GetResourceEnd())
	{
		C_Surface* pSurface = (C_Surface*)it->GetKey();
		C_ResourceInfo* pResourceInfo = it->GetValue();
		it++;

		if(pSurface->GetAddress() == pAddress)
		{
			*ppResource = pSurface;
			*ppResourceInfo = pResourceInfo;
			m_SurfaceList.UnlockResources();
			return true;
		}
	}

	m_SurfaceList.UnlockResources();
	return false;
}

bool C_ResourceTracker::FindResourceFromAddress(const void* pAddress, void** ppResource, void** ppResourceInfo, E_ResourceType& resourceType)
{
	if(FindVertexBufferFromAddress(pAddress, ppResource, ppResourceInfo))
	{
		resourceType = E_ResourceType_VertexBuffer;
		return true;
	}

	if(FindIndexBufferFromAddress(pAddress, ppResource, ppResourceInfo))
	{
		resourceType = E_ResourceType_IndexBuffer;
		return true;
	}

	if(FindTextureFromAddress(pAddress, ppResource, ppResourceInfo))
	{
		resourceType = E_ResourceType_Texture2D;
		return true;
	}
	
	if(FindSurfaceFromAddress(pAddress, ppResource, ppResourceInfo))
	{
		resourceType = E_ResourceType_Surface;
		return true;
	}

	resourceType = E_ResourceType_Unknown;
	return false;
}

} // namespace Display2
