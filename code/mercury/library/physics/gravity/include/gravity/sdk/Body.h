#ifndef NEWBODY_H
#define NEWBODY_H

#include "gravity/broadphase/Object.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/SPU_Memory.h"
#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/Spring.h"
#include "system/scheduler/jobqueue.h"
#include "Btl/Containers/Vector.h"

namespace SharedPhysics
{
	template <typename Type> class C_BlockItem;
};


namespace Collision
{
	class C_Shape;
	class C_Sphere;
	class C_Ray;
};
namespace Gravity
{
	/*		C_Body is the external view of a body/actor 
	 *		C_Body is composed of:
	 *			(1) C_Object - the broad phase/collision detection representation
	 *			(2) C_Shape - the narrow-phase collision detection representation 
	 *			(3) C_RigidBody - the actual physics representation of the object
	 *			(4) C_GraphNode - the internal solver representation of the object
	 *				- C_GraphNode has a look-back to the C_Object
	 *				- C_GraphNode stores a C_RigidBody ptr
	 *				- Stores pointers to constraints which connect objects to each-other
	 *
	 *		C_Body handle's state buffering. The external state is stored in the C_Body, whereas the 
	 *		internal state is stored in the C_RigidBody. At a critical stage of the game, the internal state
	 *		in the C_RigidBody is transferred into the C_Body
	 */

	class C_Action;
	class C_Scene;
	class C_Body;
	class C_ImportScene;

	class C_BodyRemoveCallback
	{
	public:
		virtual void BodyRemoved(Gravity::C_Body* pBody, void* pUserData) = 0;
	};
	//class C_Body;
	//class C_SceneSize;

	//An integration callback which allows the user to define custom integration behaviours for a given body.
	//Must be attached to a given body
	class C_IntegrationCallback
	{
	public:
		virtual ~C_IntegrationCallback()
		{
		}

		virtual Scheduler::GroupHandle BeginIntegrationTask(Scheduler::GroupHandle group, Scheduler::PolicyHandle policy, float time) = 0;
		virtual void PostIntegration(const Maths::C_TimeS PCREF time) {}
	};


	struct C_ExternalConstants
	{
		Maths::HVec3 m_Gravity;
		//Wind and air resistance in here as a place-holder. Code isn't yet written to respond to these
		Maths::HVec3 m_Wind;
		Maths::HVec3 m_AirResistance;

		C_ExternalConstants() : m_Gravity(Maths::Constants<Maths::HVec3>::Zero()), m_Wind(Maths::Constants<Maths::HF32>::Zero()), 
			m_AirResistance(Maths::Constants<Maths::HF32>::Zero())
		{
		}
	};

	struct C_BodyState
	{
		Maths::HVec3	m_Position;
		Maths::HMat33	m_Orientation;
		Maths::HVec3	m_LinearVelocity;
		Maths::HVec3	m_AngularVelocity;
		Maths::HVec3	m_Force;
		Maths::HVec3	m_Torque;
		U32				m_ExternalBodyState;
		bool			m_HasStateChanged;
	};

	class C_SceneSize
	{
	public:
		Collision::C_AxisBoxMinMax m_WorldBox;
		Maths::HVec3 m_DesiredSize;
		Maths::HVec3 m_MaxCellSize;
		S32 m_MaxDepth;

	public:
		C_SceneSize()
		{
		}

		C_SceneSize(Collision::C_AxisBoxMinMax& box,
			const Maths::HVec3 PCREF desiredSize, 
			const Maths::HVec3 PCREF maxCellSize, S32 maxDepth) : m_WorldBox(box),
			m_DesiredSize(desiredSize), m_MaxCellSize(maxCellSize), m_MaxDepth(maxDepth)
		{
		}
	};

	class C_Body;
	class C_CharacterController;
	class C_KinematicController;
	class C_AnimatedShapeController;
	class C_RagdollController;

	//A class which encodes any 4 vector structure into a single block of data...
	class C_CommandBundle
	{
	public:
		Maths::HMat44 m_Matrix;
		union
		{
			SharedPhysics::C_Constraint* m_pConstraint;
			SharedPhysics::C_Spring* m_pSpring;
			C_Body* m_pBody;
			C_CharacterController* m_pCharacterController;
			C_KinematicController* m_pKinematicController;
			C_AnimatedShapeController* m_pAnimatedShapeController;
			C_RagdollController* m_pRagdollController;
			C_ImportScene* m_pImportScene;
			U32 m_MaxDepth;
		};
		union
		{
			C_Scene* m_pScene;
			Collision::C_Shape* m_pShape;
		};
		void* m_pUserData;
		C_BodyRemoveCallback* m_pCallback;
	public:
		void SetSceneSize(const C_SceneSize& size)
		{
			m_Matrix.AX(Maths::HVec4(size.m_WorldBox.m_min));
			m_Matrix.AY(Maths::HVec4(size.m_WorldBox.m_max));
			m_Matrix.AZ(Maths::HVec4(size.m_DesiredSize));
			m_Matrix.AW(Maths::HVec4(size.m_MaxCellSize));
			m_MaxDepth = size.m_MaxDepth;
		}

