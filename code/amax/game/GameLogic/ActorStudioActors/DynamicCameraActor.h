#ifndef DYNAMICCAMERAACTOR_H
#define DYNAMICCAMERAACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <system/Misc/Random.h>



namespace Cam2
{
	class C_Camera;
}


//=========================================================================
//						DYNAMIC CAMERA ACTOR CLASSES
//=========================================================================



namespace Actors
{

	enum E_BLEND_TYPE
	{
		E_BLEND_TYPE_LINEAR,
		E_BLEND_TYPE_EASEIN,
		E_BLEND_TYPE_EASEOUT,
		E_BLEND_TYPE_EASEBOTH,
		E_BLEND_TYPE_PINGPONG,
	};

	enum E_FADE_TYPE
	{
		E_FADE_TYPE_OUTIN,
		E_FADE_TYPE_OUT,
		E_FADE_TYPE_IN,
	};


	struct DynamicCameraBehaviour						// How the camera will move, what it will look at etc
	{
		bool								m_Initialised;

		XtHandle<Game::DyCamPropertyDef>	m_CameraProperties;
		XtHandle<Game::DyCamShakeDef>		m_CameraShake;

		XtHandle<Game::DyCamMovementDef>	m_MovementBehaviour;
		Actor::C_Actor*						m_ParentActor;
		Maths::HVec3						m_ParentOffset;
		Btl::String32						m_ParentElementName;
		Actor::C_Actor*						m_LocalParentSpline;
		F32									m_MovementSpeed;

		XtHandle<Game::DyCamTargetDef>		m_TargetBehaviour;
		Actor::C_Actor*						m_TargetActor;
		Maths::HVec3						m_TargetOffset;
		Btl::String32						m_TargetElementName;
		Actor::C_Actor*						m_LocalTargetSpline;
		F32									m_TrackingSpeed;

		F32									m_PitchAngle;
		F32									m_RollAngle;
		Btl::String32						m_FOVAttributeName;
		Btl::String32						m_DOFAttributeName;
		Btl::String32						m_ApertureAttributeName;
		Btl::String32						m_CutflagAttributeName;

		Btl::String32						m_DOFNearBlurBeginName;
		Btl::String32						m_DOFNearBlurEndName;
		Btl::String32						m_DOFFarBlurBeginName;
		Btl::String32						m_DOFFarBlurEndName;
		Btl::String32						m_DOFFarBlurScaleName;


		// Working vals
		S32									m_ParentElementID;
		S32									m_TargetElementID;
		S32									m_ParentFOVID;		// Field of View
		S32									m_ParentDOFID;		// Depth of Field
		S32									m_ParentApertureID;	// Aperture

		S32									m_CameraCutID;		// Parameter signalling a camera cut from within an animation
		bool								m_CameraCutState;	// True if parameter is set to on

		S32									m_DOFNearBlurBeginID;
		S32									m_DOFNearBlurEndID;
		S32									m_DOFFarBlurBeginID;
		S32									m_DOFFarBlurEndID;
		S32									m_DOFFarBlurScaleID;

		F32									m_MovementSplineProgression;
		F32									m_TargetSplineProgression;

		F32									m_AverageSpeed;
		F32									m_AverageAccel;

		F32									m_TiltScale;					// Accelerations might be invalid if just changed the camera

		bool								m_UseSimplePosition;
		Maths::HVec3						m_Position;
		Maths::HVec3						m_PitchHeadingRole;
		F32									m_FOV;
	};


	struct DynamicCameraState							// Info fed directly into mercury camera system
	{
		Maths::HMat43			worldmat;				// Just position, orientation and fov for now
		Maths::HMat43			previousmat;			// Previous mat before tilts

		F32						m_FOV;

		Game::E_BLUR_TYPE		m_BlurType;

		F32						m_DepthOfField;
		F32						m_Aperture;

		F32						m_Blur;

		F32						m_DOFNearBlurBegin;
		F32						m_DOFNearBlurEnd;
		F32						m_DOFFarBlurBegin;
		F32						m_DOFFarBlurEnd;
		F32						m_DOFFarBlurScale;

		F32						m_Colour;
		F32						m_Grain;
		F32						m_Vignette;
		F32						m_Tunnel;

