#ifndef RENDER_DYNAMICEFFECT_XTINTERFACE_H
#define RENDER_DYNAMICEFFECT_XTINTERFACE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "xt\xtgame.h"

namespace Curve
{
	class C_HermiteCurve;
};



namespace DynamicEffect
{
class C_Renderer;
struct LookupTable;

namespace XtInterface
{
	struct Light
	{
		bool	receive;
		bool	emit;
		F32		radius;
		F32		intensity;
		U32		priority;
	};

	struct HermiteKeyFrame
	{
		F32 value;
		F32 tangentIn;
		F32 tangentOut;
		F32 time;
	};

	struct U32Dim2
	{
		U32		width;
		U32		height;
	};

	struct U32Dim3
	{
		U32		width;
		U32		height;
		U32		depth;
	};


	struct ColourTime
	{
		XtRgb	colour;
		F32		time;
	};

	void		PackOpen(XtDb* pDatabase, const Util::C_String& basePath, Util::C_String packName = Util::C_String());
	void		PackClose();
	XtPack*		GetPack();

	void		Initialise(C_Renderer* pRenderer);
	C_Renderer*	Renderer();
	void		CreateCurve(const Curve::C_HermiteCurve& curve, LookupTable& lookupTable, S32 nSteps = 2, S32 nElement = -1);
	bool		CreateCurve(const XtArray<HermiteKeyFrame>& keyframes, LookupTable& lookupTable, S32 nSteps = 2, S32 nElement = -1);

};

}; // namespace DynamicEffect



#endif
