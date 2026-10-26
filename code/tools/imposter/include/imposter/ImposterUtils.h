#ifndef IMPOSTER_IMPOSTERUTILS_H
#define IMPOSTER_IMPOSTERUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Imposter
{

static const char* kAutoLodFlag			= "AutoLodGeneration";

static const char* kImposterTypeParams[E_ImposterType_Count] = 
{
	"ScreenAligned",
	"Static_1_Mirrored",
	"Static_2_Mirrored",
	"Static_1",
	"Static_2"
};

struct ViewData
{
	struct ViewVector { F32 x, y, z; };

	const bool mirror;
	const U32 nImages;
	const ViewVector* pViewVectors;
};

E_ImposterType GetImposterType(const char* param);
ViewData GetViewData(const Imposter::E_ImposterType type);

RECT GetBoundingRect(const Attk::C_Image& image);
bool CropImage(Attk::C_Image& image, const RECT r);

Aftk::C_Bounds GetWorldBounds(const Aftk::C_SceneNode *pSceneNode);

template <typename T>
T UnsignedRoundUpPow2(T k)
{
	if (k == 0)
		return 1;

	k--;
	for (int i=1; i<sizeof(T)*CHAR_BIT; i<<=1)
		k = k | k >> i;

	return k+1;
}

template <typename T>
bool UnsignedIsPow2(T k)
{
	return (k & (k-1)) == 0;
}

inline Btl::String GenerateTextureName(const Btl::String& workingFolder, const Aftk::C_SceneNode* pSceneNode, U32 imageIndex)
{
	Btl::String name;
	name.Printf("%s\\%08x_%d_DIFF.tga", workingFolder.Chars(), pSceneNode->GetUniqueId(), imageIndex);

	return name;
}

void PostProcessImage(Attk::C_Image& image);

}

#endif