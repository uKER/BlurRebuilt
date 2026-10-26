#ifndef DISPLAY_SPU_DEVICE_TYPES_H
#define DISPLAY_SPU_DEVICE_TYPES_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



enum E_CullMode
{
	E_CullMode_None = 0,
	E_CullMode_Cw,
	E_CullMode_Ccw,
	E_CullMode_Count
};

enum E_Format
{
	E_Format_Index16 = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_16,
	E_Format_Index32 = CELL_GCM_DRAW_INDEX_ARRAY_TYPE_32,
};

enum E_PrimitiveType
{
	E_PrimitiveType_PointList = CELL_GCM_PRIMITIVE_POINTS,
	E_PrimitiveType_LineList = CELL_GCM_PRIMITIVE_LINES,
	E_PrimitiveType_LineStrip = CELL_GCM_PRIMITIVE_LINE_STRIP,
	E_PrimitiveType_TriangleList = CELL_GCM_PRIMITIVE_TRIANGLES,
	E_PrimitiveType_TriangleStrip = CELL_GCM_PRIMITIVE_TRIANGLE_STRIP,
	E_PrimitiveType_TriangleFan = CELL_GCM_PRIMITIVE_TRIANGLE_FAN,
	E_PrimitiveType_QuadList = CELL_GCM_PRIMITIVE_QUADS,
};

enum E_VertexElementType
{
	E_VertexElementType_F32 = CELL_GCM_VERTEX_F,
	E_VertexElementType_F16 = CELL_GCM_VERTEX_SF,
	E_VertexElementType_S16 = CELL_GCM_VERTEX_S32K,
	E_VertexElementType_S16_N = CELL_GCM_VERTEX_S1,
	E_VertexElementType_U8 = CELL_GCM_VERTEX_UB256,
	E_VertexElementType_U8_N = CELL_GCM_VERTEX_UB,
	E_VertexElementType_Hend3N = CELL_GCM_VERTEX_CMP,
};



static const U32 POSITION_INDEX		= 0;
static const U32 NORMAL_INDEX		= 2;
static const U32 COLOUR_INDEX		= 3;
static const U32 TEXCOORD_INDEX		= 8;
static const U32 TEXCOORD0_INDEX	= 8;
static const U32 TEXCOORD1_INDEX	= 9;
static const U32 TEXCOORD2_INDEX	= 10;
static const U32 TEXCOORD3_INDEX	= 11;
static const U32 TEXCOORD4_INDEX	= 12;
static const U32 TEXCOORD5_INDEX	= 13;
static const U32 TEXCOORD6_INDEX	= 14;
static const U32 TEXCOORD7_INDEX	= 15;
static const U32 TANGENT_INDEX		= 14;



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_DEVICE_TYPES_H
