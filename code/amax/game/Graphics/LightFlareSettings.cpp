#include "stdafx.h"
#include <Graphics/LightFlareSettings.h>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>

XT_TYPE(Game::LightFlareSettings, "LightFlareSettings")
	XT_FIELD(fGlobalSize,	"GlobalSize")
	XT_FIELD(fGlobalSizeX,	"GlobalSizeX")
	XT_FIELD(fGlobalSizeY,	"GlobalSizeY")
	XT_FIELD(vGlobalColour,	"GlobalColour")
	XT_FIELD(fReflectionMinSizeX,	"ReflectionMinSizeX")
	XT_FIELD(fReflectionMinSizeY,	"ReflectionMinSizeY")
	XT_FIELD(fReflectionMaxSizeX,	"ReflectionMaxSizeX")
	XT_FIELD(fReflectionMaxSizeY,	"ReflectionMaxSizeY")
	XT_FIELD(vReflectionColour,	"ReflectionColour")
	XT_FIELD(fEnvironmentSizeX,	"EnvironmentSizeX")
	XT_FIELD(fEnvironmentSizeY,	"EnvironmentSizeY")
	XT_FIELD(vEnvironmentColour,	"EnvironmentColour")
	XT_FIELD(fNearFadeDistant,	"NearFadeDistant")
	XT_FIELD(fFarFadeDistant,	"FarFadeDistant")	
	XT_FIELD(fEnvironmentNearFadeDistant,	"EnvironmentNearFadeDistant")
	XT_FIELD(fEnvironmentFarFadeDistant,	"EnvironmentFarFadeDistant")	
	XT_FIELD(fSizeDistantFactor,"SizeDistantFactor")	
	XT_FIELD(fIntensityDistantFactor,"IntensityDistantFactor")	
	XT_FIELD(fCarGlobalSize,	"CarGlobalSize")
	XT_FIELD(fCarGlobalSizeX,	"CarGlobalSizeX")
	XT_FIELD(fCarGlobalSizeY,	"CarGlobalSizeY")
	XT_FIELD(vCarGlobalColour,	"CarGlobalColour")
	XT_FIELD(fCarReflectionMinSizeX,	"CarReflectionMinSizeX")
	XT_FIELD(fCarReflectionMinSizeY,	"CarReflectionMinSizeY")
	XT_FIELD(fCarReflectionMaxSizeX,	"CarReflectionMaxSizeX")
	XT_FIELD(fCarReflectionMaxSizeY,	"CarReflectionMaxSizeY")
	XT_FIELD(vCarReflectionColour,	"CarReflectionColour")
	XT_FIELD(fCarEnvironmentSizeX,	"CarEnvironmentSizeX")
	XT_FIELD(fCarEnvironmentSizeY,	"CarEnvironmentSizeY")
	XT_FIELD(vCarWhiteFlaresColour,	"CarWhiteFlaresColour")
	XT_FIELD(vCarRedFlaresColour,	"CarRedFlaresColour")
XT_END_TYPE

namespace Game
{

