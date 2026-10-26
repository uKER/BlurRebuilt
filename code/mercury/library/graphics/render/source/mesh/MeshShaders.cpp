#include "stdafx.h"
#include "Render/Mesh/MeshShaders.h"

EXTERN_SHADER(vsMesh0);
EXTERN_SHADER(vsMesh1);
EXTERN_SHADER(vsMesh2);
EXTERN_SHADER(vsMesh3);
EXTERN_SHADER(vsMesh4);
EXTERN_SHADER(psMesh0);
EXTERN_SHADER(psMesh1);
EXTERN_SHADER(psMesh2);
EXTERN_SHADER(psMesh3);
EXTERN_SHADER(psMesh4);

#ifndef OBJ_SHADERS
#if defined(DEVICE_PS3GCM)
#include "Render/Shaders/Mesh/PS3/vsMesh0.shader"
#include "Render/Shaders/Mesh/PS3/vsMesh1.shader"
#include "Render/Shaders/Mesh/PS3/vsMesh2.shader"
#include "Render/Shaders/Mesh/PS3/vsMesh3.shader"
#include "Render/Shaders/Mesh/PS3/vsMesh4.shader"
#include "Render/Shaders/Mesh/PS3/psMesh0.shader"
#include "Render/Shaders/Mesh/PS3/psMesh1.shader"
#include "Render/Shaders/Mesh/PS3/psMesh2.shader"
#include "Render/Shaders/Mesh/PS3/psMesh3.shader"
#include "Render/Shaders/Mesh/PS3/psMesh4.shader"
#endif
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Render
{
namespace Mesh
{

C_Shaders::C_Shaders()
{
}

C_Shaders::~C_Shaders()
{
}

bool C_Shaders::CreateShaders()
{
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMesh0", 0, &m_VertexShader[E_VertexShader_Default], REF_SHADER(vsMesh0)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMesh1", 0, &m_VertexShader[E_VertexShader_Albedo], REF_SHADER(vsMesh1)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMesh2", 0, &m_VertexShader[E_VertexShader_Detail], REF_SHADER(vsMesh2)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMesh3", 0, &m_VertexShader[E_VertexShader_StandardShadowMap], REF_SHADER(vsMesh3)))
		return false;
	if (!Display::CreateVertexShaderFromFunc("Shaders/vsMesh4", 0, &m_VertexShader[E_VertexShader_VarianceShadowMap], REF_SHADER(vsMesh4)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMesh0", 0, &m_PixelShader[E_PixelShader_Default], REF_SHADER(psMesh0)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMesh1", 0, &m_PixelShader[E_PixelShader_Albedo], REF_SHADER(psMesh1)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMesh2", 0, &m_PixelShader[E_PixelShader_Detail], REF_SHADER(psMesh2)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMesh3", 0, &m_PixelShader[E_PixelShader_StandardShadowMap], REF_SHADER(psMesh3)))
		return false;
	if (!Display::CreatePixelShaderFromFunc("Shaders/psMesh4", 0, &m_PixelShader[E_PixelShader_VarianceShadowMap], REF_SHADER(psMesh4)))
		return false;

	return true;
}

void C_Shaders::DestroyShaders()
{
	for (U32 i = 0; i < E_VertexShader_Count; i++)
		Display::Device()->Destroy(&m_VertexShader[i]);
	for (U32 i = 0; i < E_PixelShader_Count; i++)
		Display::Device()->Destroy(&m_PixelShader[i]);
}

}; // namespace Mesh
}; // namespace Render
