#ifndef RENDER_DYNAMICEFFECT_ENUMS_H
#define RENDER_DYNAMICEFFECT_ENUMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

enum E_BlendMode
{
	E_BlendMode_Solid,
	E_BlendMode_SolidAlphaTest,
	E_BlendMode_SolidAlphaMask,
	E_BlendMode_Alpha,
	E_BlendMode_AlphaA8,
	E_BlendMode_AlphaAndAdditive,
	E_BlendMode_Additive,
	E_BlendMode_AdditiveA8,
	E_BlendMode_AlphaAdditive,
	E_BlendMode_AlphaSubtractive,
	E_BlendMode_Subtractive,
	E_BlendMode_SubtractiveA8,
};

enum E_Atlas
{
	E_Atlas_Default1x1 = 0,
	E_Atlas_Square2x2,
	E_Atlas_Square4x4,
	E_Atlas_Square8x8,
	E_Atlas_Strip2x1,
	E_Atlas_Strip4x1,
	E_Atlas_Strip4x2,
	E_Atlas_Strip8x2,
	E_Atlas_Strip8x4,

	E_Atlas_Count
};

enum E_AtlasType
{
	E_AtlasType_Random = 0,
	E_AtlasType_Animated
};

}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_ENUMS_H
