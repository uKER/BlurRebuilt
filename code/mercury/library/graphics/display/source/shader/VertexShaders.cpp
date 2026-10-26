#include "stdafx.h"
#include "Display/Shader/VertexShaders.h"
#include "Display/Device/Device.h"

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Display/Shaders/Render/PS3/vsDefault_XY.shader"
#endif
#endif

EXTERN_SHADER(vsDefault_XY);
EXTERN_SHADER(vsDefault_XYVertexColour);

namespace Display
{

void CreateVertexShaders()
{
	RESOURCE_MEMORY_ID("DisplayShaders");

#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM) // [Blur1PCDX11]
	CreateVertexShaderFromFunc("Shaders/vsDefault_XY", 0, &VertexShader_XY::m_VertexShader, REF_SHADER(vsDefault_XY));
	CreateVertexShaderFromFunc("Shaders/vsDefault_XYVertexColour", 0, &VertexShader_XY::m_VertexShaderVertexColour, REF_SHADER(vsDefault_XYVertexColour));
#endif
}

void DestroyVertexShaders()
{
#if defined(DEVICE_PCD3D11) || defined(DEVICE_PS3GCM) // [Blur1PCDX11]
	Display::Device()->Destroy(&VertexShader_XY::m_VertexShader);
	Display::Device()->Destroy(&VertexShader_XY::m_VertexShaderVertexColour);
#endif
}

C_VertexShader VertexShader_XY::m_VertexShader;
C_VertexShader VertexShader_XY::m_VertexShaderVertexColour;

}; // namespace Display
