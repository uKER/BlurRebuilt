#include "stdafx.h"

#include <Game\GameActors\Components\VehicleLogic\Scoring\VLC_NewScoring.h>
#include <GameLogic\Structures\NetEventManager.h>
#include <GameLogic\Structures\NetEventPackets.h>
#include <GameLogic\Structures\LevelState.h>
#include <GameLogic\Pickups\PickupBase.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic\Replays\MemChunk.h>

namespace Game
{
//	C_NetEventPacket

	enum E_FirePickupBits
	{
		E_FirePickupBits_TargetValid	= (1 << 0),
		E_FirePickupBits_Delay			= (1 << 1),
		E_FirePickupBits_Parent			= (1 << 2),
		E_FirePickupBits_GroupId		= (1 << 3),
		E_FirePickupBits_OverrideMatrix	= (1 << 4),
		E_FirePickupBits_DisableFX		= (1 << 5),
		E_FirePickupBits_HostControls	= (1 << 6),
		E_FirePickupBits_Value1Valid	= (1 << 7),
		E_FirePickupBits_Value2Valid	= (1 << 8),
		E_FirePickupBits_Value3Valid	= (1 << 9),
	};

	const U8 pickupDelayScaler = 100;

C_NetEventPacket::C_NetEventPacket(U32 p_MessageNum)
	: m_MessageNumber((U16)p_MessageNum)
{
	m_pEventBuffer = new C_MemChunk(64);
}

C_NetEventPacket::C_NetEventPacket(const C_NetEventPacket& o)
{
	*this = o;
}

C_NetEventPacket::~C_NetEventPacket()
{
	delete m_pEventBuffer;
}

C_NetEventPacket& C_NetEventPacket::operator =(const C_NetEventPacket& o)
{
	m_pEventBuffer = new C_MemChunk(64);
	m_pEventBuffer->Clone(o.GetEventBuffer());
	m_pEventBuffer->MoveForward(o.GetEventBuffer()->GetCurrent() - o.GetEventBuffer()->GetStart());
	m_pEventBuffer->SetMaxSize(o.GetEventBuffer()->GetMaxSize());
	
	m_MessageNumber = o.m_MessageNumber;

	return *this;
}

void C_NetEventPacket::ParseIntoNetMsg(RacerNet::C_NetMessage& p_Msg) const
{
	p_Msg.WriteU16(m_MessageNumber);
	p_Msg.WriteBuffer(m_pEventBuffer->GetStart(), m_pEventBuffer->GetUsedSize());
}


const U8 NET_MAX_MESSAGE_LIST_SIZE = 30;

C_NetEventManager::C_NetEventManager()
{
	m_EventList.Clear();
}


void C_NetEventManager::Clear()
{
	bool result = m_EventList.Empty();
	m_EventList.Clear();
}

bool C_NetEventManager::IsListFull()
{
	if(m_EventList.Size() >= MAX_NET_EVENT_LIST_SIZE)
		return true;
	else
		return false;
}

void C_NetEventManager::FreeOneSlot()
{
	//	Ensure we've got space to add a new event to the list, if not then delete the oldest
	//	- this loop should NEVER complete more than once (if it does then the list is larger than it should ever legally be)
	while(IsListFull())
	{
		m_EventList.PopFront();
	}
}


U16 C_NetEventManager::GetNextEventNum()
{
	FreeOneSlot();

	Game::C_LevelState& levelState = C_GameApp::GetInstance()->GetGame()->GetLevelState();
	//	Increase the message num. counter for this kit
	levelState.mCars[0].mNetMessageNos[0]++;
	U16 eventNum = levelState.mCars[0].mNetMessageNos[0];

	BIZ_ASSERT_MSG(levelState.mCars[0].mNetMessageNos[0] != (U16)(-1), "OH GOD WHY DID THIS HAPPEN?!!!! Inform GavinY please!");

	return eventNum;
}

void C_NetEventManager::Add_SetPowerupType(U32 powerupId, U8 type, bool isNext)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_SetPowerupType);
	temp.GetEventBuffer()->PushU32(powerupId);
	temp.GetEventBuffer()->PushU8(type);
	temp.GetEventBuffer()->PushU8(isNext);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_SetPowerupRespawnDelay(U32 powerupId, U8 lastJoinIDCollectedBy,  F32 respawnDelayMultiplier)
{
	U8 eventType = (U8)E_NetEventType_SetPowerupRespawnDelay;

	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(eventType);
	temp.GetEventBuffer()->PushU32(powerupId);
	temp.GetEventBuffer()->PushU8(lastJoinIDCollectedBy);
	temp.GetEventBuffer()->PushF32(respawnDelayMultiplier);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_DroppedPickupPickedUp(U32 uniqueId)
{
	U8 eventType = (U8)E_NetEventType_DroppedPickupPickedUp;

	C_NetEventPacket temp(GetNextEventNum());
	
	temp.GetEventBuffer()->PushU8(eventType);

	temp.GetEventBuffer()->PushU32(uniqueId);

	m_EventList.PushBack(temp);

}

void C_NetEventManager::Add_RegisterShocksWithHost(U8 p_FirerId, U8 p_TargetId, U32 uniqueID)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_RegisterShocks);

	temp.GetEventBuffer()->PushU8(p_FirerId);
	temp.GetEventBuffer()->PushU8(p_TargetId);
	temp.GetEventBuffer()->PushU32(uniqueID);

	m_EventList.PushBack(temp);
}

