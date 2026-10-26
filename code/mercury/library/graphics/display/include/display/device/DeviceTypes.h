#ifndef DISPLAY_DEVICE_DEVICE_TYPES_H
#define DISPLAY_DEVICE_DEVICE_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2.h>

#include "Display/Device/DeviceDefines.h"
#include "Display/Device/DeviceTypesEnums.h"

namespace Display
{

enum E_ColourWriteEnable
{
	E_ColourWriteEnable_None	= 0,

#ifdef PLATFORM_PS3
	E_ColourWriteEnable_Red		= CELL_GCM_COLOR_MASK_R,
	E_ColourWriteEnable_Green	= CELL_GCM_COLOR_MASK_G,
	E_ColourWriteEnable_Blue	= CELL_GCM_COLOR_MASK_B,
	E_ColourWriteEnable_Alpha	= CELL_GCM_COLOR_MASK_A,
	E_ColourWriteEnable_All		= CELL_GCM_COLOR_MASK_R|CELL_GCM_COLOR_MASK_G|CELL_GCM_COLOR_MASK_B|CELL_GCM_COLOR_MASK_A,
#else
	E_ColourWriteEnable_Red		= 1L<<0,
	E_ColourWriteEnable_Green	= 1L<<1,
	E_ColourWriteEnable_Blue	= 1L<<2,
	E_ColourWriteEnable_Alpha	= 1L<<3,
	E_ColourWriteEnable_All		= 0xf,
#endif

