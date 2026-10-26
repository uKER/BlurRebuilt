#include "stdafx.h"
#include <Graphics/CarLightSettings.h>

#include <Fx/EnvironmentSettings.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View/CubeMap.h>

XT_TYPE(Game::CarLightSettings, "CarLightSettings")
	XT_FIELD(fSHLightFactor,			"SHLightFactor")
	XT_FIELD(fSHLightMax,				"SHLightMax")
	XT_FIELD(fStaticLightCubemapFactor,	"StaticLightCubemapFactor")
	XT_FIELD(fDynamicLightCubemapFactor,"DynamicLightCubemapFactor")
	XT_FIELD(fLightCubemapMax,			"LightCubemapMax")
	XT_FIELD(FakeLightColour1,			"FakeLightColour1")
	XT_FIELD(fFakeLightSize1,			"FakeLightSize1")
	XT_FIELD(FakeLightPosition1,		"FakeLightPosition1")
	XT_FIELD(FakeLightColour2,			"FakeLightColour2")
	XT_FIELD(fFakeLightSize2,			"FakeLightSize2")
	XT_FIELD(FakeLightPosition2,		"FakeLightPosition2")
	XT_FIELD(FakeLightColour3,			"FakeLightColour3")
	XT_FIELD(fFakeLightSize3,			"FakeLightSize3")
	XT_FIELD(FakeLightPosition3,		"FakeLightPosition3")
	XT_FIELD(fFakeSunBlobShadowIntensity,"FakeSunBlobShadowIntensity")
	XT_FIELD(FakeSunBlobShadowPosition,	"FakeSunBlobShadowPosition")
XT_END_TYPE

