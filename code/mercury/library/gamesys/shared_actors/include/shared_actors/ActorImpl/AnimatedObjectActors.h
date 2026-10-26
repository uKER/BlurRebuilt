#ifndef ANIMATEDOBJECTACTORS_H
#define ANIMATEDOBJECTACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/AnimatedObjectActorsCreate.h>

namespace Components
{
	class C_MayaAnimationController;
};

namespace Actors
{

class C_AnimatedObjectActor : public C_SharedActorBase
{
public:
	C_AnimatedObjectActor();
	~C_AnimatedObjectActor();

	static const Actor::C_ActorTypeId      TypeId()		{return E_ActorTypeId_AnimatedObjectActor;}
	static const Actor::C_ActorTypeName    TypeName()		{return "AnimatedObjectActor";}

	const Maths::HMat43 &			GetMatrix() const		{return m_WorldMatrix;}

	const C_AnimatedObjectActorCreate*		GetCreationData() const {return static_cast<const C_AnimatedObjectActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}
	
	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage); // from C_MessageHandler

	// serialisation
	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	virtual bool					InitialiseImpl();	// from Actor::C_Actor
	//virtual bool					PrimeImpl();		// from Actor::C_Actor
	virtual bool					CreateImpl();		// from Actor::C_Actor

private:
	void	Update(F32 deltaTime);
	bool	DoesNotHaveMovementComponent() const;
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);
	
	Btl::Vector<Actor::C_ActorHandle>	m_Children;
	Actor::C_ActorHandle				m_NotificationActor;
	Maths::HMat43						m_WorldMatrix;
};

}//namespace Actors

#endif