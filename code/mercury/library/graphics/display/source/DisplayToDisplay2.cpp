#include "stdafx.h"
#include "Display/DisplayToDisplay2.h"

const Display2::E_Blend ToBlend2(const Display::E_Blend srcBlend)
{
	switch(srcBlend)
	{
	case Display::E_Blend_Zero:						return Display2::E_Blend_Zero;
	case Display::E_Blend_One:						return Display2::E_Blend_One;
	case Display::E_Blend_SrcColour:				return Display2::E_Blend_SrcColour;
	case Display::E_Blend_InvSrcColour:				return Display2::E_Blend_InvSrcColour;
	case Display::E_Blend_SrcAlpha:					return Display2::E_Blend_SrcAlpha;
	case Display::E_Blend_InvSrcAlpha:				return Display2::E_Blend_InvSrcAlpha;
	case Display::E_Blend_DstAlpha:					return Display2::E_Blend_DstAlpha;
	case Display::E_Blend_InvDstAlpha:				return Display2::E_Blend_InvDstAlpha;
	case Display::E_Blend_DstColour:				return Display2::E_Blend_DstColour;
	case Display::E_Blend_InvDstColour:				return Display2::E_Blend_InvDstColour;
	default:
		BIZ_ASSERT_FAIL("Unsupported display blend enum %i\n", srcBlend);
		return Display2::E_Blend_Unknown;
	}
}

const Display2::E_BlendOp ToBlendOp2(const Display::E_BlendOp srcBlendOp)
{
	switch(srcBlendOp)
	{
	case Display::E_BlendOp_Add:					return Display2::E_BlendOp_Add;
	case Display::E_BlendOp_Subtract:				return Display2::E_BlendOp_Subtract;
	case Display::E_BlendOp_RevSubtract:			return Display2::E_BlendOp_RevSubtract;
	case Display::E_BlendOp_Min:					return Display2::E_BlendOp_Min;
	case Display::E_BlendOp_Max:					return Display2::E_BlendOp_Max;
	default:
		BIZ_ASSERT_FAIL("Unsupported display blend op enum %i\n", srcBlendOp);
		return Display2::E_BlendOp_Unknown;
	}
}

const Display2::E_Bool ToBool2(const Display::E_Bool srcBool)
{
	switch(srcBool)
	{
	case Display::E_Bool_False:						return Display2::E_Bool_False;
	case Display::E_Bool_True:						return Display2::E_Bool_True;
	default:
		BIZ_ASSERT_FAIL("Unsupported display bool enum %i\n", srcBool);
		return Display2::E_Bool_Unknown;
	}
}

const Display2::E_CmpFunc ToCmpFunc2(const Display::E_CmpFunc srcCmpFunc)
{
	switch(srcCmpFunc)
	{
	case Display::E_CmpFunc_Never:					return Display2::E_CmpFunc_Never;
	case Display::E_CmpFunc_Less:					return Display2::E_CmpFunc_Less;
	case Display::E_CmpFunc_Equal:					return Display2::E_CmpFunc_Equal;
	case Display::E_CmpFunc_LessEqual:				return Display2::E_CmpFunc_LessEqual;
	case Display::E_CmpFunc_Greater:				return Display2::E_CmpFunc_Greater;
	case Display::E_CmpFunc_NotEqual:				return Display2::E_CmpFunc_NotEqual;
	case Display::E_CmpFunc_GreaterEqual:			return Display2::E_CmpFunc_GreaterEqual;
	case Display::E_CmpFunc_Always: 				return Display2::E_CmpFunc_Always;
	default:
		BIZ_ASSERT_FAIL("Unsupported display cmp func enum %i\n", srcCmpFunc);
		return Display2::E_CmpFunc_Unknown;
	}
}

const Display2::E_CullMode ToCullMode2(const Display::E_CullMode srcCullMode)
{
	switch(srcCullMode)
	{
	case Display::E_CullMode_None:					return Display2::E_CullMode_None;
	case Display::E_CullMode_Cw:					return Display2::E_CullMode_Cw;
	case Display::E_CullMode_Ccw:					return Display2::E_CullMode_Ccw;
	default:
		sLOGERROR(Display2::g_Log, "Unsupported display cull mode enum %i\n", srcCullMode);
		BIZ_ASSERT(0);
		return Display2::E_CullMode_Unknown;
	}
}

