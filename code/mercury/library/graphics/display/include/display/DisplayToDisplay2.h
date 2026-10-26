#ifndef DISPLAY_DISPLAY_TO_DISPLAY2_H
#define DISPLAY_DISPLAY_TO_DISPLAY2_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display2.h>

#include "Display/Device/DeviceTypesEnums.h"

const Display2::E_Blend ToBlend2(const Display::E_Blend srcBlend);
const Display2::E_BlendOp ToBlendOp2(const Display::E_BlendOp srcBlendOp);
const Display2::E_Bool ToBool2(const Display::E_Bool srcBool);
const Display2::E_CmpFunc ToCmpFunc2(const Display::E_CmpFunc srcCmpFunc);
const Display2::E_CullMode ToCullMode2(const Display::E_CullMode srcCullMode);
const Display2::E_FillMode ToFillMode2(const Display::E_FillMode srcFillMode);
const Display2::E_PrimitiveType ToPrimitiveType2(const Display::E_PrimitiveType srcPrimitiveType);
const Display2::E_StencilOp ToStencilOp2(const Display::E_StencilOp srcStencilOp);
const Display2::E_TextureAddress ToTextureAddress2(const Display::E_TextureAddress srcTextureAddress);
const Display2::E_TextureFilter ToTextureFilter2(const Display::E_TextureFilter srcTextureFilter);
const Display2::E_ResourceType ToResourceType2(const Display::E_ResourceType srcResourceType);

#if 0
bool ToVertexDeclaration2(Display::C_VertexDeclaration* pSrcVertexDeclaration, Display2::C_VertexDeclaration* pDstVertexDeclaration);
bool ToVertexBuffer2(Display::C_VertexBuffer* pSrcVertexBuffer, Display2::C_VertexBuffer* pDstVertexBuffer);
bool ToIndexBuffer2(Display::C_IndexBuffer* pSrcIndexBuffer, Display2::C_IndexBuffer* pDstIndexBuffer);
bool ToVertexShader2(Display::C_VertexShader* pSrcVertexShader, Display2::C_VertexShader* pDstVertexShader);
bool ToPixelShader2(Display::C_PixelShader* pSrcPixelShader, Display2::C_PixelShader* pDstPixelShader);
#endif

#endif // DISPLAY_DISPLAY_TO_DISPLAY2_H
