#ifndef ANIMATEDOBJECTCOMPONENTS_H
#define ANIMATEDOBJECTCOMPONENTS_H

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Interfaces/I_AnimatedObjectComponents.h>
#include <shared_actors/Components/Creation/AnimatedObjectComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_AnimationController.h>

#include <animation/common/StringId.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/deferred/CommandList.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/WindParams.h>
#include <animation/util/Player.h>

#include <animated_model/AnimatedModelInstance.h>
#include <game_resource/ModelResource.h>
#include <world_render/MaterialParameter.h>
#include <resource_utils/ResourceUtilsDeclare.h>

#include <btl/containers/vector.h>
#include <btl/allocators/pool.h>

#include <dynamic/particle/EventInstance.h>
#include <dynamic/particle/EventInstanceHandle.h>
#include <dynamic/Node.h>

#include <scenegraph/SceneGraph.h>

class C_ModelResourceUsage;

namespace Animation
{
	class C_Skeleton;
	class C_Cycle;
	class C_PoseFormat;
	class C_EvaluationState;
	class C_PoseMapping;
}

namespace WorldRender
{
	class C_EnvironmentSampler;
}

namespace DynamicEffect
{
	namespace Particle
	{
		class C_Event;
		class C_SpawnMesh;
	
		namespace XtInterface
		{
			struct Event;
		}
	}
}

namespace Gravity
{
	class C_ShapeDesc;
	class C_InstancedTriangleMeshDesc;
	class C_CompositeDesc;
	class C_ConvexHullDesc;
}

namespace SpecialFX
{
	class I_SpecialFXInstance;
	class C_SpecialFX;
}

namespace Sys
{
	class I_ContextProvider;
}

namespace ResourceUtils
{
	class I_ResourceBag;
}

template<typename Type> class XtHandle;

class C_SimpleResourceBag;

namespace Components
{

class I_Movement;

/**
Plays back a single animation.
*/
class C_AnimationPlayerComponent : public Components::I_AnimationStateComponent
{
public:
	typedef Components::I_AnimationStateComponent		Base;

	C_AnimationPlayerComponent();
	virtual ~C_AnimationPlayerComponent();

	static Component::C_ComponentInterfaceId	InterfaceType()			{ return Components::E_IF_AnimationState; }
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const	{ return Components::E_IF_AnimationState; } // from Component::I_GameComponent
 
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimationPlayer; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimationPlayer; } // from Component::I_GameComponent

	const C_AnimationPlayerCreator*		GetCreationData() const		{return static_cast<const C_AnimationPlayerCreator*>(GetCreator().GetData());}

	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

	bool			OnMessage(const Messaging::C_MessageBase* pMessage);
	
#ifndef CFG_GOLD
	void			DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const OVERRIDE;
#endif

	void			OnResourceBagLoaded();

	//
	// Resource::C_Listener
	//
	//virtual void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);


	//
	// I_AnimationStateComponent implementation
	//

	/// Returns the current animation pose.
	virtual const Animation::C_Pose& GetPose() const;
	
	/// Returns the current animation pose matrix cache.
	virtual const Animation::C_PoseMatrixCache& GetPoseMatrixCache() const;
	
	/// Returns the skeleton used by the animation pose, or NULL if all transforms are in world space.
	virtual const Animation::C_Skeleton* GetSkeleton() const;

	/// Returns optional wind parameters used for adding wind to dynamic joints.
	virtual const Animation::C_WindParams* GetWindParams() const;

	/// Returns true if animation state is changing.
	virtual bool IsAnimating() const;	
	
	/// Returns the current animation pose without synchronising active multi-threaded update tasks.
	virtual const Animation::C_Pose& GetPoseNoSync() const;
	
	/// Returns the current animation pose matrix cache without synchronising active multi-threaded update tasks.
	virtual const Animation::C_PoseMatrixCache& GetPoseMatrixCacheNoSync() const;
	
	/// Returns true if cycle streaming is enabled. Full cycle data will not be loaded until LoadCycle messages are received.
	bool CycleStreaming() const		{ return m_CycleStreaming; }

