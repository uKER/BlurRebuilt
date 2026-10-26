#ifndef CS_GEN_AUTO_SCALE_CROP_SHEET_H
#define CS_GEN_AUTO_SCALE_CROP_SHEET_H

namespace TexAtlas
{
	template <typename T>
	class List;
	class Space;

	class AutoScaleCropSheet : public CropSheet
	{
	public:
		enum Mode
		{
			UNKNOWN,

			NORMAL,
			AUTO_FIND_CROP_SHEET_SIZE,
			AUTO_FIND_CROP_SHEET_SIZE_MULTI
		};

		static Attk::C_Status Build
		(
			const TextureKeyArray& textureList,
			bool generateNormalMaps,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csWidth,
			U32 csHeight,
			U32 border,
			F32 minimumScale,
			U32 minImageSize,
			U32 maxImageSize,
			bool allowRotation,
			bool clampSmallestSide,
			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList,
			Mode mode,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

	private:
		static Attk::C_Status FindOptimalScale
		(
			bool& scaleFound,
			F32& scale,
			U32 csWidth,
			U32 csHeight,
			U32 border,
			F32 minimumScale,
			U32 minImageSize,
			U32 maxImageSize,
			bool allowRotation,
			bool clampSmallestSide,
			const TexturePackingLists& texturePackingLists,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

		static Attk::C_Status BuildAtlasSizeList
		(
			Ut::Vector<Ut::Pair<U32, U32> >& csAtlasSizes,
			U32 csAtlasMaxWidth,
			U32 csAtlasMaxHeight,
			U32 border,
			F32 minimumScale,
			U32 minImageSize,
			U32 maxImageSize,
			bool allowRotation,
			bool clampSmallestSide,
			const TexturePackingLists& _texturePackingLists,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);
	};
};

#endif