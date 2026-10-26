#ifndef AI_AMAX_DATA_H
#define AI_AMAX_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/GameData/GameDataDefs.h>

#define INVALID_RUBBERBANDING	999999

struct AISpeedXt
{
	float	m_brakingCoefficient;
	float	m_frictionCoefficient;
};

struct AIBehavioursXt
{
	bool	m_block;
	// etc, etc
};

struct AIparamsXt
{
	AISpeedXt			m_speed;
	AIBehavioursXt		m_behaviours;
};


struct AIXt : AmaxGameDataObj
{
	XtU16				contentPack;
	AIparamsXt			m_params;
};



struct AIOffsetXt
{
	bool	m_UseOffset;
	float	m_OffsetValue;
};

struct AIPIDXt
{
	float	m_P;
	float	m_I;
	float	m_D;
	int		m_Slots;
	float	m_TimeStep;
	
};

struct AISteeringParamsXt
{
	AIPIDXt		m_PID;
	bool		m_UseVelocity;
	float		m_Blend;
	float		m_ForwardsDist;
	float		m_AngVelSteering;
	float		m_AlignedSteering;
	float		m_SteeringRate;
	float		m_SteeringTimeStep;
	float		m_LerpValue;
	float		m_DelayValue;
	float		m_SmoothTimeStep;
	AIOffsetXt	m_AIOffset;
};

struct AISteeringXt  : AmaxGameDataObj
{
	XtU16				contentPack;
	AISteeringParamsXt	m_SteeringParams;
};


struct AISkillzParamsXt
{
	float m_BackwardFiring;
	float m_ForwardFiring;
	float m_Defensive;
	float m_Offensive;
	float m_Racing;
	float m_Aggression;
	float m_Blocking;
	float m_Slamming;
	float m_Mistakes;	
};

struct AISkillzXt  : AmaxGameDataObj
{
	XtU16				contentPack;
	AISkillzParamsXt	m_AISkillzParams;
};


struct AIPerkPrefsParamsXt
{
	float m_Nitro;
	float m_Shock;
	float m_Shunt;
	float m_BlindFire;
	float m_Barge;
	float m_Mine;
	float m_Repair;
	float m_Shield;		
};

struct AIPerkPrefsXt  : AmaxGameDataObj
{
	XtU16				contentPack;
	AIPerkPrefsParamsXt	m_AIPerkPrefsParams;
};


struct AIPerkSkillzParamsXt
{
	float		m_Offensive;
	float		m_Defensive;
	float		m_Avoidance;
};

struct AIPerkSkillzXt : AmaxGameDataObj
{
	XtU16					contentPack;

	AIPerkSkillzParamsXt	m_Nitro;
	AIPerkSkillzParamsXt	m_Shock;
	AIPerkSkillzParamsXt	m_Shunt;
	AIPerkSkillzParamsXt	m_Blindfire;
	AIPerkSkillzParamsXt	m_Barge;
	AIPerkSkillzParamsXt	m_Mine;
	AIPerkSkillzParamsXt	m_Repair;
	AIPerkSkillzParamsXt	m_Shield;

	float					m_Racing;
	float					m_Aggression;
	float					m_Blocking;
	float					m_Slamming;
	float					m_Mistakes;	

};




#endif //AI_AMAX_DATA_H
