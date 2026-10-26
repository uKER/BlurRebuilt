#include "SpecialFXPch.h"

#include <specialfx/SpotLightSpecialFX.h>
#include "misc_shared/Curve/HermiteCurve.h"

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

#include <game_context/RenderContext.h>
#include <misc_shared/context/ContextProvider.h>

#include <SceneGraph/SceneGraph.h>

#include <TextureResource/TextureResource.h>
#include <resource_utils/ResourceBagUtils.h>


XT_TYPE(SpecialFX::C_SpotLightSpecialFX,	"SpotLightSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_EffectGraph,			"EffectGraph")
	XT_FIELD(m_Roll,				"RollGraph")
	XT_FIELD(m_Pitch,				"PitchGraph")
	XT_FIELD(m_Heading,				"HeadingGraph")
	XT_FIELD(m_Time,				"Time")
	XT_FIELD(m_Offset,				"Offset")
	XT_FIELD(m_Colour,				"Colour")
	XT_FIELD(m_Radius,				"Radius")
	XT_FIELD(m_FieldOfViewX,		"FieldOfViewX")
	XT_FIELD(m_FieldOfViewY,		"FieldOfViewY")
	XT_FIELD(m_CullDistance,		"CullDistance")
	XT_FIELD(m_TextureName,			"Texture")
XT_END_TYPE

namespace SpecialFX
{

//Internal instance class
class C_SpotLightSpecialFXInstance : public I_SpecialFXComponentInstance
{
	F32									m_TotalTime;

	F32									m_Intensity;

	Curve::C_HermiteCurve				m_EffectCurve;	
	Curve::C_HermiteCurve				m_RollCurve;	
	Curve::C_HermiteCurve				m_PitchCurve;	
	Curve::C_HermiteCurve				m_HeadingCurve;	

	Maths::HVec3						m_Offset;
	F32									m_Radius;
	F32									m_FieldOfViewX;
	F32									m_FieldOfViewY;
	Maths::C_RGBA_F32					m_Colour;
	F32									m_CullDistance;

	SceneGraph::C_SpotLightHandle		m_LightHandle;

	SceneGraph::C_SceneGraph*			m_pSceneGraph;

	SceneGraph::C_VisibleDetect*		m_pVisibilityDetect;

	TextureResource::C_TextureHandle	m_TextureHandle;

	Maths::HMat43						m_WorldMatrix;

	U32									m_VisibilityFlags;

	friend class C_SpotLightSpecialFX;

public:
	C_SpotLightSpecialFXInstance(SceneGraph::C_SceneGraph* pSceneGraph) : m_pSceneGraph(pSceneGraph), m_pVisibilityDetect(NULL), m_VisibilityFlags(0xFFFFFFFF) { m_Intensity = 1.f;}

	virtual void Start();
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal);
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return m_TotalTime; } ;
	virtual void UpdateIntensity(F32 intensity);
	virtual void SetVisibilityDetect(SceneGraph::C_VisibleDetect* pVisibilityDetect);
	virtual void SetParameter(U32 paramID, void* pData);
};

void C_SpotLightSpecialFXInstance::Start()
{
	m_LightHandle = SceneGraph::AddSpotLight(m_pSceneGraph);
	if (m_pVisibilityDetect)
		m_LightHandle.Object()->SetVisDetect(m_pVisibilityDetect);
}
void C_SpotLightSpecialFXInstance::UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal)
{
	m_WorldMatrix = worldMatrix;
}

