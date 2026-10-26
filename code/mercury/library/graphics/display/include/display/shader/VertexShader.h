#ifndef DISPLAY_SHADER_VERTEX_SHADER_H
#define DISPLAY_SHADER_VERTEX_SHADER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11)
#include "Display/Device/D3D11/D3D11Resources.h" // [Blur1PCDX11] Native resources.
#elif defined(DEVICE_PS3GCM)
#include "Display/Shader/GCM/GCMVertexShader.h"
#elif defined(DEVICE_PCD3D10)
#include "Display/Shader/D3D10/D3D10VertexShader.h"
#elif defined(DEVICE_PCGL) || defined(DEVICE_PS3GL)
#include "Display/Shader/GL/GLVertexShader.h"
#else
#pragma message("Unsupported device define when compiling VertexShader.h")
#endif

#endif // DISPLAY_SHADER_VERTEX_SHADER_H