const Display2::E_FillMode ToFillMode2(const Display::E_FillMode srcFillMode)
{
	switch(srcFillMode)
	{
	case Display::E_FillMode_Point:					return Display2::E_FillMode_Point;
	case Display::E_FillMode_Wireframe:				return Display2::E_FillMode_Wireframe;
	case Display::E_FillMode_Solid:					return Display2::E_FillMode_Solid;
	default:
		BIZ_ASSERT_FAIL("Unsupported display fill mode enum %i\n", srcFillMode);
		return Display2::E_FillMode_Unknown;
	}
}

const Display2::E_PrimitiveType ToPrimitiveType2(const Display::E_PrimitiveType srcPrimitiveType)
{
	switch(srcPrimitiveType)
	{
	case Display::E_PrimitiveType_PointList: 		return Display2::E_PrimitiveType_PointList;
	case Display::E_PrimitiveType_LineList: 		return Display2::E_PrimitiveType_LineList;
	case Display::E_PrimitiveType_LineStrip: 		return Display2::E_PrimitiveType_LineStrip;
	case Display::E_PrimitiveType_TriangleList: 	return Display2::E_PrimitiveType_TriangleList;
	case Display::E_PrimitiveType_TriangleStrip: 	return Display2::E_PrimitiveType_TriangleStrip;
	case Display::E_PrimitiveType_TriangleFan: 		return Display2::E_PrimitiveType_TriangleFan;
#ifndef PLATFORM_PC
	case Display::E_PrimitiveType_QuadList: 		return Display2::E_PrimitiveType_QuadList;
#endif
	default:
		BIZ_ASSERT_FAIL("Unsupported display primitive type enum %i\n", srcPrimitiveType);
		return Display2::E_PrimitiveType_Unknown;
	}
}

const Display2::E_StencilOp ToStencilOp2(const Display::E_StencilOp srcStencilOp)
{
	switch(srcStencilOp)
	{
	case Display::E_StencilOp_Keep: 				return Display2::E_StencilOp_Keep;
	case Display::E_StencilOp_Zero: 				return Display2::E_StencilOp_Zero;
	case Display::E_StencilOp_Replace: 				return Display2::E_StencilOp_Replace;
	case Display::E_StencilOp_IncrSat: 				return Display2::E_StencilOp_IncrSat;
	case Display::E_StencilOp_DecrSat: 				return Display2::E_StencilOp_DecrSat;
	case Display::E_StencilOp_Invert:				return Display2::E_StencilOp_Invert;
	case Display::E_StencilOp_Incr: 				return Display2::E_StencilOp_Incr;
	case Display::E_StencilOp_Decr: 				return Display2::E_StencilOp_Decr;
	default:
		BIZ_ASSERT_FAIL("Unsupported display stencil op enum %i\n", srcStencilOp);
		return Display2::E_StencilOp_Unknown;
	}
}

const Display2::E_TextureAddress ToTextureAddress2(const Display::E_TextureAddress srcTextureAddress)
{
	switch(srcTextureAddress)
	{
	case Display::E_TextureAddress_Wrap: 			return Display2::E_TextureAddress_Wrap;
#if PLATFORM_PPU_SPU
	case Display::E_TextureAddress_Clamp: 			return Display2::E_TextureAddress_ClampToEdge;
#else
	case Display::E_TextureAddress_Clamp: 			return Display2::E_TextureAddress_Clamp;
#endif
	case Display::E_TextureAddress_Mirror: 			return Display2::E_TextureAddress_Mirror;
	case Display::E_TextureAddress_Border: 			return Display2::E_TextureAddress_Border;
	default:
		BIZ_ASSERT_FAIL("Unsupported display texture address enum %i\n", srcTextureAddress);
		return Display2::E_TextureAddress_Unknown;
	}
}

