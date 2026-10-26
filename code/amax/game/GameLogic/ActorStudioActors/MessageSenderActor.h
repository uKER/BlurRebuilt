#ifndef MESSAGESENDERACTOR_H
#define MESSAGESENDERACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>




//=========================================================================
//						MESSAGE SENDER ACTOR CLASSES
//=========================================================================



namespace Actors
{
	
class MessageSenderCreationData : public C_GameActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_MessageSender;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_MessageSender;}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};


//=========================================================================


class MessageSenderActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	MessageSenderActor();
	virtual ~MessageSenderActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
//	void								Update();								// Update logic called each frame
//	void								KillThis();								// Cleans up any children etc before killing itself


	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
	const Maths::C_Vertex				GetDir()		const {return m_WorldMatrix.AZ();}
	const Maths::C_Vertex				GetUp()			const {return m_WorldMatrix.AY();}
	const Maths::C_Vertex				GetRight()		const {return m_WorldMatrix.AX();}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_MessageSender; }
	static const Btl::String32			TypeName()		{ return "Message Sender"; }
	const MessageSenderCreationData*	GetCreationData() const {return static_cast<const MessageSenderCreationData*>(C_GameActor::GetCreationData().GetData());}


	Maths::HMat43						m_WorldMatrix;

	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
};

}// End namespace Actors


#endif // MESSAGESENDERACTOR_H
