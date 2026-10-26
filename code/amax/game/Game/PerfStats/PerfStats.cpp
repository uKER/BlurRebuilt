/////////////////////////////////////////////////////////////
// PerfStats
/////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Game/PerfStats/PerfStats.h"

#include <Framework/shared_layers/DebugLayer.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>

#define RENDERINGDATA_ALLOW_INCLUDE
#include "renderingdata/renderingdata.h"
#include <game_resource/LevelResource.h>
#include <game_resource/LevelSceneManager.h>
#include <render/debug/DebugRender.h>
#include <Render/Font/Font.h>

#include <GameApp.h>
#include <GameLogic/GameData/GameData.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Structures/LevelState.h>

#include <system/Memory.h>

#include "world_render/RenderLevel.h"
#include "game/Game.h"

//#pragma optimize("", off)

Debug::Int g_debugPerfStatsBackingAlpha("PerfStats.BackingAlpha", 128, Debug::Range(0, 255), Debug::Step(1), Debug::Persist());
Debug::Int g_debugPerfStatsFontAlpha("PerfStats.FontAlpha", 128, Debug::Range(0, 255), Debug::Step(1), Debug::Persist());

namespace Game
{

C_PerfStats C_PerfStats::s_Instance;

C_PerfStats::C_PerfStats()
{
	m_RouteNumber	= 0;

	m_pRouteStats	= NULL;
}

C_PerfStats::~C_PerfStats()
{

}

void C_PerfStats::Setup()
{
	AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mCity];
	
	if (!pCity)
		return;

	AmaxRoute* pRoute = C_GameData::Instance().mRoutes[(GAMEDATA_ID)C_Game::GetInstance()->GetLevelState().mRoute];
	
	if (!pRoute)
		return;

	m_XtCityStats = pCity->perfStats;

	m_RouteNumber = pRoute->route_number;

	m_StreamerInfo.m_Budget			= 0;
	m_StreamerInfo.m_MaxRequested	= 0;
	m_StreamerInfo.m_MaxLoaded		= 0;
	m_StreamerInfo.m_MaxDiff		= 0;

	if (GetRouteStats() && m_pRouteStats->memBudgets && C_GameApp::GetInstance()->GetDebugLayer())
	{
		C_GameApp::GetInstance()->GetDebugLayer()->GetMemoryBudgets()->ClearBudgets();
		for (S32 iBudget = 0; iBudget < m_pRouteStats->memBudgets->budgets.count; iBudget++)
		{
			if (m_pRouteStats->memBudgets->budgets[iBudget].dontUse)
				continue;

			Util::C_String renderName	= m_pRouteStats->memBudgets->budgets[iBudget].renderName.Text();
			for (S32 iNode = 0; iNode < m_pRouteStats->memBudgets->budgets[iBudget].trackerName.count; iNode++)
			{
				Util::C_String trackerName	= m_pRouteStats->memBudgets->budgets[iBudget].trackerName[iNode].Text();
#if defined(PLATFORM_PS3)
				S32 budget			= (S32)(m_pRouteStats->memBudgets->budgets[iBudget].size.platformPS3*1024*1024);
#elif defined(PLATFORM_PC)			
				S32 budget			= (S32)(m_pRouteStats->memBudgets->budgets[iBudget].size.platformPC*1024*1024);
#endif
				if (renderName.Lower() == "streamer")
					m_StreamerInfo.m_Budget = budget / (1024*1024);

				S32 memType = 0;
				if (m_pRouteStats->memBudgets->budgets[iBudget].memType == E_AmaxMemType_System)
					memType = Framework::E_MemoryBudget_System;
				else if (m_pRouteStats->memBudgets->budgets[iBudget].memType == E_AmaxMemType_Graphics)
					memType = Framework::E_MemoryBudget_Graphics;
				else
					memType = Framework::E_MemoryBudget_System | Framework::E_MemoryBudget_Graphics;
				C_GameApp::GetInstance()->GetDebugLayer()->GetMemoryBudgets()->AddBudget(trackerName, renderName, budget, memType);
			}
		}
	}

