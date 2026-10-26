#include "stdafx.h"
#include <set>

using namespace TexAtlas;

Attk::C_Status CropSheet::BuildTextureList
(
	TextureKeyArray& textureSet,
	const Aftk::C_Scene* pAftkScene,
	bool addBorderTexture,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text),
	Progress* pProgressCallback
)
{
	//Only examine materials that are actually _used_
	std::set<TextureKey> textureSetBuilder;
	Aftk::C_SceneNodeConstIterator it(pAftkScene);
	U32 textureBaseIndex = 0;
	while (it)
	{
		const Aftk::C_SceneNode* pNode = it;
		if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			Aftk::C_SceneMeshRemappableNode* pMesh = (Aftk::C_SceneMeshRemappableNode*)pNode;
			for (U16 iMaterial = 0; iMaterial < pMesh->GetNumPolygonMaterialMappings(); iMaterial++)
			{
				const Aftk::C_MaterialNode* pMaterial = &pMesh->GetPolygonMaterialMappingPtr(iMaterial)->GetContent();
				for (U32 jImage = 0; jImage < pMaterial->GetNumShaderImageNodes(); jImage++)
				{
					const Aftk::C_ShaderImageNode* pImage = pMaterial->GetShaderImageNodePtr(jImage);
					//Ignore lightmap textures
					std::string imageType;
					pImage->GetImageTypePlug().GetValue(imageType);
					if (imageType != "lightmap")
					{
						std::string lower;
						pImage->GetFilePlug().GetValue(lower);
						std::transform(lower.begin(), lower.end(), lower.begin(), tolower);

						//Add a date stamp for this file.
						TextureKey key;
						key.SetFileName(lower);
						key.SetBaseIndex(textureBaseIndex);
						key.SetLayerIndex(0);
						key.SetURepeats(1.f);
						key.SetVRepeats(1.f);
						key.SetIsBumpMap(false);
						U32 sizeBefore = (U32)textureSetBuilder.size();
						textureSetBuilder.insert(key);
						if (sizeBefore != (U32)textureSetBuilder.size())
							textureBaseIndex++;
					}
				}
			}
		}

		it++;
	}


	if (addBorderTexture)
	{
		textureBaseIndex++;
		TextureKey key;
		key.SetFileName(CS_GEN_RESERVED_TEXTURE_SPACE_NAME);
		key.SetBaseIndex(textureBaseIndex);
		key.SetLayerIndex(0);
		key.SetURepeats(1.f);
		key.SetVRepeats(1.f);
		key.SetIsBumpMap(false);
		textureSetBuilder.insert(key);
	}

	std::set<TextureKey>::const_iterator setit;
	for (setit = textureSetBuilder.begin(); setit != textureSetBuilder.end(); setit++)
	{
		textureSet.PushBack(*setit);
	}
	
	return Attk::E_Status_Success;
}

Attk::C_Status CropSheet::BuildTextureList
(
	TextureKeyArray& textureSet,
	const std::string& fileName,
	bool addBorderTexture,
	U32 minimumLayerCount,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text),
	Progress* pProgressCallback
)
{
	//Parse texture list file.
	std::ifstream file;
	file.open(fileName.c_str());
	if (!file.is_open())
		return Attk::E_Status_Failure;

	U32 numLayers = 0;
	S32 baseTextureIndex = -1;
	while(!file.eof())
	{
		std::string line;
		{
			char buffer[4096];
			file.getline(buffer, 4096);
			line = buffer;
		}
		if (line.length() == 0)
			continue;

		S32 fileNameStartIndex = -1;
		S32 fileNameEndIndex = (S32)line.length();
		for (S32 i = 0; i < fileNameEndIndex; i++)
		{
			if (Tu::IsAlphabetic(line[i]) || (line[i] == '\\'))
			{
				fileNameStartIndex = i;
				break;
			}
		}
		S32 fileNameLength = fileNameEndIndex - fileNameStartIndex;

		{
			U32 layerIndex;
			{
				std::stringstream layerStream;
				for (U32 i = 0; i < line.length(); i++)
				{
					if (!Tu::IsNumeric(line[i]))
						break;
					layerStream << line[i];
				}
				layerStream >> layerIndex;
			}
			if (layerIndex == 0)
				baseTextureIndex++;

			if (fileNameStartIndex != -1)
			{
				assert(fileNameEndIndex > 0);
				std::string imageFileName = line.substr(fileNameStartIndex, fileNameLength);
				assert(imageFileName.length() != 0);

				TextureKey key;
				key.SetFileName(imageFileName);
				key.SetBaseIndex(baseTextureIndex);
				key.SetLayerIndex(layerIndex);
				key.SetFlags(0);
				textureSet.PushBack(key);
			}
			if (layerIndex+1 > numLayers)
				numLayers = layerIndex+1;
		}
	}

	file.close();

	numLayers = max(minimumLayerCount, numLayers);

	if (addBorderTexture)
	{
		baseTextureIndex++;
		//Cater for that case where the texture list is empty - but we still want a place holder texture.
		if (numLayers == 0)
			numLayers = 1;
		for (U32 iLayer = 0; iLayer < numLayers; iLayer++)
		{
			TextureKey key;
			key.SetFileName(CS_GEN_RESERVED_TEXTURE_SPACE_NAME);
			key.SetBaseIndex(baseTextureIndex);
			key.SetLayerIndex(iLayer);
			key.SetURepeats(1.f);
			key.SetVRepeats(1.f);
			key.SetIsBumpMap(false);
			textureSet.PushBack(key);
		}
	}

	return Attk::E_Status_Success;
}

