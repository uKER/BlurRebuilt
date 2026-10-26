#ifndef DRIVERANIMATIONPARAMETERS_H
#define DRIVERANIMATIONPARAMETERS_H

namespace AnimationTree
{
	class C_Tree;
}

namespace Game
{
	class C_AnimationAction
	{
	public:
		C_AnimationAction();
		void Bind(AnimationTree::C_Tree& tree, const CHAR8* name);
		void Request(F32 lifetime = 0.f);
		void Cancel();
		void Update(AnimationTree::C_Tree& tree, F32 deltaTime);
		
		bool IsCompleted() const		{ return m_completed; }
		bool IsRequested() const		{ return m_requested; }
		F32 GetRequestLifetime() const	{ return m_requestLifetime; }

	private:
		S32	m_index;
		F32 m_requestLifetime;
		bool m_requested;
		bool m_completed;
	};
	
	enum E_DriverAction
	{
		E_DriverAction_Celebrate = 0,
		E_DriverAction_ShowFrustration,
		E_DriverAction_LookLeft,
		E_DriverAction_LookRight,
		E_DriverAction_GlanceOutWindowLeft,
		E_DriverAction_GlanceOutWindowRight,
		E_DriverAction_Jolt,
		E_DriverAction_OvertakeOnLeft,
		E_DriverAction_OvertakeOnRight,
		E_DriverAction_PassedOnLeft,
		E_DriverAction_PassedOnRight,
		E_DriverAction_ActivatedPickup,
		E_DriverAction_PlayCutSceneAnim,
		E_DriverAction_ChangeGear,
		E_DriverAction_EngageHandbrake,
		E_DriverAction_ReleaseHandbrake,
		E_DriverAction_Count
	};

	enum E_DriverParam
	{
		E_DriverParam_Steering	 = 0,
		E_DriverParam_SpeedMph,
		E_DriverParam_CollisionImpulse,
		E_DriverParam_RandomSeed,
		E_DriverParam_VehicleIndex,
		E_DriverParam_GForceX,
		E_DriverParam_GForceY,
		E_DriverParam_GForceZ,
		E_DriverParam_JoltX,
		E_DriverParam_JoltZ,
		E_DriverParam_PredictedTurn,
		E_DriverParam_Roll,					// +1 = up, -1 = down
		E_DriverParam_Airbourne,			// 0 = on ground, +1 = in air
		E_DriverParam_GravityDirX,
		E_DriverParam_GravityDirZ,

		E_DriverParam_Count
	};

	/**
	Holds the state of a driver's animation parameters.
	*/
	struct C_DriverAnimationParameters
	{
		F32					parameters[E_DriverParam_Count];
		S8					parameterIndices[E_DriverParam_Count];
		C_AnimationAction	actions[E_DriverAction_Count];
		
		/// Adds all parameters to specified animation tree.
		void Bind(AnimationTree::C_Tree& tree);

		/// Cancels all pending actions.
		void CancelActions(AnimationTree::C_Tree& tree);

		/// Updates animation tree with current values of all parameters.
		void Update(AnimationTree::C_Tree& tree, F32 deltaTime);
	};
	
} // end Game

#endif
