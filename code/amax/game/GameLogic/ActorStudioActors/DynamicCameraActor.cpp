#include "stdafx.h"

#include <Game\Game.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <camera2/camera.h>
#include <camera2/EffectInfo.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameApp.h>

#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

#include <gravity/narrowphase/PrimitiveTest.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>
#include <Shared_Actors/ActorImpl/SplineActor.h>
#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>
#include <Shared_Actors/Components/Interfaces/I_AnimatedObjectComponents.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/pose/Pose.h>
#include <animation/task/EvaluationState.h>
#include <animation/common/JointTransform.h>
#include <animation/common/JointTransform.inl>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>
#include <Graphics/PostFXSettings.h>

#include <Game/AmaxCam/VehicleFeed.h>

#include <animation/common/StringId.h>

#include "DynamicCameraActor.h"

#include <GameLogic/Structures/LevelState.h>

#include <SystemProfile/SystemProfileManager.h>
#include <GameLogic/Profile/GameProfileManager.h>
#include "Game/GameControls/GameControls.h"



//OPTIMISE_OFF
//#pragma optimize("", off)


Debug::Bool g_debugDisableDynamicCameraFOVMatching("DynamicObjects.DisableDynamicCameraFOVMatching", false);


//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)

XT_TYPE(Actors::DynamicCameraActorCreationData,		"DynamicCameraActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_GetAllMessages,						"ReceiveAllCameraMessages")
	XT_FIELD(m_Viewport,							"Viewport")
	XT_FIELD(m_RightStickParams,					"RightStickParams")
XT_END_TYPE


XT_TYPE(Actors::E_BLEND_TYPE,					"BlendType")
	XT_OPTION(Actors::E_BLEND_TYPE_LINEAR,		"Linear")
	XT_OPTION(Actors::E_BLEND_TYPE_EASEIN,		"EaseIn")
	XT_OPTION(Actors::E_BLEND_TYPE_EASEOUT,		"EaseOut")
	XT_OPTION(Actors::E_BLEND_TYPE_EASEBOTH,	"EaseBoth")
	XT_OPTION(Actors::E_BLEND_TYPE_PINGPONG,	"PingPong")
XT_END_TYPE

XT_TYPE(Actors::E_FADE_TYPE,					"FadeType")
	XT_OPTION(Actors::E_FADE_TYPE_OUTIN,		"FadeOutThenIn")
	XT_OPTION(Actors::E_FADE_TYPE_OUT,			"FadeOut")
	XT_OPTION(Actors::E_FADE_TYPE_IN,			"FadeIn")
XT_END_TYPE




//=========================================================================
//					CameraTarget Actor Core Routines
//=========================================================================



namespace Actors
{

using namespace Maths;


//=========================================================================

DynamicCameraActor::DynamicCameraActor()
{
	m_Camera = NULL;
	old_behaviour.m_Initialised = false;
	target_behaviour.m_Initialised = false;
	blending = false;
	start_blend_time = 0.0f;
	blend_duration = 0.0f;
	fade_start_time = 0.0f;
	fade_duration = 0.0f;
	fade_val = -1.0f;
	jump = false;
	disable_blur = true; //false;
	m_Random.Seed( 5489UL );
	m_shakePos = 0;
	m_Disabled = false;
	m_inputx = 0.0f;
	m_inputy = 0.0f;
	m_input_zoom = 0.0f;
}

//=========================================================================

DynamicCameraActor::~DynamicCameraActor()
{
	AmaxCam::CameraSystem().GameFeedRemove(m_Camera);
	SAFE_DELETE_PTR(m_Camera)
}

//=========================================================================

bool DynamicCameraActor::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();
	target_state.worldmat = m_WorldMatrix;					// In case we haven't got a message yet - Start at the spawn point
	old_state.worldmat = m_WorldMatrix;						// Just to be sure

	return(true);
}


//=========================================================================

bool DynamicCameraActor::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	m_Camera = new Cam2::C_Camera;
	if (!m_Camera)
		return false;
			
	m_Camera->SetName("Dynamic");
	m_Camera->SetPosition(m_WorldMatrix.AW());
	m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
	m_Camera->GetPrevPosition();

	AddSubscriber( ActorMsg::E_DynamicObjMsg_CameraUpdate );				// Update cameras after everything else
	if (GetCreationData()->m_GetAllMessages)									// Some dynamic cameras will pickup all camera messages, some won't
		AddSubscriber( ActorMsg::E_DynamicObjMsg_ChangeCameraBehaviour );
	AddSubscriber(ActorMsg::E_DynamicObjMsg_CameraCut);
	AddSubscriber(ActorMsg::E_DynamicObjMsg_SorceryUpdate);

	old_state.previousmat = m_WorldMatrix;							// Make sure fly cams move from created positions
	blend_state.previousmat = m_WorldMatrix;
	target_state.previousmat = m_WorldMatrix;

	return(true);
}


//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool DynamicCameraActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	F32					lerpval;
	Maths::VF32			castval;
	
	
	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_ChangeCameraBehaviour:		// ======= Change Camera Behaviour =======
		{
			const ActorMsg::C_ChangeCameraBehaviourMsg* msg = Messaging::MessageCast<ActorMsg::C_ChangeCameraBehaviourMsg>(pMessage);

			m_Disabled = false;

			if (msg->m_BlendTime > 0.0f)
			{
				memcpy(&old_behaviour, &target_behaviour, sizeof(DynamicCameraBehaviour));			// Keep processing the current behaviour (now held in old) so we can blend from it
				memcpy(&old_state, &target_state, sizeof(DynamicCameraState));

				blending = true;
				start_blend_time = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				blend_duration = msg->m_BlendTime;
				blend_type = msg->m_Blending;
			}
			else
			{
				blending = false;
				old_behaviour.m_Initialised = false;
				disable_blur = true;
			}

			if (msg->m_CameraProperties)
			{
				target_behaviour.m_CameraProperties		= msg->m_CameraProperties;
			}

			target_behaviour.m_CameraShake				= msg->m_CameraShake;

			if (msg->m_MovementBehaviour)
			{
				target_behaviour.m_MovementBehaviour	= msg->m_MovementBehaviour;
				target_behaviour.m_ParentActor			= GetActorPtrFromCreate(msg->m_ParentActor);
				target_behaviour.m_ParentOffset			= Maths::HVec3((float)msg->m_ParentOffset.vx, (float)msg->m_ParentOffset.vy, (float)msg->m_ParentOffset.vz);
//				target_behaviour.m_PreDefParentOffset	= msg->m_PreDefParentOffset;
				target_behaviour.m_LocalParentSpline	= GetActorPtrFromCreate(msg->m_LocalParentSpline);
				target_behaviour.m_MovementSpeed		= msg->m_MovementSpeed;

				target_behaviour.m_ParentElementName	= msg->m_ParentElementName;
				target_behaviour.m_FOVAttributeName		= msg->m_FOVAttributeName;
				target_behaviour.m_DOFAttributeName		= msg->m_DOFAttributeName;
				target_behaviour.m_ApertureAttributeName= msg->m_ApertureAttributeName;
				target_behaviour.m_CutflagAttributeName	= msg->m_CutflagAttributeName;

				target_behaviour.m_DOFNearBlurBeginName	= msg->m_DOFNearBlurBeginName;
				target_behaviour.m_DOFNearBlurEndName	= msg->m_DOFNearBlurEndName;
				target_behaviour.m_DOFFarBlurBeginName	= msg->m_DOFFarBlurBeginName;
				target_behaviour.m_DOFFarBlurEndName	= msg->m_DOFFarBlurEndName;
				target_behaviour.m_DOFFarBlurScaleName	= msg->m_DOFFarBlurScaleName;

				if (!target_behaviour.m_ParentElementName.Empty() && target_behaviour.m_ParentActor)
				{
					target_behaviour.m_ParentElementID	= GetElementIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_ParentElementName.Chars());
					target_behaviour.m_ParentFOVID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_FOVAttributeName.Chars());
					target_behaviour.m_ParentDOFID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFAttributeName.Chars());
					target_behaviour.m_ParentApertureID	= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_ApertureAttributeName.Chars());
