#ifndef ANIMATIONTESTACTOR_H
#define ANIMATIONTESTACTOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <Resource/Resource.h>

//#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
//#include <animation/pose/PoseMatrixCache.h>
//#include <animation/deferred/CommandList.h>
#include <animation/cycle/CycleInstance.h>
//#include <animation/skeleton/SkeletonInstance.h>
#include <animation/ik/IkChain.h>
#include <animation/ik/WalkController.h>
#include <animation/ik/SteeringController.h>

#include <world_render/RenderModelData.h>

namespace Animation
{
	class C_CycleResource;
	class C_SkeletonResource;
	class C_JointTransform;
	class C_EvaluationState;
	class C_CommandList;
}

namespace GameResource
{
	class C_ModelElement;
	class C_ModelResource;
}

namespace WorldRender
{
	class C_ModelInstance;
	class C_Scene;
	class C_EnvironmentSampler;
	struct MaterialParameter;
}

namespace AnimatedModel
{
	class C_AnimatedModelInstance;
}

namespace Gravity
{
	class C_Body;
	class C_CharacterController;
}

namespace Cam2
{
	class C_CharacterCamera;
}

namespace MGCam
{
	class C_PhysicsChecks;
}

namespace Game
{

	class C_GameWorld;

	namespace Actors
	{
	
		enum E_TimeControl
		{
			E_TimeControl_ActorTime	= 0,
			E_TimeControl_AxisX,
			E_TimeControl_AxisY
		};
		
		struct C_AnimationInfo
		{
			C_AnimationInfo():
				timeControl(E_TimeControl_ActorTime)
			{
			}
			
			Util::C_String	cycleFilename;
			E_TimeControl	timeControl;
		};
	
		
		class C_AnimationTestActorCreate : public ::Actors::C_PosActorCreate
		{
		public:
			static Actor::C_ActorTypeId ClassType()			{ return E_ActorTypeId_AnimationTest; }
			const Actor::C_ActorTypeId GetType() const		{ return E_ActorTypeId_AnimationTest; }

			Util::C_String 							m_skeletonFilename;
			Util::C_String 							m_cycleFilename;
			Util::C_String 							m_modelFilename;
			WorldRender::C_Scene*					m_pWorldRenderScene;
		};
		
		class C_AnimationTestActor : public C_GameActor
		{
		public:
			C_AnimationTestActor();
			virtual ~C_AnimationTestActor();
			const Maths::HMat43 &				GetMatrix() const	{ return m_worldMatrix; }
			static const Actor::C_ActorTypeId	TypeId()			{ return E_ActorTypeId_AnimationTest; }
			static const Btl::String32			TypeName()			{ return "AnimationTest"; }
			virtual bool OnMessage(const Messaging::C_MessageBase *pMessage);
			
			#ifndef CFG_GOLD
			virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
			#endif

		protected:
			virtual bool InitialiseImpl();
			virtual bool CreateImpl();

		private:

			void CreateModel();
			void DestroyModel();
			
			void CreatePose(const Animation::C_Cycle* cycle = NULL);
			void DestroyPose();
			
			void CreateCharacter();
			void DestroyCharacter();
			
			void Update( F32 deltaTime );
			
			void DebugRenderSkeleton( DbUtil::I_DebugRenderPtr debugRender, const Animation::C_Pose& pose, const Animation::C_Skeleton& skeleton, 
				bool showBoneTransforms, bool showBones, bool showBoneNames ) const;
			
			bool IsCharacter() const;
			const Animation::C_Skeleton* GetSkeleton() const;
			
			void AnimateObject(Animation::C_CommandList& commandList, F32 deltaTime);
			void AnimateCharacter(Animation::C_CommandList& commandList, F32 deltaTime);
			
		
			enum E_ResourceIndex
			{
				E_ResourceIndex_Skeleton	= 0,
				E_ResourceIndex_Model,
				E_ResourceIndex_Cycle
			};
			
			enum E_ArmIkTarget
			{
				E_ArmIkTarget_Grab	 = 0,
				E_ArmIkTarget_Steering,
				E_ArmIkTarget_Count
			};

			enum E_LegIkTarget
			{
				E_LegIkTarget_FootPlant	= 0,
				E_LegIkTarget_Count
			};

			
			Maths::HMat43						m_worldMatrix;
			
			Animation::C_CycleResource*			m_cycleResource;
			Animation::C_SkeletonResource*		m_skeletonResource;
			GameResource::C_ModelResource*		m_modelResource;
			
			AnimatedModel::C_AnimatedModelInstance*			m_animatedModelInstance;
			
			
			Animation::C_EvaluationState*		m_evaluationState;
			Animation::C_PoseFormat				m_poseFormat;
			//Animation::C_Pose					m_pose;
			//Animation::C_SkeletonInstance		m_skeletonInstance;
			//Animation::C_PoseMatrixCache		m_poseMatrixCache;
			//Animation::C_CommandList			m_commandList;
			Animation::C_CycleInstance			m_cycleInstance;
			Animation::C_IkChain				m_leftArmIkChain;
			Animation::C_IkChain				m_rightArmIkChain;
			Animation::C_IkChain				m_leftLegIkChain;
			Animation::C_IkChain				m_rightLegIkChain;
			Animation::C_SteeringController		m_steeringController;
			Animation::C_WalkController			m_walkController;
			Animation::C_GaitInfo				m_walkGait;
			Animation::C_GaitInfo				m_runGait;
			Animation::C_GaitInfo				m_sprintGait;
			F32									m_cycleSpeed;
			WorldRender::C_EnvironmentSampler*	m_pEnvSampler;
			
			WorldRender::C_ModelInstance*		m_modelInstance;
			const GameResource::C_ModelElement*	m_modelElement;
			
			Gravity::C_Body*					m_body;
			Gravity::C_CharacterController*		m_characterController;
			Cam2::C_CharacterCamera*			m_camera;
			MGCam::C_PhysicsChecks*				m_physicsChecks;
			
			bool								m_isCharacter;
			
			bool								m_leftFootDown;
			bool								m_rightFootDown;
						
			mutable Btl::Vector<Animation::C_JointTransform>		m_modelJointTransforms;
			F32									m_cycleTime;
			F32									m_spinAngle;
		};

	} // namespace Actors


} // namespace Game

#endif //  ANIMATIONTESTACTOR_H

