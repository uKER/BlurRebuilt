#ifndef GRAPH_NODE_H
#define GRAPH_NODE_H

//#include <System/AutoDma.h>
#include "gravity/sharedphysics/AllocationManager.h"


namespace DbUtil
{
	class I_DebugRender;
};

#ifndef USE_SOLVER_2

namespace Solver
{
	class C_Jacobian;
	class C_InvertMJT;
};
#else
namespace Solver2
{
	class C_Jacobian;
	class C_InvertMJT;
};
#endif

namespace Gravity
{
	class C_Body;
	class C_BodyList;
};

namespace SharedPhysics
{
	class C_ConstraintEdge;
	class C_ConstraintTwin;
	class C_Constraint;
	class C_GraphNode;
	class C_RigidBody;
	template <bool bFixedSize> class C_SafeStack;

	class C_Deactivator
	{
	public:
		C_Deactivator(U32 maxCountDown = 15) : m_sSleepCountdown(maxCountDown), m_CountDown(m_sSleepCountdown), m_pGraphNode(NULL),
			m_SleepThresholdSq(Maths::Constants<Maths::HF32>::Zero()), m_ActivateThresholdSq(Maths::Constants<Maths::HF32>::Zero()), m_StaticActivateThresholdSq(-Maths::Constants<Maths::HF32>::One())
		{
			m_StaticActivateThresholdSq = -Maths::Constants<Maths::HF32>::One();
		}
		C_Deactivator(const Maths::HF32 PCREF sleepThresh, const Maths::HF32 PCREF activThresh, C_GraphNode* pNode, U32 maxCountDown = 15) : m_sSleepCountdown(maxCountDown), m_SleepThresholdSq(sleepThresh),
			m_ActivateThresholdSq(activThresh), m_StaticActivateThresholdSq(-Maths::Constants<Maths::HF32>::One()), m_pGraphNode(pNode), m_CountDown(m_sSleepCountdown)
		{
			m_StaticActivateThresholdSq = -Maths::Constants<Maths::HF32>::One();
		}

		NOALIAS inline void SetThresholds(const F32 sleepThresh, F32 activThresh, C_GraphNode* pNode)
		{
			m_SleepThresholdSq = sleepThresh;
			m_ActivateThresholdSq = m_ActivateThresholdSq;
			m_pGraphNode = pNode;
			m_CountDown = m_sSleepCountdown;
		}
		bool CanDeactivate();
		bool MustActivate();
		bool ActiveAndAboveSleepThresh();
		void Activate();
		void LightWeightActivate();

		Maths::HF32 m_SleepThresholdSq, m_ActivateThresholdSq, m_StaticActivateThresholdSq;
		C_GraphNode* m_pGraphNode;
		S32 m_CountDown;
		S32 m_sSleepCountdown;
		void SetSleepCountdown(const U32 countdown)
		{
			m_sSleepCountdown = countdown;
		}
		
	};

	class ALIGN16 C_GraphNode
	{
	public:

		enum _TEMPFLAGS
		{
			E_TYPE_TEMPORARY_NODE
		};
		enum E_NodeFlags
		{
			E_Type_None = 0x0,
			E_Type_Active = 0x1,
			E_Type_Sleeping = 0x2,
			E_Type_Waking = 0x4,
			E_Type_NoddingOff = 0x8,
			E_Type_Sleeping_Or_Waking = (E_Type_Sleeping | E_Type_Waking),
			E_Type_Static = 0x10,
			E_Type_Sleeping_Or_Static = (E_Type_Sleeping | E_Type_Static),
			E_Type_Root_Node = 0x20,
			E_Type_Visited_Island_Generation = 0x40,
			E_Type_Visited_Island_Processing = 0x80,
			E_Type_Visited_Wakeup = 0x100,
			E_Euler_Integrate = 0x200,
			E_RK4Integrate = 0x400,
			E_Type_StaticMesh = 0x800,
			E_Type_KinematicObject = 0x1000,
			E_Type_ZeroInertia = E_Type_Sleeping_Or_Static | E_Type_KinematicObject,
			E_Type_InstancedMesh = 0x2000,
			E_Type_Removed = 0x4000,
			E_Type_NoIntegrate = 0x8000,
			E_Type_Cloth = 0x10000,
			E_ApplyClothDamping = 0x20000,
			E_OneWayClothCollision = 0x40000,
			E_FrameOfReference = 0x80000,
			E_Type_LightWakeUp = 0x100000,
			E_Type_Interfered = 0x200000,
			E_Type_Rope = 0x400000,
			E_Type_NoBuoyancy = 0x800000,
			E_Type_Continuous = 0x1000000,
			E_Type_AlreadyIntegrated = 0x2000000,
			E_Type_Visited_Resistance = 0x4000000,
			E_Type_No_Solver_Force = 0x8000000,
			E_State_Changed = 0x10000000,
			E_Type_RaytraceOnly = 0x20000000,
			E_Type_AnimatedShape = 0x40000000,
			E_Type_Debris = 0x80000000,
			E_Type_BodyStateMask = E_Type_Active | E_Type_Sleeping | E_Type_KinematicObject | 
					E_Type_Static | E_Type_RaytraceOnly | E_Type_Debris,
			//FULL! No more space in 32-bit int
		};