const Display2::E_TextureFilter ToTextureFilter2(const Display::E_TextureFilter srcTextureFilter)
{
	switch(srcTextureFilter)
	{
	case Display::E_TextureFilter_None: 			return Display2::E_TextureFilter_None;
	case Display::E_TextureFilter_Point: 			return Display2::E_TextureFilter_Point;
	case Display::E_TextureFilter_Linear: 			return Display2::E_TextureFilter_Linear;
	case Display::E_TextureFilter_Anisotropic: 		return Display2::E_TextureFilter_Anisotropic;
	default:
		BIZ_ASSERT_FAIL("Unsupported display texture filter enum %i\n", srcTextureFilter);
		return Display2::E_TextureFilter_Unknown;
	}
}

const Display2::E_ResourceType ToResourceType2(const Display::E_ResourceType srcResourceType)
{
	switch(srcResourceType)
	{
	case Display::E_ResourceType_Surface:		return Display2::E_ResourceType_Surface;
	case Display::E_ResourceType_Texture2D:		return Display2::E_ResourceType_Texture2D;
	case Display::E_ResourceType_Texture3D:		return Display2::E_ResourceType_Texture3D;
	case Display::E_ResourceType_TextureCube:	return Display2::E_ResourceType_TextureCube;
	case Display::E_ResourceType_VertexBuffer:	return Display2::E_ResourceType_VertexBuffer;
	case Display::E_ResourceType_IndexBuffer:	return Display2::E_ResourceType_IndexBuffer;
	case Display::E_ResourceType_VertexShader:	return Display2::E_ResourceType_VertexShader;
	case Display::E_ResourceType_PixelShader:	return Display2::E_ResourceType_PixelShader;
	default:
		BIZ_ASSERT_FAIL("Unsupported display resource tyoe enum %i\n", srcResourceType);
		return Display2::E_ResourceType_Unknown;
	}
}

