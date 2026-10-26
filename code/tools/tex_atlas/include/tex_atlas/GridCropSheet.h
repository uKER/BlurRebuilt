#ifndef CS_GEN_GRID_CROP_SHEET_H

namespace TexAtlas
{
	class GridCropSheet : public CropSheet
	{
	public:
		static Attk::C_Status Build
		(
			Aftk::C_Scene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			U32 border,
			U32 minImageSize,
			U32 maxImageSize,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

	private:
		static U32 CalculateSubImageSize(
			const TextureKeyArray& textureSet, 
			U32 csSize);

		static Attk::C_Status InsertSubImages
		(
			const TextureKeyArray& textureSet, 
			Attk::C_Image* pCropImage,
			Database* pDb,
			U32 cropSize, 
			U32 csSize,
			U32 border,
			U32 minImageSize,
			U32 maxImageSize,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback
		);

		static Attk::C_Status InsertSubImage
		(
			const TextureKey& key,
			Attk::C_Image* pCropImage, 
			Database* pDb,
			U32 cropX, 
			U32 cropY, 
			U32 newSize,
			U32 border,
			U32 minImageSize,
			U32 maxImageSize,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text)
		);
	};
};

#endif