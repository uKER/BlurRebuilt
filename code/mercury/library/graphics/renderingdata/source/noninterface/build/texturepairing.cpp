#include "stdafx.h"
#include "renderingdata/texturepairing.h"
#include "renderingdata/archive.h"
#include "renderingdata/lods.h"

#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include "texture_export/targets/xetex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/pctex.h"
#include <system/serialise/C_MemoryStream.h>

#include "renderingdata/noninterface/archive.inl"

//#pragma optimize("", off)

namespace RenderingData
{

namespace Build
{

TexturePairManager TexturePairManager::s_instance;

void ChangeNumMipLevels(ActTools::Texture& texture, S32 lowest, S32 highest);

template <typename Archive>
inline void WriteTextureData(Archive& ar, ActTools::Texture& texture, PlatformType platformType, NglBaseTex::OutputType outputType);

void StreamData::CalculateTexturePairing(S32 chunkIndex)
{
	if (m_platformType == PLATFORM_TYPE_360)
	{
		S32 nItems = m_chunks[chunkIndex].GetNumItems();

		TexturePairManager::Instance().BeginChunk();

		for (int i = 0; i < nItems; i++)
		{
			const ChunkInfo::Item& item =  m_chunks[chunkIndex].GetItem(i);
			TexturePairManager::Instance().SetBlock(item.block, item.lod);
			CalculateTexturePairing(m_pScene->GetResourceBlock(item.block), item.lod);
		}
	}
}

void StreamData::CalculateTexturePairing(const ResourceBlock& block, S32 lod)
{
	S32 nTextures = block.GetNumTextures();

	for (int i = 0; i < nTextures; i++)
	{
		TexturePairManager::Instance().SetTextureIndex(i);
		CalculateTexturePairing(block.GetTexture(i), lod);
	}
}

void StreamData::CalculateTexturePairing(const Texture& texture, S32 lod)
{
	TextureExport::TextureInfo info;
	ActTools::Texture actTexture = *texture.GetTextureManager()->GetTexture(texture.GetCacheIndex(), info);

	Btl::Pair<S32, S32> startEnd = LodCalculator::GetMipLevelsForLod(lod, actTexture, texture.GetScaleU(), texture.GetScaleV());

	S32 startLevel = startEnd.first;
	S32 endLevel = startEnd.second;

	U32 present = (startLevel > -1); 

	if (present)
	{
		ChangeNumMipLevels(actTexture, startLevel, actTexture.GetNumMipMaps() - 1);

		TexturePairManager::Instance().AddTexture(actTexture, texture.GetCacheIndex());
	}
}

void TexturePairManager::BeginChunk()
{
	m_textureHeaders.Clear();
	m_first.Clear();
	m_second.Clear();

	m_first.Resize(LodCalculator::GetNumLods());
	m_second.Resize(LodCalculator::GetNumLods());
}

void TexturePairManager::Begin()
{
	m_textureCount = 0;
	m_pairedCount = 0;
}

void TexturePairManager::End()
{
	int breakhere = 0;
}

void TexturePairManager::SetBlock(S32 block, S32 lod)
{
	m_block = block;
	m_lod = lod;
}

void TexturePairManager::SetTextureIndex(S32 index)
{
	m_textureIndex = index;
}

void TexturePairManager::AddTexture(ActTools::Texture& texture, S32 cacheIndex)
{
	S32 width = texture.GetWidth();
	S32 height = texture.GetHeight();

	if (cacheIndex == 114)
		int breakhere = 0;

	bool dxt = (!stricmp("DXT1", texture.GetFormat()->GetId()) || !stricmp("DXT5", texture.GetFormat()->GetId()));

	if (dxt && width < 64 && height < 64)
	{
		m_textureCount++;

		if (m_lod == 0)
		{
			Sys::C_MemoryStream memoryStream(1024 * 1024);
			WriteTextureData(memoryStream, texture, PLATFORM_TYPE_360, NglBaseTex::JUST_HEADER);
			S32 size = memoryStream.m_pCurBuffer - memoryStream.GetBufferPtr();

			Btl::Vector<U8> headerData;
			headerData.Resize(size);
			memcpy(&headerData[0], memoryStream.GetBufferPtr(), size);

			TextureMap::InsertionReturnType r = m_textureHeaders.Insert(headerData);

			// If this is lod zero, is there another unpaired texture with the same size and format in this chunk?
			if (r.inserted)
			{
				r.iterator->GetValue() = ResourceIndex::BlockIndex(m_block, m_textureIndex);
			}
			else
			{
				ResourceIndex first = r.iterator->GetValue();
				ResourceIndex second = ResourceIndex::BlockIndex(m_block, m_textureIndex);
				m_first[0][first] = cacheIndex;
				m_second[0][second] = first;
				m_textureHeaders.Erase(r.iterator);
				m_pairedCount += 2;
			}
		}
		else
		{
			/*ResourceIndex current = ResourceIndex::BlockIndex(m_block, m_textureIndex);

			// If this is not lod zero, was this texture paired with another texture in lod zero and can it still be paired here?
			PairMap::Iterator i = m_pairs[0].Find(current);
		
			if (i != m_pairs[0].End())
			{
				ResourceIndex other = i->GetValue();
				m_currentTextures.Insert(current); 

				if (m_currentTextures.Find(other) != m_currentTextures.End()) // must be in the same chunk
				{
					m_pairs[m_lod][other] = current;
					m_pairs[m_lod][current] = other;
					m_pairedCount += 2;
				}
			}*/
		}
	}
}

S32 TexturePairManager::IsFirst()
{
	if (m_first.Empty())
		return -1;

	First::Iterator i = m_first[m_lod].Find(ResourceIndex::BlockIndex(m_block, m_textureIndex));

	if (i != m_first[m_lod].End())
		return i->GetValue();
	else
		return -1;
}

ResourceIndex TexturePairManager::IsSecond()
{
	if (m_second.Empty())
		return ResourceIndex();

	Second::Iterator i = m_second[m_lod].Find(ResourceIndex::BlockIndex(m_block, m_textureIndex));

	if (i != m_second[m_lod].End())
		return i->GetValue();
	else
		return ResourceIndex();
}

}

}