#if 0
bool ToVertexDeclaration2(Display::C_VertexDeclaration* pSrcVertexDeclaration, Display2::C_VertexDeclaration* pDstVertexDeclaration)
{
	assert(pSrcVertexDeclaration && pSrcVertexDeclaration->IsCreated());
	assert(pDstVertexDeclaration && !pDstVertexDeclaration->IsCreated());

	if(pSrcVertexDeclaration->m_ElementCount < pDstVertexDeclaration->m_MaxElementCount)
	{
		pDstVertexDeclaration->m_ElementCount = pSrcVertexDeclaration->m_ElementCount;
	}
	else
	{
		BIZ_ASSERT(0 && "Too many vertex elements");
		return false;
	}
	
	for(S32 iVertexElement=0; iVertexElement<pSrcVertexDeclaration->m_ElementCount; iVertexElement++)
	{
		const Display::C_VertexElement& srcVertexElement = pSrcVertexDeclaration->m_ElementArray[iVertexElement];
		Display2::C_VertexElement& dstVertexElement = pDstVertexDeclaration->m_ElementArray[iVertexElement];

		dstVertexElement.SetStream(srcVertexElement.m_Stream);

		dstVertexElement.SetOffset(srcVertexElement.m_Offset);

		switch(srcVertexElement.m_Type)
		{
		case Display::E_VertexElementType_F32_1:	dstVertexElement.SetType(Display2::E_VertexElementType_F32_1); 		break;
		case Display::E_VertexElementType_F32_2:	dstVertexElement.SetType(Display2::E_VertexElementType_F32_2); 		break;
		case Display::E_VertexElementType_F32_3:	dstVertexElement.SetType(Display2::E_VertexElementType_F32_3); 		break;
		case Display::E_VertexElementType_F32_4:	dstVertexElement.SetType(Display2::E_VertexElementType_F32_4); 		break;
		case Display::E_VertexElementType_Colour:	dstVertexElement.SetType(Display2::E_VertexElementType_Colour);		break;
		case Display::E_VertexElementType_U8_4:		dstVertexElement.SetType(Display2::E_VertexElementType_U8_4);		break;
		case Display::E_VertexElementType_U16_2:	dstVertexElement.SetType(Display2::E_VertexElementType_U16_2); 		break;
		case Display::E_VertexElementType_U16_4: 	dstVertexElement.SetType(Display2::E_VertexElementType_U16_4); 		break;
		case Display::E_VertexElementType_S16_2: 	dstVertexElement.SetType(Display2::E_VertexElementType_S16_2); 		break;
		case Display::E_VertexElementType_S16_4: 	dstVertexElement.SetType(Display2::E_VertexElementType_S16_4); 		break;
		case Display::E_VertexElementType_U8_4N: 	dstVertexElement.SetType(Display2::E_VertexElementType_U8_4N); 		break;
		case Display::E_VertexElementType_F16_2: 	dstVertexElement.SetType(Display2::E_VertexElementType_F16_2); 		break;
		case Display::E_VertexElementType_F16_4: 	dstVertexElement.SetType(Display2::E_VertexElementType_F16_4); 		break;
		case Display::E_VertexElementType_U10_3N: 	dstVertexElement.SetType(Display2::E_VertexElementType_U10_3N); 	break;
		case Display::E_VertexElementType_U10_4N: 	dstVertexElement.SetType(Display2::E_VertexElementType_U10_4N); 	break;
		case Display::E_VertexElementType_S16_2N: 	dstVertexElement.SetType(Display2::E_VertexElementType_S16_2N); 	break;
		case Display::E_VertexElementType_SHHD_3: 	dstVertexElement.SetType(Display2::E_VertexElementType_SHHD_3); 	break;
		case Display::E_VertexElementType_SHHD_3N:	dstVertexElement.SetType(Display2::E_VertexElementType_SHHD_3N);	break;
		default:
			dstVertexElement.SetType(Display2::E_VertexElementType_Unknown);
			BIZ_ASSERT(0 && "Unsupported vertex element type");
			return false;
		}

		switch(srcVertexElement.m_Usage)
		{
		case Display::E_VertexElementUsage_Position:		dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Position);		break;
		case Display::E_VertexElementUsage_BlendWeight:		dstVertexElement.SetUsage(Display2::E_VertexElementUsage_BlendWeight);	break;
		case Display::E_VertexElementUsage_Normal: 			dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Normal);		break;
		case Display::E_VertexElementUsage_Colour: 			dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Colour);		break;
		case Display::E_VertexElementUsage_Fog:				dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Fog);			break;
		case Display::E_VertexElementUsage_PSize:			dstVertexElement.SetUsage(Display2::E_VertexElementUsage_PSize);		break;
		case Display::E_VertexElementUsage_BlendIndices:	dstVertexElement.SetUsage(Display2::E_VertexElementUsage_BlendIndices);	break;
		case Display::E_VertexElementUsage_TexCoord:		dstVertexElement.SetUsage(Display2::E_VertexElementUsage_TexCoord);		break;
		case Display::E_VertexElementUsage_Tangent:			dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Tangent);		break;
		case Display::E_VertexElementUsage_Binormal:		dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Binormal);		break;
		default:
			dstVertexElement.SetUsage(Display2::E_VertexElementUsage_Unknown);
			BIZ_ASSERT(0 && "Unsupported vertex element usage");
			return false;
		}

		dstVertexElement.SetUsageIndex(srcVertexElement.m_UsageIndex);
	}

	#if PLATFORM_PS3
	for(U32 i=0; i<pDstVertexDeclaration->m_ElementCount; i++)
	{
		Display2::C_VertexElement& dstVertexElement = pDstVertexDeclaration->m_ElementArray[i];
		for(U32 j=0; j<pDstVertexDeclaration->m_ElementCount; j++)
		{
			Display2::C_VertexElement& dstOtherVertexElement = pDstVertexDeclaration->m_ElementArray[j];
			if(dstVertexElement.m_Stream == dstOtherVertexElement.m_Stream)
			{
				dstVertexElement.m_gcmStride += dstOtherVertexElement.GetSize();
			}
		}
	}
	#endif

	return true;
}

bool ToVertexBuffer2(Display::C_VertexBuffer* pSrcVertexBuffer, Display2::C_VertexBuffer* pDstVertexBuffer)
{
	assert(pSrcVertexBuffer && pSrcVertexBuffer->IsCreated());
	assert(pDstVertexBuffer && !pDstVertexBuffer->IsCreated());

	pDstVertexBuffer->m_Size = pSrcVertexBuffer->GetSize();

	if(pSrcVertexBuffer->m_gcmLocation == CELL_GCM_LOCATION_MAIN)
		pDstVertexBuffer->m_Flags = Display2::E_Flags_System;
	else
		pDstVertexBuffer->m_Flags = Display2::E_Flags_Video;

	pDstVertexBuffer->m_Offset = pSrcVertexBuffer->m_gcmOffset;

	return true;
}

