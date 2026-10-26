#ifndef DYNAMICOBJECTACTOR_H
#define DYNAMICOBJECTACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <Game/GameDef/UserData.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#include <Game/GameWorld/GameWorld.h>

#include <Resource/Resource.h>
#include <game_resource/ModelResource.h>
#include <world_render/MaterialParameter.h>
#include <game_resource/ModelData.h>

#include <gravity/sdk/BodyHelper.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include "Framework/Timer/Timer.h"
#include <Vehicle/Damage/CollisionInfo.h>

#include <Animation/Resource/CycleResource.h>
#include <animated_model/ManagedAnimatedModel.h>
#include <SceneGraph/SceneGraph.h>


//#include <GameLogic/ActorStudioActors/ActorUtils.h>




//static const int			MAX_NUM_DYNAMIC_ANIMATIONS = 8;



namespace Animation
{
	class C_CycleInstance;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_EnvironmentSampler;
}

//=========================================================================
//						DYNAMIC ACTOR CLASSES
//=========================================================================


namespace Actors
{

class C_VisibleDetect;

class DynamicObjectCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_DynamicObject;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_DynamicObject;}

	// Creation data
	XtHandle<Game::DynamicObjectDef>		m_DynamicObjectDef;
	XtHandle<Game::DynamicObjectDefGroup>	m_ReplacementGroup;

	F32										m_AnimStartTimer;				// Time before Delayed Start animation begins
	S32										m_RaceLogicGroup;

	bool									m_IgnoreAlignment;				// Don't align this actor
	bool									m_EnableRespawn;

	// Event messages
	ActorMsg::C_XtSendMessage				m_OnPlayerCollision;
	ActorMsg::C_XtSendMessage				m_OnCollision;
	ActorMsg::C_XtSendMessage				m_OnDestroyed;
	ActorMsg::C_XtActorMessageBase			*m_VehicleCollisionMessage;

	// Components
	virtual void						DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	Components::C_MovementCreateInfo*				m_pSorceryMovementComponentCreate;
	Component::I_ComponentCreator::C_DataRef		m_pMovementCreator;
	const Components::C_MovementCreateInfo*			GetMovementCreator() const {return static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData());}
	Components::C_MovementCreateInfo*				GetMovementCreator() {return const_cast<Components::C_MovementCreateInfo*>(static_cast<const Components::C_MovementCreateInfo*>(m_pMovementCreator.GetData()));}
};


//=========================================================================


class DynamicObjectActor : public Game::Actors::C_GameActor, public Resource::C_Listener
{
public:
	DynamicObjectActor();
	virtual ~DynamicObjectActor();

	void								CreateModel();
	void								DestroyModel();
	void								ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData );
	virtual bool						OnMessage(const Messaging::C_MessageBase *pMessage);
	void								Update();								// Update logic called each frame
	void								ActivePhysicsBodyUpdate();				// Update logic called each frame if physics body is active
	void								UpdateLightSampler();
	void								UpdateDropShadow();
	void								UpdateRenderModel(bool visibleOverride = false, bool updateAnimation = true);	// Update render model position
	void								UpdateLifeTime();
	void								UpdateWorldPosition(bool movephysics, bool visibleOverride = false);	// Manually move object via a message
	void								StartAnimation(Game::E_ANIM_EVENT_TYPE eventnum);		// Check if we need to start a new animation
//	DynamicObjectActor					*SpawnDebris();
	void								SetLifespan(F32 duration);				// Sets how long before this object will commit suicide
	void								KillThis();								// Cleans up any children etc before killing itself
	F32									RegisterCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo  *collinfo);			// Return false to ignore this collision
	F32									ProcessCollision(Actor::C_Actor *collactor, Vehicle::CollisionInfo  *collinfo);			// Return false to ignore this collision
	void								NormaliseMat(Maths::HMat43 *mat);
	void								AttachWithSpring(F32 rest_len, F32 lin_coeff, F32 lin_damp, F32 ang_coeff, F32 ang_damp, F32 zoff, F32 xoff);	// Connects this object to the ground using a sping
	void								SetInitialSpeeds(Maths::HVec3 *linspeed, Maths::HVec3 *angspeed, bool collisiondetection);
	void								CheckInitialState();
	void								RenderDebugInfo();
	void								ReloadSorceryData();
	void								Respawn();



	virtual const Maths::C_Vertex		GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual const Maths::HMat43&		GetMatrix(void) const {return m_WorldMatrix;}
	
	const Maths::C_Vertex				GetDir()		const {return m_WorldMatrix.AZ();}
	const Maths::C_Vertex				GetUp()			const {return m_WorldMatrix.AY();}
	const Maths::C_Vertex				GetRight()		const {return m_WorldMatrix.AX();}

	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_DynamicObject; }
	static const Btl::String32			TypeName()		{ return "DynamicObject"; }
	const DynamicObjectCreationData*	GetCreationData() const {return static_cast<const DynamicObjectCreationData*>(C_GameActor::GetCreationData().GetData());}

	void								FlagError()		{ possible_error = true; }



	Maths::HMat43						m_WorldMatrix;
	Gravity::C_Body*					m_pBody;		// Make it public for now

	C_VisibleDetect*					GetVisibleDetect() const { return m_pVisDetect; }

	void CreateTrigger(float p_Radius,const Actor::C_ActorHandle& actorHandle,Maths::HMat43	mWorldMatrix);
	void DestroyTrigger(const Actor::C_ActorHandle& actorHandle);
	Game::E_AUDIO_MASS_TYPE  			GetAudioMassType(){ return m_AudioMassType;}

	const Gravity::C_Trigger*			GetTrigger() const { return m_pTriggerObject; }
	
protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:
	Game::C_GameWorld					*m_pGameWorld;	
	SharedPhysics::C_Spring				*m_pSprings[4];				// Remember what springs we have attached


	GameResource::C_ModelHandle			m_ModelHandle;
	GameResource::C_ModelResource*		m_pModelResource;

	U32									num_model_elements;			// How many model elements do we have

	AnimatedModel::C_ManagedAnimatedModel	m_AnimatedModel;
	WorldRender::C_ModelInstance*		m_pModelInstance;
	const GameResource::C_ModelElement*	m_pModelElement;
	WorldRender::C_EnvironmentSampler*	m_pEnvSampler;
	WorldRender::MaterialParameter		m_ShNodeHandle;
	Maths::C_Sphere						m_LocalBoundingSphere;

	bool								m_ValidAnimation[Game::NUM_ANIMATION_EVENT_TYPES];		// True toplay animation on this event, False to stop all animations
	Animation::C_CycleResourceHandle	m_CycleHandle[Game::NUM_ANIMATION_EVENT_TYPES];
	Animation::C_CycleResource*			m_pCycleResource[Game::NUM_ANIMATION_EVENT_TYPES];
	Animation::C_CycleInstance*			m_pCycleInstance[Game::NUM_ANIMATION_EVENT_TYPES];
	U32									m_NumberOfAnimations;

	F32									m_AnimationTime;
	F32									m_DelayedStartTime;
	bool								m_Freeze;

	Game::E_PHYSICS_BEHAVIOUR			m_PhysType;

	bool								anim_active;			// True if there is an animation playing
	U32									anim_number;			// Which animation to play

	bool								m_bVisible;
	bool								m_bVisibleLastFrame;
	F32									m_fLastAnimTime;


	F32									starttime;				// Time when object was created
	F32									lifespan;				// Timer for killing objects off

	const Game::DynamicObjectDef		*objdef;				// Points to standard dynamic object definition for this type of object

//	F32									simpledamage;			// Temp damage stuff until full damage system is in
//	S32									num_hits;				// How many times this has been collided with
	
	F32									minheight;				// Cull if fallen below this height

	bool								possible_error;			// True if the sanity check recons this object might have an error

	Maths::HVec3						m_ShadowPosition;
	F32									m_ShadowIntensity;
	F32									m_ShadowRadius;
	SceneGraph::C_DropShadowHandle		m_DropShadowHandle;
	bool								collision;				// True if this object should collide with other objects

	bool								m_kinematic;
	bool								initial_state;			// True to set speeds
	Maths::HVec3						initial_lin_velocity;	// What speed to start it at - Linear
	Maths::HVec3						initial_ang_velocity;	// What speed to start it at - Angular

	C_VisibleDetect						*m_pVisDetect;	

	Gravity::C_Trigger*					m_pTriggerObject;
	Actor::C_ActorHandle				m_actorHandle;
	Gravity::C_TriggerLayer*			pTriggerLayer;
	Gravity::C_KinematicController*		m_KinematicController;

	bool								m_ModelCreated;

	Game::C_BodyUserDataActor			m_UserData;
	
	WorldRender::MaterialParameter		m_ShockIntensityParameter;

	Maths::HVec3						m_ColliderPosition;
	S32									m_FramesInCollision;
	float								m_TimeInCollision;
	float								m_RespawnTime;
	bool								m_HasBeenHit;
	
//	F32									m_last_collision_time;
//	F32									m_total_collision_time;

	bool								m_Selected;

	bool								m_DontMakeKinematic;
	Game::E_AUDIO_MASS_TYPE				m_AudioMassType;
};

}// End namespace Actors


#endif // DYNAMICOBJECTACTOR_H
