#ifndef _PERKSKILLUSGAE_AI_C_H
#define _PERKSKILLUSGAE_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Perks/PerkAndSkillUsage/IPerkSkillUsage.h>
#include <ai/AI_AmaxSetup.h>
#include <vector>

namespace Game
{
	class C_PickupBase;

	namespace Actors
	{
		class C_VehicleFirePickupMsg;
		class C_VehicleLogicActor;
	}

}

struct AIPerkPrefsXt;
struct AISkillzXt;
struct AIPerkSkillzXt;
struct AIPerkSkillzParamsXt;


namespace airacingtech
{
		/*
		E_PICKUP_TYPE_NITROUS,
		E_PICKUP_TYPE_SHOCK,
		E_PICKUP_TYPE_SHUNT,
		E_PICKUP_TYPE_BLINDFIRE,
		E_PICKUP_TYPE_BARGE,
		E_PICKUP_TYPE_BOMB


		//secondary pick-ups
		E_PICKUP_TYPE_REPAIR,
		E_PICKUP_TYPE_SHIELD,
		*/

	class IData;
	class IGameServerTime;
	class IAvoidance;
	class IPerkNitro;
	class IPerkShunt;
	class IPerkShock;
	class IPerkBarge;
	class IPerkBolt;
	class IPerkShield ;
	class IPerkMine;
	class IPerkRepair;
	class ISystemAI;
	class IPerkSystem;

	struct TargetEvaluation 
	{
		bool	mFireBackwards;
		float	mEval;
	};

	class PerkPriorities
	{
	public:

		PerkPriorities(int type, float v) : m_Type(type), m_Value(v), m_Eval(FLT_MIN), m_FireBackwards(false)  {};
		~PerkPriorities(){};

		void Reset()
		{
			m_Eval = FLT_MIN;
			m_FireBackwards = false;
		}
		int		m_Type;
		float	m_Value;
		float	m_Eval;
		bool	m_FireBackwards;
	};

	BEGIN_COMPONENT_DEC(CPerkSkillUsage, IPerkSkillUsage)

	public:
	

	


		void PreUpdate();
		void Update();
		void PostUpdate();

		int					GetNumProjectiles()		{ return m_Projectiles.Size(); };
		Game::C_PickupBase*	GetProjectile(int i)	{ return (i >= 0 && i < m_Projectiles.Size() ? m_Projectiles[i] : NULL); };		

		bool				ProjectileWillHitVehicle()		{ return false; };
		bool				ProjectileWillHitProjectile()	{ return false; };

		void				PredictVehiclePos(Game::Actors::C_VehicleActor* pVehActor, Maths::HVec3& predictPos, float lookAheadTime);
		void				PredictProjectilePos(Game::C_PickupBase* pPickup, Maths::HVec3& predictPos, float lookAheadTime);
		float				PredictWillHitTarget(Maths::HVec3 shotPos, Maths::HVec3 shotVel, Maths::HVec3 targetPos, float missDot);

		float				GetTargetDistanceAhead(int idx);
		
		float				GetPerkPreferenceValue(int perkType);
		int					GetPickupStashSize();


		AIPerkPrefsXt	*GetPerkPrefs() {return m_pPerkPrefs;}
		//AISkillzXt		*GetSkillz() {return m_pAISkillz;}
		AIPerkSkillzXt		*GetSkillz() {return m_pAISkillz;}
		
		void			NitroCoolDown();

		void			Reset()
		{
			m_Projectiles.Clear();
			m_PerkPriorities.clear();
		}

		void AddFireMessage(Game::Actors::C_VehicleLogicActor *pLogicActor, Game::Actors::C_VehicleFirePickupMsg *pMessage, bool drop = false)
		{
			m_FireMessages.Add(pLogicActor, pMessage, drop);
		}

		void FireMessages();

		int GetGroup(IData *pIData);

	private:

		IData				*m_pIData;
		IGameServerTime		*m_pIGameServerTime;
		IAvoidance			*m_pIAvoidance;
		ISystemAI			*m_pISystem;
		IPerkSystem			*m_pIPerkSystem;
		AIAmax::AIFireMessages	m_FireMessages;


		IPerkNitro *m_pIPerkNitro;
		IPerkShunt *m_pIPerkShunt;
		IPerkShock *m_pIPerkShock;
		IPerkBarge *m_pIPerkBarge;
		IPerkBolt *m_pIPerkBolt;
		IPerkShield *m_pIPerkShield;
		IPerkMine *m_pIPerkMine;
		IPerkRepair *m_pIPerkRepair;

		int					m_EvalauteAI;
		int					m_IhavePerks[8];
		int					m_EvaluatePerks[8];
		float				m_lastTime;
		bool				m_Update;

		float				m_GeneralFireTimeStamp;
		float				m_GeneralDelay;

		float				m_BoltTimeStamp;
		float				m_BoltDelay;

		float				m_BargeTimeStamp;
		float				m_BargeDelay;

		float				m_fireTimeStamp;
		int					m_fireType;
		float				m_NitroCoolDownTime;

		float				m_MineTimeStamp;
		int					m_BoltCount;
		
		float				m_StateChangeDelay;		
		float				m_StateChangeTime;
	



		int					m_ActivePickCountDebug;


		Btl::Vector<Game::C_PickupBase*>	m_Projectiles;
		

		int	DebugActivePickupCount(){return m_ActivePickCountDebug;}

		float GetMineTimeStamp(){return m_MineTimeStamp;}
		bool ShouldEvaluate();
		void CachedInterfaces();
		void EvaluatePerksIHave();
		void FirePerkMessage(Game::Actors::C_VehicleFirePickupMsg *pMessage, bool drop = false);
		void EvaluateSituation();
		void SelectPerkBasedUponPriority();
		void SelectPerkBasedUponEvaluations();

		void CacheProjectiles();
		void CacheVehicles();
		
		void GeneralFirePerk(int perkType, bool backwards);

		// Seperated into individual functions so specific code can be included, rather than a void Fire(int type) function
		void FireBlindFire(bool backwards);
		void FireShunt(bool backwards);
		void FireBarge();
		void FireNitro();
		void FireShock(bool drop = false);
		void FireMine(bool backwards);
		void FireShield();
		void FireRepair();
		
		// Seperated into individual functions so specific code can be included, rather than a void Test(int type) function
		void TestBlindFire();
		void TestShunt();
		void TestBarge();
		void TestNitro();
		void TestShock();
		void TestMine();
		void TestShield();
		void TestRepair();


		void EvaluateBolt();
		void EvaluateShunt();
		void EvaluateShock();
		void EvaluateNitro();
		void EvaluateMine();
		void EvaluateBarge();
		void EvaluateShield();
		void EvaluateRepair();

		void SwitchToPickup(int type);

		void SortPerksInPriorityOrder();
		PerkPriorities *GetPerkPriorities(int perkType);
		bool KeepOneSlotFreeRule();
		void SelectLeastFavouritePerkBasedUponEvaluations();
		void LogInfoPerks();

		// Car sorting comparison functors
		struct SortPerksByPriority
		{
			bool operator ()(const PerkPriorities Pri1, const PerkPriorities Pri2)
			{
				return (Pri1.m_Value > Pri2.m_Value);
			}
		};

		int				m_PerkPrefsTemplateIndex;
		int				m_SkillzTemplateIndex;
		
		AIPerkPrefsXt	*m_pPerkPrefs;
		AIPerkSkillzXt		*m_pAISkillz;
		std::vector<PerkPriorities>	m_PerkPriorities;
		


	END_COMPONENT_DEC
}



#endif //_PERKSKILLUSGAE_AI_C_H