Attk::C_Status CropSheet::CheckForMissingTextures
(
	const TextureKeyArray& textureSet,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text)
)
{
	for (U32 i = 0; i < (U32)textureSet.Size(); i++)
	{
		const std::string& fileName = textureSet[i].FileName();
		if (fileName == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
			//Ignore placeholder texture.
			continue;
		if (_access(fileName.c_str(),0) == -1) 
		{
			//Display a message if we can't find a texture.
			Tu::StringBuilder sb;
			sb << CS_GEN_FILE_DOES_NOT_EXIST_FAIL;
			TextOutput::DoOutput(pWarningCallbackFunc, sb.Text() + std::string("\n"));
		}
	}
	return Attk::E_Status_Success;
}

Attk::C_Status CropSheet::ReExportRequired
(
	const TextureKeyArray& textureSet,
	const Database& db,
	bool* pReExportRequired,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallbackFunc)(const std::string& text),
	void (*pErrorCallbackFunc)(const std::string& text)
)
{
	Attk::C_Status status;

	if ((U32)textureSet.Size() != db.NumImageRecords())
	{
		//The database and crop sheet are out of date.
		*pReExportRequired = true;
		return Attk::E_Status_Success;
	}

	U32 dbEntryIndex = 0;
	for (U32 i = 0; i < (U32)textureSet.Size(); i++)
	{
		U32 currentLowDate;
		U32 currentHighDate;
		if (!Tu::FileDate(
			textureSet[i].FileName().c_str(),
			currentLowDate,
			currentHighDate))
		{
			*pReExportRequired = true;
			return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_GET_FILE_DATE_FAIL);
		}
		if (
			(currentLowDate != db.ImageRecord(dbEntryIndex).lowDate) &&
			(currentHighDate != db.ImageRecord(dbEntryIndex).highDate))
		{
			*pReExportRequired = true;
			return Attk::E_Status_Success;
		}

		dbEntryIndex++;
	}

	*pReExportRequired = false;
	return Attk::E_Status_Success;
}

