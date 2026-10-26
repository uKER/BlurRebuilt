#include "stdafx.h"
#include "Effect/Sky/AtmosphericSkyDome.h"
#include "Effect/Shaders/Constants.h"
#include "Render/State/RenderState.h"
#include "Display/Buffer/VertexDeclarations.h"

EXTERN_SHADER(vsAtmosphericSkyDome);
EXTERN_SHADER(psAtmosphericSkyDome);
EXTERN_SHADER(psAtmosphericSkyDomeHDR);
EXTERN_SHADER(psAtmosphericSkyDomeHDRU32);

JQ_DECLARE_PPU_MODULE(SolveAtmosphereModule, Effect::Sky::SolveAtmosphereTask);

namespace Effect
{
namespace Sky
{

Debug::Float	g_debugAtmosphericRedWavelength		("Graphics.Render.Sky.Atmospheric.RedWavelength",	0.650f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugAtmosphericGreenWavelength	("Graphics.Render.Sky.Atmospheric.GreenWavelength",	0.570f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugAtmosphericBlueWavelength	("Graphics.Render.Sky.Atmospheric.BlueWavelength",	0.475f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugAtmosphericSunIntensity		("Graphics.Render.Sky.Atmospheric.SunIntensity",		10.0f,		Debug::Range(0.0f, 100.0f));
Debug::Float	g_debugAtmosphericMieCoeff			("Graphics.Render.Sky.Atmospheric.MieCoeff",			0.0010f,	Debug::Range(0.0f, 0.01f));
Debug::Float	g_debugAtmosphericMieScale			("Graphics.Render.Sky.Atmospheric.MieScale",			0.1f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugAtmosphericMieG				("Graphics.Render.Sky.Atmospheric.MieG",				-0.99f,		Debug::Range(-2.0f, 2.0f));
Debug::Float	g_debugAtmosphericRayleighCoeff		("Graphics.Render.Sky.Atmospheric.RayleighCoeff",	0.0025f,	Debug::Range(0.0f, 0.01f));
Debug::Float	g_debugAtmosphericRayleighScale		("Graphics.Render.Sky.Atmospheric.RayleighScale",	0.25f,		Debug::Range(0.0f, 1.0f));
Debug::Float	g_debugAtmosphericInnerRadius		("Graphics.Render.Sky.Atmospheric.InnerRadius",		100.0f,		Debug::Range(0.0f, 1000.0f));
Debug::Float	g_debugAtmosphericRadiusRatio		("Graphics.Render.Sky.Atmospheric.RadiusRatio",		1.025f,		Debug::Range(1.0f, 1.1f));
Debug::Float	g_debugAtmosphericFieldOfView		("Graphics.Render.Sky.Atmospheric.FieldOfView",		22.5f,		Debug::Range(0.0f, 90.0f));
Debug::Int		g_debugAtmosphericSamples			("Graphics.Render.Sky.Atmospheric.Samples",			5,			Debug::Range(1, 25));

static const U32 kAtmosphericTextureWidth	= 128;
static const U32 kAtmosphericTextureHeight	= 64;

C_AtmosphericSkyDome::C_AtmosphericSkyDome() :
	m_vSunDirection(0.0f, 1.0f, 0.0f, 0.0f),
	m_hJob(Scheduler::JQ_GROUP_NONE)
{
	m_TextureNo = 0;
	m_TextureLineNo = kAtmosphericTextureHeight;

	GetParameters(m_Parameters);
}

C_AtmosphericSkyDome::~C_AtmosphericSkyDome()
{
	DestroyRenderer();
}

bool C_AtmosphericSkyDome::OnCreate()
{
	if (!C_SkyDomeBase::OnCreate())
		return false;

	if (!Display::CreateVertexShaderFromFunc("Shaders/vsAtmosphericSkyDome", 0, &m_VertexShader, REF_SHADER(vsAtmosphericSkyDome)))
		return false;

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		PostProcess::E_HighDynamicRangeType eHDR = Render::C_RenderState::Instance().HDR()->GetType();

		if (eHDR == PostProcess::E_HighDynamicRangeType_ShaderU32)
		{
			if (!Display::CreatePixelShaderFromFunc("psAtmosphericSkyDomeHDRU32", 0, &m_PixelShader, REF_SHADER(psAtmosphericSkyDomeHDRU32)))
				return false;
		}
		else if (eHDR == PostProcess::E_HighDynamicRangeType_Shader)
		{
			if (!Display::CreatePixelShaderFromFunc("psAtmosphericSkyDomeHDR", 0, &m_PixelShader, REF_SHADER(psAtmosphericSkyDomeHDR)))
				return false;
		}
	}
	else
	{
		if (!Display::CreatePixelShaderFromFunc("psAtmosphericSkyDome", 0, &m_PixelShader, REF_SHADER(psAtmosphericSkyDome)))
			return false;
	}

	m_LuminanceParam				= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_LUMINANCE,	"g_vLuminance");
	m_SunDirParam					= m_PixelShader.GetParameter(CP_RENDER_SETTINGS,				"g_vSunDir");
	m_RayleighSunIntensityParam		= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_1,				"g_vRayleighSunIntensity");
	m_MieSunIntensityParam			= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_2,				"g_vMieSunIntensity");
	m_MieGParam						= m_PixelShader.GetParameter(CP_RENDER_SETTINGS_3,				"g_vMieG");

	m_hJob = Scheduler::CreateGroup();

	return true;
}

bool C_AtmosphericSkyDome::OnCreateVolatile()
{
	if (!C_SkyDomeBase::OnCreateVolatile())
		return false;

	m_TextureNo = 0;
	for (U32 textureNo = 0; textureNo < kAtmosphericTextureCount; ++textureNo)
	{
		if (!Display::Device()->CreateTexture2D(kAtmosphericTextureWidth, kAtmosphericTextureHeight, 1, Display::E_Format_Linear | Display::E_Usage_Dynamic, Display::E_Format_A16B16G16R16F, &m_RayleighTextures[textureNo]))
			return false;
		if (!Display::Device()->CreateTexture2D(kAtmosphericTextureWidth, kAtmosphericTextureHeight, 1, Display::E_Format_Linear | Display::E_Usage_Dynamic, Display::E_Format_A16B16G16R16F, &m_MieTextures[textureNo]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_RayleighTextures[textureNo], "_RayleighTexture");
		Display2::Device()->SetTextureName(&m_MieTextures[textureNo], "_MieTexture");
		#endif
	}

	BeginSky();

	return true;
}

void C_AtmosphericSkyDome::OnDestroy()
{
	Scheduler::DestroyGroup(m_hJob);

	C_SkyDomeBase::OnDestroy();
}

void C_AtmosphericSkyDome::OnDestroyVolatile()
{
	EndSky();

	for (U32 textureNo = 0; textureNo < kAtmosphericTextureCount; ++textureNo)
	{
		Display::Device()->Destroy(&m_RayleighTextures[textureNo]);
		Display::Device()->Destroy(&m_MieTextures[textureNo]);
	}

	C_SkyDomeBase::OnDestroyVolatile();
}

void C_AtmosphericSkyDome::Update()
{
	IterateSky();
}

void C_AtmosphericSkyDome::Bind(Display::C_Context *pContext) const
{
	C_SkyDomeBase::Bind(pContext);

	Display::Device()->SetPixelShaderParameterF(m_SunDirParam, (F32*)&m_Parameters.m_vSunDirection);
	Display::Device()->SetPixelShaderParameterF(m_RayleighSunIntensityParam, (F32*)&m_Parameters.m_vRayleighSunIntensity);
	Display::Device()->SetPixelShaderParameterF(m_MieSunIntensityParam, (F32*)&m_Parameters.m_vMieSunIntensity);
	Display::Device()->SetPixelShaderParameterF(m_MieGParam, (F32*)&m_Parameters.m_vMieG);

	if (Render::C_RenderState::Instance().ShaderHDR())
	{
		Display::C_Float4 psLuminance = Render::C_RenderState::Instance().HDR()->GetShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&psLuminance);
		
		Display::Device()->SetTexture(15, Render::C_RenderState::Instance().HDR()->GetAdaptedLuminanceTexture());
		Display::Device()->SetTextureFilter(15, Display::E_TextureFilter_Point);
		Display::Device()->SetTextureAddress(15, Display::E_TextureAddress_Clamp);
	}
	else
	{
		Display::C_Float4 psGain = Render::C_RenderState::Instance().GetGainShaderParameters();
		Display::Device()->SetPixelShaderParameterF(m_LuminanceParam, (F32*)&psGain);
	}

	Display::Device()->SetTexture(0, &m_RayleighTextures[m_TextureNo]);
	Display::Device()->SetTexture(1, &m_MieTextures[m_TextureNo]);
	Display::Device()->SetTextureFilterMagMinMip(0, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_None); // should work on PS3 and 360 for FP16 textures
	Display::Device()->SetTextureFilterMagMinMip(1, Display::E_TextureFilter_Linear, Display::E_TextureFilter_Linear, Display::E_TextureFilter_None); // should work on PS3 and 360 for FP16 textures
	Display::Device()->SetTextureAddressUVW(0, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureAddressUVW(1, Display::E_TextureAddress_Wrap, Display::E_TextureAddress_Clamp, Display::E_TextureAddress_Clamp);
}

void C_AtmosphericSkyDome::Unbind(Display::C_Context *pContext) const
{
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Wrap);

	C_SkyDomeBase::Unbind(pContext);
}

void C_AtmosphericSkyDome::BeginSky()
{
	GetTask();
	m_Task.m_LinesToUpdate	= kAtmosphericTextureHeight;
	m_TextureLineNo			= 0;

	SolveAtmosphere(&m_Task);
}

void C_AtmosphericSkyDome::EndSky()
{
	while (Scheduler::jqGetQueuedBatchCount(m_hJob) + Scheduler::jqGetExecutingBatchCount(m_hJob) > 0);

	m_TextureNo = (m_TextureNo + 1) % kAtmosphericTextureCount;

	m_RayleighTextures[m_TextureNo].Unlock(0);
	m_MieTextures[m_TextureNo].Unlock(0);
}

void C_AtmosphericSkyDome::IterateSky()
{
	// if we have finished solving the texture
	if (m_TextureLineNo >= kAtmosphericTextureHeight)
	{
		if (Scheduler::jqGetQueuedBatchCount(m_hJob) + Scheduler::jqGetExecutingBatchCount(m_hJob) == 0)
		{
			m_TextureNo = (m_TextureNo + 1) % kAtmosphericTextureCount;

			m_RayleighTextures[m_TextureNo].Unlock(0);
			m_MieTextures[m_TextureNo].Unlock(0);

			m_Parameters = m_Task.m_Parameters;
			m_TextureLineNo = 0;

			GetTask();
		}
	}
	else
	{
		Scheduler::AddTask(SolveAtmosphereModule, 0, &m_Task, m_hJob, m_hPolicy, 0, 0);
	}
}

void C_AtmosphericSkyDome::GetTask()
{
	GetParameters(m_Task.m_Parameters);

	m_Task.m_iSamples			= g_debugAtmosphericSamples;
	m_Task.m_pLinePosition		= &m_TextureLineNo;
	m_Task.m_LinesToUpdate		= 1;
	m_Task.m_iBufferWidth		= kAtmosphericTextureWidth;
	m_Task.m_iBufferHeight		= kAtmosphericTextureHeight;

	const U32 iNextTexture = (m_TextureNo + 1) % kAtmosphericTextureCount;
	{
		const Display::C_LockedRect lockedRect	= m_RayleighTextures[iNextTexture].Lock(0);
		m_Task.m_pRayleighBuffer	= (U8*)lockedRect.pBits;
		m_Task.m_iRayleighPitch		= lockedRect.pitch;
		m_Task.m_pRayleighColour	= &m_RayleighColour[iNextTexture];
	}
	{
		const Display::C_LockedRect lockedRect	= m_MieTextures[iNextTexture].Lock(0);
		m_Task.m_pMieBuffer			= (U8*)lockedRect.pBits;
		m_Task.m_iMiePitch			= lockedRect.pitch;
		m_Task.m_pMieColour			= &m_MieColour[iNextTexture];
	}
}

void C_AtmosphericSkyDome::GetParameters(C_AtmosphericParameters& params)
{
	const F32 f4RPi					= 4.0f * Maths::Pi * g_debugAtmosphericRayleighCoeff;
	const F32 fRayleighRed			= f4RPi / powf(g_debugAtmosphericRedWavelength, 4.00f);
	const F32 fRayleighGreen		= f4RPi / powf(g_debugAtmosphericGreenWavelength, 4.00f);
	const F32 fRayleighBlue			= f4RPi / powf(g_debugAtmosphericBlueWavelength, 4.00f);
	const F32 fRayleighScale		= 1.0f / g_debugAtmosphericRayleighScale;
	const F32 fRayleighSunIntensity	= g_debugAtmosphericSunIntensity * g_debugAtmosphericRayleighCoeff;

	const F32 f4MPi					= 4.0f * Maths::Pi * g_debugAtmosphericMieCoeff;
	const F32 fMieRed				= f4MPi / powf(g_debugAtmosphericRedWavelength, 0.84f);
	const F32 fMieGreen				= f4MPi / powf(g_debugAtmosphericGreenWavelength, 0.84f);
	const F32 fMieBlue				= f4MPi / powf(g_debugAtmosphericBlueWavelength, 0.84f);
	const F32 fMieScale				= 1.0f / g_debugAtmosphericMieScale;
	const F32 fMieSunIntensity		= g_debugAtmosphericSunIntensity * g_debugAtmosphericMieCoeff;
	const F32 fG					= g_debugAtmosphericMieG;
	const F32 fG2					= fG * fG;

	params.m_vSunDirection			= m_vSunDirection;
	params.m_vRayleighCoeffs		= Display::C_Float4(fRayleighRed, fRayleighGreen, fRayleighBlue, fRayleighScale);
	params.m_vMieCoeffs				= Display::C_Float4(fMieRed, fMieGreen, fMieBlue, fMieScale);
	params.m_vRayleighSunIntensity	= Display::C_Float4(fRayleighSunIntensity / powf(g_debugAtmosphericRedWavelength, 4.0f), fRayleighSunIntensity / powf(g_debugAtmosphericGreenWavelength, 4.0f), fRayleighSunIntensity / powf(g_debugAtmosphericBlueWavelength, 4.0f), 0.0f);
	params.m_vMieSunIntensity		= Display::C_Float4(fMieSunIntensity / powf(g_debugAtmosphericRedWavelength, 0.84f), fMieSunIntensity / powf(g_debugAtmosphericGreenWavelength, 0.84f), fMieSunIntensity / powf(g_debugAtmosphericBlueWavelength, 0.84f), 0.0f);
	params.m_vMieG					= Display::C_Float4(1.5f * ((1.0f - fG2) / (2.0f + fG2)), 1.0f + fG2, 2.0f * fG, 0.0f);
	params.m_fInnerRadius			= g_debugAtmosphericInnerRadius;
	params.m_fOuterRadius			= params.m_fInnerRadius * g_debugAtmosphericRadiusRatio;
	params.m_fScale					= 1.0f / (params.m_fOuterRadius - params.m_fInnerRadius);
	params.m_fFieldOfView			= g_debugAtmosphericFieldOfView * Maths::Pi / 180.0f;
}

} // namespace Sky
} // namespace Effect