		enum E_ExtendedNodeFlags
		{
			E_Type_NotSubmerged = 0,
			E_Type_Submerged = 1,
			E_Type_PreviouslySubmerged = 2,
			E_Type_Deformable = 3
		};

		C_GraphNode() : m_pPersistentConstraints(NULL), m_pPersistentTwinConstraints(NULL), 
			m_pVolatileConstraints(NULL), m_pVolatileTwinConstraints(NULL), m_pCloseConstraints(NULL), m_pCloseTwinConstraints(NULL),
			 m_pPersistentClothConstraints(NULL), m_pPersistentTwinClothConstraints(NULL),
			m_pClothConstraints(NULL), m_pClothTwinConstraints(NULL),
			m_pBody(NULL), 	m_Index(-1), m_IslandIndex(-1),
			m_NodeFlags(E_Type_Active), m_ExtendedNodeFlags(0), m_Baumgarte(0.8f), m_Slop(0.0001f), m_TotalKineticEnergy(Maths::Constants<Maths::HVec3>::Zero())
		{
			m_Deactivator.m_pGraphNode = this;
			m_pPPuPtr = this;
		}

		C_GraphNode(C_Deactivator & deactivator, const U32 index) : m_pPersistentConstraints(NULL), m_pPersistentTwinConstraints(NULL), 
			m_pVolatileConstraints(NULL), m_pVolatileTwinConstraints(NULL), m_pCloseConstraints(NULL), m_pCloseTwinConstraints(NULL),
			m_pPersistentClothConstraints(NULL), m_pPersistentTwinClothConstraints(NULL),
			m_pClothConstraints(NULL), m_pClothTwinConstraints(NULL),
			m_pBody(NULL), 	m_IslandIndex(-1),
			m_NodeFlags(E_Type_Active), m_ExtendedNodeFlags(0), m_Deactivator(deactivator), m_Baumgarte(0.8f), m_Slop(0.0001f), m_TotalKineticEnergy(Maths::Constants<Maths::HVec3>::Zero())
		{
			m_Deactivator.m_pGraphNode = this;
			m_pPPuPtr = this;
		}

		C_GraphNode(_TEMPFLAGS tempFlags) : m_TotalKineticEnergy(Maths::Constants<Maths::HVec3>::Zero())
		{
			//Use this constructor for a temp storage class, so we don't give it a real index
			m_Deactivator.m_pGraphNode = this;
			m_pPPuPtr = this;
		}

		C_Deactivator m_Deactivator;
		Maths::HVec3 m_TotalKineticEnergy;
		//Persistent constraints are constraints which SHOULD exist between samples (e.g. joints). They
		//can be updated externally, but will not be re-calculated by the physics engine
		C_ConstraintEdge* m_pPersistentConstraints;
		C_ConstraintTwin* m_pPersistentTwinConstraints;
		//Volatile constraints are constraints which updated dynamically by the physics engine. Such constraints
		//may exist over a number of samples, but are expected to change dynamically
		C_ConstraintEdge* m_pVolatileConstraints;
		C_ConstraintTwin* m_pVolatileTwinConstraints;

		//Extra constraints...
		C_ConstraintEdge* m_pCloseConstraints;
		C_ConstraintTwin* m_pCloseTwinConstraints;

		//Persistent constraints are constraints which SHOULD exist between samples (e.g. joints). They
		//can be updated externally, but will not be re-calculated by the physics engine
		C_ConstraintEdge* m_pPersistentClothConstraints;
		C_ConstraintTwin* m_pPersistentTwinClothConstraints;
		//Cloth constraints are constraints which updated dynamically by the physics engine. Such constraints
		//may exist over a number of samples, but are expected to change dynamically
		C_ConstraintEdge* m_pClothConstraints;
		C_ConstraintTwin* m_pClothTwinConstraints;

		//A look-back from the constraint graph to the actual data it represents (rigid body/actor). 
		//There is a 1-1 mapping between rigid bodies/actors and graph nodes

