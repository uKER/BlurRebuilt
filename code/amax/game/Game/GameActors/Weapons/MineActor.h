/////////////////////////////////////////////////////////////
// MineActor
/////////////////////////////////////////////////////////////

#ifndef __MINE_ACTOR_H__
#define __MINE_ACTOR_H__

#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>

#include <gravity/sdk/BodyHelper.h>
#include <gravity/importer/Importer.h>

#include "../../../GameLogic/RaceLogicCore/Data/RLDefinitions.h"


namespace SpecialFX
{
	class I_SpecialFXInstance;
};


namespace Game
{

namespace Actors
{
class RaceLogicCore;
/* 
===============================================================
					class C_MineActorCreate

NOTES: 
===============================================================
*/
class MineActorCreate : public ::Actors::C_PosActorCreate
{
public:

	MineActorCreate()
	{
		m_pRLC = 0;

		mFiredByVehicleIndex = -1;
		mWorldMatrix = Maths::HMat43();
		mPickupPod.Initialise();

		mLifeTimeInSeconds = 0.f;
		mTriggerRadius = 0.f;
		mInfiniteLife = false;
	}

	static Actor::C_ActorTypeId					ClassType() { return E_ActorTypeId_Mine; }
	const Actor::C_ActorTypeId					GetType() const { return E_ActorTypeId_Mine; }

	Game::Actors::RaceLogicCore					*m_pRLC;
	Maths::HMat43								mWorldMatrix;
	int											mFiredByVehicleIndex;

	float										mTriggerRadius,
												mLifeTimeInSeconds;
	bool										mInfiniteLife;

	RLPickupPod									mPickupPod;
	XtHandle<SpecialFX::C_SpecialFX>			mWeaponSFX;
};

/* 
===============================================================
					class C_MineActor

NOTES: 
===============================================================
*/
class MineActor : public C_GameActor
{
public:

	MineActor(){};
	~MineActor(){};

	static const Actor::C_ActorTypeId			TypeId()	{ return E_ActorTypeId_Mine; }
	static const Btl::String32					TypeName()	{ return "Mine"; }

	//! Update
	void										Update(const float frameTime);

	virtual bool								OnMessage(const Messaging::C_MessageBase *pMessage);
	bool										Create();

protected:

	virtual bool								InitialiseImpl();
	virtual bool								CreateImpl();

private:
	void										Render();
	void										CheckCollision(const Maths::HVec3 worldPosition);
	
	void										Explode();

//Variables
private:

	Maths::HMat43								mWorldMatrix;

	float										mCollisionRadius,
												mLifeTime,				//So that the missile doesn't carry on forever
												mAge;					//The current age of the missile
												
	bool										mActive,
												mInfiniteLife;

	Game::Actors::RaceLogicCore					*m_pRLC;
	RLPickupPod									mPickupPod;
	SpecialFX::I_SpecialFXInstance				*m_pWeaponSpecialFX;
};

} //namespace Actors

} //namespace Game

#endif //__MINE_ACTOR_H__