	bool LoadingCycle() const		{ return m_LoadingCycleIndex >= 0; }

protected:

	//
	// Component::I_GameComponent interface
	//
	virtual bool InitialiseImpl();
	virtual bool PrimeImpl();
	virtual bool CreateImpl();
	

private:

	// Overall state.
	enum E_State
	{
		E_State_Uninitialised	= 0,
		E_State_Stopped,
		E_State_Playing,
		E_State_Finished
	};


	void Restart();
	void ChangeState(E_State newState);
	void Evaluate(F32 deltaTime);
	
	
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);
	
	class C_CycleNameLookup
	{
	public:
		C_CycleNameLookup(const C_AnimationPlayerComponent& player):
			m_player(player)
		{
		}
		
		const CHAR8* operator() (U32 cycleId) const;
	private:
		const C_AnimationPlayerComponent&	m_player;
	};
	
	struct CycleInfo
	{
		CycleInfo(): resource(NULL) {}
		
		Animation::C_CycleInstance						instance;
		Animation::C_CycleResource*						resource;
		ResourceUtils::C_ResourceBagTag					resourceTag;
		XtHandle<ResourceUtils::C_XtCycleResourceDef>	resourceDef;
	};
		
	U32 AddCycle(const XtHandle<ResourceUtils::C_XtCycleResourceDef>& resourceDef);
	void LoadAllCycles();
	void LoadCycle(U32 cycleIndex, bool headerOnly = false);
	void BindAllCycles();
	void BindCycle(U32 cycleIndex);
	
	Btl::Vector<CycleInfo>				m_Cycles;
	S32									m_DefaultCycleIndex;
		
	// Animation state, containing pose, pose matrix cache, skeleton instance and command list.
	Animation::C_EvaluationState*		m_EvaluationState;
		
	Animation::C_SkeletonResource*		m_SkeletonResource;
	
	Animation::C_Player					m_AnimPlayer;
	Animation::C_CycleStateHandle		m_CurrentCycleState;
	Animation::C_PoseFormat				m_PoseFormat;

	Animation::C_WindParams				m_WindParams;
	
	Actor::C_ActorHandle				m_NotificationActor;

	C_SimpleResourceBag*				m_pResourceBag;
	
	F32									m_Time;
	S32									m_LoadingCycleIndex;
	E_State								m_State;
	bool								m_FirstUpdate;
	bool								m_Selected;
	bool								m_CycleStreaming;
	bool								m_LoadingCycle;
};

class C_AnimatedCollisionShapes;

/**
Renders a number of generic animated models, driven by the I_AnimationStateComponent owned by its parent actor.
*/
class C_AnimatedModelComponent: public Components::I_AnimatedObjectVisualComponent
{
public:
	typedef Components::I_AnimatedObjectVisualComponent Base;

	// Temporary solution for disabling motion blur: game specifies render passes to use for non-motion blurred objects.
	static void SetNoMotionBlurRenderPasses(S32 opaqueRenderPass, S32 alphaTestRenderPass);

	static void SetSolidRenderPass(S32 solidRenderPass);
	static void SetPunchRenderPass(S32 punchRenderPass);
	static void SetAlphaRenderPass(S32 alphaRenderPass);

	// Set opaque render 

	C_AnimatedModelComponent();
	virtual ~C_AnimatedModelComponent();
	
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedModel; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedModel; }

	const C_AnimatedModelCreator*		GetCreationData() const		{return static_cast<const C_AnimatedModelCreator *>(GetCreator().GetData());}
	
	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage);
	
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

	
#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif


	void OnResourceBagLoaded();

	//
	// I_AnimatedObjectVisualComponent
	//
	
	/// Finds a model element with the given name.
	virtual const GameResource::C_ModelElement* FindModelElement(const CHAR8* elementName) const;

protected:

	virtual bool	InitialiseImpl();
	virtual bool	PrimeImpl();
	virtual bool	CreateImpl();
	virtual void	ExternalDependenciesCreatedImpl();
	virtual void	KillImpl();


