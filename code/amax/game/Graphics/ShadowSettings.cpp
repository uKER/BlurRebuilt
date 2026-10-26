#include "stdafx.h"
#include <Graphics/ShadowSettings.h>

#include <Fx/EnvironmentSettings.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <BlobShadows/blobshadows.h>

XT_TYPE(Game::ShadowSettings, "ShadowSettings")
	XT_FIELD(fShadowSize,	"ShadowSize")
	XT_FIELD(fShadowHeight,	"ShadowHeight")
	XT_FIELD(fDropShadowIntensity,	"DropShadowIntensity")
	XT_FIELD(fDropShadowHeight,		"DropShadowHeight")
	XT_FIELD(fIndividualShadowMaxIntensity,				"IndividualShadowMaxIntensity")
	XT_FIELD(fMultiShadowIntensity,				"MultiShadowIntensity")
	XT_FIELD(fFadeRange,		"FadeRange")
	XT_FIELD(fMaxShadows,			"MaxShadows")
XT_END_TYPE

namespace Game
{

/// constructor
ShadowEnvSettings::ShadowEnvSettings(ShadowSettings* pData)
	: m_fShadowSize(pData->fShadowSize)
	, m_fShadowHeight(pData->fShadowHeight)
	, m_fDropShadowIntensity(pData->fDropShadowIntensity)
	, m_fDropShadowHeight(pData->fDropShadowHeight)
	, m_fIndividualShadowMaxIntensity(pData->fIndividualShadowMaxIntensity)
	, m_fMultiShadowIntensity(pData->fMultiShadowIntensity)
	, m_fFadeRange(pData->fFadeRange)
	, m_fMaxShadows(pData->fMaxShadows)
{
}

/// construct by lerping two other ShadowEnvSettings
ShadowEnvSettings::ShadowEnvSettings(const ShadowEnvSettings *pA, const ShadowEnvSettings *pB, float f)
	: m_fShadowSize(pA->m_fShadowSize, pB->m_fShadowSize, f)
	, m_fShadowHeight(pA->m_fShadowHeight, pB->m_fShadowHeight, f)
	, m_fDropShadowIntensity(pA->m_fDropShadowIntensity, pB->m_fDropShadowIntensity, f)
	, m_fDropShadowHeight(pA->m_fDropShadowHeight, pB->m_fDropShadowHeight, f)
	, m_fIndividualShadowMaxIntensity(pA->m_fIndividualShadowMaxIntensity, pB->m_fIndividualShadowMaxIntensity, f)
	, m_fMultiShadowIntensity(pA->m_fMultiShadowIntensity, pB->m_fMultiShadowIntensity, f)
	, m_fFadeRange(pA->m_fFadeRange, pB->m_fFadeRange, f)
	, m_fMaxShadows(pA->m_fMaxShadows, pB->m_fMaxShadows, f)
{
}

void ShadowEnvSettings::Set()
{
	C_GameWorld *p = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
	if (p)
	{
		BlobShadows::C_BlobShadows* pBlobShadows = p->GetBlobShadows();

		pBlobShadows->SetShadowSize(m_fShadowSize);
		pBlobShadows->SetShadowHeight(m_fShadowHeight);
		pBlobShadows->SetDropShadowIntensity(m_fDropShadowIntensity);
		pBlobShadows->SetDropShadowHeight(m_fDropShadowHeight);
		pBlobShadows->SetIndividualShadowMaxIntensity(m_fIndividualShadowMaxIntensity);
		pBlobShadows->SetMultiShadowIntensity(m_fMultiShadowIntensity);
		pBlobShadows->SetShadowFade(m_fFadeRange);
		p->SetMaxBlobShadows(U32(m_fMaxShadows));
	}
}

FX::I_EnvironmentSettings* ShadowEnvSettings::Lerp(const FX::I_EnvironmentSettings* p, float f) const
{
	assert(Name() == p->Name());
	return new ShadowEnvSettings(this, (ShadowEnvSettings*)p, f);
}

void ShadowEnvSettings::AssociateDebugVariables(const Util::C_String& sName)
{
	m_fShadowSize.AssociateDebugVariables(sName + ".Shadow.ShadowSize");
	m_fShadowHeight.AssociateDebugVariables(sName + ".Shadow.ShadowHeight");
	m_fDropShadowIntensity.AssociateDebugVariables(sName + ".Shadow.DropShadowIntensity");
	m_fDropShadowHeight.AssociateDebugVariables(sName + ".Shadow.DropShadowHeight");
	m_fIndividualShadowMaxIntensity.AssociateDebugVariables(sName + ".Shadow.IndividualShadowMaxIntensity");
	m_fMultiShadowIntensity.AssociateDebugVariables(sName + ".Shadow.MultiShadowIntensity");
	m_fFadeRange.AssociateDebugVariables(sName + ".Shadow.FadeRange");
	m_fMaxShadows.AssociateDebugVariables(sName + ".Shadow.MaxShadows");
}

} // namespace

