#ifndef PHYSICALOBJECT_H
#define PHYSICALOBJECT_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <Effects/BlobShadows.h>
#include <shared_actors/Components/Impl/PhysicsBodyComponent.h>
#include <shared_actors/actordef/BasePhysicalObjectDef.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>
#include <shared_actors/Components/Creation/RenderComponentsCreate.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/Components/Impl/BaseElementManagerComponents.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/UserData.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/ActorDamage.h>
#include <world_render/MaterialParameter.h>

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <Game/GameWorld/GameWorld.h>

#include <shared_actors/XtSharedActorMessages.h>

class C_ElementMgrNotificationSpawnChildMsg;

namespace Game
{
	struct C_PhysicalObjectDef;
}

namespace Components
{
	class I_DamageableElementManager;
	class C_PhysicsBody;
	class C_MovementCreateInfo;
}

namespace ActorDamage
{
	class C_DamageData;
}


namespace Actors
{

class C_VisibleDetect;

class C_PhysicalObjectCreate : public C_PosActorCreate
{
public:
	C_PhysicalObjectCreate() : m_RootModelElementIndex(-1), m_CollideGroup(0) {}

public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_PhysicalObject;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_PhysicalObject;}

	XtHandle<BasePhysicalObject::C_ElementObjectRecord>		m_Definition;
	XtHandle<Game::PhysicalObjectDefGroup>					m_ReplacementGroup;
	S32														m_RaceLogicGroup;

	Components::C_MovementCreateInfo*				m_pSorceryMovementComponentCreate;
//	Component::I_ComponentCreator::C_DataRef		m_pMovementCreator;
//	const Components::C_MovementCreateInfo*			GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
//	Components::C_MovementCreateInfo*				GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}

	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	// Non-Xt stuff
	// For spawning child objects:
	bool IsSpawnedChild() const {return m_RootElementName!="";}
	Actor::C_ActorHandle m_ParentHandle;	// Handle of original parent actor.
	S32 m_ParentElementIndex;				// Index of parent physObj element we are a child of.  Used as a constraint association key.
	Util::C_String m_RootElementName;
	S32 m_RootModelElementIndex;
	U32 m_CollideGroup;
	Btl::Vector<Components::C_BaseDamageableElementManager::C_ElementState> m_ElementStateSetup;
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Physical object class.
//
class C_PhysicalObject : public Game::Actors::C_GameActor//, public Resource::C_Listener
{
public:
	C_PhysicalObject();
	~C_PhysicalObject();

	static const Actor::C_ActorTypeId	TypeId()	{ return Game::E_ActorTypeId_PhysicalObject; }
    static const Actor::C_ActorTypeName TypeName()	{ return "PhysicalObject"; }

	const C_PhysicalObjectCreate*	GetCreationData() const {return static_cast<const C_PhysicalObjectCreate*>(C_GameActor::GetCreationData().GetData());}

public: // Main API
	const Maths::HMat43&			GetMatrix(void) const { return m_Matrix; }
	const Maths::HMat43&			GetRenderMatrix() const;

	const Game::C_PhysicalObjectDef* Definition() const {return m_Definition;}
//	const XtHandle<ResourceUtils::C_GameModel>& DefModel() const {return GetCreationData()->m_Definition->m_Model;}
	const XtHandle<ResourceUtils::C_GameModel>& DefModel() const {return m_ElementRecord->m_Model;}

	const Components::I_PhysicsBody* PhysicsBodyComponent() const {return (const Components::I_PhysicsBody *)QueryInterface(Components::E_IF_PhysicsBody);}

	const BasePhysicalObject::C_ElementObjectRecord	 *GetElementRecord() const {return(m_ElementRecord);}

	S32							ElementCount() const;
	S32							ElementIndex(const Util::C_String& elementName) const;
	bool						IsElementIndestructible(S32 elementIndex) const;
	bool						GetElementCollisionWorldPos(S32 elementIndex, Maths::HVec3& worldPos) const;
	bool						GetElementWorldPos(const Util::C_String& elementName, Maths::HVec3& worldPos) const;
	bool						GetElementOffset(const Util::C_String& elementName, Maths::HVec3& pos) const;
	bool						GetElementWorldMatrix(const Util::C_String& elementName, Maths::HMat43& worldMatrix) const;
	S32							ModelElementIndex(const Util::C_String& elementName) const;
	S32							ElementIndexFromFlatPrimitiveIndex(S32 primIndex) const;
	const GameResource::C_ModelElement*	FindModelElement(const Util::C_String& elementName) const;
	const GameResource::C_ModelElement* FindModelElement(S32 modelElementIndex) const;

	F32							RegisterCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo);
	F32							ProcessCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo *collinfo);

	// Collision.
	U32							GetCollideGroup() const {return PhysicsBodyComponent()->CollisionFilter().GetGroupId();}
	U32							CollideGroupFlags() const {return PhysicsBodyComponent()->CollisionFilter().GetGroupFlags();}
	U32							CollideGroupMask() const {return PhysicsBodyComponent()->CollisionFilter().GetGroupMask();}
	U32							CollideGlobalFlags() const {return PhysicsBodyComponent()->CollisionFilter().GetCollisionFlags();}
	U32							CollideGlobalMask() const { return PhysicsBodyComponent()->CollisionFilter().GetCollisionMask();}

	C_VisibleDetect*			GetVisibleDetect() const { return m_pVisDetect; }

	void						SpawnParticleEffects();
	void						SetupLifeTime();

	float						GetLifeTime() const { return m_LifeTimeAfterDamage; }

	const Gravity::C_Trigger*	GetTrigger() const { return m_pTriggerObject; }

