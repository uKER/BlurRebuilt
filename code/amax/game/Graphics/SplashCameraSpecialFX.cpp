#include "stdafx.h"

#include "SplashCameraSpecialFX.h"

#include "misc_shared/Curve/HermiteCurve.h"

#include <FX/EnvironmentSettings.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <Game/GameWorld/ViewManager.h>
#include <Game/GameWorld/View.h>
#include <Scenegraph/View.h>
#include <SplashCamera/SplashCamera.h>
#include <Game/GameWorld/ViewType.h>
#include <Game/GameUtil/ResourceBag.h>
#include <render/debug/DebugRender.h>

Debug::Bool g_debugSplashCameraSpecialFXRenderDebug("GameSys.SpecialFX.SplashCameraRenderDebug", false);

XT_TYPE(Game::C_SplashCameraSpecialFX,		"SplashCameraSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_EffectGraph,			"EffectGraph")
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_SplashCameraType,	"SplashCameraType")
	XT_FIELD(m_Radius,				"Radius")
	XT_FIELD(m_SplashesPerSecond,	"SplashesPerSecond")
	XT_FIELD(m_Velocity,			"Velocity")
	XT_FIELD(m_Spread,				"Spread")
XT_END_TYPE

namespace Game
{

//Internal instance class
class C_SplashCameraSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
{
	F32									m_TotalTime;
	F32									m_Intensity;
	Curve::C_HermiteCurve				m_EffectCurve;	

	XtHandle<SplashCamera::C_SplashType>		m_SplashCameraType;
	XtF32										m_Radius;
	XtF32										m_SplashesPerSecond;
	XtF32										m_Velocity;
	XtF32										m_Spread;

	F32									m_SplashCurrentCount[4];
	Maths::HVec3						m_Position;
	S32									m_ViewportID;

	friend class C_SplashCameraSpecialFX;

	void								Apply(F32 effectFactor, S32 viewportID);

public:
	C_SplashCameraSpecialFXInstance()
	{
		m_ViewportID = -1;

		m_Intensity = 1.f;
		for (U32 i = 0; i < 4; i++)
			m_SplashCurrentCount[i] = 0.f;
	}

	virtual void Start() {};
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal) { m_Position = worldMatrix.AW();};
	virtual void Stop() {};
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } ;
	virtual void SetParameter(U32 paramID, void* pData);
	virtual void UpdateIntensity(F32 intensity);
};

bool C_SplashCameraSpecialFXInstance::Update( F32 time )
{
	if (m_SplashCameraType.Object() == NULL)
		return false;

#ifndef CFG_GOLD
	if (g_debugSplashCameraSpecialFXRenderDebug)
	{
		Render::Debug::Deferred()->Draw3dSphere(m_Position, m_Radius, Maths::RGBA(1.f, 1.f, 1.f, 1.f));
	}
#endif

	if ((time < m_TotalTime))
	{
		F32 effectFactor = m_EffectCurve.Evaluate(time / m_TotalTime) * m_Intensity;	
		
		if (m_ViewportID == -1)
		{
			for (S32 i = 0; i < Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount(); i++)
			{
				Apply(effectFactor, i);
			}
		}
		else
		{
			//Per viewport
			Apply(effectFactor, m_ViewportID);
		}

		

		return true;
	}

	return false;
}


void C_SplashCameraSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}

void C_SplashCameraSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == SpecialFX::E_ParamID_SetCameraViewport)
	{
		m_ViewportID = (S32)(INTPTR) pData;	// [Blur2PC][x64] viewport id carried in the pointer
	}
}

void C_SplashCameraSpecialFXInstance::Apply(F32 effectFactor, S32 viewportID )
{
	Game::C_GameWorld* pGameWorld = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld();

	U32 viewType = E_ViewType_Main1 + viewportID;
	Maths::HVec3 cameraPosition = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(viewportID).m_View.GetViewSettings3D().GetCameraPosition();
	Maths::HVec3 cameraDirection = cameraPosition - m_Position;
	F32 distance = cameraDirection.Length2();
	cameraDirection.Normalised();
	Maths::HVec3 velocity = cameraDirection * m_Velocity;

	const F32 radiusTwo = m_Radius * m_Radius;
	//Radius factor
	if (distance < radiusTwo)
	{
		//Distance factor
		F32 distanceFactor = (radiusTwo - distance) / (radiusTwo);
		m_SplashCurrentCount[viewportID] += m_Intensity * effectFactor * distanceFactor * m_SplashesPerSecond * C_GameApp::GetInstance()->GetTimeStep();

		if (m_SplashCurrentCount[viewportID] > 1.f)
		{
			U32 splashCount = Maths::Floor(m_SplashCurrentCount[viewportID]);
			pGameWorld->GetSplashCamera()->AddSplash(viewType, m_SplashCameraType, splashCount, m_Position, velocity, m_Spread, C_GameApp::GetInstance()->GetSpecialFXResourceBag().RawPtr(), true);
			m_SplashCurrentCount[viewportID] -= F32(splashCount);
		}
	}
}
SpecialFX::I_SpecialFXComponentInstance* C_SplashCameraSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	C_SplashCameraSpecialFXInstance* splashCameraSpecialFXInstance = new C_SplashCameraSpecialFXInstance();

	//Build Effect graph
	SpecialFX::C_HermiteKeyFrame keyFrame;

	//Build the effect curve
	for(int j = 0; j < m_EffectGraph.count; j++)
	{
		keyFrame = m_EffectGraph[j];

		splashCameraSpecialFXInstance->m_EffectCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}


	splashCameraSpecialFXInstance->m_SplashCameraType = m_SplashCameraType;
	splashCameraSpecialFXInstance->m_Radius = m_Radius;
	splashCameraSpecialFXInstance->m_SplashesPerSecond = m_SplashesPerSecond;
	splashCameraSpecialFXInstance->m_Velocity = m_Velocity;
	splashCameraSpecialFXInstance->m_Spread = m_Spread;
	
	splashCameraSpecialFXInstance->m_TotalTime = m_Time;

	return splashCameraSpecialFXInstance;
}

}