namespace Game
{

/// constructor
CarLightEnvSettings::CarLightEnvSettings(CarLightSettings* pData)
	: m_fSHLightFactor(pData->fSHLightFactor),
	  m_fSHLightMax(pData->fSHLightMax),
	  m_fStaticLightCubemapFactor(pData->fStaticLightCubemapFactor),
	  m_fDynamicLightCubemapFactor(pData->fDynamicLightCubemapFactor),
	  m_fLightCubemapMax(pData->fLightCubemapMax),
	  m_FakeLightColour1(Maths::HVec4(pData->FakeLightColour1.r, pData->FakeLightColour1.g, pData->FakeLightColour1.b, 1.f)),
	  m_fFakeLightSize1(pData->fFakeLightSize1),
	  m_fFakeLightPositionX1(pData->FakeLightPosition1.vx),
	  m_fFakeLightPositionY1(pData->FakeLightPosition1.vy),
      m_fFakeLightPositionZ1(pData->FakeLightPosition1.vz),
	  m_FakeLightColour2(Maths::HVec4(pData->FakeLightColour2.r, pData->FakeLightColour2.g, pData->FakeLightColour2.b, 1.f)),
	  m_fFakeLightSize2(pData->fFakeLightSize2),
	  m_fFakeLightPositionX2(pData->FakeLightPosition2.vx),
	  m_fFakeLightPositionY2(pData->FakeLightPosition2.vy),
	  m_fFakeLightPositionZ2(pData->FakeLightPosition2.vz),
	  m_FakeLightColour3(Maths::HVec4(pData->FakeLightColour3.r, pData->FakeLightColour3.g, pData->FakeLightColour3.b, 1.f)),
	  m_fFakeLightSize3(pData->fFakeLightSize3),
	  m_fFakeLightPositionX3(pData->FakeLightPosition3.vx),
	  m_fFakeLightPositionY3(pData->FakeLightPosition3.vy),
	  m_fFakeLightPositionZ3(pData->FakeLightPosition3.vz),
	  m_fFakeSunBlobShadowIntensity(pData->fFakeSunBlobShadowIntensity),
	  m_fFakeSunBlobShadowPositionX(pData->FakeSunBlobShadowPosition.vx),
	  m_fFakeSunBlobShadowPositionY(pData->FakeSunBlobShadowPosition.vy),
	  m_fFakeSunBlobShadowPositionZ(pData->FakeSunBlobShadowPosition.vz)
{
}

/// construct by lerping two other ShadowEnvSettings
CarLightEnvSettings::CarLightEnvSettings(const CarLightEnvSettings *pA, const CarLightEnvSettings *pB, float f)
	: m_fSHLightFactor(pA->m_fSHLightFactor, pB->m_fSHLightFactor, f)
	, m_fSHLightMax(pA->m_fSHLightMax, pB->m_fSHLightMax, f)
	, m_fStaticLightCubemapFactor(pA->m_fStaticLightCubemapFactor, pB->m_fStaticLightCubemapFactor, f)
	, m_fDynamicLightCubemapFactor(pA->m_fDynamicLightCubemapFactor, pB->m_fDynamicLightCubemapFactor, f)
	, m_fLightCubemapMax(pA->m_fLightCubemapMax, pB->m_fLightCubemapMax, f)
	, m_FakeLightColour1(pA->m_FakeLightColour1, pB->m_FakeLightColour1, f)
	, m_fFakeLightSize1(pA->m_fFakeLightSize1, pB->m_fFakeLightSize1, f)
	, m_fFakeLightPositionX1(pA->m_fFakeLightPositionX1, pB->m_fFakeLightPositionX1, f)
	, m_fFakeLightPositionY1(pA->m_fFakeLightPositionY1, pB->m_fFakeLightPositionY1, f)
	, m_fFakeLightPositionZ1(pA->m_fFakeLightPositionZ1, pB->m_fFakeLightPositionZ1, f)
	, m_FakeLightColour2(pA->m_FakeLightColour2, pB->m_FakeLightColour2, f)
	, m_fFakeLightSize2(pA->m_fFakeLightSize2, pB->m_fFakeLightSize2, f)
	, m_fFakeLightPositionX2(pA->m_fFakeLightPositionX2, pB->m_fFakeLightPositionX2, f)
	, m_fFakeLightPositionY2(pA->m_fFakeLightPositionY2, pB->m_fFakeLightPositionY2, f)
	, m_fFakeLightPositionZ2(pA->m_fFakeLightPositionZ2, pB->m_fFakeLightPositionZ2, f)
	, m_FakeLightColour3(pA->m_FakeLightColour3, pB->m_FakeLightColour3, f)
	, m_fFakeLightSize3(pA->m_fFakeLightSize3, pB->m_fFakeLightSize3, f)
	, m_fFakeLightPositionX3(pA->m_fFakeLightPositionX3, pB->m_fFakeLightPositionX3, f)
	, m_fFakeLightPositionY3(pA->m_fFakeLightPositionY3, pB->m_fFakeLightPositionY3, f)
	, m_fFakeLightPositionZ3(pA->m_fFakeLightPositionZ3, pB->m_fFakeLightPositionZ3, f)
	, m_fFakeSunBlobShadowIntensity(pA->m_fFakeSunBlobShadowIntensity, pB->m_fFakeSunBlobShadowIntensity, f)
	, m_fFakeSunBlobShadowPositionX(pA->m_fFakeSunBlobShadowPositionX, pB->m_fFakeSunBlobShadowPositionX, f)
	, m_fFakeSunBlobShadowPositionY(pA->m_fFakeSunBlobShadowPositionY, pB->m_fFakeSunBlobShadowPositionY, f)
	, m_fFakeSunBlobShadowPositionZ(pA->m_fFakeSunBlobShadowPositionZ, pB->m_fFakeSunBlobShadowPositionZ, f)
{
}

void CarLightEnvSettings::Set()
{
	C_GameWorld *p = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
	if (p)
	{
		C_CarLightSettings carLightSettings;
		carLightSettings.m_SHLightFactor = m_fSHLightFactor;
		carLightSettings.m_SHLightMax = m_fSHLightMax;
		carLightSettings.m_StaticLightCubemapFactor = m_fStaticLightCubemapFactor;
		carLightSettings.m_DynamicLightCubemapFactor = m_fDynamicLightCubemapFactor;
		carLightSettings.m_LightCubemapMax = m_fLightCubemapMax;
		carLightSettings.m_SunBlobShadowPosition = Maths::HVec3(F32(m_fFakeSunBlobShadowPositionX), F32(m_fFakeSunBlobShadowPositionY), F32(m_fFakeSunBlobShadowPositionZ));
		carLightSettings.m_SunBlobShadowIntensity = m_fFakeSunBlobShadowIntensity;
		carLightSettings.m_SunFakeLightColour = m_FakeLightColour3;
		carLightSettings.m_SunFakeLightPosition = Maths::HVec3(F32(m_fFakeLightPositionX3), F32(m_fFakeLightPositionY3), F32(m_fFakeLightPositionZ3));
		carLightSettings.m_SunFakeLightSize = m_fFakeLightSize3;
		p->SetCarLightSettings(carLightSettings);
		
		C_CubeMapView* pCubeMapView = p->GetViewManager()->GetCubeMap();
		if (pCubeMapView)
		{
			pCubeMapView->UpdateFakeLight(0, m_FakeLightColour1, Maths::HVec3(F32(m_fFakeLightPositionX1), F32(m_fFakeLightPositionY1), F32(m_fFakeLightPositionZ1)), m_fFakeLightSize1);
			pCubeMapView->UpdateFakeLight(1, m_FakeLightColour2, Maths::HVec3(F32(m_fFakeLightPositionX2), F32(m_fFakeLightPositionY2), F32(m_fFakeLightPositionZ2)), m_fFakeLightSize2);
			pCubeMapView->UpdateFakeLight(2, m_FakeLightColour3, Maths::HVec3(F32(m_fFakeLightPositionX3), F32(m_fFakeLightPositionY3), F32(m_fFakeLightPositionZ3)), m_fFakeLightSize3);
		}
	}
}

FX::I_EnvironmentSettings* CarLightEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new CarLightEnvSettings(this, (CarLightEnvSettings*)p, f);
}

void CarLightEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_fSHLightFactor.AssociateDebugVariables(sName + ".CarLight.SHLightFactor");
	m_fSHLightMax.AssociateDebugVariables(sName + ".CarLight.SHLightMax");
	m_fStaticLightCubemapFactor.AssociateDebugVariables(sName + ".CarLight.StaticLightCubemapFactor");
	m_fDynamicLightCubemapFactor.AssociateDebugVariables(sName + ".CarLight.DynamicLightCubemapFactor");
	m_fLightCubemapMax.AssociateDebugVariables(sName + ".CarLight.LightCubemapMax");

	m_FakeLightColour1.AssociateDebugVariables(sName + ".CarLight.FakeLightColour1");
	m_fFakeLightSize1.AssociateDebugVariables(sName + ".CarLight.FakeLightSize1");
	m_fFakeLightPositionX1.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionX1");
	m_fFakeLightPositionY1.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionY1");
	m_fFakeLightPositionZ1.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionZ1");

	m_FakeLightColour2.AssociateDebugVariables(sName + ".CarLight.FakeLightColour2");
	m_fFakeLightSize2.AssociateDebugVariables(sName + ".CarLight.FakeLightSize2");
	m_fFakeLightPositionX2.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionX2");
	m_fFakeLightPositionY2.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionY2");
	m_fFakeLightPositionZ2.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionZ2");

	m_FakeLightColour3.AssociateDebugVariables(sName + ".CarLight.FakeLightColour3");
	m_fFakeLightSize3.AssociateDebugVariables(sName + ".CarLight.FakeLightSize3");
	m_fFakeLightPositionX3.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionX3");
	m_fFakeLightPositionY3.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionY3");
	m_fFakeLightPositionZ3.AssociateDebugVariables(sName + ".CarLight.FakeLightPositionZ3");

	m_fFakeSunBlobShadowIntensity.AssociateDebugVariables(sName + ".CarLight.FakeSunBlobShadowIntensity");
	m_fFakeSunBlobShadowPositionX.AssociateDebugVariables(sName + ".CarLight.FakeSunBlobShadowPositionX");
	m_fFakeSunBlobShadowPositionY.AssociateDebugVariables(sName + ".CarLight.FakeSunBlobShadowPositionY");
	m_fFakeSunBlobShadowPositionZ.AssociateDebugVariables(sName + ".CarLight.FakeSunBlobShadowPositionZ");

}

} // namespace