	if (m_StreamerInfo.m_Budget == 0)
		m_StreamerInfo.m_Budget = 160; //HACK!
}

void C_PerfStats::Shutdown()
{
	Framework::C_DebugLayer* pDebugLayer = C_GameApp::GetInstance()->GetDebugLayer();
	if(pDebugLayer)
	{
		pDebugLayer->GetMemoryBudgets()->ClearBudgets();
	}
}

bool C_PerfStats::GetRouteStats()
{
	if (!m_XtCityStats)
		return false;

	m_pRouteStats = &m_XtCityStats->cityDefaults;

	for (S32 iRoute = 0; iRoute < m_XtCityStats->routeStats.count; iRoute++)
	{
		if (m_RouteNumber == m_XtCityStats->routeStats[iRoute].routeNumber)
		{
			m_pRouteStats = &m_XtCityStats->routeStats[iRoute];
			return true;
		}
	}

	return true;
}

Debug::Bool g_debugPerfStatsEnable("PerfStats.Enable", false, Debug::Persist());
Debug::Bool g_debugPerfStatsDisplayStreamerDebug("PerfStats.Streamer.DisplayDebug", false, Debug::Persist());
Debug::Bool g_debugPerfStatsDisplayObjectDebug("PerfStats.Objects.DisplayDebug",	false, Debug::Persist());

void C_PerfStats::DisplayOnScreenStats()
{
	if (!g_debugPerfStatsEnable)
		return;

	if (!GetRouteStats())
		return;

	if (g_debugPerfStatsDisplayStreamerDebug)
		DisplayStreamer();

	if (g_debugPerfStatsDisplayObjectDebug)
		DisplayObjects();
}

Debug::Float g_debugStreamerStatsXPos("PerfStats.Streamer.FontXPos",					570.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(0.001f));
Debug::Float g_debugStreamerStatsYPos("PerfStats.Streamer.FontYPos",					500.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(0.001f));
Debug::Float g_debugStreamerStatsFontSize("PerfStats.Streamer.FontSize",				18.0f, Debug::Range(0.0f, 128.0f));

void C_PerfStats::DisplayStreamer()
{
#ifndef CFG_GOLD
	if (!C_GameApp::GetInstance()->GetDebugLayer())
		return;

	DbUtil::I_DebugRender* pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Immediate();
	Render::Font::C_FontRenderer* pFontRenderer = C_GameApp::GetInstance()->GetDebugLayer()->GetFontRenderer();

	pFontRenderer->SetScaleX(g_debugStreamerStatsFontSize);
	pFontRenderer->SetScaleY(g_debugStreamerStatsFontSize);
	pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, (U32)g_debugPerfStatsFontAlpha));
	pFontRenderer->SetColourBottom(Maths::RGBA(255, 255, 255, (U32)g_debugPerfStatsFontAlpha));

	RenderingData::Target::StreamData::DebugInfoStats stats;
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetLevelResource()->GetRenderLevel()->GetScene().GetStreamData().DebugInfo(stats);
	
	Maths::HVec2 pos((float)g_debugStreamerStatsXPos, (float)g_debugStreamerStatsYPos);
	
	char streamerSpew[256];
	
	S32 sizeRequested	= stats.bytesRequested / (1024*1024);
	S32 sizeLoaded		= stats.bytesLoaded / (1024*1024);
	S32 sizeDiff		= (stats.bytesLoadedAndRequested - stats.bytesRequested) / (1024*1024);

	m_StreamerInfo.m_MaxRequested	= Maths::Max(m_StreamerInfo.m_MaxRequested, sizeRequested);
	m_StreamerInfo.m_MaxLoaded		= Maths::Max(m_StreamerInfo.m_MaxLoaded, sizeLoaded);
	m_StreamerInfo.m_MaxDiff		= Maths::Max(m_StreamerInfo.m_MaxDiff, sizeDiff);

	Maths::RGBA colour(0, 0, 0, (U32)g_debugPerfStatsBackingAlpha);
	
	sprintf_s(streamerSpew, lengthof(streamerSpew), "Streamer  : Current  /  Max");
	Maths::HVec2 dim = pFontRenderer->GetDimensions(0, streamerSpew);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, streamerSpew);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	sprintf_s(streamerSpew, lengthof(streamerSpew), "------------------------------------");
	dim = pFontRenderer->GetDimensions(0, streamerSpew);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, streamerSpew);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	colour = Maths::RGBA(0, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	if (sizeRequested > m_StreamerInfo.m_Budget)
		colour = Maths::RGBA(128, 0, 0, (U32)g_debugPerfStatsBackingAlpha);
	else if (sizeRequested > sizeLoaded)
		colour = Maths::RGBA(128, 128, 0, (U32)g_debugPerfStatsBackingAlpha);

	sprintf_s(streamerSpew, lengthof(streamerSpew), "Requested : %d MB / %d MB", sizeRequested, m_StreamerInfo.m_MaxRequested);
	dim = pFontRenderer->GetDimensions(0, streamerSpew);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, streamerSpew);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	sprintf_s(streamerSpew, lengthof(streamerSpew), "Loaded     : %d MB / %d MB", sizeLoaded, m_StreamerInfo.m_MaxLoaded);
	dim = pFontRenderer->GetDimensions(0, streamerSpew);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, streamerSpew);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	sprintf_s(streamerSpew, lengthof(streamerSpew), "Difference : %d MB / %d MB", sizeDiff, m_StreamerInfo.m_MaxDiff);
	dim = pFontRenderer->GetDimensions(0, streamerSpew);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, streamerSpew);
	pos +=  Maths::HVec2(0.0f, dim.Y());
