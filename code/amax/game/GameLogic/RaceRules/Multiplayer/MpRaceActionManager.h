#ifndef MP_RACE_ACTION_MANAGER
#define MP_RACE_ACTION_MANAGER

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\RaceRules\Multiplayer\MpRaceActionBase.h>

namespace Game
{

class C_MpRaceActionManager
{
public:

	C_MpRaceActionManager() {};
	~C_MpRaceActionManager() {};

	//	Singleton accessor method
	static C_MpRaceActionManager&	Instance()
	{
		static C_MpRaceActionManager s_Instance;
		return s_Instance;
	}

	void										ClearActions();
	void										Update();
	void										AddRaceAction(C_MpRaceActionBase* p_Action);

	Btl::Vector<C_MpRaceActionBase*, Btl::VectorParameters_Dynamic<> >			m_RaceActions;
};

} // namespace Game


#endif // MP_RACE_ACTION_MANAGER