//					target_behaviour.m_CameraCutID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, "cutflag");
					target_behaviour.m_CameraCutID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_CutflagAttributeName.Chars());

					target_behaviour.m_DOFNearBlurBeginID	= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFNearBlurBeginName.Chars());
					target_behaviour.m_DOFNearBlurEndID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFNearBlurEndName.Chars());
					target_behaviour.m_DOFFarBlurBeginID	= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFFarBlurBeginName.Chars());
					target_behaviour.m_DOFFarBlurEndID		= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFFarBlurEndName.Chars());
					target_behaviour.m_DOFFarBlurScaleID	= GetCustomScalarIDFromActor(target_behaviour.m_ParentActor, target_behaviour.m_DOFFarBlurScaleName.Chars());
				}
				else
				{
					target_behaviour.m_ParentElementID	= -1;					// No element offset
					target_behaviour.m_ParentFOVID		= -1;
					target_behaviour.m_ParentDOFID		= -1;
					target_behaviour.m_ParentApertureID	= -1;
					target_behaviour.m_CameraCutID		= -1;

					target_behaviour.m_DOFNearBlurBeginID	= -1;
					target_behaviour.m_DOFNearBlurEndID		= -1;
					target_behaviour.m_DOFFarBlurBeginID	= -1;
					target_behaviour.m_DOFFarBlurEndID		= -1;
					target_behaviour.m_DOFFarBlurScaleID	= -1;
				}

				target_behaviour.m_MovementSplineProgression = 0.0f;			// Initialise any misc behaviour vars
			}

			if (msg->m_TargetBehaviour)
			{
				target_behaviour.m_TargetBehaviour		= msg->m_TargetBehaviour;
				target_behaviour.m_TargetActor			= GetActorPtrFromCreate(msg->m_TargetActor);
				target_behaviour.m_TargetOffset			= Maths::HVec3((float)msg->m_TargetOffset.vx, (float)msg->m_TargetOffset.vy, (float)msg->m_TargetOffset.vz);
//				target_behaviour.m_PreDefTargetOffset	= msg->m_PreDefTargetOffset;
				target_behaviour.m_LocalTargetSpline	= GetActorPtrFromCreate(msg->m_LocalTargetSpline);
				target_behaviour.m_TrackingSpeed		= msg->m_TrackingSpeed;

				target_behaviour.m_TargetElementName	= msg->m_TargetElementName;

				if (!target_behaviour.m_TargetElementName.Empty() && target_behaviour.m_TargetActor)
					target_behaviour.m_TargetElementID	= GetElementIDFromActor(target_behaviour.m_TargetActor, target_behaviour.m_TargetElementName.Chars());
				else
					target_behaviour.m_TargetElementID	= -1;					// No element offset

				target_behaviour.m_TargetSplineProgression = 0.0f;
			}


			target_behaviour.m_TiltScale			= 0.0f;

			target_behaviour.m_PitchAngle			= msg->m_PitchAngle;
			target_behaviour.m_RollAngle			= msg->m_RollAngle;

			target_behaviour.m_AverageSpeed			= 0.0f;
			target_behaviour.m_AverageAccel			= 0.0f;

			target_behaviour.m_CameraCutState		= false;

			target_behaviour.m_Initialised			= true;
			target_behaviour.m_UseSimplePosition	= false;

			jump = true;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_SimpleCameraBehaviour:
		{
			const ActorMsg::C_SimpleCameraBehaviourMsg* msg = Messaging::MessageCast<ActorMsg::C_SimpleCameraBehaviourMsg>(pMessage);

			m_Disabled = false;

			if (msg->m_BlendTime > 0.0f)
			{
				memcpy(&old_behaviour, &target_behaviour, sizeof(DynamicCameraBehaviour));			// Keep processing the current behaviour (now held in old) so we can blend from it
				memcpy(&old_state, &target_state, sizeof(DynamicCameraState));

				blending = true;
				start_blend_time = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
				blend_duration = msg->m_BlendTime;
				blend_type = msg->m_Blending;
			}
			else
			{
				blending = false;
				old_behaviour.m_Initialised = false;
				disable_blur = true;
			}

			if (msg->m_CameraProperties)
			{
				target_behaviour.m_CameraProperties		= msg->m_CameraProperties;
			}
			target_behaviour.m_CameraShake				= msg->m_CameraShake;


			target_behaviour.m_ParentActor			= GetActorPtrFromCreate(msg->m_ParentActor);
			target_behaviour.m_Position				= Maths::HVec3((float)msg->m_Position.vx, (float)msg->m_Position.vy, (float)msg->m_Position.vz);
			target_behaviour.m_PitchHeadingRole		= Maths::HVec3((float)Maths::DegToRad(msg->m_PitchHeadingRole.vx), (float)Maths::DegToRad(msg->m_PitchHeadingRole.vy), (float)Maths::DegToRad(msg->m_PitchHeadingRole.vz));

			target_behaviour.m_FOV					= msg->m_FOV;

			target_behaviour.m_TiltScale			= 0.0f;
			target_behaviour.m_AverageSpeed			= 0.0f;
			target_behaviour.m_AverageAccel			= 0.0f;

			target_behaviour.m_CameraCutState		= false;

			target_behaviour.m_Initialised			= true;
			target_behaviour.m_UseSimplePosition	= true;

			jump = true;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_FreezeCameraState:
		{
			m_Disabled = false;
			blending = false;
			disable_blur = true;

			old_behaviour.m_Initialised = false;								// Don't process any logic if frozen
			target_behaviour.m_Initialised = false;

			memcpy(&target_state, &blend_state, sizeof(DynamicCameraState));	// And store the current state so we can blend from it
			memcpy(&old_state, &blend_state, sizeof(DynamicCameraState));

			break;
		}

		case ActorMsg::E_DynamicObjMsg_FadeDynamicCamera:			// ======= Make it fade in and out =======
		{
			const ActorMsg::C_FadeDynamicCameraMsg* msg = Messaging::MessageCast<ActorMsg::C_FadeDynamicCameraMsg>(pMessage);
			fade_type = msg->m_FadeType;
			fade_duration = msg->m_TotalDuration;
			if (fade_duration != 0.0f)
			{
				fade_start_time = C_GameApp::GetInstance()->GetTimer().GetElapsedTime();
			}
			else
			{
				switch(fade_type)							// Apply immediately if duration is zero
				{
					case E_FADE_TYPE_OUT:
						C_ActorUtils::Instance().gameworld->SetGlobalFade(0.0f);
						break;

					case E_FADE_TYPE_OUTIN:
					case E_FADE_TYPE_IN:
						C_ActorUtils::Instance().gameworld->SetGlobalFade(1.0f);
						break;
				}
			}
				

			break;
		}

		case ActorMsg::E_DynamicObjMsg_SorceryUpdate:
		case ActorMsg::E_DynamicObjMsg_CameraUpdate:				// ======= Camera Update =======
		{
			if (m_Disabled)
				break;

			deltatime = Messaging::MessageCast<ActorMsg::C_CameraUpdateMsg>(pMessage)->GetValue();
//			LOGINFO( Resource::g_Log, "Delta Time = %f\n", deltatime);


			ProcessCameraBehaviour(&old_behaviour, &old_state);
			ProcessCameraBehaviour(&target_behaviour, &target_state);

			jump = false;				// Processed one frame since camera has been re-configured
			
			if (!blending)
			{
				blend_state.worldmat		= target_state.worldmat;				// No lerping yet

				blend_state.m_FOV			= target_state.m_FOV;

				blend_state.m_BlurType		= target_state.m_BlurType;

				blend_state.m_DepthOfField	= target_state.m_DepthOfField;
				blend_state.m_Aperture		= target_state.m_Aperture;

				blend_state.m_Blur			= target_state.m_Blur;

				blend_state.m_DOFNearBlurBegin	= target_state.m_DOFNearBlurBegin;
				blend_state.m_DOFNearBlurEnd	= target_state.m_DOFNearBlurEnd;
				blend_state.m_DOFFarBlurBegin	= target_state.m_DOFFarBlurBegin;
				blend_state.m_DOFFarBlurEnd		= target_state.m_DOFFarBlurEnd;
				blend_state.m_DOFFarBlurScale	= target_state.m_DOFFarBlurScale;

				blend_state.m_Colour		= target_state.m_Colour;
				blend_state.m_Grain			= target_state.m_Grain;
				blend_state.m_Vignette		= target_state.m_Vignette;
				blend_state.m_Tunnel		= target_state.m_Tunnel;

				blend_state.m_RedSaturation		= target_state.m_RedSaturation;
				blend_state.m_GreenSaturation	= target_state.m_GreenSaturation;
				blend_state.m_BlueSaturation	= target_state.m_BlueSaturation;

				blend_state.m_ShakeX		= target_state.m_ShakeX;
				blend_state.m_ShakeY		= target_state.m_ShakeY;
				blend_state.m_ShakeZ		= target_state.m_ShakeZ;
				blend_state.m_ShakeFreq		= target_state.m_ShakeFreq;
			}
			else
			{
				lerpval = (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - start_blend_time) / blend_duration;
				if ((lerpval >= 1.0f) && (blend_type != E_BLEND_TYPE_PINGPONG))
				{
					blending = false;
					lerpval = 1.0f;
				}

				switch(blend_type)									// Modify lerpval to account for accelerations (based on simple sin wave)
				{
					case E_BLEND_TYPE_LINEAR:
						// No change
						break;
				
					case E_BLEND_TYPE_EASEIN:
						lerpval = 1.0f - Maths::Cos(lerpval * (Maths::Pi/2.0f));
						break;
				
					case E_BLEND_TYPE_EASEOUT:
						lerpval = Maths::Sin(lerpval * (Maths::Pi/2.0f));
						break;
				
					case E_BLEND_TYPE_EASEBOTH:
						lerpval = 0.5f - (Maths::Cos(lerpval * Maths::Pi)/2.0f);
						break;

					case E_BLEND_TYPE_PINGPONG:
						lerpval = fmod(lerpval, 2.0f);
						lerpval = 0.5f - (Maths::Cos(lerpval * Maths::Pi)/2.0f);	// Pingpong with ease in and out
						break;
				}


				castval = Load<VF32>(&lerpval);
				QuatTrans	oldqtran( MakeQuatTrans(old_state.worldmat));
				QuatTrans	targqtran(MakeQuatTrans(target_state.worldmat));
//				QuatTrans	slerpqtran = Slerp( oldqtran, targqtran, castval);
				QuatTrans	slerpqtran = NearestSlerp( oldqtran, targqtran, castval);
				blend_state.worldmat = MakeMat43( slerpqtran );

				blend_state.m_FOV			= LerpF32(old_state.m_FOV, target_state.m_FOV, lerpval);

				blend_state.m_BlurType		= target_state.m_BlurType;			// Can't blend blur type

				blend_state.m_DepthOfField	= LerpF32(old_state.m_DepthOfField, target_state.m_DepthOfField, lerpval);
				blend_state.m_Aperture		= LerpF32(old_state.m_Aperture, target_state.m_Aperture, lerpval);

				blend_state.m_Blur			= LerpF32(old_state.m_Blur, target_state.m_Blur, lerpval);

				blend_state.m_DOFNearBlurBegin			= LerpF32(old_state.m_DOFNearBlurBegin, target_state.m_DOFNearBlurBegin, lerpval);
				blend_state.m_DOFNearBlurEnd			= LerpF32(old_state.m_DOFNearBlurEnd, target_state.m_DOFNearBlurEnd, lerpval);
				blend_state.m_DOFFarBlurBegin			= LerpF32(old_state.m_DOFFarBlurBegin, target_state.m_DOFFarBlurBegin, lerpval);
				blend_state.m_DOFFarBlurEnd				= LerpF32(old_state.m_DOFFarBlurEnd, target_state.m_DOFFarBlurEnd, lerpval);
				blend_state.m_DOFFarBlurScale			= LerpF32(old_state.m_DOFFarBlurScale, target_state.m_DOFFarBlurScale, lerpval);

				blend_state.m_Colour		= LerpF32(old_state.m_Colour, target_state.m_Colour, lerpval);
				blend_state.m_Grain			= LerpF32(old_state.m_Grain, target_state.m_Grain, lerpval);
				blend_state.m_Vignette		= LerpF32(old_state.m_Vignette, target_state.m_Vignette, lerpval);
				blend_state.m_Tunnel		= LerpF32(old_state.m_Tunnel, target_state.m_Tunnel, lerpval);

				blend_state.m_RedSaturation		= LerpF32(old_state.m_RedSaturation, target_state.m_RedSaturation, lerpval);
				blend_state.m_GreenSaturation	= LerpF32(old_state.m_GreenSaturation, target_state.m_GreenSaturation, lerpval);
				blend_state.m_BlueSaturation	= LerpF32(old_state.m_BlueSaturation, target_state.m_BlueSaturation, lerpval);

				blend_state.m_ShakeX		= LerpF32(old_state.m_ShakeX, target_state.m_ShakeX, lerpval);
				blend_state.m_ShakeY		= LerpF32(old_state.m_ShakeY, target_state.m_ShakeY, lerpval);
				blend_state.m_ShakeZ		= LerpF32(old_state.m_ShakeZ, target_state.m_ShakeZ, lerpval);
				blend_state.m_ShakeFreq		= LerpF32(old_state.m_ShakeFreq, target_state.m_ShakeFreq, lerpval);
			}


			m_WorldMatrix = blend_state.worldmat;

			if (GetCreationData()->m_RightStickParams)				// Modify world mat if we've got right stick control
				ModifyWithRightStick();

			m_Camera->StartUpdate(deltatime);
			m_Camera->SetPosition(m_WorldMatrix.AW());
			m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
			m_Camera->SetShakeXYZ(blend_state.m_ShakeX, blend_state.m_ShakeY, blend_state.m_ShakeZ, blend_state.m_ShakeFreq);
			m_Camera->EndUpdate();

			if (GetCreationData()->m_RightStickParams)				// Reset world mat back to original if we have moved it using right stick
				m_WorldMatrix = blend_state.worldmat;


			if (target_behaviour.m_Initialised)				// Only update camera settings if we've received a camera message already
			{
				if (blend_state.m_FOV > 0.0f)
				{
					F32 fov = DegToRad(blend_state.m_FOV);
					fov *= AmaxCam::CameraSystem().FovScale();
					m_Camera->SetFov(fov);
				}

				// If we are using DOF, set the motion blur to zero so that the camera system selects the DOF shader:
				m_Camera->SetShutterAngle(blend_state.m_Blur);

				// If Dynamic Camera Actor is setting depth of field information, set aperture to -1
				// This tells C_CameraSystem to set the Depth of Field shader, but not to set parameters:
				F32 aperture = 0.f;
				if (blend_state.m_BlurType == Game::E_BLUR_TYPE_DEPTHOFFIELD)
					aperture = -1.f;
				m_Camera->SetAperture(aperture);

				S32 viewportId = GetCreationData()->m_Viewport;

				FX::I_EnvironmentSettings* pEnvironmentSettings = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetEnvironmentSettings(viewportId);
				if(pEnvironmentSettings)
				{
					Game::PostFxEnvSettings* pPostFXEnvSettings = (Game::PostFxEnvSettings*)pEnvironmentSettings->GetSettingsBlockByName("PostFX");

					if (pPostFXEnvSettings)
					{
						CalcFadeValue();

						const F32 fR = Maths::Clamp(blend_state.m_RedSaturation, -1.0f, 1.0f) + 1.0f;		// Work out saturation values
						const F32 fG = Maths::Clamp(blend_state.m_GreenSaturation, -1.0f, 1.0f) + 1.0f; 
						const F32 fB = Maths::Clamp(blend_state.m_BlueSaturation, -1.0f, 1.0f) + 1.0f; 
						const F32 fLR = 0.2125f;
						const F32 fLG = 0.7154f;
						const F32 fLB = 0.0721f;

						MathsStorage::MatrixRow44 m =
						{
							(1.0f - fR) * fLR + fR, (1.0f - fR) * fLR, (1.0f - fR) * fLR, 0.0f,
							(1.0f - fG) * fLG, (1.0f - fG) * fLG + fG, (1.0f - fG) * fLG, 0.0f,
							(1.0f - fB) * fLB, (1.0f - fB) * fLB, (1.0f - fB) * fLB + fB, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f
						};

						Maths::Mat44 saturationMatrix = Maths::Load<Maths::Mat44>(&m);

						pPostFXEnvSettings->m_ColourMatrix = pPostFXEnvSettings->m_ColourMatrix * saturationMatrix * Maths::MakeVF32(blend_state.m_Colour);
						pPostFXEnvSettings->m_FilmGrainIntensity = pPostFXEnvSettings->m_FilmGrainIntensity + blend_state.m_Grain;
						pPostFXEnvSettings->m_VignetteWeight = pPostFXEnvSettings->m_VignetteWeight + blend_state.m_Vignette;
						pPostFXEnvSettings->m_TunnelAmount = pPostFXEnvSettings->m_TunnelAmount + blend_state.m_Tunnel;
					}
				}

				if (blend_state.m_DOFNearBlurEnd == blend_state.m_DOFNearBlurBegin)			// Stop Full blur when both are zero
					blend_state.m_DOFNearBlurEnd += 0.01f;

				Cam2::C_EffectInfo effect;
				effect.SetInfo(blend_state.m_DOFNearBlurBegin, blend_state.m_DOFNearBlurEnd, blend_state.m_DOFFarBlurBegin, blend_state.m_DOFFarBlurEnd, blend_state.m_DOFFarBlurScale);
				m_Camera->SetEffectInfo(effect);

				if (fade_val != -1.0f)
					C_ActorUtils::Instance().gameworld->SetGlobalFade(fade_val);
			}

			break;
		}

		case ActorMsg::E_DynamicObjMsg_CameraCut:
		{
			disable_blur = true;
			break;
		}

		case ActorMsg::E_DynamicObjMsg_ResetDynamicCamera:				// Stop processing camera (so no longer accessing animation data etc)
		{
			m_Disabled = true;
			break;
		}
	
		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}


