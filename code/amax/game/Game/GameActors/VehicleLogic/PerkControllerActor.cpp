#include "stdafx.h"

#include <Xt/XtGame.h>

#include <Game/GameActors/VehicleLogic/PerkControllerActor.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

// All these are need to access the racelogiccore... Must be a better way?
#include <GameApp.h>
#include <Game/Game.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>


#include <GameLogic/ActorStudioActors/ActorUtils.h>

#include <GameLogic\RaceLogicCore\Database\RLDatabase.h>

#include <game\gameactors\vehiclelogic\VehicleLogicActor.h>
#include <game\gameactors\vehicles\VehicleActor.h>

Debug::Logger &gAmaxPerksLogger = Debug::GetLogger("amax.Perks.messages");


namespace Game
{

namespace Actors
{

// Actor Info

C_PerkControllerActor::C_PerkControllerActor()
{

}

C_PerkControllerActor::~C_PerkControllerActor()
{

}

void C_PerkControllerActor::ActivatePerk(Actor::C_ActorHandle owner,E_PICKUP_TYPE type,int level)
{
	if(type == E_PICKUP_TYPE_SHIELD)
	{
		mVehicleShielded[owner] = true;
	}

	if(type == E_PICKUP_TYPE_MINE)
	{
		PerkUpdater nu;
		nu.mRadius=0.f;
		nu.mTotalRadius=MINE_RADIUS;
		nu.mTimer=MINE_TIMER;
		nu.mLevel=level;
		nu.mOwner=owner;
		nu.mType=type;
		
		VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( owner );

		BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

		nu.mPosition=owner_it->GetValue().mPosition;
		mMineUpdaters.PushBack(nu);
	}

	if(type == E_PICKUP_TYPE_NITROUS)
	{
		PerkUpdater nu;
		nu.mTimer=0.f;
		nu.mLevel=level;
		nu.mOwner=owner;
		nu.mType=type;
		mNitrousUpdaters.PushBack(nu);
	}

	if(type == E_PICKUP_TYPE_SHOULDER_BARGE)
	{
		PerkUpdater su;
		su.mRadius=0.f;
		su.mTotalRadius=SHOULDER_BARGE_RADIUS;
		su.mOwner=owner;
		su.mType=type;
		su.mLevel=level;
		mShoulderBargeUpdaters.PushBack(su);
	}

	if(type == E_PICKUP_TYPE_SHOCK)
	{
		bool add_one=true;

		for(int a=0;a<mShockUpdaters.Size();a++)
		{
			if(mShockUpdaters[a].mOwner == owner)
			{
				mShockUpdaters[a].mTotalRadius+=(SHOCK_RADIUS*0.3f);
				add_one=false;
			}
		}

		if(add_one)
		{
			PerkUpdater su;
			su.mRadius=0.f;
			su.mTotalRadius=SHOCK_RADIUS;
			su.mOwner=owner;
			su.mType=type;
			su.mLevel=level;
			mShockUpdaters.PushBack(su);

			VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( owner );

			BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

			::Actors::C_ActorUtils::Instance().RegisterForceEffectWithActors(owner_it->GetValue().mMatrix, E_FORCE_EFFECT_SHOCK);
		}
	}

	if(type == E_PICKUP_TYPE_SHUNT)
	{
		PerkUpdater su;
		su.mRadius=0.f;
		su.mTotalRadius=SHUNT_RADIUS;
		su.mOwner=owner;
		su.mType=type;
		su.mLevel=level;
		mShuntUpdaters.PushBack(su);
	}
}

bool C_PerkControllerActor::OnMessage(const Messaging::C_MessageBase *pMessage)
{
	PERF_EVENT("C_PerkControllerActor::MessageHandler");
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			{
				mDeltaTime=Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				Update();
			}
			break;

		case Game::E_VehicleMessage_VehicleWorldPosition:
			{
				const Game::Actors::C_VehicleWorldPositionMsg* pMsg = Messaging::MessageCast<Game::Actors::C_VehicleWorldPositionMsg>(pMessage);

				PerkCarInfo info;
				info.mPosition=pMsg->mWorldPosition;
				info.mMatrix=pMsg->mWorldMatrix;

				mVehiclePositions[pMsg->mActorHandle] = info;
			}
			break;
		case E_VehicleLogicMsg_FiredPerk:
			{		
				const C_VehicleLogicMsg_FiredPerkMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);

				//temp - need to convert to graphical vehicle actor
				C_VehicleLogicActor *vla=(C_VehicleLogicActor *)GetActorList()->GetActorPtr(pMsg->mActorHandle);

				ActivatePerk(vla->GetGraphicalActor(pMsg->mVehicleIndex)->GetHandle(),pMsg->mPickupType,pMsg->mLevel);
			}
			break;

