#include "stdafx.h"
#include "renderingdata/build.h"
#include "btl/containers/algorithm.h"
#include "renderingdata/archive.h"
#include "renderingdata/noninterface/archive.inl"
#include "renderingdata/lods.h"
#include <algorithm>
#include <system/DebugLog.h>
#include "renderingdata/texturepairing.h"

// to do:
// documentation
// primitive types
//   point lists (no vertex sharing)
//   quads -> triangles
// instance batches

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

namespace RenderingData {
namespace Build
{

///////////////////////////////////////////////////
// Material List
S32 MaterialList::InsertMaterialType(const MaterialType& m)
{
	//assert(m.GetNumElements() > 0);

	// If it already exists, use the same entry.
	for (int i = 0; i < m_materialTypes.Size(); i++)
	{
		if (m_materialTypes[i] == m)
			return i;
	}

	m_materialTypes.PushBack(m);
	return m_materialTypes.Size() - 1;
}

void MaterialList::Clear()
{
	m_materialTypes.Clear();
	m_materials.Clear();
}

S32 MaterialList::InsertMaterial(const Material& m)
{
	assert(m_materialTypes[m.GetTypeIndex()].GetDataSize() == m.GetDataSize());

	MaterialMap::InsertionReturnType i = m_materialMap.Insert(m);
	if (i.inserted)
	{	
		i.iterator->GetValue() = m_materials.Size();
		m_materials.PushBack(m);
	}

	return i.iterator->GetValue();
}

// TextureList
S32 TextureList::InsertTexture(const Texture& texture)
{
	TextureMap::InsertionReturnType i = m_textureMap.Insert(texture);

	if (i.inserted)
	{
		m_textures.PushBack(texture);
		i.iterator->GetValue() = m_textures.Size() - 1;
	}

	return i.iterator->GetValue();
}

// IndexBuffer
S32 IndexBuffer::CreateLod(S32 nIndices)
{
	if (m_lodSizes.Empty())
		m_lodSizes.PushBack(m_indices.Size());
    
    const S32 newLodIndex( m_lodSizes.Size() );
	const S32 newLodOffset( m_indices.Size() );
	//m_indices.Resize(m_indices.Size() + nIndices);
	m_indices.Resize(newLodOffset + nIndices);

	//for (int i = 0; i < nIndices; i++)
	//	m_indices[m_lodSizes.Back() + i] = ResetIndex32();
	for (int i = 0; i < nIndices; i++)
		m_indices[newLodOffset + i] = ResetIndex32();

	m_lodSizes.PushBack(nIndices);
	//return m_lodSizes.Size() - 1;
	return newLodIndex;
}

void IndexBuffer::InsertVertexFormat(const VertexFormat& vertexFormat, S32 streamIndex)
{
	for (int i = 0; i < m_vertexFormatData.Size(); i++)
	{
		if (m_vertexFormatData[i].streamIndex == streamIndex)
		{
			assert(vertexFormat == m_vertexFormatData[i].format);
			return;
		}
	}

	m_vertexFormatData.PushBack();
	m_vertexFormatData.Back().format = vertexFormat;
	m_vertexFormatData.Back().streamIndex = streamIndex;
}

// StreamData
StreamData::StreamData() : m_chunkDataReady(false), m_streamDataReady(false)
{
}

// Assign a lod of a block to a chunk.
void StreamData::AddBlockToChunk(S32 chunkIndex, S32 blockIndex, S32 lod)
{
	if (m_chunks.Size() <= chunkIndex)
		m_chunks.Resize(chunkIndex + 1);

	m_chunks[chunkIndex].m_items.PushBack();
	m_chunks[chunkIndex].m_items.Back().block = blockIndex;
	m_chunks[chunkIndex].m_items.Back().lod = lod;
}

void StreamData::GetRequiredBlocks(Node* pNode, Btl::Set<S32>& blocks)
{
	// Require lod 0 of all textures used by the node
	// (because the renderer won't check that textures are loaded).
	/*if (RenderListNode* pRenderListNode = RenderListNode::GetPointer(pNode))
	{
		for (int j = 0; j < pRenderListNode->GetNumTextures(); j++)
			blocks.Insert(pRenderListNode->GetTextureSource(j).GetBlock());
	}*/

	// Require blocks containing parent nodes
	// (because this node will be ignored otherwise).
	if (!pNode->GetParent().IsNull() && !pNode->GetParent().IsShared())
	{
		blocks.Insert(pNode->GetParent().GetBlock());
		Node* pParent = m_pScene->GetNodePtr(pNode->GetParent());
		GetRequiredBlocks(pParent, blocks);
	}
}

void StreamData::AddRequirement(S32 chunkIndex, S32 requiredChunk)
{
	if (requiredChunk != chunkIndex)
	{
		Btl::Vector<S32>::Iterator i = Btl::Find(m_chunks[chunkIndex].m_prerequisites, requiredChunk);
		if (i == m_chunks[chunkIndex].m_prerequisites.End())
		{
			m_chunks[chunkIndex].m_prerequisites.PushBack(requiredChunk);
			m_chunks[requiredChunk].m_dependants.PushBack(chunkIndex);
		}
	}
}

void StreamData::CalculateDependencies()
{
	typedef Btl::Map<ChunkInfo::Item, S32> ItemMap;
	ItemMap itemMap;

	// Build a list of blocks in the chunk, sorted by block index.
	// Used for updating blocks while streaming.
	for (int chunkIndex = 0; chunkIndex < m_chunks.Size(); chunkIndex++)
	{
		m_chunks[chunkIndex].m_dependants.Clear();
		m_chunks[chunkIndex].m_prerequisites.Clear();
		m_chunks[chunkIndex].m_blocks.Clear();

		typedef Btl::Set<S32> BlockSet;
		BlockSet blockSet;

		for (int itemIndex = 0; itemIndex < m_chunks[chunkIndex].m_items.Size(); itemIndex++)
		{
			ChunkInfo::Item& item = m_chunks[chunkIndex].m_items[itemIndex];
			itemMap[item] = chunkIndex;
			blockSet.Insert(item.block);
		}

		for (BlockSet::Iterator i = blockSet.Begin(); i != blockSet.End(); ++i)
			m_chunks[chunkIndex].m_blocks.PushBack(*i);
	}

	// Set up dependencies.
	for (int chunkIndex = 0; chunkIndex < m_chunks.Size(); chunkIndex++)
	{
		for (int itemIndex = 0; itemIndex < m_chunks[chunkIndex].m_items.Size(); itemIndex++)
		{
			ChunkInfo::Item& item = m_chunks[chunkIndex].m_items[itemIndex];
		
			if (item.lod > 0) // high lods need low lods first
			{
				ChunkInfo::Item previous;
				previous.block = item.block;
				previous.lod = item.lod - 1;
				ItemMap::Iterator find = itemMap.Find(previous);

				if (find != itemMap.End())
					AddRequirement(chunkIndex, find->GetValue());
				else
					assert(0);
			}
			else // lowest lod depends on nodes
			{
				Btl::Set<S32> blocks;

				for (int i = 0; i < m_pScene->GetResourceBlock(item.block).GetNumNodes(); i++)
				{
					Node* pNode = m_pScene->GetResourceBlock(item.block).GetNodePtr(i);
					GetRequiredBlocks(pNode, blocks);
				}

				for (Btl::Set<S32>::Iterator it = blocks.Begin(); it != blocks.End(); ++it)
				{
					ChunkInfo::Item previous;
					previous.block = *it;
					previous.lod = 0;
					ItemMap::Iterator find = itemMap.Find(previous);

					if (find != itemMap.End())
						AddRequirement(chunkIndex, find->GetValue());
				}
			}
		}
	}
}

// Prepare the chunk data for writing.
void StreamData::Finish()
{
	// Within each chunk, sort by lod
	for (int chunkIndex = 0; chunkIndex < m_chunks.Size(); chunkIndex++)
	{
		std::sort(m_chunks[chunkIndex].m_items.Begin(), m_chunks[chunkIndex].m_items.End());
	}

	CalculateDependencies();

	// Re-order the chunks so that chunks depend only on earlier chunks.
	if (m_chunks.Size() > 0)
	{
		assert(m_orderedChunks.Size() == 0);
		S32 current = 0;

		enum ChunkStatus
		{
			UNPROCESSED,
			IN_STACK,
			IN_ORDERED_LIST
		};

		Btl::Vector<ChunkStatus> chunkStatus;
		Btl::Vector<S32> chunkStack;

		chunkStatus.Resize(m_chunks.Size());

		for (int i = 0; i < m_chunks.Size(); i++)
			chunkStatus[i] = UNPROCESSED;

		do
		{
			assert(chunkStatus[current] == UNPROCESSED);
			S32 previous = current;

			for (int i = 0; i < m_chunks[current].m_prerequisites.Size(); i++)
			{
				S32 requiredChunk = m_chunks[current].m_prerequisites[i];

				assert(requiredChunk != current); // cannot depend on itself

				if (chunkStatus[requiredChunk] == UNPROCESSED)
				{
					chunkStack.PushBack(current);
					chunkStatus[current] = IN_STACK;
					current = requiredChunk;
					break;
				}
				else if (chunkStatus[requiredChunk] == IN_STACK)
				{
					assert(0); // cannot create ordered list
					return;
				}
			}

			if (current == previous)
			{
				m_orderedChunks.PushBack(current);
				chunkStatus[current] = IN_ORDERED_LIST;

				if (chunkStack.Size() > 0)
				{
					current = chunkStack.Back();
					chunkStack.PopBack();
					chunkStatus[current] = UNPROCESSED;
				}
				else
				{
					do {current++;} while (current < m_chunks.Size() && chunkStatus[current] == IN_ORDERED_LIST);
				}
			}

		} while (current < m_chunks.Size());

		assert(m_orderedChunks.Size() == m_chunks.Size());

		Btl::Vector<ChunkInfo> newChunks;

		for (int i = 0; i < m_orderedChunks.Size(); i++)
			newChunks.PushBack(m_chunks[m_orderedChunks[i]]);
		
		Btl::Swap(newChunks, m_chunks);
	}

	// Recalculate since the order changed.
	CalculateDependencies();

	S32 nGroups = 32; // hack to fix empty groups
	m_groups.Resize(m_chunks.Size());

	// Work out bounds.
	for (int chunkIndex = 0; chunkIndex < m_chunks.Size(); chunkIndex++)
	{
		m_chunks[chunkIndex].m_bounds.SetEmpty();
		bool allGroups = false;

		for (int itemIndex = 0; itemIndex < m_chunks[chunkIndex].m_items.Size(); itemIndex++)
		{
			ChunkInfo::Item& item = m_chunks[chunkIndex].m_items[itemIndex];
			const ResourceBlock& block(m_pScene->GetResourceBlock(item.block));
			Bounds bounds = block.GetFullBounds();
			bounds.distanceMax = LodCalculator::LodDrawDistance(item.lod, 1280.f, 1.f);
			m_chunks[chunkIndex].m_bounds.SetUnion(bounds);
			
			S32 nCells = m_pScene->GetResourceBlock(item.block).GetPvsBits().Size();
			m_chunkPvs.Resize(Maths::Max(nCells, m_chunkPvs.Size()));

			for (int i = 0; i < nCells; i++)
			{
				if (m_chunkPvs[i].Size() == 0)
				{
					m_chunkPvs[i].Resize(m_chunks.Size());
					m_chunkPvs[i].ClearBits();
				}

				if (m_pScene->GetResourceBlock(item.block).GetPvsBits(item.lod)[i])
				{
					m_chunkPvs[i][chunkIndex] = true;
				}
			}

			// Update the group bitfields.
			for (Btl::Set<S32>::ConstIterator gi = block.GetGroups().Begin(); gi != block.GetGroups().End(); ++gi)
			{
				S32 groupIndex = *gi;

				if (groupIndex > -1)
				{
					nGroups = Maths::Max(nGroups, groupIndex + 1);

					if (!allGroups)
					{
						while (m_groups[chunkIndex].Size() < nGroups)
							m_groups[chunkIndex].PushBack(false);

						m_groups[chunkIndex][groupIndex] = true;
					}
				}
				else
				{
					m_groups[chunkIndex].Clear();
					allGroups = true;
				}
			}

			// Rooms
			S32 nRooms = m_pScene->GetResourceBlock(item.block).GetRoomBits().Size();
			m_chunkRooms.Resize(Maths::Max(nRooms, m_chunkRooms.Size()));

			for (int i = 0; i < nRooms; i++)
			{
				if (m_chunkRooms[i].Size() == 0)
				{
					m_chunkRooms[i].Resize(m_chunks.Size());
					m_chunkRooms[i].ClearBits();
				}

				if (m_pScene->GetResourceBlock(item.block).GetRoomBits()[i])
				{
					m_chunkRooms[i][chunkIndex] = true;
				}
			}
		}

		m_chunkBounds.PushBack(m_chunks[chunkIndex].m_bounds);
	}

	// Fill out the group bitfields.
	for (int chunkIndex = 0; chunkIndex < m_chunks.Size(); chunkIndex++)
	{
		if (!m_groups[chunkIndex].Empty())
			while (m_groups[chunkIndex].Size() < nGroups)
				m_groups[chunkIndex].PushBack(false);
	}

	m_pScene->BuildChunkData();

	m_chunkDataReady = true;
}

U32 ResourceBlock::GetGroupBits() const
{
	if (GetGroups().Empty())
		return ~0;

	U32 bits = 0;

	for (Btl::Set<S32>::ConstIterator gi = GetGroups().Begin(); gi != GetGroups().End(); ++gi)
	{
		S32 groupIndex = *gi;

		if (groupIndex > -1) 
			bits |= 1 << groupIndex;
		else
			return ~0;
	}

	return bits;
}

void StreamData::WriteChunkData(Sys::I_File* pFile, S32 chunkIndex)
{
	Arch::BinaryOutputArchive ar(*pFile, RenderingData::GetEndianSwap(m_platformType));

	S32 nItems = m_chunks[chunkIndex].GetNumItems();
	ar & nItems;

	//CalculateTexturePairing(chunkIndex);

	for (int i = 0; i < nItems; i++)
	{
		const ChunkInfo::Item& item = m_chunks[chunkIndex].GetItem(i);
		//TexturePairManager::Instance().SetBlock(item.block, item.lod);
		RenderingData::Build::ResourceBlockLodWriter blockLodWriter(m_pScene->GetResourceBlock(item.block), m_platformType, item.lod);
		ar << blockLodWriter;
	}
}

S32 StreamData::GetChunkSize(S32 chunkIndex, PlatformType platformType) const
{
	S32 size = 0;
	S32 nItems = m_chunks[chunkIndex].GetNumItems();

	for (int i = 0; i < nItems; i++)
	{
		const ChunkInfo::Item& item =  m_chunks[chunkIndex].GetItem(i);
		size += m_pScene->GetResourceBlock(item.block).GetSize(item.lod, platformType);
	}

	return size;
}

S32 StreamData::GetItemSize(S32 chunkIndex, S32 itemIndex, PlatformType platformType) const
{
	const ChunkInfo::Item& item =  m_chunks[chunkIndex].GetItem(itemIndex);
	return m_pScene->GetResourceBlock(item.block).GetSize(item.lod, platformType);
}

void StreamData::AlignPosition(Sys::I_File* pFile, U64 align)
{
	U64 pos = pFile->GetPosition();
	U64 aligned = Btl::AlignUp(pos, align);
	pFile->SetPosition(aligned);
}

// Write all the stream data to a file (must be done before serialising the stream data).
void StreamData::WriteChunkData(Sys::I_File* pFile, PlatformType platformType)
{
	U64 pos = pFile->GetPosition();
	assert(pos == 0);

	m_platformType = platformType;

	if (!m_chunkDataReady)
		Finish();

	Build::ActivateHandlers();

	LOGDEBUG(g_Log, "*********** Chunk Data *************");

	if (m_chunkDataReady)
	{
		LOGDEBUG(g_Log, "index    offset     size     GPU data    lods\t items");

		//TexturePairManager::Instance().Begin();

		// to do: if a chunk is not visible from anywhere, there's no need to include it in the file.
		for (int i = 0; i < m_orderedChunks.Size(); i++)
		{
			S32 chunkIndex = i;//m_orderedChunks[i];
			U64 offset = pFile->GetPosition();

			WriteChunkData(pFile, chunkIndex);
			U64 compressedSize = pFile->GetPosition() - offset;
			AlignPosition(pFile, 2048);
			U64 size = pFile->GetPosition() - offset;
			U64 originalSize = GetChunkSize(chunkIndex, platformType);

			m_chunks[chunkIndex].m_offset = offset;
			m_chunks[chunkIndex].m_compressedSize = size;
			m_chunks[chunkIndex].m_originalSize = originalSize;

			S32 minLod = 10;
			S32 maxLod = 0;
			S32 nItems = 0;//m_chunks[chunkIndex].GetNumItems();

			for (int j = 0; j < m_chunks[chunkIndex].GetNumItems(); j++)
			{
				if (GetItemSize(chunkIndex, j, platformType) > 0)
				{
					S32 lod = m_chunks[chunkIndex].GetItem(j).lod;
					minLod = min(minLod, lod);
					maxLod = max(maxLod, lod);
					nItems++;
				}
			}

			LOGDEBUG(g_Log, "%4i %10I64u %10I64u %10I64u    %i-%i \t %i", chunkIndex, offset, size, originalSize, minLod, maxLod, nItems);
		}

		//TexturePairManager::Instance().End();
	}

	m_streamDataReady = true;

	LOGDEBUG(g_Log, "****************************");
}

void Texture::UpdateScale(float uScale, float vScale, float weight) const
{
	m_textureScaleU.PushBack();
	m_textureScaleV.PushBack();
	m_textureScaleU.Back().scale = uScale;
	m_textureScaleV.Back().scale = vScale;
	m_textureScaleU.Back().weight = weight;
	m_textureScaleV.Back().weight = weight;
	m_textureScaleCalculated = false;
}

float Texture::GetScaleU() const
{
	if (!m_textureScaleCalculated)
		CalculateTextureScale();

	float b = (((F32)(m_samplerState.GetMipMapLodBias()))/128.0f)*8.0f;
	float s = pow(2.f, b);
	return m_uScale / s;
}

float Texture::GetScaleV() const
{
	if (!m_textureScaleCalculated)
		CalculateTextureScale();

	float b = (((F32)(m_samplerState.GetMipMapLodBias()))/128.0f)*8.0f;
	float s = pow(2.f, b);
	return m_vScale / s;
}

bool operator < (const Texture::TextureScale& a, const Texture::TextureScale& b)
{
	return a.scale < b.scale;
}

void Texture::CalculateTextureScale() const
{
	m_uScale = 1.f;
	m_vScale = 1.f;

	std::sort(m_textureScaleU.Begin(), m_textureScaleU.End());
	std::sort(m_textureScaleV.Begin(), m_textureScaleV.End());

	float totalWeight = 0.f;
	float totalU = 0.f;
	float totalV = 0.f;

	for (int i = 0; i < m_textureScaleU.Size(); i++)
	{
		float weight = m_textureScaleU[i].weight;
		totalWeight += weight;
		totalU += m_textureScaleU[i].scale * weight;
	}

	for (int i = 0; i < m_textureScaleV.Size(); i++)
	{
		float weight = m_textureScaleV[i].weight;
		totalV += m_textureScaleV[i].scale * weight;
	}

	float median = totalWeight * 0.5f;

	float currentWeight = 0.f;

	for (int i = 0; i < m_textureScaleU.Size(); i++)
	{
		currentWeight += m_textureScaleU[i].weight;

		if (currentWeight >= median)
		{
			m_uScale = m_textureScaleU[i].scale;
			break;
		}
	}

	currentWeight = 0.f;

	for (int i = 0; i < m_textureScaleV.Size(); i++)
	{
		currentWeight += m_textureScaleV[i].weight;

		if (currentWeight >= median)
		{
			m_vScale = m_textureScaleV[i].scale;
			break;
		}
	}

	/*if (totalWeight > 0)
	{
		float averageU = totalU / totalWeight;
		float averageV = totalV / totalWeight;

		if (abs(m_uScale - averageU) > 10.f || abs(m_vScale - averageV) > 10.f)
		{
			LOGDEBUG(g_Log, "average U = %0.2f, median U = %0.2f, average V = %0.2f, median V = %0.2f", averageU, m_uScale, averageV, m_vScale);
		}
	}*/

	m_textureScaleCalculated = true;
}

}} 
