#include "SpecialFXPch.h"

#include <specialfx/LightSpecialFX.h>
#include "misc_shared/Curve/HermiteCurve.h"

#include <game_context/RenderContext.h>
#include <misc_shared/context/ContextProvider.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <SceneGraph/SceneGraph.h>

XT_TYPE(SpecialFX::C_LightSpecialFX,		"LightSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_EffectGraph,			"EffectGraph")
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_Offset,				"Offset")
	XT_FIELD(m_Colour,				"Colour")
	XT_FIELD(m_Radius,				"Radius")
	XT_FIELD(m_CullDistance,		"CullDistance")
XT_END_TYPE

namespace SpecialFX
{

//Internal instance class
class C_LightSpecialFXInstance : public I_SpecialFXComponentInstance
{
	F32									m_TotalTime;

	Curve::C_HermiteCurve				m_EffectCurve;	

	Maths::HVec3						m_Position;
	Maths::HVec3						m_Offset;
	F32									m_Radius;
	Maths::C_RGBA_F32					m_Colour;
	F32									m_CullDistance;

	F32									m_Intensity;

	SceneGraph::C_PointLightHandle		m_LightHandle;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;

	SceneGraph::C_VisibleDetect*		m_pVisibilityDetect;
	U32									m_VisibilityFlags;
	
	friend class C_LightSpecialFX;

public:
	C_LightSpecialFXInstance(SceneGraph::C_SceneGraph* pSceneGraph) : m_pSceneGraph(pSceneGraph), m_pVisibilityDetect(NULL), m_VisibilityFlags(0xFFFFFFFF) { m_Intensity = 1.f;}

	virtual void Start();
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal);
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } ;
	virtual void UpdateIntensity(F32 intensity);
	virtual void SetVisibilityDetect(SceneGraph::C_VisibleDetect* pVisibilityDetect);
	virtual void SetParameter(U32 paramID, void* pData);

};

void C_LightSpecialFXInstance::Start()
{
	if (!m_pSceneGraph)
		return;
	
	m_LightHandle = SceneGraph::AddPointLight(m_pSceneGraph);
	if (m_pVisibilityDetect && m_LightHandle.Object())
		m_LightHandle.Object()->SetVisDetect(m_pVisibilityDetect);
}
void C_LightSpecialFXInstance::UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal)
{
	m_Position = m_Offset * worldMatrix;
}

bool C_LightSpecialFXInstance::Update( F32 time )
{
	if (time < m_TotalTime)
	{
		F32 effectFactor = m_EffectCurve.Evaluate(time / m_TotalTime) * m_Intensity;
		
		if (SceneGraph::C_PointLight* pLight = m_LightHandle.Object())
		{
			pLight->SetColour(m_Colour * effectFactor);
			pLight->SetCullDistance(m_CullDistance);
			pLight->SetDimensions(m_Position, m_Radius);
			pLight->SetVisibilityFlags(m_VisibilityFlags);
		}

		return true;
	}

	return false;
}

void C_LightSpecialFXInstance::Stop()
{
	if (!m_pSceneGraph)
		return;
	
	SceneGraph::RemovePointLight(m_pSceneGraph, m_LightHandle);
	m_LightHandle = SceneGraph::C_PointLightHandle();
}

void C_LightSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}

void C_LightSpecialFXInstance::SetVisibilityDetect( SceneGraph::C_VisibleDetect* pVisibilityDetect )
{
	m_pVisibilityDetect = pVisibilityDetect;

	//If it is already added
	if (m_LightHandle.Object())
	{
		m_LightHandle.Object()->SetVisDetect(pVisibilityDetect);		
	}
}

void C_LightSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == E_ParamID_SetMeshViewFlags)
	{
		U32* pParamMask = (U32*) pData;
		m_VisibilityFlags = ((m_VisibilityFlags & ~pParamMask[1]) | (pParamMask[0]&pParamMask[1]));
	}
}
I_SpecialFXComponentInstance* C_LightSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	const GameContext::I_RenderContext* pContext = pContextProvider->GetContext<GameContext::I_RenderContext>();
	if (pContext == NULL)
		return NULL;

	C_LightSpecialFXInstance* lightSpecialFXInstance = new C_LightSpecialFXInstance(pContext->GetSceneGraph());

	//Build Effect graph
	C_HermiteKeyFrame keyFrame;

	//Build the effect curve
	for(int j = 0; j < m_EffectGraph.count; j++)
	{
		keyFrame = m_EffectGraph[j];

		lightSpecialFXInstance->m_EffectCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	lightSpecialFXInstance->m_TotalTime = m_Time;
	lightSpecialFXInstance->m_Offset = Maths::HVec3(m_Offset.vx, m_Offset.vy, m_Offset.vz);
	lightSpecialFXInstance->m_Colour = Maths::C_RGBA_F32(m_Colour.r, m_Colour.g, m_Colour.b, 1.0f);
	lightSpecialFXInstance->m_Radius = m_Radius;
	lightSpecialFXInstance->m_CullDistance = m_CullDistance;

	return lightSpecialFXInstance;
}

}
