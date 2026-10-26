#include "SpecialFXPch.h"

#include "SpecialFX/SpecialFX.h"
#include "specialFX/AnimatedMesh.h"
#include "specialFX/ParticleSpecialFX.h"
#include "specialFX/LightSpecialFX.h"
#include "specialFX/SpotLightSpecialFX.h"
#include "specialFX/FlareSpecialFX.h"
#include "specialFX/RumbleSpecialFX.h"

XT_TYPE(SpecialFX::C_HermiteKeyFrame,		"SFXHermiteKeyFrame")
	XT_FIELD(mTime,							"Time")
	XT_FIELD(mValue,						"Value")
	XT_FIELD(mInTangent,					"InTangent")
	XT_FIELD(mOutTangent,					"OutTangent")
XT_END_TYPE

XT_TYPE(SpecialFX::C_HermiteCurve,			"SFXHermiteCurve")
	XT_FIELD(mKeyFrames,					"KeyFrames")
XT_END_TYPE

XT_TYPE(SpecialFX::C_BaseSpecialFXComponent,"BaseSpecialFXComponent")
XT_END_TYPE

XT_TYPE(SpecialFX::C_SpecialFXComponentData,"SpecialFXComponentData")
	XT_FIELD(m_SpeedFactor,					"SpeedFactor")
	XT_FIELD(m_StartOffset,					"StartOffset")
	XT_FIELD(m_Component,					"Component")
XT_END_TYPE

XT_TYPE(SpecialFX::C_SpecialFX,				"SpecialFX")
	XT_FIELD(m_Components,					"Components")
XT_END_TYPE

XT_LIBRARY(SpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_HermiteKeyFrame)
	XT_REGISTER_TYPE(SpecialFX::C_HermiteCurve)
	XT_REGISTER_TYPE(SpecialFX::C_BaseSpecialFXComponent)
	XT_REGISTER_TYPE(SpecialFX::C_SpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_SpecialFXComponentData)
	XT_REGISTER_TYPE(SpecialFX::C_AnimatedMeshSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_InternalSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_ParticleSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_LightSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_SpotLightSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_FlareSpecialFX)
	XT_REGISTER_TYPE(SpecialFX::C_RumbleSpecialFX)
XT_END_LIBRARY

Debug::Logger &g_SpecialFXLog( Debug::GetLogger("gamesys.specialfx") );

//#pragma optimize("", off)

