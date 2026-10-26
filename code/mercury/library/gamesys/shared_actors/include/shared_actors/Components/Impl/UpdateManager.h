#ifndef UPDATE_MANAGER_H
#define UPDATE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/StaggerManager.h>

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Interfaces/I_UpdateManager.h>
#include <shared_actors/Components/Creation/UpdateManagerCreate.h>
#include <shared_actors/Utility/ActorDistanceItem.h>


namespace Components
{

class C_UpdateManagerComponent : public I_UpdateManagerComponent
{
public:
	C_UpdateManagerComponent();
	virtual ~C_UpdateManagerComponent();
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_UpdateManagerComponent::OnMessage(pMessage);}	

	virtual F32		GetDistanceFromSeed() const {return m_DistFromSeed;}

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	F32		m_DistFromSeed;
	U32		m_WarmUpFrames;
};

// Active Range Update Component
// -----------------------------

// Switches actor update on/off based on range from the seed
class C_ActiveRangeUpdateComponent : public C_UpdateManagerComponent
{
public:
	C_ActiveRangeUpdateComponent():m_ActiveRange(FLT_MAX),m_bOutOfRange(false),m_SeedCalcStaggerGroup(-1){}
	~C_ActiveRangeUpdateComponent();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_ActiveRange;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateManager_ActiveRange;} // from Component::I_GameComponent

	bool	OnMessage(const Messaging::C_MessageBase *pMessage );		// message handler

	const C_ActiveRangeUpdateComponentCreator	*GetCreationData(void) const {return static_cast<const C_ActiveRangeUpdateComponentCreator *>(GetCreator().GetData());}
	
	void	SetActiveRange(F32 range){m_ActiveRange = range;}
	void	SetSeedCalcStaggerLevel(Util::E_StaggerLevel level);

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);


protected:
	F32		m_ActiveRange;
	bool	m_bOutOfRange;
	S32		m_SeedCalcStaggerGroup;

	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent
private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

};

// Stagger Update Component
// ------------------------

// Staggers actor update based on range from the seed

class C_StaggerUpdateComponent : public C_ActiveRangeUpdateComponent
{
public:
	C_StaggerUpdateComponent():m_UpdateStaggerGroup(-1){}
	~C_StaggerUpdateComponent();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_Stagger;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateManager_Stagger;} // from Component::I_GameComponent

	bool	OnMessage(const Messaging::C_MessageBase *pMessage );		// message handler

	const C_StaggerUpdateComponentCreator	*GetCreationData(void) const {return static_cast<const C_StaggerUpdateComponentCreator *>(GetCreator().GetData());}
	void	SetUpdateStaggerLevel(Util::E_StaggerLevel level);

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	S32		m_UpdateStaggerGroup;

	virtual bool		CreateImpl(); // from Component::I_GameComponent
};

// Counter Update Component
// ------------------------

// Updates based on how many objects of the same group ID are between it and the seed
class C_CounterUpdateComponent : public C_UpdateManagerComponent
{
public:
	C_CounterUpdateComponent():m_GroupId(-1),m_UpdateOrderLimit(0),m_bOutOfRange(false){}
	~C_CounterUpdateComponent(){RemoveSubscriber(ActorMsg::E_GameActorMsg_SeedRequestPosition);}

	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateManager_Counter;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateManager_Counter;} // from Component::I_GameComponent

	bool	OnMessage(const Messaging::C_MessageBase *pMessage );		// message handler

	const C_CounterUpdateComponentCreator	*GetCreationData(void) const {return static_cast<const C_CounterUpdateComponentCreator *>(GetCreator().GetData());}

	S32		m_GroupId;
	U32		m_UpdateOrderLimit;
	bool	m_bOutOfRange;

	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);
protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent
private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

};


// Update Seeds
// ------------




class C_UpdateSeedSingularComponent : public I_UpdateSeedComponent
{
public:
	C_UpdateSeedSingularComponent();
	virtual ~C_UpdateSeedSingularComponent();

	static Component::C_ComponentTypeId		ClassType() {return Components::E_UpdateSeed_Singular;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_UpdateSeed_Singular;} // from Component::I_GameComponent

	bool	OnMessage(const Messaging::C_MessageBase *pMessage );		// message handler

	const C_UpdateSeedSingularCreator	*GetCreationData(void) const {return static_cast<const C_UpdateSeedSingularCreator *>(GetCreator().GetData());}

	// factory methods
	static Component::I_GameComponent*	Construct(void) 
										{
											if(m_pSingleInstance == NULL)
												m_pSingleInstance = new C_UpdateSeedSingularComponent;
											// return error on double construction?
											return m_pSingleInstance;
										}
	static bool							Destroy(Component::I_GameComponent *component)
										{ 
											if(component != m_pSingleInstance)
												return false;

											delete component; 
											m_pSingleInstance = NULL;
											return true;
										}
#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Component::I_GameComponent
#endif


	static C_UpdateSeedSingularComponent *	m_pSingleInstance;

	typedef Btl::Map<S32, Actor::C_ActorDistanceItem::C_List> C_CountedItemListMap;

	C_CountedItemListMap*					m_pCountedItemListMap;
	U32										m_FrameCounter;
protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

};

}

#endif

