#ifndef SHOWROOMCAMERA_H
#define SHOWROOMCAMERA_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>



namespace Cam2	{ class C_Camera; }

namespace Input	{ class C_GameInput; }



namespace Actors
{


class ShowRoomCameraParams
{
public:
	XtF32	m_TiltDefault;
	XtF32	m_TiltScale;
	XtF32	m_TiltMin;
	XtF32	m_TiltMax;

	XtF32	m_RotDefault;
	XtF32	m_RotScale;
	XtF32	m_RotMin;
	XtF32	m_RotMax;

	XtF32	m_XOffset;
	XtF32	m_YOffset;
	XtF32	m_RangeScaler;
	XtF32	m_RangeModifier;
	XtF32	m_RangeModifierZoomed;
	XtF32	m_HeightModifier;

	XtF32	m_GroundOffset;

	XtF32	m_Blending;
	XtF32	m_BlendingSpinTilt;

	XtF32	m_FOVScale;
	XtF32	m_DOFBegin;
	XtF32	m_DOFEnd;
	XtF32	m_DOFScale;

	XtF32	m_AutospinNullZone;
	XtF32	m_AutospinDelay;
	XtF32	m_AutospinSpeed;
	XtF32	m_AutospinTilt;
	XtF32	m_AutospinRoll;
};


//=========================================================================
//						SHOWROOM CAMERA ACTOR CLASSES
//=========================================================================


class ShowRoomCameraCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_ShowRoomCamera;}

	XtHandle<ShowRoomCameraParams>		m_CameraParams;

//	S32									m_Viewport;
};

//=========================================================================


class ShowRoomCamera : public Game::Actors::C_GameActor
{
public:
	ShowRoomCamera();
	virtual ~ShowRoomCamera();

	static const Actor::C_ActorTypeId			TypeId()		{ return Game::E_ActorTypeId_ShowRoomCamera; }
	const ShowRoomCameraCreationData*			GetCreationData() const {return static_cast<const ShowRoomCameraCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&				GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex				GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	const Cam2::C_Camera* 						GetCamera() const { return m_Camera; }

	static ShowRoomCamera*						GetInstance()		{ return m_pInstance; }

protected:
	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();
	virtual void								KillImpl();


private:
	static ShowRoomCamera*						m_pInstance;

	void										CheckAutoSpin();
	void										SetAutoTargetRotation();
	void										SetManualTargetRotation();
	void										UpdateVehicleParams();
	void										UpdateWorldPosition();
	void										PointMatAtTarget(Maths::HMat43* pointmat, Maths::HVec3* target);
	F32											Blend(F32 current_val, F32 target_val, F32 blend);
	void										UpdateCamera();


	Maths::HMat43								m_WorldMatrix;
	Maths::HMat43								m_RotateOrigin;

	Cam2::C_Camera* 							m_Camera;

	Input::C_GameInput*							m_GameInput;

	F32											m_deltatime;					// Frame time in seconds
	F32											m_rotation;						// Rotation about Y axis
	F32											m_tilt;							// Rotation about X axis

	F32											m_range;						// Range to origin					// Temp
	F32											m_height_offset;				// Centre of car from floor

	F32											m_spintime;						// Timer used for starting autospin

	F32											m_inputx;						// Controller inputs
	F32											m_inputy;
	bool										m_input_zoom;

	bool										m_spin;
	F32											m_spin_rotation;

	const ShowRoomCameraParams*					m_params;						// Global showroom camera parameters

	F32											m_blended_tilt;					// Other blended values
	F32											m_blended_roll;
	F32											m_blended_range;
	F32											m_blended_height;

	F32											m_target_rotation;
	F32											m_target_tilt;

};

}// End namespace Actors


#endif // SHOWROOMCAMERA_H