//	Net event for a Pickup being fired (activated, so in the case of Pickups with an initial delay this is at the start of the delay timer)
void C_NetEventManager::Add_PickupFired( U8 p_FirerId, U8 p_TargetId, PickupCreationStruct& pickupInfo )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_PickupFired);

	U16 infoBits = 0;
	if(p_TargetId != 0)
		infoBits |= E_FirePickupBits_TargetValid;
	if(pickupInfo.mDelay != .0f)
		infoBits |= E_FirePickupBits_Delay;
	if(pickupInfo.mParentPickupId != 0)
		infoBits |= E_FirePickupBits_Parent;
	if(pickupInfo.mGroupPickupId != 0)
		infoBits |= E_FirePickupBits_GroupId;
	if(pickupInfo.mOverrideMatrix)
		infoBits |= E_FirePickupBits_OverrideMatrix;
	if(pickupInfo.mDisableVehicleFX)
		infoBits |= E_FirePickupBits_DisableFX;
	if(pickupInfo.mHostIsController)
		infoBits |= E_FirePickupBits_HostControls;
	if(pickupInfo.mVal != 1.0f)
		infoBits |= E_FirePickupBits_Value1Valid;
	if(pickupInfo.mVal2 != 1.0f)
		infoBits |= E_FirePickupBits_Value2Valid;
	if(pickupInfo.mVal3 != 0)
		infoBits |= E_FirePickupBits_Value3Valid;

	U8 pickupTypeAndSubType = (pickupInfo.mPickupType << 4) | pickupInfo.mPickupSubType;
	U8 scaledDelay = pickupInfo.mDelay * pickupDelayScaler;

	temp.GetEventBuffer()->PushU16(infoBits);
	temp.GetEventBuffer()->PushU8(p_FirerId);
	temp.GetEventBuffer()->PushU8(pickupTypeAndSubType);
	temp.GetEventBuffer()->PushU32(pickupInfo.mPickupId);

	//	Optionals
	if(infoBits & E_FirePickupBits_TargetValid)
		temp.GetEventBuffer()->PushU8(p_TargetId);
	if(infoBits & E_FirePickupBits_Delay)
		temp.GetEventBuffer()->PushU8(scaledDelay);
	if(infoBits & E_FirePickupBits_Parent)
		temp.GetEventBuffer()->PushU8(pickupInfo.mPickupId - pickupInfo.mParentPickupId);
	if(infoBits & E_FirePickupBits_GroupId)
		temp.GetEventBuffer()->PushU16((U16)pickupInfo.mGroupPickupId);	//	Cast this U32 down to a U16 - there's no reasonable way we'll fill a U16
	if(infoBits & E_FirePickupBits_OverrideMatrix)
		temp.GetEventBuffer()->Push((U8*)&pickupInfo.mMatrix, sizeof(pickupInfo.mMatrix));
	if(infoBits & E_FirePickupBits_Value1Valid)
		temp.GetEventBuffer()->PushF32(pickupInfo.mVal);
	if(infoBits & E_FirePickupBits_Value2Valid)
		temp.GetEventBuffer()->PushF32(pickupInfo.mVal2);
	if(infoBits & E_FirePickupBits_Value3Valid)
		temp.GetEventBuffer()->PushU32(pickupInfo.mVal3);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Read_PickupFired( RacerNet::C_NetMessage &p_Msg, U8& p_FirerId, U8& p_TargetId, PickupCreationStruct& pickupInfo )
{
	U16 infoBits, pickupGroupId;
	U8 typeAndSubType, parentPickupIdDelta;
	U8 scaledDelay = 0.f;

	p_Msg.ReadU16(infoBits);
	p_Msg.ReadU8(p_FirerId);
	p_Msg.ReadU8(typeAndSubType);		//	need to cast this to enum
	p_Msg.ReadU32(pickupInfo.mPickupId);

	//	Optionals
	if(infoBits & E_FirePickupBits_TargetValid)
		p_Msg.ReadU8(p_TargetId);
	if(infoBits & E_FirePickupBits_Delay)
		p_Msg.ReadU8(scaledDelay);
	if(infoBits & E_FirePickupBits_Parent)
	{
		p_Msg.ReadU8(parentPickupIdDelta);
		pickupInfo.mParentPickupId = pickupInfo.mPickupId - parentPickupIdDelta;
	}
	if(infoBits & E_FirePickupBits_GroupId)
	{
		p_Msg.ReadU16(pickupGroupId);
	}
	if(infoBits & E_FirePickupBits_OverrideMatrix)
	{
		pickupInfo.mOverrideMatrix = true;
		p_Msg.ReadBuffer(&pickupInfo.mMatrix, sizeof(pickupInfo.mMatrix));
	}
	if(infoBits & E_FirePickupBits_Value1Valid)
		p_Msg.ReadF32(pickupInfo.mVal);
	if(infoBits & E_FirePickupBits_Value2Valid)
		p_Msg.ReadF32(pickupInfo.mVal2);
	if(infoBits & E_FirePickupBits_Value3Valid)
		p_Msg.ReadU32(pickupInfo.mVal3);

	pickupInfo.mDelay	= F32(F32(scaledDelay) / pickupDelayScaler);
	pickupInfo.mGroupPickupId = pickupGroupId;	//	Put the U16 back into the U32 - it's fine :)
	pickupInfo.mDisableVehicleFX	= (infoBits & E_FirePickupBits_DisableFX) ? true : false;
	pickupInfo.mHostIsController	= (infoBits & E_FirePickupBits_HostControls) ? true : false;

	pickupInfo.mPickupSubType = (E_PICKUP_SUB_TYPE)(typeAndSubType & 0x0F);
	pickupInfo.mPickupType = (E_PICKUP_TYPE)((typeAndSubType >> 4) & 0x0F);
}