	extern GameWorldFxParams	g_FxParams;

/// constructor
LightFlareEnvSettings::LightFlareEnvSettings(LightFlareSettings* pData)
	: m_fGlobalSize(pData->fGlobalSize)
	, m_fGlobalSizeX(pData->fGlobalSizeX)
	, m_fGlobalSizeY(pData->fGlobalSizeY)
	, m_vGlobalColour(Maths::HVec4(pData->vGlobalColour.r, pData->vGlobalColour.g, pData->vGlobalColour.b, 1.0f))
	, m_fReflectionMinSizeX(pData->fReflectionMinSizeX)
	, m_fReflectionMinSizeY(pData->fReflectionMinSizeY)
	, m_fReflectionMaxSizeX(pData->fReflectionMaxSizeX)
	, m_fReflectionMaxSizeY(pData->fReflectionMaxSizeY)
	, m_vReflectionColour(Maths::HVec4(pData->vReflectionColour.r, pData->vReflectionColour.g, pData->vReflectionColour.b, 1.0f))
	, m_fEnvironmentSizeX(pData->fEnvironmentSizeX)
	, m_fEnvironmentSizeY(pData->fEnvironmentSizeY)
	, m_vEnvironmentColour(Maths::HVec4(pData->vEnvironmentColour.r, pData->vEnvironmentColour.g, pData->vEnvironmentColour.b, 1.0f))
	, m_fNearFadeDistant(pData->fNearFadeDistant)
	, m_fFarFadeDistant(pData->fFarFadeDistant)
	, m_fEnvironmentNearFadeDistant(pData->fEnvironmentNearFadeDistant)
	, m_fEnvironmentFarFadeDistant(pData->fEnvironmentFarFadeDistant)
	, m_fSizeDistantFactor(pData->fSizeDistantFactor)
	, m_fIntensityDistantFactor(pData->fIntensityDistantFactor)
	, m_fCarGlobalSize(pData->fCarGlobalSize)
	, m_fCarGlobalSizeX(pData->fCarGlobalSizeX)
	, m_fCarGlobalSizeY(pData->fCarGlobalSizeY)
	, m_vCarGlobalColour(Maths::HVec4(pData->vCarGlobalColour.r, pData->vCarGlobalColour.g, pData->vCarGlobalColour.b, 1.0f))
	, m_fCarReflectionMinSizeX(pData->fCarReflectionMinSizeX)
	, m_fCarReflectionMinSizeY(pData->fCarReflectionMinSizeY)
	, m_fCarReflectionMaxSizeX(pData->fCarReflectionMaxSizeX)
	, m_fCarReflectionMaxSizeY(pData->fCarReflectionMaxSizeY)
	, m_vCarReflectionColour(Maths::HVec4(pData->vCarReflectionColour.r, pData->vCarReflectionColour.g, pData->vCarReflectionColour.b, 1.0f))
	, m_fCarEnvironmentSizeX(pData->fCarEnvironmentSizeX)
	, m_fCarEnvironmentSizeY(pData->fCarEnvironmentSizeY)
	, m_vCarWhiteFlaresColour(Maths::HVec4(pData->vCarWhiteFlaresColour.r, pData->vCarWhiteFlaresColour.g, pData->vCarWhiteFlaresColour.b, 1.0f))
	, m_vCarRedFlaresColour(Maths::HVec4(pData->vCarRedFlaresColour.r, pData->vCarRedFlaresColour.g, pData->vCarRedFlaresColour.b, 1.0f))
{
}

/// construct by lerping two other PostProcessEnvSettings
LightFlareEnvSettings::LightFlareEnvSettings(const LightFlareEnvSettings *pA, const LightFlareEnvSettings *pB, float f)
	: m_fGlobalSize(pA->m_fGlobalSize, pB->m_fGlobalSize, f)
	, m_fGlobalSizeX(pA->m_fGlobalSizeX, pB->m_fGlobalSizeX, f)
	, m_fGlobalSizeY(pA->m_fGlobalSizeY, pB->m_fGlobalSizeY, f)
	, m_vGlobalColour(pA->m_vGlobalColour, pB->m_vGlobalColour, f)
	, m_fReflectionMinSizeX(pA->m_fReflectionMinSizeX, pB->m_fReflectionMinSizeX, f)
	, m_fReflectionMinSizeY(pA->m_fReflectionMinSizeY, pB->m_fReflectionMinSizeY, f)
	, m_fReflectionMaxSizeX(pA->m_fReflectionMaxSizeX, pB->m_fReflectionMaxSizeX, f)
	, m_fReflectionMaxSizeY(pA->m_fReflectionMaxSizeY, pB->m_fReflectionMaxSizeY, f)
	, m_vReflectionColour(pA->m_vReflectionColour, pB->m_vReflectionColour, f)
	, m_fEnvironmentSizeX(pA->m_fEnvironmentSizeX, pB->m_fEnvironmentSizeX, f)
	, m_fEnvironmentSizeY(pA->m_fEnvironmentSizeY, pB->m_fEnvironmentSizeY, f)
	, m_vEnvironmentColour(pA->m_vEnvironmentColour, pB->m_vEnvironmentColour, f)
	, m_fNearFadeDistant(pA->m_fNearFadeDistant, pB->m_fNearFadeDistant, f)
	, m_fFarFadeDistant(pA->m_fFarFadeDistant, pB->m_fFarFadeDistant, f)
	, m_fEnvironmentNearFadeDistant(pA->m_fEnvironmentNearFadeDistant, pB->m_fEnvironmentNearFadeDistant, f)
	, m_fEnvironmentFarFadeDistant(pA->m_fEnvironmentFarFadeDistant, pB->m_fEnvironmentFarFadeDistant, f)
	, m_fSizeDistantFactor(pA->m_fSizeDistantFactor, pB->m_fSizeDistantFactor, f)
	, m_fIntensityDistantFactor(pA->m_fIntensityDistantFactor, pB->m_fIntensityDistantFactor, f)
	, m_fCarGlobalSize(pA->m_fCarGlobalSize, pB->m_fCarGlobalSize, f)
	, m_fCarGlobalSizeX(pA->m_fCarGlobalSizeX, pB->m_fCarGlobalSizeX, f)
	, m_fCarGlobalSizeY(pA->m_fCarGlobalSizeY, pB->m_fCarGlobalSizeY, f)
	, m_vCarGlobalColour(pA->m_vCarGlobalColour, pB->m_vCarGlobalColour, f)
	, m_fCarReflectionMinSizeX(pA->m_fCarReflectionMinSizeX, pB->m_fCarReflectionMinSizeX, f)
	, m_fCarReflectionMinSizeY(pA->m_fCarReflectionMinSizeY, pB->m_fCarReflectionMinSizeY, f)
	, m_fCarReflectionMaxSizeX(pA->m_fCarReflectionMaxSizeX, pB->m_fCarReflectionMaxSizeX, f)
	, m_fCarReflectionMaxSizeY(pA->m_fCarReflectionMaxSizeY, pB->m_fCarReflectionMaxSizeY, f)
	, m_vCarReflectionColour(pA->m_vCarReflectionColour, pB->m_vCarReflectionColour, f)
	, m_fCarEnvironmentSizeX(pA->m_fCarEnvironmentSizeX, pB->m_fCarEnvironmentSizeX, f)
	, m_fCarEnvironmentSizeY(pA->m_fCarEnvironmentSizeY, pB->m_fCarEnvironmentSizeY, f)
	, m_vCarWhiteFlaresColour(pA->m_vCarWhiteFlaresColour, pB->m_vCarWhiteFlaresColour, f)
	, m_vCarRedFlaresColour(pA->m_vCarRedFlaresColour, pB->m_vCarRedFlaresColour, f)
{
}

void LightFlareEnvSettings::Set()
{
	
	Game::C_AmaxWorldLayer * pAmaxWorldLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer();
	if (pAmaxWorldLayer)
	{
		Game::C_LightFlareSettings lightFlareSettings;
		lightFlareSettings.m_GlobalSize = m_fGlobalSize;
		lightFlareSettings.m_GlobalSizeX = m_fGlobalSizeX;
		lightFlareSettings.m_GlobalSizeY = m_fGlobalSizeY;
		lightFlareSettings.m_GlobalColour = m_vGlobalColour;
		lightFlareSettings.m_ReflectionMinSizeX = m_fReflectionMinSizeX;
		lightFlareSettings.m_ReflectionMinSizeY = m_fReflectionMinSizeY;
		lightFlareSettings.m_ReflectionMaxSizeX = m_fReflectionMaxSizeX;
		lightFlareSettings.m_ReflectionMaxSizeY = m_fReflectionMaxSizeY;
		lightFlareSettings.m_ReflectionColour = m_vReflectionColour;
		lightFlareSettings.m_EnvironmentSizeX = m_fEnvironmentSizeX;
		lightFlareSettings.m_EnvironmentSizeY = m_fEnvironmentSizeY;
		lightFlareSettings.m_EnvironmentColour = m_vEnvironmentColour;
		lightFlareSettings.m_NearFadeDistant = m_fNearFadeDistant;
		lightFlareSettings.m_FarFadeDistant = m_fFarFadeDistant;
		lightFlareSettings.m_EnvironmentNearFadeDistant = m_fEnvironmentNearFadeDistant;
		lightFlareSettings.m_EnvironmentFarFadeDistant = m_fEnvironmentFarFadeDistant;
		lightFlareSettings.m_SizeDistantFactor = m_fSizeDistantFactor;
		lightFlareSettings.m_IntensityDistantFactor = m_fIntensityDistantFactor;

	
		pAmaxWorldLayer->GetGameWorld()->SetLevelLightFlareParams(lightFlareSettings);

		Game::C_CarLightFlareSettings carLightFlareSettings;

		carLightFlareSettings.m_GlobalSize = m_fCarGlobalSize;
		carLightFlareSettings.m_GlobalSizeX = m_fCarGlobalSizeX;
		carLightFlareSettings.m_GlobalSizeY = m_fCarGlobalSizeY;
		carLightFlareSettings.m_GlobalColour = m_vCarGlobalColour;
		carLightFlareSettings.m_ReflectionMinSizeX = m_fCarReflectionMinSizeX;
		carLightFlareSettings.m_ReflectionMinSizeY = m_fCarReflectionMinSizeY;
		carLightFlareSettings.m_ReflectionMaxSizeX = m_fCarReflectionMaxSizeX;
		carLightFlareSettings.m_ReflectionMaxSizeY = m_fCarReflectionMaxSizeY;
		carLightFlareSettings.m_ReflectionColour = m_vCarReflectionColour;
		carLightFlareSettings.m_EnvironmentSizeX = m_fCarEnvironmentSizeX;
		carLightFlareSettings.m_EnvironmentSizeY = m_fCarEnvironmentSizeY;
		carLightFlareSettings.m_CarWhiteFlaresColour = m_vCarWhiteFlaresColour;
		carLightFlareSettings.m_CarRedFlaresColour = m_vCarRedFlaresColour;

		pAmaxWorldLayer->GetGameWorld()->SetCarLightFlareParams(carLightFlareSettings);
	}
}

FX::I_EnvironmentSettings* LightFlareEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new LightFlareEnvSettings(this, (LightFlareEnvSettings*)p, f);
}

void LightFlareEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_fGlobalSize.AssociateDebugVariables(sName + ".LightFlare.GlobalSize");
	m_fGlobalSizeX.AssociateDebugVariables(sName + ".LightFlare.GlobalSizeX");
	m_fGlobalSizeY.AssociateDebugVariables(sName + ".LightFlare.GlobalSizeY");
	m_vGlobalColour.AssociateDebugVariables(sName + ".LightFlare.GlobalColour");
	m_fReflectionMinSizeX.AssociateDebugVariables(sName + ".LightFlare.ReflectionMinSizeX");
	m_fReflectionMinSizeY.AssociateDebugVariables(sName + ".LightFlare.ReflectionMinSizeY");
	m_fReflectionMaxSizeX.AssociateDebugVariables(sName + ".LightFlare.ReflectionMaxSizeX");
	m_fReflectionMaxSizeY.AssociateDebugVariables(sName + ".LightFlare.ReflectionMaxSizeY");
	m_vReflectionColour.AssociateDebugVariables(sName + ".LightFlare.ReflectionColour");
	m_fEnvironmentSizeX.AssociateDebugVariables(sName + ".LightFlare.EnvironmentSizeX");
	m_fEnvironmentSizeY.AssociateDebugVariables(sName + ".LightFlare.EnvironmentSizeY");
	m_vEnvironmentColour.AssociateDebugVariables(sName + ".LightFlare.EnvironmentColour");
	m_fNearFadeDistant.AssociateDebugVariables(sName + ".LightFlare.NearFadeDistant");
	m_fFarFadeDistant.AssociateDebugVariables(sName + ".LightFlare.FarFadeDistant");
	m_fEnvironmentNearFadeDistant.AssociateDebugVariables(sName + ".LightFlare.EnvironmentNearFadeDistant");
	m_fEnvironmentFarFadeDistant.AssociateDebugVariables(sName + ".LightFlare.EnvironmentFarFadeDistant");
	m_fSizeDistantFactor.AssociateDebugVariables(sName + ".LightFlare.SizeDistantFactor");
	m_fIntensityDistantFactor.AssociateDebugVariables(sName + ".LightFlare.IntensityDistantFactor");
	m_fCarGlobalSize.AssociateDebugVariables(sName + ".LightFlare.CarGlobalSize");
	m_fCarGlobalSizeX.AssociateDebugVariables(sName + ".LightFlare.CarGlobalSizeX");
	m_fCarGlobalSizeY.AssociateDebugVariables(sName + ".LightFlare.CarGlobalSizeY");
	m_vCarGlobalColour.AssociateDebugVariables(sName + ".LightFlare.CarGlobalColour");
	m_fCarReflectionMinSizeX.AssociateDebugVariables(sName + ".LightFlare.CarReflectionMinSizeX");
	m_fCarReflectionMinSizeY.AssociateDebugVariables(sName + ".LightFlare.CarReflectionMinSizeY");
	m_fCarReflectionMaxSizeX.AssociateDebugVariables(sName + ".LightFlare.CarReflectionMaxSizeX");
	m_fCarReflectionMaxSizeY.AssociateDebugVariables(sName + ".LightFlare.CarReflectionMaxSizeY");
	m_vCarReflectionColour.AssociateDebugVariables(sName + ".LightFlare.CarReflectionColour");
	m_fCarEnvironmentSizeX.AssociateDebugVariables(sName + ".LightFlare.CarEnvironmentSizeX");
	m_fCarEnvironmentSizeY.AssociateDebugVariables(sName + ".LightFlare.CarEnvironmentSizeY");
	m_vCarWhiteFlaresColour.AssociateDebugVariables(sName + ".LightFlare.CarWhiteFlaresColour");
	m_vCarRedFlaresColour.AssociateDebugVariables(sName + ".LightFlare.CarRedFlaresColour");
}

}; // namespace Game
