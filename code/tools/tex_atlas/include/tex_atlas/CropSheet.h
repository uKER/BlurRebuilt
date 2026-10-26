#ifndef CS_GEN_CROP_SHEET_H
#define CS_GEN_CROP_SHEET_H

namespace TexAtlas
{

	class CropSheet
	{
	public:
		static Attk::C_Status BuildTextureList
		(
			TextureKeyArray& textureList,
			const Aftk::C_Scene* pAftkScene,
			bool addBorderTexture,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

		static Attk::C_Status BuildTextureList
		(
			TextureKeyArray& textureList,
			const std::string& fileName,
			bool addBorderTexture,
			U32 minimumLayerCount,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

	protected:
		static Attk::C_Status CheckForMissingTextures
		(
			const TextureKeyArray& textureList,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text)
		);

		static Attk::C_Status ReExportRequired
		(
			const TextureKeyArray& textureList,
			const Database& pDB,
			bool* pReExportRequired,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text)
		);

		static void ScaleTextureRecord
		(
			TexRecord& r,
			U32 border,
			F32 scale,
			bool clampSmallestSide,
			U32 minImageSize,
			U32 maxImageSize,
			U32 csWidth,
			U32 csHeight
		);

		static Attk::C_Status PostProcessImage
		(
			const char* pFilename,
			Attk::C_Image *pCropImage,
			F32 widthScale,
			F32 heightScale
		);

	};
}

#endif
