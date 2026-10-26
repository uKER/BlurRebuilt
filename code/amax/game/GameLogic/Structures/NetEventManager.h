#ifndef NET_EVENT_MANAGER_H
#define NET_EVENT_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\MultiplayerUtilities.h>
#include <Network\Message\NetMessage.h>
#include <GameLogic\Pickups\PickupsManager.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>
namespace Components
{
	struct BlurScoreMoveSummary;
}

static const int MAX_NET_EVENT_LIST_SIZE = 256;

namespace Game
{

//	Forward declarations
class C_MemChunk;
struct PickupHitInfo;
struct PickupAutopsyReport;

class C_NetEventPacket
{
public:
	C_NetEventPacket(U32 p_MessageNum);
	C_NetEventPacket(const C_NetEventPacket& c);
	~C_NetEventPacket();

	C_NetEventPacket& operator =(const C_NetEventPacket& o);

	U16											GetMessageNumber() const	{ return m_MessageNumber; }
	C_MemChunk*									GetEventBuffer() const		{ return m_pEventBuffer; }
	
	void										ParseIntoNetMsg(RacerNet::C_NetMessage& p_Msg) const;

private:
	C_MemChunk*									m_pEventBuffer;
	U16											m_MessageNumber;
};

typedef Btl::List<C_NetEventPacket, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter, Btl::ListElementSingle, Btl::StaticPool<Btl::NullType, MAX_NET_EVENT_LIST_SIZE> > > EventList;

class C_NetEventManager
{
public:
	C_NetEventManager();
	~C_NetEventManager() { Clear(); }

//	Add net events
	void										Add_DroppedPickupPickedUp(U32 uniqueId);
	void										Add_PickupDroppedOnTrack(U32 uniqueID, U8 pickupType, U8 p_JoinId, Maths::HMat43& positionMatrix);
	void										Add_DecoyDroppedOnTrack(U32 uniqueID, U8 pickupType, U8 p_JoinId, Maths::HMat43& positionMatrix);
	//	Net event for a Pickup being fired (activated, so in the case of Pickups with an initial delay this is at the start of the delay timer)
	void										Add_RegisterShocksWithHost(U8 p_FirerId, U8 p_TargetId, U32 uniqueID);
	void										Add_PickupFired(U8 p_FirerId, U8 p_TargetId, PickupCreationStruct& pickupInfo);
	void										Add_PickupKilled(U32 pickupUid, const Game::PickupAutopsyReport& autopsyReport);
	void										Add_PickupPostDelay(U32 pickupUid, Maths::HMat43 targetMatrix, float fVal = 0.f);
	//	Net event for a vehicle being hit by an offensive Pickup
	void										Add_HitByPickup(bool judged, U8 p_HitJoinId, U8 p_FiredJoinId, E_PICKUP_TYPE p_PickupType, E_PICKUP_SUB_TYPE subType, E_PICKUP_HIT_TYPE p_HitType, E_TARGETTING_DIRECTION hitDir, U32 p_PickupUid, U32 pickupGroupId, PickupHitInfo hitInfo);
	void										Add_Ghosted(U8 p_JoinId, F32 p_GhostTime);
	void										Add_Idle(U8 p_JoinId, bool p_Idle);
	void										Add_MineMoved(U8 p_DropperId, U32 p_MineId, F32 p_PosX, F32 p_PosY, F32 p_PosZ);
	void										Add_Eliminated(NetVehicleIndexStack& eliminatedNetJoinIds);
	void										Add_Repaired(U8 p_JoinId);
	void										Add_ShieldedPickup( U8 p_ShieldedJoin, U8 p_FiredJoinId, E_PICKUP_TYPE p_PickupType, U32 p_PickupUid);
	void										Add_AggressiveAction(U8 p_AggressorId, U8 p_VictimId, Components::BlurScoreMoveSummary& summary );
	void										Add_SetPowerupType(U32 powerupId, U8 type, bool isNext);
	void										Add_DamageDealt(U8 damagerId, U8 damagedId, S16 damage);
	void										Add_AbilityActivated(U8 p_JoinIdFirer, U8 p_JoinIdVictim, UI::E_MultiplayerVehicleAbilities ability, bool alternateFX);
	void										Add_JudgedPowerupShielded(U8 shieldedJoinId, U8 firingJoinId, E_PICKUP_TYPE pickupType, U32 pickupId);
	void										Add_MineDestroyedByProjectile(E_PICKUP_TYPE pickupKillerType, E_PICKUP_SUB_TYPE pickupKillerSubType, U8 pickupKillerOwnerID, U32 pickupKillerActorID, E_PICKUP_TYPE pickupVictimType, E_PICKUP_SUB_TYPE pickupVictimSubType, U8 pickupVictimOwnerID, U32 pickupVictimActorID);
	void										Add_SetPowerupRespawnDelay(U32 powerupId, U8 lastJoinIDCollectedBy,  F32 respawnDelayMultiplier);
	void										Add_EvadedShockEvent(U8 p_EvaderJoinId, U8 p_firerJoinId);
	void										Add_HitAllShockEvent(U8 p_firerJoinId);
	void										Add_SilentRunningEvent(U8 vehicleId, bool cloakOn);

	void										WriteEventsIntoMessage(U16 p_StartMessageNum, RacerNet::C_NetMessage &p_Msg, U8 p_NumberOfEvents = 0);

//	Read net events
	void										Read_PickupFired(RacerNet::C_NetMessage &p_Msg, U8& p_FirerId, U8& p_TargetId, PickupCreationStruct& pickupInfo);
	void										Read_HitByPickup(RacerNet::C_NetMessage &p_Msg, U8& p_HitJoinId, U8& p_FiredJoinId, E_PICKUP_TYPE& p_PickupType, E_PICKUP_SUB_TYPE& subType, E_PICKUP_HIT_TYPE& p_HitType, E_TARGETTING_DIRECTION& hitDir, U32& p_PickupUid, U32& pickupGroupId, PickupHitInfo& hitInfo);
	void										Read_AggressiveAction(RacerNet::C_NetMessage &p_Msg, U8& p_AggressorId, U8& p_VictimId, Components::BlurScoreMoveSummary& summary);
	void										Read_MineDestroyedByProjectile(RacerNet::C_NetMessage &p_Msg, E_PICKUP_TYPE& pickupKillerType, E_PICKUP_SUB_TYPE& pickupKillerSubType, U8& pickupKillerOwnerID, U32& pickupKillerActorID, E_PICKUP_TYPE& pickupVictimType, E_PICKUP_SUB_TYPE& pickupVictimSubType, U8& pickupVictimOwnerID, U32& pickupVictimActorID);

	void										Clear();

private:
	bool										IsListFull();
	void										FreeOneSlot();
	EventList::Iterator							FindEventNumber(U16 p_MessageNum);
	U16											GetNextEventNum();
	
	EventList									m_EventList;

};

} // namespace Game



#endif // NET_EVENT_MANAGER_H