		case E_VehicleLogicMsg_ShieldFinished:
			{
				const C_VehicleLogicMsg_ShieldFinishedMsg* pMsg = Messaging::MessageCast< C_VehicleLogicMsg_ShieldFinishedMsg >(pMessage);

				//temp - need to convert to graphical vehicle actor
				C_VehicleLogicActor *vla=(C_VehicleLogicActor *)GetActorList()->GetActorPtr(pMsg->mActorHandle);

				mVehicleShielded[vla->GetGraphicalActor(pMsg->mVehicleIndex)->GetHandle()] = false;
			}
			break;
	}

	return C_GameActor::OnMessage(pMessage);
}


bool C_PerkControllerActor::InitialiseImpl()
{
	if (!C_GameActor::InitialiseImpl())
		return false;

	const C_PerkControllerActorCreate* pCreate = (const C_PerkControllerActorCreate*)(GetCreationData().GetData());

	
	return true;
}

bool C_PerkControllerActor::CreateImpl()
{
	if (!C_GameActor::CreateImpl())
		return false;

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_VehicleWorldPosition );
	AddSubscriber( E_VehicleLogicMsg_FiredPerk );
	AddSubscriber( E_VehicleLogicMsg_ShieldFinished );

	return true;
}

bool C_PerkControllerActor::UpdateNitrous(PerkUpdater *pu)
{
	pu->mTimer+=mDeltaTime;

	if(pu->mTimer > 1.f)
	{
		return true;
	}

	RLPickupType* pPickup = (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_NITROUS);

	//TEMP HACK 
	RLPickupEffect *pEffect = pPickup->mLevels[pu->mLevel]->mEffects[0];
	RLBoostEffect *pBoostEffect = (RLBoostEffect *)pEffect;

	RLHermiteKeyFrame keyFrame;
	//RLHermiteCurve effectCurve;
	Curve::C_HermiteCurve effectCurve;

	//Build the effect curve
	for(int j = 0; j < pBoostEffect->mEffectGraph.count; j++)
	{
		keyFrame = pBoostEffect->mEffectGraph[j];

		effectCurve.AddKeyFrame(keyFrame.mTime, 
								keyFrame.mValue, 
								keyFrame.mInTangent, 
								keyFrame.mOutTangent);
	}
	//DEBUG DEBUG DEBUG
	
	VehiclePerformanceModifiers mods;

	mods.Initialise();
	mods.mBoost=effectCurve.Evaluate(pu->mTimer);
	mods.mBoost *= pBoostEffect->mMaxBoost;

	C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pu->mOwner, 0, mods);
	/*[ChadW] - BroadcastMessage_CleanUp*///GetActorList()->BroadcastMessage(&perfModMsg);
	SendMessageToActor(pu->mOwner, &perfModMsg);

	return false;
}

Debug::Bool g_debugShowShockDebug("Perks.ShowShockDebug",false,Debug::Persist());

bool C_PerkControllerActor::UpdateShock(PerkUpdater *pu)
{
	Maths::HVec3 dist;

	pu->mRadius+=((mDeltaTime*4.f) * SHOCK_RADIUS);	//arbitary numbers until I get an effect to attach to

	if(pu->mRadius > pu->mTotalRadius)
	{
		return true;
	}

	VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( pu->mOwner );

	BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

	dist=owner_it->GetValue().mPosition;

	if(g_debugShowShockDebug)
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(dist,Maths::HVec3(0,1,0),pu->mRadius,Maths::RGBA(255,255,255));

	for(VehicleWorldPos::Iterator it = mVehiclePositions.Begin(); it != mVehiclePositions.End(); ++it)
	{
		if(owner_it == it)
			continue;

		float d=dist.Distance(it->GetValue().mPosition);
		if(d < pu->mRadius)
		{
			if(!CanHit(pu,it))
				continue;

			pu->mAlreadyHit.PushBack(it->GetKey());

			if(g_debugShowShockDebug)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,it->GetValue().mPosition,Maths::RGBA(255,255,0));

			Maths::HVec3 vehicle_vector = owner_it->GetValue().mPosition - it->GetValue().mPosition;
			vehicle_vector = Maths::Normalise(vehicle_vector);

			float dot_prod1 = Maths::DotProduct(owner_it->GetValue().mMatrix.AX(), vehicle_vector);
			VehiclePerformanceModifiers mods;

			mods.Initialise();

			if(dot_prod1 <=0)	//enemy is to the right of the cars direction line
				mods.mNewSpinCar=1;
			else
				mods.mNewSpinCar=-1;

			C_VehicleUpdatePerformanceModifiersMsg perfModMsg(it->GetKey(), 0, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///GetActorList()->BroadcastMessage(&perfModMsg);
			SendMessageToActor(it->GetKey(), &perfModMsg);

			C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pu->mType,pu->mLevel,0);
			GetActorList()->BroadcastMessage(&hit_by_msg);
		}
	}

	return false;
}

