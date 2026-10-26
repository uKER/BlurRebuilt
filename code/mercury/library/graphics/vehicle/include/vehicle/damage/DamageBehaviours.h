/////////////////////////////////////////////////////////////
// DamageBehaviours
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_DAMAGE_BEHAVIOURS_H__
#define __VEHICLE_DAMAGE_BEHAVIOURS_H__

#include "Vehicle/Damage/DamageLayer.h"

namespace Vehicle
{

class C_Part;
class C_ModelInput;

namespace Damage
{

enum E_BehaviourType
{
	E_BehaviourType_None,

	E_BehaviourType_LightCluster,
	E_BehaviourType_Light,
	E_BehaviourType_GlassSmash,

	E_BehaviourType_Max,
};

/* 
===============================================================
					class C_BehaviourBase

NOTES: 
===============================================================
*/
class C_BehaviourBase
{
public:

	C_BehaviourBase();

	//! Update behaviour
	virtual void Update(C_ModelInput* p_pModelInput, float p_fTime) = 0;

	//! Sets the component node pointer
	void SetNodePtr(C_Part* p_pNode);

	//! Gets component node pointer
	C_Part* GetNodePtr();

	E_BehaviourType GetType() { return m_eType; }

	//! Handles impact damage
	virtual E_DefferedEvent HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter) = 0;
	
	//! Handles repair
	virtual void Repair(float p_fHealthLevel) = 0;

	virtual void SetState(U8 p_State) = 0;

protected:
	
	E_BehaviourType			m_eType;								//!< Behaviour type	

	U8						m_State;

	C_Part*					m_pPart;								//!< Pointer back to the part this belongs to
};

/* 
===============================================================
					class C_BehaviourLightCluster

NOTES: 
===============================================================
*/
class C_BehaviourLightCluster : public C_BehaviourBase
{
public:

	C_BehaviourLightCluster();

	//! Update behaviour
	void Update(C_ModelInput* p_pModelInput, float p_fTime);

	//! Handles impact damage
	E_DefferedEvent HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter);

	//! Handles repair
	void Repair(float p_fHealthLevel);

	//! Adds a child to the light
	void AddChild(C_Part* p_pChildNode);

	//! Set damage state
	void SetState(U8 p_State);

public:
	
	float							m_fImpactSizeThreshold;		//!< Impact size needed to break lights

	bool							m_bBroken;					//!< Is the light broken

	Btl::Vector<C_Part*>			m_pChildren;				//!< List of lights that belong to this cluster
};

/* 
===============================================================
					class C_BehaviourLight

NOTES: 
===============================================================
*/
class C_BehaviourLight : public C_BehaviourBase
{
public:

	C_BehaviourLight();

	//! Update behaviour
	void Update(C_ModelInput* p_pModelInput, float p_fTime);

	//! Handles impact damage
	E_DefferedEvent HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter);

	//! Handles repair
	void Repair(float p_fHealthLevel);

	//! Break the light
	bool Break(bool force = false);

	//! Flicker lights for a time
	void Flicker(float p_fDuration);

	//! Bust the headlight completely
	bool KnockOut(bool force = false);

	//! Set damage state
	void SetState(U8 p_State);

public:
	
	
	float							m_fFlickerTime;				//!< How long have been flicerking for
	float							m_fFlickerDuration;			//!< How long to flicker for
	float							m_fIntensityScale;			//!< Damage intensity scale
	float							m_fInitialIntensity;
	float							m_fTimeSinceLastEvent;

	bool							m_bInitialState;
	
	bool							m_bBroken;					//!< Is the light broken
	bool							m_bFlickering;				//!< Is flickering
	bool							m_bKnockedOut;				//!< Has the headlight actually bust
};

/* 
===============================================================
					class C_BehaviourGlassSmash

NOTES: 
===============================================================
*/
class C_BehaviourGlassSmash : public C_BehaviourBase
{
public:

	C_BehaviourGlassSmash();

	//! Update behaviour
	void Update(C_ModelInput* p_pModelInput, float p_fTime);

	//! Handles impact damage
	E_DefferedEvent HandleImpactDamage(float p_fImpactSize, Vehicle::C_DamageFilter* p_pDamageFilter);

	//! Handles repair
	void Repair(float p_fHealthLevel);
	
	void SetState(U8 p_State);

	bool Broken() { return m_bBroken; }

	E_DefferedEvent Break();

protected:

	bool					m_bBroken;
};

} //namespace Damage

} //namespace Vehicle

#endif

