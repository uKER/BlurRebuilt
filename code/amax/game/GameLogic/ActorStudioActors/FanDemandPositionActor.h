#ifndef FANDEMANDPOSITIONACTOR_H
#define FANDEMANDPOSITIONACTOR_H


#include <actor_system/ActorSys.h>
#include <Game/ActorDef/ActorTypes.h>
#include <Game/ActorDef/GameActor.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedActorMessages.h>

#include <GameLogic/ActorStudioActors/ResourceTypes.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>
#include <GameLogic\ActorStudioActors\PickupPositionActor.h>
#include <GameLogic\ActorStudioActors\TriggerRegionActor.h>
#include <GameLogic\ActorStudioActors\EffectGeneratorActor.h>

#include <GameLogic\RaceLogicCore\Data\RLEnums.h>

//=========================================================================
//						SPAWNER ACTOR CLASSES
//=========================================================================


namespace Actors
{
//=========================================================================

enum FANDEMANDMOVE_TYPE
{
	FANDEMANDMOVE_TYPE_TRIGGER_BOX,
	FANDEMANDMOVE_TYPE_SCORE,
	FANDEMANDMOVE_TYPE_OVERTAKE,
	FANDEMANDMOVE_TYPE_JUMP,
	FANDEMANDMOVE_TYPE_SPEED,
	FANDEMANDMOVE_TYPE_DRIFTING,
	FANDEMANDMOVE_TYPE_PICKUPS,
	FANDEMANDMOVE_TYPE_CARNAGE,
	FANDEMANDMOVE_TYPE_DAMAGE,
	FANDEMANDMOVE_TYPE_MISC,
};

struct FanDemandMoveBase
{
	FANDEMANDMOVE_TYPE type;

	XtArray<const Actors::TriggerRegionCreationData*> do_within;
};

enum FANDEMANDMOVE_TRIGGERBOX_TYPE
{
	FANDEMANDMOVE_TRIGGERBOX_TYPE_ENTER,
	FANDEMANDMOVE_TRIGGERBOX_TYPE_EXIT,
	FANDEMANDMOVE_TRIGGERBOX_TYPE_DONT_ENTER,
	FANDEMANDMOVE_TRIGGERBOX_TYPE_IN_SEQUENCE,
};

enum FANDEMANDMOVE_SPEED_TYPE
{
	FANDEMANDMOVE_SPEED_TYPE_GET_TO,
	FANDEMANDMOVE_SPEED_TYPE_STAY_AT,
};

enum FANDEMANDMOVE_DRIFTING_TYPE
{
	FANDEMANDMOVE_DRIFTING_TYPE_NORMAL,
	FANDEMANDMOVE_DRIFTING_TYPE_HANDBRAKE,
};

enum FANDEMANDMOVE_PICKUPS_TYPE
{
	FANDEMANDMOVE_PICKUPS_TYPE_USE,
	FANDEMANDMOVE_PICKUPS_TYPE_PICKUP,
	FANDEMANDMOVE_PICKUPS_TYPE_HIT,

	FANDEMANDMOVE_PICKUPS_TYPE_USE_DOUBLE,
	FANDEMANDMOVE_PICKUPS_TYPE_PICKUP_DOUBLE,
	FANDEMANDMOVE_PICKUPS_TYPE_HIT_DOUBLE,

	FANDEMANDMOVE_PICKUPS_TYPE_DONT_USE,
	FANDEMANDMOVE_PICKUPS_TYPE_DONT_PICKUP,
	FANDEMANDMOVE_PICKUPS_TYPE_DONT_HIT,
};

enum FANDEMANDMOVE_DAMAGE_TYPE
{
	FANDEMANDMOVE_DAMAGE_TYPE_STAY_BELOW,
	FANDEMANDMOVE_DAMAGE_TYPE_DO_OVER,
};

enum FANDEMANDMOVE_MISC_TYPE
{
	FANDEMANDMOVE_MISC_TYPE_CROSS_LINE_BACKWARD,
	FANDEMANDMOVE_MISC_TYPE_360,
};

struct FanDemandMoveTriggerBox : FanDemandMoveBase
{
	FanDemandMoveTriggerBox(){type = FANDEMANDMOVE_TYPE_TRIGGER_BOX;};

	FANDEMANDMOVE_TRIGGERBOX_TYPE usage;
	const Actors::TriggerRegionCreationData* trigger_box;

	float time_out;
	int sequence_range_from;
	int sequence_range_to;
};

struct FanDemandMoveScore : FanDemandMoveBase
{
	FanDemandMoveScore(){type = FANDEMANDMOVE_TYPE_SCORE;};

	AMAX_SCORING_EVENT score;
	int number;

