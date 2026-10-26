#include "stdafx.h"

#include <Game\Game.h>
#include <Game/AmaxCam/CameraSystem.h>
#include <camera2/camera.h>
#include <camera2/EffectInfo.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic/ActorStudioActors/CutSceneMessages.h>
#include <GameLogic/ActorStudioActors/ActorUtils.h>
#include <GameApp.h>

//#include "Core/Maths/HVec/New/QuatTrans.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>

#include "Game/GameControls/GameControls.h"

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
//#include <Game/GameActors/Drivers/DriverManager.h>
#include <Game/GameActors/Vehicles/VehicleActor.h>

#include <../library/kanzen/camera/HollywoodVehicleCamera.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>

#include <Game/AmaxCam/VehicleFeed.h>

#include <Game\GameWorld\PostProcessLayer.h>
#include <Graphics/PostFXSettings.h>
#include <Effect/PostProcess/DoFFgBg.h>

#include <SystemProfile/SystemProfileManager.h>
#include <GameLogic/Profile/GameProfileManager.h>

/*
#include <animation/common/StringId.h>

#include <Render/PostProcess/Bloom/BloomFeedbackGhost.h>
#include <GameLogic/Structures/LevelState.h>
*/

#include "ShowRoomCamera.h"


//OPTIMISE_OFF
//#pragma optimize("", off)

/*
Debug::Float g_debug_ShowRoomCam_TiltDefault("Camera2.ShowRoomCamera.TiltDefault", 0.13f, Debug::Range(0.0f, 0.5f));
Debug::Float g_debug_ShowRoomCam_TiltScale("Camera2.ShowRoomCamera.TiltScale", 0.5f, Debug::Range(0.1f, 2.0f));
Debug::Float g_debug_ShowRoomCam_TiltMin("Camera2.ShowRoomCamera.TiltMin", -0.1f, Debug::Range(-0.1f, 0.0f));
Debug::Float g_debug_ShowRoomCam_TiltMax("Camera2.ShowRoomCamera.TiltMax", 1.5f, Debug::Range(0.0f, 2.0f));

Debug::Float g_debug_ShowRoomCam_RotDefault("Camera2.ShowRoomCamera.RotDefault", 0.6f, Debug::Range(-4.0f, 4.0f));
Debug::Float g_debug_ShowRoomCam_RotScale("Camera2.ShowRoomCamera.RotScale", 1.0f, Debug::Range(0.5f, 2.0f));
Debug::Float g_debug_ShowRoomCam_RotMin("Camera2.ShowRoomCamera.RotMin", -5.0f, Debug::Range(-10.0f, 10.0f));
Debug::Float g_debug_ShowRoomCam_RotMax("Camera2.ShowRoomCamera.RotMax", 5.0f, Debug::Range(-10.0f, 10.0f));

Debug::Float g_debug_ShowRoomCam_XOffset("Camera2.ShowRoomCamera.XOffset", -1.2f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_debug_ShowRoomCam_YOffset("Camera2.ShowRoomCamera.YOffset", -0.25f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_debug_ShowRoomCam_RangeScaler("Camera2.ShowRoomCamera.RangeScaler", 0.5f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debug_ShowRoomCam_RangeModifier("Camera2.ShowRoomCamera.RangeModifier", 0.0f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_debug_ShowRoomCam_ZoomRangeModifier("Camera2.ShowRoomCamera.ZoomRangeModifier", 0.0f, Debug::Range(-5.0f, 5.0f));
Debug::Float g_debug_ShowRoomCam_HeightModifier("Camera2.ShowRoomCamera.HeightModifier", 0.0f, Debug::Range(-5.0f, 5.0f));

Debug::Float g_debug_ShowRoomCam_Blending("Camera2.ShowRoomCamera.Blending", 0.9f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debug_ShowRoomCam_SpinTiltBlending("Camera2.ShowRoomCamera.SpinTiltBlending", 0.998f, Debug::Range(0.5f, 1.0f));

Debug::Float g_debug_ShowRoomCam_FOVScale("Camera2.ShowRoomCamera.FOVScale", 1.0f, Debug::Range(0.2f, 2.0f));
Debug::Float g_debug_ShowRoomCam_DOFBegin("Camera2.ShowRoomCamera.DOFBegin", 2.0f, Debug::Range(0.0f, 100.0f));
Debug::Float g_debug_ShowRoomCam_DOFEnd("Camera2.ShowRoomCamera.DOFEnd", 20.0f, Debug::Range(0.1f, 100.0f));
Debug::Float g_debug_ShowRoomCam_DOFScale("Camera2.ShowRoomCamera.DOFScale", 0.75f, Debug::Range(0.0f, 1.0f));

Debug::Float g_debug_ShowRoomCam_AutospinNullZone("Camera2.ShowRoomCamera.AutospinNullZone", 0.01f, Debug::Range(0.0f, 1.0f));
Debug::Float g_debug_ShowRoomCam_AutospinDelay("Camera2.ShowRoomCamera.AutospinDelay", 5.0f, Debug::Range(1.0f, 30.0f));
Debug::Float g_debug_ShowRoomCam_AutospinSpeed("Camera2.ShowRoomCamera.AutospinSpeed", 0.08f, Debug::Range(0.0f, 0.2f));

Debug::Float g_debug_ShowRoomCam_AutospinTilt("Camera2.ShowRoomCamera.AutospinTilt", 0.15f, Debug::Range(-1.0f, 1.0f));
Debug::Float g_debug_ShowRoomCam_AutospinRoll("Camera2.ShowRoomCamera.AutospinRoll", 0.15f, Debug::Range(-1.0f, 1.0f));
*/



