#include "stdafx.h"
#include <Game/Game.h>	// [Blur2PC]

#include <GameLogic/ActorStudioActors/FanDemandPositionActor.h>

//=========================================================================
//		Sorcery definitions
//=========================================================================


XT_TYPE(Actors::FanDemandMoveBase, "FanDemandMoveBase")
	XT_FIELD(do_within,"DoWithin")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE,							"FanDemandMoveTriggerBoxType")
	XT_OPTION(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_ENTER,				"Enter")
	XT_OPTION(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_EXIT,				"Exit")
	XT_OPTION(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_DONT_ENTER,			"DontEnter")
	XT_OPTION(Actors::FANDEMANDMOVE_TRIGGERBOX_TYPE_IN_SEQUENCE,		"InSequence")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_SPEED_TYPE,								"FanDemandMoveSpeedType")
	XT_OPTION(Actors::FANDEMANDMOVE_SPEED_TYPE_GET_TO,					"GetTo")
	XT_OPTION(Actors::FANDEMANDMOVE_SPEED_TYPE_STAY_AT,					"StayAt")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_DRIFTING_TYPE,							"FanDemandMoveDriftingType")
	XT_OPTION(Actors::FANDEMANDMOVE_DRIFTING_TYPE_NORMAL,				"Normal")
	XT_OPTION(Actors::FANDEMANDMOVE_DRIFTING_TYPE_HANDBRAKE,			"Handbrake")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_PICKUPS_TYPE,								"FanDemandMovePickupsType")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_USE,					"Use")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_PICKUP,				"Pickup")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_HIT,					"Hit")

	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_USE_DOUBLE,			"UseDouble")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_PICKUP_DOUBLE,			"PickupDouble")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_HIT_DOUBLE,			"HitDouble")

	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_USE,				"DontUse")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_PICKUP,			"DontPickup")
	XT_OPTION(Actors::FANDEMANDMOVE_PICKUPS_TYPE_DONT_HIT,				"DontHit")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_DAMAGE_TYPE,								"FanDemandMoveDamageType")
	XT_OPTION(Actors::FANDEMANDMOVE_DAMAGE_TYPE_STAY_BELOW,				"StayBelow")
	XT_OPTION(Actors::FANDEMANDMOVE_DAMAGE_TYPE_DO_OVER,				"DoOver")
XT_END_TYPE

XT_TYPE(Actors::FANDEMANDMOVE_MISC_TYPE,								"FanDemandMoveMiscType")
	XT_OPTION(Actors::FANDEMANDMOVE_MISC_TYPE_CROSS_LINE_BACKWARD,		"CrossLineBackward")
	XT_OPTION(Actors::FANDEMANDMOVE_MISC_TYPE_360,						"ThreeSixty")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveTriggerBox, "FanDemandMoveTriggerBox")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
	XT_FIELD(trigger_box, "TriggerBox")

	XT_FIELD(time_out, "TimeOut")
	XT_FIELD(sequence_range_from,	"SequenceRangeFrom")
	XT_FIELD(sequence_range_to,		"SequenceRangeTo")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveScore, "FanDemandMoveScore")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(score, "Score")
	XT_FIELD(number, "Number")

	XT_FIELD(base_move, "BaseMove")
	XT_FIELD(skills, "Skills")
	XT_FIELD(bonuses, "Bonuses")
	XT_FIELD(number_needed, "NumberNeeded")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveOvertake, "FanDemandMoveOvertake")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(number, "Number")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveJump, "FanDemandMoveJump")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(number, "Number")
	XT_FIELD(distance, "Distance")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveSpeed, "FanDemandMoveSpeed")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
	XT_FIELD(time, "Time")
	XT_FIELD(mph, "MPH")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveDrifting, "FanDemandMoveDrifting")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
	XT_FIELD(distance, "Distance")
	XT_FIELD(time, "Time")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMovePickups, "FanDemandMovePickups")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
	XT_FIELD(pickup_type, "PickupType")
	XT_FIELD(amount, "Amount")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveCarnage, "FanDemandMoveCarnage")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(number, "Number")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveDamage, "FanDemandMoveDamage")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
	XT_FIELD(amount, "Amount")
XT_END_TYPE

XT_TYPE(Actors::FanDemandMoveMisc, "FanDemandMoveMisc")
	XT_BASE(Actors::FanDemandMoveBase)
	XT_FIELD(usage, "Usage")
XT_END_TYPE


XT_TYPE(Actors::FanDemandGate, "FanDemandGate")
	XT_FIELD(trigger,				"Trigger")

	XT_FIELD(fail_trigger_left,		"FailTriggerLeft")
	XT_FIELD(fail_trigger_right,	"FailTriggerRight")

	XT_FIELD(gate_marker_left,		"GateMarkerLeft")
	XT_FIELD(gate_marker_right,		"GateMarkerRight")

	XT_FIELD(num_fans_awarded,		"NumFansAwarded")
XT_END_TYPE

XT_TYPE(Actors::FanDemand, "FanDemand")
	XT_FIELD(string,				"String")
	XT_FIELD(fans,					"Fans")
	XT_FIELD(time_to_complete,		"TimeToComplete")
	XT_FIELD(gold_demand,			"GoldDemand")
	XT_FIELD(use_aggressive_icon,	"UseAggressiveIcon")
	XT_FIELD(fan_bonus_on_completion, "FanBonusOnCompletion")

	XT_FIELD(gift_slot1,			"GiftSlot1")
	XT_FIELD(slot1,					"Slot1")
	XT_FIELD(gift_slot2,			"GiftSlot2")
	XT_FIELD(slot2,					"Slot2")
	XT_FIELD(gift_slot3,			"GiftSlot3")
	XT_FIELD(slot3,					"Slot3")
	XT_FIELD(refill_slots,			"RefillSlotsWhenUsed")

	XT_FIELD(start_deactivated,		"StartDeactivated")
	XT_FIELD(chained_demand,		"ChainedDemand")
	XT_FIELD(reset_demands,			"DemandsResetOnFail")

	XT_FIELD(fail_trigger_zones,	"FailTriggerZones")
	XT_FIELD(gates,					"Gates")

	XT_FIELD(demand_markers,		"DemandMarkers")

	XT_FIELD(number_to_do,			"NumberToDo")
	XT_FIELD(do_all_on_same_frame,	"DoAllOnSameFrame")
	XT_FIELD(do_in_order,			"DoInOrder")
	XT_FIELD(moves,					"Moves")
XT_END_TYPE

//The sorcery definition for the creation data structure
XT_TYPE(Actors::FanDemandPositionCreationData, "FanDemandPosition")
	XT_BASE(Actors::PickupPositionCreate)
	XT_FIELD(mFanDemand, "FanDemandData")
XT_END_TYPE


//=========================================================================
//					EffectGenerator Actor Core Routines
//=========================================================================

namespace Actors
{
//#pragma optimize("", off)


//=========================================================================
//						STANDARD FUNCTIONS
//=========================================================================

FanDemandPositionActor::FanDemandPositionActor()
{ 

}

FanDemandPositionActor::~FanDemandPositionActor()
{

}

bool FanDemandPositionActor::InitialiseImpl()
{
	PickupPositionActor::InitialiseImpl();

	Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
	pLayer->mNumFanPickups++;			

	return(true);
}

//=========================================================================

bool FanDemandPositionActor::CreateImpl()
{
	pCreationData = GetCreationData();

	


	PickupPositionActor::CreateImpl();

	return(true);
}

}
