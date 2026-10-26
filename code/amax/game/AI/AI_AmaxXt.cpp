#include "stdafx.h"
#include <AI_AmaxXt.h>
#include <GameApp.h>

AIAmaxXt AIAmaxXt::m_Instance;

#define AI_AMAX_CALLBACK(Type, Member) \
class Callback##Type { public: AmaxGameData<Type*>* operator()() { return &AIAmaxXt::Instance().Member; } };


XT_TYPE(AISpeedXt,"AISpeed")
	XT_FIELD(m_brakingCoefficient,"BrakingCoefficient")
	XT_FIELD(m_frictionCoefficient,"FrictionCoefficient")
XT_END_TYPE

XT_TYPE(AIBehavioursXt,"AIBehaviours")
	XT_FIELD(m_block,"Block")
XT_END_TYPE

XT_TYPE(AIparamsXt,"AIparams")
	XT_FIELD(m_speed,"Speed")
	XT_FIELD(m_behaviours,"Behaviours")
XT_END_TYPE

AI_AMAX_CALLBACK(AIXt, m_AItemplates)
	XT_TYPE(AIXt,"AI")
		XT_CALLBACK((GameDataHandler<AIXt, CallbackAIXt>))
			XT_BASE(AmaxGameDataObj)
			XT_FIELD(contentPack,"ContentPack")
			XT_FIELD(m_params,"PersonalityParams")
	XT_END_TYPE

XT_TYPE(AIPIDXt,"AIPID")
	XT_FIELD(m_P,"P")
	XT_FIELD(m_I,"I")
	XT_FIELD(m_D,"D")
	XT_FIELD(m_Slots,"Slots")
	XT_FIELD(m_TimeStep,"TimeStep")
XT_END_TYPE

XT_TYPE(AIOffsetXt,"AIOffset")
	XT_FIELD(m_UseOffset,"UseOffset")
	XT_FIELD(m_OffsetValue,"UseOffsetValue")
XT_END_TYPE


XT_TYPE(AISteeringParamsXt,"AISteeringParams")
	XT_FIELD(m_PID,"PID")
	XT_FIELD(m_UseVelocity,"UseVelocity")
	XT_FIELD(m_Blend,"Blend")
	XT_FIELD(m_ForwardsDist,"ForwardsDist")
	XT_FIELD(m_AngVelSteering,"AngVelSteering")
	XT_FIELD(m_AlignedSteering,"AlignedSteering")
	XT_FIELD(m_SteeringRate,"SteeringRate")
	XT_FIELD(m_SteeringTimeStep,"SteeringTimeStep")
	XT_FIELD(m_LerpValue,"LerpValue")
	XT_FIELD(m_DelayValue,"DelayValue")
	XT_FIELD(m_SmoothTimeStep,"SmoothTimeStep")
	XT_FIELD(m_AIOffset,"AIOffset")
XT_END_TYPE


AI_AMAX_CALLBACK(AISteeringXt, m_AISteeringParams)
	XT_TYPE(AISteeringXt,"AISteering")
		XT_CALLBACK((GameDataHandler<AISteeringXt, CallbackAISteeringXt>))
			XT_BASE(AmaxGameDataObj)
			XT_FIELD(contentPack,"ContentPack")
			XT_FIELD(m_SteeringParams,"SteeringParams")
	XT_END_TYPE


XT_TYPE(AISkillzParamsXt,"AISkillParams")
	XT_FIELD(m_BackwardFiring,"BackwardFiring")
	XT_FIELD(m_ForwardFiring,"ForwardFiring")
	XT_FIELD(m_Defensive,"Defensive")
	XT_FIELD(m_Offensive,"Offensive")
	XT_FIELD(m_Racing,"Racing")
	XT_FIELD(m_Aggression,"Aggression")
	XT_FIELD(m_Blocking,"Blocking")
	XT_FIELD(m_Slamming,"Slamming")
	XT_FIELD(m_Mistakes,"Mistakes")
XT_END_TYPE