float C_PerkControllerActor::VehicleToVehicleAngle(const PerkCarInfo *owner, const PerkCarInfo *other)
{
	Maths::HVec3 diff = other->mPosition - owner->mPosition;
	float vangle = Maths::ArcTan( owner->mMatrix.AZ().X(), owner->mMatrix.AZ().Z() );
	float ang = Maths::ArcTan( diff.X(), diff.Z() );
	ang=vangle-ang;

	while ( ang < 0.0f )
		ang += Maths::Pi2;
	while ( ang > Maths::Pi2 )
		ang -= Maths::Pi2;

	return ang;
}

Debug::Bool g_debugShowShoulderBargeDebug("Perks.ShowShoulderBargeDebug",false,Debug::Persist());

bool C_PerkControllerActor::UpdateShoulderBarge(PerkUpdater *pu)
{
	Maths::HVec3 dist;

	pu->mRadius+=((mDeltaTime*4.f) * SHOULDER_BARGE_RADIUS);	//arbitary numbers until I get an effect to attach to

	if(pu->mRadius > pu->mTotalRadius)
	{
		return true;
	}

	VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( pu->mOwner );

	BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

	dist=owner_it->GetValue().mPosition;

	if(g_debugShowShoulderBargeDebug)
	{
		Maths::HVec3 hv=owner_it->GetValue().mMatrix.AZ();
		hv=Maths::Normalise(hv);
		hv*=pu->mTotalRadius;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+hv,Maths::RGBA(0,0,128));

		Maths::HMat33 rot(Maths::RotY(Maths::PiBy2 + Maths::Pi*0.2f));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot),Maths::RGBA(0,0,255));

		Maths::HMat33 rot1(Maths::RotY(Maths::PiBy2 - Maths::Pi*0.2f));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot1),Maths::RGBA(0,0,255));

		Maths::HMat33 rot2(Maths::RotY(-Maths::PiBy2 + Maths::Pi*0.2f));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot2),Maths::RGBA(0,0,255));

		Maths::HMat33 rot3(Maths::RotY(-Maths::PiBy2 - Maths::Pi*0.2f));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot3),Maths::RGBA(0,0,255));

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(dist,Maths::HVec3(0,1,0),pu->mRadius,Maths::RGBA(0,0,255));
	}

	for(VehicleWorldPos::Iterator it = mVehiclePositions.Begin(); it != mVehiclePositions.End(); ++it)
	{
		if(owner_it == it)
			continue;

		if(!CanHit(pu,it))
			continue;

		float d=dist.Distance(it->GetValue().mPosition);
		if(d < pu->mRadius)
		{
			pu->mAlreadyHit.PushBack(it->GetKey());

			Maths::HVec3 vehicle_vector = owner_it->GetValue().mPosition - it->GetValue().mPosition;
			vehicle_vector = Maths::Normalise(vehicle_vector);

			if(g_debugShowShoulderBargeDebug)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,it->GetValue().mPosition,Maths::RGBA(255,255,0));

			float ang=VehicleToVehicleAngle(&owner_it->GetValue(), &it->GetValue());

			if(ang > Maths::Pi15-(Maths::Pi*0.2f) && ang < Maths::Pi15+(Maths::Pi*0.2f) || 
				ang > Maths::PiBy2-(Maths::Pi*0.2f) && ang < Maths::PiBy2+(Maths::Pi*0.2f) )
			{
				Maths::HVec3 vehicle_vector = owner_it->GetValue().mPosition - it->GetValue().mPosition;
				vehicle_vector = Maths::Normalise(vehicle_vector);

				float dot_prod1 = Maths::DotProduct(owner_it->GetValue().mMatrix.AX(), vehicle_vector);
				VehiclePerformanceModifiers mods;

				mods.Initialise();

				if(dot_prod1 <=0)	//enemy is to the right of the cars direction line
					mods.mNewPushCar=1;
				else
					mods.mNewPushCar=-1;

				C_VehicleUpdatePerformanceModifiersMsg perfModMsg(it->GetKey(), 0, mods);
				/*[ChadW] - BroadcastMessage_CleanUp*///GetActorList()->BroadcastMessage(&perfModMsg);
				SendMessageToActor(it->GetKey(), &perfModMsg);

				C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pu->mType,pu->mLevel,0);
				GetActorList()->BroadcastMessage(&hit_by_msg);
			}
		}
	}

	return false;
}

