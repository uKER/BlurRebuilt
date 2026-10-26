#include "stdafx.h"
#include "renderingdata/build.h"
#include "renderingdata/archive.h"
#include "renderingdata/lods.h"
#include "renderingdata/texturepairing.h"

#include <texture_export/texture.h>
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <system/serialise.h>
#include "texture_export/targets/xetex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/pctex.h"
#include <system/serialise/C_MemoryStream.h>
#include "misc_shared/archive/serialiser.h"

#include "renderingdata/noninterface/archive.inl"

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

namespace RenderingData {
namespace Build
{

template <typename Archive>
inline void Serialise(Archive& ar, Effect& t)
{
	SERIALISE_BASE_FREE(RenderingData::Effect, ar, t);
}

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode& t)
{
	SERIALISE_BASE_FREE(RenderingData::RenderListNode, ar, t)
}

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode& t)
{
	assert(0);
}

template <typename Archive>
inline void Serialise(Archive& ar, StreamData& t)
{
	SERIALISE_BASE_FREE(RenderingData::StreamData, ar, t);
}

template <typename Archive>
inline void Serialise(Archive& ar, Writer<Texture>& t);

template <typename Archive>
inline void Serialise(Archive& ar, LodWriter<Texture>& t);

template <typename Archive>
inline void Serialise(Archive& ar, Writer<TextureList>& t)
{
	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	S32 nTextures = t.m_t.m_textures.Size();
	ar & Arch::MakeNvp("Size", nTextures);

	for (int i = 0; i < nTextures; i++)
	{
		// Write name and hash first so we can look up the texture on loading.
		Btl::String name = t.m_t.m_textures[i].GetName();
		U32 hash = t.m_t.m_textures[i].GetHashValue();
		ar & Arch::MakeNvp("Name", name);
		ar & Arch::MakeNvp("Hash", hash);

		Writer<Texture> textureWriter(t.m_t.m_textures[i], t.m_platformType);
		ar & Arch::MakeNvp("Texture", textureWriter);
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, MaterialList& t)
{
	SERIALISE_BASE_FREE(const RenderingData::MaterialList, ar, t);
}

template <typename Archive>
inline void Serialise(Archive& ar, SceneWriter& t)
{
	ActivateHandlers();

	assert(t.m_t.m_exportData.exportComplete);

	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	SERIALISE_BASE_FREE(const RenderingData::Scene, ar, t.m_t);

	ar & Arch::MakeNvp("VertexDeclarations", t.m_t.m_vertexDeclarations);
	ar & Arch::MakeNvp("Effects", t.m_t.m_effects);
	S32 n = t.m_t.m_resourceBlocks.Size();
	ar & Arch::MakeNvp("NResourceBlocks", n);

	Writer<TextureList> textureListWriter(t.m_t.m_sharedTextures, t.m_platformType);
	ar & Arch::MakeNvp("SharedTextures", textureListWriter);

	ar & Arch::MakeNvp("StreamData", t.m_t.m_streamData);

	if (ar.GetVersion() > 0)
		ar & Arch::MakeNvp("DynamicMaterialList", t.m_t.m_dynamicMaterialLists);
}

template <typename Archive>
inline void Serialise(Archive& ar, ResourceBlockLodWriter& t)
{
	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	S32 lod = t.m_lod;
	ar & Arch::MakeNvp("Lod", lod);

	if (lod == 0)
	{
		ar & Arch::MakeNvp("VertexBuffers", t.m_t.m_vertexBuffers);
		ar & Arch::MakeNvp("IndexBuffers", t.m_t.m_indexBuffers);
		ar & Arch::MakeNvp("Nodes", t.m_t.m_nodes);
	}

	S32 nTextures = t.m_t.m_textures.Size();
	ar & Arch::MakeNvp("NTextures", nTextures);
	
	if (!Btl::SameType<Archive, Arch::XmlOutputArchive>::result)
	{
		for (int i = 0; i < nTextures; i++)
		{
			//TexturePairManager::Instance().SetTextureIndex(i);
			LodWriter<Texture> textureLod(t.m_t.m_textures[i], t.m_platformType, t.m_lod);
			ar & Arch::MakeNvp("TextureLod", textureLod);
		}
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, ResourceBlockWriter& t)
{
	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	S32 numLods = LodCalculator::GetNumLods();
	ar & Arch::MakeNvp("NLods", numLods);

	for (int i = 0; i < numLods; i++)
	{
		ResourceBlockLodWriter blockLod(t.m_t, t.m_platformType, i);
		ar & Arch::MakeNvp("ResourceBlocks", blockLod);
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, VertexBuffer& t)
{
	S32 size = t.m_vertices.Size();
	ar & Arch::MakeNvp("VertexFormat", t.m_vertices.GetVertexFormat());
	ar & Arch::MakeNvp("Size", size);

	if (!Btl::SameType<Archive, Arch::XmlOutputArchive>::result)
	{
		S32 dataSize = t.m_vertices.GetVertexFormat().GetDataSize() * size;

		Arch::CompressionType compressionType = Arch::NO_COMPRESSION;
		if (t.m_platformType == PLATFORM_TYPE_360) compressionType = Arch::XMEM_COMPRESSION;
		if (t.m_platformType == PLATFORM_TYPE_PS3) compressionType = Arch::ZLIB_COMPRESSION; 
		if (dataSize < 128) compressionType = Arch::NO_COMPRESSION;

		Arch::Serialiser data(ar.GetEndianSwap(), 16, compressionType);

		const U8* pData = t.m_vertices.GetData();

		for (int i = 0; i < size; i++)
		{
			for (int e = 0; e < t.m_vertices.GetVertexFormat().GetNumVertexElements(); e++)
			{
				switch (t.m_vertices.GetVertexFormat().GetVertexElement(e).type)
				{
					case VERTEX_ELEMENT_TYPE_S16_2:
					case VERTEX_ELEMENT_TYPE_S16_2N:
					case VERTEX_ELEMENT_TYPE_F16_2:
					{
						data << *(S16*) pData; pData += 2;
						data << *(S16*) pData; pData += 2;
						break;
					}
					
					case VERTEX_ELEMENT_TYPE_S16_4: 
					case VERTEX_ELEMENT_TYPE_S16_4N:
					case VERTEX_ELEMENT_TYPE_F16_4:
					{
						data << *(S16*) pData; pData += 2;
						data << *(S16*) pData; pData += 2;
						data << *(S16*) pData; pData += 2;
						data << *(S16*) pData; pData += 2;
						break;
					}

					case VERTEX_ELEMENT_TYPE_F32_1:
					{
						data << *(float*) pData; pData += 4;
						break;
					}

					case VERTEX_ELEMENT_TYPE_F32_2:
					{
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						break;
					}

					case VERTEX_ELEMENT_TYPE_F32_3:
					{
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						break;
					}

					case VERTEX_ELEMENT_TYPE_F32_4:
					{
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						data << *(float*) pData; pData += 4;
						break;
					}

					case VERTEX_ELEMENT_TYPE_U8_4:
					case VERTEX_ELEMENT_TYPE_U8_4N:
					case VERTEX_ELEMENT_TYPE_SHHD_3N:
					case VERTEX_ELEMENT_TYPE_SDDD_3N:
					{
						data << *(U32*) pData; pData += 4;
						break;
					}

					default:
						BIZ_ASSERT_FAIL("VertexElementType corrupt");
						return;
				}
			}
		}

		ar & data;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, IndexBuffer& t)
{
	S32 size =  t.m_indices.Size();
	assert(size > 0);
	ar & Arch::MakeNvp("Size", size);

	if (!Btl::SameType<Archive, Arch::XmlOutputArchive>::result)
	{
		U32 max = 0;

		for (int i = 0; i < t.m_indices.Size(); i++)
			if (t.m_indices[i] > max && t.m_indices[i] != t.ResetIndex32())
				max = t.m_indices[i];

		Arch::CompressionType compressionType = Arch::NO_COMPRESSION;
		if (t.m_platformType == PLATFORM_TYPE_360) compressionType = Arch::XMEM_COMPRESSION;
		if (t.m_platformType == PLATFORM_TYPE_PS3) compressionType = Arch::ZLIB_COMPRESSION; 
		if (size < 128) compressionType = Arch::NO_COMPRESSION;

		Arch::Serialiser data(ar.GetEndianSwap(), 16, compressionType);

		if (max < 0xffff) // 16 bit
		{
			IndexType indexType = INDEX_TYPE_U16;
			ar & indexType;
			
			for (int i = 0; i < t.m_indices.Size(); i++)
			{
				U16 index = t.m_indices[i];

				if (t.m_indices[i] == t.ResetIndex32())
					index = t.ResetIndex16();

				data << index;
			}
		}
		else
		{
			IndexType indexType = INDEX_TYPE_U32;
			ar & indexType;

			for (int i = 0; i < t.m_indices.Size(); i++)
				data << t.m_indices[i];
		}

		ar & data;
	}
}

void ChangeNumMipLevels(ActTools::Texture& texture, S32 lowest, S32 highest)
{
	ActTools::Texture newTexture;
	S32 nMipLevels = highest - lowest + 1;
	S32 width = texture.GetMipmap(0, lowest)->GetWidth();
	S32 height = texture.GetMipmap(0, lowest)->GetHeight();
	newTexture.Create(texture.GetType(), texture.GetFormat(), width, height, texture.GetDepth(), nMipLevels);
	newTexture.SetSRGBFlag(texture.GetSRGBFlag());

	for (int m = lowest; m <= highest; m++)
	{
		for (U32 d = 0; d < texture.GetDepth(); d++)
		{
			ActTools::Surface surface;
			texture.DetachSurface(d, m, &surface);
			newTexture.AttachSurface(d, m - lowest, surface);
		}
	}

	texture = newTexture;
}

void ChangeNumMipLevels(ActTools::Texture& texture, const ActTools::Texture& other)
{
	S32 width0 = other.GetMipmap(0, 0)->GetWidth();
	S32 height0 = other.GetMipmap(0, 0)->GetHeight();

	S32 lowest = -1;

	for (int m = 0; m < (S32) texture.GetNumMipMaps(); m++)
	{
		S32 width = texture.GetMipmap(0, m)->GetWidth();
		S32 height = texture.GetMipmap(0, m)->GetHeight();

		if (width == width0 && height == height0)
			lowest = m;
	}

	assert(lowest > -1);
	
	ChangeNumMipLevels(texture, lowest, lowest + other.GetNumMipMaps() - 1);
}

template <typename Archive>
inline void WriteTextureData(Archive& ar, ActTools::Texture& texture, PlatformType platformType, NglBaseTex::OutputType outputType)
{
	switch (platformType)
	{
		case PLATFORM_TYPE_360:
		{
			NglXeTex texWriter;
			texWriter.SerialOutTexture(&ar, &texture, outputType);
			break;
		}
		case PLATFORM_TYPE_PS3:
		{
			NglPs3Tex texWriter;
			texWriter.SerialOutTexture(&ar, &texture, outputType);
			break;
		}
		case PLATFORM_TYPE_PC:
		{
			NglPcTex texWriter;
			texWriter.SerialOutTexture(&ar, &texture, outputType);
			break;
		}
		default:
			assert(0); // unknown platform
	}
}

template <typename Archive>
inline void WriteTextureDataMerged(Archive& ar, ActTools::Texture& first, ActTools::Texture& second)
{
	S32 width = first.GetWidth();
	S32 height = first.GetHeight();

	bool dxt1 = !stricmp("DXT1", first.GetFormat()->GetId());
	bool dxt5 = !stricmp("DXT5", first.GetFormat()->GetId());

	Sys::C_MemoryStream m1(1024 * 1024);
	WriteTextureData(m1, first, PLATFORM_TYPE_360, NglBaseTex::JUST_DATA);

	Sys::C_MemoryStream m2(1024 * 1024);
	WriteTextureData(m2, second, PLATFORM_TYPE_360, NglBaseTex::JUST_DATA);

	U32 tileSize = dxt1 ? 8192 : 16384;

	assert(m1.GetBufferSize() == m2.GetBufferSize());
	assert(m1.GetBufferSize() > tileSize / 2);

	U8* pDst = m1.GetBufferPtr();
	U8* pSrc = m2.GetBufferPtr();
	U32 size = (U32) m1.GetBufferSize();

	for (U32 i = 0; i < size; i++)
	{
		if (i < size - tileSize / 2)
		{
			if (pSrc[i] != 0xcd)
			{
				assert(pDst[i + tileSize / 2] == 0xcd);
				pDst[i + tileSize / 2] = pSrc[i];
			}
		}
		else
		{
			assert(pSrc[i] ==  0xcd);
		}

		//pDst[i] = 0;
	}

	ar.Write(m1.GetBufferPtr(), size);
}

template <typename Archive>
inline void Serialise(Archive& ar, LodWriter<Texture>& t)
{
	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	if (t.m_lod == 0)
	{
		ar & Arch::MakeNvp("Name", t.m_t.m_name);
		ar & Arch::MakeNvp("SamplerState", t.m_t.m_samplerState);
		ar & Arch::MakeNvp("HashValue", t.m_t.m_hashValue);
	}

	// get exported texture data and read into ActTools::Texture
	TextureExport::TextureInfo info;
	ActTools::Texture texture = *t.m_t.m_pTextureManager->GetTexture(t.m_t.GetCacheIndex(), info);

	Btl::Pair<S32, S32> startEnd = LodCalculator::GetMipLevelsForLod(t.m_lod, texture, t.m_t.GetScaleU(), t.m_t.GetScaleV());

	S32 startLevel = startEnd.first;
	S32 endLevel = startEnd.second;

	U32 present = (startLevel > -1);

	//ResourceIndex firstResourceIndex = TexturePairManager::Instance().IsSecond();
	//if (present) present += (firstResourceIndex.GetBlock() > -1);

	ar << present;

	if (present)
	{
		// write header for texture with all mip levels
		ChangeNumMipLevels(texture, startLevel, texture.GetNumMipMaps() - 1);

		{
			Sys::C_MemoryStream memoryStream(1024 * 1024);
			WriteTextureData(memoryStream, texture, t.m_platformType, NglBaseTex::JUST_HEADER);
			S32 size = memoryStream.m_pCurBuffer - memoryStream.GetBufferPtr();
			ar & Arch::MakeNvp("Header Size", size);
			ar.Write(memoryStream.GetBufferPtr(), size);
		}

		//if (firstResourceIndex.GetBlock() == -1)
		{
			// just the texture data for the required mip levels
			ChangeNumMipLevels(texture, 0, endLevel - startLevel);

			/*S32 secondCacheIndex = TexturePairManager::Instance().IsFirst();

			if (secondCacheIndex > -1)
			{
				// merge textures
				TextureExport::TextureInfo info;
				ActTools::Texture second = *t.m_t.m_pTextureManager->GetTexture(secondCacheIndex, info);
				ChangeNumMipLevels(second, texture);

				Arch::Serialiser data(ar.GetEndianSwap(), 16, Arch::XMEM_COMPRESSION);
				WriteTextureDataMerged(data.Stream(), texture, second);
				ar & data;
			}
			else*/
			{		
				Arch::CompressionType compressionType = Arch::NO_COMPRESSION;
				if (t.m_platformType == PLATFORM_TYPE_360) compressionType = Arch::XMEM_COMPRESSION;
				if (t.m_platformType == PLATFORM_TYPE_PS3) compressionType = Arch::ZLIB_COMPRESSION; 
				Arch::Serialiser data(ar.GetEndianSwap(), 16, compressionType);
				WriteTextureData(data.Stream(), texture, t.m_platformType, NglBaseTex::JUST_DATA);
				ar & data;

				if (data.GetBufferSize() > 2 * 1024 * 1024)
					LOGWARN(g_Log, "Texture in stream file larger than 2 Mb - may not be possible to load (%s).", t.m_t.m_name.Chars());

			}
		}
		//else
		//{
		//	ar & firstResourceIndex;
		//}
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, Writer<Texture>& t)
{
	ar & Arch::MakeNvp("PlatformType", t.m_platformType);

	ar & Arch::MakeNvp("Name", t.m_t.m_name);
	ar & Arch::MakeNvp("SamplerState", t.m_t.m_samplerState);
	ar & Arch::MakeNvp("HashValue", t.m_t.m_hashValue);

	if (!Btl::SameType<Archive, Arch::XmlOutputArchive>::result)
	{
		// get exported texture data and read into ActTools::Texture
		TextureExport::TextureInfo info;
		ActTools::Texture &texture = *t.m_t.m_pTextureManager->GetTexture(t.m_t.GetCacheIndex(), info);

		Sys::C_MemoryStream memoryStream(1024 * 1024);
		WriteTextureData(memoryStream, texture, t.m_platformType, NglBaseTex::HEADER_AND_DATA);
		S32 size = memoryStream.m_pCurBuffer - memoryStream.GetBufferPtr();
		ar & Arch::MakeNvp("Texture Size", size);
		ar.Write(memoryStream.GetBufferPtr(), size);
	}
}

} // Build //////////////////////////////////////////////////////
} // RenderingData //////////////////////////////////////////////

// Default serialisation for Writer.
SERIALISATION_VERSION(RenderingData::Build::Effect, 0, "RenderingData::Build::Effect")
SERIALISATION_VERSION(RenderingData::Build::VertexBuffer, 1, "RenderingData::Build::VertexBuffer")
SERIALISATION_VERSION(RenderingData::Build::IndexBuffer, 1, "RenderingData::Build::IndexBuffer")
SERIALISATION_VERSION(RenderingData::Build::Writer<RenderingData::Build::Texture>, 0, "RenderingData::Build::Texture")
SERIALISATION_VERSION(RenderingData::Build::LodWriter<RenderingData::Build::Texture>, 2, "RenderingData::Build::TextureLod")
SERIALISATION_VERSION(RenderingData::Build::Writer<RenderingData::Build::ResourceBlock>, 0, "RenderingData::Build::ResourceBlock")
SERIALISATION_VERSION(RenderingData::Build::LodWriter<RenderingData::Build::ResourceBlock>, 0, "RenderingData::Build::ResourceBlockLod")
SERIALISATION_VERSION(RenderingData::Build::Writer<RenderingData::Build::TextureList>, 0, "RenderingData::Build::TextureList")
SERIALISATION_POLYMORPHIC(RenderingData::Build::RenderListNode, 0, "RenderingData::RenderListNode_Common")
SERIALISATION_VERSION(RenderingData::Build::StreamData, 0, "RenderingData::Build::StreamData")

/////////////////////////////////////////////////////////////////

namespace RenderingData
{

// just to avoid linker errors
void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, RenderListNode::Primitive& t) {assert(0);}
void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, RenderListNode::Primitive& t) {assert(0);}

void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, MaterialList& t) {assert(0);}
void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, MaterialList& t) {assert(0);}

