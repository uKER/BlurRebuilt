#ifndef TRIGGERREGIONACTOR_H
#define TRIGGERREGIONACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <btl/Containers/containers.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>
#include <resource/Listener.h>
#include <gravity/Triggers/TriggerSystem.h>

#include <GameLogic\ActorStudioActors\ResourceTypes.h>


namespace Gravity
{
	class C_Trigger;
}




//=========================================================================
//						TRIGGER REGION ACTOR CLASSES
//=========================================================================


namespace Actors
{

	
class TriggerRegionCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_TriggerRegion;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_TriggerRegion;}

	XtBoundingBox						box;						// Collision region

	Game::E_TRIGGER_TYPE				m_TriggerType;				// Info that gets sent to vehicles
	S32									m_RefNumber;

	bool								m_DestroyWhenTriggered;		// Will only trigger once then kill itself
	
	bool								m_PlayerControlled;			// Trigger conditions
	bool								m_AiControlled;
	S32									m_MinLap;
	S32									m_MaxLap;

	// Event messages
	ActorMsg::C_XtSendMessage			m_OnTriggered;
	ActorMsg::C_XtSendMessage			m_OnExited;

	ActorMsg::C_XtActorMessageBase		*m_VehicleMessage;
	ActorMsg::C_XtActorMessageBase		*m_VehicleMessageOnExit;

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};


//=========================================================================


class TriggerRegionActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	TriggerRegionActor();
	virtual ~TriggerRegionActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	void								CheckRegion(Actor::C_ActorHandle vehicle_handle, S8 index, Maths::HVec3 pos, S32 event_type);
	bool								CheckTriggered(Actor::C_ActorHandle vehicle_handle);		// Checks vehicle against trigger conditions
	void								KillThis();										// Cleans up any children etc before killing itself

	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_TriggerRegion; }
	static const Btl::String32			TypeName()		{ return "Trigger Region"; }
	const TriggerRegionCreationData*	GetCreationData() const {return static_cast<const TriggerRegionCreationData*>(C_GameActor::GetCreationData().GetData());}

	void								DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;

	Maths::HMat43						GetElementMatrix() const	{ return ( m_pTriggerObject && m_pTriggerObject->m_pTriggerShape ) ? m_pTriggerObject->m_pTriggerShape->GetElementMatrix() : Maths::HMat43(); }

	Maths::HMat43						m_WorldMatrix;

	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();
	virtual void						KillImpl();

private:
	bool								m_Active;
	bool								m_Selected;
	Gravity::C_Trigger*					m_pTriggerObject;
};

}// End namespace Actors


#endif // TRIGGERREGIONACTOR_H