//=========================================================================


void DynamicCameraActor::CalcFadeValue()
{
	F32		normtime;

	if (fade_start_time == 0.0f)
	{
		fade_val = -1.0f;			// w 1.0f			Disable fade control
		return;
	}

	normtime = (C_GameApp::GetInstance()->GetTimer().GetElapsedTime() - fade_start_time) / fade_duration;

	if (normtime >= 1.0f)
	{
		normtime = 1.0f;					// Process last frame then switch off
		fade_start_time = 0.0f;
	}

	switch(fade_type)
	{
		case E_FADE_TYPE_OUTIN:
			if (normtime <= 0.5f)
				fade_val = 1.0f - (normtime * 2.0f);
			else
				fade_val = ((normtime - 0.5f) * 2.0f);
			break;

		case E_FADE_TYPE_OUT:
			fade_val = 1.0f - normtime;
			break;

		case E_FADE_TYPE_IN:
			fade_val = normtime;
			break;
	}

}

//=========================================================================

Actor::C_Actor* DynamicCameraActor::GetActorPtrFromCreate(Actors::C_GameActorCreate* createdata)
{
	Actor::C_ActorHandle		handle;

	if (!createdata)
		return(NULL);

	handle = createdata->GetActorHandle();
	if (!handle)
		return(NULL);

	return( (Actor::C_Actor*)GetActorList()->GetActorPtr(handle) );
}

