#include "stdafx.h"

using namespace TexAtlas;

Attk::C_Status GridCropSheet::Build
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
)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Processing...\n");
		pProgressCallback->OnProgress(0);
	}

	Attk::C_Status status;
	if (csSize < maxImageSize)
		maxImageSize = csSize;
	if (border > minImageSize)
		minImageSize = border;

	if (pAftkScene->GetNumMaterialNodes() == 0)
	{
		return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_NO_MATERIALS);
	}

	//Create a database in which to store the crop information.
	Database db;

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

	Attk::C_Image cropImage;
	status = cropImage.NewPixelData(csSize, csSize, 1, Attk::E_PixelFormat_R8G8B8A8U);
	if (!status) return status;

	Attk::C_PixelA32R32G32B32F pixel;
	pixel.red = 255;
	pixel.green = 0;
	pixel.blue = 0;
	pixel.alpha = 0;
	cropImage.Fill(pixel);

	U32 cropSize = CalculateSubImageSize(textureSet, csSize);
	{
		std::stringstream ss;
		ss << "Sub image size " << cropSize << ".\n";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}
	if (cropSize < 4)
	{
		return Attk::C_Status(Attk::E_Status_Failure, "Too many textures for crop sheet. The sub images would be < 4px^2.");
	}

	status = InsertSubImages(
		textureSet,
		&cropImage,
		&db,
		cropSize, 
		csSize,
		border,
		minImageSize,
		maxImageSize,
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc,
		pProgressCallback);
	if (!status) return status;

	status = cropImage.Write(csFileName.c_str());
	if (!status) return status;

	Database::DBAtlasRecord r;
	r.fileName = csFileName;
	db.AddAtlasRecord(r);

	if (!db.Write(dbFileName))
		return Attk::E_Status_Failure;

	{
		std::stringstream ss;
		ss << "Wrote grid crop sheet [" << csFileName << "]. Containing " << (U32)textureSet.Size() << " textures.\n";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}

	return Attk::E_Status_Success;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

Attk::C_Status GridCropSheet::InsertSubImages(
	const TextureKeyArray& uniqueTextureSet, 
	Attk::C_Image* pCropImage, 
	Database* pDb,
	U32 subImageSize,
	U32 csSize,
	U32 borderSize,
	U32 minImageSize,
	U32 maxImageSize,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text),
	Progress* pProgressCallback)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Procesing Textures...\n");
		pProgressCallback->OnProgress(0);
	}

	U32 numTextures = (U32)uniqueTextureSet.Size();

	F32 workDone;
	U32 texturesProcessed = 0;
	U32 cropX = 0;
	U32 cropY = 0;

	U32 dbEntryIndex = 0;

	U32 count = 0;
	for (U32 i = 0; i < (U32)uniqueTextureSet.Size(); i++)
	{
		InsertSubImage(
			uniqueTextureSet[i], 
			pCropImage, 
			pDb,
			cropX, 
			cropY, 
			subImageSize,
			borderSize,
			minImageSize,
			maxImageSize,
			pMessageCallbackFunc,
			pWarningCallbackFunc,
			pErrorCallbackFunc);
		
		//Update progress.
		{
			texturesProcessed++;
			workDone = (F32)texturesProcessed / (F32)numTextures;
			workDone *= 100.f;
			if (pProgressCallback)
			{
				if (pProgressCallback->OnProgress(workDone))
				{
					//Cancel button clicked.
					return Attk::E_Status_Cancelled;
				}
			}
		}

		//Move to next insertion position.
		cropX += subImageSize;
		if (cropX+subImageSize > csSize)
		{
			cropX = 0;
			cropY += subImageSize;
		}

		count++;
		dbEntryIndex++;
	}

	return Attk::E_Status_Success;
}

Attk::C_Status GridCropSheet::InsertSubImage(
	const TextureKey& key,
	Attk::C_Image* pCropImage, 
	Database* pDb,
	U32 cropX, 
	U32 cropY, 
	U32 newSize,
	U32 minImageSize,
	U32 maxImageSize,
	U32 borderSize,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text))
{
	U32 imageSizeWithoutBorder = newSize - borderSize*2;
	U32 imageTopLeftWithoutBorderX = cropX+borderSize;
	U32 imageTopLeftWithoutBorderY = cropY+borderSize;

	Attk::C_Status status;
	Attk::C_Image image;
	status = image.Read(key.FileName().c_str());
	if (status)
	{
		U32 w = image.GetWidth(), h = image.GetHeight();
		AttkUt::C_ImageResize::Resize(image, imageSizeWithoutBorder, imageSizeWithoutBorder, 1);
		AttkUt::C_ImageResize::GrowBorder(image, borderSize);

		PostProcessImage(key.FileName().c_str(), &image, (F32)imageSizeWithoutBorder/(F32)w, (F32)imageSizeWithoutBorder/(F32)h);
		pCropImage->Insert(cropX,cropY, image);
	}
	else
	{
		//Display a message if we can't load a texture.
		Tu::StringBuilder sb;
		sb << CS_GEN_LOAD_TEXTURE_FAIL;
		TextOutput::DoOutput(pWarningCallbackFunc, sb.Text() + std::string("\n"));
	}

	//Build databse entry.
	Database::DBImageRecord r;

	//Get file date.
	if (!Tu::FileDate(key.FileName().c_str(), r.lowDate, r.highDate))
		return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_GET_FILE_DATE_FAIL);

	//Calculate UV offsets
	r.key = key;
	r.uScale = (F32)imageSizeWithoutBorder / (F32)pCropImage->GetWidth();
	r.vScale = (F32)imageSizeWithoutBorder / (F32)pCropImage->GetHeight();
	r.uOffset = (F32)imageTopLeftWithoutBorderX / (F32)pCropImage->GetWidth();
	r.vOffset = (F32)imageTopLeftWithoutBorderY / (F32)pCropImage->GetHeight();
	r.atlasIndex = 0;
	r.rotated = false;

	pDb->AddImageRecord(r);
	
	return Attk::E_Status_Success;
}

U32 GridCropSheet::CalculateSubImageSize(const TextureKeyArray& textureSet, U32 csSize)
{
	U32 slotSize = csSize;
	U32 numTextures = (U32)textureSet.Size();

	while (true)
	{
		//We can't fit this slot size within the crop-sheet, reduce slot size.
		if (slotSize == 1)
		{
			//We can't reduce the slot size any further. 
			//Abort with size zero.
			return 0;
		}

		U32 availableSlotsPerAxis = (csSize / slotSize);
		U32 availableSlots = availableSlotsPerAxis * availableSlotsPerAxis;
		if (availableSlots < numTextures)
		{
			//We can't fit all the textures in, reduce slot size.
			if (slotSize == 4)
			{
				//We can't make things any smaller!
				return 0;
			}
			//Find our next power of four size.
			slotSize--;
			while(slotSize % 4 != 0)
			{
				slotSize--;
			}
		}
		else
		{
			//We've found a size that will allow us to fit all of the textures in.
			return slotSize;
		}
	}

	return slotSize;
}

#endif
