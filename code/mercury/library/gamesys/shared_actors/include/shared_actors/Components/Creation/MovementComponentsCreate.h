#ifndef MERCURY_MOVEMENT_COMPONENTS_CREATE_H
#define MERCURY_MOVEMENT_COMPONENTS_CREATE_H

#include <shared_actors/Components/SharedComponents.h>


namespace Actors
{
	class C_GameActorCreate;
	class C_PosActorCreate;
	class C_SplineActorCreate;
}


namespace Components
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// Movement component creation info.
//
class C_MovementCreateInfo : public I_GameComponentCreator
{
public:
	C_MovementCreateInfo();
	virtual ~C_MovementCreateInfo();

public:
	virtual bool IsParented() const;
	virtual Actor::C_ActorHandle ParentHandle() const;

public:
	const Actors::C_GameActorCreate* m_pParentActorCreate;
};


class C_MovementAttacherCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAttacher;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAttacher;}

	F32 m_DopplerSpeed;
};

// Dynamic create.
class C_MovementAttacherDynamicCreateInfo : public C_MovementAttacherCreateInfo
{
public:
	bool IsParented() const {return U32(m_ParentHandle)!=0;}
	Actor::C_ActorHandle ParentHandle() const {return m_ParentHandle;}
public:
	Actor::C_ActorHandle m_ParentHandle;
};


class C_MovementTranslatorCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementTranslator;}
};

class C_MovementRotatorCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementRotator;}
};


class C_MovementRotTransCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementRotTrans;}
};


class C_MovementSpinnerCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementSpinner;}

public:
	F32 m_InitialRPM;
};


class C_MovementWheelCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementWheel;}

public:
	F32 m_Radius;
};


class C_MovementConstrainerCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementConstrainer;}

public:
	bool m_ConstrainTransX;
	bool m_ConstrainPitch;
};

class C_MovementAnimationSlaveCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAnimationSlave;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAnimationSlave;}
};

class C_MovementAimerCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAimer;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAimer;}

public:
	const Actors::C_PosActorCreate*	m_pTargetActor;
	F32	m_TrackingSpeed;
	bool m_LimitAngle;
	F32 m_LimitAngleAmount;
};

class C_MovementSplineCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementSpline;}

public:
	const Actors::C_SplineActorCreate* m_pSplineActorCreate;
	F32		m_MoveSpeed;
	F32		m_Rotation;
	bool	m_Loop;						// Repositions actor at the start of the spline when it's reached the end
	bool	m_StartImmediately;			// Don't wait for activate, just start
	bool	m_StartFromNearestPoint;	// Snap actor to nearest spline position and move from that offset.
};

class C_MovementKinematicSplineCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementKinematicSpline;}

public:
	const Actors::C_SplineActorCreate* m_pSplineActorCreate;
	F32		m_StartSpeed;
	F32		m_MinSpeed;
	F32		m_MaxSpeed;
	F32		m_Acceleration;
	bool	m_BrakeAtEnd;
	bool	m_Loop;						// Repositions actor at the start of the spline when it's reached the end
	bool	m_StartImmediately;			// Don't wait for activate, just start
	bool	m_StartFromNearestPoint;	// Snap actor to nearest spline position and move from that offset.
};

class C_MovementKinematicSplineTrackerCreateInfo : public C_MovementKinematicSplineCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementKinematicSplineTracker;}
};

class C_MovementKinematicSplineSlaveCreateInfo : public C_MovementCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementKinematicSplineSlave;}

public:
	const Actors::C_SplineActorCreate* m_pSplineActorCreate;
	const Actors::C_PosActorCreate*	m_pSlaveMaster;
	bool m_LastSlave;
	
};

class C_MovementRelativeToBoneCreateInfo: public C_MovementCreateInfo
{
public:
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_MovementRelativeToBone; }
	
public:
	XtSz8		m_BoneName;				// Name of bone or element.
	XtVec3		m_TranslationOffset;	// Translation in bone space.
	XtVec3		m_RotationOffset;		// Euler XYZ rotation in degrees in bone space.
	bool		m_IgnoreBoneRotation;	// If true, actor retains its rotation but still follows bone position.
};

class C_MovementAimSplineCreateInfo : public C_MovementSplineCreateInfo
{
public: // From C_MovementCreateInfo
	S32 GetComponentTypeID() const {return Components::E_MovementAimSpline;}

public:
	const Actors::C_PosActorCreate*	m_pTargetActor;
	F32								m_TrackingSpeed;
};

class C_MovementBobbingBoatCreateInfo : public C_MovementCreateInfo
{
public:
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_MovementBobbingBoat; }
	F32 m_MaxPitchScale;
	F32 m_PitchTimerOffset;
	F32 m_PitchTimerScale;
	F32 m_MaxRollScale;
	F32 m_RollTimerOffset;
	F32 m_RollTimerScale;
};




} // namespace Components

#endif
