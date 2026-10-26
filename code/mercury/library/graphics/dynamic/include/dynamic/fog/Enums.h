#ifndef RENDER_DYNAMICEFFECT_FOG_ENUMS_H
#define RENDER_DYNAMICEFFECT_FOG_ENUMS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{
namespace Fog
{

enum E_Draw
{
	E_Draw_Front = 0,
	E_Draw_Back,
	E_Draw_Count
};

enum E_Detail
{
	E_Detail_PerVertex = 0,
	E_Detail_PerPixel,
	E_Detail_Count
};

enum E_Shape
{
	E_Shape_Cuboid = 0,
	E_Shape_Cylinder,
	E_Shape_Ellipsoid,
	E_Shape_Count
};

enum E_Shader
{
	E_Shader_DefaultFrontFaces = 0,
	E_Shader_DefaultBackFaces,
	E_Shader_CuboidFrontFaces,
	E_Shader_CuboidBackFaces,
	E_Shader_CylinderFrontFaces,
	E_Shader_CylinderBackFaces,
	E_Shader_EllipsoidFrontFaces,
	E_Shader_EllipsoidBackFaces,
	E_Shader_Count
};

}; // namespace Fog
}; // namespace DynamicEffect

#endif
