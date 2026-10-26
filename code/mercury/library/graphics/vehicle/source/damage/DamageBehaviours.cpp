/////////////////////////////////////////////////////////////
// DamageBehaviours
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Damage/DamageBehaviours.h"
#include "Vehicle/Component.h"
#include "Vehicle/Input.h"
#include "Vehicle/Material/Material.h"
#include "Vehicle/Damage/CollisionInfo.h"

#include <System/Misc/Random.h>

Debug::Logger& g_DamageBehaviourLog = Debug::GetLogger("mercury.vehicle.damage.behaviours");

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

namespace Damage
{

/* 
===============================================================
					class C_BehaviourBase

NOTES: 
===============================================================
*/

/*
===============
NAME: C_BehaviourBase

NOTE: Constructor
===============
*/
C_BehaviourBase::C_BehaviourBase()
{
	m_eType		= E_BehaviourType_None;

	m_State		= 0;

	m_pPart		= NULL;	
}

/*
===============
NAME: SetNodePtr

NOTE: Sets the component node pointer
===============
*/
void C_BehaviourBase::SetNodePtr(C_Part* p_pPart)
{
	m_pPart		= p_pPart;
}

/*
===============
NAME: GetNodePtr

NOTE: Gets component node pointer
===============
*/
C_Part* C_BehaviourBase::GetNodePtr()
{
	return m_pPart;
}

/* 
===============================================================
					class C_BehaviourLightCluster

NOTES: 
===============================================================
*/

/*
===============
NAME: C_BehaviourLightCluster

NOTE: Constructor
===============
*/
C_BehaviourLightCluster::C_BehaviourLightCluster()
{
	m_eType					= E_BehaviourType_LightCluster;

	m_pPart					= NULL;

	m_fImpactSizeThreshold	= 0.0f;

	m_bBroken				= false;
}

/*
===============
NAME: Update

NOTE: Update behaviour
===============
*/
void C_BehaviourLightCluster::Update(C_ModelInput* p_pModelInput, float p_fTime)
{

}

//Debug::Float g_debugLightDamageDimScale("Vehicle.Damage.Lights.DimScale", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightDamageFlickerTime("Vehicle.Damage.Lights.FlickerTime", 1.0f, Debug::Range(0.0f, 10.0f), Debug::Step(0.001f));
Debug::Float g_debugLightDamageFlickerFrequency("Vehicle.Damage.Lights.FlickerFrequency", 0.2f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: HandleImpactDamage

NOTE: Handles impact damage
===============
*/
E_DefferedEvent C_BehaviourLightCluster::HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter)
{
	if (p_fImpactSize >= m_fImpactSizeThreshold && !m_bBroken)
	{
		for (S32 iChild = 0; iChild < m_pChildren.Size(); iChild++)
		{
			if (!m_pChildren[iChild] || !m_pChildren[iChild]->m_pDamageBehaviour || 
				m_pChildren[iChild]->m_pDamageBehaviour->GetType() != E_BehaviourType_Light)
				continue;

			((C_BehaviourLight*)m_pChildren[iChild]->m_pDamageBehaviour)->Break();
			((C_BehaviourLight*)m_pChildren[iChild]->m_pDamageBehaviour)->Flicker((float)g_debugLightDamageFlickerTime);
		}

		m_bBroken	= true;
	}

	return E_DefferedEvent_Unknown;
}

/*
===============
NAME: Repair

NOTE: Handles repair
===============
*/
void C_BehaviourLightCluster::Repair(float p_fHealthLevel)
{

}

/*
===============
NAME: SetState

NOTE: Sets damage state
===============
*/
void C_BehaviourLightCluster::SetState(U8 p_State)
{
}

/*
===============
NAME: AddChild

NOTE: Adds a child to the light cluster
===============
*/
void C_BehaviourLightCluster::AddChild(C_Part* p_pChildNode)
{
	if (!p_pChildNode)
		return;

	m_pChildren.PushBack(p_pChildNode);
}

/* 
===============================================================
					class C_BehaviourLight

NOTES: 
===============================================================
*/

Debug::Float g_debugLightPulseMinNoise("Vehicle.Damage.Lights.MinNoise", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightPulseMaxNoise("Vehicle.Damage.Lights.MaxNoise", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightFlickerMinValue("Vehicle.Damage.Lights.FlickerMinValue", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightFlickerMaxScale("Vehicle.Damage.Lights.FlickerMaxScale", 1.0f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

Debug::Float g_debugLightsTimeTilNextEvent("Vehicle.Damage.Lights.SafeTime", 2.0f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));

/*
===============
NAME: C_BehaviourLight

NOTE: Constructor
===============
*/
C_BehaviourLight::C_BehaviourLight()
{
	m_eType					= E_BehaviourType_Light;

	m_pPart					= NULL;

	m_bBroken				= false;
	m_bFlickering			= false;
	m_bInitialState			= false; 

	m_fFlickerTime			= 0.0f;
	m_fFlickerDuration		= 0.0f;
	m_fIntensityScale		= 1.0f;
	m_fInitialIntensity		= 0.0f;
	m_fTimeSinceLastEvent	= (float)g_debugLightsTimeTilNextEvent;
}

/*
===============
NAME: Update

NOTE: Update behaviour
===============
*/
static Util::C_Random g_BehaviourLightRandom;

void C_BehaviourLight::Update(C_ModelInput* p_pModelInput, float p_fTime)
{
	if (!m_pPart || !m_pPart->m_pMaterialParameters || m_pPart->m_pMaterialParameters->m_Type != E_MaterialType_Light)
		return;

	m_fTimeSinceLastEvent += p_fTime;

	if (m_fTimeSinceLastEvent > (float)g_debugLightsTimeTilNextEvent)
		m_fTimeSinceLastEvent = (float)g_debugLightsTimeTilNextEvent;

	C_MaterialLight* pMaterial = (C_MaterialLight*)m_pPart->m_pMaterialParameters;
	m_pPart->m_MaterialParametersUpdate = true; //Update every frame... Don't really want to be doing this...
	
	if (m_bBroken)
	{
		float fNoise = (float)g_BehaviourLightRandom.FRand((float)g_debugLightPulseMinNoise, (float)g_debugLightPulseMaxNoise);
		pMaterial->m_Noise = fNoise;
	}
	else		
		pMaterial->m_Noise = 1.0f;

	if (!m_pPart || !m_bFlickering)
		return;

	bool reset		= m_fFlickerTime >= m_fFlickerDuration ? true : false;
	if (reset)
		m_bFlickering = false;
	bool getState	= m_fFlickerTime == 0.0f ? true : false;

	float rpercent = (float)g_BehaviourLightRandom.FRand(0.0f, 1.0f);
	
	if (getState)
		m_fInitialIntensity = pMaterial->m_DefaultIntensity;
	if (reset)
		pMaterial->m_Intensity = m_fInitialIntensity;
	
	if (reset)
		return;

	if (rpercent <= (float)g_debugLightDamageFlickerFrequency)
	{
		float fInt = (float)g_BehaviourLightRandom.FRand((float)g_debugLightFlickerMinValue, m_fInitialIntensity * (float)g_debugLightFlickerMaxScale);
		
		pMaterial->m_Intensity = fInt;
	}

	m_fFlickerTime += p_fTime;	
}

Debug::Float g_debugLightDamageFlickerThreshold("Vehicle.Damage.Lights.FlickerThreshold", 0.15f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightDamageBreakThreshold("Vehicle.Damage.Lights.BreakThreshold", 0.25f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugLightDamageKnockOutThreshold("Vehicle.Damage.Lights.KnockOutThreshold", 0.5f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: HandleImpactDamage

NOTE: Handles impact damage
===============
*/
E_DefferedEvent C_BehaviourLight::HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter)
{
	if (p_fImpactSize >= g_debugLightDamageFlickerThreshold)
		Flicker((float)g_debugLightDamageFlickerTime);

	E_DefferedEvent effect = E_DefferedEvent_Unknown;

	if (p_fImpactSize >= g_debugLightDamageKnockOutThreshold && m_bBroken)
	{
		if (p_pDamageFilter->m_LightKnockOut)
			if (KnockOut())
				effect = E_DefferedEvent_LightSmash;
	}
	else if (p_fImpactSize >= g_debugLightDamageBreakThreshold && !m_bBroken)
	{
		if (p_pDamageFilter->m_LightBreak)
			if (Break())
				effect = E_DefferedEvent_LightCrack;
	}

	return effect;
}

/*
===============
NAME: SetState

NOTE: Sets damage state
===============
*/
void C_BehaviourLight::SetState(U8 p_State)
{
	if (p_State == 0)
	{
		m_bBroken		= false;
		m_bKnockedOut	= false;
		m_bFlickering	= false;
		m_State			= 0;
	}
	else if (p_State == 1)
	{
		Break(true);
	}
	else if (p_State == 2)
	{
		Break(true);
		KnockOut(true);
	}
}


/*
===============
NAME: KnockOut

NOTE: Bust the headlight completely
===============
*/
bool C_BehaviourLight::KnockOut(bool force)
{
	if (!m_pPart || !m_pPart->m_pMaterialParameters || m_pPart->m_pMaterialParameters->m_Type != E_MaterialType_Light || !m_bBroken)
		return false;

	if (m_fTimeSinceLastEvent < (float)g_debugLightsTimeTilNextEvent && !force)
		return false;

	C_MaterialLight* pMaterial = (C_MaterialLight*)m_pPart->m_pMaterialParameters;

	m_pPart->m_Damage.m_DamageLevel	= 1.0f;
	pMaterial->m_Damaged			= true;
	pMaterial->m_KnockedOut			= true;
	m_pPart->m_MaterialParametersUpdate = true;

	m_bKnockedOut					= true;

	m_State							= 2;

	m_fTimeSinceLastEvent			= 0.0f;

	return true;
}

/*
===============
NAME: Break

NOTE: Break the light
===============
*/
bool C_BehaviourLight::Break(bool force)
{
	if (!m_pPart || !m_pPart->m_pMaterialParameters || m_pPart->m_pMaterialParameters->m_Type != E_MaterialType_Light || m_bBroken)
		return false;

	if (m_fTimeSinceLastEvent < (float)g_debugLightsTimeTilNextEvent && !force)
		return false;

	C_MaterialLight* pMaterial = (C_MaterialLight*)m_pPart->m_pMaterialParameters;

	m_pPart->m_Damage.m_DamageLevel	= 1.0f;
	pMaterial->m_Damaged			= true;
	//pMaterial->m_Intensity			= pMaterial->m_DefaultIntensity * (float)g_debugLightDamageDimScale;					
	m_pPart->m_MaterialParametersUpdate = true;

	m_bBroken						= true;

	m_State							= 1;

	m_fTimeSinceLastEvent			= 0.0f;

	return true;
}

Debug::Float g_debugLightRepairHealthThreshold("Vehicle.Damage.Behaviours.Lights.RepairHealthThreshold", 0.75f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: Flicker

NOTE: Flicker lights for a time
===============
*/
void C_BehaviourLight::Flicker(float p_fDuration)
{
	if (!m_pPart || !m_pPart->m_pMaterialParameters || m_pPart->m_pMaterialParameters->m_Type != E_MaterialType_Light || m_bFlickering)
		return;

	m_fFlickerDuration	= p_fDuration;
	m_fFlickerTime		= 0.0f;
	m_bFlickering		= true;
}

/*
===============
NAME: Repair

NOTE: Handles repair
===============
*/
void C_BehaviourLight::Repair(float p_fHealthLevel)
{
	if (!m_pPart || !m_pPart->m_pMaterialParameters || m_pPart->m_pMaterialParameters->m_Type != E_MaterialType_Light)
		return;

	if (p_fHealthLevel >= g_debugLightRepairHealthThreshold && m_bBroken)
	{
		m_pPart->SetPermanentVisibility(1);

		m_bBroken = false;
	}

	C_MaterialLight* pMaterial = (C_MaterialLight*)m_pPart->m_pMaterialParameters;

	m_pPart->m_Damage.m_DamageLevel	= 1.0f - p_fHealthLevel;
	pMaterial->m_Damaged			= false;
	pMaterial->m_KnockedOut			= false;
	m_pPart->m_MaterialParametersUpdate = true;

	m_bBroken						= false;
	m_bKnockedOut					= false;

	m_State							= 0;
	m_fTimeSinceLastEvent			= 0.0f;
}

/* 
===============================================================
					class C_BehaviourGlassSmash

NOTES: 
===============================================================
*/

/*
===============
NAME: C_BehaviourGlassSmash

NOTE: Constructor
===============
*/
C_BehaviourGlassSmash::C_BehaviourGlassSmash()
{
	m_eType					= E_BehaviourType_GlassSmash;

	m_pPart					= NULL;

	m_State					= 0;

	m_bBroken				= false;
}

/*
===============
NAME: Update

NOTE: Update behaviour
===============
*/
void C_BehaviourGlassSmash::Update(C_ModelInput* p_pModelInput, float p_fTime)
{

}

Debug::Float g_debugGlassSmashOutImpactSize("Vehicle.Damage.Behaviours.GlassSmash.ImpactSize", 0.85f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugGlassSmashRepairHealthThreshold("Vehicle.Damage.Behaviours.GlassSmash.RepairHealthThreshold", 0.75f, Debug::Range(0.0f, 1.0f), Debug::Step(0.001f));

/*
===============
NAME: HandleImpactDamage

NOTE: Handles impact damage
===============
*/
E_DefferedEvent C_BehaviourGlassSmash::HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter)
{
	//if (!m_pPart || p_fImpactSize < (float)g_debugGlassSmashOutImpactSize || m_bBroken)
		return E_DefferedEvent_Unknown;

	return Break();
}

/*
===============
NAME: HandleImpactDamage

NOTE: Handles impact damage
===============
*/
E_DefferedEvent C_BehaviourGlassSmash::Break()
{
	if (!m_pPart || m_bBroken)
		return E_DefferedEvent_Unknown;

	m_bBroken	= true;

	m_pPart->SetPermanentVisibility(0);

	return E_DefferedEvent_WindowSmash;
}

/*
===============
NAME: Repair

NOTE: Handles repair
===============
*/
void C_BehaviourGlassSmash::Repair(float p_fHealthLevel)
{
	if (p_fHealthLevel >= g_debugGlassSmashRepairHealthThreshold && m_bBroken)
	{
		m_pPart->SetPermanentVisibility(1);

		m_bBroken = false;
	}
}

/*
===============
NAME: SetState

NOTE: SetState
===============
*/
void C_BehaviourGlassSmash::SetState(U8 p_State)
{

}

} //namespace Damage

} //namespace Vehicle