//=========================================================================
//		Sorcery definitions
//=========================================================================



//	Actor creation definition (Sorcery)


XT_TYPE(Actors::ShowRoomCameraParams,		"ShowRoomCameraParams")
	XT_FIELD(m_TiltDefault,			"TiltDefault")
	XT_FIELD(m_TiltScale,			"TiltScale")
	XT_FIELD(m_TiltMin,				"TiltMin")
	XT_FIELD(m_TiltMax,				"TiltMax")

	XT_FIELD(m_RotDefault,			"RotDefault")
	XT_FIELD(m_RotScale,			"RotScale")
	XT_FIELD(m_RotMin,				"RotMin")
	XT_FIELD(m_RotMax,				"RotMax")

	XT_FIELD(m_XOffset,				"XOffset")
	XT_FIELD(m_YOffset,				"YOffset")
	XT_FIELD(m_RangeScaler,			"RangeScaler")
	XT_FIELD(m_RangeModifier,		"RangeModifier")
	XT_FIELD(m_RangeModifierZoomed,	"RangeModifierZoomed")
	XT_FIELD(m_HeightModifier,		"HeightModifier")

	XT_FIELD(m_GroundOffset,		"GroundOffset")

	XT_FIELD(m_Blending,			"Blending")
	XT_FIELD(m_BlendingSpinTilt,	"BlendingSpinTilt")

	XT_FIELD(m_FOVScale,			"FOVScale")
	XT_FIELD(m_DOFBegin,			"DOFBegin")
	XT_FIELD(m_DOFEnd,				"DOFEnd")
	XT_FIELD(m_DOFScale,			"DOFScale")

	XT_FIELD(m_AutospinNullZone,	"AutospinNullZone")
	XT_FIELD(m_AutospinDelay,		"AutospinDelay")
	XT_FIELD(m_AutospinSpeed,		"AutospinSpeed")
	XT_FIELD(m_AutospinTilt,		"AutospinTilt")
	XT_FIELD(m_AutospinRoll,		"AutospinRoll")
XT_END_TYPE


XT_TYPE(Actors::ShowRoomCameraCreationData,		"ShowRoomCamera")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_CameraParams,					"CameraParams")
//	XT_FIELD(m_Viewport,						"Viewport")
XT_END_TYPE


//=========================================================================
//					ShowRoom Camera Core Routines
//=========================================================================



