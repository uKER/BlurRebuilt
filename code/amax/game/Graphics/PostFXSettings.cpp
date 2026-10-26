#include "stdafx.h"
#include <Graphics/PostFxSettings.h>
#include <Graphics/PostFxMatrix.h>
#include <Graphics/HDR.h>

#include <Fx/EnvironmentSettings.h>

#include <GameApp.h>
#include <Render/PostProcess/Bloom/BloomFeedbackGhost.h>

namespace HDR
{
	extern Maths::Mat44			g_mColour;
	extern F32					g_fFilmGrainIntensity;
	extern F32					g_fFilmGrainLuminance;
	extern F32					g_fVignetteWeight;
	extern F32					g_fTunnelAmount;
	extern F32					g_ColourSep, g_Blockiness, g_ScreenOverlay;
	extern Display::C_Float2	g_vFlareScales[4];
	extern Display::C_Float4	g_vFlareColours[4];
	extern bool					g_bReloadBloomCurve;
};

void PostFxSettingsCallback(XtEventType event, XtRecord* record, Game::PostFxSettings* pObj, XtCallbackContext* context)
{
	if (event == kXtEtConstruct)
		HDR::g_bReloadBloomCurve = true;
}

XT_TYPE(Game::BlurType, "BlurType")
	XT_OPTION(Game::BlurType_MotionBlur,	"MotionBlur")
	XT_OPTION(Game::BlurType_DepthOfField,	"DepthOfField")
XT_END_TYPE

XT_TYPE(Game::PostFxSettings, "PostFxSettings")
	XT_CALLBACK(PostFxSettingsCallback)
	XT_FIELD(matrices,				"matrices")
	XT_FIELD(root,					"root")
	XT_FIELD(lookupTexture,			"lookupTexture")
	XT_FIELD(bloomThresholdCurve,	"bloomThresholdCurve")
	XT_FIELD(lensFlares,			"lensFlares")
	XT_FIELD(fFilmGrainIntensity,	"filmGrainIntensity")
	XT_FIELD(fFilmGrainLuminance,	"filmGrainLuminance")
	XT_FIELD(fVignetteWeight,		"vignetteWeight")
	XT_FIELD(fTunnelAmount,			"tunnelAmount")
	XT_FIELD(fBloomFeedback,		"bloomFeedback")
	XT_FIELD(blurType,				"blurType")
	XT_FIELD(uid,					"uid")
XT_END_TYPE

