#include "stdafx.h"
#include "renderingdata/build.h"
#include "renderingdata/lods.h"
#include <system/DebugLog.h>
#include "atg/atgstrip.h"
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

//#pragma optimize("", off)

namespace RenderingData {
namespace Build
{

void ResourceBlock::Clear()
{
	m_vertexBuffers.Clear();
	m_indexBuffers.Clear();
	m_textures.Clear();
	m_textureMap.Clear();
	m_nodes.Clear();
}

void ResourceBlock::ClearTextures()
{
	m_textures.Clear();
	m_textureMap.Clear();
}

void ResourceBlock::Copy(const ResourceBlock& t)
{
	Clear();

	for (int i = 0; i < t.m_vertexBuffers.Size(); i++)
	{
		m_vertexBuffers.PushBack();
		Allocate(m_vertexBuffers.Back());
		*m_vertexBuffers.Back() = *t.m_vertexBuffers[i];
	}

	for (int i = 0; i < t.m_indexBuffers.Size(); i++)
	{
		m_indexBuffers.PushBack();
		Allocate(m_indexBuffers.Back());
		*m_indexBuffers.Back() = *t.m_indexBuffers[i];
	}

	for (int i = 0; i < t.m_nodes.Size(); i++)
	{
		m_nodes.PushBack();
		m_nodes.Back() = Clone(t.m_nodes[i]);
		m_nodes.Back()->SetScenePtr(m_pScene);
	}

	m_textures = t.m_textures;
	m_textureMap = t.m_textureMap;
}

S32 ResourceBlock::InsertTexture(const Texture& texture)
{
	TextureMap::InsertionReturnType i = m_textureMap.Insert(texture);

	if (i.inserted)
	{
		m_textures.PushBack(texture);
		i.iterator->GetValue() = m_textures.Size() - 1;
	}

	return i.iterator->GetValue();
}

S32 ResourceBlock::InsertVertexBuffer(const VertexFormat& vertexFormat, S32 batchID, S32 instanceID, S32 streamIndex)
{
	for (int i = 0; i < m_vertexBuffers.Size(); i++)
	{
		if (GetVertexBuffer(i).GetBatchID() == batchID
			&& GetVertexBuffer(i).GetInstanceID() == instanceID
			&& GetVertexBuffer(i).GetStreamIndex() == streamIndex
			&& GetVertexBuffer(i).GetVertices().GetVertexFormat() == vertexFormat)
			return i;
	}

	m_vertexBuffers.PushBack();
	Allocate(m_vertexBuffers.Back());
	m_vertexBuffers.Back()->m_vertices.Create(vertexFormat);
	m_vertexBuffers.Back()->m_batchID = batchID;
	m_vertexBuffers.Back()->m_instanceID = instanceID;
	m_vertexBuffers.Back()->m_streamIndex = streamIndex;

	return m_vertexBuffers.Size() - 1;
}

S32 ResourceBlock::GetSize(S32 lod, PlatformType platformType) const
{
	S32 vertexDataSize = 0;
	S32 indexDataSize = 0;
	S32 textureSize = 0;

	if (lod == 0)
	{
		for (int vb = 0; vb < GetNumVertexBuffers(); vb++)
		{
			S32 vertexSize = GetVertexBuffer(vb).GetVertices().GetVertexFormat().GetDataSize();
			S32 vertexBufferSize = GetVertexBuffer(vb).GetVertices().Size() * vertexSize;
			vertexDataSize += vertexBufferSize;
		}

		for (int ib = 0; ib < GetNumIndexBuffers(); ib++)
		{
			S32 indexSize = (GetIndexBuffer(ib).GetIndices().Size() < 0xffff) ? 2 : 4;
			S32 indexBufferSize = GetIndexBuffer(ib).GetIndices().Size() * indexSize;
			indexDataSize += indexBufferSize;
		}
	}

	for (int it = 0; it < GetNumTextures(); it++)
	{
		// to do: use texture info cache
		TextureExport::TextureInfo info;
		ActTools::Texture &texture = *GetTexture(it).GetTextureManager()->GetTexture(GetTexture(it).GetCacheIndex(), info);
		S32 size = LodCalculator::GetSize(lod, texture, GetTexture(it).GetScaleU(), GetTexture(it).GetScaleV(), platformType);
		textureSize += size;
	}

	return vertexDataSize + indexDataSize + textureSize;
}

// Represents all the vertices indexed by a particular value in an index buffer.
class VertexFinder
{
public:
	typedef Btl::Map<ResourceIndex, VertexBuffer> VertexBufferMap;