//=========================================================================


void DynamicCameraActor::ProcessCameraBehaviour(DynamicCameraBehaviour *behaviour, DynamicCameraState *state)
{
	Actors::C_SplineActor*	pSplineActor;
	Maths::HMat43			mat;
	Maths::C_QuatTrans		localtransform;
	Maths::HVec3			tvec;
	F32						length, newprogression;
	F32						dist(0.f), speed, lag;
	F32						shake;
//	Maths::HMat43			oldworldmat(state->worldmat);
	F32						oldspeed;
	F32						xtilt = 0.0f, ztilt = 0.0f;


	if (!behaviour->m_Initialised)
		return;


	if (behaviour->m_UseSimplePosition)		// Used for world/relative camera/orientation positioning (mainly for matching Dave's ingame camera positions)
	{
		SetDefaultCameraProperties(behaviour, state);
		if (behaviour->m_ParentActor)
		{
			GetWorldMat(state->worldmat, behaviour->m_ParentActor);
			AddLocalOffsetToMatrix(&state->worldmat, &behaviour->m_Position, behaviour->m_ParentActor, -1, true);

			Maths::HMat33   localrot(Maths::C_MatrixRotatePHR(behaviour->m_PitchHeadingRole));
			localrot = localrot * state->worldmat.Rotation();

/*
			F32	pitch = behaviour->m_PitchHeadingRole.X();
			F32	yaw = behaviour->m_PitchHeadingRole.Y();
			F32	roll = behaviour->m_PitchHeadingRole.Z();
			F32 c1 = cos(yaw  *-0.5f); F32 s1 = sin(yaw  *-0.5f);
			F32 c2 = cos(pitch* 0.5f); F32 s2 = sin(pitch* 0.5f);
			F32 c3 = cos(roll * 0.5f); F32 s3 = sin(roll * 0.5f);
			Maths::C_Quaternion qy(0.f, s1 , 0.f, c1);
			Maths::C_Quaternion qx(s2 , 0.f, 0.f, c2);
			Maths::C_Quaternion qz(0.f, 0.f, s3 , c3);
			Maths::C_Quaternion quat(qy * qx * qz);
			Maths::HMat33 localrot(quat);
*/

			state->worldmat.Rotation(localrot);

		}
		else
		{
			state->worldmat.Rotation(Maths::HMat33(Maths::C_MatrixRotatePHR(behaviour->m_PitchHeadingRole)));
			state->worldmat.Translation(behaviour->m_Position);
		}

		state->m_FOV = behaviour->m_FOV;
		return;
	}

	switch(behaviour->m_MovementBehaviour->m_MovementType)		// -------- Work out where we are --------
	{
		case Game::E_DYCAM_MOVEMENT_NONE:					// Just uses current position (although this could be blended)
			state->worldmat = m_WorldMatrix;
			break;

		case Game::E_DYCAM_MOVEMENT_ATTACHED:				// Positions camera relative to another actor
			if (behaviour->m_ParentActor)
			{
				GetWorldMat(state->worldmat, behaviour->m_ParentActor);
				AddLocalOffsetToMatrix(&state->worldmat, &behaviour->m_ParentOffset, behaviour->m_ParentActor, behaviour->m_ParentElementID, true);
			}
			break;

		case Game::E_DYCAM_MOVEMENT_FLYCAM:					// Will fly towards a point
			if (behaviour->m_ParentActor)
			{
				GetWorldMat(mat, behaviour->m_ParentActor);
				AddLocalOffsetToMatrix(&mat, &behaviour->m_ParentOffset, behaviour->m_ParentActor, behaviour->m_ParentElementID, true);

				state->worldmat = state->previousmat;
				tvec = mat.Translation() - state->previousmat.Translation();		// TODO - Move this into lag function
				dist = tvec.Length();
				speed = behaviour->m_MovementSpeed * deltatime;
				lag = behaviour->m_MovementBehaviour->m_Lag;
				if (lag == 0.0f)													// Use a min lag of 0.1
					lag = 0.1f;
				if (dist < (behaviour->m_MovementSpeed*lag))						// Decelleration
					speed *= Maths::Sin((dist/(behaviour->m_MovementSpeed*lag)) * (Maths::Pi/2.0f));

				PointMatAtMat(&state->worldmat, &mat);
				state->worldmat.Translation( state->worldmat.Translation() + (state->worldmat.AZ() * speed));
			}
			break;


		case Game::E_DYCAM_MOVEMENT_SPLINEFOLLOWER:			// Moves along a spline in world space
		{
			pSplineActor = Actor::ActorCast_Safe<Actors::C_SplineActor>(behaviour->m_ParentActor);
			if (!pSplineActor)
				return;

			length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();		// How far along it?
			newprogression = behaviour->m_MovementSplineProgression + (behaviour->m_MovementSpeed * deltatime);
			if (newprogression > length)
			{
				if (behaviour->m_MovementBehaviour->m_DoNotLoop)
					newprogression = length;
				else
					newprogression -= length;
			}

			behaviour->m_MovementSplineProgression = pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(localtransform, newprogression);
			state->worldmat.Translation(localtransform.Translation());						// Get new position
			state->worldmat.Rotation(localtransform.m_Quaternion);

			AddLocalOffsetToMatrix(&state->worldmat, &behaviour->m_ParentOffset, behaviour->m_ParentActor, behaviour->m_ParentElementID, false);
			break;
		}

		case Game::E_DYCAM_MOVEMENT_LOCALSPLINEFOLLOWER:	// Moves along a spline relative to a parent actor
		{
			pSplineActor = Actor::ActorCast_Safe<Actors::C_SplineActor>(behaviour->m_LocalParentSpline);
			if (!pSplineActor)
				return;

			length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();		// How far along it?
			newprogression = behaviour->m_MovementSplineProgression + (behaviour->m_MovementSpeed * deltatime);
			if (newprogression > length)
			{
				if (behaviour->m_MovementBehaviour->m_DoNotLoop)
					newprogression = length;
				else
					newprogression -= length;
			}

			behaviour->m_MovementSplineProgression = pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(localtransform, newprogression);
			state->worldmat.Translation(localtransform.Translation());						// Get new position
			state->worldmat.Rotation(localtransform.m_Quaternion);

			AddLocalOffsetToMatrix(&state->worldmat, &behaviour->m_ParentOffset,behaviour->m_ParentActor, behaviour->m_ParentElementID, false);

			tvec = state->worldmat.Translation() - pSplineActor->GetCreationData()->GetCreationMatrix().Translation();			// Remove spline origin + add in parent origin + rotate relative to parent
			tvec = tvec * behaviour->m_ParentActor->GetMatrix().Rotation();
			tvec += behaviour->m_ParentActor->GetMatrix().Translation();
			state->worldmat.Translation( tvec );
			state->worldmat.Rotation( state->worldmat.Rotation() * behaviour->m_ParentActor->GetMatrix().Rotation() );
			break;
		}

		case Game::E_DYCAM_MOVEMENT_VEHICLEFEED:			// Matches movement of in-game camera
		{
			AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
			state->worldmat = cameraSystem.VehicleFeed(GetCreationData()->m_Viewport)->Mat43();
			break;
		}

		case Game::E_DYCAM_MOVEMENT_LASTCAMERA:			// Matches movement of in-game camera
		{
			if ((jump) && (behaviour == &target_behaviour))			// Copy whatever the last camera position was if we've just got an update message and this is the target behaviour
			{
				AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
				state->worldmat = cameraSystem.GameFeed().Mat43();
			}
			break;
		}

/*
		case Game::E_DYCAM_MOVEMENT_SIMPLECAMERA:		// Used for world/relative camera/orientation positioning (mainly for matching Dave's ingame camera positions)
		{
			break;
		}
*/
	}


	tvec = state->worldmat.Translation() - state->previousmat.Translation();
	if (deltatime != 0.0f)						// Should always have some time but just to make sure
	{
		speed = tvec.Length() / deltatime;
		oldspeed = behaviour->m_AverageSpeed;
		behaviour->m_AverageSpeed += (speed/4.0f) - (behaviour->m_AverageSpeed/4.0f);		// Iron out framerate fluctuations

		F32 accel = behaviour->m_AverageSpeed - oldspeed;
		tvec.Normalise();
		F32	dotprod = tvec | state->previousmat.AZ();				// Get dot product against old direction
		if (dotprod < 0.0f)									// If moving backwards then reverse acceleration so it looks good
			accel = -accel;
		behaviour->m_AverageAccel += (accel/4.0f) - (behaviour->m_AverageAccel/4.0f);		// Iron out framerate fluctuations
	}


	if (behaviour->m_TargetBehaviour)
	{
		switch(behaviour->m_TargetBehaviour->m_TargetType)			// -------- Work out where we're looking --------
		{
			case Game::E_DYCAM_TARGET_NONE:						// Just use whatever the movement type throws out
				break;

			case Game::E_DYCAM_TARGET_SIMPLE:					// Point directly at a point relative to an actor (can be the same as the movement parent actor)
				GetWorldMat(mat, behaviour->m_TargetActor);
				AddLocalOffsetToMatrix(&mat, &behaviour->m_TargetOffset, behaviour->m_TargetActor, behaviour->m_TargetElementID, false);
				tvec = mat.Translation() - state->previousmat.Translation();
				dist = tvec.Length();
//				AddTargetShake(&mat, behaviour, dist);
//				PointMatAtMat(&state->worldmat, &mat);
				break;

			case Game::E_DYCAM_TARGET_SPLINEFOLLOWER:			// Moves target point along a spline in world space
			{
				pSplineActor = Actor::ActorCast_Safe<Actors::C_SplineActor>(behaviour->m_TargetActor);
				if (!pSplineActor)
					return;

				length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();		// How far along it?
				newprogression = behaviour->m_TargetSplineProgression + (behaviour->m_TrackingSpeed * deltatime);
				if (newprogression > length)
				{
					if (behaviour->m_TargetBehaviour->m_DoNotLoop)
						newprogression = length;
					else
						newprogression -= length;
				}

				behaviour->m_TargetSplineProgression = pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(localtransform, newprogression);
				mat.Translation(localtransform.Translation());				// Convert new quat and put into mat
				mat.Rotation(localtransform.m_Quaternion);
				AddLocalOffsetToMatrix(&mat, &behaviour->m_TargetOffset, behaviour->m_TargetActor, behaviour->m_TargetElementID, false);
				tvec = mat.Translation() - state->previousmat.Translation();
				dist = tvec.Length();
//				AddTargetShake(&mat, behaviour, dist);
//				PointMatAtMat(&state->worldmat, &mat);						// Point at it
				break;
			}

			case Game::E_DYCAM_TARGET_LOCALSPLINEFOLLOWER:		// Moves target point along a spline relative to a parent actor
			{
				pSplineActor = Actor::ActorCast_Safe<Actors::C_SplineActor>(behaviour->m_LocalTargetSpline);
				if (!pSplineActor)
					return;

				length = pSplineActor->GetSplineWrapper()->GetCurveEval().GetArcLength();		// How far along it?
				newprogression = behaviour->m_TargetSplineProgression + (behaviour->m_TrackingSpeed * deltatime);
				if (newprogression > length)
				{
					if (behaviour->m_TargetBehaviour->m_DoNotLoop)
						newprogression = length;
					else
						newprogression -= length;
				}

				behaviour->m_TargetSplineProgression = pSplineActor->GetSplineWrapper()->GetForwardFacingSplinePos(localtransform, newprogression);
				mat.Translation(localtransform.Translation());				// Convert new quat and put into mat
				mat.Rotation(localtransform.m_Quaternion);					// Not really used - Mat used just to store position
				AddLocalOffsetToMatrix(&mat, &behaviour->m_TargetOffset, behaviour->m_TargetActor, behaviour->m_TargetElementID, false);

				tvec = mat.Translation() - pSplineActor->GetCreationData()->GetCreationMatrix().Translation();			// Remove spline origin + add in parent origin + rotate relative to parent
				tvec = tvec * behaviour->m_TargetActor->GetMatrix().Rotation();
				tvec += behaviour->m_TargetActor->GetMatrix().Translation();
				mat.Translation( tvec );
				tvec = mat.Translation() - state->previousmat.Translation();
				dist = tvec.Length();
//				AddTargetShake(&mat, behaviour, dist);
//				PointMatAtMat(&state->worldmat, &mat);						// Point at it
				break;
			}
		}

		if (behaviour->m_TargetBehaviour->m_TargetType != Game::E_DYCAM_TARGET_NONE)
		{
			AddTargetShake(&mat, behaviour, dist);
			PointMatAtMat(&state->worldmat, &mat);
			if (behaviour->m_TargetBehaviour->m_CheckLineOfSight)					// Optional target shake
				CheckLineOfSight(&state->worldmat, (Maths::HVec3*)&mat.Translation());
		}

	}


	xtilt += behaviour->m_PitchAngle;										// Perminant pitch?
	ztilt += behaviour->m_RollAngle;										// Perminant roll?

	F32		turntilt = behaviour->m_MovementBehaviour->m_TurnTilt;			// Bank based on turn rate?
	if (turntilt != 0.0f)
	{
		F32		dotprod = state->previousmat.AZ() | state->worldmat.AZ();			// Get dot product
		HF32	angleCos = Maths::ArcCos(dotprod);
		F32		angle = angleCos * turntilt / deltatime;
		if ((state->previousmat.AX() | state->worldmat.AZ()) > 0.0f)				// Check which direction
			angle = -angle;
 		ztilt += angle * behaviour->m_TiltScale;
	}

	F32		dynpitch = behaviour->m_MovementBehaviour->m_DynPitch;		// Pitch based on acceleration?
	if (dynpitch != 0.0f)
		xtilt -= behaviour->m_AverageAccel * dynpitch * behaviour->m_TiltScale;

	if (behaviour->m_TiltScale < 1.0f)
		behaviour->m_TiltScale += 0.1f;

	state->previousmat = state->worldmat;							// Store pre-tilited ready for next frame

	Maths::HMat33	rot(Maths::C_MatrixRotateRPH(ztilt, xtilt, Maths::HF32(0.0f)));
	Maths::HMat43	newmat(rot * state->worldmat.Rotation(), state->worldmat.Translation());	
	state->worldmat	= newmat;

	shake = behaviour->m_MovementBehaviour->m_Shake * speed;		// Camera Shake based on camera speed
	if (shake != 0.0f)
	{
		GetRandomDistVec(&tvec, shake * 0.001f);					// * .001 to get sensible vals
		state->worldmat.Translation( state->worldmat.Translation() + tvec);
	}

	SetDefaultCameraProperties(behaviour, state);

	// Override FOV if pulling in vehicle feed camera data
	if (behaviour->m_MovementBehaviour->m_MovementType == Game::E_DYCAM_MOVEMENT_VEHICLEFEED)
	{
		if (!g_debugDisableDynamicCameraFOVMatching)
		{
			AmaxCam::C_CameraSystem& cameraSystem = AmaxCam::CameraSystem();
			state->m_FOV = Maths::RadToDeg(cameraSystem.VehicleFeed()->Fov());
			
			F32 fovScale = Max(cameraSystem.FovScale(), 0.001f);
			state->m_FOV /= fovScale;
		}
	}

	// Override any camera properties that are stored within the animation
	if (behaviour->m_ParentFOVID != -1.0f)
		state->m_FOV = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_ParentFOVID) * 57.2957795f;		// Convert radians to degrees to fit in with standard camera stuff
	if (behaviour->m_ParentDOFID != -1.0f)
		state->m_DepthOfField = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_ParentDOFID);
	if (behaviour->m_ParentApertureID != -1.0f)
		state->m_Aperture = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_ParentApertureID);

	if (behaviour->m_DOFNearBlurBeginID != -1.0f)
		state->m_DOFNearBlurBegin = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_DOFNearBlurBeginID);
	if (behaviour->m_DOFNearBlurEndID != -1.0f)
		state->m_DOFNearBlurEnd = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_DOFNearBlurEndID);
	if (behaviour->m_DOFFarBlurBeginID != -1.0f)
		state->m_DOFFarBlurBegin = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_DOFFarBlurBeginID);
	if (behaviour->m_DOFFarBlurEndID != -1.0f)
		state->m_DOFFarBlurEnd = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_DOFFarBlurEndID);
	if (behaviour->m_DOFFarBlurScaleID != -1.0f)
		state->m_DOFFarBlurScale = GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_DOFFarBlurScaleID);

	if (behaviour->m_CameraCutID != -1.0f)				// Check for camera cuts
	{
		if (GetCustomScalarFromActor(behaviour->m_ParentActor, behaviour->m_CameraCutID) > 0.5)
		{
			if (!behaviour->m_CameraCutState)							// Broadcast Cut Message
			{
				C_ActorUtils::Instance().gameworld->GetActorList()->GetSubscriberSystem().BroadcastMessage( ActorMsg::E_DynamicObjMsg_CameraCut );
				disable_blur = true;			// Note: This should already be set by the above message being sent back to this actor
			}
			behaviour->m_CameraCutState = true;
		}
		else
		{
			behaviour->m_CameraCutState = false;
		}
	
	}
}