namespace Game
{

/// constructor
PostFxEnvSettings::PostFxEnvSettings(PostFxSettings* pData)
	: m_LookupTexture(pData->lookupTexture)
	, m_FilmGrainIntensity(pData->fFilmGrainIntensity)
	, m_FilmGrainLuminance(pData->fFilmGrainLuminance)
	, m_VignetteWeight(pData->fVignetteWeight)
	, m_TunnelAmount(pData->fTunnelAmount)
	, m_BloomFeedback(pData->fBloomFeedback)
	, m_BlurType(pData->blurType)
	, m_BloomThresholdCurve(pData->bloomThresholdCurve)
	, m_UID(pData->uid)
{
	m_ColourMatrix = Maths::Const<Maths::Mat44>::Identity();
	
	PostFxMatrix* pNode = pData->root;
	while(pNode)
	{
		if (pNode->m_bEnabled)
			m_ColourMatrix = m_ColourMatrix * pNode->Matrix();
		pNode = pNode->m_pOut;
	}

	m_FlareScales[0].x = pData->lensFlares.m_Flare0.m_Scale.vx * pData->lensFlares.m_Scale.vx;
	m_FlareScales[0].y = pData->lensFlares.m_Flare0.m_Scale.vy * pData->lensFlares.m_Scale.vy;
	m_FlareColours[0].x = pData->lensFlares.m_Flare0.m_Colour.r * pData->lensFlares.m_Colour.r;
	m_FlareColours[0].y = pData->lensFlares.m_Flare0.m_Colour.g * pData->lensFlares.m_Colour.g;
	m_FlareColours[0].z = pData->lensFlares.m_Flare0.m_Colour.b * pData->lensFlares.m_Colour.b;
	m_FlareColours[0].w = 1.0f;

	m_FlareScales[1].x = pData->lensFlares.m_Flare1.m_Scale.vx * pData->lensFlares.m_Scale.vx;
	m_FlareScales[1].y = pData->lensFlares.m_Flare1.m_Scale.vy * pData->lensFlares.m_Scale.vy;
	m_FlareColours[1].x = pData->lensFlares.m_Flare1.m_Colour.r * pData->lensFlares.m_Colour.r;
	m_FlareColours[1].y = pData->lensFlares.m_Flare1.m_Colour.g * pData->lensFlares.m_Colour.g;
	m_FlareColours[1].z = pData->lensFlares.m_Flare1.m_Colour.b * pData->lensFlares.m_Colour.b;
	m_FlareColours[1].w = 1.0f;

	m_FlareScales[2].x = pData->lensFlares.m_Flare2.m_Scale.vx * pData->lensFlares.m_Scale.vx;
	m_FlareScales[2].y = pData->lensFlares.m_Flare2.m_Scale.vy * pData->lensFlares.m_Scale.vy;
	m_FlareColours[2].x = pData->lensFlares.m_Flare2.m_Colour.r * pData->lensFlares.m_Colour.r;
	m_FlareColours[2].y = pData->lensFlares.m_Flare2.m_Colour.g * pData->lensFlares.m_Colour.g;
	m_FlareColours[2].z = pData->lensFlares.m_Flare2.m_Colour.b * pData->lensFlares.m_Colour.b;
	m_FlareColours[2].w = 1.0f;

	m_FlareScales[3].x = pData->lensFlares.m_Flare3.m_Scale.vx * pData->lensFlares.m_Scale.vx;
	m_FlareScales[3].y = pData->lensFlares.m_Flare3.m_Scale.vy * pData->lensFlares.m_Scale.vy;
	m_FlareColours[3].x = pData->lensFlares.m_Flare3.m_Colour.r * pData->lensFlares.m_Colour.r;
	m_FlareColours[3].y = pData->lensFlares.m_Flare3.m_Colour.g * pData->lensFlares.m_Colour.g;
	m_FlareColours[3].z = pData->lensFlares.m_Flare3.m_Colour.b * pData->lensFlares.m_Colour.b;
	m_FlareColours[3].w = 1.0f;
}

/// construct by lerping two other PostProcessEnvSettings
PostFxEnvSettings::PostFxEnvSettings(const PostFxEnvSettings *pA, const PostFxEnvSettings *pB, float f)
	: m_LookupTexture(pA->m_LookupTexture, pB->m_LookupTexture, f)
	, m_FilmGrainIntensity(pA->m_FilmGrainIntensity, pB->m_FilmGrainIntensity, f)
	, m_FilmGrainLuminance(pA->m_FilmGrainLuminance, pB->m_FilmGrainLuminance, f)
	, m_VignetteWeight(pA->m_VignetteWeight, pB->m_VignetteWeight, f)
	, m_TunnelAmount(pA->m_TunnelAmount, pB->m_TunnelAmount, f)
	, m_BloomFeedback(pA->m_BloomFeedback, pB->m_BloomFeedback, f)
	, m_ColourMatrix(Maths::MakeVF32(1 - f) * pA->m_ColourMatrix + Maths::MakeVF32(f) * pB->m_ColourMatrix)
	, m_BlurType((f < 0.5f) ? pA->m_BlurType : pB->m_BlurType)
	, m_BloomThresholdCurve((f < 0.5f) ? pA->m_BloomThresholdCurve : pB->m_BloomThresholdCurve)
	, m_UID((f < 0.5f) ? pA->m_UID : pB->m_UID)
{
	for (S32 i = 0; i < 4; ++i)
	{
		m_FlareScales[i].x = (1.0f - f) * pA->m_FlareScales[i].x + f * pB->m_FlareScales[i].x;
		m_FlareScales[i].y = (1.0f - f) * pA->m_FlareScales[i].y + f * pB->m_FlareScales[i].y;
		m_FlareColours[i].x = (1.0f - f) * pA->m_FlareColours[i].x + f * pB->m_FlareColours[i].x;
		m_FlareColours[i].y = (1.0f - f) * pA->m_FlareColours[i].y + f * pB->m_FlareColours[i].y;
		m_FlareColours[i].z = (1.0f - f) * pA->m_FlareColours[i].z + f * pB->m_FlareColours[i].z;
	}
}

void PostFxEnvSettings::Set()
{
	HDR::g_mColour				= m_ColourMatrix;
	HDR::g_fFilmGrainIntensity	= m_FilmGrainIntensity;
	HDR::g_fFilmGrainLuminance	= m_FilmGrainLuminance;
	HDR::g_fVignetteWeight		= m_VignetteWeight;
	HDR::g_fTunnelAmount		= m_TunnelAmount;
	HDR::g_ColourSep			= m_ColourSep;
	HDR::g_Blockiness			= m_Blockiness;
	HDR::g_ScreenOverlay		= m_ScreenOverlay;
	
	for (S32 i = 0; i < 4; ++i)
	{
		HDR::g_vFlareScales[i]	= m_FlareScales[i];
		HDR::g_vFlareColours[i]	= m_FlareColours[i];
	}

	BIZ_ASSERT(HighDynamicRangeExists());

	if (HighDynamicRangeExists())
	{
		if(GetHighDynamicRange()->GetLookupTextureName() != Util::C_String(m_LookupTexture))
		{
			GetHighDynamicRange()->SetLookupTextureName(m_LookupTexture);
			GetHighDynamicRange()->ReloadLookupTexture();
		}

		GetHighDynamicRange()->ReloadBloomCurve(m_BloomThresholdCurve, m_UID);

		PostProcess::C_BloomFeedbackGhost* pBloom = (PostProcess::C_BloomFeedbackGhost*)GetHighDynamicRange()->GetBloom();
		pBloom->SetFeedbackFade(m_BloomFeedback);
	}
}

FX::I_EnvironmentSettings* PostFxEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new PostFxEnvSettings(this, (PostFxEnvSettings*)p, f);
}

void PostFxEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_FilmGrainIntensity.AssociateDebugVariables(sName + ".HDR.FilmGrainIntensity");
	m_FilmGrainLuminance.AssociateDebugVariables(sName + ".HDR.FilmGrainLuminance");
	m_VignetteWeight.AssociateDebugVariables(sName + ".HDR.VignetteWeight");
	m_TunnelAmount.AssociateDebugVariables(sName + ".HDR.TunnelAmount");
	m_BloomFeedback.AssociateDebugVariables(sName + ".HDR.BloomFeedback");
}

} // namespace

