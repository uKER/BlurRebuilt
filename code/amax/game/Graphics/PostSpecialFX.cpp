#include "stdafx.h"

#include "PostSpecialFX.h"
#include "PostFXMatrix.h"
#include "misc_shared/Curve/HermiteCurve.h"

#include <FX/EnvironmentSettings.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <Fx/Fx.h>
#include <Fx/EnvironmentSettings.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View.h>
#include <Scenegraph/View.h>
#include "PostFXSettings.h"
#include <render/debug/DebugRender.h>

Debug::Bool g_debugPostProcessSpecialFXRenderDebug("GameSys.SpecialFX.PostProcessWorldSpaceRenderDebug", false);

XT_TYPE(Game::C_PostSpecialFX,		"PostSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_EffectGraph,			"EffectGraph")
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_Matrices,			"matrices")
	XT_FIELD(m_Root,				"root")
	XT_FIELD(m_FilmGrainIntensity,	"filmGrainIntensity")
	XT_FIELD(m_VignetteWeight,		"vignetteWeight")
	XT_FIELD(m_TunnelAmount,		"tunnelAmount")
	XT_FIELD(m_ColourSep,			"ColourSep")
	XT_FIELD(m_Blockiness,			"Blockiness")
	XT_FIELD(m_ScreenOverlay,		"ScreenOverlay")
	XT_FIELD(m_WorldSpaceEffectRadius,		"WorldSpaceEffectRadius")
	XT_FIELD(m_WorldSpaceEffectCameraDirectionFactor, "WorldSpaceEffectCameraDirectionFactor")
XT_END_TYPE

namespace Game
{

//Internal instance class
class C_PostSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
{
	F32									m_TotalTime;

	F32									m_Intensity;

	Curve::C_HermiteCurve				m_EffectCurve;	

	Maths::Mat44						m_mColour;
	F32									m_fFilmGrainIntensity;
	F32									m_fVignetteWeight;
	F32									m_fTunnelAmount;
	F32									m_ColourSep, m_Blockiness, m_ScreenOverlay;
	F32									m_WorldSpaceEffectRadius;
	F32									m_WorldSpaceEffectCameraDirectionFactor;

	S32									m_ViewportID;

	Maths::HVec3						m_Position;
	bool								m_ViewportDependent;

	friend class C_PostSpecialFX;

	void								Apply(F32 effectFactor, S32 viewportID);

public:
	C_PostSpecialFXInstance()
	{
		m_ViewportID = 0;
		m_ViewportDependent = false;
		m_Intensity = 1.f;
		m_ColourSep = m_Blockiness = m_ScreenOverlay = 0.f;
	}

	virtual void Start() {};
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) { m_Position = worldMatrix.AW();};
	virtual void Stop() {};
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } ;
	virtual void SetParameter(U32 paramID, void* pData);
	virtual void UpdateIntensity(F32 intensity);
};

