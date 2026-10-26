#ifndef RAGDOLL_CONTROLLER_H
#define RAGDOLL_CONTROLLER_H

#include "gravity/sharedphysics/sortedArray.h"
#include "gravity/sdk/Body.h"
#include "gravity/sharedphysics/GraphNode.h"

/*
 * A ragdoll controller - stores a list of ragdoll bodies and constraints and provides
 * functionality to pose individual ragdoll body parts/joints.
 */

namespace DbUtil
{
	class I_DebugRender;
}

namespace Solver2
{
	class C_FixedRotationalConstraint;
}

namespace Gravity
{

class C_Scene;
class C_RagdollController;
// C_Config, used to create a ragdoll C_Controller from bodies and constraints that have already been created in the scene

class C_RagdollConstraint
{
public:
	friend class C_RagdollController;
	SharedPhysics::C_Constraint* m_pConstraint;
	Solver2::C_FixedRotationalConstraint* m_pFixedConstraint;
	U32 m_IndexA, m_IndexB;
	

	C_RagdollConstraint() : m_pConstraint(NULL), m_pFixedConstraint(NULL)
	{
	}
	void Initialise(SharedPhysics::C_Constraint* pConstraint, U32 indexA, U32 indexB,
		SharedPhysics::C_Constraint* pFixedConstraint = NULL);
};

class C_RagdollConfig
{
friend class C_RagdollController;
public:
	C_RagdollConfig() : m_Bodies(32), m_Constraints(32)
	{
	}
	
	~C_RagdollConfig()
	{
	}

	const C_RagdollConfig& operator=( const C_RagdollConfig& other )
	{
		m_Bodies.m_numElements = 0;
		m_Constraints.m_numElements = 0;
		for(S32 i=0; i < other.m_Bodies.m_numElements; ++i)
		{
			m_Bodies.AddToEnd(&other.m_Bodies[i]);
		}

		for(S32 i=0; i < other.m_Constraints.m_numElements; ++i)
		{
			m_Constraints.AddToEnd(&other.m_Constraints[i]);
		}
		return *this;
	}
	
	const S32 GetBodyCount() const
	{
		return m_Bodies.m_numElements;
	}
	
	void SetBodyCount( const S32 bodyCount )
	{
		m_Bodies.Resize(bodyCount);
	}
	
	void SetBody( const S32 index, Gravity::C_Body* pBody )
	{
		BIZ_ASSERT(index < m_Bodies.m_numElements);
		m_Bodies[index] = pBody;
	}

	void SetBody(Gravity::C_Body* pBody)
	{
		m_Bodies.AddToEnd(&pBody);
	}

    Gravity::C_Body* GetBody( const S32 index ) const
	{
		return m_Bodies[index];
	}

    const S32 GetConstraintCount() const
	{
		return m_Constraints.m_numElements;
	}

    void SetConstraintCount( const S32 constraintCount )
	{
		m_Constraints.Resize(constraintCount);
	}

    void SetConstraint( const S32 index, C_RagdollConstraint& pConstraint )
	{
		BIZ_ASSERT(index < m_Constraints.m_numElements);
		m_Constraints[index] = pConstraint;
	}

	void SetConstraint(C_RagdollConstraint& constraint)
	{
		m_Constraints.AddToEnd(&constraint);
	}
	
	C_RagdollConstraint& GetConstraint( const S32 index ) const
	{
		return m_Constraints[index];
	}

	S32 GetIndex(C_Body* pBody)
	{
		for(S32 a = 0; a < m_Bodies.m_numElements; ++a)
		{
			if(pBody == m_Bodies[a])
				return a;
		}
		return -1;
	}

private:

	SharedPhysics::SortedArray<C_Body* > m_Bodies;

	SharedPhysics::SortedArray< C_RagdollConstraint > m_Constraints;

};

// C_Pose. Array of body matricies used to set and retrieve ragdoll transformations
class C_RagdollQuaternionPose
{
	friend class C_RagdollController;
public:
	C_RagdollQuaternionPose() : m_BodyQuaternions(32), m_BodyPositions(32)
	{
	}
	~C_RagdollQuaternionPose()
	{
	}
	C_RagdollQuaternionPose( const C_RagdollQuaternionPose& other )
	{
		m_BodyQuaternions.m_numElements = 0;
		for(S32 i=0; i < other.m_BodyQuaternions.m_numElements; ++i)
		{
			m_BodyQuaternions.AddToEnd(&other.m_BodyQuaternions[i]);
		}
		m_BodyPositions.m_numElements =0;
		for(S32 i=0; i< other.m_BodyPositions.m_numElements; ++i)
		{
			m_BodyPositions.AddToEnd(&other.m_BodyPositions[i]);
		}
	}
	