//=========================================================================

void DynamicCameraActor::SetDefaultCameraProperties(DynamicCameraBehaviour *behaviour, DynamicCameraState *state)
{
	state->m_FOV				= behaviour->m_CameraProperties->m_FOV;
	state->m_BlurType			= behaviour->m_CameraProperties->m_BlurType;
	state->m_DepthOfField		= behaviour->m_CameraProperties->m_DepthOfField;
	state->m_Aperture			= behaviour->m_CameraProperties->m_Aperture;
	state->m_Blur				= behaviour->m_CameraProperties->m_Blur;

	state->m_DOFNearBlurBegin	= behaviour->m_CameraProperties->m_DOFNearBlurBegin;
	state->m_DOFNearBlurEnd		= behaviour->m_CameraProperties->m_DOFNearBlurEnd;
	state->m_DOFFarBlurBegin	= behaviour->m_CameraProperties->m_DOFFarBlurBegin;
	state->m_DOFFarBlurEnd		= behaviour->m_CameraProperties->m_DOFFarBlurEnd;
	state->m_DOFFarBlurScale	= behaviour->m_CameraProperties->m_DOFFarBlurScale;

	state->m_Colour				= behaviour->m_CameraProperties->m_Colour;
	state->m_Grain				= behaviour->m_CameraProperties->m_Grain;
	state->m_Vignette			= behaviour->m_CameraProperties->m_Vignette;
	state->m_Tunnel				= behaviour->m_CameraProperties->m_Tunnel;

	state->m_RedSaturation		= behaviour->m_CameraProperties->m_RedSaturation;
	state->m_GreenSaturation	= behaviour->m_CameraProperties->m_GreenSaturation;
	state->m_BlueSaturation		= behaviour->m_CameraProperties->m_BlueSaturation;

	if (behaviour->m_CameraShake)
	{
		state->m_ShakeX = behaviour->m_CameraShake->m_X;
		state->m_ShakeY = behaviour->m_CameraShake->m_Y;
		state->m_ShakeZ = behaviour->m_CameraShake->m_Z;
		state->m_ShakeFreq = behaviour->m_CameraShake->m_Freq;
	}
	else
	{
		state->m_ShakeX = state->m_ShakeY = state->m_ShakeZ = 0.0f;
		state->m_ShakeFreq = 0.0f;
	}
}


