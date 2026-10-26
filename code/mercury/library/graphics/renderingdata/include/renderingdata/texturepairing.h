#ifndef RD_TEXTURE_PAIRING_H
#define RD_TEXTURE_PAIRING_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/map.h"
#include "btl/containers/set.h"
#include "build.h"

namespace TextureExport {class TextureManager;}
namespace ActTools {class Texture;}

namespace RenderingData {
namespace Build
{
	class TexturePairManager
	{
	public:
		static TexturePairManager& Instance() {return s_instance;}

		void Begin();
		void BeginChunk();
		void SetBlock(S32 block, S32 lod);
		void SetTextureIndex(S32 index);
		void AddTexture(ActTools::Texture& texture, S32 cacheIndex);
		void End();

		// If this is the first texture in a pair, returns the texture cache index of the second texture.
		// Otherwise returns -1.
		S32 IsFirst();
		
		// If this is the first texture in a pair, returns the resource index of the first texture.
		// Otherwise returns a null resource index.
		ResourceIndex IsSecond();

	private:
		TexturePairManager() {}

		typedef Btl::Map<Btl::Vector<U8>, ResourceIndex> TextureMap; // Header (lod 0) -> resource index
		TextureMap m_textureHeaders;

		typedef Btl::Map<ResourceIndex, S32> First; // ResourceIndex -> texture cache index
		typedef Btl::Map<ResourceIndex, ResourceIndex> Second;
		Btl::Vector<First> m_first; // per lod
		Btl::Vector<Second> m_second; // per lod

		S32 m_lod;
		S32 m_block;
		S32 m_textureIndex;

		S32 m_textureCount;
		S32 m_pairedCount;

		static TexturePairManager s_instance;
	};

}}

#endif