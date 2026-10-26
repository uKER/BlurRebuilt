
#include "stdafx.h"

#include "Render/Debug/Charts/LineChart.h"
#include "Display/Buffer/VertexDeclarations.h"

using namespace Render::Chart;

C_LineChart::C_LineChart()
{
};

C_LineChart::~C_LineChart()
{
};

void C_LineChart::RenderChart() const
{
	F32 fWidthPerValue = m_vSize.x/(F32)m_nHistorySize;
	
	for(S32 l = 0; l < m_Series.Size(); ++l)
	{
		if (!m_Series[l]->bShow)
			continue;

		C_SeriesHistory* pLine = (C_SeriesHistory*)m_Series[l];

		Display::Vertex_XY_C* pVertex = (Display::Vertex_XY_C*)Display::Device()->BeginVertices(Display::E_PrimitiveType_LineStrip, m_nHistorySize, sizeof(Display::Vertex_XY_C), Display::Vertex_XY_C::m_pDeclaration, true); // This can fail
		
		if(pVertex)
		{
			U32 i = pLine->iWrap;
			for(S32 v = 0; v <  pLine->fValues.Size(); ++v, ++i) 
			{
				if(i == m_nHistorySize)
					i = 0;

				// get the value as a number between 0 and 1 relative to the chart's axis
				F32 fValue = GetValueRelative(pLine->fValues[i] * pLine->fScale);

				pVertex->x = m_vPosition.x + (v * fWidthPerValue);
				pVertex->y = m_vPosition.y + m_vSize.y - (fValue * m_vSize.y);
				pVertex->c = Display::GPU_RGBA(pLine->colour);
				pVertex++;
			}

			Display::Device()->EndVertices(pVertex);
		}
	}
}