void C_NetEventManager::Add_PickupKilled( U32 pickupUid , const Game::PickupAutopsyReport& autopsyReport )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_PickupKilled);

	temp.GetEventBuffer()->PushU32(pickupUid);

	// Before sending convert local join IDs to net join ids
	Game::PickupAutopsyReport autopsyReportNetIDs = autopsyReport; 
	autopsyReportNetIDs.mTargettedVehicleId = Game::GetJoinIdFromLocalIndex(autopsyReport.mTargettedVehicleId);
	autopsyReportNetIDs.mVehicleIdHit = Game::GetJoinIdFromLocalIndex(autopsyReport.mVehicleIdHit);

	temp.GetEventBuffer()->Push((U8*)&autopsyReportNetIDs, sizeof(autopsyReportNetIDs));

	m_EventList.PushBack(temp);
}


void C_NetEventManager::Add_PickupPostDelay( U32 pickupUid, Maths::HMat43 targetMatrix, float fVal )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_PickupPostDelay);

	temp.GetEventBuffer()->PushU32(pickupUid);
	temp.GetEventBuffer()->Push((U8*)&targetMatrix, sizeof(targetMatrix));
	temp.GetEventBuffer()->PushF32(fVal);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_PickupDroppedOnTrack( U32 uniqueId, U8 pickupType, U8 p_JoinId, Maths::HMat43& positionMatrix )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_PickupDroppedOnTrack);

	temp.GetEventBuffer()->PushU32(uniqueId);
	temp.GetEventBuffer()->PushU8(pickupType);
	temp.GetEventBuffer()->PushU8(p_JoinId);
	temp.GetEventBuffer()->Push((U8*)&positionMatrix, sizeof(positionMatrix));

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_DecoyDroppedOnTrack( U32 uniqueId, U8 pickupType, U8 p_JoinId, Maths::HMat43& positionMatrix )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_DecoyDroppedOnTrack);

	temp.GetEventBuffer()->PushU32(uniqueId);
	temp.GetEventBuffer()->PushU8(pickupType);
	temp.GetEventBuffer()->PushU8(p_JoinId);
	temp.GetEventBuffer()->Push((U8*)&positionMatrix, sizeof(positionMatrix));

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_HitByPickup( bool judged, U8 p_HitJoinId, U8 p_FiredJoinId, E_PICKUP_TYPE p_PickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE p_HitType, E_TARGETTING_DIRECTION hitDir, U32 p_PickupUid, U32 pickupGroupId, PickupHitInfo hitInfo )
{
	C_NetEventPacket temp(GetNextEventNum());
	E_NetEventType hitType = (judged) ? E_NetEventType_JudgedPowerupHit : E_NetEventType_HitByPickup;
	temp.GetEventBuffer()->PushU8(hitType);

	U8 pickupTypeAndSubType = (p_PickupType << 4) | subType;
	U8 hitTypeAndDirection = (p_HitType << 4) | hitDir;

	temp.GetEventBuffer()->PushU8(p_HitJoinId);
	temp.GetEventBuffer()->PushU8(p_FiredJoinId);
	temp.GetEventBuffer()->PushU8(pickupTypeAndSubType);
	temp.GetEventBuffer()->PushU8(hitTypeAndDirection);
	temp.GetEventBuffer()->PushU32(p_PickupUid);
	temp.GetEventBuffer()->PushU16((U16)pickupGroupId);	// No way we'll use up a U16
	temp.GetEventBuffer()->Push((U8*)&hitInfo.mPosition, sizeof(float)*3);
	temp.GetEventBuffer()->Push((U8*)&hitInfo.mDirection, sizeof(float)*3);

	m_EventList.PushBack(temp);

	return;
}

