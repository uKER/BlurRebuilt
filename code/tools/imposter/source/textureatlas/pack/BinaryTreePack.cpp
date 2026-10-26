#include "ImposterPch.h"

#include "TextureAtlas/ImageDescription.h"
#include "TextureAtlas/Pack/BinaryTreePack.h"

#include <algorithm>

namespace TextureAtlas
{
Debug::Logger& g_Log = Debug::GetLogger("tools.Imposter.TextureAtlas");

static bool SortDescriptors(const C_InputImageDescription* l, const C_InputImageDescription* r)
{
	if (l->size.y == r->size.y)
		return l->size.x > r->size.x;
	else
		return l->size.y > r->size.y;
}

C_BinaryTreePack::C_BinaryTreePack()
{
}

C_BinaryTreePack::~C_BinaryTreePack()
{
}


bool C_BinaryTreePack::Pack(const InputImageDescArray& input, OutputImageDescArray& output, const ImageSizeArray& atlasSizes, U32 border)
{
	if (!input.Size() || !atlasSizes.Size())
		return true;

	Btl::Vector<const C_InputImageDescription*> workingList;
	for (S32 i=0; i<input.Size(); ++i)
		workingList.PushBack(&input[i]);

	std::sort(workingList.Begin(), workingList.End(), SortDescriptors);

	Btl::Vector< C_TreePackNode<C_InputImageDescription> > atlasNodes;
	for (U32 i=0; i<(U32)atlasSizes.Size(); ++i)
		atlasNodes.PushBack(C_TreePackNode<C_InputImageDescription>(atlasSizes[i], border));

	for (U32 i=0; i<(U32)workingList.Size(); ++i)
	{
		const C_TreePackNode<C_InputImageDescription>* pPackResult = 0;
		for (S32 j=0; j<(S32)atlasNodes.Size(); ++j)
		{
			pPackResult = atlasNodes[j].Insert(workingList[i]);
			if (pPackResult)
			{
				output.PushBack(C_OutputImageDescription(workingList[i]->fileName, pPackResult->GetBounds(), j));
				break;
			}
		}

		if (!pPackResult)
			return false;
	}

	return true;
}



}