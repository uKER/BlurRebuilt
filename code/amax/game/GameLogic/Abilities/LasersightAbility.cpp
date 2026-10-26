#include "LasersightAbility.h"

#include "Game/Game.h"
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"
#include "Game/AmaxCam/CameraSystem.h"
#include "Game/AmaxCam/VehicleFeed.h"


#ifndef CFG_GOLD
Debug::Bool g_debugLaserSightDraw("Multiplayer.Upgrades.LaserSightDebugDraw",false);
#endif //CFG_GOLD

namespace Game
{

C_LasersightAbility::C_LasersightAbility()
{
	mpMultiplayerAbilities = 0;
	mActive = false;
}

void C_LasersightAbility::Init(U8 vehicleIndex, C_LevelStateMultiplayerVehicleAbilities*  multiplayerAbilities, Gravity::C_Scene* physicsScene, MiscModSettings* modSettings, S8 viewportIndex)
{
	mVehicleIndex = vehicleIndex;
	mpMultiplayerAbilities = multiplayerAbilities;
	mPhysicsScene = physicsScene;
	mActive = false;
	mSettings = modSettings;
	mViewportIndex = viewportIndex;

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( GameMessaging::E_GameMsg_FinalUpdate );
}

void C_LasersightAbility::Activate(bool activate)
{
	if(mActive && !activate)
	{
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Laser_Sight, false);
	}
	mActive = activate;

}


void C_LasersightAbility::Update(float timeStep)
{
	if(!mActive)
	{
		return;
	}

	Game::C_MultiplayerVehicleAbility* laserSightAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Laser_Sight);

	if(laserSightAbility->GetActive())
	{
		Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(mVehicleIndex);
		Game::Actors::C_VehicleActor* pVehicle = pLogicActor->GetGraphicalActor(mVehicleIndex);

		if(mViewportIndex != -1 && pVehicle->GetCameraType(mViewportIndex) == AmaxCam::E_VFCamera_Bumper && fabs(AmaxCam::CameraSystem().VehicleFeed(mViewportIndex)->GetLookAngle()) > 1.5f)
		{
			Activate(false);
			return;
		}

		Collision::C_Ray ray;
		Collision::C_CollisionFilter filter(0xffffffff, Game::PolyFlags::kCameraSolid, pVehicle->GetCollisionBody()->GetShape()->GetFilter()->GetGroupId(),0,0 );

		//Cast the initial ray straight in front of the car
		Maths::C_Normal direction( pVehicle->GetDirection() );
		Maths::HVec3 start = pVehicle->GetPosition();
		
		ray.InitialiseRay(start, direction , laserSightAbility->GetValue(), &filter, 0.f);

		U8 data[1024 * sizeof(void*)];
		SharedPhysics::C_SafeStack<true> stack(data, 1024);

		mPhysicsScene->DoRayTrace(ray, stack);

		F32 angle = 0.f;
		F32 angleDeg = 0.f;
		F32 distance = ray.m_result.Distance();

		//If the distance is shorter than half the max length and we are not aiming at a wall then aim higher
		if(ray.m_result.Distance() < laserSightAbility->GetValue() / 2.f && ray.m_result.Normal().Y() > mSettings->m_laserSightMinNormal)
		{
			Maths::HVec3 dirCrossRay = direction ^ ray.m_result.Normal();
			dirCrossRay.Normalise();

			Maths::HVec3 resCrossRay = ray.m_result.Normal() ^ dirCrossRay;
			resCrossRay.Normalise();

			Collision::C_Ray ray2;
			Collision::C_CollisionFilter filter(0xffffffff, Game::PolyFlags::kCameraSolid, pVehicle->GetCollisionBody()->GetShape()->GetFilter()->GetGroupId(),0,0 );

			Maths::C_Normal direction( resCrossRay );
			
			ray2.InitialiseRay(start, direction, laserSightAbility->GetValue(), &filter, 0.f);

			U8 data[1024 * sizeof(void*)];
			SharedPhysics::C_SafeStack<true> stack(data, 1024);

			mPhysicsScene->DoRayTrace(ray2, stack);

#ifndef CFG_GOLD
			//debug rendering
			if (g_debugLaserSightDraw)
			{
				DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
				debugRender->Draw3dLine(start, ray2.m_result.HitPoint(), Maths::RGBA(0xff00ff00));
			}
#endif //CFG_GOLD

			Maths::HVec3 test = ray2.m_result.HitPoint() - start;
			test.Normalise();

			angle = acosf(Maths::DotProduct(Maths::HVec3(pVehicle->GetDirection()), resCrossRay));
			static F32 adjustment = 0.f;
			angle += adjustment;
			angleDeg = angle * 180.0f / Maths::Pi;
			distance = ray2.m_result.Distance();

			if(Maths::DotProduct(resCrossRay, pLogicActor->GetWorldMatrix().AY()) < 0.f)
			{
				angle = -angle;
			}

			//Max/min angle as we don't want it shooting out the top of the car
			angle = Maths::Clamped<F32>(angle, -mSettings->m_laserSightMaxAngle, mSettings->m_laserSightMaxAngle);
		}

		distance <= laserSightAbility->GetValue() ? distance : 200.f;
		
		mpMultiplayerAbilities->SetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Laser_Sight, true, false, mVehicleIndex, -1, false, distance, timeStep, Maths::HVec3(-angle, 0.f, 0.f));

#ifndef CFG_GOLD
		//debug rendering
		if (g_debugLaserSightDraw)
		{
			DbUtil::I_DebugRender* debugRender = DbUtil::DefaultDebugRenderSys()->Deferred();
			debugRender->Draw3dLine(start, ray.m_result.HitPoint(), Maths::RGBA(0xffff0000));

			if (ray.m_result.m_pShape)
			{
				debugRender->Draw3dSphere(ray.m_result.HitPoint(), 0.5f, Maths::RGBA(0xffff0000));
			}
		}
#endif //CFG_GOLD
		
	}

}

bool C_LasersightAbility::MessageHandler(const Messaging::C_MessageBase *pMessage) 
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_FinalUpdate:
		{
			const GameMessaging::C_FinalUpdateMsg* msg = Messaging::MessageCast< GameMessaging::C_FinalUpdateMsg >(pMessage);
			Update(msg->GetValue());
		}
		break;
	}

	return true;
}

}