void C_NetEventManager::Read_HitByPickup( RacerNet::C_NetMessage &p_Msg, U8& p_HitJoinId, U8& p_FiredJoinId, E_PICKUP_TYPE& p_PickupType, E_PICKUP_SUB_TYPE& subType, E_PICKUP_HIT_TYPE& p_HitType, E_TARGETTING_DIRECTION& hitDir, U32& p_PickupUid, U32& pickupGroupId, PickupHitInfo& hitInfo )
{
	U8 pickupTypeAndSubType, hitTypeAndDirection;
	U16 groupId;

	p_Msg.ReadU8(p_HitJoinId);
	p_Msg.ReadU8(p_FiredJoinId);
	p_Msg.ReadU8(pickupTypeAndSubType);
	p_Msg.ReadU8(hitTypeAndDirection);
	p_Msg.ReadU32(p_PickupUid);
	p_Msg.ReadU16(groupId);
	p_Msg.ReadBuffer(&hitInfo.mPosition, sizeof(float)*3);
	p_Msg.ReadBuffer(&hitInfo.mDirection, sizeof(float)*3);

	subType = (E_PICKUP_SUB_TYPE)(pickupTypeAndSubType & 0x0F);
	p_PickupType = (E_PICKUP_TYPE)((pickupTypeAndSubType >> 4) & 0x0F);

	hitDir = (E_TARGETTING_DIRECTION)(hitTypeAndDirection & 0x0F);
	p_HitType = (E_PICKUP_HIT_TYPE)((hitTypeAndDirection >> 4) & 0x0F);

	pickupGroupId = groupId; // Cast U16 back into U32
}

