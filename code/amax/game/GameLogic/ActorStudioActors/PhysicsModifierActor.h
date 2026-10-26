#ifndef PHYSICSMODIFIERACTOR_H
#define PHYSICSMODIFIERACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <btl/Containers/containers.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>



namespace Gravity
{
	class C_Trigger;
}




//=========================================================================
//						PHYSICS MODIFIER ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class PhysicsModifierCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_PhysicsModifier;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_PhysicsModifier;}

	XtBoundingBox						box;

	bool								m_Deactivate;
	F32									m_Force;

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};


//=========================================================================


class PhysicsModifierActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	PhysicsModifierActor();
	virtual ~PhysicsModifierActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
//	void								Update();								// Update logic called each frame
	void								KillThis();								// Cleans up any children etc before killing itself


	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_PhysicsModifier; }
	static const Btl::String32			TypeName()		{ return "Physics Modifier"; }
	const PhysicsModifierCreationData*	GetCreationData() const {return static_cast<const PhysicsModifierCreationData*>(C_GameActor::GetCreationData().GetData());}


	Maths::HMat43						m_WorldMatrix;

	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	bool								m_Active;
	Gravity::C_Trigger*					m_pTriggerObject;
};

}// End namespace Actors


#endif // PHYSICSMODIFIERACTOR_H
