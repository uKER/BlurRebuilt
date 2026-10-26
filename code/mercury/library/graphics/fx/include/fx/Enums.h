#ifndef __FX_ENUMS_H__
#define __FX_ENUMS_H__

namespace FX
{

enum E_Bool
{
	E_Bool_False,
	E_Bool_True,
	E_Bool_Unknown,
};

enum E_Blend
{
	E_Blend_Unknown = 0,
	E_Blend_Zero,
	E_Blend_One,
	E_Blend_SrcColour,
	E_Blend_InvSrcColour,
	E_Blend_SrcAlpha,
	E_Blend_InvSrcAlpha,
	E_Blend_DstAlpha,
	E_Blend_InvDstAlpha,
	E_Blend_DstColour,
	E_Blend_InvDstColour,
};

enum E_BlendOp
{
	E_BlendOp_Unknown = 0,
	E_BlendOp_Add,
	E_BlendOp_Subtract,
	E_BlendOp_RevSubtract,
	E_BlendOp_Min,
	E_BlendOp_Max,
};

enum E_CmpFunc
{
	E_CmpFunc_Unknown = 0,
	E_CmpFunc_Default,
	E_CmpFunc_Never,
	E_CmpFunc_Less,
	E_CmpFunc_Equal,
	E_CmpFunc_LessEqual,
	E_CmpFunc_Greater,
	E_CmpFunc_NotEqual,
	E_CmpFunc_GreaterEqual,
	E_CmpFunc_Always,
};

enum E_CullMode
{
	E_CullMode_Unknown = 0,
	E_CullMode_Default,
	E_CullMode_None,
	E_CullMode_Cw,
	E_CullMode_Ccw,
	E_CullMode_Reversed,
	E_CullMode_Front,
	E_CullMode_Back,
};

enum E_FillMode
{
	E_FillMode_Unknown = 0,
	E_FillMode_Default,
	E_FillMode_Point,
	E_FillMode_Wireframe,
	E_FillMode_Solid,
};

enum E_StencilOp
{
	E_StencilOp_Keep = 0,
	E_StencilOp_Zero,
	E_StencilOp_Replace,
	E_StencilOp_IncrSat,
	E_StencilOp_DecrSat,
	E_StencilOp_Invert,
	E_StencilOp_Incr,
	E_StencilOp_Decr,
	E_StencilOp_Count,
};

enum E_TextureAddress
{
	E_TextureAddress_Wrap = 0,
	E_TextureAddress_Clamp,
	E_TextureAddress_Mirror,
	E_TextureAddress_Border,
	E_TextureAddress_Count,
	E_TextureAddress_Default,
	E_TextureAddress_Unknown,
};

enum E_TextureFilter
{
	E_TextureFilter_None,
	E_TextureFilter_Point,
	E_TextureFilter_Linear,
	E_TextureFilter_Anisotropic,
	E_TextureFilter_Count,
	E_TextureFilter_Default,
	E_TextureFilter_Unknown,
};

} // namespace FX

#endif