private:
	enum E_State
	{
		E_State_Uninitialised = 0,
		E_State_Inactive,
		E_State_Active
	};
	
	struct ModelInstance;
	
	bool CreateModels(const Animation::C_Pose* sourcePose, const Animation::C_Skeleton* sourceSkeleton);
	void DestroyModels();
	void UpdateFromAnimationPlayer(const Maths::Mat43& worldMatrix, F32 deltaTime);
	void ChangeState(E_State state, bool setCollisionState);
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);
	Maths::Point3 GetLightSamplerPosition(const ModelInstance& modelInstance, const Maths::Mat43& worldMatrix, const Animation::C_Pose& pose, const Animation::C_Skeleton* skeleton) const;
	void SetVisible(bool visible);

	const Components::I_Movement*	GetActorMovementComponent() const;
	bool ShouldUpdateDuringUpdateMessage() const;

#ifndef CFG_GOLD
	Maths::C_Sphere CalcWorldSphere() const;
	void AccumulateResourceUsage(C_ModelResourceUsage& ioResourceUsage) const;
#endif
	
	struct ModelPart
	{
		ModelPart();
	
		GameResource::C_ModelResource*	modelResource;
		S32								modelIndex;
	};

	static const U32 kMaxDropShadows = 4;

	class C_DropShadow
	{
	public:
		C_DropShadow();
		~C_DropShadow();

		enum E_AttachFlags
		{
			E_AttachFlag_KeepLowestYPosition	= 1
		};

		void Create(SceneGraph::C_SceneGraph* pSceneGraph);
		void Destroy(SceneGraph::C_SceneGraph* pSceneGraph);

		void SetIntensity(F32 intensity);
		void SetRadius(F32 radius);

		void AttachToBoundingSphere();
		void AttachToBone(S32 jointIndex);
		void AttachBetweenBones(S32 jointIndex1, S32 jointIndex2, F32 blend, U32 attachFlags);
		void SetOffset(const Maths::Vector3& offset);

		void Update(const Animation::C_Pose& pose, const Animation::C_Skeleton* pSkeleton, const AnimatedModel::C_AnimatedModelInstance& animatedModelInstance, const Maths::Mat43& worldMatrix);
						
	private:
		SceneGraph::C_DropShadowHandle	m_handle;
		Maths::Sphere					m_sphere;
		Maths::Vector3					m_offset;
		F32								m_blend;
		S16								m_jointIndex1;
		S16								m_jointIndex2;
		bool							m_firstUpdate;
		bool							m_keepLowestYPosition;
		
	};

	struct ModelInstance
	{
		ModelInstance();
		~ModelInstance() {}
		
		void Destroy();

		bool HasDropShadows() const;
		void UpdateDropShadows(const Animation::C_Pose& pose, const Animation::C_Skeleton* pSkeleton, const Maths::Mat43& worldMatrix);
		S32 GetShadowBoneJointIndex(const char* boneName, const Animation::C_Pose& pose) const;
		
		Btl::Vector<ModelPart>					modelParts;
		
		Animation::C_SkeletonResource*			skeletonResource;
		const Animation::C_Skeleton*			skeleton;
		AnimatedModel::C_AnimatedModelInstance	animatedModelInstance;
		Animation::C_EvaluationState*			evaluationState;
		Animation::C_PoseMapping*				poseMapping;				// From source (AnimationState) pose to local pose
		WorldRender::C_EnvironmentSampler*		envSampler;
		C_AnimatedModelCreator::E_LightSamplerPositionType	lightSamplerPositionType;
		Maths::Point3							lightSamplerOffset;

		XtHandle<C_AnimatedModelCreator::C_XtDropShadowList>	m_xtDropShadowList;
		U32										m_xtDropShadowListChecksum;
		C_DropShadow							m_dropShadows[kMaxDropShadows];
		U32										m_numDropShadows;
			
		U32										creationIndex;			// Index in original model list.
		const char*								namespacePrefix;
		const char*								lightSamplerBoneName;
		S16										lightSamplerBoneIndex;
		bool									motionBlurDisabled;
		bool									enableScale;
	};
	
	Actor::C_ActorHandle			m_ControllerHandle;		// Actor containing this model's animation controller.
	Btl::Vector<ModelInstance>		m_ModelInstances;
	C_AnimatedCollisionShapes*		m_AnimatedCollisionShapes;
	C_SimpleResourceBag*			m_pResourceBag;
	E_State							m_State;
	U8								m_CollisionState;
	bool							m_Visible;
	bool							m_FirstUpdate;
	bool							m_Selected;
	
	static S32						s_NoMotionBlurOpaqueRenderPass;
	static S32						s_NoMotionBlurAlphaTestRenderPass;
	static S32						s_SolidRenderPass;
	static S32						s_PunchRenderPass;
	static S32						s_AlphaRenderPass;
};