bool C_SpotLightSpecialFXInstance::Update( F32 time )
{
	if (time < m_TotalTime)
	{
		F32 effectFactor = m_EffectCurve.Evaluate(time / m_TotalTime) * m_Intensity;
		F32 rollFactor = m_RollCurve.Evaluate(time / m_TotalTime);
		F32 pitchFactor = m_PitchCurve.Evaluate(time / m_TotalTime);
		F32 headingFactor = m_HeadingCurve.Evaluate(time / m_TotalTime);
		
		if (SceneGraph::C_SpotLight* pLight = m_LightHandle.Object())
		{
			Maths::HMat33 directionMatrix = Maths::C_MatrixRotateRPH(Maths::DegToRad(rollFactor), Maths::DegToRad(pitchFactor), Maths::DegToRad(headingFactor));
			Maths::HMat43 localMatrix(directionMatrix, m_Offset);
			Maths::HMat43 lightMatrix = localMatrix * m_WorldMatrix;

			pLight->SetColour(m_Colour * effectFactor);
			pLight->SetCullDistance(m_CullDistance);
			pLight->SetDimensions(lightMatrix, m_Radius, Maths::DegToRad(m_FieldOfViewX), Maths::DegToRad(m_FieldOfViewY));

			TextureResource::C_TextureResource* pTextureResource = m_TextureHandle;
			if (pTextureResource && pTextureResource->GetDisplayTexture())
			{
				pLight->m_pData = (Display::C_Texture2D*)pTextureResource->GetDisplayTexture();
				pLight->m_RenderFlags = SceneGraph::E_SpotLightType_Projected;
			}
			else
			{
				pLight->m_pData = 0;
				pLight->m_RenderFlags = SceneGraph::E_SpotLightType_Standard;
			}
			pLight->SetVisibilityFlags(m_VisibilityFlags);
		}

		return true;
	}

	return false;
}

void C_SpotLightSpecialFXInstance::Stop()
{
	if (!m_pSceneGraph)
		return;
		
	SceneGraph::RemoveSpotLight(m_pSceneGraph, m_LightHandle);
	m_LightHandle = SceneGraph::C_SpotLightHandle();
}

void C_SpotLightSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}

void C_SpotLightSpecialFXInstance::SetVisibilityDetect( SceneGraph::C_VisibleDetect* pVisibilityDetect )
{
	m_pVisibilityDetect = pVisibilityDetect;

	//If it is already added
	if (m_LightHandle.Object())
	{
		m_LightHandle.Object()->SetVisDetect(pVisibilityDetect);		
	}
}

void C_SpotLightSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == E_ParamID_SetMeshViewFlags)
	{
		U32* pParamMask = (U32*) pData;
		m_VisibilityFlags = ((m_VisibilityFlags & ~pParamMask[1]) | (pParamMask[0]&pParamMask[1]));
	}
}

I_SpecialFXComponentInstance* C_SpotLightSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	const GameContext::I_RenderContext* pContext = pContextProvider->GetContext<GameContext::I_RenderContext>();
	if (pContext == NULL)
		return NULL;

	C_SpotLightSpecialFXInstance* lightSpecialFXInstance = new C_SpotLightSpecialFXInstance(pContext->GetSceneGraph());

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

	//Build the roll curve
	for(int j = 0; j < m_Roll.count; j++)
	{
		keyFrame = m_Roll[j];

		lightSpecialFXInstance->m_RollCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	//Build the roll curve
	for(int j = 0; j < m_Pitch.count; j++)
	{
		keyFrame = m_Pitch[j];

		lightSpecialFXInstance->m_PitchCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	//Build the heading curve
	for(int j = 0; j < m_Heading.count; j++)
	{
		keyFrame = m_Heading[j];

		lightSpecialFXInstance->m_HeadingCurve.AddKeyFrame(keyFrame.mTime, 
			keyFrame.mValue, 
			keyFrame.mInTangent, 
			keyFrame.mOutTangent);
	}

	lightSpecialFXInstance->m_TotalTime = m_Time;
	lightSpecialFXInstance->m_Offset = Maths::HVec3(m_Offset.vx, m_Offset.vy, m_Offset.vz);
	lightSpecialFXInstance->m_Colour = Maths::C_RGBA_F32(m_Colour.r, m_Colour.g, m_Colour.b, 1.0f);
	lightSpecialFXInstance->m_Radius = m_Radius;
	lightSpecialFXInstance->m_FieldOfViewX = m_FieldOfViewX;
	lightSpecialFXInstance->m_FieldOfViewY = m_FieldOfViewY;
	lightSpecialFXInstance->m_CullDistance = m_CullDistance;
	lightSpecialFXInstance->m_WorldMatrix.SetIdentity();
	lightSpecialFXInstance->m_TextureHandle = pResourceBag->GetResource<TextureResource::C_TextureResource>( MakeResourceBagTag(m_TextureName.Text()) );

	return lightSpecialFXInstance;
}

void C_SpotLightSpecialFX::AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const
{
	pResourceBag.AddResource( MakeResourceBagTag(m_TextureName.Text()), m_TextureName.Text(), TextureResource::C_TextureResource::StaticGetResourceTypeId());
}
}
