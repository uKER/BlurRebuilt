#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include <system/serialise/C_MemoryStream.h>
#include <system/serialise/FilePath.h>

//#pragma optimize ("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata.streamer.shared");

namespace RenderingData {
namespace Target
{

IndexBuffer::~IndexBuffer()
{
	Display2::Device()->DestroyIndexBuffer(&m_data);
}

VertexBuffer::~VertexBuffer()
{
	Display2::Device()->DestroyVertexBuffer(&m_data);
}

Texture::~Texture()
{
	Destroy();
}

void Texture::Destroy()
{
	if (m_data.IsCreated())
		Display::Device()->DestroyTexture2D(&m_data);

	m_headers.Destroy();
}

bool Texture::Create(const char* fileName)
{
	m_name = Sys::RemoveFileExtension(Sys::ExtractFilename(fileName)).Chars();
	m_samplerStateIndex = 0; // to do: set defaults
	m_hashValue = 0;
	
	const bool result = Display::CreateTexture2DFromFile(fileName, &m_data);
	m_data.SetFlags( m_data.GetFlags() | Display::E_Usage_DoNotReload );

	return result;
}

///////////////////////////////////////////////

void ResourceBlock::Resources::Destroy()
{
	vertexBuffers.Destroy();
	indexBuffers.Destroy();
	textures.Destroy();
	nodes.Destroy();
	lod = -1;
}

void ResourceBlock::Resources::RemoveFromDevice()
{
#if TRACK_DISPLAY2_RESOURCES
	for (int i = 0; i < textures.Size(); i++)
	{
		if (textures[i] != 0 && textures[i]->m_data.IsCreated())
			Display2::Device()->RemoveTexture(&textures[i]->m_data);
	}
#endif
}

ResourceBlock::Resources::~Resources()
{
	Destroy();
}

bool ResourceBlock::Resources::Empty() const
{
	return lod == -1;
}

void ResourceBlock::ReduceLod(const ResourceBlock::Resources* pFront, const ResourceBlock::Resources& back, ResourceBlock::Resources& dest)
{
	assert(dest.Empty());
	assert(back.lod > -1);

	dest.lod = back.lod - 1;
	dest.discard = false;

	if (back.lod == 0)
		return;

	dest.textures.Resize(back.textures.Size());

	for (int i = 0; i < back.textures.Size(); i++)
	{
		const Texture* pTexture = 0;

		if (back.textures[i] != 0 && back.textures[i]->GetData().IsCreated())
			pTexture = &*back.textures[i];
		else if (pFront && pFront->textures[i] != 0 && pFront->textures[i]->GetData().IsCreated())
			pTexture = &*pFront->textures[i];

		assert(pTexture);

		assert(!(pTexture->GetData().GetFlags() & Display::E_Usage_Indirect));

		if (pTexture != 0 && pTexture->m_headers.Back().lod == back.lod)
		{
			Allocate(dest.textures[i]);

			dest.textures[i]->m_headers = pTexture->m_headers;
			dest.textures[i]->m_headers.PopBack();

			dest.textures[i]->m_hashValue = pTexture->m_hashValue;
			dest.textures[i]->m_name = pTexture->m_name;
			dest.textures[i]->m_samplerStateIndex = pTexture->m_samplerStateIndex;

			S32 nHeaders = dest.textures[i]->m_headers.Size();

			if (nHeaders > 0)
			{
				const Texture::HeaderData& header = dest.textures[i]->m_headers[nHeaders - 1];
				Display::CloneTexture2D(&dest.textures[i]->m_data, &pTexture->m_data, header.width, header.height, header.levels, header.size);
				dest.textures[i]->m_data.SetFlags( dest.textures[i]->m_data.GetFlags() | Display::E_Usage_DoNotReload | Display::E_Usage_Streamed);
				if (dest.textures[i]->m_data.IsCreated())
				{
#ifdef PLATFORM_PC
					Display::CopyTextureData2D(&dest.textures[i]->m_data, &pTexture->m_data);
#else
					Display::C_Texture2D *pDst = &dest.textures[i]->m_data;
					const Display::C_Texture2D *pSrc = &pTexture->m_data;
					int size = Maths::Min(pSrc->GetSize(), pDst->GetSize());
					int dstOffset = Maths::Max((S32) pDst->GetSize() - (S32) pSrc->GetSize(), (S32) 0);
					int srcOffset = Maths::Max((S32) pSrc->GetSize() - (S32) pDst->GetSize(), (S32) 0);
					StreamManager::Instance().AsynchCopy(pDst->GetAddress() + dstOffset, pSrc->GetAddress() + srcOffset, size, StreamManager::ASYNCH_COPY_GPU_TO_GPU);
#endif
					#if TRACK_DISPLAY2_RESOURCES
					Display2::Device()->SetTextureName(&dest.textures[i]->m_data, pTexture->GetName());
					#endif
				}
				else
				{
					dest.discard = true;
					break;
				}
			}
		}
	}
}

void ResourceBlock::SwapNew(Resources& dest, Resources& source)
{
	assert(source.lod != dest.lod);

	if (source.discard)
	{
		dest.discard = true;
		return;
	}

	if (dest.lod == -1 || source.lod == -1)
	{
		// One of them is empty; swap everything.
		Swap(dest.vertexBuffers, source.vertexBuffers);
		Swap(dest.indexBuffers, source.indexBuffers);
		Swap(dest.nodes, source.nodes);
		Swap(dest.textures, source.textures);
	}
	else
	{
		// Vertex and index buffers are in m_resources.
		for (int i = 0; i < dest.textures.Size(); i++)
		{
			if (source.textures[i] != 0 && source.textures[i]->GetData().IsCreated())
			{
				Swap(dest.textures[i], source.textures[i]);

				if (source.textures[i] != 0)
					source.textures[i]->m_headers.Clear();
			}
			else if (dest.textures[i] != 0 && dest.textures[i]->GetData().IsCreated()
						&& dest.textures[i]->m_headers.Back().lod == dest.lod
						&& source.lod < dest.lod)
			{
				Swap(dest.textures[i], source.textures[i]);
			}
		}
	}

	Btl::Swap(dest.lod, source.lod);
}

void Swap(ResourceBlock::Resources& a, ResourceBlock::Resources& b)
{
	Btl::Swap(a.vertexBuffers, b.vertexBuffers);
	Btl::Swap(a.indexBuffers, b.indexBuffers);
	Btl::Swap(a.nodes, b.nodes);
	Btl::Swap(a.textures, b.textures);
	Btl::Swap(a.lod, b.lod);
	Btl::Swap(a.pScene, b.pScene);
}

///////////////////////////////////////////

void ResourceBlock::Destroy()
{
	m_resources.Destroy();
}

void ResourceBlock::SetChunk(S32 chunkIndex)
{
	for (int i = 0; i < m_back.nodes.Size(); i++)
	{
		if (m_back.nodes[i] != 0)
		{
			if (m_back.nodes[i]->IsNode(NODE_TYPE_RENDER_LIST_PLATFORM))
			{
				Target::RenderListNode* p =(Target::RenderListNode*) Pointer(m_back.nodes[i]);
				p->SetChunk(chunkIndex);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////

SharedTextureList::~SharedTextureList()
{
	g_resourceAllocatorMutex.Wait();

	for (int i = 0; i < m_textures.Size(); i++)
		SharedTextureManager::Instance().ReleaseTexture(m_textures[i]);

	g_resourceAllocatorMutex.Release();
}

void SharedTextureList::AccumulateTextureInfo(TextureInfoList& ioTextureInfoList) const
{
	if (!m_textures.Empty())
	{
		S32 dstIndex = ioTextureInfoList.Size();
		ioTextureInfoList.Resize(ioTextureInfoList.Size() + m_textures.Size());

		SharedTextureManager& manager = SharedTextureManager::Instance();

		for (Btl::Vector<const Texture*>::ConstIterator it = m_textures.Begin(); it != m_textures.End(); ++it)
		{
			TextureInfo& dst = ioTextureInfoList[dstIndex++];
			dst.pTexture = *it;
			dst.flags = (manager.GetCurrentSharing(*it) > 0) ? TextureInfo::E_Flag_Shared : 0;
		}
	}
}

const Texture* SharedTextureManager::InsertTexture(const char* name, U32 hashValue)
{
	for (TextureList::Iterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
	{
		if (stricmp(i->texture.GetName(), name) == 0 
			&& i->texture.GetHashValue() == hashValue)
		{
			i->referenceCount++;
			return &i->texture;
		}
	}

	m_textureList.PushBack();
	m_textureList.Back().referenceCount++;
	return &m_textureList.Back().texture;
}

SharedTextureData* SharedTextureManager::FindTexture(const char* name, U32 hashValue)
{
	for (TextureList::Iterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
	{
		if (stricmp(i->texture.GetName(), name) == 0 
			&& i->texture.GetHashValue() == hashValue)
		{
			return &*i;
		}
	}

	return NULL;
}

void SharedTextureManager::CheckPotentialTextureSharing(const char* name, U32 hashValue)
{
	SharedTextureData* p = FindTexture(name, hashValue);
	if (p)
		p->potentialShareCount = 1;
}

S32 SharedTextureManager::GetTotalPotentialSharing() const
{
	S32 total = 0;

	for (TextureList::ConstIterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
	{
		if (i->potentialShareCount)
		{
			total += i->texture.GetData().GetSize();
		}
	}

	return total;
}

void SharedTextureManager::DumpPotentialTextureSharing() const
{
	if (Debug::GetEffectiveLevel(g_Log) == DebugLevel::Debug)
	{
		S32 total = 0;

		LOGINFO(g_Log, "SharedTextureManager: Listing scene textures that could be shared:");

		for (TextureList::ConstIterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
		{
			if (i->potentialShareCount)
			{
				S32 size = i->texture.GetData().GetSize();
				total += size;
				LOGINFO(g_Log, "%s, %i bytes", i->texture.GetName(), size);
			}
		}

		LOGINFO(g_Log, "SharedTextureManager: %i bytes could be saved by sharing textures with models.", total);
	}
}

void SharedTextureManager::IncreaseRefCount(SharedTextureData *pTexture)
{
	if (pTexture)
		pTexture->referenceCount++;
}

void SharedTextureManager::DecreaseRefCount(SharedTextureData *pTexture)
{
	if (!pTexture)
		return;

	pTexture->referenceCount--;

	if (pTexture->referenceCount == 0)
		ReleaseTexture(&pTexture->texture);
}

void SharedTextureManager::ReleaseTexture(const Texture* p)
{
	for (TextureList::Iterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
	{
		if (&i->texture == p)
		{
			i->referenceCount--;

			if (i->referenceCount <= 0)
			{
				m_textureList.Erase(i);
				return;
			}
		}
	}
}

S32 SharedTextureManager::GetCurrentSharing(const Texture* p)
{
	for (TextureList::Iterator i = m_textureList.Begin(); i != m_textureList.End(); ++i)
	{
		if (&i->texture == p)
		{
			return i->referenceCount;
		}
	}

	return 0;
}

}}
