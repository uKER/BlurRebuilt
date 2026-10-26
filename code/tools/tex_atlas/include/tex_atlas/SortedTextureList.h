#ifndef CS_GEN_SORTED_TEXTURE_LIST_H
#define CS_GEN_SORTED_TEXTURE_LIST_H

namespace TexAtlas
{
	class SortedTextureListBuilder
	{
	public:
		static Attk::C_Status Build(
			List<U32>* pTextureList, 
			const TexturePackingLists& packingList);

	private:
		static void Insert(
			List<U32>* pTextureList, 
			const TexturePackingLists& packingList, 
			U32 cacheIndex);
	};
};

#endif