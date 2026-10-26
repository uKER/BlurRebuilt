#ifndef CS_GEN_ATLAS_H
#define CS_GEN_ATLAS_H

namespace TexAtlas
{
	class Atlas
	{
	public:

		typedef std::string (*AtlasNameCallback) (const std::string& baseName, U32 atlasIndex, U32 layerIndex);

		enum Mode
		{
			UNKNOWN,
			PACK,
			PACK_DRY_RUN,
		};

		struct TextureInfo
		{
			const Attk::C_Image* pImage;
			S32 atlasIndex;
		};

		Atlas();
		
		~Atlas();

		Attk::C_Status Pack
		(
			U32 atlasIndex,
			U32 width, 
			U32 height,
			Attk::E_PixelFormat foramt,
			bool allowRotation,
			U32 border,
			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList,
			Database& database,
			List<U32>& sortedTextureList,
			TexturePackingLists& packingList,
			ImageCache& imageCache,
			Progress* pProgressCallback
		);

		Attk::C_Status PackDryRun
		(
			U32 width, 
			U32 height,
			bool allowRotation,
			U32 border,
			const TexturePackingLists& packingList,
			List<U32>& sortedTextureList
		);

		Attk::C_Status Write
		(
			const std::string& baseFileName,
			U32 atlasIndex,
			Database& database,
			void (*pMessageCallbackFunc)(const std::string& text)
		);

		void Clear();

		bool Dirty() const;

		U32 NumTextures(U32 layerIndex) const;

		F32 Efficiency(U32 layerIndex) const;

		std::string GetAtlasName(std::string baseName, U32 atlasIndex, U32 layerIndex);

		static void SetAtlasNameCallback(AtlasNameCallback callback);

	private:
		enum ContainershipPossiblities
		{
			CAN_CONTAIN,
			CAN_CONTAIN_ROTATED,
			CANNOT_CONTAIN
		};
		void OneTimeInit();

		//Insert
		Attk::E_Status Insert
		(
			bool& couldNotPack,
			U32 atlasIndex,
			bool allowRotation,
			U32 border,
			U32 textureIndex,
			Database* pDatabase,
			ImageCache* pImageCache,
			Progress* pProgressCallback
		);

		void AddSpace(const Space& newSpace);

		void ClipSpaces(const Space& newSpace);

		//A list of atlases - one per layer.
		Attk::C_Image* m_pAtlasList;
		U32 m_nLayers;

		bool m_dirty;
		U32 m_textureCountList[CS_GEN_MAX_LAYERS];
		U32 m_textureAreaList[CS_GEN_MAX_LAYERS];

		Mode m_mode;

		TexturePackingLists* m_pTexturePackingLists;

		//Map of spaces sorted by area.
		Ut::MultiMap<U32, Space> m_spaces;

		static AtlasNameCallback m_atlasNameCallback;
	};
}

#endif