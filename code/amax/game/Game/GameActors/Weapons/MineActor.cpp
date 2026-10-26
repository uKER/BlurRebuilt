/////////////////////////////////////////////////////////////
// MineActor
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MineActor.h"


#include <Game/ActorDef/GameActor.h>

#include "../../Messaging/GameMessages.h"
#include "../../Messaging/VehicleActorMessages.h"
#include "../../Messaging/PickupActorMessages.h"

#include "../../../GameLogic/RaceLogicCore/Core/RaceLogicCore.h"
#include <SpecialFX/SpecialFX.h>


Debug::Logger &g_MineActorLog = Debug::GetLogger("amax.actors.missile");



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//From RLDebugLineController.cpp
extern Debug::Bool g_RenderMinePositions;
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
namespace Game
{
namespace Actors
{


bool MineActor::InitialiseImpl()
{
	return true;
}

bool MineActor::CreateImpl()
{
	const MineActorCreate *pMineCreate = (const MineActorCreate *)(GetCreationData().GetData());


	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_VehicleWorldPosition );
	AddSubscriber( Game::E_GameflowMessage_ClearStoredActorPtrs );

	mActive = true;
	mCollisionRadius = pMineCreate->mTriggerRadius;

	mWorldMatrix = pMineCreate->mWorldMatrix;

	mPickupPod = pMineCreate->mPickupPod;
	mPickupPod.mWorldMatrix = mWorldMatrix;

	
	mInfiniteLife = pMineCreate->mInfiniteLife;

	if(!mInfiniteLife)
		mLifeTime = pMineCreate->mLifeTimeInSeconds;
	else
		mLifeTime = 1.f;

	mAge = 0.f;

	m_pRLC = pMineCreate->m_pRLC;

	m_pWeaponSpecialFX = NULL;

	//Create our specialFX if we've got one
	if (pMineCreate->mWeaponSFX.Object())
	{
		m_pWeaponSpecialFX = SpecialFX::C_SpecialFXManager::CreateSpecialFXInstance(pMineCreate->mWeaponSFX, GetActorList()->GetDefaultContextProvider());
		m_pWeaponSpecialFX->Start();
		m_pWeaponSpecialFX->UpdateWorldMatrix(mWorldMatrix);
		m_pWeaponSpecialFX->Update( 0 );
	}

	return true;
}

void MineActor::Update(const float frameTime)
{

		//Update the age of the missile so it doesn't run on forever
		if((mAge + frameTime) >= mLifeTime)
		{
			if( mPickupPod.mTimeLeft > 0.f)
				Explode();
			else
			{
				if (m_pWeaponSpecialFX)
				{
					m_pWeaponSpecialFX->Stop();
					SAFE_DELETE_PTR(m_pWeaponSpecialFX);
				}

				Kill();
			}

			return;
		}
		else if(!mInfiniteLife)
			mAge += frameTime;
	

	if(m_pWeaponSpecialFX)
	{
		m_pWeaponSpecialFX->UpdateWorldMatrix(mWorldMatrix);
		m_pWeaponSpecialFX->Update( frameTime );
	}

}

void MineActor::CheckCollision(const Maths::HVec3 worldPosition)
{
	if(!mActive)
		return;

	float radiusSquared = mCollisionRadius * mCollisionRadius;
	Maths::HVec3 toTarget = worldPosition - mWorldMatrix.AW();

	if(toTarget.LengthSquared() <= radiusSquared)
	{
		Explode();
	}
}

void MineActor::Explode()
{
	if(mActive)
	{
		mActive = false;
		mAge = mLifeTime;

		MineExplodedMsg bangMsg;
		bangMsg.mMineActorHandle = GetCreationData()->GetActorHandle();
		bangMsg.mWorldPosition = mWorldMatrix.AW();

		BroadcastMessage(&bangMsg);
	}

}

void MineActor::Render()
{
	//DEBUG DEBUG DEBUG
	if(g_RenderMinePositions)
	{
		Maths::HVec3 a, b;

		a = b = mWorldMatrix.AW();

		a += mWorldMatrix.AY() * 10.f;
		b -= mWorldMatrix.AY() * 10.f;

		Maths::RGBA colour = Maths::RGBA(0xff, 0x00, 0x00, 0xff);

		if(mActive)
			colour = Maths::RGBA(0xff, 0xff, 0xff, 0xff);

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(mWorldMatrix.AW(), mCollisionRadius, colour);
	}
	//DEBUG DEBUG DEBUG
}

bool MineActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("MineActor::MessageHandler");
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		{
			const GameMessaging::C_UpdateMsg* pMsg = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage);

			Update(pMsg->GetValue());
			Render();
			return true;
			break;
		}
	case Game::E_VehicleMessage_VehicleWorldPosition:
		{
			const Game::Actors::C_VehicleWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleWorldPositionMsg>(pMessage);

			//See if we've been bumped into
			CheckCollision(pMsg->mWorldPosition);
			return true;
		}
	case Game::E_GameflowMessage_ClearStoredActorPtrs:
		{
			m_pRLC = NULL;
			return true;
		}

	}

	return Actor::C_Actor::OnMessage(pMessage);
}


}; //namespace Actors
}; //namespace Game