	// Either point to new vertex buffers, or existing ones in scene.
	VertexFinder(S32 vertexIndex, Scene* pScene, IndexBuffer* pIndexBuffer) 
		: m_pVertexBuffers(0), m_pScene(pScene), m_pIndexBuffer(pIndexBuffer), m_vertexIndex(vertexIndex) {}

	VertexFinder(S32 vertexIndex, const VertexBufferMap* pVertexBuffers, IndexBuffer* pIndexBuffer, const Scene::BufferRemap* pVertexBufferRemap) 
		: m_pVertexBuffers(pVertexBuffers), m_pScene(0), m_pIndexBuffer(pIndexBuffer), m_vertexIndex(vertexIndex), m_pVertexBufferRemap(pVertexBufferRemap) {}

	VertexData::ConstVertexReference GetVertex(S32 i) const
	{
		ResourceIndex resourceIndex = m_pIndexBuffer->m_vertexBuffers[i];

		if (m_pVertexBuffers)
		{
			ResourceIndex remappedIndex = m_pVertexBufferRemap->Find(resourceIndex)->GetValue().newIndex;
			return m_pVertexBuffers->Find(remappedIndex)->GetValue().GetVertices()[m_vertexIndex];
		}

		assert(m_pScene);

		return m_pScene->GetVertexBuffer(resourceIndex).GetVertices()[m_vertexIndex];
	}

	S32 Index() const {return m_vertexIndex;}

	// Compare vertices.
	inline bool operator < (const VertexFinder& other) const
	{
		for (int i = 0; i != m_pIndexBuffer->m_vertexBuffers.Size(); i++)
		{
			int c = Compare(GetVertex(i), other.GetVertex(i));
			if (c != 0) 
				return (c < 0); 
		}

		return false;
	}

private:
	const VertexBufferMap* m_pVertexBuffers;
	const Scene::BufferRemap* m_pVertexBufferRemap;
	Scene* m_pScene;
	IndexBuffer* m_pIndexBuffer;
	S32 m_vertexIndex;
};

class VertexBufferList
{
public:
	VertexBufferList(const Scene* pScene, const IndexBuffer& indexBuffer)
	{
		for (int i = 0; i < indexBuffer.m_vertexBuffers.Size(); i++)
		{
			VertexType vertexType;
			vertexType.format = pScene->GetVertexBuffer(indexBuffer.m_vertexBuffers[i]).GetVertices().GetVertexFormat();
			vertexType.block = indexBuffer.m_vertexBuffers[i].GetBlock();
			vertexType.stream = pScene->GetVertexBuffer(indexBuffer.m_vertexBuffers[i]).GetStreamIndex();
			vertexType.id = pScene->GetVertexBuffer(indexBuffer.m_vertexBuffers[i]).GetInstanceID();
	
			vertexBuffers.PushBack(vertexType);
		}
	}

	struct VertexType
	{
		VertexFormat format;
		S32 block;
		S32 stream;
		S32 id;
	};