/**
Animated particle effects component.

Spawns particle effects attached to bones.
*/
class C_AnimatedParticleEffectsComponent: public Components::I_AnimatedParticleEffectsComponent
{
public:
	typedef Components::I_AnimatedParticleEffectsComponent Base;

	C_AnimatedParticleEffectsComponent();
	virtual ~C_AnimatedParticleEffectsComponent();

	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedParticleEffects; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedParticleEffects; }

	const C_AnimatedParticleEffectsCreator*	 GetCreationData() const;
	
	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage);
	
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:

	virtual bool	InitialiseImpl();
	virtual bool	PrimeImpl();
	virtual void	ExternalDependenciesCreatedImpl();

	void OnResourceBagLoaded();


private:
	enum E_State
	{
		E_State_Uninitialised = 0,
		E_State_Inactive,
		E_State_Active
	};
	
	void ChangeState(E_State newState);
	void UpdateFromAnimationPlayer(const Maths::Mat43& worldMatrix, F32 deltaTime);
	void CreateEmitters();
	void DestroyEmitters();
	void ResetEmitters();
	void SetVisible(bool visible);
	void BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_ShapeDesc* shapeDesc, const Maths::HMat43& transform) const;
	void BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_InstancedTriangleMeshDesc* instancedTriMeshDesc, const Maths::HMat43& transform) const;
	void BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_CompositeDesc* compositeDesc, const Maths::HMat43& transform) const;
	void BuildSpawnMesh(DynamicEffect::Particle::C_SpawnMesh& spawnMesh, Gravity::C_ConvexHullDesc* convexHullDesc, const Maths::HMat43& transform) const;

	const Components::I_Movement*	GetActorMovementComponent() const;
	bool ShouldUpdateDuringUpdateMessage() const;

	ResourceUtils::I_ResourceBag* GetResourceBag();
	
	struct Emitter
	{
		Emitter(): 
			spawnMesh(NULL),
			intensityScale(1.f), jointIndex(-1), intensityScalarIndex(-1) {}
		
		~Emitter();
	
		DynamicEffect::C_Node									node;
		Maths::HVec3											velocity;
		XtHandle<DynamicEffect::Particle::XtInterface::Event>	xtEvent;
		DynamicEffect::Particle::C_EventInstanceHandle			eventInstance;
		const SpecialFX::C_SpecialFX*							xtSpecialFx;
		DynamicEffect::Particle::C_SpawnMesh*					spawnMesh;
		F32														intensityScale;
		F32														intensity;			// Current value.
		S16														jointIndex;
		S16														intensityScalarIndex;
		
		bool IsActive() const;
		bool IsContinuous() const;
	};

	class SpecialFXEmitter
	{
	public:
		SpecialFXEmitter();
		~SpecialFXEmitter();
		
		void Spawn(const SpecialFX::C_SpecialFX& specialFXRecord,
			const Sys::I_ContextProvider* pContextProvider,
			const ResourceUtils::I_ResourceBag* pResourceBag);
		void Destroy();
			
		void SetEmitterIndex(S32 emitterIndex)	{ m_emitterIndex = emitterIndex; }
		S32 GetEmitterIndex() const				{ return m_emitterIndex; }
		
		void Update(F32 deltaTime, const Emitter& parentEmitter);
		void SetContinuous(bool isContinuous);
		bool IsActive() const;

	private:
		SpecialFX::I_SpecialFXInstance*	m_instance;
		F32								m_lifetime;
		F32								m_duration;
		S16								m_emitterIndex;
		bool							m_isContinuous;
	};

	typedef Btl::DynamicPool<SpecialFXEmitter>	SpecialFXEmitterPool;

	SpecialFXEmitterPool			m_SpecialFXEmitterPool;
	Actor::C_ActorHandle			m_ControllerHandle;			// Actor containing this model's animation controller.
	Btl::Vector<Emitter>			m_Emitters;
	Btl::Vector<SpecialFXEmitter*>	m_SpecialFXEmitters;
	C_SimpleResourceBag*			m_pResourceBag;
	E_State							m_State;
	bool							m_Visible;
	bool							m_FirstUpdate;
};

