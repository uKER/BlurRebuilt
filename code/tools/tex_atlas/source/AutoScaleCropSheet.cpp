#include "stdafx.h"
#include <tex_atlas/SortedTextureList.h>
#include <tex_atlas/Space.h>
#include <tex_atlas/TexturePackingLists.h>
#include <tex_atlas/Atlas.h>

using namespace TexAtlas;

Attk::C_Status AutoScaleCropSheet::BuildAtlasSizeList
(
	Ut::Vector<Ut::Pair<U32, U32> >& csWidthHeightArray,
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
)
{
	//Minimum image size cannot be less than twice the border size.
	if (minImageSize < border*2)
		minImageSize = border*2;

	Attk::C_Status status;
	TextOutput::DoOutput(pMessageCallbackFunc, "BuildAtlasSizeList...\n");

	TexturePackingLists texturesToPack(_texturePackingLists);
	//Store bits indicating if a texture has been packed previously or not. Index by base index.
	Ut::UtSet<std::string> texturesPacked;

	U32 nBaseTexturesPacked = 0;
	U32 nBaseTexturesToPack = texturesToPack.NumRecords();
	//Loop until we've been able to dry-run pack everything.
	while(nBaseTexturesPacked != nBaseTexturesToPack)
	{
		assert(nBaseTexturesPacked < nBaseTexturesToPack);
		//Build a texture packing list from our remaining textures.
		TextureKeyArray textureKeyArray;
		TexturePackingLists texturePackingListsForDryRun;
		{
			U32 newBaseIndex = 0;

			U32 added = 0;
			//Make a new texture packing list from our remaining textures.
			for (U32 iImage = 0; iImage < nBaseTexturesToPack; iImage++)
			{
				//Copy all layers into our list to pack.
				bool inserted = false;
				for (U32 jLayer = 0; jLayer < texturesToPack.NumLayers(); jLayer++)
				{
					TexRecord r = texturesToPack.Record(jLayer, iImage);
					r.key.SetBaseIndex(newBaseIndex);

					const std::string& str = r.key.FileName();

					//Ignore previously packed textures.
					Ut::UtSet<std::string>::Iterator it = 
						texturesPacked.Find(str);
					if (it != texturesPacked.End())
						//Ignore this texture and all it's layers.
						break;

					//Pack this texture.
					if (r.key.BaseIndex() != -1 && r.key.LayerIndex() != -1)
					{
						if (jLayer == 0)
						{
//							std::cout << "ADDED " << r.key.FileName() << std::endl;
							added ++;
						}
						textureKeyArray.PushBack(r.key);
						inserted = true;
					}
				}
				if (inserted)
					newBaseIndex++;
			}

			//Sort our texture list to pack.
			texturePackingListsForDryRun.Build(
				textureKeyArray,
				pMessageCallbackFunc,
				pWarningCallbackFunc,
				pErrorCallbackFunc,
				0);

			for (U32 baseIndex = 0; baseIndex < texturePackingListsForDryRun.NumRecords(); baseIndex++)
			{
				const TexRecord& r = texturePackingListsForDryRun.Record(CS_GEN_PACKING_LAYER, baseIndex);
				//Flag as having packed this texture.
//				std::cout << "TEST : "<< r.key.FileName() << std::endl;
				if (texturesPacked.Find(r.key.FileName()) != texturesPacked.End())
					assert("!Textures in the given list must be unique!");
			}
			S32 n = texturePackingListsForDryRun.NumRecords();
			S32 remaining = nBaseTexturesToPack - nBaseTexturesPacked;
			assert(n <= remaining);
		}

		//Scale texture packing list for dry run only.
		for (U32 iImage = 0; iImage < texturePackingListsForDryRun.NumRecords(); iImage++)
		{
			TexRecord& layer0Record = 
				texturePackingListsForDryRun.Record(CS_GEN_PACKING_LAYER, iImage);

			//All of the texture records in the first layer (CS_GEN_PACKING_LAYER) _must_ be used.
			assert(layer0Record.used);

			ScaleTextureRecord(
				layer0Record,
				border,
				1.f,
				clampSmallestSide,
				minImageSize,
				maxImageSize,
				csAtlasMaxWidth,
				csAtlasMaxHeight);

			for (U32 jLayer = 1; jLayer < texturePackingListsForDryRun.NumLayers(); jLayer++)
			{
				TexRecord& r = 
					texturePackingListsForDryRun.Record(jLayer, iImage);
				//Don't bother sizing unused texture records.
				if (!r.used)
					continue;

				r.width = layer0Record.width;
				r.height = layer0Record.height;
			}
		}

		Ut::UtSet<U32> baseIndicesToFlagAsPacked;
		List<U32> _sortedTextureIndices;
		U32 nBaseRecords = texturePackingListsForDryRun.NumRecords();
		for (U32 iImage = 0; iImage < nBaseRecords; iImage++)
		{
			baseIndicesToFlagAsPacked.Insert(iImage);
		}
		status = SortedTextureListBuilder::Build(
			&_sortedTextureIndices, 
			texturePackingListsForDryRun);
		if (!status) return status;

		//Pack until all we've no more textures or we exceed our max atlas size.
		U32 atlasWidth = minImageSize;
		U32 atlasHeight = minImageSize;
		while(true)
		{
			//Reset ourselves.
			List<U32> sortedTextureIndices = _sortedTextureIndices;

			//Create a new texture atlas.
			U32 numPackedTexturesInThisAtlas = 0;
			U32 numTexturesToPack = (U32)sortedTextureIndices.Size();
			Atlas atlas;
/*
			if (atlasWidth == 32 && atlasHeight == 32)
			{
				ListItem<U32>* pTextureIndexItem = sortedTextureIndices.Begin();
				U32 i = 0;
				while(pTextureIndexItem)
				{
					std::cout << i << " " << 
						pTextureIndexItem->GetData() << 
						texturePackingListsForDryRun.Record(0, pTextureIndexItem->GetData()).key.FileName() << " " <<
						texturePackingListsForDryRun.Record(0, pTextureIndexItem->GetData()).width << "," <<
						texturePackingListsForDryRun.Record(0, pTextureIndexItem->GetData()).height << ","
						<< std::endl;
					pTextureIndexItem = pTextureIndexItem->GetNext();
				}
			}
*/
			status = atlas.PackDryRun(
				atlasWidth, 
				atlasHeight, 
				allowRotation,
				border,
				texturePackingListsForDryRun,
				sortedTextureIndices);
			if (!status) return status;

			//We've still got some textures remaining, grow atlas.
			//Increment smallest dimension.
			U32 lastAtlasWidth = atlasWidth;
			U32 lastAtlasHeight = atlasHeight;
			if (atlasWidth < atlasHeight)
				atlasWidth <<= 1;
			else
				atlasHeight <<= 1;

			if (
				sortedTextureIndices.Size() == 0 ||
				(lastAtlasWidth >= csAtlasMaxWidth && lastAtlasHeight >= csAtlasMaxHeight))
			{
				csWidthHeightArray.PushBack(Ut::Pair<U32, U32>(lastAtlasWidth, lastAtlasHeight));
				numPackedTexturesInThisAtlas = numTexturesToPack - (U32)sortedTextureIndices.Size();
				nBaseTexturesPacked += numPackedTexturesInThisAtlas;
//				std::cout << "Predicted packing " << numPackedTexturesInThisAtlas << " base textures into atlas " << csWidthHeightArray.Size() << std::endl;

				//Unflag anything we didn't manager to pack.
				ListItem<U32>* pTextureIndexItem = sortedTextureIndices.Begin();
				U32 nFlagged = 0;
				U32 originalFlagCount = baseIndicesToFlagAsPacked.Size();
				while(pTextureIndexItem)
				{
					U32 baseIndex = pTextureIndexItem->GetData();
					//We've packed the texture at this base index, remove it from our list to flag.
					Ut::UtSet<U32>::Iterator it = baseIndicesToFlagAsPacked.Find(baseIndex);
					if (it != baseIndicesToFlagAsPacked.End())
					{
						baseIndicesToFlagAsPacked.Erase(it);
						nFlagged++;
					}

					//Trundle along our list.
					pTextureIndexItem = pTextureIndexItem->GetNext();
				}
				{
					U32 test = baseIndicesToFlagAsPacked.Size();
					assert(test == numPackedTexturesInThisAtlas);
				}

				//Flag packed textures.
				for (Ut::UtSet<U32>::Iterator it = baseIndicesToFlagAsPacked.Begin(); it != baseIndicesToFlagAsPacked.End(); ++it)
				{
					U32 baseIndex = *it;
					const TexRecord& r = texturePackingListsForDryRun.Record(CS_GEN_PACKING_LAYER, baseIndex);
					//Flag as having packed this texture.
//					std::cout << "PACKED : "<< r.key.FileName() << std::endl;
					if (texturesPacked.Find(r.key.FileName()) != texturesPacked.End())
						assert(!"Textures in the given list must be unique!");
					texturesPacked.Insert(r.key.FileName());
				}

				//We're done!
				break;
			}
		}
	}

//	std::cout << "Predicted " << "atlases" << csWidthHeightArray.Size() << std::endl;
//	for (U32 i = 0; i < (U32)csWidthHeightArray.Size(); i++)
//		std::cout << "\t" << csWidthHeightArray[i].first << "," << csWidthHeightArray[i].second << std::endl;

	return Attk::E_Status_Success;
}