bool C_PerkControllerActor::UpdateShunt(PerkUpdater *pu)
{
	Maths::HVec3 dist;

	pu->mRadius+=((mDeltaTime*4.f) * SHUNT_RADIUS);	//arbitary numbers until I get an effect to attach to

	if(pu->mRadius > pu->mTotalRadius)
	{
		return true;
	}

	VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( pu->mOwner );

	BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

	dist=owner_it->GetValue().mPosition;

	::Actors::C_ActorUtils::Instance().RegisterForceEffectWithActors(owner_it->GetValue().mMatrix, E_FORCE_EFFECT_SHUNT_FIRE);

	//if(g_debugShowShuntDebug)
	//{
	//	Maths::HVec3 hv=owner_it->GetValue().mMatrix.AZ();
	//	hv=Maths::Normalise(hv);
	//	hv*=15.f;
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+hv,Maths::RGBA(128,0,0));

	//	Maths::HMat33 rot(Maths::RotY(Maths::Pi*0.1f));
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot),Maths::RGBA(255,0,0));

	//	Maths::HMat33 rot1(Maths::RotY(-Maths::Pi*0.1f));
	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,owner_it->GetValue().mPosition+(hv*rot1),Maths::RGBA(255,0,0));

	//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(dist,Maths::HVec3(0,1,0),pu->mRadius,Maths::RGBA(255,0,0));
	//}

	for(VehicleWorldPos::Iterator it = mVehiclePositions.Begin(); it != mVehiclePositions.End(); ++it)
	{
		if(owner_it == it)
			continue;

		if(!CanHit(pu,it))
			continue;

		float d=dist.Distance(it->GetValue().mPosition);
		if(d < pu->mRadius)
		{
			Maths::HVec3 vehicle_vector = owner_it->GetValue().mPosition - it->GetValue().mPosition;
			vehicle_vector = Maths::Normalise(vehicle_vector);

			//if(g_debugShowShuntDebug)
			//	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(owner_it->GetValue().mPosition,it->GetValue().mPosition,Maths::RGBA(255,255,0));

			float ang=VehicleToVehicleAngle(&owner_it->GetValue(), &it->GetValue());

			if(ang > Maths::Pi2-(Maths::Pi*0.1f) || ang < Maths::Pi*0.1f)
			{
				float dot_prod1 = Maths::DotProduct(owner_it->GetValue().mMatrix.AX(), vehicle_vector);
				VehiclePerformanceModifiers mods;

				pu->mAlreadyHit.PushBack(it->GetKey());

				mods.Initialise();

				if(dot_prod1 <=0)	//enemy is to the right of the cars direction line
					mods.mNewFlipCar=1;
				else
					mods.mNewFlipCar=-1;

				C_VehicleUpdatePerformanceModifiersMsg perfModMsg(it->GetKey(), 0, mods);
				/*[ChadW] - BroadcastMessage_CleanUp*///GetActorList()->BroadcastMessage(&perfModMsg);
				SendMessageToActor(it->GetKey(), &perfModMsg);

				C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pu->mType,pu->mLevel,0);
				GetActorList()->BroadcastMessage(&hit_by_msg);
			}
		}
	}

	return false;
}

bool C_PerkControllerActor::CanHit(PerkUpdater *pu,VehicleWorldPos::Iterator it)
{
	bool already_done=false;
	for(int b=0;b < pu->mAlreadyHit.Size();b++)
	{
		if(pu->mAlreadyHit[b] == it->GetKey())
			already_done=true;
	}

	VehicleShielded::ConstIterator shielded_it = mVehicleShielded.Find( it->GetKey() );

	if(shielded_it != mVehicleShielded.End())
	{
		if(shielded_it->GetValue() == true)
			already_done=true;
	}

	return !already_done;
}