#endif
}

Debug::Float g_debugObjectDebugXPos("PerfStats.Objects.FontXPos",						570.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(0.001f));
Debug::Float g_debugObjectDebugYPos("PerfStats.Objects.FontYPos",						600.0f, Debug::Range(0.0f, 1280.0f), Debug::Step(0.001f));
Debug::Int g_debugObjectBuffer("PerfStats.Objects.AllowableGrowth",						50, Debug::Range(0, 200), Debug::Step(1));
Debug::Float g_debugObjectStatsFontSize("PerfStats.Objects.sFontSize",				18.0f, Debug::Range(0.0f, 128.0f));

U32 C_PerfStats::GetAllocatedMemory(Util::C_String p_Name)
{
#ifndef CFG_GOLD
	Memory::ContextId id = Memory::TrackNodes::Instance().FindId(p_Name);
	if(id != INVALID_CONTEXT_ID)
	{
		Memory::ContextNode node;
		Memory::Tracker::Instance().GetContextNode(id, node);
		
		return node.Allocated();		
	}
#endif
	return 0;
}

void C_PerfStats::DisplayObjects()
{
#ifndef CFG_GOLD
	if (!C_GameApp::GetInstance()->GetDebugLayer())
		return;

	DbUtil::I_DebugRender* pDebugRenderer = DbUtil::DefaultDebugRenderSys()->Immediate();
	Render::Font::C_FontRenderer* pFontRenderer = C_GameApp::GetInstance()->GetDebugLayer()->GetFontRenderer();

	pFontRenderer->SetScaleX(g_debugObjectStatsFontSize);
	pFontRenderer->SetScaleY(g_debugObjectStatsFontSize);
	pFontRenderer->SetColourTop(Maths::RGBA(255, 255, 255, (U32)g_debugPerfStatsFontAlpha));
	pFontRenderer->SetColourBottom(Maths::RGBA(255, 255, 255, (U32)g_debugPerfStatsFontAlpha));

	Maths::HVec2 pos((float)g_debugObjectDebugXPos, (float)g_debugObjectDebugYPos);
	
	U32 dynMb	= GetAllocatedMemory("actors.dynamicobject") / (1024*1024);
	U32 physMb	= GetAllocatedMemory("actors.physicalobject") / (1024*1024);
	S32 totalMb = dynMb + physMb;

	S32 numPhys			= ::Actors::C_ActorUtils::Instance().m_NumPhysicalObjectActors;
	S32 numPhysOrig		= ::Actors::C_ActorUtils::Instance().m_OriginalNumPhysicalObjectActors;
	S32 physDiff		= numPhys - numPhysOrig;
	S32 numDyn			= ::Actors::C_ActorUtils::Instance().m_NumDynamicObjectActors;
	S32 numDynOrig		= ::Actors::C_ActorUtils::Instance().m_OriginalNumDynamicObjectActors;
	S32 dynDiff			= ::Actors::C_ActorUtils::Instance().m_NumDynamicObjectActors - numDynOrig;
	S32 num				= numPhys+numDyn;
	S32 numOrig			= numPhysOrig+numDynOrig;
	S32 totalDiff		= num - numOrig;
	
	m_ObjectInfo.m_MaxDynamic			= Maths::Max(m_ObjectInfo.m_MaxDynamic,			numDyn);
	m_ObjectInfo.m_MaxDynamicDiff		= Maths::Max(m_ObjectInfo.m_MaxDynamicDiff,		dynDiff);
	m_ObjectInfo.m_MaxPhysical			= Maths::Max(m_ObjectInfo.m_MaxPhysical,		numPhys);
	m_ObjectInfo.m_MaxPhysicalDiff		= Maths::Max(m_ObjectInfo.m_MaxPhysicalDiff,	physDiff);
	m_ObjectInfo.m_MaxTotal				= Maths::Max(m_ObjectInfo.m_MaxPhysical,		numPhys);
	m_ObjectInfo.m_MaxTotalDiff			= Maths::Max(m_ObjectInfo.m_MaxPhysicalDiff,	physDiff);
	
	char objStr[256];
	
	Maths::RGBA colour(0, 0, 0, (U32)g_debugPerfStatsBackingAlpha);

	sprintf_s(objStr, lengthof(objStr), "Objects  : Num / Orig / Diff / Max / Mem");
	Maths::HVec2 dim = pFontRenderer->GetDimensions(0, objStr);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, objStr);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	_snprintf(objStr, lengthof(objStr), "---------------------------------------------------------");
	dim = pFontRenderer->GetDimensions(0, objStr);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, objStr);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	colour = Maths::RGBA(0, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	if (numPhys > numPhysOrig + g_debugObjectBuffer)
		colour = Maths::RGBA(128, 0, 0, (U32)g_debugPerfStatsBackingAlpha);
	else if (numPhys > numPhysOrig)
		colour = Maths::RGBA(128, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	_snprintf(objStr, lengthof(objStr), "Physical  : %d / %d / %d / %d / %d MB", numPhys, numPhysOrig, physDiff, m_ObjectInfo.m_MaxPhysical, physMb);
	dim = pFontRenderer->GetDimensions(0, objStr);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, objStr);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	colour = Maths::RGBA(0, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	if (numDyn > numDynOrig + g_debugObjectBuffer)
		colour = Maths::RGBA(128, 0, 0, (U32)g_debugPerfStatsBackingAlpha);
	else if (numDyn > numDynOrig)
		colour = Maths::RGBA(128, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	_snprintf(objStr, lengthof(objStr), "Dynamic : %d / %d / %d / %d / %d MB", numDyn, numDynOrig, dynDiff, m_ObjectInfo.m_MaxDynamic, dynMb);
	dim = pFontRenderer->GetDimensions(0, objStr);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, objStr);
	pos +=  Maths::HVec2(0.0f, dim.Y());

	colour = Maths::RGBA(0, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	if (num > numOrig + g_debugObjectBuffer)
		colour = Maths::RGBA(128, 0, 0, (U32)g_debugPerfStatsBackingAlpha);
	else if (num > numOrig)
		colour = Maths::RGBA(128, 128, 0, (U32)g_debugPerfStatsBackingAlpha);
	_snprintf(objStr, lengthof(objStr), "Total     : %d / %d / %d / %d / %d MB", num, numOrig, totalDiff, m_ObjectInfo.m_MaxTotal, totalMb);
	dim = pFontRenderer->GetDimensions(0, objStr);
	pDebugRenderer->Draw2dBar(pos, dim, colour);
	pFontRenderer->Print(pos, objStr);
	pos +=  Maths::HVec2(0.0f, dim.Y());
#endif
}

} //namespace Game

