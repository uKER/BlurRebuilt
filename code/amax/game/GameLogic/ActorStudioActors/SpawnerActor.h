#ifndef SPAWNERACTOR_H
#define SPAWNERACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

//=========================================================================
//						SPAWNER ACTOR CLASSES
//=========================================================================


// Game::E_ActorTypeId_Spawner


namespace Actors
{
	
class SpawnerCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_Spawner;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_Spawner;}

/*
	// Creation data
	XtHandle<Game::DynamicObjectDef>				m_DynamicObjectDef;

	// Event messages
	ActorMsg::C_XtSendMessage						m_OnPlayerCollision;

	// Components
	Components::C_MovementCreateInfo*				m_pSorceryMovementComponentCreate;

	Component::I_ComponentCreator::C_DataRef		m_pMovementCreator;
	const Components::C_MovementCreateInfo*			GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
	Components::C_MovementCreateInfo*				GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}
*/

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};


//=========================================================================


class SpawnerActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	SpawnerActor();
	virtual ~SpawnerActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	void								Update();								// Update logic called each frame
	void								KillThis();								// Cleans up any children etc before killing itself


	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
	const Maths::C_Vertex				GetDir()		const {return m_WorldMatrix.AZ();}
	const Maths::C_Vertex				GetUp()			const {return m_WorldMatrix.AY();}
	const Maths::C_Vertex				GetRight()		const {return m_WorldMatrix.AX();}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_Spawner; }
	static const Btl::String32			TypeName()		{ return "Spawner"; }
	const SpawnerCreationData*			GetCreationData() const {return static_cast<const SpawnerCreationData*>(C_GameActor::GetCreationData().GetData());}


	Maths::HMat43						m_WorldMatrix;

	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
};

}// End namespace Actors


#endif // SPAWNERACTOR_H