		Gravity::C_Body* m_pBody; 

		C_RigidBody* m_pRigidBody;
		U32 m_Index;
		S32 m_IslandIndex; //A temporary index value within the island
		S32 m_IslandSize;
		U32 m_NodeFlags;
		U32 m_ExtendedNodeFlags;
		F32 m_Slop;
		F32 m_Baumgarte;
		C_GraphNode* m_pPPuPtr;
		

		//static U32 index;

		SharedPhysics::C_ConstraintEdge* FindPrevConstraint(SharedPhysics::C_GraphNode* pNodeB);

		bool ContainsEdge(C_ConstraintEdge* pEdge);

		bool HasEnteredWater()
		{
			return (m_ExtendedNodeFlags & (E_Type_Submerged | E_Type_PreviouslySubmerged)) == E_Type_Submerged;
		}

		bool HasExitedWater()
		{
			return (m_ExtendedNodeFlags & (E_Type_Submerged | E_Type_PreviouslySubmerged)) == E_Type_PreviouslySubmerged;
		}

		//Clears the submerged flag and sets the previously submerged flag!
		void RefreshSubmergedFlags()
		{
			m_ExtendedNodeFlags = (m_ExtendedNodeFlags & (~(E_Type_Submerged | E_Type_PreviouslySubmerged))) | ((m_ExtendedNodeFlags & (E_Type_Submerged)) << 1);
		}

		void SetSubmerged()
		{
			m_ExtendedNodeFlags |= E_Type_Submerged;
		}

		void CalculateBVector(const Maths::HF32 PCREF time);

		NOALIAS inline void Slop(const F32 slop)
		{
			m_Slop = slop;
		}

		NOALIAS inline void Baumgarte(const F32 baumgarte)
		{
			m_Baumgarte = baumgarte;
		}

		NOALIAS inline F32 Slop()
		{
			return m_Slop;
		}

		NOALIAS FORCE_INLINE F32 BaumGarte()
		{
			return m_Baumgarte;
		}

		NOALIAS inline bool HasConstraints()
		{
			return m_pPersistentConstraints || m_pPersistentTwinConstraints || m_pVolatileConstraints || 
				m_pVolatileTwinConstraints;
		}

		NOALIAS inline const C_Deactivator& Deactivator() const
		{
			return m_Deactivator;
		}

		bool CanDeactivateConstraints() const;

		NOALIAS inline void SetDeactivator(C_Deactivator& deactivator)
		{
			m_Deactivator = deactivator;
			m_Deactivator.m_pGraphNode = this;
			m_Deactivator.m_CountDown = m_Deactivator.m_sSleepCountdown;
		}

		NOALIAS inline void ClearNodeFlags(U32 flags)
		{
			m_NodeFlags &= (~flags);
		}

		NOALIAS inline void SetNodeFlags(U32 flags)
		{
			m_NodeFlags |= flags;
		}

		NOALIAS inline bool TestNodeFlags(U32 flags) const
		{
			return (m_NodeFlags & flags) != 0;
		}

		NOALIAS inline void SetExtendedNodeFlags(U32 flags)
		{
			m_ExtendedNodeFlags |= flags;
		}

		NOALIAS inline bool TestExtendedNodeFlags(U32 flags) const
		{
			return (m_ExtendedNodeFlags & flags) != 0;
		}

		NOALIAS inline bool HasInfiniteMass() const
		{
			return (m_NodeFlags & (E_Type_KinematicObject | E_Type_Static)) != 0;
		}

		void WakeUpConstrainedObjects(SharedPhysics::C_SafeStack<true>& stack, Gravity::C_BodyList& wakingList);

		void ResistDeactivationConstrainedObjects(SharedPhysics::C_SafeStack<true>& stack);

		NOALIAS inline bool operator < (const C_GraphNode& other) const __restrict
		{
			return m_Index < other.m_Index;
		}

		NOALIAS inline bool operator <= (const C_GraphNode& other) const __restrict
		{
			return m_Index <= other.m_Index;
		}

		NOALIAS inline bool operator > (const C_GraphNode& other) const __restrict
		{
			return m_Index > other.m_Index;
		}

		NOALIAS inline bool operator >= (const C_GraphNode& other) const __restrict
		{
			return m_Index >= other.m_Index;
		}

		NOALIAS inline bool operator == (const C_GraphNode& other) const __restrict
		{
			return m_Index == other.m_Index;
		}

		NOALIAS inline bool operator != (const C_GraphNode& other) const __restrict
		{
			return m_Index != other.m_Index;
		}
	};