void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, MaterialNode& t) {assert(0);}
void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, MaterialNode& t) {assert(0);}

namespace Build
{

void ActivateHandlers()
{
	RenderingData::ActivateHandlers();
	SERIALISATION_HANDLER(RenderingData::Build::RenderListNode)
}

SERIALISATION_KEY(RenderListNode)

template void Serialise(Arch::BinaryOutputArchive_t<Ut::NoEndianSwap>& ar, SceneWriter& t);
template void Serialise(Arch::BinaryOutputArchive_t<Ut::NoEndianSwap>& ar, ResourceBlockWriter& t);
template void Serialise(Arch::BinaryOutputArchive_t<Ut::NoEndianSwap>& ar, ResourceBlockLodWriter& t);

template void Serialise(Arch::BinaryOutputArchive_t<Ut::DoEndianSwap>& ar, SceneWriter& t);
template void Serialise(Arch::BinaryOutputArchive_t<Ut::DoEndianSwap>& ar, ResourceBlockWriter& t);
template void Serialise(Arch::BinaryOutputArchive_t<Ut::DoEndianSwap>& ar, ResourceBlockLodWriter& t);

template void Serialise(Arch::XmlOutputArchive& ar, SceneWriter& t);
template void Serialise(Arch::XmlOutputArchive& ar, ResourceBlockWriter& t);
template void Serialise(Arch::XmlOutputArchive& ar, ResourceBlockLodWriter& t);

}

}