		C_SceneSize GetSceneSize()
		{
			Collision::C_AxisBoxMinMax box;
			box.m_min = (Maths::HVec3)m_Matrix.AX();
			box.m_max = (Maths::HVec3)m_Matrix.AY();
			return C_SceneSize(box, (Maths::HVec3)m_Matrix.AZ(), (Maths::HVec3)m_Matrix.AW(), m_MaxDepth);
		}

		void SetTensor(const Maths::HMat33& tensor)
		{
			//m_Matrix.Rotation(tensor);
			m_Matrix.AX(Maths::HVec4(tensor.AX()));
			m_Matrix.AY(Maths::HVec4(tensor.AY()));
			m_Matrix.AZ(Maths::HVec4(tensor.AZ()));
		}

		Maths::HMat33 GetTensor()
		{
			return Maths::HMat33((Maths::HVec3)m_Matrix.AX(), (Maths::HVec3)m_Matrix.AY(), (Maths::HVec3)m_Matrix.AZ());
		}

		void SetValue(const Maths::HVec3 PCREF value)
		{
			m_Matrix.AX(Maths::HVec4(value));
		}

		Maths::HVec3 GetValue()
		{
			return (Maths::HVec3)m_Matrix.AX();
		}

		void SetPosition(const Maths::HVec3 PCREF position)
		{
			m_Matrix.AY(Maths::HVec4(position));
		}

		Maths::HVec3 GetPosition()
		{
			return (Maths::HVec3)m_Matrix.AY();
		}

		void SetQuaternion(const Maths::C_Quaternion PCREF quat)
		{
			m_Matrix.AZ((Maths::HVec4)quat);
		}

		Maths::C_Quaternion GetQuaternion()
		{
			return (Maths::C_Quaternion)m_Matrix.AZ();
		}
	};

	class C_DeferredBodyCommand : public SharedPhysics::C_BlockItem<C_DeferredBodyCommand>
	{
	public:

		enum E_CommandType
		{
			E_NoCommand,
			E_OverridePosition,
			E_OverrideOrientation,
			E_OverrideLinearVelocity,
			E_OverrideAngularVelocity,
			E_OverrideInertiaTensor,
			E_ApplyImpulse,
			E_ApplyImpulseLocalSpace,
			E_AddImpulse,
			E_AddAngularImpulse,
			E_Deactivate,
			E_Activate,
			E_MakeStatic,
			E_ChangeBodyState,
			E_MakeRaytraceOnly,
			E_DisableRaytraceOnly,
			E_MakeDebris,
			E_DisableDebris,
			E_MakeKinematic,
			E_AddBody,
			E_RemoveBody,
			E_BroadPhaseWakeUp,
			E_AddClothBody,
			E_RemoveClothBody,
			E_AddDeformableBody,
			E_RemoveDeformableBody,
			E_AddRopeBody,
			E_RemoveRopeBody,
			E_AddFluidBody,
			E_RemoveFluidBody,
			E_AddFluidMeshBody,
			E_RemoveFliudMeshBody,
			E_ReleaseBody,
			E_AddStaticBody,
			E_RemoveStaticBody,
			E_RemoveBodyNoDelete,
			E_AddMeshBody,
			E_RemoveMeshBody,
			//E_AddContinuousBody,
			//E_RemoveContinuousBody,
			E_SetCentreOfMassOffset,
			E_UpdateBodyShape,
			E_MaxBodyCommands,
			////Body commands must be above E_MaxBodyCommands!
			E_AddKinematicController,
			E_RemoveKinematicController,
			E_AddAnimatedShapeController,
			E_RemoveAnimatedShapeController,
			E_AddRagdollController,
			E_RemoveRagdollController,
			E_AddCharacterController,
			E_RemoveCharacterController,
			E_AddConstraint,
			E_RemoveConstraint,
			E_AddSpring,
			E_RemoveSpring,
			E_ResizeWorld,
			E_DeleteImportScene
		};

