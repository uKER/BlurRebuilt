#include "stdafx.h"

#include "RLDebugLineController.h"
#include "../Data/RLDefinitions.h"

#include "../Core/RaceLogicCore.h"

#include "../Database/RLDatabase.h"

#ifdef RLC_DEBUG
#pragma optimize ("", off) //RLC_DEBUG
#endif

static Debug::Logger &g_RLDebugLineLog = Debug::GetLogger("Amax.RLC.RLDebugLineLog");
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//From RaceLogicCore.cpp
extern Debug::Bool g_Stamp;

Debug::Bool g_RenderSplitLines("RLC.DebugLines.ShowSplitLines", false, Debug::Persist());
Debug::Bool g_RenderStartPositions("RLC.DebugLines.ShowStartPositions", false, Debug::Persist());
Debug::Bool g_RenderMissilePositions("RLC.DebugLines.ShowMissilePosition", true);
Debug::Bool g_RenderMinePositions("RLC.DebugLines.ShowMinePosition", true);
Debug::Bool g_RenderPileUpAreas("RLC.DebugLines.ShowPileUpAreas", false);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////DEBUG VARIABLES//////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace Game
{
namespace Actors
{
RLDebugLineController::RLDebugLineController()
{
	Initialise();
}

RLDebugLineController::~RLDebugLineController()
{
}

void RLDebugLineController::Initialise()
{
	meType = E_RLCOMPONENTTYPE_DEBUGLINE_CONTROLLER;
}

void RLDebugLineController::Update()
{
}

void RLDebugLineController::Render()
{
	for(int lineIndex = 0; lineIndex < m_pRLC->GetDebugLineData()->mLines.Size(); lineIndex++ )
	{
		RLDebugLine *pLine = m_pRLC->GetDebugLineData()->mLines[lineIndex];

		switch(pLine->meType)
		{
		case E_DEBUG_LINE_TYPE_START_POSITION:
			{
				if(!g_RenderStartPositions)
					continue;
				break;
			}
		case E_DEBUG_LINE_TYPE_SPLIT_LINE:
			{
				if(!g_RenderSplitLines)
					continue;
				break;
			}
		case E_DEBUG_LINE_TYPE_PICKUP_POSITION:
			{
				if(!g_RenderPickupPositions)
					continue;
				break;
			}
		}

		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pLine->mStart, pLine->mEnd, pLine->mColour);
	}

	if(g_RenderPileUpAreas)
	{
		for(int i = 0; i < m_pRLC->GetTrackData()->mPileUpData.Size(); i++)
		{
			float radius = m_pRLC->GetRLSystem()->mReportingParameters->mPileUpMinimumDistance; 
			
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCircle(m_pRLC->GetTrackData()->mPileUpData[i].mPileUpCentrePosition, Maths::HVec3(0.f, 1.f, 0.f), radius, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross(m_pRLC->GetTrackData()->mPileUpData[i].mPileUpCentrePosition, 5.f, Maths::RGBA(0x00, 0xff, 0x00, 0xff) );
		}
	}

}


RLDebugLine* RLDebugLineController::AddDebugLine(	const enum E_DEBUG_LINE_TYPE eType, 
													const Maths::HVec3 start, 
													const Maths::HVec3 end, 
													const Maths::RGBA colour)
{
	RLDebugLine* pNewLine = new RLDebugLine;

	pNewLine->meType = eType;
	pNewLine->mColour = colour;
	pNewLine->mStart = start;
	pNewLine->mEnd = end;
	
	m_pRLC->GetDebugLineData()->mLines.PushBack(pNewLine);

	return pNewLine;
}

void RLDebugLineController::RemoveDebugLine( RLDebugLine* pLineToBeRemoved )
{
	//Go through the lines looking for the one we need to remove
	for(int iLine = 0; iLine < m_pRLC->GetDebugLineData()->mLines.Size(); iLine++)
	{
		if(pLineToBeRemoved == m_pRLC->GetDebugLineData()->mLines[iLine])
		{
			//Delete the memory
			delete pLineToBeRemoved;
			//Delete the item from the line array
			m_pRLC->GetDebugLineData()->mLines.EraseByIndex(iLine);
			break;
		}
	}
}

void RLDebugLineController::RemoveDebugLinesByType(const enum E_DEBUG_LINE_TYPE eType)
{
	//Go through the lines looking for the one we need to remove
	for(int iLine = 0; iLine < m_pRLC->GetDebugLineData()->mLines.Size();)
	{
		if(eType == m_pRLC->GetDebugLineData()->mLines[iLine]->meType)
		{
			//Delete the memory
			delete m_pRLC->GetDebugLineData()->mLines[iLine];
			//Delete the item from the line array
			m_pRLC->GetDebugLineData()->mLines.EraseByIndex(iLine);
		}
		else
			 iLine++;
	}
}

void RLDebugLineController::CleanUpDebugLines()
{
	for(int i = 0; i < m_pRLC->GetDebugLineData()->mLines.Size(); i++)
		delete m_pRLC->GetDebugLineData()->mLines[i];

	m_pRLC->GetDebugLineData()->mLines.Clear();
}


};//Actors
};//Game

#ifdef RLC_DEBUG
#pragma optimize ("", on) //RLC_DEBUG
#endif