void C_PerkControllerActor::ExplodeMine(PerkUpdater *pu)
{
	Maths::HVec3 dist=pu->mPosition;
	
	for(int a=0;a<mMineUpdaters.Size();a++)
	{
		float d=dist.Distance(mMineUpdaters[a].mPosition);
		
		if(d < pu->mTotalRadius*2)
		{			
			mMineUpdaters[a].mTimer=0.1f;	//explode in x secs ... trying to create a ripple effect(ish)
		}
	}
}

Debug::Bool g_debugShowMineDebug("Perks.ShowMineDebug",false,Debug::Persist());

bool C_PerkControllerActor::UpdateMine(PerkUpdater *pu)
{
	Maths::HVec3 dist;

	pu->mTimer-=mDeltaTime;

	if(pu->mTimer < 0.f)
	{
		ExplodeMine(pu);
		return true;
	}	

	pu->mRadius+=mDeltaTime*4.f;
	if(pu->mRadius > pu->mTotalRadius)
	{
		pu->mRadius=0.f;
	}

	if(g_debugShowMineDebug)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(pu->mPosition,1.f,Maths::RGBA(255,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(pu->mPosition,Maths::HVec3(0,1,0),pu->mRadius,Maths::RGBA(128,0,128));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(pu->mPosition,Maths::HVec3(0,1,0),pu->mTotalRadius,Maths::RGBA(255,0,255));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(pu->mPosition+Maths::HVec3(0,0.5f,0),Util::C_String().Printf("%0.1f",pu->mTimer));
	}

	VehicleWorldPos::ConstIterator owner_it = mVehiclePositions.Find( pu->mOwner );

	BIZ_ASSERT_MSG(owner_it != mVehiclePositions.End(), "Unable to find car in ActivatePerk");

	dist=pu->mPosition;

	for(VehicleWorldPos::Iterator it = mVehiclePositions.Begin(); it != mVehiclePositions.End(); ++it)
	{
		if(owner_it == it)
			continue;

		if(!CanHit(pu,it))
			continue;

		float d=dist.Distance(it->GetValue().mPosition);
		if(d < pu->mTotalRadius)
		{
			pu->mAlreadyHit.PushBack(it->GetKey());

			VehiclePerformanceModifiers mods;

			mods.Initialise();

			mods.mNewMineCar=1;

			C_VehicleUpdatePerformanceModifiersMsg perfModMsg(it->GetKey(), 0, mods);
			/*[ChadW] - BroadcastMessage_CleanUp*///GetActorList()->BroadcastMessage(&perfModMsg);
			SendMessageToActor(it->GetKey(), &perfModMsg);

			C_VehicleLogicMsg_HitByPerkMsg hit_by_msg(pu->mType,pu->mLevel,0);
			GetActorList()->BroadcastMessage(&hit_by_msg);

			ExplodeMine(pu);
			return true;
		}
	}

	return false;
}

void C_PerkControllerActor::Update()
{
	for(int a=0;a<mShockUpdaters.Size();a++)
	{
		bool erase=UpdateShock(&mShockUpdaters[a]);

		if(erase)
		{
			mShockUpdaters.EraseByIndex(a);
		}
	}

	for(int a=0;a<mNitrousUpdaters.Size();a++)
	{
		bool erase=UpdateNitrous(&mNitrousUpdaters[a]);

		if(erase)
		{
			mNitrousUpdaters.EraseByIndex(a);
		}
	}

	for(int a=0;a<mShoulderBargeUpdaters.Size();a++)
	{
		bool erase=UpdateShoulderBarge(&mShoulderBargeUpdaters[a]);

		if(erase)
		{
			mShoulderBargeUpdaters.EraseByIndex(a);
		}
	}

	for(int a=0;a<mShuntUpdaters.Size();a++)
	{
		bool erase=UpdateShunt(&mShuntUpdaters[a]);

		if(erase)
		{
			mShuntUpdaters.EraseByIndex(a);
		}
	}

	for(int a=0;a<mMineUpdaters.Size();a++)
	{
		bool erase=UpdateMine(&mMineUpdaters[a]);

		if(erase)
		{
			mMineUpdaters.EraseByIndex(a);
		}
	}
}

} // namespace Actors

} // namespace Game