//=========================================================================

void DynamicCameraActor::GetWorldMat(Maths::HMat43 & matrix, Actor::C_Actor* actor)
{
	BIZ_ASSERT_MSG(actor, "Dynamic camera trying to look at null actor");
	BIZ_ASSERT_MSG(actor->GetLifetimeState() != ActorSys::E_LifetimeState_Killed, "Dynamic camera trying to look at killed actor");
	if (actor->GetLifetimeState() == ActorSys::E_LifetimeState_Killed)
	{
		matrix.SetIdentity();
		return;
	}

	if (actor->GetActorTypeId() == Actors::E_ActorTypeId_Spline)
		matrix = actor->GetCreationData()->GetCreationMatrix();
	else
		matrix = actor->GetMatrix();
}


//=========================================================================

void DynamicCameraActor::AddLocalOffsetToMatrix(Maths::HMat43 *mat, Maths::HVec3 *offset, Actor::C_Actor* actor, S32 elementID, bool include_rotation)
{
	 Maths::HMat43	localmat;

	if (!actor)
	{
		mat->SetIdentity();
		LOGWARN( Resource::g_Log, "Camera trying to look at null actor");
		return;
	}

	if (elementID != -1)							// Get local mat from model element
	{
		localmat = GetElementMatrixFromActor(actor, elementID);

		if (include_rotation)
		{
			*mat = localmat * (*mat);
		}
		else
		{
			mat->Translation(mat->Translation() + (mat->AX() * localmat.Translation().X()) + (mat->AY() * localmat.Translation().Y()) + (mat->AZ() * localmat.Translation().Z()));
		}
	}

	mat->Translation(mat->Translation() + (mat->AX() * offset->X()) + (mat->AY() * offset->Y()) + (mat->AZ() * offset->Z()));
}

