#ifndef CS_GEN_IMAGE_PACKING_LISTS_H
#define CS_GEN_IMAGE_PACKING_LISTS_H

namespace TexAtlas
{
	struct TexRecord
	{
		TextureKey key;
		U32 lowDate;
		U32 highDate;
		U32 width;
		U32 height;
		U32 originalWidth;
		U32 originalHeight;
		bool missing;
		bool used;
	};

	class TexturePackingLists
	{
	public:
		TexturePackingLists();

		TexturePackingLists(const TexturePackingLists& rhs);

		const TexturePackingLists& operator =(const TexturePackingLists& rhs);

		virtual ~TexturePackingLists();

		Attk::C_Status Build(
			const TextureKeyArray& textureList, 
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback);

		const TexRecord& Record(U32 layerIndex, U32 imageIndex) const;

		TexRecord& Record(U32 layerIndex, U32 imageIndex);

		U32 NumRecords() const;

		U32 NumLayers() const;

		void Clear();

		U32 Width(U32 layerIndex, U32 imageIndex) const;

		U32 Height(U32 layerIndex, U32 imageIndex) const;
		
	private:
		void OneTimeInit();

		//[ImageLayer][ImageHeader]
		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<TexRecord> > > > m_records;
	};
}

#endif