bool C_PostSpecialFXInstance::Update( F32 time )
{
	if ((time < m_TotalTime) && (m_ViewportID != -1))
	{
		F32 effectFactor = m_EffectCurve.Evaluate(time / m_TotalTime) * m_Intensity;	
		
		if (m_WorldSpaceEffectRadius < 0.01f)
		{
			//Normal behaviour
			FX::I_EnvironmentSettings* pEnvironmentSettings = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetEnvironmentSettings(m_ViewportID);

			PostFxEnvSettings* pPostFXEnvSettings = (PostFxEnvSettings*)pEnvironmentSettings->GetSettingsBlockByName("PostFX");

			BIZ_ASSERT(pPostFXEnvSettings);

			if (pPostFXEnvSettings)
			{
				Maths::Mat44 identity = Maths::Const<Maths::Mat44>::Identity();
				Maths::VF32 factorVector = Maths::MakeVF32(effectFactor);

				Maths::Mat44 colourFactor;
				Maths::SetRowX(&colourFactor, Maths::Lerp( Maths::RowX(identity), Maths::RowX(m_mColour), factorVector));
				Maths::SetRowY(&colourFactor, Maths::Lerp( Maths::RowY(identity), Maths::RowY(m_mColour), factorVector));
				Maths::SetRowZ(&colourFactor, Maths::Lerp( Maths::RowZ(identity), Maths::RowZ(m_mColour), factorVector));
				Maths::SetRowW(&colourFactor, Maths::Lerp( Maths::RowW(identity), Maths::RowW(m_mColour), factorVector));

				//Apply changes to global HDR variables
				pPostFXEnvSettings->m_ColourMatrix = pPostFXEnvSettings->m_ColourMatrix * colourFactor;
				pPostFXEnvSettings->m_FilmGrainIntensity = pPostFXEnvSettings->m_FilmGrainIntensity + m_fFilmGrainIntensity * effectFactor;
				pPostFXEnvSettings->m_VignetteWeight = pPostFXEnvSettings->m_VignetteWeight + m_fVignetteWeight * effectFactor;
				pPostFXEnvSettings->m_TunnelAmount = pPostFXEnvSettings->m_TunnelAmount + m_fTunnelAmount * effectFactor;
				pPostFXEnvSettings->m_ColourSep = pPostFXEnvSettings->m_ColourSep + m_ColourSep * effectFactor;
				pPostFXEnvSettings->m_Blockiness = pPostFXEnvSettings->m_Blockiness + m_Blockiness * effectFactor;
				pPostFXEnvSettings->m_ScreenOverlay = pPostFXEnvSettings->m_ScreenOverlay + m_ScreenOverlay * effectFactor;
			}
		}
		else
		{
#ifndef CFG_GOLD
			if (g_debugPostProcessSpecialFXRenderDebug)
			{
				Render::Debug::Deferred()->Draw3dSphere(m_Position, m_WorldSpaceEffectRadius, Maths::RGBA(1.f, 1.f, 0.f, 1.f));
			}
#endif
			//World Space behaviour
			if (m_ViewportDependent)
			{
				Apply(effectFactor, m_ViewportID); //Only affect viewport selected
			}
			else
			{
				Game::C_GameWorld* pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();
				S32 numViews = pGameWorld->GetViewManager()->GetNumViews();

				for (S32 i = 0; i < numViews; i++)
				{
					Apply(effectFactor, i);
				}
			}
		}


		return true;
	}

	return false;
}

