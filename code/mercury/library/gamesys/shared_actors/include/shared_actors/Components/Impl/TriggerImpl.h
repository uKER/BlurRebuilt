#ifndef TRIGGERIMPL_7EFFE49A_F52E_4510_99D3_F002557BAFA6_H
#define TRIGGERIMPL_7EFFE49A_F52E_4510_99D3_F002557BAFA6_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/components/creation/TriggerCreate.h>
#include <shared_actors/components/interfaces/I_Trigger.h>

namespace Gravity
{
	class C_Trigger; 
	class C_TriggerActor;
	class C_TriggerLayer;
};

namespace Components
{

class C_VolumeTrigger : public I_VolumeTrigger
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	virtual ~C_VolumeTrigger();

	const I_VolumeTriggerCreate* GetCreationData() const {return static_cast<const I_VolumeTriggerCreate*>(GetCreator().GetData());}

	virtual bool	OnMessage(const Messaging::C_MessageBase* pMessage); // from Messaging::C_MessageBase

protected:
	C_VolumeTrigger();

	virtual bool	InitialiseImpl();	// from Component::I_GameComponent
	virtual bool	CreateImpl();		// from Component::I_GameComponent

	const bool		IsInsideAdditionalVolumes(const Maths::C_Vertex& point) const;

	Gravity::C_TriggerLayer*	m_pTriggerLayer;
	Gravity::C_Trigger*			m_pTrigger;
private:
	virtual void	SetMatrix(const Maths::HMat43& matrix) = 0;

	Btl::Vector<Actor::C_ActorHandle>	m_AdditionalVolumes;


};

class C_BoxTrigger : public C_VolumeTrigger
{
public:
	C_BoxTrigger();
	virtual ~C_BoxTrigger();

	static Component::C_ComponentTypeId ClassType() {return E_Trigger_Box;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Trigger_Box;} // from Component::I_GameComponent

	const C_BoxTriggerCreate* GetCreationData() const {return static_cast<const C_BoxTriggerCreate*>(GetCreator().GetData());}

	// from I_VolumeTrigger
	virtual bool	IsInside(const Maths::C_Vertex& point) const;
	virtual bool	GetPositionInside(const Maths::C_Vertex& point, Maths::C_Vertex& position) const;
	virtual bool	CheckLineIntersection(const Maths::C_Vertex &lineStart, const Maths::C_Vertex &lineEnd) const;
	virtual bool	GetBoxDimensions(Maths::C_Vector &dimensions) const {dimensions = GetCreationData()->m_Size; return true;}
	virtual bool	GetSphereRadius(F32 &radius) const {return false;}


#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Component::I_GameComponent
#endif

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);
protected:
	virtual bool	CreateImpl();		// from Component::I_GameComponent

private:
	Maths::HMat43	m_InverseMatrix;

	virtual void	SetMatrix(const Maths::HMat43& matrix); // from I_VolumeTrigger

	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};

class C_SphereTrigger : public C_VolumeTrigger
{
public:
	C_SphereTrigger();
	virtual ~C_SphereTrigger();

	static Component::C_ComponentTypeId ClassType() {return E_Trigger_Sphere;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Trigger_Sphere;} // from Component::I_GameComponent

	const C_SphereTriggerCreate* GetCreationData() const {return static_cast<const C_SphereTriggerCreate*>(GetCreator().GetData());}

	// from I_VolumeTrigger
	virtual bool	IsInside(const Maths::C_Vertex& point) const;
	virtual bool	GetPositionInside(const Maths::C_Vertex& point, Maths::C_Vertex& position) const;
	virtual bool	CheckLineIntersection(const Maths::C_Vertex &lineStart, const Maths::C_Vertex &lineEnd) const;
	virtual bool	GetBoxDimensions(Maths::C_Vector &dimensions) const {return false;}
	virtual bool	GetSphereRadius(F32 &radius) const {radius = GetCreationData()->m_Sphere.Radius(); return true;}

#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const; // from Component::I_GameComponent
#endif

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	virtual bool	InitialiseImpl(); // from Component::I_GameComponent
	virtual bool	CreateImpl();		// from Component::I_GameComponent

private:
	Maths::C_Sphere	m_Sphere;
	F32				m_RadiusSquared;

	virtual void	SetMatrix(const Maths::HMat43& matrix); // from I_VolumeTrigger

	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};

// Trigger Source Components

class C_TriggerSourceVolume : public I_TriggerSourceVolume
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()
public:
	C_TriggerSourceVolume();
	virtual ~C_TriggerSourceVolume();

	virtual bool	OnMessage(const Messaging::C_MessageBase* pMessage); // from Messaging::C_MessageBase

	virtual void	SetMatrix(const Maths::HMat43& matrix) = 0;

protected:
	Gravity::C_TriggerLayer*	m_pTriggerLayer;
	Gravity::C_TriggerActor*	m_pTriggerActor;

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);
private:
	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};

class C_TriggerSourceSphere : public C_TriggerSourceVolume
{
public:
	C_TriggerSourceSphere();
	virtual ~C_TriggerSourceSphere();

	static Component::C_ComponentTypeId ClassType() {return E_TriggerSource_Sphere;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_TriggerSource_Sphere;} // from Component::I_GameComponent

	const C_TriggerSourceSphereCreate* GetCreationData() const {return static_cast<const C_TriggerSourceSphereCreate*>(GetCreator().GetData());}

	virtual void	SetMatrix(const Maths::HMat43& matrix); // from C_TriggerSourceVolume

protected:
	virtual bool	InitialiseImpl();	// from Component::I_GameComponent
	virtual bool	CreateImpl();		// from Component::I_GameComponent



private:

};

class C_TriggerSourceBox : public C_TriggerSourceVolume
{
public:
	C_TriggerSourceBox();
	virtual ~C_TriggerSourceBox();

	static Component::C_ComponentTypeId ClassType() {return E_TriggerSource_Box;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_TriggerSource_Box;} // from Component::I_GameComponent

	const C_TriggerSourceBoxCreate* GetCreationData() const {return static_cast<const C_TriggerSourceBoxCreate*>(GetCreator().GetData());}

	virtual void	SetMatrix(const Maths::HMat43& matrix); // from C_TriggerSourceVolume

protected:
	virtual bool	InitialiseImpl();	// from Component::I_GameComponent
	virtual bool	CreateImpl();		// from Component::I_GameComponent


private:

};

} // namespace Components


#endif // TRIGGERIMPL_7EFFE49A_F52E_4510_99D3_F002557BAFA6_H