//=========================================================================

void DynamicCameraActor::AddTargetShake(Maths::HMat43 *mat, DynamicCameraBehaviour *behaviour, F32 dist)
{
	Maths::HVec3			tvec;
	F32						shake;

	shake = behaviour->m_TargetBehaviour->m_Shake;					// Optional target shake
	if (dist > 0.0f)
		shake += behaviour->m_TargetBehaviour->m_ProximityShake / dist;
	if (shake != 0.0f)
	{
		GetRandomDistVec(&tvec, shake);
		mat->Translation( mat->Translation() + tvec);
	}
}

//=========================================================================

void DynamicCameraActor::PointMatAtMat(Maths::HMat43 *pointmat, Maths::HMat43 *targetmat)
{
	Maths::HVec3 toz = targetmat->Translation() - pointmat->Translation();
	toz.Normalise();
	Maths::HVec3 tox = Maths::HVec3(0.f,1.f,0.f) ^ toz;
	tox.Normalise();
	Maths::HVec3 toy = toz ^ tox;
	toy.Normalise();

	pointmat->AX(tox);
	pointmat->AY(toy);
	pointmat->AZ(toz);
}

//=========================================================================

void DynamicCameraActor::CheckLineOfSight(Maths::HMat43 *pointmat, Maths::HVec3 *targetpos)
{
	Maths::HVec3	collision;
	
	if (C_ActorUtils::Instance().CheckCollision(targetpos, (Maths::HVec3*)&pointmat->Translation(), &collision))
		pointmat->Translation(collision);
}

