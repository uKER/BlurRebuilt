#include "stdafx.h"
#include <tex_atlas/SortedTextureList.h>
#include <tex_atlas/Space.h>
#include <tex_atlas/TexturePackingLists.h>
#include <tex_atlas/Atlas.h>

using namespace TexAtlas;

Attk::C_Status ScaleCropSheet::Build(
	const TextureKeyArray& textureSet,
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
	Progress* pProgressCallback)
{
	if (csWidthHeightArray.Size() == 0)
		return Attk::E_Status_Success;

	Attk::C_Status status;

	U32 firstAtlasWidth = csWidthHeightArray[0].first;
	U32 firstAtlasHeight = csWidthHeightArray[0].second;

	if (Maths::Min(firstAtlasWidth, firstAtlasHeight) < maxImageSize)
		maxImageSize = Maths::Min(firstAtlasWidth, firstAtlasHeight);
	if (border > minImageSize)
		minImageSize = border;

	status = CheckForMissingTextures(
		textureSet,
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc);
	if (!status) return status;

	TexturePackingLists texturePackingLists;
	status = texturePackingLists.Build(
		textureSet, 
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc,
		pProgressCallback);
	if (!status) return status;

	//Cache processing - progress state 1.
	//If there is an existing image cache, load this. Otherwise we'll be needing a new one.
	ImageCache imageCache;
	std::string icFileName = Tu::ReplaceFileExtension(csFileName, "ic");
	if (Tu::FileExists(icFileName))
	{
		//Use existing cache.
		imageCache.Read(icFileName, pProgressCallback);
		if (!status) return status;

		//Update any missing textures or changed textures.
		//Also, remove any unused textures.
		status = imageCache.Update(
			textureSet,
			border, 
			pProgressCallback);
		if (!status) return status;
	}
	else
	{
		//Create a new cache.
		status = imageCache.Build(
			textureSet, 
			border, 
			pProgressCallback);
		if (!status) return status;
	}

	//Create a new texture atlas.
	U32 numPackedTextures = 0;
	U32 atlasCount = 0;

	Database db;
	//Scale texture packing list and image cache.
	{
		//Perform image processing.
		if (pProgressCallback)
		{
			pProgressCallback->OnPrintProgress("Performing image processing...\n");
			pProgressCallback->OnProgress(0);
		}
		for (U32 iImage = 0; iImage < texturePackingLists.NumRecords(); iImage++)
		{
			TexRecord& layer0Record = 
				texturePackingLists.Record(CS_GEN_PACKING_LAYER, iImage);

			//All of the texture records in the first layer (CS_GEN_PACKING_LAYER) _must_ be used.
			assert(layer0Record.used);
			//Clamp texture record.
			ScaleTextureRecord(
				layer0Record,
				border,
				scale,
				clampSmallestSide,
				minImageSize,
				maxImageSize,
				firstAtlasWidth,
				firstAtlasHeight);

			for (U32 jLayer = 1; jLayer < texturePackingLists.NumLayers(); jLayer++)
			{
				TexRecord& r = 
					texturePackingLists.Record(jLayer, iImage);

				r.width = layer0Record.width;
				r.height = layer0Record.height;
			}

			for (U32 jLayer = 0; jLayer < texturePackingLists.NumLayers(); jLayer++)
			{
				TexRecord& r = 
					texturePackingLists.Record(jLayer, iImage);

				if (r.used)
				{
					//Update progress bar.
					if (pProgressCallback)
					{
						std::stringstream ss;
						ss << "Processing image [" << Tu::FileNameFromString(r.key.FileName()) << "]...\n";
						pProgressCallback->OnPrintProgress(ss.str());
						F32 progress = ((F32)iImage+1.f) /(F32)texturePackingLists.NumRecords();
						if (pProgressCallback->OnProgress(progress*100.f))
							return Attk::E_Status_Cancelled;
					}

					//Resample the image
					S32 cacheIndex = imageCache.RecordIndex(r.key);
					if (cacheIndex == -1)
						return Attk::E_Status_Cancelled;
					ImageCache::ICRecord& icRecord = imageCache.Record(cacheIndex);
					F32 imageWidthScale = (F32)r.width / (F32)r.originalWidth;
					F32 imageHeightScale = (F32)r.height / (F32)r.originalHeight;
					if (!(	icRecord.widthScale == imageWidthScale &&
							icRecord.heightScale == imageHeightScale &&
							icRecord.loaded))
					{
						if (!icRecord.missing)
						{
							if (r.key.FileName() == CS_GEN_RESERVED_TEXTURE_SPACE_NAME)
							{
								icRecord.image.NewPixelData(1,1,1, Attk::E_PixelFormat_R8G8B8A8U);
								U32 layerIndex = r.key.LayerIndex();
								if (layerIndex < (U32)layerBackgroundColourList.Size())
								{
									Attk::C_PixelA32R32G32B32F pixel;
									pixel.red = layerBackgroundColourList[layerIndex].X();
									pixel.green = layerBackgroundColourList[layerIndex].Y();
									pixel.blue = layerBackgroundColourList[layerIndex].Z();
									pixel.alpha = layerBackgroundColourList[layerIndex].W();
									icRecord.image.Fill(pixel);
								}
								else
								{
									Attk::C_PixelA32R32G32B32F pixel;
									pixel.red = 255;
									pixel.green = 0;
									pixel.blue = 0;
									pixel.alpha = 0;
									icRecord.image.Fill(pixel);
								}

							}
							else
							{
								status = icRecord.image.Read(r.key.FileName().c_str());
								if (!status)
								{
									break;
									//return status;
								}
								//Standardise pixel formats.
								if (icRecord.image.GetChannelsPerPixel() < 4)
									icRecord.image.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8U);
								else
									icRecord.image.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);
							}
							icRecord.loaded = true;

							AttkUt::C_ImageResize::Resize(icRecord.image, r.width-border*2, r.height-border*2, 1);
							//Increase the image border size.
							AttkUt::C_ImageResize::GrowBorder(icRecord.image, border);
							//Calculate scaling factor for image cache.
							icRecord.widthScale = imageWidthScale;
							icRecord.heightScale = imageHeightScale;

							PostProcessImage(
								icRecord.key.FileName().c_str(),
								&icRecord.image, 
								icRecord.widthScale,
								icRecord.heightScale);

						}
					}
				}
			}
		}
	}

	//Build a sorted texture list.
	List<U32> sortedTextureList;
	SortedTextureListBuilder::Build(&sortedTextureList, texturePackingLists);
	U32 numTexturesToPack = (U32)sortedTextureList.Size();


	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Packing atlases...\n");
		pProgressCallback->OnProgress(0);
	}

	while (numPackedTextures != numTexturesToPack)
	{
		U32 atlasSizeIndex = atlasCount;
		if (atlasSizeIndex >= (U32)csWidthHeightArray.Size())
		{
			assert(!"Pack list mismatch!");
			atlasSizeIndex = csWidthHeightArray.Size()-1;
		}

		U32 atlasWidth = csWidthHeightArray[atlasSizeIndex].first;
		U32 atlasHeight = csWidthHeightArray[atlasSizeIndex].second;
		Atlas atlas;
/*
		if (atlasCount == 0)
		{
			ListItem<U32>* pTextureIndexItem = sortedTextureList.Begin();
			U32 i = 0;
			while(pTextureIndexItem)
			{
				std::cout << i << " " << 
					pTextureIndexItem->GetData() << 
					texturePackingLists.Record(0, pTextureIndexItem->GetData()).key.FileName() << " " <<
					texturePackingLists.Record(0, pTextureIndexItem->GetData()).width << "," <<
					texturePackingLists.Record(0, pTextureIndexItem->GetData()).height << ","
					<< std::endl;
				pTextureIndexItem = pTextureIndexItem->GetNext();
			}
		}
*/
		status = atlas.Pack(
			atlasCount,
			atlasWidth, 
			atlasHeight, 
			Attk::E_PixelFormat_R8G8B8A8U,
			allowRotation,
			border,
			layerBackgroundColourList,
			db,
			sortedTextureList,
			texturePackingLists,
			imageCache,
			0);
		if (!status) return status;

		//Atlas is full, write current crop sheet and get a new one.
		status = atlas.Write(
			csFileName,
			atlasCount, 
			db, 
			pMessageCallbackFunc);

		atlasCount++;
		numPackedTextures += atlas.NumTextures(CS_GEN_PACKING_LAYER);

		if (pProgressCallback)
		{
			pProgressCallback->OnProgress(((F32)numPackedTextures / (F32)numTexturesToPack)*100.f);
		}
	}
	{
		std::stringstream ss;
		ss << "Packing complete. ";
		ss << atlasCount;
		if (atlasCount > 1)
		{
			ss << " crop sheets generated.\n";
		}
		else
		{
			ss << " crop sheet generated.\n";
		}
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}

	imageCache.Write(icFileName, pProgressCallback);
	//Write crop sheet database file.
	if (!db.Write(dbFileName))
		return Attk::E_Status_Failure;

	{
		std::stringstream ss;
		ss << "Wrote database file [" << dbFileName << "]";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}

	return Attk::E_Status_Success;
}