	E_ColourWriteEnable_Default = UINT_MAX,
};

inline const char* GetName(Display::E_VertexElementType elementType)
{
	switch( elementType )
	{
		case E_VertexElementType_Unknown:	return "Unknown";
		case E_VertexElementType_F32_1:		return "F32_1";
		case E_VertexElementType_F32_2:		return "F32_2";
		case E_VertexElementType_F32_3:		return "F32_3";		
		case E_VertexElementType_F32_4:		return "F32_4";
		case E_VertexElementType_Colour:	return "Colour";
		case E_VertexElementType_U8_4:		return "U8_4";
		case E_VertexElementType_U8_4N:		return "U8_4N";
		case E_VertexElementType_S16_2:		return "S16_2";
		case E_VertexElementType_S16_4:		return "S16_4";
		case E_VertexElementType_F16_2:		return "F16_2";
		case E_VertexElementType_F16_4:		return "F16_4";
		case E_VertexElementType_S16_2N:	return "S16_2N";
		case E_VertexElementType_U16_2:		return "U16_2";
		case E_VertexElementType_U16_4:		return "U16_4";
		case E_VertexElementType_U10_3N:	return "U10_3N";
		case E_VertexElementType_U10_4N:	return "U10_4N";
		case E_VertexElementType_U32_1:		return "U32_1";
		case E_VertexElementType_SHHD_3:	return "SHHD_3";
		case E_VertexElementType_SHHD_3N:	return "SHHD_3N";
	}

	return "Unknown";
}

inline const char* GetName(Display::E_VertexElementUsage elementUsage)
{
	switch( elementUsage )
	{
		case E_VertexElementUsage_Unknown:			return "Unknown";
		case E_VertexElementUsage_Position:			return "Position";
		case E_VertexElementUsage_BlendWeight:		return "BlendWeight";
		case E_VertexElementUsage_BlendIndices:		return "BlendIndices";
		case E_VertexElementUsage_Normal:			return "Normal";
		case E_VertexElementUsage_PSize:			return "PSize";
		case E_VertexElementUsage_TexCoord:			return "TexCoord";
		case E_VertexElementUsage_Tangent:			return "Tangent";
		case E_VertexElementUsage_Binormal:			return "Binormal";
		case E_VertexElementUsage_Colour:			return "Colour";
		case E_VertexElementUsage_Fog:				return "Fog";
		case E_VertexElementUsage_TessFactor:		return "TessFactor";
		case E_VertexElementUsage_Depth:			return "Depth";
		case E_VertexElementUsage_Sample:			return "Sample";
		case E_VertexElementUsage_PositionT:		return "PositionT";
	}
	return "Unknown";
}

INLINE U32 IndexCount(const E_PrimitiveType primitiveType, const U32 primitiveCount)
{
	switch(primitiveType)
	{
	case E_PrimitiveType_PointList:
		return primitiveCount;
	case E_PrimitiveType_LineList:
		return primitiveCount * 2;
	case E_PrimitiveType_LineStrip:
		return primitiveCount + 1;
	case E_PrimitiveType_TriangleList:
		return primitiveCount * 3;
	case E_PrimitiveType_QuadList:
		return IndexCount(E_PrimitiveType_PointList, primitiveCount);
	case E_PrimitiveType_TriangleStrip:
	case E_PrimitiveType_TriangleFan:
		return primitiveCount + 2;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		return 0;
	}
}

INLINE U32 PrimitiveCount(const E_PrimitiveType primitiveType, const U32 indexCount)
{
	switch(primitiveType)
	{
	case E_PrimitiveType_PointList:
		return indexCount;
	case E_PrimitiveType_LineList:
		return indexCount / 2;
	case E_PrimitiveType_LineStrip:
		return indexCount - 1;
	case E_PrimitiveType_TriangleList:
		return indexCount / 3;
	case E_PrimitiveType_QuadList:
		return PrimitiveCount(E_PrimitiveType_PointList, indexCount);
	case E_PrimitiveType_TriangleStrip:
	case E_PrimitiveType_TriangleFan:
		return indexCount - 2;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		return 0;
	}
}

INLINE void ValidateIndexCount(const E_PrimitiveType primitiveType, const U32 indexCount)
{
	switch(primitiveType)
	{
	case E_PrimitiveType_PointList:
		BIZ_ASSERT_MSG(indexCount >= 1, "Trying to render zero indices!");
		break;
	case E_PrimitiveType_LineList:
	case E_PrimitiveType_LineStrip:
		BIZ_ASSERT_MSG(indexCount >= 2, "Trying to render fewer than 2 indices!");
		break;
	case E_PrimitiveType_TriangleList:
	case E_PrimitiveType_TriangleStrip:
	case E_PrimitiveType_TriangleFan:
		BIZ_ASSERT_MSG(indexCount >= 3, "Trying to render fewer than 3 indices!");
		break;
	case E_PrimitiveType_QuadList:
		ValidateIndexCount(E_PrimitiveType_PointList, indexCount);
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		break;
	}
}

INLINE void ValidatePrimitiveCount(const E_PrimitiveType primitiveType, const U32 primitiveCount)
{
	switch(primitiveType)
	{
	case E_PrimitiveType_PointList:
	case E_PrimitiveType_LineList:
	case E_PrimitiveType_LineStrip:
	case E_PrimitiveType_TriangleList:
	case E_PrimitiveType_TriangleStrip:
	case E_PrimitiveType_TriangleFan:
	case E_PrimitiveType_QuadList:
		BIZ_ASSERT_MSG(primitiveCount >= 1, "Trying to render zero primitives!");
		break;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		break;
	}
}

INLINE U32 SizeOf(const E_Format format)
{
	switch(format)
	{
	case E_Format_Index16:
		return sizeof(U16);
	case E_Format_Index32:
		return sizeof(U32);
	default:
		BIZ_ASSERT_FAIL("Unsupported format");
		return sizeof(U16);
	}
}

INLINE const E_CmpFunc FlippedCmpFunc(const E_CmpFunc cmpFunc)
{
	switch(cmpFunc)
	{
	case E_CmpFunc_Greater:
		return E_CmpFunc_Less;
	case E_CmpFunc_GreaterEqual:
		return E_CmpFunc_LessEqual;
	case E_CmpFunc_Less:
		return E_CmpFunc_Greater;
	case E_CmpFunc_LessEqual:
		return E_CmpFunc_GreaterEqual;
	default:
		return cmpFunc;
	};
	return cmpFunc;
}

INLINE const E_CullMode FlippedCullMode(const E_CullMode cullMode)
{
	if (cullMode == E_CullMode_Cw)
		return E_CullMode_Ccw;
	else if (cullMode == E_CullMode_Ccw)
		return E_CullMode_Cw;
	else if (cullMode == E_CullMode_Front)
		return E_CullMode_Back;
	else if (cullMode == E_CullMode_Back)
		return E_CullMode_Front;
	else
		return cullMode;
}

INLINE const U32 GPU_RGBA(const U8 r, const U8 g, const U8 b, const U8 a)
{
	return ((r & 0xff) << 24) | ((g & 0xff) << 16) | ((b & 0xff) << 8) | (a & 0xff);
}

INLINE const U32 GPU_RGBA(const Maths::C_RGBA_U8 & colour)
{
	U32 raw = colour;
	return (raw << 8) | (raw >> 24);
}

extern const Maths::RGBA BLACK;
extern const Maths::RGBA WHITE;
extern const Maths::RGBA GREY;
extern const Maths::RGBA RED;
extern const Maths::RGBA GREEN;
extern const Maths::RGBA BLUE;
extern const Maths::RGBA YELLOW;
extern const Maths::RGBA PINK;

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_TYPES_H