	const C_RagdollQuaternionPose& operator=( const C_RagdollQuaternionPose& other )
	{
		m_BodyQuaternions.m_numElements = 0;
		for(S32 i=0; i < other.m_BodyQuaternions.m_numElements; ++i)
		{
			m_BodyQuaternions.AddToEnd(&other.m_BodyQuaternions[i]);
		}
		m_BodyPositions.m_numElements =0;
		for(S32 i=0; i< other.m_BodyPositions.m_numElements; ++i)
		{
			m_BodyPositions.AddToEnd(&other.m_BodyPositions[i]);
		}
		return *this;
	}

	void Reset()
	{
		m_BodyQuaternions.m_numElements = 0;
		m_BodyPositions.m_numElements = 0;
	}
	
	void SetBodyCount( const S32 bodyCount )
	{
		m_BodyQuaternions.Resize(bodyCount);
	}

	const S32 GetBodyCount() const
	{
		return m_BodyQuaternions.m_numElements;
	}
	
	void SetBodyQuaternion( const S32 index, const Maths::C_Quaternion PCREF m )
	{
		m_BodyQuaternions[index] = m;
	}

	void SetBodyQuaternion(Maths::C_Quaternion PCREF m)
	{
		m_BodyQuaternions.AddToEnd(&m);
	}

	void SetBodyPosition( const S32 index, const Maths::HVec3 PCREF p )
	{
		m_BodyPositions[index] = p;
	}

	void SetBodyPosition(Maths::HVec3 PCREF p)
	{
		m_BodyPositions.AddToEnd(&p);
	}


	const Maths::C_Quaternion GetBodyQuaternion( const S32 index ) const
	{
		return m_BodyQuaternions[index];
	}

	const Maths::HVec3 GetBodyPosition( const S32 index ) const
	{
		return m_BodyPositions[index];
	}

private:
	SharedPhysics::SortedArray<Maths::C_Quaternion> m_BodyQuaternions;

	SharedPhysics::SortedArray<Maths::HVec3 > m_BodyPositions;
	
};

class C_RagdollRelativePose
{
	friend class C_RagdollController;
public:
	C_RagdollRelativePose() : m_RelativeQuatTrans(32)
	{
	}

	~C_RagdollRelativePose()
	{
	}

	C_RagdollRelativePose(const C_RagdollRelativePose& other)
	{
		m_RelativeQuatTrans.m_numElements = 0;
		for(S32 i=0; i<other.m_RelativeQuatTrans.m_numElements; ++i)
		{
			m_RelativeQuatTrans.AddToEnd(&other.m_RelativeQuatTrans[i]);
		}
	}

	C_RagdollRelativePose& operator=(const C_RagdollRelativePose& other)
	{
		m_RelativeQuatTrans.m_numElements = 0;
		for(S32 i=0; i<other.m_RelativeQuatTrans.m_numElements; ++i)
		{
			m_RelativeQuatTrans.AddToEnd(&other.m_RelativeQuatTrans[i]);
		}

		return *this;

	}

	const Maths::C_Quaternion GetRelativeQuaternion(const S32 index) const;
	
	const Maths::HVec3 GetRelativePosition(const S32 index) const;

	const Maths::QuatTrans GetRelativeQuatTrans(const S32 index)const
	{
		return m_RelativeQuatTrans[index];
	}

	void SetQuatTrans(Maths::QuatTrans PCREF relQuat)
	{
		m_RelativeQuatTrans.AddToEnd(&relQuat);
	}

private:
	
	SharedPhysics::SortedArray<Maths::QuatTrans > m_RelativeQuatTrans;
};



class C_Weights
{
public:

	C_Weights()
	{
	}

	~C_Weights()
	{
	}

	C_Weights(const F32 animationWeight, const F32 convergenceWeight): m_AnimationWeight(animationWeight),
		m_ConvergenceWeight(convergenceWeight)
	{
	}

	

	C_Weights(const C_Weights& other)
	{
		//m_VelocityWeight = other.m_VelocityWeight;
		m_AnimationWeight = other.m_AnimationWeight;
		m_ConvergenceWeight = other.m_ConvergenceWeight;
	}

	
	const C_Weights& operator = (const C_Weights& other)
	{
		//m_VelocityWeight = other.m_VelocityWeight;
		m_AnimationWeight = other.m_AnimationWeight;
		m_ConvergenceWeight = other.m_ConvergenceWeight;

		return *this;
	}

