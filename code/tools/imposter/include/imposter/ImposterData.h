#ifndef IMPOSTER_IMPOSTERDATA_H
#define IMPOSTER_IMPOSTERDATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Imposter
{
	enum E_ImposterType
	{
		E_ScreenAligned,
		E_Static1_Mirrored,			
		E_Static2_Mirrored,
		E_Static1,
		E_Static2,

		E_ImposterType_Count
	};

	class C_ImageHandle;

	struct ImposterData
	{
		static void InitViews();

		struct ImageData
		{
			C_ImageHandle	image;
			F32				viewvector[3];	//view vector
			F32				anchor[3];		//camera focal point during render
			F32				offset[4];		//vertex offset in world space	[min x, min y, max x, max y]
			F32				texcoord[4];	//texture coords [min x, min y, max x, max y]			

			ImageData()
			{
				memset(viewvector, 0, sizeof(F32)*3);
				memset(anchor, 0, sizeof(F32)*3);
				memset(offset, 0, sizeof(F32)*4);
				texcoord[0] = 0.f; texcoord[1] = 0.f; texcoord[1] = 1.f; texcoord[0] = 1.f;
			}
		};

		Aftk::C_SceneNode				*pSceneNode;
		Btl::Vector<ImageData>			imageData;
		Btl::Vector<E_ImposterType>		types;

		ImposterData()
		: pSceneNode(0)
		{ }
	};

	typedef Btl::Vector<ImposterData> ImposterDataArray;

}

#endif