void C_NetEventManager::Add_Ghosted(U8 p_JoinId, F32 p_GhostTime)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_Ghosted);

	temp.GetEventBuffer()->PushU8(p_JoinId);
	temp.GetEventBuffer()->PushF32(p_GhostTime);

	m_EventList.PushBack(temp);

	return;
}

void C_NetEventManager::Add_Idle(U8 p_JoinId, bool p_Idle)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_Idle);

	temp.GetEventBuffer()->PushU8(p_JoinId);
	temp.GetEventBuffer()->PushU8(p_Idle);

	m_EventList.PushBack(temp);

	return;
}

void C_NetEventManager::Add_MineMoved( U8 p_DropperId, U32 p_MineId, F32 p_PosX, F32 p_PosY, F32 p_PosZ )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_MineMoved);

	temp.GetEventBuffer()->PushU8(p_DropperId);
	temp.GetEventBuffer()->PushU32(p_MineId);
	temp.GetEventBuffer()->PushF32(p_PosX);
	temp.GetEventBuffer()->PushF32(p_PosY);
	temp.GetEventBuffer()->PushF32(p_PosZ);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_MineDestroyedByProjectile( E_PICKUP_TYPE pickupKillerType, E_PICKUP_SUB_TYPE pickupKillerSubType, U8 pickupKillerOwnerID, U32 pickupKillerActorID, E_PICKUP_TYPE pickupVictimType, E_PICKUP_SUB_TYPE pickupVictimSubType, U8 pickupVictimOwnerID, U32 pickupVictimActorID )
{
	U8 killerTypeAndSubType = (pickupKillerType << 4) | pickupKillerSubType;
	U8 victimTypeAndSubType = (pickupVictimType << 4) | pickupVictimSubType;

	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_MineDestroyedByProjectile);

	temp.GetEventBuffer()->PushU8(killerTypeAndSubType);
	temp.GetEventBuffer()->PushU8(pickupKillerOwnerID);
	temp.GetEventBuffer()->PushU32(pickupKillerActorID);

	temp.GetEventBuffer()->PushU8(victimTypeAndSubType);
	temp.GetEventBuffer()->PushU8(pickupVictimOwnerID);
	temp.GetEventBuffer()->PushU32(pickupVictimActorID);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Read_MineDestroyedByProjectile( RacerNet::C_NetMessage &p_Msg, E_PICKUP_TYPE& pickupKillerType, E_PICKUP_SUB_TYPE& pickupKillerSubType, U8& pickupKillerOwnerID, U32& pickupKillerActorID, E_PICKUP_TYPE& pickupVictimType, E_PICKUP_SUB_TYPE& pickupVictimSubType, U8& pickupVictimOwnerID, U32& pickupVictimActorID )
{
	U8 killerTypeAndSubType, victimTypeAndSubType;

	p_Msg.ReadU8(killerTypeAndSubType);
	p_Msg.ReadU8(pickupKillerOwnerID);
	p_Msg.ReadU32(pickupKillerActorID);

	p_Msg.ReadU8(victimTypeAndSubType);
	p_Msg.ReadU8(pickupVictimOwnerID);
	p_Msg.ReadU32(pickupVictimActorID);

	pickupKillerSubType = (E_PICKUP_SUB_TYPE)(killerTypeAndSubType & 0x0F);
	pickupKillerType = (E_PICKUP_TYPE)((killerTypeAndSubType >> 4) & 0x0F);

	pickupVictimSubType = (E_PICKUP_SUB_TYPE)(victimTypeAndSubType & 0x0F);
	pickupVictimType = (E_PICKUP_TYPE)((victimTypeAndSubType >> 4) & 0x0F);
}