	// How much each body's velocity need to be change to reach its target pose in world space
	//F32 m_VelocityWeight;
	// How much each body is trying to reach its target pose in world space
	F32 m_AnimationWeight;
	F32 m_ConvergenceWeight;
};

class C_RagdollWeights
{
public:
	friend class C_RagdollController;
	C_RagdollWeights() :m_BodyWeights(32), m_ConstraintWeights(32)
	{
	}
	
	~C_RagdollWeights()
	{
	}
	
	C_RagdollWeights( const C_RagdollWeights& other )
	{
		
		m_BodyWeights.m_numElements = 0;
		m_ConstraintWeights.m_numElements = 0;

		for(S32 i=0; i<other.m_BodyWeights.m_numElements; ++i)
		{
			m_BodyWeights.AddToEnd(&other.m_BodyWeights[i]);
		}


		for(S32 i=0; i<other.m_ConstraintWeights.m_numElements; ++i)
		{
			m_ConstraintWeights.AddToEnd(&other.m_ConstraintWeights[i]);
		}

	}
	
	const C_RagdollWeights& operator=( const C_RagdollWeights& other )
	{
		
		m_BodyWeights.m_numElements = 0;
		m_ConstraintWeights.m_numElements = 0;

		for(S32 i=0; i<other.m_BodyWeights.m_numElements; ++i)
		{
			m_BodyWeights.AddToEnd(&other.m_BodyWeights[i]);
		}

		for(S32 i=0; i<other.m_ConstraintWeights.m_numElements; ++i)
		{
			m_ConstraintWeights.AddToEnd(&other.m_ConstraintWeights[i]);
		}

		return *this;
	}
	
	void Lerp( const C_RagdollWeights& other, const F32 blend )
	{
		const F32 rest = 1.f - blend;
		for(S32 i=0; i<m_BodyWeights.m_numElements; ++i)
		{
			//m_BodyWeights[i].m_VelocityWeight = blend*other.m_BodyWeights[i].m_VelocityWeight + rest*m_BodyWeights[i].m_VelocityWeight;
			m_BodyWeights[i].m_AnimationWeight = blend*other.m_BodyWeights[i].m_AnimationWeight + rest*m_BodyWeights[i].m_AnimationWeight;
		}

		for(S32 i=0; i<m_ConstraintWeights.m_numElements; ++i)
		{
			m_ConstraintWeights[i] = blend*other.m_ConstraintWeights[i] + rest*m_ConstraintWeights[i];
		}
	}
	
	const S32 GetBodyCount() const
	{
		return m_BodyWeights.m_numElements;
	}
	
	void SetBodyCount( const S32 bodyCount )
	{
		m_BodyWeights.Resize(bodyCount);
	}
	
	C_Weights GetBodyWeight( const S32 index ) const
	{
		BIZ_ASSERT(index < m_BodyWeights.m_numElements);
		return m_BodyWeights[index];
	}
	
	void SetBodyWeight( const S32 index, C_Weights& weight )
	{
		BIZ_ASSERT(index < m_BodyWeights.m_numElements);
		m_BodyWeights[index] = weight;
	}

	void SetBodyWeight(C_Weights& weight )
	{
		m_BodyWeights.AddToEnd(&weight);
	}
	
	const S32 GetConstraintCount() const
	{
		return m_ConstraintWeights.m_numElements;
	}
	
	void SetConstraintCount( const S32 constraintCount );
	
	F32 GetConstraintWeight( const S32 index ) const
	{
		BIZ_ASSERT(index < m_ConstraintWeights.m_numElements);
		return m_ConstraintWeights[index];
	}
	
	void SetConstraintWeight( const S32 index, const F32 weight )
	{
		m_ConstraintWeights[index] = weight;
	}

	void SetConstraintWeight( F32 weight )
	{
		m_ConstraintWeights.AddToEnd(&weight);
	}

	
private:
	
	// How much each body's velocity need to be change to reach its target pose in world space
	SharedPhysics::SortedArray<C_Weights> m_BodyWeights;

