#ifndef ANIMATION_WALKCONTROLLER_H
#define ANIMATION_WALKCONTROLLER_H

#include <animation/common/BlendModes.h>

namespace DbUtil
{
	class I_DebugRender;
}

namespace Animation
{
	static const U32	kMaxWalkControllerLegs	= 4;
	
	class C_IkChain;

	class I_GroundCollision
	{
	public:
		virtual bool TraceRay(const Maths::Point3& start, const Maths::Point3& end, Maths::Point3* outContactPoint, Maths::Normal3* outContactNormal)=0;
	};
	
	class C_GroundPlaneCollision
	{
	public:
	
		void	SetGroundPlane();
	
	private:
		Maths::Plane	m_groundPlane;
	};

	/**
	For gait cycles.
	
	Step
	Support
	
	SupportTime/GaitTime = DutyFactor
	
	Gait period = duration of cycle.
	
	StepData
		F32	stepTrigger;
		F32 dutyFactor;
		
	GaitData
		U32 numLegs;
		F32 gaitPeriod;
		StepData[] legData;
	*/

	class C_StepInfo
	{
	public:
		F32	m_stepTrigger;		// Phase at which foot enters step phase. (i.e. foot up)
		F32	m_dutyFactor;		// Amount of time foot spends in support phase relative to gait period.
	};
	
	class C_Cycle;
	class C_Skeleton;
	
	// Created from a cycle.
	//	Either attempt to generate info by looking at foot positions or use scalar curves.
	// Speeding up or slowing down a cycle... 
	class C_GaitInfo
	{
	public:
		C_GaitInfo();
		
		void CreateFromCycle(const S16* footJointIndices, U32 numLegs, const C_Skeleton& skeleton, const C_Cycle& cycle);
		
		void Clear();
		
		// Blending.
		// Incorporate modification of gait duration.
		void BlendInto(C_GaitInfo& dest, Animation::E_BlendMode blendMode, F32 weight) const;
		void Normalise();
		
		F32 GetPeriod() const;
		
		const C_StepInfo* GetStepInfo(U32 legIndex) const
		{
			return legIndex < m_numLegs ? &m_stepInfo[legIndex] : NULL;
		}
		
	private:
		U32			m_numLegs;
		F32			m_gaitPeriod;		// Duration.
		C_StepInfo	m_stepInfo[kMaxWalkControllerLegs];
		
		F32			m_totalWeight;		// Accumulated weight.
	};
	

	class C_StepController
	{
	public:
		C_StepController();
		
		void Reset();
		
		void SetCurrentFootPosition(const Maths::Point3& pos);
	
		void Update(
			F32 lastGaitPhase,
			F32 gaitPhase,
			F32 gaitPeriod,
			F32 smoothing,
			const Maths::Vector3& rootVelocity,
			const Maths::Mat43& modelWorldMatrix,
			bool onGround,
			I_GroundCollision* groundCollision
		);
		
		F32 GetGroundHeight() const;
		F32 GetHeightDelta() const;
		Maths::Normal3 GetGroundNormal() const;
		Maths::Plane GetGroundPlane() const;
		
		#ifndef CFG_GOLD
		void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix) const;
		#endif
		
		Maths::Point3		m_currentFootPos;		// Current foot position, usually from the last frame.
		Maths::Point3		m_futureFootPos;
		Maths::Normal3		m_futureGroundNormal;
		Maths::Normal3		m_groundNormal;			// Smoothed ground normal.
		Maths::Normal3		m_groundContactNormal;	// Normal at ground point below current foot position.
		Maths::Point3		m_groundContactPoint;	// Point at ground point below current foot position.
		
		//Maths::Plane		m_groundPlane;
		const C_StepInfo*	m_stepInfo;
		F32					m_stepPhase;
		F32					m_prevGroundHeight;
		F32					m_futureGroundHeight;
		F32					m_groundHeight;
		F32					m_heightDelta;
		bool				m_stepping;
		bool				m_currentFootPosValid;
		bool				m_onGround;
	};

	class C_Pose;
	class C_Skeleton;

	/**
	Experimental walk IK controller.
	
	Ignore gait information for the time being.
	*/
	class C_WalkController
	{
	public:
		C_WalkController();
	
		/// Removes all legs and resets the controller.
		void Clear();
	
		/// Adds a leg. Returns false if maximum number of legs was exceeded.
		bool AddLeg(C_IkChain& ikChain, U32 ikTargetIndex, S32 footJointIndex, F32 footToGroundHeight = 0.f);
		
		/// Returns number of legs being controlled.
		U32 GetNumLegs() const;
		
		/// Resets state typically if the character has not been updated in the previous frame or has teleported.
		void Reset();
		
		/// Updates foot positions from a recent pose.
		void UpdateFootPositions(const C_Pose& pose, const C_Skeleton* skeleton);
					
		/// Updates the walk controller, setting IK targets on its IK chains and calculating a hip offset.
		void Update(
			F32 deltaTime,
			const Maths::Vector3& rootVelocity,
			const Maths::Mat43& modelWorldMatrix,
			const C_GaitInfo* gaitInfo,
			F32 speed,
			I_GroundCollision* groundCollision,
			bool onGround,
			F32 ikWeight
		);
		
		/// Returns the hip offset calculated during Update().
		Maths::Vector3	GetHipOffset() const;
		
		#ifndef CFG_GOLD
		void DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix) const;
		#endif
	
	private:
		C_StepController	m_stepControllers[kMaxWalkControllerLegs];
		C_IkChain*			m_legIkChains[kMaxWalkControllerLegs];
		Maths::Vector3		m_hipOffset;
		U8					m_legIkTargetIndices[kMaxWalkControllerLegs];
		U32					m_numLegs;
		F32					m_hipHeightSmoothing;
		F32					m_groundHeightSmoothing;
		F32					m_footToGroundHeights[kMaxWalkControllerLegs];
		F32					m_minLegLength;
		S16					m_hipJointIndex;
		S16					m_footJointIndices[kMaxWalkControllerLegs];	// for ray traces.
			
		F32					m_hipHeightDelta;
		F32					m_gaitPhase;
		
		bool				m_allowHipMovement;
	};

}


#endif
