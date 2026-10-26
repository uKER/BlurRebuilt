#ifndef SP_BOSSDEMANDS_H
#define SP_BOSSDEMANDS_H


#include <GameLogic/SinglePlayer/SinglePlayer.h>



// #pragma once


enum E_BOSSDEMAND_ACTION
{
	E_BOSSDEMAND_ACTION_NEW_EVENT,					// Reset event counters
	E_BOSSDEMAND_ACTION_EVENT_FINISHED,				// Just crossed line so can read cars/mods used
	E_BOSSDEMAND_ACTION_EVENT_RESULTS_UPDATED,		// Sp event should be updated by now

	E_BOSSDEMAND_ACTION_GOT_FANS,
	E_BOSSDEMAND_ACTION_PLAYER_WRECKED,
	E_BOSSDEMAND_ACTION_WRECKED_ANOTHER,
	E_BOSSDEMAND_ACTION_RAM_INTO_SEA,
	E_BOSSDEMAND_ACTION_BARGE_OFF_CLIFF,
	E_BOSSDEMAND_ACTION_DRIFTED,
	E_BOSSDEMAND_ACTION_AIR_TIME,
	E_BOSSDEMAND_ACTION_SPEED,
	E_BOSSDEMAND_ACTION_USED_POWERUP,
	E_BOSSDEMAND_ACTION_SHOCK_HIT,
	E_BOSSDEMAND_ACTION_BARGE_HIT,
	E_BOSSDEMAND_ACTION_SHUNT_HIT,
	E_BOSSDEMAND_ACTION_REVERSE_SHUNT_HIT,
	E_BOSSDEMAND_ACTION_MINE_HIT,
	E_BOSSDEMAND_ACTION_BOLT_HIT,
	E_BOSSDEMAND_ACTION_FORWARD_MINE_HIT,
	E_BOSSDEMAND_ACTION_MULTIPLE_BARGE_HIT,
	E_BOSSDEMAND_ACTION_3_BOLT_HIT,
	E_BOSSDEMAND_ACTION_REVERSE_BOLT,
	E_BOSSDEMAND_ACTION_COMPLETE_FAN_DEMAND,
	E_BOSSDEMAND_ACTION_COMPLETED_SINGLE_FAN_RUN,
	E_BOSSDEMAND_ACTION_REPAIR,
	E_BOSSDEMAND_ACTION_SHIELD_POWERUPS,
	E_BOSSDEMAND_ACTION_EVADE_SHOCK,
	E_BOSSDEMAND_ACTION_USE_WEAPON_DEFENCE,
	E_BOSSDEMAND_ACTION_SHIELD_MINES,
	E_BOSSDEMAND_ACTION_CLEAN_NITRO,
	E_BOSSDEMAND_ACTION_CLEAN_TRIPLE_NITRO,
	E_BOSSDEMAND_ACTION_NITRO_RAM,
	E_BOSSDEMAND_ACTION_X5_FAN_COMBO,
	E_BOSSDEMAND_ACTION_SECONDS_IN_FIRST,
	E_BOSSDEMAND_ACTION_WIN_WITH_1_HEALTH,
	E_BOSSDEMAND_ACTION_DESTRUCTION_BONUS_HIT,
	E_BOSSDEMAND_ACTION_COLLECT_STOPWATCH,
};

//=========================================================================


class SP_BossDemands
{
public:
	static SP_BossDemands& Instance() { return s_Instance; }
	
	SP_BossDemands();
	~SP_BossDemands();

	void		RegisterBossDemandAction(E_BOSSDEMAND_ACTION type, F32 value = 1.0f);
	void		UpdateBossDemandsForTier(SP_SaveGame* sg, SP_Tier* tier);
	bool		ShouldSendHUDMessage(S32 oldval, S32 newval, S32 target);


private:

	SP_Boss*	GetCurrentBoss();
	void		ResetEventStats();
	void		ModifyBossDemandValue(SP_BossDemandKind kind, F32 value, bool use_max_value);



	static SP_BossDemands				s_Instance;					// Store single instance of this class

	bool			used_shield;
	bool			used_repair;
	bool			has_been_wrecked;
	bool			got_x5_fan_combo;
	bool			finished_with_1_health;

	F32				min_speed;
	F32				min_speed_start_pos;
	F32				min_speed_last_pos;

	S32				num_AI_wrecked;
	S32				target_AI_wrecked;
};


#endif // SP_BOSSDEMANDS_H
