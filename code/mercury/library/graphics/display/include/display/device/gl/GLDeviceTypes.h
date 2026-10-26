#ifndef DISPLAY_GLDEVICETYPES_H
#define DISPLAY_GLDEVICETYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceTypes.h"

namespace NewDisplay
{
namespace GL
{

INLINE GLenum GLBlend(const E_Blend blend)
{
	switch (blend)
	{
	case E_Blend_Zero:
		return GL_ZERO;
	case E_Blend_One:
		return GL_ONE;
	case E_Blend_SrcColoor:
		return GL_SRC_COLOR;
	case E_Blend_InvSrcColour:
		return GL_ONE_MINUS_SRC_COLOR;
	case E_Blend_SrcAlpha:
		return GL_SRC_ALPHA;
	case E_Blend_InvSrcAlpha:
		return GL_ONE_MINUS_SRC_ALPHA;
	case E_Blend_DstAlpha:
		return GL_DST_ALPHA;
	case E_Blend_InvDstAlpha:
		return GL_ONE_MINUS_DST_ALPHA;
	case E_Blend_DstColour:
		return GL_DST_COLOR;
	case E_Blend_InvDstColour:
		return GL_ONE_MINUS_DST_COLOR;
	default:
		//BIZ_ASSERT_FAIL("Unsupported blend"); [LloydW] Dont assert cos of glBlendFunc.
		return GL_ZERO;
	}
}

INLINE GLenum GLBlendOp(const E_BlendOp blendOp)
{
	switch (blendOp)
	{
	case E_BlendOp_Add:
		return GL_FUNC_ADD;
	case E_BlendOp_Subtract:
		return GL_FUNC_SUBTRACT;
	case E_BlendOp_RevSubtract:
		return GL_FUNC_REVERSE_SUBTRACT;
	case E_BlendOp_Min:
		return GL_MIN;
	case E_BlendOp_Max:
		return GL_MAX;
	default:
		BIZ_ASSERT_FAIL("Unsupported blend op");
		return GL_FUNC_ADD;
	}
}

INLINE GLenum GLCmpFunc(const E_CmpFunc cmpFunc)
{
	switch (cmpFunc)
	{
	case E_CmpFunc_Never:
		return GL_NEVER;
	case E_CmpFunc_Less:
		return GL_LESS;
	case E_CmpFunc_Equal:
		return GL_EQUAL;
	case E_CmpFunc_LessEqual:
		return GL_LEQUAL;
	case E_CmpFunc_Greater:
		return GL_GREATER;
	case E_CmpFunc_NotEqual:
		return GL_NOTEQUAL;
	case E_CmpFunc_GreaterEqual:
		return GL_GEQUAL;
	case E_CmpFunc_Always:
		return GL_ALWAYS;
	default:
		BIZ_ASSERT_FAIL("Unsupported cmp func");
		return GL_NEVER;
	}
}

INLINE GLenum GLFillMode(const E_FillMode fillMode)
{
	switch (fillMode)
	{
	case E_FillMode_Point:
		return GL_POINT;
	case E_FillMode_Wireframe:
		return GL_LINE;
	case E_FillMode_Solid:
		return GL_FILL;
	default:
		BIZ_ASSERT_FAIL("Unsupported fill mode");
		return GL_FILL;
	}
}

INLINE GLint GLFormat(const E_Format format)
{
	switch (format)
	{
	case E_Format_DXT1:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case E_Format_DXT3:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case E_Format_DXT5:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case E_Format_A8R8G8B8:
		return GL_RGBA;
	default:
		BIZ_ASSERT_FAIL("Unsupported texture format");
		return GL_RGBA;
	}
}

INLINE GLenum GLPrimitiveType(const E_PrimitiveType primitiveType)
{
	switch (primitiveType)
	{
	case E_PrimitiveType_PointList:
		return GL_POINTS;
	case E_PrimitiveType_LineList:
		return GL_LINES;
	case E_PrimitiveType_LineStrip:
		return GL_LINE_STRIP;
	case E_PrimitiveType_TriangleList:
		return GL_TRIANGLES;
	case E_PrimitiveType_TriangleStrip:
		return GL_TRIANGLE_STRIP;
	case E_PrimitiveType_TriangleFan:
		return GL_TRIANGLE_FAN;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		return GL_TRIANGLE_STRIP;
	}
}

INLINE GLint GLTextureAddress(const E_TextureAddress textureAddress)
{
	switch (textureAddress)
	{
	case E_TextureAddress_Wrap:
		return GL_REPEAT;
	case E_TextureAddress_Clamp:
		return GL_CLAMP;
	default:
		BIZ_ASSERT_FAIL("Unsupported texture address");
		return GL_REPEAT;
	}
}

INLINE GLint GLTextureFilter(const E_TextureFilter textureFilter)
{
	switch (textureFilter)
	{
	case E_TextureFilter_None:
		return GL_NONE;
	case E_TextureFilter_Point:
		return GL_POINT;
	case E_TextureFilter_Linear:
		return GL_LINEAR;
	default:
		BIZ_ASSERT_FAIL("Unsupported texture filter");
		return GL_NONE;
	}
}

INLINE GLenum GLUsage(const U32 flags)
{
	if (flags & E_Usage_Dynamic)
		return GL_DYNAMIC_DRAW;
	else
		return GL_STATIC_DRAW;
}

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLDISPLAY_GLDEVICETYPES_HTYPES_H