Attk::C_Status AutoScaleCropSheet::Build
(
	const TextureKeyArray& textureSet,
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
)
{
	if (mode == UNKNOWN)
		return Attk::E_Status_Failure;

	TextOutput::DoHeader(pMessageCallbackFunc);

	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Processing...\n");
		pProgressCallback->OnProgress(0);
	}

	Attk::C_Status status;

	if (Maths::Min(csWidth, csHeight) < maxImageSize)
		maxImageSize = Maths::Min(csWidth, csHeight);
	if (border > minImageSize)
		minImageSize = border;

	bool scaleFound;
	F32 scale;

	TexturePackingLists texturePackingLists;
	status = texturePackingLists.Build(
		textureSet, 
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc,
		pProgressCallback);
	if (!status) return status;
	U32 nRecords = texturePackingLists.NumRecords();

	Ut::Vector<Ut::Pair<U32, U32> > csWidthHeightArray;
	if (mode == AUTO_FIND_CROP_SHEET_SIZE_MULTI)
	{
		status = BuildAtlasSizeList(
			csWidthHeightArray,
			csWidth,
			csHeight,
			border,
			minimumScale,
			minImageSize,
			maxImageSize,
			allowRotation,
			clampSmallestSide,
			texturePackingLists,
			pMessageCallbackFunc,
			pWarningCallbackFunc,
			pErrorCallbackFunc,
			pProgressCallback);
		if (!status) return status;
		scale = 1.f;
		scaleFound = true;
	}
	else if (mode == AUTO_FIND_CROP_SHEET_SIZE)
	{
		//From the minimum scale and image size, work out the texture area for 100% packing.
        //We can then reject certain space combinations.
		U32 textureArea = 0;
		for (U32 iTex = 0; iTex < texturePackingLists.NumRecords(); iTex++)
		{
			TexRecord r =
				texturePackingLists.Record(CS_GEN_PACKING_LAYER, iTex);

			ScaleTextureRecord(
				r,
				border,
				minimumScale,
				clampSmallestSide,
				minImageSize,
				minImageSize,
				csWidth,
				csHeight);

			if (!r.missing)
				textureArea += r.width * r.height;
		}

		//Build a list of possible crop sheet area combinations.
		Ut::UtVector<Ut::DynamicVector<Space> > spaces;
		U32 currentCSWidthPow2 = 4;
		while(true)
		{
			//Height and width increment in powers of 2
			U32 currentCSWidth = (U32)pow(2.0, (int)currentCSWidthPow2);
			if (currentCSWidth > csWidth)
				break;

			U32 currentCSHeightPow2 = 4;
			while(true)
			{
				U32 currentCSHeight = (U32)pow(2.0, (int)currentCSHeightPow2);
				if (currentCSHeight > csHeight)
					break;
				Space space;
				space.Set(0,0, currentCSWidth, currentCSHeight);
				//If space can fit 100% efficient packing then it is potentially valid.
				if (space.Area() >= (S32)textureArea)
					spaces.PushBack(space);

				currentCSHeightPow2++;
			}

			currentCSWidthPow2++;
		}

		//Return if no space.
		if (spaces.Size() == 0)
			return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_COULD_NOT_PACK_TO_CONSTRAINTS);
		//Sort our list of spaces.
		{
			Space* pBegin = &spaces[0];
			Space* pEnd = pBegin + spaces.Size();
			std::sort(pBegin, pEnd);
		}

		for (U32 iSpace = 0; iSpace < (U32)spaces.Size(); iSpace++)
		{
			const Space& space = spaces[iSpace];
			std::stringstream ss;
			ss << "Attempting to pack in to " << space.BottomRightCorner().X() << "x" << space.BottomRightCorner().Y() << " area " << space.Area() << " atlas...\n";
			pMessageCallbackFunc(ss.str());
			status = FindOptimalScale(
				scaleFound,
				scale,
				space.BottomRightCorner().X(),
				space.BottomRightCorner().Y(),
				border,
				minimumScale,
				minImageSize,
				maxImageSize,
				allowRotation,
				clampSmallestSide,
				texturePackingLists,
				pMessageCallbackFunc,
				pWarningCallbackFunc,
				pErrorCallbackFunc,
				pProgressCallback);
			if (!status) return status;
			if (scaleFound)
			{
				csWidth = space.BottomRightCorner().X();
				csHeight = space.BottomRightCorner().Y();
				break;
			}
		}
	}
	else
	{
		status = FindOptimalScale(
			scaleFound,
			scale,
			csWidth,
			csHeight,
			border,
			minimumScale,
			minImageSize,
			maxImageSize,
			allowRotation,
			clampSmallestSide,
			texturePackingLists,
			pMessageCallbackFunc,
			pWarningCallbackFunc,
			pErrorCallbackFunc,
			pProgressCallback);
		if (!status) return status;
	}
	if (!scaleFound)
		return Attk::C_Status(Attk::E_Status_Failure, CS_GEN_COULD_NOT_PACK_TO_CONSTRAINTS);
	{
		std::stringstream ss;
		ss << "Optimal scale for " << csWidth << "x" << csHeight << " atlas was found to be " << scale << "\n";
		TextOutput::DoOutput(pMessageCallbackFunc, ss);
	}

	//Generate a scale crop sheet from our optimal scaling factor.
	status = ScaleCropSheet::Build(
		textureSet,
		generateNormalMaps,
		dbFileName,
		csFileName,
		csWidthHeightArray,
		scale,
		border,
		minImageSize,
		maxImageSize,
		allowRotation,
		clampSmallestSide,
		layerBackgroundColourList,
		pMessageCallbackFunc,
		pWarningCallbackFunc,
		pErrorCallbackFunc,
		pProgressCallback);

	if (!status) return status;

	return Attk::E_Status_Success;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

