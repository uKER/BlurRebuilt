#include "stdafx.h"
#include "Effect/Sky/CelestialBody.h"

#include <Display/Buffer/VertexDeclarations.h>
#include <Display/Context/Context.h>
#include <Render/Shaders/Constants.h>
#include <Render/State/RenderState.h>

#define SCENEGRAPH_ALLOW_INCLUDE
#include <SceneGraph/SceneGraphImpl.h>
#include <SceneGraph/View.h>
#include <SceneGraph/Visitor.h>
#include <SceneGraph/Context.h>

EXTERN_SHADER(vsCelestialBody);
EXTERN_SHADER(psCelestialBody);
EXTERN_SHADER(psCelestialBodyHDR);
EXTERN_SHADER(psCelestialBodyHDRU32);

namespace Effect
{
namespace Sky
{

Debug::Float	g_debugCelestialBodyDepth	("Graphics.Effect.CelestialBody.Depth",		0.99f,	Debug::Range(0.0f, 1.0f));
Debug::Bool		g_debugCelestialBodyEnabled	("Graphics.Effect.CelestialBody.Enabled",	true);

C_CelestialBody::C_CelestialBody()
	: Display::I_Renderer("CelestialBody")
	, m_fSize(0.0f)
	, m_fHeadingOffset(0.0f)
	, m_fElevationOffset(0.0f)
	, m_vColour(0.0f, 0.0f, 0.0f, 0.0f)
	, m_iRenderPass(0)
{
}

C_CelestialBody::~C_CelestialBody()
{
	DestroyRenderer();
}

bool C_CelestialBody::OnCreate()
{
	RESOURCE_MEMORY_ID("Effect::C_CelestialBody");

	if (!Display::CreateVertexShaderFromFunc("Shader/vsCelestialBody", 0, &m_VertexShader, REF_SHADER(vsCelestialBody)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("Shader/psCelestialBodyHDRU32", 0, &m_PixelShader, REF_SHADER(psCelestialBodyHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("Shader/psCelestialBodyHDR", 0, &m_PixelShader, REF_SHADER(psCelestialBodyHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("Shader/psCelestialBody", 0, &m_PixelShader, REF_SHADER(psCelestialBody)))
			return false;
	}

	m_LuminanceParameter = m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE, "g_vLuminance");

	Display::CreateTexture2DFromFile(m_TextureName, &m_Texture);

	return true;
}

void C_CelestialBody::OnDestroy()
{
	Display::Device()->Destroy(&m_Texture);
	Display::Device()->Destroy(&m_VertexShader);
	Display::Device()->Destroy(&m_PixelShader);
}

void C_CelestialBody::OnRender(SceneGraph::C_Context& context) const
{
	// say that this uses display. this will cause a context reset for other stuff later
	Display::C_Context* pDisplay = Display::GetContext(&context);
	
	if(context.GetContextFlags() != pDisplay->GetFlags())
	{
		Display::Device()->InvalidateState();
	}

	context.SetContextFlags(0);

	Render();
}

void C_CelestialBody::OnGather(const SceneGraph::C_View& view) const
{
	SceneGraph::VolatileNodeInfo volatileNodeInfo;
	volatileNodeInfo.distance = 0x7FFFFFFF;
	
	view.AddVisibleNode(this, m_iRenderPass, 0, volatileNodeInfo.data);
}

void C_CelestialBody::Accept(SceneGraph::I_Visitor& visitor) const
{
	visitor.Visit(*this);
}

void C_CelestialBody::SetRenderPass(const U32 renderPass)
{
	m_iRenderPass = renderPass;
}

void C_CelestialBody::SetDirection(const Maths::HVec3& vDirection)
{
	const F32 fLength = sqrtf(vDirection.X() * vDirection.X() + vDirection.Z() * vDirection.Z());
	const F32 fHeading = atan2f(vDirection.X(), vDirection.Z()) + m_fHeadingOffset;
	const F32 fElevation = Maths::PiBy2 - atan2(fLength, vDirection.Y()) + m_fElevationOffset;
	const F32 fSinHeading = sinf(fHeading);
	const F32 fCosHeading = cosf(fHeading);
	const F32 fSinElevation = sinf(fElevation);
	const F32 fCosElevation = cosf(fElevation);
	m_vDirection = Display::C_Float3(fSinHeading * fCosElevation, fSinElevation, fCosHeading * fCosElevation);
}

void C_CelestialBody::SetDirection(const Display::C_Float3& vDirection)
{
	const F32 fLength = sqrtf(vDirection.x * vDirection.x + vDirection.z * vDirection.z);
	const F32 fHeading = atan2f(vDirection.x, vDirection.z) + m_fHeadingOffset;
	const F32 fElevation = Maths::PiBy2 - atan2(fLength, vDirection.y) + m_fElevationOffset;
	const F32 fSinHeading = sinf(fHeading);
	const F32 fCosHeading = cosf(fHeading);
	const F32 fSinElevation = sinf(fElevation);
	const F32 fCosElevation = cosf(fElevation);
	m_vDirection = Display::C_Float3(fSinHeading * fCosElevation, fSinElevation, fCosHeading * fCosElevation);
}

void C_CelestialBody::Render() const
{
	PERF_EVENT("CelestialBody");

	#ifndef CFG_SPEED
	if (!g_debugCelestialBodyEnabled)
		return;
	#endif

	const F32	fDepth	= Display::Device()->GetViewSettings3D().GetFarClip() * g_debugCelestialBodyDepth;

	Display::Device()->SetBlendMode(Display::E_BlendMode_AlphaAdditive);
	Display::Device()->SetCullMode(Display::E_CullMode_None);
	Display::Device()->SetDepthEnable(Display::E_Bool_True);
	Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);

	Display::Device()->SetTexture(0, &m_Texture);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);

	const Display::C_Float4 vsSettings(m_vDirection.x * fDepth, m_vDirection.y * fDepth, m_vDirection.z * fDepth, m_fSize * fDepth);

	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&m_vColour, 1);
	Display::Device()->SetVertexShaderConstantF(CV_RENDER_SETTINGS_1, (F32*)&vsSettings, 1);

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParameter, (F32*)&psLuminance);
	}

	Display::Device()->SetVertexShader(&m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Vertex_XY* pVertex = (Display::Vertex_XY*) Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleStrip, 4, sizeof(Display::Vertex_XY), Display::Vertex_XY::m_pDeclaration);
	if (pVertex)
	{
		pVertex->x = 0.0f;
		pVertex->y = 0.0f;
		++pVertex;

		pVertex->x = 1.0f;
		pVertex->y = 0.0f;
		++pVertex;

		pVertex->x = 0.0f;
		pVertex->y = 1.0f;
		++pVertex;

		pVertex->x = 1.0f;
		pVertex->y = 1.0f;
		++pVertex;

		Display::Device()->EndVertices();
	}

	Display::Device()->SetCullMode(Display::E_CullMode_Default);

	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Default);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Default);
}

bool C_CelestialBody::ReloadTexture()
{
	#if PLATFORM_PS3
	Display::Device()->BlockUntilIdle();
	#endif

	Display::Device()->Destroy(&m_Texture);

	if (!Display::CreateTexture2DFromFile(m_TextureName, &m_Texture))
		return false;

	return true;
}

}; // namespace Sky
}; // namespace Effect