private:
	void						MakeDynamic(bool makeDynamic=true);
	void						MakeAnimated();

	// Velocity stuff:
	void						ApplyVelocity(const Maths::C_Vector& vel);
	void						ApplyLocalVelocity(const Maths::C_Vector& vel);

 	bool						SetWarpPosition(const Maths::HMat43& matrix);
	void						SetAngularDamping(F32 factor);

	void						UpdateDropShadow();
	void						UpdateMoveableParts(const Maths::C_TimeS& timeStep);

	void						EnableElement(S32 index, bool enable);

	void						SetCollideGroup(S32 group);
	void						SetCollideGroupFlags(U32 flags);
	void						SetCollideGroupMask(U32 mask);
	void						SetCollideGlobalFlags(U32 flags);
	void						SetCollideGlobalMask(U32 mask);
	void						SetCollideMaterialMask(U32 mask);

public:
	void						UpdateLifeTime();
	void						Update(const Maths::C_TimeS& timeStep);
	virtual bool				OnMessage(const Messaging::C_MessageBase *pMessage );

#ifndef CFG_GOLD
	void						DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	void CreateTrigger(float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix);
	void DestroyTrigger(const Actor::C_ActorHandle& actorHandle);
	Game::E_AUDIO_MASS_TYPE 	GetAudioMassType(){ return m_AudioMassType;}

protected:
	void						Enable(bool enable);
	void						SetBodyUserData();
	void						SetVisibilityDetect();
	void						UpdateShape();
	void						ApplyDamageImpulse(const ActorDamage::C_DamageData* damageData);
	void						Break(const ActorDamage::C_DamageData *pDamageData, bool ignoreIndestructibleFlag=true);
	void						Destroy(const ActorDamage::C_DamageData *pDamageData, bool ignoreIndestructibleFlag=true);

	virtual bool				InitialiseImpl();	// from Actor::C_Actor
	virtual bool				CreateImpl();		// from Actor::C_Actor
	virtual void				KillImpl();

private:
	const Components::I_DamageableElementManager* ElementManagerComponent() const {return (const Components::I_DamageableElementManager*)(QueryInterface(Components::E_IF_DamageableElementManager));}

 	bool						ElementIsActive(S32 elementIndex) const;
 	Maths::HVec3				ElementWorldPos(const GameResource::C_ModelElement& element) const;
 	Maths::HVec3				ElementOffset(const GameResource::C_ModelElement& element) const;

	void						Damage(ActorDamage::C_DamageData *pDamageData);
	void						DamageAllElements(ActorDamage::C_DamageData *pDamageData);

	void						SpawnChild(const C_ElementMgrNotificationSpawnChildMsg& msg);

	void						OnElementStateBroken();
	void						OnElementStateChanged();
	void						OnComponentCreated(S32 componentIf);
	void						OnPhysicsCollision(const C_PhysicsEventCollisionMsg& collisionMsg);
	void						OnPhysicsBodyEvent(const Gravity::C_BodyEvent& bodyEvent);
	void						OnBodyTypeChanged(ActorPhysics::TBodyType newBodyType);

private:
	const Game::C_PhysicalObjectDef					*m_Definition;	// Root def for this object (differs from create def root if this is a spawned child)
	XtHandle<BasePhysicalObject::C_ElementObjectRecord>	m_ElementRecord;

	Maths::HMat43				m_Matrix;
	bool						m_IsEnabled;
	bool						m_BodyActive;
	bool						m_UpdateShape;
	bool						m_Damaged;
	bool						m_VisibleLastFrame;
	float						m_TimeUntilKilled;
	float						m_LifeTimeAfterDamage;
//	C_BlobShadow				m_BlobShadow;

	// Body data:
	ActorPhysics::TBodyType		m_CurrentBodyType;

	DynamicEffect::C_Node		m_LocalSpace;
	ActorDamage::C_DamageData	m_DamageData;

	C_VisibleDetect*			m_pVisDetect;	

	Gravity::C_Trigger*			m_pTriggerObject;
	Gravity::C_TriggerLayer*			pTriggerLayer;

	Game::C_GameWorld					*m_pGameWorld;	

#ifndef CFG_GOLD
	bool m_Initialising;
#endif
	
	SceneGraph::C_DropShadowHandle		m_DropShadowHandle;
	float						m_ShadowRadius;
	Maths::HVec3				m_ShadowPosition;
	Maths::HVec3				m_ImpactVelocity;

	WorldRender::MaterialParameter m_ShockIntensityParameter;

	Game::C_BodyUserDataActor	m_UserData;

	Maths::HVec3				m_ColliderPosition;
	S32							m_FramesInCollision;
	float						m_TimeInCollision;

	F32							minheight;				// Cull if fallen below this height
	Game::E_AUDIO_MASS_TYPE		m_AudioMassType;
};

} //namespace Actors

#endif
