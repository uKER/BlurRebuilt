#ifndef SYSTEM_ACTORS_H
#define SYSTEM_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/ActorCreation/SystemActorsCreate.h>

namespace Actors
{

// Null Actor
class C_NullActor : public C_SharedActorBase
{
public:
	C_NullActor(){}

	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_NullActor; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "NullActor"; }

	const C_NullActorCreate*	GetCreationData() const {return static_cast<const C_NullActorCreate*>(C_Actor::GetCreationData().GetData());}
	virtual bool	InitialiseImpl(); // from Actor::C_Actor
	virtual const Maths::HMat43	&	GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex	GetPosition(void) const {return m_WorldMatrix.AW();}

private:
	Maths::HMat43	m_WorldMatrix;
};

// Proxy Actor
class C_ProxyActor : public C_SharedActorBase
{
public:
	C_ProxyActor(){}
	bool							OnMessage(const Messaging::C_MessageBase *pMessage );

	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_ProxyActor; }
	static const Actor::C_ActorTypeName	TypeName()	{ return "ProxyActor"; }

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

	const C_ProxyActorCreate*	GetCreationData() const {return static_cast<const C_ProxyActorCreate*>(C_Actor::GetCreationData().GetData());}

	virtual const Maths::HMat43	&	GetMatrix(void) const;
	virtual const Maths::C_Vertex	GetPosition(void) const;

protected:
	virtual bool	InitialiseImpl(); // from Actor::C_Actor

private:
	Actor::C_ActorHandle	m_hActor;

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

};

// Actor Info
class C_NodeActor : public C_SharedActorBase
{
public:
	C_NodeActor(){}

	bool							OnMessage(const Messaging::C_MessageBase *pMessage );
	const Maths::HMat43 &			GetMatrix() const {return m_WorldMatrix;}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

	static const Actor::C_ActorTypeId	TypeId()	{ return E_ActorTypeId_Node; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "Node"; }

	const C_NodeActorCreate*		GetCreationData() const {return static_cast<const C_NodeActorCreate*>(C_Actor::GetCreationData().GetData());}
#ifndef CFG_GOLD
	virtual void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	virtual bool					InitialiseImpl(); // from Actor::C_Actor

private:
	Maths::HMat43	m_WorldMatrix;
	bool			m_selected;

	void							Serialise(Sys::C_ContextualStream& stream) const;
	void							Deserialise(Sys::C_ContextualStream& stream);

};

}
#endif