bool ToIndexBuffer2(Display::C_IndexBuffer* pSrcIndexBuffer, Display2::C_IndexBuffer* pDstIndexBuffer)
{
	assert(pSrcIndexBuffer && pSrcIndexBuffer->IsCreated());
	assert(pDstIndexBuffer && !pDstIndexBuffer->IsCreated());

	pDstIndexBuffer->m_Size = pSrcIndexBuffer->GetSize();

	if(pSrcIndexBuffer->m_gcmLocation == CELL_GCM_LOCATION_MAIN)
		pDstIndexBuffer->m_Flags = Display2::E_Flags_System;
	else
		pDstIndexBuffer->m_Flags = Display2::E_Flags_Video;

	pDstIndexBuffer->m_Offset = pSrcIndexBuffer->m_gcmOffset;

	switch(pSrcIndexBuffer->m_Format)
	{
	case Display::E_Format_Index16: pDstIndexBuffer->m_Format = Display2::E_IndexFormat_16; break;
	case Display::E_Format_Index32: pDstIndexBuffer->m_Format = Display2::E_IndexFormat_32; break;
	default:
		pDstIndexBuffer->m_Format = Display2::E_IndexFormat_Unknown;
		BIZ_ASSERT(0 && "Unsupported index format");
		break;
	}

	return true;
}

bool ToVertexShader2(Display::C_VertexShader* pSrcVertexShader, Display2::C_VertexShader* pDstVertexShader)
{
	assert(pSrcVertexShader && pSrcVertexShader->IsCreated());
	assert(pDstVertexShader && !pDstVertexShader->IsCreated());

	pDstVertexShader->m_Size = pSrcVertexShader->m_uCodeSize;
	pDstVertexShader->m_Program = pSrcVertexShader->m_cgbProgram;
	pDstVertexShader->m_ProgramConfig = pSrcVertexShader->m_cgbProgramConfig;

	if(Display2::IsMainAddress(pSrcVertexShader->m_uCodeData))
		pDstVertexShader->m_Flags = Display2::E_Flags_System;
	else
		pDstVertexShader->m_Flags = Display2::E_Flags_Video;

	return true;
}

bool ToPixelShader2(Display::C_PixelShader* pSrcPixelShader, Display2::C_PixelShader* pDstPixelShader)
{
	assert(pSrcPixelShader && pSrcPixelShader->IsCreated());
	assert(pDstPixelShader && !pDstPixelShader->IsCreated());

	pDstPixelShader->m_Size = pSrcPixelShader->m_uCodeSize;
	pDstPixelShader->m_Program = pSrcPixelShader->m_cgbProgram;
	pDstPixelShader->m_ProgramConfig = pSrcPixelShader->m_cgbProgramConfig;

	if(Display2::IsMainAddress(pSrcPixelShader->m_uCodeData))
		pDstPixelShader->m_Flags = Display2::E_Flags_System;
	else
		pDstPixelShader->m_Flags = Display2::E_Flags_Video;

	/*pDstPixelShader->m_Params.Resize(pSrcPixelShader->m_Parameters.Size());
	for(S32 iParam=0; iParam<pSrcPixelShader->m_Parameters.Size(); iParam++)
	{
		Display::C_PixelShaderParameterBase& srcParam = pSrcPixelShader->m_Parameters[iParam];
		Display2::C_PixelShaderParamBase& dstParam = pDstPixelShader->m_Params[iParam];

		dstParam.m_pPixelShader = pDstPixelShader;
		dstParam.m_Offsets.Resize(srcParam.m_Offsets.Size());

		srcParam.m_Param2.m_pPixelShader = pDstPixelShader;
		srcParam.m_Param2.m_Offsets.Resize(srcParam.m_Offsets.Size());

		for(S32 iOffset=0; iOffset<srcParam.m_Offsets.Size(); iOffset++)
		{
			dstParam.m_Offsets[iOffset] = srcParam.m_Offsets[iOffset];
			srcParam.m_Param2.m_Offsets[iOffset] = srcParam.m_Offsets[iOffset];
		}
	}*/

	return true;
}
#endif