	class ALIGN16 C_Constraint
	{
	public:
		enum E_ConstraintType
		{
			E_Type_Fixed,
			E_Type_Distance,
			E_Type_Spherical,
			E_Type_SphericalLimit,
			E_Type_Revolute,
			E_Type_RevoluteLimit,
			E_Type_Prismatic,
			E_Type_PrismaticLimit,
			E_Type_CarWheel,
			E_Type_Contact,
			E_Type_Invalid,
			E_Type_TwoRotational,
			E_Type_FixedRotational,
			E_Type_RotationalLimit,
			E_Type_Proxy
		};

		enum E_AxisOrder
		{
			E_AxisOrder_XYZ,
			E_AxisOrder_XZY,
			E_AxisOrder_ZXY
		};

		enum E_ConstraintState
		{
			E_ExceededLowLimit,
			E_ExceededHighLimit,
			E_Normal
		};

		C_Constraint():pRoot(NULL), pNodeA(NULL), pNodeB(NULL), m_MassScales(Maths::UNDEFINED)
		{};
		C_Constraint(const E_ConstraintType type);

		virtual ~C_Constraint()
		{
			m_pUserData = NULL;
		}
		
		#if 0 //defined( PLATFORM_SPU )
		C_Constraint( const Spu::C_ConstructVirtualTable& cvt ) {}
		#endif
		
		virtual void DebugRender(DbUtil::I_DebugRender* pRender)
		{
		}

		NOALIAS inline E_ConstraintType ConstraintType()
		{
			return m_ConstraintType;
		}

		NOALIAS inline bool operator < (const C_Constraint& other)
		{
			return m_ConstraintIndex < other.m_ConstraintIndex;
		}

		NOALIAS inline bool operator <= (const C_Constraint& other)
		{
			return m_ConstraintIndex <= other.m_ConstraintIndex;
		}

		NOALIAS inline bool operator > (const C_Constraint& other)
		{
			return m_ConstraintIndex > other.m_ConstraintIndex;
		}

		NOALIAS inline bool operator >= (const C_Constraint& other)
		{
			return m_ConstraintIndex >= other.m_ConstraintIndex;
		}

		NOALIAS inline bool operator == (const C_Constraint& other)
		{
			return m_ConstraintIndex == other.m_ConstraintIndex;
		}

		NOALIAS inline bool operator != (const C_Constraint& other)
		{
			return m_ConstraintIndex != other.m_ConstraintIndex;
		}

		virtual void SetScale(const F32 scale)
		{
			m_Scale = scale;
		}

		virtual void SetRotationalScale(const F32 scale)
		{
		}

		bool FlipConstraint(SharedPhysics::C_GraphNode* pFetchedNodeA, SharedPhysics::C_GraphNode* pFetchedNodeB,
			U32& nodeBFlags);

//#ifndef USE_SOLVER_2
//
//		//No longer required to be virtual!
//		virtual void CreateJacobian(Solver::C_Jacobian* jaco)
//		{
//		}
//		//virtual void CreateLambdaLimit3(Solver::C_Jacobian3* jaco) = 0;
//		virtual void CreateInvertMJT(Solver::C_InvertMJT* imjt, Solver::C_Jacobian* jaco)
//		{
//		}
//#else
//		/*virtual void CreateJacobian(Solver2::C_Jacobian* jaco)
//		{
//		}
//		//virtual void CreateLambdaLimit3(Solver::C_Jacobian3* jaco) = 0;
//		virtual void CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* jaco)
//		{
//		}*/
//#endif
		
		virtual void Release() = 0;
		
		/*NOALIAS inline void StoreWarmstartValues(F32 *warmStart)
		{
			m_WarmStartValues[0] = Maths::Select(_finite(warmStart[0])!= 0, warmStart[0], 0.f);
			m_WarmStartValues[1] = Maths::Select(_finite(warmStart[1])!= 0, warmStart[1], 0.f);
			m_WarmStartValues[2] = Maths::Select(_finite(warmStart[2])!= 0, warmStart[2], 0.f);
			m_WarmStartValues[3] = Maths::Select(_finite(warmStart[3])!= 0, warmStart[3], 0.f);
			m_WarmStartValues[4] = Maths::Select(_finite(warmStart[4])!= 0, warmStart[4], 0.f);
			m_WarmStartValues[5] = Maths::Select(_finite(warmStart[5])!= 0, warmStart[5], 0.f);
			m_WarmStartValues[6] = Maths::Select(_finite(warmStart[6])!= 0, warmStart[6], 0.f);
		}*/

		void StoreWarmstartValues(Maths::VF32 PCREF lambda);

		void StoreWarmstartValues(Maths::Vector2 PCREF lambda);

