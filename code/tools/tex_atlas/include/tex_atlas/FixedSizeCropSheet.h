#ifndef CS_GEN_SCALE_CROP_SHEET_H

namespace TexAtlas
{
	class FixedSizeCropSheet : public CropSheet
	{
	public:
		static Attk::C_Status Build
		(
			Aftk::C_Scene* pAftkScene,
			const std::string& dbFileName,
			const std::string& csFileName,
			U32 csSize,
			U32 subImageSize,
			U32 borderSize,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text),
			Progress* pProgressCallback	
		);

	private:
		struct IRecord
		{
			TextureKey key;
			U32 lowDate;
			U32 highDate;
			Attk::C_Image image;
			bool rotated;
			bool missing;
		};

		static Attk::C_Status InsertSubImage
		(
			Attk::C_Image* pCropImage, 
			const IRecord& ir,
			Database* pDb,
			U32 atlasIndex,
			U32 cropX, 
			U32 cropY,
			U32 subImageSize,
			U32 borderSize,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallbackFunc)(const std::string& text),
			void (*pErrorCallbackFunc)(const std::string& text)
		);
	};
};

#endif