AI_AMAX_CALLBACK(AISkillzXt, m_AISkillz)
	XT_TYPE(AISkillzXt,"AISkillz")
		XT_CALLBACK((GameDataHandler<AISkillzXt, CallbackAISkillzXt>))
		XT_BASE(AmaxGameDataObj)
		XT_FIELD(contentPack,"ContentPack")
		XT_FIELD(m_AISkillzParams,"SkillParams")
	XT_END_TYPE


XT_TYPE(AIPerkPrefsParamsXt,"AIPerkPrefsParams")
	XT_FIELD(m_Nitro,"Nitro")
	XT_FIELD(m_Shock,"Shock")
	XT_FIELD(m_Shunt,"Shunt")
	XT_FIELD(m_BlindFire,"BlindFire")
	XT_FIELD(m_Barge,"Barge")
	XT_FIELD(m_Mine,"Mine")
	XT_FIELD(m_Repair,"Repair")
	XT_FIELD(m_Shield,"Shield")
XT_END_TYPE

AI_AMAX_CALLBACK(AIPerkPrefsXt, m_AIPerkPrefs)
	XT_TYPE(AIPerkPrefsXt,"AIPerkPrefs")
		XT_CALLBACK((GameDataHandler<AIPerkPrefsXt, CallbackAIPerkPrefsXt>))
		XT_BASE(AmaxGameDataObj)
		XT_FIELD(contentPack,"ContentPack")
		XT_FIELD(m_AIPerkPrefsParams,"PerkPrefsParams")
	XT_END_TYPE



///////////////////////////////////////////////////////////////////////////////////
XT_TYPE(AIPerkSkillzParamsXt,"AIPerkSkillzParams")
	XT_FIELD(m_Offensive,"Offensive")
	XT_FIELD(m_Defensive,"Defensive")
	XT_FIELD(m_Avoidance,"Avoidance")
XT_END_TYPE

AI_AMAX_CALLBACK(AIPerkSkillzXt, m_AIPerkSkillz)
	XT_TYPE(AIPerkSkillzXt,"AIPerkSkillz")
		XT_CALLBACK((GameDataHandler<AIPerkSkillzXt, CallbackAIPerkSkillzXt>))
		XT_BASE(AmaxGameDataObj)
		XT_FIELD(contentPack,"ContentPack")
		XT_FIELD(m_Nitro,"Nitro")
		XT_FIELD(m_Shock,"Shock")
		XT_FIELD(m_Shunt,"Shunt")
		XT_FIELD(m_Blindfire,"Blindfire")
		XT_FIELD(m_Barge,"Barge")
		XT_FIELD(m_Mine,"Mine")
		XT_FIELD(m_Repair,"Repair")
		XT_FIELD(m_Shield,"Shield")

		XT_FIELD(m_Racing,"Racing")
		XT_FIELD(m_Aggression,"Aggression")
		XT_FIELD(m_Blocking,"Blocking")
		XT_FIELD(m_Slamming,"Slamming")
		XT_FIELD(m_Mistakes,"Mistakes")

	XT_END_TYPE


XT_LIBRARY(AIR)
	XT_REGISTER_TYPE(AIPerkSkillzParamsXt)
	XT_REGISTER_TYPE(AIPerkSkillzXt)

	XT_REGISTER_TYPE(AISkillzParamsXt)
	XT_REGISTER_TYPE(AISkillzXt)
	XT_REGISTER_TYPE(AIPerkPrefsParamsXt)
	XT_REGISTER_TYPE(AIPerkPrefsXt)
	XT_REGISTER_TYPE(AIPIDXt)
	XT_REGISTER_TYPE(AIOffsetXt)
	XT_REGISTER_TYPE(AISteeringParamsXt)
	XT_REGISTER_TYPE(AISteeringXt)
	XT_REGISTER_TYPE(AISpeedXt)
	XT_REGISTER_TYPE(AIBehavioursXt)
	XT_REGISTER_TYPE(AIparamsXt)
	XT_REGISTER_TYPE(AIXt)
XT_END_LIBRARY