void CropSheet::ScaleTextureRecord(
	TexRecord& texRecord, 
	U32 border,
	F32 scale,
	bool clampSmallestSide,
	U32 minImageSize, 
	U32 maxImageSize,
	U32 csWidth,
	U32 csHeight)
{
	//Min image size must be at least equal to twice the border size.
	assert(minImageSize >= border*2);

	//If the smallest side is equal to the minimum size, return.
	if (clampSmallestSide)
	{
		if (texRecord.width < texRecord.height)
			if (texRecord.width == minImageSize)
			{
				//Clamp height.
				texRecord.height = Maths::Min(texRecord.height, csHeight);
				return;
			}
		if (texRecord.height < texRecord.width)
			if (texRecord.height == minImageSize)
			{
				//Clamp width.
				texRecord.width = Maths::Min(texRecord.width, csWidth);
				return;
			}
	}

	F32 newWidth;
	F32 newHeight;

	//If the smallest side is less than the minimum image size after scaling then scale up the smallest
	//side. Store the scaling factor used, and scale up the longest size to preserve the aspect ratio.
	if ((clampSmallestSide) && (texRecord.width != texRecord.height))
	{
		//Width is smallest side.
		if (texRecord.width < texRecord.height)
		{
			//Need to calculate the scale factor that will reduce the smallest side of the image to the minimum
			//allowable scale.
			F32 minScale = (F32)minImageSize / (F32)texRecord.width;
			F32 clampedScale;
			if (scale < minScale)
				clampedScale = minScale;
			else
				clampedScale = scale;

			newWidth = (F32)texRecord.width;
			newWidth *= clampedScale;
			newHeight = (F32)texRecord.height;
			newHeight *= clampedScale;

			if (newWidth > maxImageSize)
			{
				//Clamp width to maxImageSize
				F32 scaleFactor = (F32)maxImageSize / (F32)newWidth;
				newWidth = (F32)maxImageSize;
				newHeight = newHeight*scaleFactor;
			}
			else if (newWidth < minImageSize)
			{
				//Clamp width to minImageSize
				F32 scaleFactor = (F32)minImageSize / (F32)newWidth;
				newWidth = (F32)minImageSize;
				newHeight = newHeight*scaleFactor;
			}
		}
		//Height is smallest side
		else
		{
			//Need to calculate the scale factor that will reduce the smallest side of the image to the minimum
			//allowable scale.
			F32 minScale = (F32)minImageSize / (F32)texRecord.height;
			F32 clampedScale;
			if (scale < minScale)
				clampedScale = minScale;
			else
				clampedScale = scale;

			newWidth = (F32)texRecord.width;
			newWidth *= clampedScale;
			newHeight = (F32)texRecord.height;
			newHeight *= clampedScale;

			if (newHeight > maxImageSize)
			{
				//Clamp height maxImageSize
				F32 scaleFactor = (F32)maxImageSize / (F32)newHeight;
				newHeight = (F32)maxImageSize;
				newWidth = newWidth*scaleFactor;
			}
			else if (newHeight < minImageSize)
			{
				//Clamp height minImageSize
				F32 scaleFactor = (F32)minImageSize / (F32)newHeight;
				newHeight = (F32)minImageSize;
				newWidth = newWidth*scaleFactor;
			}
		}
	}
	//Clamp square textures
	if (texRecord.width == texRecord.height)
	{
		//Perform calculations with floating-point accuracy.
		newWidth = (F32)texRecord.width;
		newWidth *= scale;
		newHeight = (F32)texRecord.height;
		newHeight *= scale;

		//Perform clamping for both width and height
		if (newWidth > (F32)maxImageSize)
			newWidth = (F32)maxImageSize;
		if (newHeight > (F32)maxImageSize)
			newHeight = (F32)maxImageSize;
	}



	texRecord.width = (U32)Tu::Round(newWidth);
	texRecord.height = (U32)Tu::Round(newHeight);

	//Clamp minimum size.
	if (texRecord.width < minImageSize)
		texRecord.width = minImageSize;
	if (texRecord.height < minImageSize)
		texRecord.height = minImageSize;

	//Clamp to pow2
	if (!IsPowerOf2(texRecord.width))
		texRecord.width = RoundUpPow2(texRecord.width);
	if (!IsPowerOf2(texRecord.height))
		texRecord.height = RoundUpPow2(texRecord.height);

	texRecord.width = Maths::Min(texRecord.width, csWidth);
	texRecord.height = Maths::Min(texRecord.height, csHeight);
}


Attk::C_Status CropSheet::PostProcessImage(
	const char* pFilename,
	Attk::C_Image *pCropImage,
	F32 widthScale,
	F32 heightScale)
{
	const FileTag::C_Tag* pTag = 
		FileTag::C_TagDBManager::Instance().GetTagPtr(pFilename);
	if (!pTag)
		return Attk::E_Status_Success;

	for (U32 i = 0; i < pTag->GetNumAttributes(); i++)
	{
		const FileTag::C_GenericAttribute& gAttr = pTag->GetAttributes()[i];
		switch (gAttr.GetDataType())
		{
			case FileTag::C_GenericAttribute::E_DataType_Boolean:
			{
				if (gAttr.GetName() == "Enhance Alpha Contrast")
				{
					bool value;
					gAttr.GetValue(value);
					if (value)
					{
					AttkUt::C_ImageAdjustmentCurve<AttkUt::Point2> adjustmentCurve =
						AttkUt::CreateContrastEnhancementCurve(100, 0);
					AttkUt::C_ImageAdjustment::Adjust(
						*pCropImage,
						adjustmentCurve, 
						AttkUt::C_ImageAdjustment::E_AdjustmentChannel_Alpha);
					}
				}
				break;
			}

			default:
				break;

		}
	}

	return Attk::E_Status_Success;
}