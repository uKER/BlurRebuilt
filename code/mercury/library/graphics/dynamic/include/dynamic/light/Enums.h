#ifndef RENDER_DYNAMICEFFECT_LIGHT_ENUMS_H
#define RENDER_DYNAMICEFFECT_LIGHT_ENUMS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{
namespace Light
{

enum E_Shape
{
	E_Shape_Beam2d = 0,
	E_Shape_Cross2d,
	E_Shape_Quad2d,
	E_Shape_Beam3d,
	E_Shape_BeamCapped3d,
	E_Shape_Decal3d,
	E_Shape_Quad3d,
	E_Shape_Count
};

enum E_Occlusion
{
	E_Occlusion_None = 0,
	E_Occlusion_Query,
	E_Occlusion_Custom,
	E_Occlusion_Count
};

enum E_VertexShader
{
	E_VertexShader_Default2d = 0,
	E_VertexShader_Default3d,
	E_VertexShader_Count
};

enum E_PixelShader
{
	E_PixelShader_Default = 0,
	E_PixelShader_Count
};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_ENUMS_H
