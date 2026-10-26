#ifndef DISPLAY_SHADER_GCM_GCM_PIXEL_SHADER_H
#define DISPLAY_SHADER_GCM_GCM_PIXEL_SHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/GCM/GCM.h"
#include "Display/Resource/Resource.h"

namespace Display
{

typedef Display2::C_PixelShaderParam C_PixelShaderParameter;

typedef Display2::C_PixelShader C_PixelShader;

typedef Display2::C_PixelShaderParamArray C_PixelShaderParameterArray;

typedef U32 C_PixelShaderConstantLock;

typedef U32 C_PixelShaderConstantLockKey;

}; // namespace Display

#include "Display/Shader/GCM/GCMPixelShader.inl"

#endif // DISPLAY_SHADER_GCM_GCM_PIXEL_SHADER_H