	Btl::Vector<VertexType> vertexBuffers;
};

bool operator < (const VertexBufferList::VertexType& p1, const VertexBufferList::VertexType& p2) 
{
	if (p1.format < p2.format) return true;
	if (p2.format < p1.format) return false;
	if (p1.block < p2.block) return true;
	if (p2.block < p1.block) return false;
	if (p1.stream < p2.stream) return true;
	if (p2.stream < p1.stream) return false;
	if (p1.id < p2.id) return true;
	if (p2.id < p1.id) return false;
	return false;
}

bool operator < (const VertexBufferList& p1, const VertexBufferList& p2)
{
	return p1.vertexBuffers < p2.vertexBuffers;
}

class Counter
{
public:
	Counter() : i(0) {}
	S32 operator ++(int) {return i++;}
	operator const int () const {return i;}

private:
	int i;
};

// Convert triangle list to shared vertices.
void Scene::ShareVertices()
{
	LOGDEBUG(g_Log, "Scene::ShareVertices");

	VertexFinder::VertexBufferMap newResourceVertexBuffers;
	Btl::Map<S32, Counter> vertexBufferCount;
	typedef Btl::Set<VertexFinder> VertexSet;
	typedef Btl::Map<VertexBufferList, VertexSet> VertexMap;
	VertexMap vertexMap;
	Btl::Map<VertexBufferList, Btl::Map<S32, ResourceIndex> > newIndices;

	for (int blockIndex = 0; blockIndex < GetNumResourceBlocks(); blockIndex++)
	{
		for (int indexBufferIndex = 0; indexBufferIndex < GetResourceBlock(blockIndex).GetNumIndexBuffers(); indexBufferIndex++)
		{
			IndexBuffer& indexBuffer(GetResourceBlock(blockIndex).GetIndexBuffer(indexBufferIndex));
					
			VertexMap::InsertionReturnType r = vertexMap.Insert(VertexBufferList(this, indexBuffer));

			// Add vertex buffers to the new map.
			if (r.inserted)
			{
				for (int b = 0; b < indexBuffer.m_vertexBuffers.Size(); b++)
				{
					S32 block = indexBuffer.m_vertexBuffers[b].GetBlock();
					S32 index = vertexBufferCount[block]++;
					ResourceIndex newIndex = ResourceIndex::BlockIndex(block, index);
					VertexBuffer& vertexBuffer = newResourceVertexBuffers[newIndex];
					const VertexFormat& vertexFormat = GetVertexBuffer(indexBuffer.m_vertexBuffers[b]).GetVertices().GetVertexFormat();
					vertexBuffer.GetVertices().Create(vertexFormat);
					vertexBuffer.m_streamIndex = GetVertexBuffer(indexBuffer.m_vertexBuffers[b]).GetStreamIndex();
					vertexBuffer.m_instanceID = GetVertexBuffer(indexBuffer.m_vertexBuffers[b]).GetInstanceID();
					newIndices[r.iterator->GetKey()][b] = newIndex;
				}
			}

			// Map from old to new vertex buffers.
			for (int b = 0; b < indexBuffer.m_vertexBuffers.Size(); b++)
				m_bufferRemap[indexBuffer.m_vertexBuffers[b]] = newIndices[r.iterator->GetKey()][b];

			VertexSet& vertexSet = r.iterator->GetValue();

			IndexBuffer::IndexData newIndexBuffer;

			for (IndexBuffer::IndexData::Iterator pIndex = indexBuffer.GetIndices().Begin(); pIndex != indexBuffer.GetIndices().End(); ++pIndex)
			{
				if (*pIndex == IndexBuffer::ResetIndex32())
				{
					newIndexBuffer.PushBack(*pIndex);
				}
				else
				{
					// Search the set for a matching vertex.
					VertexSet::Iterator found = vertexSet.Find(VertexFinder(*pIndex, this, &indexBuffer));

					// Any buffer will do here as they are all the same size.
					int size = newResourceVertexBuffers[m_bufferRemap[indexBuffer.m_vertexBuffers[0]].newIndex].GetVertices().Size();
					int index = (found != vertexSet.End()) ? found->GetKey().Index() : size;

					// if it is a new vertex, add to all vertex buffers
					if (index == size)
					{
						for (int b = 0; b < indexBuffer.m_vertexBuffers.Size(); b++)
						{
							VertexData::ConstVertexReference vertex = GetVertexBuffer(indexBuffer.m_vertexBuffers[b]).GetVertices()[*pIndex];
							newResourceVertexBuffers[m_bufferRemap[indexBuffer.m_vertexBuffers[b]].newIndex].GetVertices().PushBack(vertex);
						}

						vertexSet.Insert(VertexFinder(index, &newResourceVertexBuffers, &indexBuffer, &m_bufferRemap));
					}

					// add the index
					newIndexBuffer.PushBack(index);
				}
			}

			// replace the old buffers with the new ones
			Swap(indexBuffer.GetIndices(), newIndexBuffer);
		}
	}

	// Erase original vertex buffers.
	for (int blockIndex = 0; blockIndex < GetNumResourceBlocks(); blockIndex++)
	{
		GetResourceBlock(blockIndex).m_vertexBuffers.Clear();
		GetResourceBlock(blockIndex).m_vertexBuffers.Resize(vertexBufferCount[blockIndex]);
	}

	// Copy the new vertex buffers to the blocks.
	for (VertexFinder::VertexBufferMap::Iterator i = newResourceVertexBuffers.Begin(); i != newResourceVertexBuffers.End(); ++i)
	{
		S32 block = i->GetKey().GetBlock();
		S32 index = i->GetKey().GetIndex();

		Allocate(GetResourceBlock(block).m_vertexBuffers[index]);
		Swap(GetVertexBuffer(i->GetKey()).GetVertices(), i->GetValue().GetVertices());
		GetVertexBuffer(i->GetKey()).m_streamIndex = i->GetValue().GetStreamIndex();
		GetVertexBuffer(i->GetKey()).m_instanceID = i->GetValue().GetInstanceID();
	}

	// Remap the blocks.
	for (int blockIndex = 0; blockIndex < GetNumResourceBlocks(); blockIndex++)
	{
		for (int indexBufferIndex = 0; indexBufferIndex < GetResourceBlock(blockIndex).GetNumIndexBuffers(); indexBufferIndex++)
		{
			IndexBuffer& indexBuffer = GetResourceBlock(blockIndex).GetIndexBuffer(indexBufferIndex);

			for (int vertexBufferIndex = 0; vertexBufferIndex < indexBuffer.m_vertexBuffers.Size(); vertexBufferIndex++)
			{
				ResourceIndex oldIndex = indexBuffer.m_vertexBuffers[vertexBufferIndex];
				indexBuffer.m_vertexBuffers[vertexBufferIndex] = m_bufferRemap[oldIndex].newIndex;
			}
		}
	}

	ProcessPrimitiveStreams(&Scene::RemapVertexBuffer);
	m_bufferRemap.Clear();
}

void Scene::CombineVertexBuffers()
{
	LOGDEBUG(g_Log, "Scene::CombineVertexBuffers");

	for (int block = 0; block < GetNumResourceBlocks(); block++)
	{
		typedef Btl::Map<VertexFormat, S32> VertexBufferMap;
		VertexBufferMap vertexBufferMap;
		Btl::Vector<Btl::AutoPointer<VertexBuffer> > newVertexBuffers;

		for (int vertexBufferIndex = 0; vertexBufferIndex < GetResourceBlock(block).GetNumVertexBuffers(); vertexBufferIndex++)
		{
			VertexBuffer& vertexBuffer(GetResourceBlock(block).GetVertexBuffer(vertexBufferIndex));

			VertexBufferMap::InsertionReturnType r = vertexBufferMap.Insert(vertexBuffer.GetVertices().GetVertexFormat());

			if (r.inserted)
			{
				// Copy the old buffer to the new vector.
				r.iterator->GetValue() = newVertexBuffers.Size();
				BufferRemapData& data(m_bufferRemap[ResourceIndex::BlockIndex(block, vertexBufferIndex)]);
				data.newIndex = ResourceIndex::BlockIndex(block, newVertexBuffers.Size());
				data.offset = 0;
				newVertexBuffers.PushBack(GetResourceBlock(block).m_vertexBuffers[vertexBufferIndex]);
			}
			else
			{
				S32 index = r.iterator->GetValue();
				S32 offset = newVertexBuffers[index]->GetVertices().Size();

				if (offset + vertexBuffer.GetVertices().Size() >= 65536) // Doesn't fit, make a new one.
				{
					// Use the smaller buffer for the next attempt.
					if (vertexBuffer.GetVertices().Size() < offset)
						r.iterator->GetValue() = newVertexBuffers.Size();

					BufferRemapData& data(m_bufferRemap[ResourceIndex::BlockIndex(block, vertexBufferIndex)]);
					data.newIndex = ResourceIndex::BlockIndex(block, newVertexBuffers.Size());
					data.offset = 0;
					newVertexBuffers.PushBack(GetResourceBlock(block).m_vertexBuffers[vertexBufferIndex]);
				}
				else
				{
					// Add to the end of the existing buffer.
					S32 vertexSize = vertexBuffer.GetVertices().GetVertexFormat().GetDataSize();

					for (int i = 0; i < vertexBuffer.GetVertices().Size(); i++)
						newVertexBuffers[index]->GetVertices().PushBack(vertexBuffer.GetVertices()[i]);

					BufferRemapData& data(m_bufferRemap[ResourceIndex::BlockIndex(block, vertexBufferIndex)]);
					data.newIndex = ResourceIndex::BlockIndex(block, index);
					data.offset = offset * vertexSize;
				}
			}
		}

		GetResourceBlock(block).m_vertexBuffers = newVertexBuffers;
	}

	ProcessPrimitiveStreams(&Scene::RemapVertexBuffer);
	m_bufferRemap.Clear();
}

void Scene::CombineIndexBuffers()
{
	LOGDEBUG(g_Log, "Scene::CombineIndexBuffers");

	for (int block = 0; block < GetNumResourceBlocks(); block++)
	{
		S32 indexBufferCount = 0;
		typedef Btl::Map<S32, IndexBuffer> IndexBufferMap;
		IndexBufferMap newIndexBuffers;
		Btl::Map<PrimitiveType, S32> primitiveTypeMap;

		// Create new index buffers.
		for (int indexBufferIndex = 0; indexBufferIndex < GetResourceBlock(block).GetNumIndexBuffers(); indexBufferIndex++)
		{
			IndexBuffer& indexBuffer(GetResourceBlock(block).GetIndexBuffer(indexBufferIndex));
			Btl::Map<PrimitiveType, S32>::InsertionReturnType r = primitiveTypeMap.Insert(indexBuffer.GetPrimitiveType());
		
			if (r.inserted)
			{
				r.iterator->GetValue() = indexBufferCount++;
				newIndexBuffers[r.iterator->GetValue()].SetPrimitiveType(indexBuffer.GetPrimitiveType());
			}

			S32 newIndex = r.iterator->GetValue();
			IndexBuffer& newIndexBuffer = newIndexBuffers[newIndex];

			m_bufferRemap[ResourceIndex::BlockIndex(block, indexBufferIndex)].newIndex = ResourceIndex::BlockIndex(block, newIndex);
			m_bufferRemap[ResourceIndex::BlockIndex(block, indexBufferIndex)].offset = newIndexBuffer.GetIndices().Size();

			for (int index = 0; index < indexBuffer.GetIndices().Size(); index++)
				newIndexBuffer.GetIndices().PushBack(indexBuffer.GetIndices()[index]);
		}

		// Erase original index buffers.
		GetResourceBlock(block).m_indexBuffers.Clear();
		GetResourceBlock(block).m_indexBuffers.Resize(indexBufferCount);

		// Copy the new index buffers to the block.
		for (IndexBufferMap::Iterator i = newIndexBuffers.Begin(); i != newIndexBuffers.End(); ++i)
		{
			S32 index = i->GetKey();

			Allocate(GetResourceBlock(block).m_indexBuffers[index]);
			Swap(GetResourceBlock(block).GetIndexBuffer(index).GetIndices(), i->GetValue().GetIndices());
			GetResourceBlock(block).GetIndexBuffer(index).SetPrimitiveType(i->GetValue().GetPrimitiveType());
		}
	}

	ProcessPrimitives(&Scene::RemapIndexBuffer);
	m_bufferRemap.Clear();
}

void Scene::BuildStrips()
{
	LOGDEBUG(g_Log, "Scene::BuildStrips");

	for (int block = 0; block < GetNumResourceBlocks(); block++)
	{
		for (int indexBufferIndex = 0; indexBufferIndex < GetResourceBlock(block).GetNumIndexBuffers(); indexBufferIndex++)
		{
			IndexBuffer& indexBuffer(GetResourceBlock(block).GetIndexBuffer(indexBufferIndex));

			if (indexBuffer.GetPrimitiveType() == PRIMITIVE_TYPE_TRIANGLE_LIST)
			{
				Btl::Vector<U32> newIndices;
				Btl::Vector<U32> newlodSizes;

				for (int lod = 0; lod < indexBuffer.GetNumLods(); lod++)
				{
					//assert(indexBuffer.GetLodSize(lod) > 0);
					if(indexBuffer.GetLodSize(lod) <= 0)
                        goto CouldNotTriStrip;
                    
					Btl::Vector<U16> oldIndices;
					for (int i = indexBuffer.GetLodOffset(lod); i < indexBuffer.GetLodOffset(lod) + indexBuffer.GetLodSize(lod); i++)
					{
						U32 index = indexBuffer.GetIndices()[i];
						if (index < 0xffff)
							oldIndices.PushBack((U16)index);
						else
						{
							LOGWARN(g_Log, "Index too large for tri-stripping");
							goto CouldNotTriStrip;
						}
					}
                    
					DWORD nNewIndices = 0;
					U16* pNewIndices = 0;

					U32 flags = 0;
					if (m_exportData.platformType == PLATFORM_TYPE_360 || m_exportData.platformType == PLATFORM_TYPE_PS3)
						flags = ATGTRISTRIPPER_USE_RESET_INDEX;

					TriStripper::MakeStrips(oldIndices.Size() / 3, &oldIndices[0], &nNewIndices, &pNewIndices, flags);

					if (nNewIndices == 0)
					{
						LOGWARN(g_Log, "Index buffer contained only degenerate triangles.");
						delete pNewIndices;
						goto CouldNotTriStrip;
					}

					for (U16 i = 0; i < nNewIndices; i++)
					{
						if (pNewIndices[i] != IndexBuffer::ResetIndex16())
							newIndices.PushBack(pNewIndices[i]);
						else
							newIndices.PushBack(IndexBuffer::ResetIndex32());
					}

					newlodSizes.PushBack(nNewIndices);

					delete pNewIndices;
				}

				// Copy to the index buffer.
				Swap(indexBuffer.m_indices, newIndices);
				indexBuffer.SetPrimitiveType(PRIMITIVE_TYPE_TRIANGLE_STRIP);
				Swap(indexBuffer.m_lodSizes, newlodSizes);

				CouldNotTriStrip: continue;
			}
		}
	}

	ProcessPrimitives(&Scene::SetPrimitiveType);
}

void Scene::ChangeVertexElementType(VertexElementType originalType, VertexElementType finalType)
{
	LOGDEBUG(g_Log, "Scene::ChangeVertexElementType");

	for (int block = 0; block < GetNumResourceBlocks(); block++)
	{
		for (int vertexBufferIndex = 0; vertexBufferIndex < GetResourceBlock(block).GetNumVertexBuffers(); vertexBufferIndex++)
		{
			VertexBuffer& vertexBuffer(GetResourceBlock(block).GetVertexBuffer(vertexBufferIndex));
			vertexBuffer.GetVertices().ChangeVertexElementType(originalType, finalType);
		}
	}
}

void Scene::ReverseU8_4()
{
	LOGDEBUG(g_Log, "Scene::ReverseU8_4");

	for (int block = 0; block < GetNumResourceBlocks(); block++)
	{
		for (int vertexBufferIndex = 0; vertexBufferIndex < GetResourceBlock(block).GetNumVertexBuffers(); vertexBufferIndex++)
		{
			VertexBuffer& vertexBuffer(GetResourceBlock(block).GetVertexBuffer(vertexBufferIndex));
			vertexBuffer.GetVertices().ReverseU8_4();
		}
	}
}

void Scene::RemapVertexBuffer(PrimitiveStream& stream)
{
	ResourceIndex oldIndex = stream.vertexBufferIndex;
	stream.vertexBufferIndex = m_bufferRemap[oldIndex].newIndex;
	stream.vertexOffset = m_bufferRemap[oldIndex].offset;
}

void Scene::RemapIndexBuffer(Primitive& primitive)
{
	ResourceIndex oldIndex = primitive.indexBufferIndex;
	primitive.indexBufferIndex = m_bufferRemap[oldIndex].newIndex;

	for (int i = 0; i < primitive.lods.Size(); i++)
	{
		S32 offset = m_bufferRemap[oldIndex].offset;
		primitive.lods[i].offset += offset;

		IndexBuffer& indexBuffer(GetIndexBuffer(primitive.indexBufferIndex));
		assert(primitive.lods[i].offset + primitive.lods[i].nIndices <= indexBuffer.GetIndices().Size());
	}
}

void Scene::SetPrimitiveType(Primitive& primitive)
{
	primitive.primitiveType = GetIndexBuffer(primitive.indexBufferIndex).GetPrimitiveType();

	for (int i = 0; i < primitive.lods.Size(); i++)
	{
		primitive.lods[i].nIndices = GetIndexBuffer(primitive.indexBufferIndex).GetLodSize(i);
		primitive.lods[i].offset = GetIndexBuffer(primitive.indexBufferIndex).GetLodOffset(i);
	}
}

void IndexBuffer::GetMinIndexAndVertexCount(S32 offset, S32 nIndices, U32& minIndex, U32& vertexCount) const
{
	minIndex = INT_MAX;
	vertexCount = 0;
	U32 maxIndex = 0;

	for (int i = 0; i < nIndices; i++)
	{
		S32 index = i + offset;
		minIndex = Maths::Min(m_indices[index], minIndex);

		if (m_indices[index] != ResetIndex32())
			maxIndex = Maths::Max(m_indices[index], maxIndex);
	}

	if (maxIndex > minIndex)
		vertexCount = maxIndex - minIndex + 1;
}

void ResourceBlock::CreateLodPvs()
{
	m_lodPvs.Resize(LodCalculator::GetNumLods());

	for (int i = 0; i < m_lodPvs.Size(); i++)
		m_lodPvs[i] = m_pvsBits;
}

void ResourceBlock::UpdateLodPvs(S32 cell, float distance)
{
	for (int lod = 0; lod < m_lodPvs.Size(); lod++)
	{
		if (m_lodPvs[lod][cell])
		{
			float d = LodCalculator::LodDrawDistance(lod, 1280.f, 1.f);
			if (distance > d) m_lodPvs[lod][cell] = false;
		}
	}
}

void ResourceBlock::SetPlatformType(PlatformType platformType)
{
	for (int i = 0; i < m_vertexBuffers.Size(); i++)
		m_vertexBuffers[i]->m_platformType = platformType;

	for (int i = 0; i < m_indexBuffers.Size(); i++)
		m_indexBuffers[i]->m_platformType = platformType;
}

}}
