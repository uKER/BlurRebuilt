#include "SpecialFXPch.h"

#include "specialFX/FlareSpecialFX.h"

#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Light/Manager.h>
#include <Dynamic/Node.h>

#include <game_context/LightingContext.h>
#include <misc_shared/context/ContextProvider.h>

#include "misc_shared/Curve/HermiteCurve.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

XT_TYPE(SpecialFX::C_FlareSpecialFX,	"FlareSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_Offset,			"Offset")
	XT_FIELD(m_RotationX,		"Roll")
	XT_FIELD(m_RotationY,		"Pitch")
	XT_FIELD(m_Time,			"Time")
	XT_FIELD(m_IntensityGraph,	"Intensity")
	XT_FIELD(m_XtHandle,		"Flare")
XT_END_TYPE


namespace SpecialFX	
{

//Internal class implementation
class C_FlareSpecialFXInstance : public SpecialFX::I_SpecialFXComponentInstance
{
	F32												m_TotalTime;
	F32												m_Intensity;
	Curve::C_HermiteCurve							m_IntensityCurve;	
	DynamicEffect::Light::C_Manager*				m_pLightManager;
	DynamicEffect::Light::LightDefinition*			m_pLightDefinition;
	DynamicEffect::Light::LightHandle				m_Handle;
	DynamicEffect::C_Node							m_Node;
	Maths::C_QuatTrans								m_Offset;
	U32												m_VisibilityFlags;
	friend class C_FlareSpecialFX;
public:
	C_FlareSpecialFXInstance(DynamicEffect::Light::C_Manager* pLightManager);
	virtual ~C_FlareSpecialFXInstance();
	
	virtual void Start();
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal);
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } ; //We need to get the time from the particle event
	virtual void UpdateIntensity(F32 intensity);
	virtual void SetParameter(U32 paramID, void* pData);
};

C_FlareSpecialFXInstance::C_FlareSpecialFXInstance(DynamicEffect::Light::C_Manager* pLightManager)
: m_pLightManager(pLightManager), m_VisibilityFlags(0xFFFFFFFF)
{
	m_Intensity = 1.f;
}

C_FlareSpecialFXInstance::~C_FlareSpecialFXInstance()
{
}

void C_FlareSpecialFXInstance::Start()
{
	//Create flare
	m_Handle = m_pLightManager->CreateLight(m_pLightDefinition, m_Offset, &m_Node, Maths::RGBA(255, 255, 255, 255));
	if (m_Handle)
		m_Handle->m_fIntensity = 0.f;
}

void C_FlareSpecialFXInstance::UpdateWorldMatrix( const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal )
{
	m_Node.SetPosition(worldMatrix.Translation());
	m_Node.SetRotation(Maths::C_Quaternion(worldMatrix.Rotation()));
}

void C_FlareSpecialFXInstance::Stop()
{
	if (m_Handle)
	{
		m_pLightManager->DestroyLight(m_Handle);
	}
}

bool C_FlareSpecialFXInstance::Update( F32 time )
{
	if (time > m_TotalTime || !m_Handle)
		return false;

	F32 intensity = m_IntensityCurve.Evaluate(time / m_TotalTime) * m_Intensity;

	m_Handle->m_fIntensity = intensity;
	m_Handle->m_VisibilityFlags = m_VisibilityFlags;

	return true;
}

void C_FlareSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;

}

void C_FlareSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == E_ParamID_SetMeshViewFlags)
	{
		U32* pParamMask = (U32*) pData;
		m_VisibilityFlags = ((m_VisibilityFlags & ~pParamMask[1]) | (pParamMask[0]&pParamMask[1]));
	}
}

SpecialFX::I_SpecialFXComponentInstance* C_FlareSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* /*pResourceBag*/) const
{
	C_FlareSpecialFXInstance	*pFlareSpecialFXInstance = 0;

	const GameContext::I_LightingContext* pContext = pContextProvider->GetContext<GameContext::I_LightingContext>();
	if (pContext != NULL)
	{

		pFlareSpecialFXInstance = new C_FlareSpecialFXInstance(pContext->GetLightManager());

		pFlareSpecialFXInstance->m_TotalTime = m_Time;
		
		//Build Effect graph
		SpecialFX::C_HermiteKeyFrame keyFrame;
		//Build the effect curve
		for(int j = 0; j < m_IntensityGraph.count; j++)
		{
			keyFrame = m_IntensityGraph[j];

			pFlareSpecialFXInstance->m_IntensityCurve.AddKeyFrame(keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}

		pFlareSpecialFXInstance->m_Offset = Maths::C_QuatTrans(Maths::C_Quaternion(Maths::HMat33(Maths::C_MatrixRotateRPH(Maths::DegToRad(m_RotationX), Maths::DegToRad(m_RotationY), 0.f))), Maths::HVec3(m_Offset.vx, m_Offset.vy, m_Offset.vz));
		pFlareSpecialFXInstance->m_pLightDefinition = m_XtHandle->pDefinition;
	
	}

	return pFlareSpecialFXInstance;		
}
}

