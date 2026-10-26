//=============================================================================
// Graph.cpp
// 30/11/09
//=============================================================================
//=============================================================================
// Includes
//=============================================================================
#include "stdafx.h"
#include "Kanzen_PCH.h"
#include <System/Debug/I_DebugRender.h>	// [Blur2PC] DbUtil::DefaultDebugRenderSys
#include "Util/Graph.h"

//=============================================================================
// Class Implementation
//=============================================================================
namespace Kanzen
{
	//=========================================================================
	// GraphData
	//=========================================================================
	//-------------------------------------------------------------------------
	// Size
	//-------------------------------------------------------------------------
	int 
	GraphData::Size(
	)
	{
		return s_graphDataElements;
	}

	//-------------------------------------------------------------------------
	// GraphData
	//-------------------------------------------------------------------------
	GraphData::GraphData(
	)
	{
		m_visible = true;
		m_index = 0;
		for(int i=0; i<Size(); i++)
		{
			m_data[i] = 0.0f;
		}
	}

	//-------------------------------------------------------------------------
	// SetValue
	//-------------------------------------------------------------------------
	void
	GraphData::SetValue(
		F32 val
	)
	{
		m_data[m_index] = val;
		m_index++;
		m_index = m_index % Size();
	}

	//-------------------------------------------------------------------------
	// GetValue
	//-------------------------------------------------------------------------
	F32 
	GraphData::GetValue(
		int index
	)
	{
		BIZ_ASSERT(index<Size());

		int actualIndex = (m_index - index);
		actualIndex = actualIndex < 0 ? actualIndex + Size():actualIndex; 
		return m_data[ actualIndex ];
	}


	//=========================================================================
	// GraphRenderer
	//=========================================================================
	//-------------------------------------------------------------------------
	// RenderYAxisLine
	//-------------------------------------------------------------------------
	void 
	GraphRenderer::RenderYAxisLine( 
		Graph& graph, 
		F32 yPos 
	)
	{
		DbUtil::I_DebugRenderPtr dr = DbUtil::DefaultDebugRenderSys()->Deferred();

		// Calculate percentage of yPos in our yAxis range;
		F32 normalisedAxisPos = (yPos - graph.m_yAxisMin) / (graph.m_yAxisMax - graph.m_yAxisMin);

		// Only render the axis line if it falls within our axis range
		if( normalisedAxisPos >= 0.0f && normalisedAxisPos <= 1.0f )
		{
			// Draw Horizontal Line
			F32 absYPos = graph.GetStartY() + (graph.m_height * normalisedAxisPos);

			dr->Draw2dLine( Maths::HVec2(graph.GetStartX(),absYPos), Maths::HVec2(graph.GetEndX(),absYPos), Maths::RGBA(0,255,0,255));
		}
	}

	//-------------------------------------------------------------------------
	// RenderXAxisLine
	//-------------------------------------------------------------------------
	void 
	GraphRenderer::RenderXAxisLine( 
		Graph& graph, 
		F32 xPos 
		)
	{
		DbUtil::I_DebugRenderPtr dr = DbUtil::DefaultDebugRenderSys()->Deferred();

		// Calculate percentage of yPos in our yAxis range;
		F32 normalisedAxisPos = (xPos - graph.m_xPos) / (graph.m_width);

		// Only render the axis line if it falls within our axis range
		if( normalisedAxisPos >= 0.0f && normalisedAxisPos <= 1.0f )
		{
			// Draw Horizontal Line
			F32 absXPos = graph.GetStartX() + (graph.m_width * normalisedAxisPos);

			dr->Draw2dLine( Maths::HVec2(absXPos,graph.GetStartY()), Maths::HVec2(absXPos,graph.GetEndY()), Maths::RGBA(0,255,0,255));
		}
	}

	//-------------------------------------------------------------------------
	// RenderAxis
	//-------------------------------------------------------------------------
	void 
	GraphRenderer::RenderAxis( 
		Graph& graph 
	)
	{
		DbUtil::I_DebugRenderPtr dr = DbUtil::DefaultDebugRenderSys()->Deferred();

		// Draw Horizontal Axis
		dr->Draw2dLine( Maths::HVec2(graph.GetStartX(),graph.GetEndY()), Maths::HVec2(graph.GetEndX(),graph.GetEndY()), Maths::RGBA(0,255,0,255));

		// Draw Vertical Axis
		dr->Draw2dLine( Maths::HVec2(graph.GetStartX(),graph.GetStartY()), Maths::HVec2(graph.GetStartX(),graph.GetEndY()), Maths::RGBA(0,255,0,255));
	}


	//-------------------------------------------------------------------------
	// RenderData
	//-------------------------------------------------------------------------
	void 
	GraphRenderer::RenderData( 
		Graph& graph, 
		GraphData& data, 
		Maths::RGBA& col 
	)
	{

		if(data.m_visible)
		{

			DbUtil::I_DebugRenderPtr dr = DbUtil::DefaultDebugRenderSys()->Deferred();

			F32 xStep = graph.m_width / (F32)data.Size();
			F32 xPos = graph.GetStartX();
			F32 yPos = graph.GetStartX();

			F32 prevValue = data.GetValue(0);
			F32 y1 = (prevValue - graph.m_yAxisMin) / (graph.m_yAxisMax - graph.m_yAxisMin);
			y1 = yPos + ((1.0f -y1) * graph.m_height);
			for(int i=1;i<data.Size();i++)
			{
				F32 nextValue = data.GetValue(i);


				F32 y2 = (nextValue - graph.m_yAxisMin) / (graph.m_yAxisMax - graph.m_yAxisMin);
				y2 = yPos + ((1.0f-y2) * graph.m_height);

				if( y1 >= yPos && y1 <= yPos + graph.m_height &&
					y2 >= yPos && y2 <= yPos + graph.m_height)
				{
					// Draw graph section
					dr->Draw2dLine( Maths::HVec2(xPos,y1), Maths::HVec2(xPos+xStep,y2), col);
				}

				xPos += xStep;
				y1 = y2;
				prevValue = nextValue;
			}
		}
	}
}
 
