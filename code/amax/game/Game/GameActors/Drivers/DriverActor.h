#ifndef DRIVERACTOR_H
#define DRIVERACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/ik/IkChain.h>
#include <animation/ik/SteeringController.h>
#include <Game/GameActors/Drivers/AnimationTree.h>
#include <Game/GameActors/Drivers/XtAnimationTreeListener.h>
#include <Game/GameActors/Drivers/DriverAnimationParameters.h>
#include <Game/GameUtil/ResourceBagListener.h>
#include <Game/GameUtil/ResourceBagDeclare.h>
#include <system/misc/Random.h>
#include <world_render/RenderModelData.h>

struct AmaxVehicle;
struct AmaxBaseCharacter;

namespace Actors
{
	class C_VisibleDetect;
}

namespace GameResource
{
	class C_ModelResource;
}

namespace Animation
{
	class C_SkeletonResource;
	class C_CycleResource;
	class C_EvaluationState;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_LightSampler;
}

namespace AnimatedModel
{
	class C_AnimatedModelInstance;
}

namespace ActorMsg
{
	class C_SetDriverAnimMsg;
}

namespace Vehicle
{
	class C_ModelInput;
}

namespace SceneGraph
{
	class C_SceneGraph;
}

class C_ResourceBag;

namespace Game
{
	class C_ImpactMsg;
	
	/**
	Creation parameters for a driver actor.
	*/
	class C_DriverActorCreate
	{
	public:
		C_DriverActorCreate();
		
		/// Returns driving style index.
		U32 GetDrivingStyle() const;

		/// Returns true if the driver cannot rest his arm on the car door due to obstructions, narrow windows etc.
		bool CannotRestArmOnDoor() const;

		/// Returns true if the driver cannot play animations where his arm goes through the window because of intersection.
		bool CannotPutArmThroughWindow() const;

		/// Returns the character model filename.
		const CHAR8* GetDriverModelFilename() const;
		
		/// Returns the character skeleton filename.
		const CHAR8* GetSkeletonFilename() const;

		/// Returns true if driver has a seatbelt model.
		bool HasSeatbeltModel() const;

		/// Returns the seatbelt model filename, which may be NULL if there is no separate seatbelt model.
		Util::C_String GetSeatbeltModelFilename() const;
		
		/// Returns true if driving position is on right hand side of vehicle.
		bool IsRightHandDrive() const;
		
		/// Returns vehicle-space translation offset applied to driver's root.
		Maths::HVec3 GetOffset() const;
		
		/// Returns an offset used to prevent feet clipping with cabin floor.
		F32 GetCabinFloorOffset() const;

		/// Returns an offset used to prevent feet clipping with footwell (and wheel arches).
		F32 GetCabinFootwellOffset() const;
		
		bool IsSteeringWheelIkEnabled() const;
		Maths::Point3 GetSteeringWheelPos() const;
		F32 GetSteeringWheelTilt() const;
		F32 GetSteeringWheelRadius() const;
		
		XtHandle<AmaxVehicle>				vehicleDef;
		XtHandle<AmaxBaseCharacter> 		characterDef;
		U8									vehicleIndex;		// Index of vehicle.
	};
	
	/**
	Driver 'actor', although it is now no longer part of the actor system.
	*/
	class C_DriverActor: public AnimationTree::C_XtAnimTreeListener, public C_ResourceBagListener
	{
	public:
		C_DriverActor();
		virtual ~C_DriverActor();

		bool Prime(const C_DriverActorCreate& creationParams, SceneGraph::C_SceneGraph* pSceneGraph);
		bool Create();
						
		void SetVehicleModelInput(const Vehicle::C_ModelInput& modelInput);
		void SetVehicleLightSampler(const WorldRender::C_LightSampler* pLightSampler);
		void SetVehicleCubeMapTexture(const void* pCubeMapTexture);
		
		/// Set visibility of driver in a particular viewport.
		void SetVisibility(bool isVisible, U32 viewportIndex);

		/// Set result of visibility test after rendering last frame
		void SetVisibilityDetectionFlags(U32 flags);

		void Update(F32 deltaTime, const Maths::HVec4* pLodScaleAndOffset, U32 numViewports);

		void Reset();

		//
		// Message handling.
		//
		void OnImpactMessage(const Game::C_ImpactMsg* pImpactMsg);
		void OnPlayDriverAnimation(const ActorMsg::C_SetDriverAnimMsg* pSetDriverAnimMsg);
		void HitByPerk();
		void Overtake(const Maths::HVec3& otherPos, bool isOvertaker);
		void PassedEvent();
		void FailedEvent();
		void VehicleFatallyDamaged();

		#ifndef CFG_GOLD
		void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
		#endif
		
		/// Returns actor head position and orientation.
		Maths::C_QuatTrans GetHeadTransform() const;

		/// Returns animation synchronised steering wheel rotation angle, in radians.
		F32 GetSteeringWheelRotationAngle() const;

		Maths::HVec3 GetPosition() const		{ return m_matrix.Translation(); }
		