namespace Actors
{

ShowRoomCamera* ShowRoomCamera::m_pInstance = NULL;

//=========================================================================

ShowRoomCamera::ShowRoomCamera()
{
	m_Camera = NULL;
	m_GameInput	= NULL;
	m_params = NULL;

	m_rotation = 0.0f;
	m_tilt = 0.0f;

	m_range = 4.0f;
	m_height_offset = 1.0f;

	m_spintime = 0.0f;

	m_inputx = 0.0f;
	m_inputy = 0.0f;
	m_input_zoom = false;

	m_spin = false;
	m_spin_rotation = 0.0f;

	m_blended_tilt = 0.0f;
	m_blended_roll = 0.0f;
	m_blended_range = 0.0f;
	m_blended_height = 0.0f;
}

//=========================================================================

ShowRoomCamera::~ShowRoomCamera()
{
	AmaxCam::CameraSystem().GameFeedRemove(m_Camera);
	SAFE_DELETE_PTR(m_Camera)
}

//=========================================================================

bool ShowRoomCamera::InitialiseImpl()					// Called first
{
	if (!C_GameActor::InitialiseImpl())
		return(false);

	m_WorldMatrix = GetCreationData()->GetCreationMatrix();
	m_RotateOrigin = GetCreationData()->GetCreationMatrix();

	return(true);
}


//=========================================================================

bool ShowRoomCamera::CreateImpl()						// Called second
{
	if (!C_GameActor::CreateImpl())
		return(false);

	m_Camera = new Cam2::C_Camera;
	if (!m_Camera)
		return false;
			
	m_Camera->SetName("ShowRoom");
	m_Camera->SetPosition(m_WorldMatrix.AW());
	m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
	m_Camera->GetPrevPosition();

	m_pInstance = this;

	AddSubscriber( ActorMsg::E_DynamicObjMsg_CameraUpdate );					// Update cameras after everything else
	AddSubscriber( ActorMsg::E_CutSceneMsg_Start_GarageCamera );				// Reset camera position whenever we enter the garage

	return(true);
}

void ShowRoomCamera::KillImpl()
{
	m_pInstance = NULL;
}

//=========================================================================
//						MESSAGE HANDLERS
//=========================================================================


bool ShowRoomCamera::OnMessage(const Messaging::C_MessageBase *pMessage)
{
//	F32		spin_tilt_blend = 0.99;		// m_params->m_Blending


	switch(pMessage->Type())
	{
		case ActorMsg::E_DynamicObjMsg_CameraUpdate:				// ======= Camera Update =======
		{
			m_deltatime = Messaging::MessageCast<ActorMsg::C_CameraUpdateMsg>(pMessage)->GetValue();

			m_params = GetCreationData()->m_CameraParams;
			if (!m_params)
				break;

			UpdateVehicleParams();									// Keep checking this in case the vehicle has changed

			S32 sPrimaryPad = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
			m_GameInput = ( sPrimaryPad != -1 ) ? Input::GetGameInput( sPrimaryPad ) : NULL;

			if (m_GameInput)
			{
				m_inputx = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyRotateX);			 // w E_Axis_FlyMoveX 
				m_inputy = m_GameInput->GetAxisSignedValue(Framework::E_Axis_FlyRotateY);
				m_input_zoom = m_GameInput->GetButtonPressed(Framework::E_Button_Debug_ButtonRightStickClick);
			}

			// check for inverty
			Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();

			if ( pGameProfile && pGameProfile->mInvertY )
				m_inputy *= -1;

			CheckAutoSpin();

			if (m_spin)
				SetAutoTargetRotation();
			else
				SetManualTargetRotation();

			// Blend current towards target
			m_rotation = Blend(m_rotation, m_target_rotation, m_params->m_Blending);
			m_tilt = Blend(m_tilt, m_target_tilt, m_params->m_Blending);

			// Blend camera tilts
			if (m_spin)
			{
				m_blended_roll = Blend(m_blended_roll, m_params->m_AutospinRoll, m_params->m_BlendingSpinTilt);
				m_blended_tilt = Blend(m_blended_tilt, m_params->m_AutospinTilt, m_params->m_BlendingSpinTilt);
			}
			else
			{
				m_blended_roll = Blend(m_blended_roll, 0.0f, m_params->m_BlendingSpinTilt);
				m_blended_tilt = Blend(m_blended_tilt, 0.0f, m_params->m_BlendingSpinTilt);
			}

			UpdateWorldPosition();
			UpdateCamera();
			break;
		}

		case ActorMsg::E_CutSceneMsg_Start_GarageCamera:				// Reset camera position whenever we enter the garage
		{
			m_params = GetCreationData()->m_CameraParams;
			if (m_params)
			{
				m_spin_rotation = m_params->m_RotDefault;
				m_rotation = m_params->m_RotDefault;				// Change the actual rotation since we're blending

				m_spintime = 0.0f;									// Reset auto spin
				m_spin = false;
				m_blended_tilt = 0.0f;
				m_blended_roll = 0.0f;
			}
			break;
		}

		default:
			return C_GameActor::OnMessage(pMessage);
	}