//=========================================================================

void DynamicCameraActor::LagMatrix(Maths::HMat43 *mat, Maths::HMat43 *oldmat, F32 lag)
{
	mat->Translation((mat->Translation() * (Maths::HF32)(1.0f - lag)) + (oldmat->Translation() * (Maths::HF32)lag));
}

//=========================================================================

F32 DynamicCameraActor::LerpF32(F32 start, F32 end, F32 lerp)
{
	return(start + ((end - start) * lerp));
}

//=========================================================================

void DynamicCameraActor::GetRandomDistVec(Maths::HVec3 *vec, F32 dist)
{
	vec->X(GetRandomDist() * dist);
	vec->Y(GetRandomDist() * dist);
	vec->Z(GetRandomDist() * dist);
}


//=========================================================================

F32 DynamicCameraActor::GetRandomDist()
{
	F32	num = Maths::Cos(m_Random.FRand() * Maths::Pi2) / 2.0f;
	if (num < 0.0f)
		num += 1.0f;
//	LOGINFO( Resource::g_Log, "Random Dist = %f\n", (float)num);
	return(num);
}


//=========================================================================
//						Actor Element stuff
//=========================================================================


S32	DynamicCameraActor::GetElementIDFromActor(const Actor::C_Actor* pActor, const CHAR8* elementName) const
{
	if (NULL == pActor)
		return -1;
	
	// If the actor has an animation state component then search for a joint transform matching elementName.
	const Components::I_AnimationStateComponent* pAnimationState = NULL;
	if (pActor->QueryInterface(&pAnimationState))
	{
		S32 jointIndex = pAnimationState->GetPose().FindJointTransformByName(elementName);
		if (jointIndex < 0)
			LOGWARN(Actor::GetDebugLogLevelDesign(this), "Could not find bone '%s' in animated actor '%s'", elementName, pActor->GetActorName());
		return jointIndex;
	}
	else
	{
		// ... element transforms from objects with models etc.	
	}
		
	return -1;
}

//=========================================================================

Maths::HMat43 DynamicCameraActor::GetElementMatrixFromActor(const Actor::C_Actor* pActor, S32 elementId) const
{
	Maths::HMat43 matrix;
	if (NULL == pActor)
		return matrix;
	
	// If the actor has an animation state component then extract the corresponding joint transform matrix.
	const Components::I_AnimationStateComponent* pAnimationState = NULL;
	if (pActor->QueryInterface(&pAnimationState))
	{
		if (elementId >= 0)
			matrix = pAnimationState->GetPose().GetModelJointTransform(elementId, pAnimationState->GetSkeleton()).AsMat43();
	}
	else
	{
		// ... element transforms from objects with models etc.	
	}
		
	return matrix;
}

//=========================================================================

S32	DynamicCameraActor::GetCustomScalarIDFromActor(const Actor::C_Actor* pActor, const CHAR8* scalarName) const
{
	if (NULL == pActor)
		return -1;
	
	// If the actor has an animation state component then search for a joint transform matching elementName.
	const Components::I_AnimationStateComponent* pAnimationState = NULL;
	if (pActor->QueryInterface(&pAnimationState))
	{
		const Animation::C_StringId tempstr(scalarName, false);
		S32 scalarIndex = pAnimationState->GetPose().FindCustomScalarByName(tempstr);
//		S32 scalarIndex = pAnimationState->GetPose().FindCustomScalarByName(scalarName);
		if (scalarIndex < 0)
			LOGWARN(Actor::GetDebugLogLevelDesign(this), "Could not find custom scalarbone '%s' in animated actor '%s'", scalarName, pActor->GetActorName());
		return scalarIndex;
	}
		
	return -1;
}

//=========================================================================

F32 DynamicCameraActor::GetCustomScalarFromActor(const Actor::C_Actor* pActor, S32 scalarId) const
{
	if (NULL == pActor)
		return(-1.0f);
	
	// If the actor has an animation state component then extract the corresponding joint transform matrix.
	const Components::I_AnimationStateComponent* pAnimationState = NULL;
	if (pActor->QueryInterface(&pAnimationState))
	{
		if (scalarId >= 0)
			return(pAnimationState->GetPose().GetCustomScalar(scalarId));
	}
		
	return(-1.0f);
}

//=========================================================================

void DynamicCameraActor::ModifyWithRightStick()
{
	const Game::RightStickCameraParams*	rsparams = GetCreationData()->m_RightStickParams.Object();
	F32		blend = rsparams->m_BlendValue;

	S32 sPrimaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();								// Get Controller
	Input::C_GameInput*	gameinput = ( sPrimaryPad != -1 ) ? Input::GetGameInput( sPrimaryPad ) : NULL;
	if (!gameinput)
		return;

	F32	inputx = gameinput->GetAxisSignedValue(Framework::E_Axis_FlyRotateX);							// Get input values
	F32	inputy = gameinput->GetAxisSignedValue(Framework::E_Axis_FlyRotateY);
	F32 input_zoom = 0.0f;
	if (gameinput->GetButtonPressed(Framework::E_Button_Debug_ButtonRightStickClick))
		input_zoom = 1.0f;

	Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();		// Flip Y if needed
	if ( pGameProfile && pGameProfile->mInvertY )
		inputy = -inputy;

	m_inputx = (m_inputx * (1.0f - blend)) + (inputx * blend);											// Blend input values
	m_inputy = (m_inputy * (1.0f - blend)) + (inputy * blend);
	m_input_zoom = (m_input_zoom * (1.0f - blend)) + (input_zoom * blend);

	m_WorldMatrix.Translation(m_WorldMatrix.Translation() + (m_WorldMatrix.AZ() * rsparams->m_PivotDistance));	// Move camera to pivot position
	Maths::HMat33 spin(Maths::C_MatrixRotatePHR(Maths::HVec3((m_inputy * Maths::DegToRad(rsparams->m_MaxVerticalAngle)), (m_inputx * Maths::DegToRad(rsparams->m_MaxHorizontalAngle)), 0.0f)));
	m_WorldMatrix.Rotation( spin * m_WorldMatrix.Rotation());													// Rotate the camera
	m_WorldMatrix.Translation(m_WorldMatrix.Translation() - (m_WorldMatrix.AZ() * (rsparams->m_PivotDistance - (rsparams->m_DollyDistance * m_input_zoom))));	// Move camera back out
}


}// End namespace Actors



//#pragma optimize("", on)



//=========================================================================
//						Might be useful one day
//=========================================================================


//		LOGINFO( Resource::g_Log, "THIS IS A DEBUG MESSAGE\n");