		C_DeferredBodyCommand(const Maths::C_Quaternion PCREF newOrientation, C_Body* pBody)
		{
			m_CommandBundle.SetQuaternion(newOrientation);
			m_CommandBundle.m_pBody = pBody;
			m_CommandType = E_OverrideOrientation;
		}

		C_DeferredBodyCommand(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF impulsePosition, C_Body* pBody)
		{
			m_CommandBundle.SetValue(impulse);
			m_CommandBundle.SetPosition(impulsePosition);
			m_CommandBundle.m_pBody = pBody;
			m_CommandType = E_ApplyImpulse;
		}

		C_DeferredBodyCommand(const E_CommandType type, const Maths::HVec3 PCREF value, C_Body* pBody)
		{
			m_CommandBundle.SetValue(value);
			m_CommandBundle.m_pBody = pBody;
			m_CommandType = type;
		}

		C_DeferredBodyCommand(const E_CommandType type, C_Body* pBody)
		{
			m_CommandBundle.m_pBody = pBody;
			m_CommandType = type;
		}

		C_DeferredBodyCommand(const Maths::HMat33 PCREF matrix, C_Body* pBody)
		{
			m_CommandBundle.SetTensor(matrix);
			m_CommandBundle.m_pBody = pBody;
			m_CommandType = E_OverrideInertiaTensor;
		}
		
		C_DeferredBodyCommand()
		{
			m_CommandType = E_NoCommand;
		}

		C_DeferredBodyCommand(SharedPhysics::C_Constraint* pConstraint)
		{
			m_CommandBundle.m_pConstraint = pConstraint;
			m_CommandType = E_RemoveConstraint;
		}

		//TODO - encode data into this!!!
		/*C_SceneSize m_SceneSize;
		Maths::HVec3 m_Value;
		Maths::HMat33 m_Tensor;
		Maths::HVec3 m_ImpulsePosition;
		Maths::C_Quaternion m_NewOrientation;		*/

		C_CommandBundle m_CommandBundle;
		E_CommandType m_CommandType;

	};

	class C_Scene;

	class ALIGN16 C_Body : public SharedPhysics::C_BlockItem<C_Body>
	{
		friend class C_BodyHelper;
		friend class C_ImportSceneStaticInstance;
	public:
		friend class C_BodyList;
		friend class C_AnimatedShapeController;
		friend void CollideMesh(Scheduler::Task* pTask);
		friend void CollideGroundPlane(Scheduler::Task* pTask);
		friend void CollideDynamic(Scheduler::Task* pTask);
		friend void PostIntegrate(Scheduler::Task* pTask);
		enum E_BodyFlags
		{
			E_Type_Apply_Gravity = 1,
			E_Type_Overridden_PosRot = 2,
			E_Type_Overridden_Velocities = 4,
			E_InsertedIntoWorld = 8,
			E_TemporarilyRemoved = 16,
			E_Type_Removed = 32
		};

	private:

		friend class C_Scene; 
		//Can't change the order, this is spu friendly structure
		Gravity::C_Object m_Object;
		SharedPhysics::C_GraphNode m_GraphNode;
		SharedPhysics::C_RigidBody m_RigidBody;
		Maths::HVec3 m_ElementOffset; //Copied from shape! Avoids any L2 cache misses dereferencing the shape pointer!
		
		Maths::HF32 m_ContinuousThreshold; //In energy!!!
		C_BodyState m_ExternalState;
		Collision::C_Shape* m_pShape;
		Collision::C_Shape* m_pFutureShape;
		C_Scene* m_pOwningScene;
		C_Action* m_pAction;
		C_ExternalConstants* m_pExternalConstants;
		C_IntegrationCallback* m_pIntegrationCallback;
		void* m_pUsrData;
		U32 m_BodyFlags;
		S32 m_RemapExportId;
		S32 m_OriginalExportId;//the unique id come from maya, so that the constraint can find its relative body
		S32 m_ConstraintBodyId;
		C_Body* m_pPrev;

		
		//An array of actions which can be performed on a body...

	public:
		C_Body(Collision::C_Shape* pShape, SharedPhysics::C_RigidBody& body, const Maths::HF32 PCREF sleepThreshold, 
			const Maths::HF32 PCREF wakeupThreshold, C_Scene* scene, const U32 index);

		C_Body();

		~C_Body();

		static C_Body* GetBody(C_Object* pObject);

		static C_Body* GetBody(SharedPhysics::C_GraphNode* pGraphNode);

		static C_Body* GetBody(SharedPhysics::C_RigidBody* pRigid);

		void SetIndex(U32 index);

