#include "stdafx.h"
#include <Graphics/RoadTrackSettings.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View.h>
#include <Game/GameWorld/View/Reflection.h>

XT_TYPE(Game::RoadTrackSettings, "RoadTrackSettings")
	XT_FIELD(fTrackReflectionBrightness,	"TrackReflectionBrightness")
	XT_FIELD(fTrackReflectionContrast,		"TrackReflectionContrast")
	XT_FIELD(fReflectionBlurX,				"ReflectionBlurX")
	XT_FIELD(fReflectionBlurY,				"ReflectionBlurY")
	XT_FIELD(fReflectionBlurDesviation,		"ReflectionBlurDesviation")
	XT_FIELD(fReflectionBrightness,			"ReflectionBrightness")
	XT_FIELD(fCubeMapSmoothness,			"CubeMapSmoothness")
	XT_FIELD(fTrackReflectionScale,			"TrackReflectionScale")
	XT_FIELD(fWaterReflectionScale,			"WaterReflectionScale")
	XT_FIELD(fTrackReflectionOffset,		"TrackReflectionOffset")
	XT_FIELD(BlackBorderReflection,			"BlackBorderReflection")
	XT_FIELD(FakeSunLightColour,			"FakeSunLightColour")
	XT_FIELD(fFakeSunLightMinSize,			"FakeSunLightMinSize")
	XT_FIELD(fFakeSunLightMaxSize,			"FakeSunLightMaxSize")
	XT_FIELD(FakeSunLightPosition,			"FakeSunLightPosition")
	XT_FIELD(FakeSunLightDepthTest,			"FakeSunLightDepthTest")
XT_END_TYPE

namespace Game
{

struct RoadTrackFxParams
{
	FX::ParameterHandle		roadReflectionBrightness;
	FX::ParameterHandle		roadReflectionContrast;
	FX::ParameterHandle		roadSurfaceSmoothness;
	FX::ParameterHandle		waterSurfaceSmoothness;
	FX::ParameterHandle		roadDistortionScale; 
	FX::ParameterHandle		waterDistortionScale;
	FX::ParameterHandle		roadDistortionOffset;
};

static RoadTrackFxParams g_TrackFxParams;

/// constructor
RoadTrackEnvSettings::RoadTrackEnvSettings(RoadTrackSettings* pData)
	: m_fTrackReflectionBrightness(pData->fTrackReflectionBrightness)
	, m_fTrackReflectionContrast(pData->fTrackReflectionContrast)
	, m_fReflectionBlurX(pData->fReflectionBlurX)
	, m_fReflectionBlurY(pData->fReflectionBlurY)
	, m_fReflectionBlurDesviation(pData->fReflectionBlurDesviation)
	, m_fReflectionBrightness(pData->fReflectionBrightness)
	, m_fCubeMapSmoothness(pData->fCubeMapSmoothness)
	, m_fTrackReflectionScale(pData->fTrackReflectionScale)
	, m_fWaterReflectionScale(pData->fWaterReflectionScale)
	, m_fTrackReflectionOffset(pData->fTrackReflectionOffset)
	, m_BlackBorderReflection(pData->BlackBorderReflection)
	, m_FakeSunLightColour(Maths::HVec4(pData->FakeSunLightColour.r, pData->FakeSunLightColour.g, pData->FakeSunLightColour.b, 1.f))
	, m_fFakeSunLightMinSize(pData->fFakeSunLightMinSize)
	, m_fFakeSunLightMaxSize(pData->fFakeSunLightMaxSize)
	, m_fFakeSunLightPositionX(pData->FakeSunLightPosition.vx)
	, m_fFakeSunLightPositionY(pData->FakeSunLightPosition.vy)
	, m_fFakeSunLightPositionZ(pData->FakeSunLightPosition.vz)
	, m_FakeSunLightDepthTest(pData->FakeSunLightDepthTest)
{
}

/// construct by lerping two other PostProcessEnvSettings
RoadTrackEnvSettings::RoadTrackEnvSettings(const RoadTrackEnvSettings *pA, const RoadTrackEnvSettings *pB, float f)
	: m_fTrackReflectionBrightness(pA->m_fTrackReflectionBrightness, pB->m_fTrackReflectionBrightness, f)
	, m_fTrackReflectionContrast(pA->m_fTrackReflectionContrast, pB->m_fTrackReflectionContrast, f)
	, m_fReflectionBlurX(pA->m_fReflectionBlurX, pB->m_fReflectionBlurX, f)
	, m_fReflectionBlurY(pA->m_fReflectionBlurY, pB->m_fReflectionBlurY, f)
	, m_fReflectionBlurDesviation(pA->m_fReflectionBlurDesviation, pB->m_fReflectionBlurDesviation, f)
	, m_fReflectionBrightness(pA->m_fReflectionBrightness, pB->m_fReflectionBrightness, f)
	, m_fCubeMapSmoothness(pA->m_fCubeMapSmoothness, pB->m_fCubeMapSmoothness, f)
	, m_fTrackReflectionScale(pA->m_fTrackReflectionScale, pB->m_fTrackReflectionScale, f)
	, m_fWaterReflectionScale(pA->m_fWaterReflectionScale, pB->m_fWaterReflectionScale, f)
	, m_fTrackReflectionOffset(pA->m_fTrackReflectionOffset, pB->m_fTrackReflectionOffset, f)
	, m_BlackBorderReflection((f < 0.5f)?pA->m_BlackBorderReflection:pB->m_BlackBorderReflection)
	, m_FakeSunLightColour(pA->m_FakeSunLightColour, pB->m_FakeSunLightColour, f)
	, m_fFakeSunLightMinSize(pA->m_fFakeSunLightMinSize, pB->m_fFakeSunLightMinSize, f)
	, m_fFakeSunLightMaxSize(pA->m_fFakeSunLightMaxSize, pB->m_fFakeSunLightMaxSize, f)
	, m_fFakeSunLightPositionX(pA->m_fFakeSunLightPositionX, pB->m_fFakeSunLightPositionX, f)
	, m_fFakeSunLightPositionY(pA->m_fFakeSunLightPositionY, pB->m_fFakeSunLightPositionY, f)
	, m_fFakeSunLightPositionZ(pA->m_fFakeSunLightPositionZ, pB->m_fFakeSunLightPositionZ, f)
	, m_FakeSunLightDepthTest((f < 0.5f)?pA->m_FakeSunLightDepthTest:pB->m_FakeSunLightDepthTest)
{
}

void RoadTrackEnvSettings::InitialiseParameters()
{
	g_TrackFxParams.roadReflectionBrightness = FX::GetSharedParameter("g_fRoadReflectionBrightness");
	g_TrackFxParams.roadReflectionContrast = FX::GetSharedParameter("g_fRoadReflectionContrast");
	g_TrackFxParams.roadSurfaceSmoothness = FX::GetSharedParameter("g_fRoadSurfaceSmoothness");
	g_TrackFxParams.waterSurfaceSmoothness = FX::GetSharedParameter("g_fWaterSurfaceSmoothness");
	g_TrackFxParams.roadDistortionScale = FX::GetSharedParameter("g_fRoadDistortionScale");
	g_TrackFxParams.waterDistortionScale = FX::GetSharedParameter("g_fWaterDistortionScale");
	g_TrackFxParams.roadDistortionOffset = FX::GetSharedParameter("g_fRoadDistortionOffset");
}

void RoadTrackEnvSettings::Set()
{
	FX::SetParameter(g_TrackFxParams.roadReflectionBrightness, Display::C_PSConstant(m_fTrackReflectionBrightness));
	FX::SetParameter(g_TrackFxParams.roadReflectionContrast, Display::C_PSConstant(m_fTrackReflectionContrast));
	FX::SetParameter(g_TrackFxParams.roadSurfaceSmoothness, Display::C_PSConstant(m_fCubeMapSmoothness));
	FX::SetParameter(g_TrackFxParams.waterSurfaceSmoothness, Display::C_PSConstant(m_fCubeMapSmoothness));
	FX::SetParameter(g_TrackFxParams.roadDistortionScale, Display::C_PSConstant(m_fTrackReflectionScale));
	FX::SetParameter(g_TrackFxParams.waterDistortionScale, Display::C_PSConstant(m_fWaterReflectionScale));
	FX::SetParameter(g_TrackFxParams.roadDistortionOffset, Display::C_PSConstant(m_fTrackReflectionOffset));

	C_GameWorld *p = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
	if (p)
	{
		p->SetReflectionBlur(m_fReflectionBlurX, m_fReflectionBlurY, m_fReflectionBlurDesviation);
		if (p->GetViewManager()->GetNumViews() == 1) //if not splitscreen
		{
			C_ReflectionView* pReflectionView = p->GetViewManager()->GetView(0)->GetReflection();
			if (pReflectionView)
			{
				pReflectionView->EnableBlackBorderReflection(m_BlackBorderReflection);
				pReflectionView->UpdateFakeSunLight( m_FakeSunLightColour, Maths::HVec3(F32(m_fFakeSunLightPositionX), F32(m_fFakeSunLightPositionY), F32(m_fFakeSunLightPositionZ)), m_fFakeSunLightMinSize, m_fFakeSunLightMaxSize, m_FakeSunLightDepthTest);
			}
		}
		
	}

}

FX::I_EnvironmentSettings* RoadTrackEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new RoadTrackEnvSettings(this, (RoadTrackEnvSettings*)p, f);
}

void RoadTrackEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_fTrackReflectionBrightness.AssociateDebugVariables(sName + ".RoadTrack.TrackReflectionBrightness");
	m_fTrackReflectionContrast.AssociateDebugVariables(sName + ".RoadTrack.TrackReflectionContrast");
	m_fReflectionBlurX.AssociateDebugVariables(sName + ".RoadTrack.ReflectionBlurX");
	m_fReflectionBlurY.AssociateDebugVariables(sName + ".RoadTrack.ReflectionBlurY");
	m_fReflectionBlurDesviation.AssociateDebugVariables(sName + ".RoadTrack.ReflectionBlurDesviation");
	m_fReflectionBrightness.AssociateDebugVariables(sName + ".RoadTrack.ReflectionBrightness");
	m_fCubeMapSmoothness.AssociateDebugVariables(sName + ".RoadTrack.CubeMapSmoothness");
	m_fTrackReflectionScale.AssociateDebugVariables(sName + ".RoadTrack.TrackReflectionScale");
	m_fWaterReflectionScale.AssociateDebugVariables(sName + ".RoadTrack.WaterReflectionScale");
	m_fTrackReflectionOffset.AssociateDebugVariables(sName + ".RoadTrack.TrackReflectionOffset");
	m_FakeSunLightColour.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightColour");
	m_fFakeSunLightMinSize.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightMinSize");
	m_fFakeSunLightMaxSize.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightMaxSize");
	m_fFakeSunLightPositionX.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightPositionX");
	m_fFakeSunLightPositionY.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightPositionY");
	m_fFakeSunLightPositionZ.AssociateDebugVariables(sName + ".RoadTrack.FakeSunLightPositionZ");

}

} // namespace