/**
Sends messages based upon changes in state of animation curves.
*/
class C_AnimatedEventTriggerComponent: public Components::I_AnimatedEventTriggerComponent
{
public:
	typedef Components::I_AnimatedEventTriggerComponent Base;

	C_AnimatedEventTriggerComponent();
	virtual ~C_AnimatedEventTriggerComponent();

	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedEventTrigger; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedEventTrigger; }

	const C_AnimatedEventTriggerCreator*	 GetCreationData() const;
	
	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage);
	
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
	virtual void	DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:

	virtual bool	InitialiseImpl();
	virtual void	ExternalDependenciesCreatedImpl();

private:

	enum E_State
	{
		E_State_Inactive = 0,
		E_State_Active,
		E_State_Count,
		E_State_Uninitialised = E_State_Count
	};
	
	void ChangeState(E_State state);
	void CreateEvents();
	void Update(F32 deltaTime);

	struct Event
	{
		S16		scalarCurveIndex;
		S16		eventIndex;
		F32		currentValue;
	};
	
	Actor::C_ActorHandle	m_ControllerHandle;
	Btl::Vector<Event>		m_Events;
	E_State					m_State;
};

/**
Provides a I_AnimationController interface for animated object actors.
*/
class C_AnimatedObjectAnimationController: public I_AnimationController
{
public:
	C_AnimatedObjectAnimationController();
	virtual ~C_AnimatedObjectAnimationController();

	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedObjectAnimationController; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedObjectAnimationController; }

	const I_AnimatedObjectAnimationControllerCreator*	 GetCreationData() const;

	virtual F32				GetAnimationLength() const;

	virtual const Level::C_AnimTValue	GetCurrentTValue(void) const;

	virtual const bool	GetObjectTranslation(const S32 objectID, Maths::C_Vertex &translation) const;
	virtual const bool	GetObjectLocalTranslation(const S32 objectID, Maths::C_Vertex &translation) const;
	virtual const bool	GetObjectRotation(const S32 objectID, Maths::C_Quaternion &rotation) const;
	virtual const bool	GetObjectLocalRotation(const S32 objectID, Maths::C_Quaternion &rotation) const;
	virtual const bool	GetCameraFOV(const S32 objectID, F32 &fValue) const;

	virtual const S32	GetNamedFloatCurveChannelID(const S32 objectID, const Util::C_String &name) const;
	virtual const bool	GetNamedFloatCurveValue(const S32 objectID, const S32 channelID, F32 &fValue) const;

	virtual const U32	GetNumberOfNamedCurves(const S32 objectID) const;
	virtual const char*	GetNamedCurveName(const S32 objectID,U32 index) const;


#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	virtual bool CreateImpl();
	virtual void InternalDependenciesCreatedImpl();

private:

	bool	GetObjectTransform(S32 objectId, Animation::C_JointTransform& outTransform) const;

	const I_AnimationStateComponent*	m_AnimationState;
	
	struct ObjectInfo
	{
		ObjectInfo(): cameraFovIndex(-1), transformIndex(-1) {}
		
		Actor::C_ActorHandle	actorHandle;
		S32						index;
		S16						cameraFovIndex;
		S16						transformIndex;
		Btl::Vector<U16>		scalarCurveIndices;

		U32 GetNumScalarCurves() const	{ return scalarCurveIndices.Size(); }
	};
	
	Btl::Vector<ObjectInfo>		m_ObjectInfo;
};


} //namespace Components

#endif
