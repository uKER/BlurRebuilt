#ifndef ANIMATION_ACTORS_H
#define ANIMATION_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/AnimationActorsCreate.h>

// forward decs
namespace Components
{
	class I_AnimationController;
};

namespace Actors
{

class C_AnimationPlaybackActor : public C_SharedActorBase
{
public:
	enum E_State
	{
		E_State_Uninitialised = 0,
		E_State_Stopped,
		E_State_Playing,
		E_State_Finished
	};

	C_AnimationPlaybackActor();
	~C_AnimationPlaybackActor();

	static const Actor::C_ActorTypeId      TypeId()		{return E_ActorTypeId_AnimationPlaybackActor;}
	static const Actor::C_ActorTypeName    TypeName()	{return "AnimationPlaybackActor";}

	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}

	const C_AnimationPlaybackActorCreate*		GetCreationData() const {return static_cast<const C_AnimationPlaybackActorCreate*>(Actor::C_Actor::GetCreationData().GetData());}
	const Components::I_AnimationController *	GetAnimationController() const { return (const Components::I_AnimationController *)QueryInterface(Components::E_IF_AnimationController);}
	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage);

	const E_State					GetState() const { return m_State; }
	void							ChangeState(E_State newState);
	void							SetAnimSpeedScale(F32 newScale){m_AnimSpeedScale = newScale;}
	void							Start(void);
	void							Stop(void);
	void							Reset(void);
	void							Update(F32 timeStep);
	void							DebugOutput(bool paused = false);

	// serialisation
	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	virtual bool					InitialiseImpl();	// from Actor::C_Actor
	//virtual bool					PrimeImpl();		// from Actor::C_Actor
	virtual bool					CreateImpl();		// from Actor::C_Actor

private:
	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

	Actor::C_ActorHandle			m_NotificationActor;
	Maths::HMat43					m_WorldMatrix;
	E_State							m_State;
	Level::C_AnimTValue				m_CurrTValue;
	F32								m_AnimSpeedScale;
};

class C_AnimationSlaveController : public C_SharedActorBase
{
public:
	C_AnimationSlaveController(){}
	~C_AnimationSlaveController(){}

	static const Actor::C_ActorTypeId      TypeId()		{return E_ActorTypeId_AnimationSlaveController;}
	static const Actor::C_ActorTypeName    TypeName()	{return "AnimationSlaveController";}

	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}

	const C_AnimationSlaveControllerCreate*		GetCreationData() const {return static_cast<const C_AnimationSlaveControllerCreate*>(Actor::C_Actor::GetCreationData().GetData());}
	const Components::I_AnimationController *	GetAnimationController() const { return (const Components::I_AnimationController *)QueryInterface(Components::E_IF_AnimationController);}

	virtual bool					OnMessage(const Messaging::C_MessageBase* pMessage);

protected:
	virtual bool					InitialiseImpl();	// from Actor::C_Actor
	//virtual bool					PrimeImpl();		// from Actor::C_Actor
	virtual bool					CreateImpl();		// from Actor::C_Actor

private:
	Maths::HMat43					m_WorldMatrix;

};

}//namespace Actors

#endif