		void GetConstraints(Btl::Vector<SharedPhysics::C_Constraint*>& resultVector);

		void UpdateShape(Collision::C_Shape* pShape);

		void SetContinuousSpeedThreshold(const Maths::HF32 PCREF speed);

		bool IsContinuousShape() const;

		bool IsContinuous() const;

		void SetMaxLinearVelocity(const Maths::HF32 PCREF maxLinearVelocity);
		
		void SetMaxAngularVelocity(const Maths::HF32 PCREF maxAngularVelocity);

		const Maths::HF32 Mass() const;

		const Maths::HF32 ReciprocalMass() const;

		//Returns current inertia tensor - note - INERTIA TENSOR - not local-space I-Body matrix
		const Maths::HMat33 InertiaTensor() const;

		void Slop(const F32 slop);

		void Baumgarte(const F32 bg);

		Maths::HVec3 GetGravity();
		
		Maths::HVec3 PointVelocity(const Maths::HVec3 PCREF worldPos) const;

		Maths::HVec3 ElementOffset() const { return m_ElementOffset; }


		bool Raytrace(Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache);

		void SetBuoyancyCoefficient(const Maths::HF32 PCREF coefficient);
		void SetLinearDragCoefficient(const Maths::HF32 PCREF coefficient);
		void SetAngularDragCoefficient(const Maths::HF32 PCREF coefficient);

		void CalculateBodyVolume();

		Maths::HF32 GetBuoyancyCoefficient() const;
		Maths::HF32 GetLinearDragCoefficient() const;
		Maths::HF32 GetAngularDragCoefficient() const;
	
		void FlagInsertedIntoWorld();

		SharedPhysics::C_Integrator& GetIntegrator();

		void MakeFrameOfReference();

		void UnMakeFrameOfReference();

		void SetExternalConstants(C_ExternalConstants* pExternalConstants);

		void SetDeactivator(SharedPhysics::C_Deactivator& deactivator);

		void SetLinearDampingCoefficient(const Maths::HF32 PCREF damping);
		
		void SetAngularDampingCoefficient(const Maths::HF32 PCREF damping);

		void MakeOneWayClothObject();

		void ClearOneWayClothObject();

		void DisableDefaultIntegrationStep();
		void EnableDefaultIntegrationStep();
		void SetIntegrationCallback(C_IntegrationCallback* pCallback);

		void ClearEdgeFlags();

		void Activate();

		void Deactivate();

		void MakeStatic();

		void MakeRaytraceOnly();

		void DisableRaytraceOnly();

		void MakeDebris();

		void DisableDebris();

		void MakeKinematic();

		void MakeStaticMesh();

		void MakeInstancedCollidableMesh();

		SharedPhysics::C_RigidBody& GetBody()
		{
			return m_RigidBody;
		}
		
		const SharedPhysics::C_RigidBody& GetBody() const
		{
			return m_RigidBody;
		}

		const SharedPhysics::C_GraphNode* GraphNode() const __restrict;

		void SetBody(SharedPhysics::C_RigidBody& body);

		void SetRemapExportBodyId(const S32 id);
	
		S32 GetRemapExportBodyId() const;

		void SetOriginalExportBodyId(const S32 id);

		S32 GetOriginalExportBodyId() const;

		void SetConstraintBodyId(const S32 id);

		S32 GetConstraintBodyId() const;

		Collision::C_Shape* GetShape()
		{
			return m_pFutureShape;
		}

		const Collision::C_Shape* GetShape() const
		{
			return m_pFutureShape;
		}

		Gravity::C_Object* GetObject();
	
		const Gravity::C_Object* const GetObject() const;

		const Collision::C_CRAxisBox GetLocalBoundingBox() const;

		//Warning - this will CALCULATE the local sphere, not just return it.
		//Please cache the result of this and transform it rather than repeatedly
		//calling this when you need it!
		const Collision::C_Sphere GetLocalBoundingSphere() const;
		

		void SetScene(C_Scene* pScene);
		
		C_Scene* GetScene() const;

		bool IsExternalActive() const;

		bool IsExternalSleeping() const;

		bool IsExternalStatic() const;

		bool IsExternalKinematic() const;

		bool IsExternalDebris() const;

		bool IsExternalRaytraceOnly() const;

		bool IsActive() const;

		bool IsDynamic() const;

		NOALIAS __forceinline const SharedPhysics::C_GraphNode* GetGraphNode() const
		{
			return &m_GraphNode;
		}
		
		NOALIAS __forceinline SharedPhysics::C_GraphNode* GetGraphNode()
		{
			return &m_GraphNode;
		}

