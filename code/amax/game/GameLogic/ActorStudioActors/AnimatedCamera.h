#ifndef ANIMATEDCAMERA_H
#define ANIMATEDCAMERA_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>
#include <GameLogic/ActorStudioActors/ResourceTypes.h>
#include <Game/GameUtil/ResourceBagListener.h>
#include <animation/pose/Pose.h>
#include <animation/cycle/CycleInstance.h>


#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>


namespace AmaxCam { class C_DirectorActorCreate; }
namespace Animation		{ class C_CycleResource; }
namespace Cam2			{ class C_Camera; }

struct RLGenericCameraSequences;




namespace Actors
{

	class AnimatedCameraSequence
	{
	public:
		XtHandle<ResourceUtils::C_XtCycleResourceDef>		m_Animation;
		// Ui params
		XtF32												m_UiStageTime;
		XtVec2												m_UiTextPosition0;
		XtVec2												m_UiTextPosition1;
		XtVec2												m_UiLbPosition;
		XtVec2												m_UiLbSize;
		XtSz8												m_UiLbAnimName;
		bool												m_UiLbAnimForwards;
		bool												m_HideFocusVehicle;
		bool												m_HideOpponentVehicles;
	};



	enum E_ANIMCAMERASEQ_TYPE
	{
		E_ANIMCAMERASEQ_ROUTEVISTA = 0,
		E_ANIMCAMERASEQ_ROUTEPACK,

		E_ANIMCAMERASEQ_RACESTART4,
		E_ANIMCAMERASEQ_RACESTART3,
		E_ANIMCAMERASEQ_RACESTART2,
		E_ANIMCAMERASEQ_RACESTART1,
		E_ANIMCAMERASEQ_RACESTARTGO,
		E_ANIMCAMERASEQ_MPWINNER,
		E_ANIMCAMERASEQ_MMWINNER,

		E_ANIMCAMERASEQ_RIVALSEQ01,
		E_ANIMCAMERASEQ_RIVALSEQ02,
		E_ANIMCAMERASEQ_RIVALSEQ03,
		E_ANIMCAMERASEQ_RIVALSEQ04,
		E_ANIMCAMERASEQ_RIVALSEQ05,
		E_ANIMCAMERASEQ_RIVALSEQ06,
		E_ANIMCAMERASEQ_RIVALSEQ07,
		E_ANIMCAMERASEQ_RIVALSEQ08,
		E_ANIMCAMERASEQ_RIVALSEQ09,
		E_ANIMCAMERASEQ_RIVALSEQ10,

		E_ANIMCAMERASEQ_MPSPECTATE1,
		E_ANIMCAMERASEQ_MPSPECTATE2,
		E_ANIMCAMERASEQ_MPSPECTATE3,
		E_ANIMCAMERASEQ_MPSPECTATE4,
		E_ANIMCAMERASEQ_MPSPECTATE5,
		E_ANIMCAMERASEQ_MPSPECTATE_END,

		E_NUM_ANIMCAMERASEQS=E_ANIMCAMERASEQ_MPSPECTATE_END,
	};


//=========================================================================
//						ANIMATED CAMERA ACTOR CLASSES
//=========================================================================


class AnimatedCameraCreationData : public C_PosActorCreate
{
public:
	// Standard stuff
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_AnimatedCamera;}

	const AmaxCam::C_DirectorActorCreate*			m_Director;
	S32												m_Viewport;
	bool											m_FutureFeed;
};

//=========================================================================


enum E_ANIMCAMERA_MODE
{
	E_ANIMCAMERA_MODE_NORMAL,						// Animated about world, vehicle or old camera origin
	E_ANIMCAMERA_MODE_BLENDTORACECAM,				// Blends camera into race camera
	E_ANIMCAMERA_MODE_AUTOSELECT,					// Chooses the best camera position from a list of offsets
};



class AnimatedCamera : public Game::Actors::C_GameActor, public C_ResourceBagListener
{
public:
	AnimatedCamera();
	virtual ~AnimatedCamera();

	static const Actor::C_ActorTypeId			TypeId()		{ return Game::E_ActorTypeId_AnimatedCamera; }
	const AnimatedCameraCreationData*			GetCreationData() const {return static_cast<const AnimatedCameraCreationData*>(C_GameActor::GetCreationData().GetData());}
	virtual const Maths::HMat43&				GetMatrix(void) const {return m_WorldMatrix;}
	virtual const Maths::C_Vertex				GetPosition(void) const{return m_WorldMatrix.AW();}
	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	const Cam2::C_Camera* 						GetCamera() const { return m_Camera; }

protected:
	virtual bool								InitialiseImpl();
	virtual bool								PrimeImpl();
	virtual bool								CreateImpl();

	virtual void OnResourceBagLoaded(const C_ResourceBag& bag);

//	void										RequestAnimation(XtHandle<ResourceUtils::C_XtCycleResourceDef> anim);
//	bool										AddAnimation(Animation::C_PoseFormat *poseFormat, Animation::C_CycleResource** ptr, XtHandle<ResourceUtils::C_XtCycleResourceDef> anim);
	void										RequestAnimation(XtHandle<Actors::AnimatedCameraSequence> anim);
	bool										AddAnimation(Animation::C_PoseFormat *poseFormat, Animation::C_CycleResource** ptr, XtHandle<Actors::AnimatedCameraSequence> anim);

private:
	void										UpdateCamera(F32 deltatime);
	void										StartAnimation(S32 vehicleindex);
	bool										ValidCameraPosition(S32 vehicleindex, Maths::HVec3 localpos);
	bool										CheckLineOfSight(Maths::HVec3* camerapos, Maths::HMat43* vehiclemat, F32 xoffset, F32 yoffset, F32 zoffset);

	void										HandleVehicleVisibility(S32 p_Sequence, S32 p_FocusVehicleIndex);

	const bool									IsSilentRunningActiveAndRunningOnThisVehicle(const S32 vehicleIndex);

	AmaxRoute*									m_pRoute;
	RLGenericCameraSequences*					m_pGenericSequences;

	Cam2::C_Camera* 							m_Camera;
	Maths::HMat43								m_WorldMatrix;
	bool										m_Active;						// True is playing an animation
	F32											m_AnimTime;					// How far into the animation
	S32											m_CarIndex;					// Index of vehicle if moving relative
	C_ResourceBagHandle							m_pResourceBag;

	Animation::C_CycleResource*					m_pVistaAnimation;
	Animation::C_CycleResource*					m_pPackAnimation;
	Animation::C_CycleResource*					m_pGenericAnimation[E_NUM_ANIMCAMERASEQS];
	bool										m_GenericAnimationLooped[E_NUM_ANIMCAMERASEQS];
	Animation::C_Pose							m_Pose;
	Animation::C_CycleInstance					m_CurrentAnimation;
	bool										m_Looped;

	bool										m_Paused;

	S32											m_CameraJointIndex;
	S32											m_CameraFovIndex;
	F32											m_CameraFOV;

	E_ANIMCAMERA_MODE							m_CameraMode;
	
//	bool										m_BlendToRaceCam;
	F32											m_BlendDuration;
	F32											m_BlendTime;

	Maths::HVec3								m_VehicleOffset;

	Maths::HMat43								m_LocalOrigin;							// Used for freezing camera at end of race
	F32											m_LocalFOV;

	S32											m_ViewportIndex;
	bool										m_FutureFeed;
	bool										m_AnimFinishedSent;
};

}// End namespace Actors


#endif // ANIMATEDCAMERA_H
