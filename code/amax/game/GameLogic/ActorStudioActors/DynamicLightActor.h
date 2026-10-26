#ifndef DYNMAMICLIGHTACTOR_H
#define DYNMAMICLIGHTACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <Dynamic/Node.h>
#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <SceneGraph/SceneGraph.h>


namespace Actors
{


//=========================================================================
//					LIGHTING RIG ACTOR CLASSES
//=========================================================================

	
class LightingRigCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_LightingRig;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_LightingRig;}

	const ::Actors::C_PosActorCreate*	m_BackLight1;
	const ::Actors::C_PosActorCreate*	m_BackLight2;
	const ::Actors::C_PosActorCreate*	m_BackLight3;
	const ::Actors::C_PosActorCreate*	m_BackLight4;
};


//=========================================================================


class LightingRigActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	LightingRigActor();
	virtual ~LightingRigActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);

	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_LightingRig; }
	static const Btl::String32			TypeName()		{ return "Lighting Rig"; }
	const LightingRigCreationData*		GetCreationData() const {return static_cast<const LightingRigCreationData*>(C_GameActor::GetCreationData().GetData());}

	void								UpdateLights();
	void								SetupLightVals(S32 index, const ::Actors::C_PosActorCreate *backlight);
	void								StoreRigActorInLight(const ::Actors::C_PosActorCreate *backlight);

	const LightingRigCreationData		*pCreationData;

	Maths::HMat43						m_WorldMatrix;

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
};



//=========================================================================
//					DYNAMIC LIGHT ACTOR CLASSES
//=========================================================================

	
class DynamicLightCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_DynamicLight;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_DynamicLight;}

	XtRgba								m_Colour;
};


//=========================================================================


class DynamicLightActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	DynamicLightActor();
	virtual ~DynamicLightActor();

	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);

	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_DynamicLight; }
	static const Btl::String32			TypeName()		{ return "Dynamic Light"; }
	const DynamicLightCreationData*		GetCreationData() const {return static_cast<const DynamicLightCreationData*>(C_GameActor::GetCreationData().GetData());}

	void								DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;

	const DynamicLightCreationData		*pCreationData;

	Maths::HMat43						m_WorldMatrix;
	LightingRigActor					*m_RigActor;
	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	bool								m_Selected;
	Actor::C_ActorHandle				m_LightingRig;				// Object to update if this light gets dynamically changed
};


}// End namespace Actors


#endif // DYNMAMICLIGHTACTOR_H