		void StoreWarmstartValues(Maths::Vector3 PCREF lambda);

		void StoreWarmstartValues(Maths::Vector4 PCREF lambda);

		void StoreWarmstartValues(Maths::Vector6 PCREF lambda);

		void StoreWarmstartValues(Maths::Vector8 PCREF lambda);

		virtual E_ConstraintState GetConstraintState()
		{
			return E_Normal;
		}

		NOALIAS inline void ZeroWarmStartValues()
		{
			m_WarmStartValues = Maths::Const<Maths::Vector8>::Zero();
		}

		virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset) {return true;}
        
        #if !defined( PLATFORM_SPU )
        
		//Note - this automatically sets the mass and inertia scales.
		virtual void SetBodyAMassScale(const Maths::HF32 PCREF scale)
		{
			m_MassScales.X(scale);
			m_MassScales.Z(scale);
		}
		
		//Note - this automatically sets the mass and inertia scales.
		virtual void SetBodyBMassScale(const Maths::HF32 PCREF scale)
		{
			m_MassScales.Y(scale);
			m_MassScales.W(scale);
		}

		//Note - this only sets the inertia scale.
		virtual void SetBodyAInertiaScale(const Maths::HF32 PCREF scale)
		{
			m_MassScales.Z(scale);
		}
		
		//Note - this only sets the inertia scale.
		virtual void SetBodyBInertiaScale(const Maths::HF32 PCREF scale)
		{
			m_MassScales.W(scale);
		}

		virtual void NormaliseMassRatios(const Maths::HF32 PCREF maxMassRatio, const Maths::HF32 PCREF assumedInfiniteMassRatio);
		
		#endif

		virtual void PostStabilise()
		{
			//Do nothing...
		}

		//Assuming that these bodies will be in local store on the SPU!
		virtual void PostStabilise(Gravity::C_Body* pBodyA, Gravity::C_Body* pBodyB)
		{
			//Do nothing...
		}

		NOALIAS __forceinline Maths::HF32 GetBodyAMassScale()
		{
			return m_MassScales.X();
		}

		NOALIAS __forceinline Maths::HF32 GetBodyBMassScale()
		{
			return m_MassScales.Y();
		}

		NOALIAS __forceinline Maths::HF32 GetBodyAInertiaScale()
		{
			return m_MassScales.Z();
		}

		NOALIAS __forceinline Maths::HF32 GetBodyBInertiaScale()
		{
			return m_MassScales.W();
		}

		virtual void SetBaumgarte(F32 baumgarte)
		{
			m_Baumgarte = baumgarte;
		}

		virtual void SetUserData(void* data)
		{
			m_pUserData = data;
		}

		template <typename Serialiser>
		void SaveState(Serialiser& serial)
		{
		}

		template <typename Deserialiser>
		void LoadState(Deserialiser& deserial)
		{
		}

		void* GetUserData()
		{
			return m_pUserData;
		}
        
		//BodyA mass Scale, bodyB mass scale!
		//Maths::HVec2 m_MassScales;
		////BodyA and BodyB
		//Maths::HVec2 m_InertiaScales;

		//X = BodyA mass scale
		//Y = BodyB mass scale
		//Z = BodyA inertia scale
		//W = BodyB inertia scale
		Maths::HVec4 m_MassScales;

		/*Maths::HVec3 m_BodyAInertiaScale;
		Maths::HVec3 m_BodyBInertiaScale;*/

		//ALIGN16 F32 m_WarmStartValues[7];
		Maths::Vector8 m_WarmStartValues;
		E_ConstraintType m_ConstraintType;
		U32 m_ConstraintIndex;
		C_Constraint* m_pPPUConstraint;
		F32 m_Scale;
		F32 m_Baumgarte;
		static U32 m_NumConstraints;
		

		C_GraphNode* pNodeA;
		C_GraphNode* pNodeB;
		C_Constraint* pRoot;
		void* m_pUserData;

	};

	class C_ConstraintPoint : public C_BlockItem<C_ConstraintPoint>
	{
	public:
		C_ConstraintPoint(C_Constraint* pConstraint = NULL) : m_pConstraint(pConstraint), m_pPrev(NULL)
		{
		}
	
		C_Constraint* m_pConstraint;	//The thing which stores the constraint itself...
		U32 m_HashValue;				//Used to index contacts!!!!
		C_ConstraintPoint* m_pPrev;
	};

	inline void C_Deactivator::Activate()
	{
		m_CountDown = C_Deactivator::m_sSleepCountdown;
	}

	inline void C_Deactivator::LightWeightActivate()
	{
		m_CountDown = 4;
	}
}

#endif
