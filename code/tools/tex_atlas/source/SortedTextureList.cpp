#include "stdafx.h"
#include <tex_atlas/SortedTextureList.h>

using namespace TexAtlas;

Attk::C_Status SortedTextureListBuilder::Build(
	List<U32>* pTextureList,
	const TexturePackingLists& packingList)
{
	if (!pTextureList)
		return Attk::E_Status_Failure;

	for (U32 imageIndex = 0; imageIndex < packingList.NumRecords(); imageIndex++)
	{
		Insert(
			pTextureList,
			packingList,
			imageIndex);
	}

	return Attk::E_Status_Success;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void SortedTextureListBuilder::Insert(
	List<U32>* pTextureList, 
	const TexturePackingLists& packingList, 
	U32 imageIndex)
{
	//Find suitable insertion point.
	ListItem<U32>* pTextureIndexItem = pTextureList->Begin();

	U32 newTextureArea =
		packingList.Width(CS_GEN_PACKING_LAYER, imageIndex) * 
		packingList.Height(CS_GEN_PACKING_LAYER, imageIndex);

	bool inserted = false;
	while (pTextureIndexItem)
	{
		U32 width = packingList.Width(CS_GEN_PACKING_LAYER, pTextureIndexItem->GetData());
		U32 height = packingList.Height(CS_GEN_PACKING_LAYER, pTextureIndexItem->GetData());
		U32 textureArea = width * height;
		if (newTextureArea > textureArea)
		{
			//Insert here.
			pTextureList->AddBefore(imageIndex, pTextureIndexItem);
			inserted = true;
			break;
		}
		else if (newTextureArea == textureArea)
		{
			if (packingList.Width(CS_GEN_PACKING_LAYER, imageIndex) > width)
			{
				//Insert here.
				pTextureList->AddBefore(imageIndex, pTextureIndexItem);
				inserted = true;
				break;
			}
			else
			{
				//Insert here.
				pTextureList->AddBefore(imageIndex, pTextureIndexItem);
				inserted = true;
				break;
			}
		}
		pTextureIndexItem = pTextureIndexItem->GetNext();
	}
	if (!inserted)
	{
		pTextureList->Add(imageIndex);
	}
}

#endif

