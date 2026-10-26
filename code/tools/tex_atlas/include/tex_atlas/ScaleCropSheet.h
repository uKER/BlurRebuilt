#ifndef CS_GEN_SCALE_CROP_SHEET_H
#define CS_GEN_SCALE_CROP_SHEET_H

namespace TexAtlas
{
	class ScaleCropSheet : public CropSheet
	{
	public:
		static Attk::C_Status Build
		(
			const TextureKeyArray& textureList,
			bool generateNormalMaps,
			const std::string& dbFileName,
			const std::string& csFileName,
			const Ut::Vector<Ut::Pair<U32, U32> >& csWidthHeightArray,
			F32 scale,
			U32 border,
			U32 minImageSize,
			U32 maxImageSize,
			bool allowRotation,
			bool clampSmallestSide,
			Ut::UtVector< Ut::DynamicVector<LiteMaths::Vec4U8> > layerBackgroundColourList,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

	private:
	};
};

#endif