/////////////////////////////////////////////////////////////
// PerfStats
/////////////////////////////////////////////////////////////

#ifndef __AMAX_PERF_STATS_H__
#define __AMAX_PERF_STATS_H__

#include <xt/xtgame.h>

struct AmaxCity;

namespace Game
{

enum E_AmaxMemType
{
	E_AmaxMemType_Combined,
	E_AmaxMemType_System,
	E_AmaxMemType_Graphics,
};

struct AmaxPlatformAllocation
{
	float						platform360;
	float						platformPS3;
	float						platformPC;
};

struct AmaxMemBudget
{
	XtSz8						renderName;
	XtArray<XtSz8>				trackerName;
	E_AmaxMemType				memType;
	AmaxPlatformAllocation		size;
	bool						dontUse;
};

struct AmaxMemBudgets
{
	XtArray<AmaxMemBudget>		budgets;	
};

struct AmaxRoutePerfStats
{
	int 						routeNumber;
	int							dynObjectsMaxNum;
	int							physObjectsMaxNum;
	XtHandle<AmaxMemBudgets>	memBudgets;
};

struct AmaxCityPerfStats
{	
	AmaxRoutePerfStats			cityDefaults;
	XtArray<AmaxRoutePerfStats>	routeStats;
};

class C_PerfStats
{
public:

	static C_PerfStats& Instance() { return s_Instance; }

	C_PerfStats();
	~C_PerfStats();
	
	void Setup();
	void Shutdown();
	void DisplayOnScreenStats();

private:

	U32 GetAllocatedMemory(Util::C_String p_Name);
	bool GetRouteStats();
	void DisplayStreamer();
	void DisplayObjects();

	static C_PerfStats				s_Instance;

	int								m_RouteNumber;

	XtHandle<AmaxCityPerfStats>		m_XtCityStats;
	AmaxRoutePerfStats*				m_pRouteStats;

	struct StreamerInfo
	{
		int			m_Budget;
		int			m_MaxRequested;
		int			m_MaxLoaded;
		int			m_MaxDiff;
	};
	StreamerInfo					m_StreamerInfo;

	struct ObjectInfo
	{
		int			m_MaxDynamic;
		int			m_MaxDynamicDiff;
		int			m_MaxPhysical;
		int			m_MaxPhysicalDiff;
		int			m_MaxTotal;
		int			m_MaxTotalDiff;
	};
	ObjectInfo						m_ObjectInfo;
	
};

} //namespace Game

#endif
