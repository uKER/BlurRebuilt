#ifndef RENDER_DYNAMICEFFECT_TRACER_ENUMS_H
#define RENDER_DYNAMICEFFECT_TRACER_ENUMS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{
namespace Tracer
{

enum E_Shape
{
	E_Shape_BeamCapped = 0,
	E_Shape_Count
};

enum E_PixelShader
{
	E_PixelShader_Default = 0,
	E_PixelShader_Temperature,
	E_PixelShader_Count
};

}; // namespace Tracer
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACER_ENUMS_H