    // How much each constraint is trying to match the relative positions of it's bodies in the target pose.
	SharedPhysics::SortedArray<F32> m_ConstraintWeights;
};

class C_RagdollController : public SharedPhysics::C_BlockItem<C_RagdollController>
{
public:
	friend class C_Scene;
	C_RagdollController(const Maths::HF32 PCREF expectedTimeStep) : m_Index(m_sIndex++), m_pFrameOfReference(NULL), m_Active(false), m_RaytraceOnly(true),
		m_RecipExpectedTimeStep(Maths::Reciprocal(expectedTimeStep)), m_ReciprocalFrameOfReferenceTime(Maths::Constants<Maths::HF32>::Zero()),
		m_RemainingReferenceTime(Maths::Constants<Maths::HF32>::Zero())
	{
	}
	// Sets up the bodies and constraints this ragdoll controller handles
	void Initialise( const C_RagdollConfig& ragdollConfig )
	{
		//DebugBreak();
		m_RagdollConfig = ragdollConfig;
		
	}
	const C_RagdollConfig& GetConfig() const
	{
		return m_RagdollConfig;
	}

	void SetScene(Gravity::C_Scene* pScene)
	{
		m_pScene = pScene;
	}

	void MakeRaytraceOnly(bool bRaytraceOnly);

	void Stop();

	void Start();

	void RecordCurrentFrameOfReferencePose();

	void Update(const Maths::HF32 PCREF time);

	void MakeAnimated();
	void MakeAnimated(const U32 bodyIndex);
	void MakeActive();
	void MakeActive(const U32 bodyIndex);

	void Synchronise();

	void SetFrameOfReferenceBody(Gravity::C_Body* pBody)
	{
		m_pFrameOfReference = pBody;
	}

	// Directly sets the position and orientation of the ragdoll bodies
	void SetQuaternionPose( const C_RagdollQuaternionPose& pose);

	// Returns the current position and orientation of the ragdoll bodies
	C_RagdollQuaternionPose GetQuaternionPose() const;

	//void SetRelativePose(const C_RagdollRelativePose& pose);

	const C_RagdollRelativePose& GetRelativePose() const;

	/*C_RagdollQuaternionPose& GetInitialQuaternionPose()
	{
		return m_InitialQuaternionPose;
	}*/

	/*C_RagdollRelativePose& GetInitialRelativePose()
	{
		return m_InitialRelativePose;
	}*/

	// Sets the world space target pose the driven ragdoll is trying to get to.
	void SetTargetQuaternionPose( const C_RagdollQuaternionPose& targetPose, const Maths::HF32 PCREF time );

	void SetTargetFrameOfReferencePose( const C_RagdollRelativePose& targetPose, const Maths::HF32 PCREF time );

	void SetTargetRelativePose(const C_RagdollRelativePose& targetRelativePose, const Maths::HF32 PCREF time);

	void FollowFrameOfReference(const Maths::HF32 PCREF time, C_Body** pBodies);

	const C_RagdollQuaternionPose& GetTargetQuaternionPose() const;

	void DebugRender(DbUtil::I_DebugRender* pRender);

	// Sets how strongly each body is trying to reach the target pose.
	void SetTargetWeights( const C_RagdollWeights& targetWeights );

	C_RagdollWeights& GetTargetWeights();

	bool operator < (const C_RagdollController& controller)
	{
		return m_Index < controller.m_Index;
	}
	bool operator <= (const C_RagdollController& controller)
	{
		return m_Index <= controller.m_Index;
	}
	bool operator > (const C_RagdollController& controller)
	{
		return m_Index > controller.m_Index;
	}
	bool operator >= (const C_RagdollController& controller)
	{
		return m_Index >= controller.m_Index;
	}
	bool operator == (const C_RagdollController& controller)
	{
		return m_Index == controller.m_Index;
	}
	bool operator != (const C_RagdollController& controller)
	{
		return m_Index != controller.m_Index;
	}

 

private:
	void Activate();
	void Deactivate();

	C_RagdollWeights m_Weights;
	C_RagdollConfig m_RagdollConfig;
	C_RagdollQuaternionPose m_TargetQuaternionPose;
	C_RagdollRelativePose m_TargetRelativePose;

	Maths::HF32 m_ReciprocalFrameOfReferenceTime;
	Maths::HF32 m_RemainingReferenceTime;
	Maths::HF32 m_RecipExpectedTimeStep;
	C_RagdollRelativePose m_TargetFrameOfReferencePose;
	C_RagdollRelativePose m_InitialFrameOfReferencePose;

	C_Body* m_pFrameOfReference;
	C_Scene* m_pScene;
	C_RagdollController* m_pPrev;
	U32 m_Index;
	bool m_Active;
	bool m_RaytraceOnly;
	static U32 m_sIndex;
};

} // namespace Gravity



#endif