		//
		// C_XtAnimTreeListener interface
		//
		virtual void OnXtAnimTreeCreated(XtRecord* record, const AnimationTree::C_XtTree* tree);
		virtual void OnXtAnimTreeDestroyed(XtRecord* record, const AnimationTree::C_XtTree* tree);
		virtual const char* GetXtAnimTreeName() const;

		void PhotoMoveDriver(const Maths::HVec3& offset);

		//! Set sh material parameters that can change per-frame
		void SetDynamicSHLightShaderParameters(const F32 additiveSHLightData[16]);


		

		// C_ResourceBagListener
		virtual void OnResourceBagLoaded(const C_ResourceBag& bag);
		
	private:
		void CreateAnimTree(ResourceUtils::I_ResourceBag& resourceBag);
		void Destroy();

		void Animate(F32 deltaTime);
		void UpdateSteeringController();
		void UpdateParameters(const Vehicle::C_ModelInput& modelInput);
	#ifndef CFG_SPEED
		void UpdateFromDebugVariables();
	#endif
			
		void BuildAnimTree(const AnimationTree::C_XtTree* tree);
		
	
		const C_DriverActorCreate* GetCreationParams() const	{ return &m_creationParams; }

		Maths::HVec3 GetDriverOffset() const;
		
	private:
		Maths::HMat43 GetVehicleWorldMatrix() const;
	
		enum E_ArmIkTarget
		{
			E_ArmIkTarget_Steering	= 0,
			E_ArmIkTarget_Misc,
			E_ArmIkTarget_Count
		};
		enum E_ArmIkTargetFlag
		{
			E_ArmIkTargetFlag_Steering	= 1 << E_ArmIkTarget_Steering,
			E_ArmIkTargetFlag_Misc		= 1 << E_ArmIkTarget_Misc
		};
		
		enum E_LegIkTarget
		{
			E_LegIkTarget_Floor	= 0,
			E_LegIkTarget_Count
		};

		enum E_ModelPart
		{
			E_ModelPart_Driver	= 0,
			E_ModelPart_Seatbelt,
			E_ModelPart_Count
		};

		struct C_DriverModelPart
		{
			C_DriverModelPart():
				m_pResource(0), m_pModel(0), m_primed(false) {}

			void AddToResourceBag(ResourceUtils::I_ResourceBag& resourceBag, const char* filename);
			void Create(ResourceUtils::I_ResourceBag& resourceBag);

			GameResource::C_ModelResource*			m_pResource;
			ResourceUtils::C_ResourceBagTag			m_tag;
			AnimatedModel::C_AnimatedModelInstance*	m_pModel;
			WorldRender::MaterialParameter			m_shNodeParameter;
			WorldRender::MaterialParameter			m_envMapParameter;
			S8										m_partIndex;
			bool									m_primed;
		};

		C_DriverActorCreate					m_creationParams;
		
		Animation::C_IkChain				m_leftArmIkChain;
		Animation::C_IkChain				m_rightArmIkChain;
		Animation::C_IkChain				m_leftLegIkChain;
		Animation::C_IkChain				m_rightLegIkChain;
		C_DriverAnimationParameters			m_animParameters;
		AnimationTree::C_Tree				m_animTree;
		Animation::C_SteeringController		m_steeringController;

		C_DriverModelPart					m_modelParts[E_ModelPart_Count];
		AnimatedModel::C_AnimatedModelInstance*	m_pDriverModel;

		Maths::HMat43						m_matrix;
		Maths::HVec3						m_driverOffset;
		U32									m_visibilityDetectionFlags;
		Animation::C_EvaluationState*		m_evaluationState;
		const Vehicle::C_ModelInput*		m_pVehicleModelInput;
		bool								m_hasValidAnimationPose;
		bool								m_wasAnimatedLastUpdate;
		bool								m_isVisible;				// Used to control driver visibility in cutscenes.
		bool								m_visibilityChanged;
		bool								m_receivedCollision;
		bool								m_vehicleIsRightHandDrive;
		bool								m_onScreen;
		bool								m_created;
	
		Maths::Point3						m_futurePosition;
		Maths::HVec3						m_otherDriverPos;
		
		Animation::C_SkeletonResource*		m_skeletonResource;
		ResourceUtils::C_ResourceBagTag		m_skeletonTag;
				
		const WorldRender::C_LightSampler*	m_pVehicleLightSampler;
		const void*							m_pVehicleEnvMapTexture;
		SceneGraph::C_SceneGraph*			m_pSceneGraph;

		C_ResourceBagHandle					m_resourceBagHandle;
						
		F32									m_maxCollisionImpulse;
		Maths::Normal3						m_localCollisionDir;
		
		F32									m_lodDistance;
		F32									m_steeringWheelRotationAngle;
		
		S16									m_headJointIndex;
		U8									m_vehicleIndex;
		U8									m_skeletonResourceIndex;
		S8									m_drivingStyle;
		S8									m_currentGear;
		S8									m_handbrake;
	};

} // Game

#endif
