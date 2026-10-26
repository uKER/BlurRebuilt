#ifndef DISPLAY_BUFFER_VERTEX_DECLARATION_H
#define DISPLAY_BUFFER_VERTEX_DECLARATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceDefines.h"

#if defined(DEVICE_PCD3D11)
#include "Display/Device/D3D11/D3D11Resources.h" // [Blur1PCDX11] Native resources.
#elif defined(DEVICE_PS3GCM)
#include "Display/Buffer/GCM/GCMVertexDeclaration.h"
#elif defined(DEVICE_PCD3D10)
#include "Display/Buffer/D3D10/D3D10VertexDeclaration.h"
#elif defined(DEVICE_PCGL) || defined(DEVICE_PS3GL)
#include "Display/Buffer/GL/GLVertexDeclaration.h"
#else
#pragma message("Unsupported device define when compiling VertexDeclaration.h")
#endif

#endif // DISPLAY_BUFFER_VERTEX_DECLARATION_H