Attk::C_Status AutoScaleCropSheet::FindOptimalScale
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
)
{
	//Minimum image size cannot be less than twice the border size.
	if (minImageSize < border*2)
		minImageSize = border*2;

	Attk::C_Status status;
	TextOutput::DoOutput(pMessageCallbackFunc, "Attempting to find optimal packing scale...\n");

	static const U32 MAX_ITERATIONS = 99;
	//Start off at full-size.
	scale = 1.0f;
	F32 stepScale = 0.01f;
	//Perform dry-run packing to find best fit scale.
	U32 iteration = 0;
	
	while(true)
	{
		TexturePackingLists texturePackingListsForDryRun(texturePackingLists);
		
		//Scale texture packing list for dry run only.
		for (U32 iImage = 0; iImage < texturePackingListsForDryRun.NumRecords(); iImage++)
		{
			TexRecord& layer0Record = 
				texturePackingListsForDryRun.Record(CS_GEN_PACKING_LAYER, iImage);

			//All of the texture records in the first layer (CS_GEN_PACKING_LAYER) _must_ be used.
			assert(layer0Record.used);

			ScaleTextureRecord(
				layer0Record,
				border,
				scale,
				clampSmallestSide,
				minImageSize,
				maxImageSize,
				0xffffffff,
				0xffffffff);

			for (U32 jLayer = 1; jLayer < texturePackingListsForDryRun.NumLayers(); jLayer++)
			{
				TexRecord& r = 
					texturePackingListsForDryRun.Record(jLayer, iImage);
				//Don't bother sizing unused texture records.
				if (!r.used)
					continue;

				r.width = layer0Record.width;
				r.height = layer0Record.height;
			}
		}

		List<U32> sortedTextureIndices;
		status = SortedTextureListBuilder::Build(&sortedTextureIndices, texturePackingListsForDryRun);
		if (!status) return status;

		//Create a new texture atlas.
		U32 numPackedTextures = 0;
		U32 numTexturesToPack = (U32)sortedTextureIndices.Size();
		{
			Atlas atlas;
			status = atlas.PackDryRun(
				csWidth, 
				csHeight, 
				allowRotation,
				border,
				texturePackingListsForDryRun,
				sortedTextureIndices);
			if (!status) return status;
		
			if (sortedTextureIndices.Size() == 0)
			{
				scaleFound = true;
				break;
			}

			//Try again with a smaller scale.
			scale -= stepScale;
			if ((scale < 1e-10) || (scale < minimumScale))
			{
				scaleFound = false;
				break;
			}
		}

		iteration++;

		//Break - scaling will have no effect.
		if (minImageSize == maxImageSize)
		{
			scaleFound = false;
			break;
		}

		if (iteration == MAX_ITERATIONS)
		{
			scaleFound = false;
			break;
		}
	}

	return Attk::E_Status_Success;
}

#endif
