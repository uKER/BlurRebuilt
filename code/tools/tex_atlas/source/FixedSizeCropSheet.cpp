#include "stdafx.h"

using namespace TexAtlas;

Attk::C_Status FixedSizeCropSheet::Build
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
)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Processing...\n");
		pProgressCallback->OnProgress(0);
	}

	Attk::C_Status status;
	if (pAftkScene->GetNumMaterialNodes() == 0)
	{
		return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_NO_MATERIALS);
	}

	//Build unique texture set.
	TextureKeyArray textureSet;
	status = BuildTextureList(
		textureSet,
		pAftkScene,
		false,
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc,
		pProgressCallback);
	if (!status) return status;

	status = CheckForMissingTextures(
		textureSet,
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc);
	if (!status) return status;

	//Load all textures at specified sub-image size.
	//If subImageSize > csSize then set image size to csSize.
	U32 texturesRemaining = (U32)textureSet.Size();
	Ut::UtVector< Ut::DynamicVector<IRecord> > textureList;
	textureList.Resize(texturesRemaining);
	{
		U32 iRecord = 0;

		for (U32 i = 0; i < (U32)textureSet.Size(); i++, iRecord++)
		{
			IRecord& r = textureList[iRecord];
			r.key = textureSet[i];
			r.image.Read(textureSet[i].FileName().c_str());

			if (!Tu::FileDate(r.key.FileName().c_str(), r.lowDate, r.highDate))
			{
				r.missing = true;
			}
			else
			{
				r.missing = false;
				U32 w = r.image.GetWidth(), h = r.image.GetHeight();
				//Scale image...
				AttkUt::C_ImageResize::Resize(r.image, subImageSize-borderSize*2, subImageSize-borderSize*2, 1);
				AttkUt::C_ImageResize::GrowBorder(r.image, borderSize);

				PostProcessImage(r.key.FileName().c_str(), &r.image, (F32)subImageSize/(F32)w, (F32)subImageSize/(F32)h);

			}
		}
	}

	//How many images / crop?
	U32 imagesPerRowAndColumn = csSize / subImageSize;
	U32 imagesPerCrop = imagesPerRowAndColumn*imagesPerRowAndColumn;

	//Create a database in which to store the crop information.
	Database db;

	//Populate crop sheets until no textures remaining.
	U32 textureIndex = 0;
	U32 atlasCount = 0;
	while(texturesRemaining != 0)
	{
		U32 texturesForThisCrop = texturesRemaining;
		if (texturesForThisCrop > imagesPerCrop)
			texturesForThisCrop = imagesPerCrop;
		
		//Populate a single crop sheet.
		//Create crop sheet image.
		Attk::C_Image cropImage;
		status = cropImage.NewPixelData(csSize, csSize, 1, Attk::E_PixelFormat_R8G8B8A8U);
		if (!status) return status;

		Attk::C_PixelA32R32G32B32F pixel;
		pixel.red = 255;
		pixel.green = 0;
		pixel.blue = 0;
		pixel.alpha = 0;
		cropImage.Fill(pixel);

		U32 cropX = 0;
		U32 cropY = 0;
		for (U32 iTexture = 0; iTexture < texturesForThisCrop; iTexture++)
		{
			status = InsertSubImage(
				&cropImage,
				textureList[textureIndex],
				&db,
				atlasCount,
				cropX, 
				cropY,
				subImageSize,
				borderSize,
				pMessageCallbackFunc,
				pWarningCallbackFunc,
				pErrorCallbackFunc);
			if (!status) return status;

			//Move to next insertion position.
			cropX += subImageSize;
			if (cropX+subImageSize > csSize)
			{
				cropX = 0;
				cropY += subImageSize;
			}

			textureIndex++;
			texturesRemaining--;
		}

		//Atlas is full, write current crop sheet and get a new one.
		std::string atlasFileName = Tu::CreateNumberedFileName(csFileName, atlasCount);
		status = cropImage.Write(atlasFileName.c_str());
		Database::DBAtlasRecord r;
		r.fileName = atlasFileName;
		db.AddAtlasRecord(r);
		std::stringstream ss;
		ss << "Wrote fixed size crop sheet [" << atlasFileName << "]. Containing " << texturesForThisCrop << " textures." << "\n";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
		atlasCount++;
	}

	//Save database.
	if (!db.Write(dbFileName))
		return Attk::E_Status_Failure;

	{
		std::stringstream ss;
		ss << "Wrote database file [" << dbFileName << "]\n";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}

	return Attk::E_Status_Success;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

Attk::C_Status FixedSizeCropSheet::InsertSubImage
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
)
{
	U32 imageSizeWithoutBorder = subImageSize - borderSize*2;
	U32 imageTopLeftWithoutBorderX = cropX+borderSize;
	U32 imageTopLeftWithoutBorderY = cropY+borderSize;

	//Calculate UV offsets
	Database::DBImageRecord r;
	r.key = ir.key;
	r.lowDate = ir.lowDate;
	r.highDate = ir.highDate;
	r.uScale = (F32)imageSizeWithoutBorder / (F32)pCropImage->GetWidth();
	r.vScale = (F32)imageSizeWithoutBorder / (F32)pCropImage->GetHeight();
	r.uOffset = (F32)imageTopLeftWithoutBorderX / (F32)pCropImage->GetWidth();
	r.vOffset = (F32)imageTopLeftWithoutBorderY / (F32)pCropImage->GetHeight();
	r.atlasIndex = atlasIndex;
	r.rotated = false;
	pDb->AddImageRecord(r);

	pCropImage->Insert(cropX,cropY, ir.image);

	return Attk::E_Status_Success;
}

#endif