	return(true);
}

//=========================================================================

void ShowRoomCamera::CheckAutoSpin()
{
	if ((m_inputx <= m_params->m_AutospinNullZone) && (m_inputx >= -m_params->m_AutospinNullZone) && (m_inputy <= m_params->m_AutospinNullZone) && (m_inputy >= -m_params->m_AutospinNullZone))
	{
		m_spintime += m_deltatime;
		if (m_spintime >= m_params->m_AutospinDelay)
		{
			if (!m_spin)
				m_spin_rotation = m_params->m_RotDefault;
			m_spin = true;
		}
		else
		{
			m_spin = false;
		}
	}
	else
	{
		m_spintime = 0.0f;
		m_spin = false;
	}				
}

//=========================================================================

void ShowRoomCamera::SetAutoTargetRotation()
{
	m_spin_rotation += m_params->m_AutospinSpeed * m_deltatime;								// Automatic control
	if (m_spin_rotation > Maths::Pi)					// Check for wrapping it
	{
		m_spin_rotation -= Maths::Pi * 2.0f;
		m_rotation -= Maths::Pi * 2.0f;					// Stop it blending over 360 degs
	}
	m_target_rotation = m_spin_rotation;
	m_target_tilt = m_params->m_TiltDefault;
}

//=========================================================================

void ShowRoomCamera::SetManualTargetRotation()
{
	m_target_rotation = m_inputx * Maths::Pi;												// Manual control
	m_target_tilt = m_inputy * Maths::Pi;

	m_target_tilt = (m_target_tilt * m_params->m_TiltScale) + m_params->m_TiltDefault;		// Modify based on default values
	if (m_target_tilt < m_params->m_TiltMin)
		m_target_tilt = m_params->m_TiltMin;
	else if (m_target_tilt > m_params->m_TiltMax)
		m_target_tilt = m_params->m_TiltMax;

	m_target_rotation = (m_target_rotation * m_params->m_RotScale) + m_params->m_RotDefault;
	if (m_target_rotation < m_params->m_RotMin)
		m_target_rotation = m_params->m_RotMin;
	else if (m_target_rotation > m_params->m_RotMax)
		m_target_rotation = m_params->m_RotMax;
}

//=========================================================================