void C_PostSpecialFXInstance::Apply(F32 effectFactor, S32 viewportID )
{
	Game::C_GameWorld* pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();

	//Calculate radius and attenuated effect
	Maths::HVec3 cameraPosition = pGameWorld->GetViewManager()->GetView(viewportID)->GetMainView().GetViewSettings3D().GetCameraPosition();
	Maths::HVec3 cameraDirection = pGameWorld->GetViewManager()->GetView(viewportID)->GetMainView().GetViewSettings3D().GetCameraToWorld().AZ();
	cameraDirection.Normalise();

	Maths::HVec3 effectDirection = m_Position - cameraPosition;
	effectDirection.Normalise();

	F32 radius = (cameraPosition - m_Position).Length2();
	const F32 worldSpaceEffectRadiusTwo = m_WorldSpaceEffectRadius * m_WorldSpaceEffectRadius;
	F32 distance = worldSpaceEffectRadiusTwo - radius;
	if (distance > 0.f)
	{
		F32 cameraDirectionFactor = Maths::Clamp(F32(cameraDirection | effectDirection) + 2.f * (1.f - m_WorldSpaceEffectCameraDirectionFactor), 0.f, 1.f);
		F32 effectFactorPerView = effectFactor * cameraDirectionFactor * distance / worldSpaceEffectRadiusTwo;

		FX::I_EnvironmentSettings* pEnvironmentSettings = pGameWorld->GetEnvironmentSettings(viewportID);
		PostFxEnvSettings* pPostFXEnvSettings = (PostFxEnvSettings*)pEnvironmentSettings->GetSettingsBlockByName("PostFX");

		BIZ_ASSERT(pPostFXEnvSettings);

		if (pPostFXEnvSettings)
		{
			Maths::Mat44 identity = Maths::Const<Maths::Mat44>::Identity();
			Maths::VF32 factorVector = Maths::MakeVF32(effectFactorPerView);

			Maths::Mat44 colourFactor;
			Maths::SetRowX(&colourFactor, Maths::Lerp( Maths::RowX(identity), Maths::RowX(m_mColour), factorVector));
			Maths::SetRowY(&colourFactor, Maths::Lerp( Maths::RowY(identity), Maths::RowY(m_mColour), factorVector));
			Maths::SetRowZ(&colourFactor, Maths::Lerp( Maths::RowZ(identity), Maths::RowZ(m_mColour), factorVector));
			Maths::SetRowW(&colourFactor, Maths::Lerp( Maths::RowW(identity), Maths::RowW(m_mColour), factorVector));

			//Apply changes to global HDR variables
			pPostFXEnvSettings->m_ColourMatrix = pPostFXEnvSettings->m_ColourMatrix * colourFactor;
			pPostFXEnvSettings->m_FilmGrainIntensity = pPostFXEnvSettings->m_FilmGrainIntensity + m_fFilmGrainIntensity * effectFactorPerView;
			pPostFXEnvSettings->m_VignetteWeight = pPostFXEnvSettings->m_VignetteWeight + m_fVignetteWeight * effectFactorPerView;
			pPostFXEnvSettings->m_TunnelAmount = pPostFXEnvSettings->m_TunnelAmount + m_fTunnelAmount * effectFactorPerView;
			pPostFXEnvSettings->m_ColourSep = pPostFXEnvSettings->m_ColourSep + m_ColourSep * effectFactorPerView;
			pPostFXEnvSettings->m_Blockiness = pPostFXEnvSettings->m_Blockiness + m_Blockiness * effectFactorPerView;
			pPostFXEnvSettings->m_ScreenOverlay = pPostFXEnvSettings->m_ScreenOverlay + m_ScreenOverlay * effectFactorPerView;
		}
	}
}

void C_PostSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
	{
		m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] viewport id carried in the pointer
		m_ViewportDependent = true;
	}
}

void C_PostSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}

SpecialFX::I_SpecialFXComponentInstance* C_PostSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	C_PostSpecialFXInstance* postSpecialFXInstance = new C_PostSpecialFXInstance();

	//Build Effect graph
	SpecialFX::C_HermiteKeyFrame keyFrame;

	//Build the effect curve
	for(int j = 0; j < m_EffectGraph.count; j++)
	{
		keyFrame = m_EffectGraph[j];

		postSpecialFXInstance->m_EffectCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	//Build Post Matrices
	postSpecialFXInstance->m_mColour = Maths::Const<Maths::Mat44>::Identity();

	PostFxMatrix* pNode = m_Root;
	while(pNode)
	{
		if (pNode->m_bEnabled)
			postSpecialFXInstance->m_mColour = postSpecialFXInstance->m_mColour * pNode->Matrix();
		pNode = pNode->m_pOut;
	}

	postSpecialFXInstance->m_fFilmGrainIntensity = m_FilmGrainIntensity;
	postSpecialFXInstance->m_fVignetteWeight = m_VignetteWeight;
	postSpecialFXInstance->m_fTunnelAmount = m_TunnelAmount;
	postSpecialFXInstance->m_ColourSep = m_ColourSep;
	postSpecialFXInstance->m_Blockiness = m_Blockiness;
	postSpecialFXInstance->m_ScreenOverlay = m_ScreenOverlay;
	postSpecialFXInstance->m_WorldSpaceEffectRadius = m_WorldSpaceEffectRadius;
	postSpecialFXInstance->m_WorldSpaceEffectCameraDirectionFactor = m_WorldSpaceEffectCameraDirectionFactor;
	postSpecialFXInstance->m_TotalTime = m_Time;

	return postSpecialFXInstance;
}

}