void C_NetEventManager::Add_Eliminated(NetVehicleIndexStack& eliminatedNetJoinIds)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_Eliminated);

	temp.GetEventBuffer()->PushU8(eliminatedNetJoinIds.Size());
	for(int i = 0; i < eliminatedNetJoinIds.Size(); i++)
	{
		temp.GetEventBuffer()->PushU8(eliminatedNetJoinIds[i]);
	}

	m_EventList.PushBack(temp);
	
	return;
}

void C_NetEventManager::Add_Repaired( U8 p_JoinId )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_Repaired);

	temp.GetEventBuffer()->PushU8(p_JoinId);

	m_EventList.PushBack(temp);

	return;
}

void C_NetEventManager::Add_ShieldedPickup( U8 p_ShieldedJoin, U8 p_FiredJoinId, E_PICKUP_TYPE p_PickupType, U32 p_PickupUid )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_ShieldedPickup);

	temp.GetEventBuffer()->PushU8(p_ShieldedJoin);
	temp.GetEventBuffer()->PushU8(p_FiredJoinId);
	temp.GetEventBuffer()->PushU8(p_PickupType);
	temp.GetEventBuffer()->PushU32(p_PickupUid);

	m_EventList.PushBack(temp);

	return;
}

void C_NetEventManager::Add_AggressiveAction( U8 p_AggressorId, U8 p_VictimId, Components::BlurScoreMoveSummary& summary )
{
	U8 scoreBonus = (U8)summary.mBonus;
	U8 scoreBonusLevel = (U8)summary.mBonusLevel;
	U8 scorePacked = (scoreBonusLevel << 5) | scoreBonus;
	U8 scoreBase  = (U8)summary.mBase;

	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_Score_Aggressive);

	temp.GetEventBuffer()->PushU8(p_AggressorId);
	temp.GetEventBuffer()->PushU8(p_VictimId);
	temp.GetEventBuffer()->PushU8(scorePacked);
	temp.GetEventBuffer()->PushU8(scoreBase);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Read_AggressiveAction( RacerNet::C_NetMessage &p_Msg, U8& p_AggressorId, U8& p_VictimId, Components::BlurScoreMoveSummary& summary )
{
	U8 scorePacked, scoreBonus, scoreBonusLevel, scoreBase;

	p_Msg.ReadU8(p_AggressorId);
	p_Msg.ReadU8(p_VictimId);
	p_Msg.ReadU8(scorePacked);
	p_Msg.ReadU8(scoreBase);

	scoreBonus = (scorePacked & 0x1F);		//	scoreBonus is in bottom 5 bits
	scoreBonusLevel = (scorePacked >> 5);	//	scoreBonusLevel is in top 3 bits

	summary.mBonus		= (E_BLUR_SCORE_MOVE_BONUS)scoreBonus;
	summary.mBonusLevel	= (E_BLUR_SCORE_MOVE_BONUS_LEVEL)scoreBonusLevel;
	summary.mBase		= (E_BLUR_SCORE_MOVE_BASE)scoreBase;
}