		void SynchroniseState();

		bool MustSolve();

		void SynchronisePosRot();
	

		bool TestBodyFlags(U32 bodyFlags)
		{
			return (m_BodyFlags & bodyFlags) != 0;
		}
	
		void SetBodyFlags(U32 bodyFlag);

		void ClearBodyFlags(U32 bodyFlag);

		C_IntegrationCallback* GetIntegrationCallback();
	

		template <class Data>
		Data* GetUserData() const
		{
			return (Data*)m_pUsrData;
		}

		void SetUserData(void* data);
		

		void LoadExternalForces();
		void ZeroExternalForces();
		void ClearExternalForces();
		void ApplyDeferredCommand(C_DeferredBodyCommand& command);
		void ApplyImpulse(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF position);
		void ApplyImpulseLocalSpace(const Maths::HVec3 PCREF impulse, const Maths::HVec3 PCREF position);
		void AddImpulse(const Maths::HVec3 PCREF impulse);
		void AddAngularImpulse(const Maths::HVec3 PCREF impulse);
		void ApplyForce(const Maths::HVec3 PCREF force, const Maths::HVec3 PCREF position);
		void AddForce(const Maths::HVec3 PCREF force);
		void AddTorque(const Maths::HVec3 PCREF torque);
		void Force(const Maths::HVec3 PCREF force);
		Maths::HVec3 Force() const;
		void Torque(const Maths::HVec3 PCREF torque);
		Maths::HVec3 Torque();

		void Position(const Maths::HVec3 PCREF position);
		Maths::HVec3 Position() const;
		Maths::HVec3 CenterOfRotation() const;

		void Orientation(const Maths::C_Quaternion PCREF orientation);
		void Orientation(const Maths::HMat33 PCREF orientation);
		Maths::HMat33 Orientation() const;
		Maths::C_Quaternion GetQuaternion() const;

		void LinearVelocity(const Maths::HVec3 PCREF velocity);
		Maths::HVec3 LinearVelocity() const;

		void AngularVelocity(const Maths::HVec3 PCREF angVel);
		Maths::HVec3 AngularVelocity() const;

		void SetCentreOfMassOffset(const Maths::HVec3 PCREF offset);

		//Set/Get matrices including relative offsets calculated when remapping objects to their centre-of-mass
		Maths::HMat43 ElementMatrix() const;
		void ElementMatrix(const Maths::HMat43 PCREF matrix);

		Maths::HMat33 IBodyMatrix() const;
		Maths::HMat33 IBodyInverse() const;
		void IBodyMatrix(const Maths::HMat33 PCREF matrix);

		void SetAction(C_Action* pAction);

		void SetMass(const Maths::HF32 PCREF mass);
		

		void Integrate(const SharedPhysics::C_Integrator & integrator, const Maths::C_TimeS PCREF time);

		bool MatchExternalState();

		bool operator < (const C_Body& other) const;

		bool operator <= (const C_Body& other) const;
	

		bool operator > (const C_Body& other) const;
	

		bool operator >= (const C_Body& other) const;
		

		bool operator == (const C_Body& other) const;
	
		
		bool operator != (const C_Body& other) const;


private:
		Maths::HMat43 InternalElementMatrix() const;
		void SynchroniseStateFlags();
		bool HasExternalStateChanged();
		void _Activate();
		void _Deactivate();
		void _MakeStatic();
		void _MakeRaytraceOnly();
		void _DisableRaytraceOnly();
		void _MakeDebris();
		void _DisableDebris();
		void _MakeKinematic();
		void WakeUpBroadPhase();
		void DoWakeUpBroadPhase(Gravity::C_DistributionTree* tree);
		void ApplyForcesAndTorques(const Maths::HF32 PCREF time);
		void UpdateBodyShape(Collision::C_Shape* pNewShape);
		void UpdateConstraints(const Maths::HVec3 PCREF elementOffset);
		void SetShape(Collision::C_Shape* pShape);

	};

	class C_BodyList
	{
	public:
		C_Body* m_pFirstBody;
		C_BodyList() : m_pFirstBody(NULL)
		{
		}

		void AddBody(C_Body* pBody);
		void RemoveBody(C_Body* pBody);
		void RemoveBodyNoClear(C_Body* pBody);
		void ClearList();
	};

	class ALIGN16 C_ConstraintBody
	{
	public:
		SharedPhysics::C_GraphNode m_GraphNode;
		SharedPhysics::C_RigidBody m_RigidBody;
	}; 
};

#endif
