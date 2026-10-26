#ifndef RENDER_MESHTYPES_H
#define RENDER_MESHTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Mesh
{

enum E_FileVersion
{
	E_FileVersion_Initial = 0,
	E_FileVersion_Current = E_FileVersion_Initial,
};

enum E_PrimitiveType
{
	E_PrimitiveType_Unknown = 0,
	E_PrimitiveType_TriangleList,
	E_PrimitiveType_TriangleStrip,
};

enum E_VertexElementType
{
	E_VertexElementType_Unknown = 0,
	E_VertexElementType_F32_1,
	E_VertexElementType_F32_2,
	E_VertexElementType_F32_3,
	E_VertexElementType_F32_4,
	E_VertexElementType_F16_2,
	E_VertexElementType_U8_4N,
	E_VertexElementType_U10_3N,
	E_VertexElementType_U10_4N,
};

enum E_VertexElementUsage
{
	E_VertexElementUsage_Unknown = 0,
	E_VertexElementUsage_Position,
	E_VertexElementUsage_Normal,
	E_VertexElementUsage_TexCoord,
	E_VertexElementUsage_Tangent,
	E_VertexElementUsage_Binormal,
	E_VertexElementUsage_Colour,
};

}; // namespace Mesh
}; // namespace Render

#endif // RENDER_MESHTYPES_H
