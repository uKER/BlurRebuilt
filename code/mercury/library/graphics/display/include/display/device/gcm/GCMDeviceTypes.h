#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_TYPES_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Core/Includes.h"
#include "Display/Device/DeviceTypes.h"

namespace Display
{
namespace GCM
{

#define GCM_ALIGN_SIZE(size, align) (( (size)+(align)-1)/(align)*(align))

INLINE U32 GCMBlend(const E_Blend blend)
{
	switch(blend)
	{
	case E_Blend_Zero:
		return CELL_GCM_ZERO;
	case E_Blend_One:
		return CELL_GCM_ONE;
	case E_Blend_SrcColour:
		return CELL_GCM_SRC_COLOR;
	case E_Blend_InvSrcColour:
		return CELL_GCM_ONE_MINUS_SRC_COLOR;
	case E_Blend_SrcAlpha:
		return CELL_GCM_SRC_ALPHA;
	case E_Blend_InvSrcAlpha:
		return CELL_GCM_ONE_MINUS_SRC_ALPHA;
	case E_Blend_DstAlpha:
		return CELL_GCM_DST_ALPHA;
	case E_Blend_InvDstAlpha:
		return CELL_GCM_ONE_MINUS_DST_ALPHA;
	case E_Blend_DstColour:
		return CELL_GCM_DST_COLOR;
	case E_Blend_InvDstColour:
		return CELL_GCM_ONE_MINUS_DST_COLOR;
	default:
		//BIZ_ASSERT_FAIL("Unsupported blend");
		return CELL_GCM_ZERO;
	}
}

INLINE U32 GCMBlendOp(const E_BlendOp blendOp)
{
	switch(blendOp)
	{
	case E_BlendOp_Add:
		return CELL_GCM_FUNC_ADD;
	case E_BlendOp_Subtract:
		return CELL_GCM_FUNC_SUBTRACT;
	case E_BlendOp_RevSubtract:
		return CELL_GCM_FUNC_REVERSE_SUBTRACT;
	case E_BlendOp_Min:
		return CELL_GCM_MIN;
	case E_BlendOp_Max:
		return CELL_GCM_MAX;
	default:
		BIZ_ASSERT_FAIL("Unsupported blend op");
		return -1;
	}
}

INLINE U32 GCMCmpFunc(const E_CmpFunc cmpFunc)
{
	switch(cmpFunc)
	{
	case E_CmpFunc_Never:
		return CELL_GCM_NEVER;
	case E_CmpFunc_Less:
		return CELL_GCM_LESS;
	case E_CmpFunc_Equal:
		return CELL_GCM_EQUAL;
	case E_CmpFunc_LessEqual:
		return CELL_GCM_LEQUAL;
	case E_CmpFunc_Greater:
		return CELL_GCM_GREATER;
	case E_CmpFunc_NotEqual:
		return CELL_GCM_NOTEQUAL;
	case E_CmpFunc_GreaterEqual:
		return CELL_GCM_GEQUAL;
	case E_CmpFunc_Always:
		return CELL_GCM_ALWAYS;
	default:
		BIZ_ASSERT_FAIL("Unsupported cmp func");
		return -1;
	}
}

INLINE U32 GCMCullMode(const E_CullMode cullMode)
{
	switch(cullMode)
	{
	case E_CullMode_Cw:
		return CELL_GCM_CW;
	case E_CullMode_Ccw:
		return CELL_GCM_CCW;
	default:
		BIZ_ASSERT_FAIL("Unsupported cull mode");
		return -1;
	}
}

INLINE U32 GCMFillMode(const E_FillMode fillMode)
{
	switch(fillMode)
	{
	case E_FillMode_Point:
		return CELL_GCM_POLYGON_MODE_POINT;
	case E_FillMode_Wireframe:
		return CELL_GCM_POLYGON_MODE_LINE;
	case E_FillMode_Solid:
		return CELL_GCM_POLYGON_MODE_FILL;
	default:
		BIZ_ASSERT_FAIL("Unsupported fill mode");
		return -1;
	}
}

INLINE U32 GCMFormat(const E_Format format)
{
	return format;
}

INLINE U32 GCMSurfaceFormat(const U32 format)
{
	switch(format)
	{
	case CELL_GCM_SURFACE_A8R8G8B8:			return CELL_GCM_SURFACE_A8R8G8B8;
	case CELL_GCM_SURFACE_Z16:				return CELL_GCM_SURFACE_Z16;
	case CELL_GCM_SURFACE_Z24S8:			return CELL_GCM_SURFACE_Z24S8;
	case CELL_GCM_SURFACE_F_X32:			return CELL_GCM_SURFACE_F_X32;
	case CELL_GCM_SURFACE_F_W16Z16Y16X16:	return CELL_GCM_SURFACE_F_W16Z16Y16X16;
	case CELL_GCM_SURFACE_F_W32Z32Y32X32:	return CELL_GCM_SURFACE_F_W32Z32Y32X32;
	case CELL_GCM_SURFACE_B8:				return CELL_GCM_SURFACE_B8;
	
	case CELL_GCM_TEXTURE_A8R8G8B8:			return CELL_GCM_SURFACE_A8R8G8B8;
	case CELL_GCM_TEXTURE_DEPTH24_D8:		return CELL_GCM_SURFACE_A8R8G8B8;
	
	default:
		BIZ_ASSERT_FAIL("Unsupported surface format %i", format);
		return -1;
	}
}

INLINE U32 GCMLodBias(const S8 lodBias)
{
	return (S16)((((F32)lodBias/128.0f)*8.0f)*(F32)(1<<8));
}

INLINE U32 GCMMultisample(const E_Multisample multisample)
{
	switch(multisample)
	{
	case E_Multisample_None:
		return CELL_GCM_SURFACE_CENTER_1;
	case E_Multisample_2:
		return CELL_GCM_SURFACE_DIAGONAL_CENTERED_2;
	case E_Multisample_4:
		return CELL_GCM_SURFACE_SQUARE_ROTATED_4;
	default:
		BIZ_ASSERT_FAIL("Unsupported multisample");
		return -1;
	}
}

INLINE U32 GCMPrimitiveType(const E_PrimitiveType primitiveType)
{
	switch(primitiveType)
	{
	case E_PrimitiveType_PointList:
		return CELL_GCM_PRIMITIVE_POINTS;
	case E_PrimitiveType_LineList:
		return CELL_GCM_PRIMITIVE_LINES;
	case E_PrimitiveType_LineStrip:
		return CELL_GCM_PRIMITIVE_LINE_STRIP;
	case E_PrimitiveType_TriangleList:
		return CELL_GCM_PRIMITIVE_TRIANGLES;
	case E_PrimitiveType_TriangleStrip:
		return CELL_GCM_PRIMITIVE_TRIANGLE_STRIP;
	case E_PrimitiveType_TriangleFan:
		return CELL_GCM_PRIMITIVE_TRIANGLE_FAN;
	case E_PrimitiveType_QuadList:
		return CELL_GCM_PRIMITIVE_QUADS;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		return -1;
	}
}

INLINE U32 GCMStencilOp(const E_StencilOp stencilOp)
{
	switch(stencilOp)
	{
	case E_StencilOp_Keep:
		return CELL_GCM_KEEP;
	case E_StencilOp_Zero:
		return CELL_GCM_ZERO;
	case E_StencilOp_Replace:
		return CELL_GCM_REPLACE;
	case E_StencilOp_IncrSat:
		return CELL_GCM_INCR_WRAP;
	case E_StencilOp_DecrSat:
		return CELL_GCM_DECR_WRAP;
	case E_StencilOp_Invert:
		return CELL_GCM_INVERT;
	case E_StencilOp_Incr:
		return CELL_GCM_INCR;
	case E_StencilOp_Decr:
		return CELL_GCM_DECR;
	default:
		BIZ_ASSERT_FAIL("Unsupported primitive type");
		return -1;
	}
}

INLINE U32 GCMTextureAddress(const E_TextureAddress textureAddress)
{
	switch(textureAddress)
	{
	case E_TextureAddress_Wrap:
		return CELL_GCM_TEXTURE_WRAP;
	case E_TextureAddress_Clamp:
		return CELL_GCM_TEXTURE_CLAMP_TO_EDGE;
	case E_TextureAddress_Mirror:
		return CELL_GCM_TEXTURE_MIRROR;
	case E_TextureAddress_Border:
		return CELL_GCM_TEXTURE_BORDER;
	default:
		//BIZ_ASSERT_FAIL("Unsupported texture address");
		return CELL_GCM_TEXTURE_CLAMP_TO_EDGE;
	}
}

INLINE U32 GCMTextureFormat(const E_Format format)
{
	switch(format)
	{
	case E_Format_A8R8G8B8:
		return CELL_GCM_TEXTURE_A8R8G8B8;
	case E_Format_D16:
		return CELL_GCM_TEXTURE_DEPTH16;
	case E_Format_D24S8:
		return CELL_GCM_TEXTURE_DEPTH24_D8;
	case E_Format_R32F:
		return CELL_GCM_TEXTURE_X32_FLOAT;
	case E_Format_A16B16G16R16F:
		return CELL_GCM_TEXTURE_W16_Z16_Y16_X16_FLOAT;
	case E_Format_A32B32G32R32F:
		return CELL_GCM_TEXTURE_W32_Z32_Y32_X32_FLOAT;
	case E_Format_A8:
		return CELL_GCM_TEXTURE_B8;
	default:
		BIZ_ASSERT_FAIL("Unsupported format");
		return -1;
	}
}

INLINE U32 GCMGamma(const E_Bool gamma)
{
	switch(gamma)
	{
	case E_Bool_Unknown:
		return 0;
	case E_Bool_False:
		return 0;
	case E_Bool_True:
		return CELL_GCM_TEXTURE_GAMMA_R | CELL_GCM_TEXTURE_GAMMA_G | CELL_GCM_TEXTURE_GAMMA_B;
	default:
		return 0;
	}
}

INLINE U32 GCMTextureFilter(const E_TextureFilter textureFilter, const E_TextureFilter filterMip = E_TextureFilter_None)
{
	switch(textureFilter)
	{
	case E_TextureFilter_Point:
		return filterMip !=  E_TextureFilter_None ? CELL_GCM_TEXTURE_NEAREST_NEAREST : CELL_GCM_TEXTURE_NEAREST;
	case E_TextureFilter_Linear:
	case E_TextureFilter_Anisotropic:
		return filterMip !=  E_TextureFilter_None ? CELL_GCM_TEXTURE_LINEAR_LINEAR : CELL_GCM_TEXTURE_LINEAR;
	default:
		return CELL_GCM_TEXTURE_NEAREST;
	}
}

INLINE U32 GCMTextureFilterMin(const E_TextureFilter textureFilter, const E_TextureFilter filterMip = E_TextureFilter_None)
{
	switch(textureFilter)
	{
	case E_TextureFilter_Point:
		return filterMip !=  E_TextureFilter_None ? CELL_GCM_TEXTURE_NEAREST_NEAREST : CELL_GCM_TEXTURE_NEAREST;
	case E_TextureFilter_Linear:
	case E_TextureFilter_Anisotropic:
		return filterMip !=  E_TextureFilter_None ? CELL_GCM_TEXTURE_LINEAR_LINEAR : CELL_GCM_TEXTURE_LINEAR;
	default:
		return CELL_GCM_TEXTURE_NEAREST;
	}
}

INLINE U32 GCMTextureFilterMag(const E_TextureFilter textureFilter)
{
	switch(textureFilter)
	{
	case E_TextureFilter_Point:
		return CELL_GCM_TEXTURE_NEAREST;
	case E_TextureFilter_Linear:
	case E_TextureFilter_Anisotropic:
		return CELL_GCM_TEXTURE_LINEAR;
	default:
		return CELL_GCM_TEXTURE_NEAREST;
	}
}

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_TYPES_H