void ShowRoomCamera::UpdateVehicleParams()
{
	Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
	const Game::Actors::C_VehicleActor *pActor = pRLC ? pRLC->GetVehicleActor(0) : 0;
	if (pActor)
	{
		Kanzen::HollywoodVehicleParams pParams = pActor->GetHollywoodCameraParams(0);
		if (m_input_zoom)
			m_range = (pParams.IdealDistanceNormal * m_params->m_RangeScaler) + (5.0f * (1.0f - m_params->m_RangeScaler)) + m_params->m_RangeModifierZoomed;			// Make sure smaller vehicles actually look smaller
		else
			m_range = (pParams.IdealDistanceNormal * m_params->m_RangeScaler) + (5.0f * (1.0f - m_params->m_RangeScaler)) + m_params->m_RangeModifier;			// Make sure smaller vehicles actually look smaller
//		m_height_offset = (pParams.LookAtPos.vy * m_params->m_RangeScaler) + (1.1f * (1.0f - m_params->m_RangeScaler));		// Make sure smaller vehicles actually look smaller
		m_height_offset = (pParams.IdealBase.vy * m_params->m_RangeScaler) + (1.1f * (1.0f - m_params->m_RangeScaler));		// Make sure smaller vehicles actually look smaller
	}

	m_blended_range = Blend(m_blended_range, m_range, m_params->m_Blending);
	m_blended_height = Blend(m_blended_height, m_height_offset, m_params->m_Blending);
}

//=========================================================================

void ShowRoomCamera::UpdateWorldPosition()
{
	Maths::HVec3 new_origin = m_RotateOrigin.Translation() + Maths::HVec3(0.0f, m_blended_height, 0.0f);	// Need to look above the vehicles origin (depends on vehicle height)

	Maths::HMat33 localrot(Maths::C_MatrixRotatePHR(Maths::HVec3(-m_tilt, -m_rotation, m_blended_tilt)));

	localrot = localrot * m_RotateOrigin.Rotation();

//	m_WorldMatrix.Translation( new_origin + (localrot.AZ() * (m_range + m_params->m_RangeModifier)) );
	m_WorldMatrix.Translation( new_origin + (localrot.AZ() * m_blended_range) );


	F32	miny = m_RotateOrigin.Translation().Y() + m_params->m_GroundOffset;				// Stop camera going through floor
	if (m_WorldMatrix.Translation().Y() < miny)
		m_WorldMatrix.Translation( Maths::HVec3(m_WorldMatrix.Translation().X(), miny, m_WorldMatrix.Translation().Z()) );


	new_origin.Y(new_origin.Y() + m_params->m_HeightModifier);

	PointMatAtTarget(&m_WorldMatrix, &new_origin);

	Maths::HMat33 tilt(Maths::C_MatrixRotatePHR(Maths::HVec3(0.0f, 0.0f, m_blended_roll)));
	m_WorldMatrix.Rotation( tilt * m_WorldMatrix.Rotation());

	// Offset from centre of screen
	m_WorldMatrix.Translation( m_WorldMatrix.Translation() + (m_WorldMatrix.AX() * m_params->m_XOffset) + (m_WorldMatrix.AY() * m_params->m_YOffset));
}


//=========================================================================

void ShowRoomCamera::PointMatAtTarget(Maths::HMat43* pointmat, Maths::HVec3* target)
{
	Maths::HVec3 toz = *target - pointmat->Translation();
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

F32 ShowRoomCamera::Blend(F32 current_val, F32 target_val, F32 blend)
{
	return( (current_val * blend) + (target_val * (1.0f - blend)) );
}

//=========================================================================

void ShowRoomCamera::UpdateCamera()
{
	m_Camera->SetFov(m_params->m_FOVScale);
	if (m_params->m_DOFBegin == 0.0f)
	{
		m_Camera->SetAperture(0.0f);
	}
	else
	{
//		C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetDepthOfField()->SetDepthOfField(0, 0.0f, 0.01f, m_params->m_DOFBegin, m_params->m_DOFEnd, m_params->m_DOFScale);
		Cam2::C_EffectInfo effect;
		effect.SetInfo(0.0f, 0.01f, m_params->m_DOFBegin, m_params->m_DOFEnd, m_params->m_DOFScale);
		m_Camera->SetEffectInfo(effect);

		m_Camera->SetAperture(-1.0f);
	}

	m_Camera->StartUpdate(m_deltatime);
	m_Camera->SetPosition(m_WorldMatrix.AW());
	m_Camera->SetOrient((Maths::C_Quaternion)m_WorldMatrix.Rotation());
	m_Camera->EndUpdate();
}


}// End namespace Actors



//#pragma optimize("", on)