void C_NetEventManager::Add_DamageDealt( U8 damagerId, U8 damagedId, S16 damage )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_DamageDealt);

	temp.GetEventBuffer()->PushU8(damagerId);
	temp.GetEventBuffer()->PushU8(damagedId);
	temp.GetEventBuffer()->PushS16(damage);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_AbilityActivated( U8 p_JoinIdFirer, U8 p_JoinIdVictim, UI::E_MultiplayerVehicleAbilities ability, bool alternateFX )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_AbilityActivated);

	U8 abilityAndAlternateFlag = (U8)ability;
	if(alternateFX)
		abilityAndAlternateFlag |= 1 << 7;

	temp.GetEventBuffer()->PushU8(p_JoinIdFirer);
	temp.GetEventBuffer()->PushU8(p_JoinIdVictim);
	temp.GetEventBuffer()->PushU8(abilityAndAlternateFlag);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_JudgedPowerupShielded( U8 shieldedJoinId, U8 firingJoinId, E_PICKUP_TYPE pickupType, U32 pickupId )
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_JudgedPowerupShielded);

	temp.GetEventBuffer()->PushU8(shieldedJoinId);
	temp.GetEventBuffer()->PushU8(firingJoinId);
	temp.GetEventBuffer()->PushU8((U8)pickupType);
	temp.GetEventBuffer()->PushU32(pickupId);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_EvadedShockEvent(U8 p_EvaderJoinId, U8 p_firerJoinId)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_EvadedShock);

	temp.GetEventBuffer()->PushU8(p_EvaderJoinId);
	temp.GetEventBuffer()->PushU8(p_firerJoinId);
	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_HitAllShockEvent(U8 p_firerJoinId)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_HitAllShockDomes);

	temp.GetEventBuffer()->PushU8(p_firerJoinId);
	m_EventList.PushBack(temp);
}

void C_NetEventManager::Add_SilentRunningEvent(U8 vehicleId, bool cloakOn)
{
	C_NetEventPacket temp(GetNextEventNum());
	temp.GetEventBuffer()->PushU8(E_NetEventType_SilentRunning);

	U8 joinId = Game::GetJoinIdFromLocalIndex(vehicleId);

	temp.GetEventBuffer()->PushU8(joinId);
	temp.GetEventBuffer()->PushU8(cloakOn ? 1 : 0);

	m_EventList.PushBack(temp);
}

void C_NetEventManager::WriteEventsIntoMessage(U16 p_StartMessageNum, RacerNet::C_NetMessage &p_Msg, U8 p_NumberOfEvents)
{
	// Just a safety check to avoid trying to iterate through an empty list
	if(m_EventList.Empty())
	{
		p_Msg.WriteU8(0);
		return;
	}
	//	Write the number of events that need to be processed at the other end
	p_Msg.WriteU8(p_NumberOfEvents);

	EventList::Iterator it = FindEventNumber(p_StartMessageNum);
	if(p_NumberOfEvents > 0)
	{
		//	We only want to send a given number of events in this message
		for(int i = 0; i < p_NumberOfEvents; i++)
		{
			if(it != m_EventList.End()) // Just to be doubly-sure we don't try to read over the end of the list (shouldn't happen though)
			{
				it->ParseIntoNetMsg(p_Msg);
				++it;
			}
		}
	}
}

EventList::Iterator C_NetEventManager::FindEventNumber(U16 p_MessageNum)
{
	bool foundEvent = false;
	EventList::Iterator it;
	for(it = m_EventList.Begin(); it != m_EventList.End(); ++it)
	{
		if(it->GetMessageNumber() == p_MessageNum)
		{
			foundEvent = true;
			break;
		}
	}
	//	If we couldn't find the specified message number, just use the first in the list (this is very unlikely to happen)
	if(!foundEvent)
	{
		it = m_EventList.Begin() + 1;
	}
	return it;
}

} // namespace Game