	E_BLUR_SCORE_MOVE_BASE base_move;						//E_BLUR_SCORE_MOVE_BASE
	XtArray< E_BLUR_SCORE_MOVE_SKILL > skills;				//E_BLUR_SCORE_MOVE_SKILL
	XtArray< E_BLUR_SCORE_MOVE_BONUS > bonuses;				//E_BLUR_SCORE_MOVE_BONUS
	int number_needed;
};

struct FanDemandMoveOvertake : FanDemandMoveBase
{
	FanDemandMoveOvertake(){type = FANDEMANDMOVE_TYPE_OVERTAKE;};

	int number;
};

struct FanDemandMoveJump : FanDemandMoveBase
{
	FanDemandMoveJump(){type = FANDEMANDMOVE_TYPE_JUMP;};

	int number;
	float distance;
};

struct FanDemandMoveSpeed : FanDemandMoveBase
{
	FanDemandMoveSpeed(){type = FANDEMANDMOVE_TYPE_SPEED;};

	FANDEMANDMOVE_SPEED_TYPE usage;
	float time;
	float mph;
};

struct FanDemandMoveDrifting : FanDemandMoveBase
{
	FanDemandMoveDrifting(){type = FANDEMANDMOVE_TYPE_DRIFTING;};

	FANDEMANDMOVE_DRIFTING_TYPE usage;
	float distance;
	float time;
};

struct FanDemandMovePickups : FanDemandMoveBase
{
	FanDemandMovePickups(){type = FANDEMANDMOVE_TYPE_PICKUPS;};

	FANDEMANDMOVE_PICKUPS_TYPE usage;
	E_PICKUP_TYPE pickup_type;
	
	int amount;
};

struct FanDemandMoveCarnage : FanDemandMoveBase
{
	FanDemandMoveCarnage(){type = FANDEMANDMOVE_TYPE_CARNAGE;};

	int number;
};

struct FanDemandMoveDamage : FanDemandMoveBase
{
	FanDemandMoveDamage(){type = FANDEMANDMOVE_TYPE_DAMAGE;};

	FANDEMANDMOVE_DAMAGE_TYPE usage;
	float amount;
};

struct FanDemandMoveMisc : FanDemandMoveBase
{
	FanDemandMoveMisc(){type = FANDEMANDMOVE_TYPE_MISC;};

	FANDEMANDMOVE_MISC_TYPE usage;
};


struct FanDemandGate
{
	Actors::TriggerRegionCreationData*		trigger;

	Actors::TriggerRegionCreationData*		fail_trigger_left;
	Actors::TriggerRegionCreationData*		fail_trigger_right;

	Actors::EffectGeneratorCreationData*	gate_marker_left;
	Actors::EffectGeneratorCreationData*	gate_marker_right;

	int										num_fans_awarded;
};

struct FanDemand
{
	XtLocId				string;
	int					fans;
	float				time_to_complete;

	bool				gold_demand;
	bool				use_aggressive_icon;
	int					fan_bonus_on_completion;

	bool				gift_slot1;
	E_PICKUP_TYPE		slot1;
	bool				gift_slot2;
	E_PICKUP_TYPE		slot2;
	bool				gift_slot3;
	E_PICKUP_TYPE		slot3;
	bool				refill_slots;

	bool				start_deactivated;

	const Actors::PickupPositionCreate*						chained_demand;
	XtArray<const Actors::PickupPositionCreate*>			reset_demands;

	XtArray<const Actors::TriggerRegionCreationData*> 		fail_trigger_zones;
	XtArray<FanDemandGate> 									gates;
	XtArray<const Actors::EffectGeneratorCreationData*>		demand_markers;

	int					number_to_do;
	bool				do_all_on_same_frame;
	bool				do_in_order;
	XtArray<FanDemandMoveBase *> moves;
};

//=========================================================================



class FanDemandPositionCreationData : public PickupPositionCreate
{
public:
	static Actor::C_ActorTypeId			ClassType()		{return Game::E_ActorTypeId_FanDemandPosition;}
	const Actor::C_ActorTypeId			GetType()		const {return Game::E_ActorTypeId_FanDemandPosition;}

	FanDemand mFanDemand;
};


//=========================================================================


class FanDemandPositionActor : public PickupPositionActor
{
public:
	static const Actor::C_ActorTypeId	TypeId()		{ return Game::E_ActorTypeId_FanDemandPosition; }
	static const Btl::String32			TypeName()		{ return "Fan Demand Position"; }


	FanDemandPositionActor();
	~FanDemandPositionActor();

	const FanDemandPositionCreationData		*pCreationData;
	const FanDemandPositionCreationData		*GetCreationData() const {return static_cast<const FanDemandPositionCreationData*>(C_Actor::GetCreationData().GetData());}

protected:
	virtual bool						InitialiseImpl();
	virtual bool						CreateImpl();

private:

};


}// End namespace Actors

#endif // FANDEMANDPOSITIONACTOR_H
