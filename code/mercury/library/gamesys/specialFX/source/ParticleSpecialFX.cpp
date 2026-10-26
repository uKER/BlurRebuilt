#include "SpecialFXPch.h"

#include "SpecialFX/ParticleSpecialFX.h"
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>
#include <Dynamic/Common.h>
#include <Dynamic/Renderer.h>
#include <Dynamic/Particle/Manager.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

XT_TYPE(SpecialFX::C_ParticleSpecialFX,	"ParticleSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_Offset,					"Offset")
	XT_FIELD(m_CollisionHeightOffset,	"CollisionHeightOffset")
	XT_FIELD(m_ParticleEvent,			"ParticleEvent")
XT_END_TYPE

extern Debug::Logger &g_SpecialFXLog;

namespace SpecialFX
{
//Internal class implementation
class C_ParticleSpecialFXInstance : public I_SpecialFXComponentInstance
{
	Maths::HVec4								   m_Offset;
	DynamicEffect::Particle::C_EventInstanceHandle m_EventHandle;
	bool										   m_Repeating;
	bool										   m_Created;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;

	F32												m_Intensity;
	F32												m_CollisionHeightOffset;
	U32												m_VisibilityFlags;

	friend class C_ParticleSpecialFX;
public:
	C_ParticleSpecialFXInstance();
	virtual ~C_ParticleSpecialFXInstance();
	
	virtual void Start();
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal );
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration() { return 100000.f; } ; //We need to get the time from the particle event
	virtual void UpdateIntensity(F32 intensity);
	virtual void SetParameter(U32 paramID, void* pData);
};

C_ParticleSpecialFXInstance::C_ParticleSpecialFXInstance()
{
	m_Repeating = false;
	m_Intensity = 1.f;
	m_Created = true;
	m_VisibilityFlags = 0xFFFFFFFF;
}

C_ParticleSpecialFXInstance::~C_ParticleSpecialFXInstance()
{
}

void C_ParticleSpecialFXInstance::Start()
{
	m_Created = false;
}

void C_ParticleSpecialFXInstance::UpdateWorldMatrix( const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal )
{
	if ((m_EventHandle.Object() == NULL) && (!m_Created || m_Repeating))
	{
		m_Created = true;
		DynamicEffect::C_Node node;
		node.SetPosition(Maths::HVec3(m_Offset * worldMatrix));
		node.SetRotation(Maths::C_Quaternion(worldMatrix.Rotation()));
		if (m_ParticleEvent.Object())
		{
			m_EventHandle = m_ParticleEvent->pEvent->Create(node);
			if (m_EventHandle.Object())
			{
				m_Repeating = m_EventHandle.Object()->GetEvent()->GetDefinition().bRepeating;
				m_EventHandle->SetVelocity(velocity);
			}
		}
	}

	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EventHandle.Object())
	{
		DynamicEffect::C_Node node;

		node.SetPosition(Maths::HVec3(m_Offset * worldMatrix));
		node.SetRotation(Maths::C_Quaternion(worldMatrix.Rotation()));
		pEventInstance->SetTransform(node);
		pEventInstance->SetLighting(Maths::C_RGBA_F32(1.f, 1.f, 1.f, 1.f));
		pEventInstance->SetVisibilityFlags(m_VisibilityFlags);
		//if (m_CollisionHeightOffset < 0.01f)
		//	pEventInstance->SetCollisionPlane(Maths::HVec3(node.GetPosition().x, node.GetPosition().y + m_CollisionHeightOffset, node.GetPosition().z), Maths::HVec3(0.f, 1.f, 0.f));
		//else
		//	pEventInstance->SetCollisionPlane(Maths::HVec3(0.0f, -10000.f, 0.0f), Maths::HVec3(0.f, 1.f, 0.f));
		pEventInstance->SetCollisionPlane(planePos, planeNormal);
	}
}

void C_ParticleSpecialFXInstance::Stop()
{
	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EventHandle.Object())
	{
		pEventInstance->Free();
	}
	m_Created = true;
	m_Repeating = false;
}

bool C_ParticleSpecialFXInstance::Update( F32 time )
{
	return true;
}

void C_ParticleSpecialFXInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
	if(DynamicEffect::Particle::C_EventInstance* pEventInstance = m_EventHandle.Object())
	{
		pEventInstance->SetIntensity(m_Intensity);
	}
}

void C_ParticleSpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	if (paramID == E_ParamID_SetMeshViewFlags)
	{
		//Particles system specialFX doesn't support viewflags settings
		//LOGWARN(g_SpecialFXLog, "<%s> Particles system specialFX doesn't support viewflags settings", m_ParticleEvent->name.Text());
		U32* pParamMask = (U32*) pData;
		m_VisibilityFlags = ((m_VisibilityFlags & ~pParamMask[1]) | (pParamMask[0]&pParamMask[1]));
	}
}
C_ParticleSpecialFX::C_ParticleSpecialFX()
{
}

I_SpecialFXComponentInstance* C_ParticleSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	C_ParticleSpecialFXInstance	*pParticleSpecialFXInstance = new C_ParticleSpecialFXInstance();
	
	if (m_ParticleEvent.Object() != NULL)
	{
		pParticleSpecialFXInstance->m_Offset = Maths::HVec4(m_Offset.vx, m_Offset.vy, m_Offset.vz, 1.f);
		pParticleSpecialFXInstance->m_CollisionHeightOffset = m_CollisionHeightOffset;
		//respawn info
		pParticleSpecialFXInstance->m_ParticleEvent = m_ParticleEvent;
	}
	
	return pParticleSpecialFXInstance;		
}
}

