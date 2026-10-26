#ifndef RENDER_MESH2_TYPES_H
#define RENDER_MESH2_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{

enum E_Mesh2FilePlatform
{
	E_Mesh2FilePlatform_PCD3D9 = 0,
	E_Mesh2FilePlatform_XB2,
	E_Mesh2FilePlatform_PS3GCM,
	E_Mesh2FilePlatform_Count,
};

enum E_Mesh2FileVersion
{
	E_Mesh2FileVersion_Unknown = 0,
	E_Mesh2FileVersion_1,
	E_Mesh2FileVersion_Latest = E_Mesh2FileVersion_1,
};

enum E_Mesh2IndexFormat
{
	E_Mesh2IndexFormat_Unknown = 0,
	E_Mesh2IndexFormat_16,
	E_Mesh2IndexFormat_32,
};

enum E_Mesh2PrimitiveType
{
	E_Mesh2PrimitiveType_Unknown = 0,
	E_Mesh2PrimitiveType_TriangleList,
};

enum E_Mesh2VertexElementType
{
	E_Mesh2VertexElementType_Unknown = 0,
	E_Mesh2VertexElementType_F32_1,
    E_Mesh2VertexElementType_F32_2,
    E_Mesh2VertexElementType_F32_3,
    E_Mesh2VertexElementType_F32_4,
    E_Mesh2VertexElementType_Colour,
    E_Mesh2VertexElementType_U8_4,
    E_Mesh2VertexElementType_U16_2,
    E_Mesh2VertexElementType_U16_4,
    E_Mesh2VertexElementType_U8_4N,
    E_Mesh2VertexElementType_F16_2,
    E_Mesh2VertexElementType_F16_4,
};

enum E_Mesh2VertexElementUsage
{
	E_Mesh2VertexElementUsage_Unknown = 0,
	E_Mesh2VertexElementUsage_Position,
	E_Mesh2VertexElementUsage_BlendWeight,
	E_Mesh2VertexElementUsage_BlendIndices,
	E_Mesh2VertexElementUsage_Normal,
	E_Mesh2VertexElementUsage_PSize,
	E_Mesh2VertexElementUsage_TexCoord,
	E_Mesh2VertexElementUsage_Tangent,
	E_Mesh2VertexElementUsage_Binormal,
	E_Mesh2VertexElementUsage_TessFactor,
	E_Mesh2VertexElementUsage_Colour,
	E_Mesh2VertexElementUsage_Fog,
	E_Mesh2VertexElementUsage_Depth,
	E_Mesh2VertexElementUsage_Sample,
};

} // namespace Render

#endif // RENDER_MESH2_TYPES_H
