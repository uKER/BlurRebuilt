
#include "stdafx.h"

#include "Render/Debug/Charts/StackedBarChartHistory.h"
#include "Display/Buffer/VertexDeclarations.h"


using namespace Render::Chart;

void C_StackedBarChartHistory::RenderChart() const
{
	F32 fLabelOffset = 0.0f;

	BarParameters bar;
	bar.fCumulativeX = 0.0f;
	bar.fCumulativeY = 0.0f;

	if(m_eOrientation == E_ChartOrientation_Horizontal)
	{
		bar.fLabelSize = m_vSize.x * m_fLabelSpacing;
		bar.fBarLength = m_vSize.x * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = m_vSize.y / m_nHistorySize;
	}
	else
	{
		bar.fLabelSize = m_vSize.y * m_fLabelSpacing;
		bar.fBarLength = m_vSize.y * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = m_vSize.x / m_nHistorySize;
	}

	void* pBuffer = Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, m_Series.Size() * 6 * m_nHistorySize, sizeof(Display::Vertex_XY_C), Display::Vertex_XY_C::m_pDeclaration, true); // This can fail
	
	if(pBuffer)
	{
		C_SeriesHistory* pLastUpdate = (C_SeriesHistory*)m_Series[m_iLastUpdate];
		U32 i = pLastUpdate->iWrap;

		for(U32 ll = 0; ll < m_nHistorySize; ++ll, ++i)
		{
			if(i == m_nHistorySize)
				i = 0;

			for(S32 b = 0; b < m_Series.Size(); ++b)
			{
				C_SeriesHistory* pSeries = (C_SeriesHistory*)m_Series[b];
				// get the value as a number between 0 and 1 relative to the chart's axis
				bar.fValue = GetValueRelative(pSeries->fValues[i]);

				pBuffer = DrawBar(this, pBuffer, bar, pSeries->colour);

				if(m_eOrientation == E_ChartOrientation_Horizontal)
					bar.fCumulativeX += bar.fValue * bar.fBarLength;
				else
					bar.fCumulativeY += bar.fValue * bar.fBarLength;
			}
			
			if(m_eOrientation == E_ChartOrientation_Horizontal)
			{
				bar.fCumulativeX = 0.0f;
				bar.fCumulativeY += bar.fBarWidth;
			}
			else
			{
				bar.fCumulativeX += bar.fBarWidth;
				bar.fCumulativeY = 0.0f;
			}
		}
		Display::Device()->EndVertices(NULL); // TODO: Pass in end of written vertex data
	}
}
