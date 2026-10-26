#ifndef CS_GEN_IMAGE_CACHE_H
#define CS_GEN_IMAGE_CACHE_H

namespace TexAtlas
{
	class ImageCache
	{
	public:
		struct ICRecord
		{
			TextureKey key; //DB Key
			Attk::C_Image image;
			F32 widthScale;
			F32 heightScale;
			bool missing;
			bool loaded;
			U32 lowDate;
			U32 highDate;
		};

		ImageCache();

		~ImageCache();

		Attk::C_Status Build(
			const TextureKeyArray& uniqueTextureList, 
			U32 border, 
			Progress* pProgressCallback);

		Attk::C_Status Update(
			const TextureKeyArray& uniqueTextureList,
			U32 border,
			Progress* pProgressCallback);

		Attk::C_Status Read(
			const std::string& fileName,
			Progress* pProgressCallback);

		Attk::C_Status Write(
			const std::string& fileName,
			Progress* pProgressCallback) const;

		const ICRecord& Record(U32 index) const;

		ICRecord& Record(U32 index);

		S32 RecordIndex(const TextureKey& key) const;

		U32 NumRecords() const;

		U32 Border() const;

		U32 Width(U32 imageIndex) const;

		U32 Height(U32 imageIndex) const;

		void Clear();
		
	private:
		void OneTimeInit();

		S32 FindTextureKey(
			const TextureKeyArray& records, 
			const TextureKey& key) const;

		S32 FindICRecord(const TextureKey& key) const;

		ImageCache(const ImageCache& rhs);

		const ImageCache& operator=(const ImageCache& rhs);

		void Serialise(const Attk::C_Image& image, Tu::GenericDiskWriter* pWriter) const;

		void Deserialise(Attk::C_Image& image, Tu::GenericDiskReader* pReader);

		Ut::UtVector< Ut::DynamicVector<ICRecord> >  m_records;
		U32 m_border;
	};
}

#endif