		F32						m_RedSaturation;
		F32						m_GreenSaturation;
		F32						m_BlueSaturation;

		F32						m_ShakeX;
		F32						m_ShakeY;
		F32						m_ShakeZ;
		F32						m_ShakeFreq;
	};


//=========================================================================

	
class DynamicCameraActorCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_DynamicCameraActor;}

	bool								m_GetAllMessages;
	S32									m_Viewport;

	XtHandle<Game::RightStickCameraParams>	m_RightStickParams;
};

//=========================================================================


class DynamicCameraActor : public Game::Actors::C_GameActor
{
public:
	DynamicCameraActor();
	virtual ~DynamicCameraActor();

	static const Actor::C_ActorTypeId			TypeId()		{ return Game::E_ActorTypeId_DynamicCameraActor; }
	const DynamicCameraActorCreationData*		GetCreationData() const {return static_cast<const DynamicCameraActorCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&				GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex				GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	void										CalcFadeValue();
	Actor::C_Actor*								GetActorPtrFromCreate(Actors::C_GameActorCreate* createdata);
	void										ProcessCameraBehaviour(DynamicCameraBehaviour *behaviour, DynamicCameraState *state);
	void										SetDefaultCameraProperties(DynamicCameraBehaviour *behaviour, DynamicCameraState *state);
	void										GetWorldMat(Maths::HMat43& matrix, Actor::C_Actor* actor);
	void										AddLocalOffsetToMatrix(Maths::HMat43 *mat, Maths::HVec3 *offset, Actor::C_Actor* actor, S32 elementID, bool include_rotation);
	void										AddTargetShake(Maths::HMat43 *mat, DynamicCameraBehaviour *behaviour, F32 dist);
	void										PointMatAtMat(Maths::HMat43 *pointmat, Maths::HMat43 *targetmat);
	void										CheckLineOfSight(Maths::HMat43 *pointmat, Maths::HVec3 *targetpos);
	void										LagMatrix(Maths::HMat43 *mat, Maths::HMat43 *oldmat, F32 lag);
	F32											LerpF32(F32 start, F32 end, F32 lerp);
	void										GetRandomDistVec(Maths::HVec3 *vec, F32 dist);
	F32											GetRandomDist();	
	S32											GetElementIDFromActor(const Actor::C_Actor* pActor, const CHAR8* elementName) const;
	Maths::HMat43								GetElementMatrixFromActor(const Actor::C_Actor* pActor, S32 elementId) const;
	S32											GetCustomScalarIDFromActor(const Actor::C_Actor* pActor, const CHAR8* scalarName) const;
	F32											GetCustomScalarFromActor(const Actor::C_Actor* pActor, S32 scalarId) const;
	void										ModifyWithRightStick();


	const Cam2::C_Camera* 						GetCamera() const { return m_Camera; }


	DynamicCameraState							old_state;					// State we're lerping from
	DynamicCameraState							blend_state;				// Transition state if blending
	DynamicCameraState							target_state;				// State we're trying to get to

	DynamicCameraBehaviour						old_behaviour;				// Used to calc old_state
	DynamicCameraBehaviour						target_behaviour;			// Used to calc target_state

	bool										blending;					// True if blending between old and target state
	bool										jump;						// True to forget about lag etc and move camera immediately
	bool										disable_blur;				// True to forget about motion blur for one frame
	F32											start_blend_time;			// When we started the blend
	F32											blend_duration;
	E_BLEND_TYPE								blend_type;					// Ease in etc
	
	F32											deltatime;					// Frame time in seconds

	F32											fade_start_time;			// When fade started
	F32											fade_duration;				// How long to carry out total fade out then in
	E_FADE_TYPE									fade_type;					// Are we fading out, in or both
	F32											fade_val;					// Current fade value


protected:
	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();


private:
	Maths::HMat43								m_WorldMatrix;

	Cam2::C_Camera* 							m_Camera;

	Util::C_Random								m_Random;
	S32											m_shakePos;

	bool										m_Disabled;

	F32											m_inputx;					// Blended input values
	F32											m_inputy;
	F32											m_input_zoom;
};

}// End namespace Actors


#endif // DYNAMICCAMERAACTOR_H