namespace SpecialFX
{
C_SpecialFXManager* g_SpecialFXManager;

// C_BaseSpecialFXComponent

void C_BaseSpecialFXComponent::AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const
{	
}

// C_SpecialFX

void C_SpecialFX::AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const
{
	for(S32 i=0; i<m_Components.count; i++)
	{
		const XtHandle<C_BaseSpecialFXComponent>& handle = m_Components[i].m_Component;
		if(handle)
		{
			handle->AddToResourceBag(pResourceBag);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// SpecialFXInstance
//////////////////////////////////////////////////////////////////////////




//Implementation
I_SpecialFXInstance* C_SpecialFXManager::CreateSpecialFXInstance(const C_SpecialFX& specialFXRecord, const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag, SceneGraph::C_VisibleDetect* pVisibilityDetect)
{
	I_SpecialFXInstance* pSpecialFXinstance = new I_SpecialFXInstance();
	pSpecialFXinstance->CreateComponentInstances(specialFXRecord, pContextProvider, pResourceBag, pVisibilityDetect);

	return pSpecialFXinstance;
}

void* C_SpecialFXManager::ms_InitParamVector[E_InitParamID_Count] =
{
	(void*)S32(-1), //Default 0xFFFFFFFF
	0, //Default None
	0, //Default None
	0 //Default None
};

void C_SpecialFXManager::SetInitParam( E_InitParamID initParamID, void* pInitParam )
{
	BIZ_ASSERT(initParamID < E_InitParamID_Count);
	ms_InitParamVector[initParamID] = pInitParam;
}
void* C_SpecialFXManager::GetInitParam( E_InitParamID initParamID )
{
	BIZ_ASSERT(initParamID < E_InitParamID_Count);
	return ms_InitParamVector[initParamID];
}
void InitSpecialFXManager(C_SpecialFXManager* pSpecialFXManager)
{
	g_SpecialFXManager= pSpecialFXManager;
}

void I_SpecialFXInstance::CreateComponentInstances(const C_SpecialFX& specialFXRecord, const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag, SceneGraph::C_VisibleDetect* pVisibilityDetect)
{
	for (int i = 0; i < specialFXRecord.m_Components.count; i++)
	{
		I_SpecialFXComponentInstance*	pComponentInstance;
		//Create component instance
		pComponentInstance = specialFXRecord.m_Components[i].m_Component->CreateInstance(pContextProvider, pResourceBag);

		//Set visibility detect if needed
		if (pVisibilityDetect)
		{
			pComponentInstance->SetVisibilityDetect(pVisibilityDetect);
		}

		//Add to instance vector
		m_ComponentInstanceVector.PushBack(pComponentInstance);
		
		C_ComponentDataSpecialFX componentData;

		componentData.m_Started = false;
		componentData.m_StartOffset = specialFXRecord.m_Components[i].m_StartOffset;
		componentData.m_SpeedFactor = specialFXRecord.m_Components[i].m_SpeedFactor;

		m_ComponentDataInstanceVector.PushBack(componentData);

	}
}
I_SpecialFXInstance::I_SpecialFXInstance()
{
	m_Started = false;
}

I_SpecialFXInstance::~I_SpecialFXInstance()
{
	BIZ_ASSERT(!m_Started);
	for (int i = 0; i < m_ComponentInstanceVector.Size(); i++)
	{
		SAFE_DELETE_PTR(m_ComponentInstanceVector[i]);
	}
}

void I_SpecialFXInstance::Start()
{
	m_Started = true;
}

void I_SpecialFXInstance::UpdateWorldMatrix( const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal )
{
	if (!m_Started)
		return;

	if(m_ComponentInstanceVector.Size())
	{
		int i;
		int size = m_ComponentInstanceVector.Size() - 1;
		for (i = 0; i < size; i++)
		{
			DCBT(0, m_ComponentInstanceVector[i+1]);
		#ifndef CFG_SPEED
			if (!m_ComponentInstanceVector[i])
			{
				LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::UpdateWorldMatrix::m_ComponentInstanceVector[%d] == NULL", i);
				continue;
			}
		#endif

			m_ComponentInstanceVector[i]->UpdateWorldMatrix(worldMatrix, velocity, planePos, planeNormal);			
		}
	#ifndef CFG_SPEED
		if (!m_ComponentInstanceVector[i])
		{
			LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::UpdateWorldMatrix::m_ComponentInstanceVector[%d] == NULL", i);
		}
		else
	#endif
			m_ComponentInstanceVector[i]->UpdateWorldMatrix(worldMatrix, velocity, planePos, planeNormal);			
	}
}

void I_SpecialFXInstance::Stop()
{
	if (m_Started)
	{
		for (int i = 0; i < m_ComponentInstanceVector.Size(); i++)
		{
		#ifndef CFG_SPEED
			if (!m_ComponentInstanceVector[i])
			{
				LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::Stop::m_ComponentInstanceVector[%d] == NULL", i);
				continue;
			}
		#endif

			if (m_ComponentDataInstanceVector[i].m_Started)
			{
				m_ComponentInstanceVector[i]->Stop();
				m_ComponentDataInstanceVector[i].m_Started = false;
			}
		}
		m_Started = false;
	}
}

bool I_SpecialFXInstance::Update( F32 time )
{
	if (!m_Started)
		return false;

	PERF_EVENT("I_SpecialFXInstance::Update");
	bool IsFinish = true;
	if(m_ComponentInstanceVector.Size())
	{
		int size = m_ComponentInstanceVector.Size() - 1;
		int i;
		for (i = 0; i < size; i++)
		{
		#ifndef CFG_SPEED
			if (!m_ComponentInstanceVector[i])
			{
				LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::Update::m_ComponentInstanceVector[%d] == NULL", i);
				continue;
			}
		#endif
			DCBT(0, m_ComponentInstanceVector[i+1]);

			if (time >= m_ComponentDataInstanceVector[i].m_StartOffset)
			{
				if (!m_ComponentDataInstanceVector[i].m_Started)
				{
					m_ComponentInstanceVector[i]->Start();
					m_ComponentDataInstanceVector[i].m_Started = true;
				}

				IsFinish &= m_ComponentInstanceVector[i]->Update((time - m_ComponentDataInstanceVector[i].m_StartOffset) * m_ComponentDataInstanceVector[i].m_SpeedFactor );
			}
			
		}
		if (time >= m_ComponentDataInstanceVector[i].m_StartOffset)
		{
			if (!m_ComponentDataInstanceVector[i].m_Started)
			{
				m_ComponentInstanceVector[i]->Start();
				m_ComponentDataInstanceVector[i].m_Started = true;
			}

			IsFinish &= m_ComponentInstanceVector[i]->Update((time - m_ComponentDataInstanceVector[i].m_StartOffset) * m_ComponentDataInstanceVector[i].m_SpeedFactor );
		}
	}

	return IsFinish;
}

F32 I_SpecialFXInstance::GetDuration()
{
	F32 duration = 1000000.f;
	for (int i = 0; i < m_ComponentInstanceVector.Size(); i++)
	{
	#ifndef CFG_SPEED
		if (!m_ComponentInstanceVector[i])
		{
			LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::GetDuration::m_ComponentInstanceVector[%d] == NULL", i);
			continue;
		}
	#endif

		if (duration > (m_ComponentDataInstanceVector[i].m_StartOffset + m_ComponentInstanceVector[i]->GetDuration()))
		{
			duration = m_ComponentDataInstanceVector[i].m_StartOffset + m_ComponentInstanceVector[i]->GetDuration();
		}
	}

	return duration;
}

void I_SpecialFXInstance::SetParameter( U32 paramID, void* pData )
{
	for (int i = 0; i < m_ComponentInstanceVector.Size(); i++)
	{
	#ifndef CFG_SPEED
		if (!m_ComponentInstanceVector[i])
		{
			LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::Update::m_ComponentInstanceVector[%d] == NULL", i);
			continue;
		}
	#endif
		m_ComponentInstanceVector[i]->SetParameter(paramID, pData);	
	}
}

void I_SpecialFXInstance::UpdateIntensity( F32 intensity )
{
	if (!m_Started)
		return;

	for (int i = 0; i < m_ComponentInstanceVector.Size(); i++)
	{
	#ifndef CFG_SPEED
		if (!m_ComponentInstanceVector[i])
		{
			LOGERROR(g_SpecialFXLog, "I_SpecialFXInstance::Update::m_ComponentInstanceVector[%d] == NULL", i);
			continue;
		}
	#endif
		m_ComponentInstanceVector[i]->UpdateIntensity( intensity );	
	}
}

C_SpecialFXManager* SpecialFXManager()
{
  return g_SpecialFXManager;
}

const char *g_sfxLanguageCodes[] = { "en", "fr", "de", "it", "es", "hu", "ru", "pl", "cs", "sc", "tc", "ja", "ko", NULL };
const char* g_curLanguageCode = NULL;

bool SfxIsLanguageCode(const char* szCode)
{
	for (const char** curCode = g_sfxLanguageCodes; *curCode; ++curCode)
	{
		if (!strcmp(*curCode, szCode))
			return true;
	}

	return false;
}

bool SfxIgnoreLanguageCode(const char* szCode)
{
	BIZ_ASSERT(g_curLanguageCode);

	if (strcmp(g_curLanguageCode, szCode))
		return true;

	return false;
}

void SfxSetLanguageCode(const char* szCode)
{
	g_curLanguageCode = szCode;
}

}
