
#include "stdafx.h"

#include "Render/Debug/Charts/StackedBarChart.h"
#include "Display/Buffer/VertexDeclarations.h"

// use display's font rendering
#include "Display/Render/FontRender.h"

using namespace Render::Chart;

void C_StackedBarChart::RenderChart() const
{
	F32 fLabelOffset = 0.0f;

	BarParameters bar;
	bar.fCumulativeX = 0.0f;
	bar.fCumulativeY = 0.0f;

	if(m_eOrientation == E_ChartOrientation_Horizontal)
	{
		bar.fLabelSize = m_vSize.x * m_fLabelSpacing;
		bar.fBarLength = m_vSize.x * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = m_vSize.y;
	}
	else
	{
		bar.fLabelSize = m_vSize.y * m_fLabelSpacing;
		bar.fBarLength = m_vSize.y * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = m_vSize.x;
	}

	void* pBuffer = Display::Device()->BeginVertices(Display::E_PrimitiveType_TriangleList, m_Series.Size() * 6, sizeof(Display::Vertex_XY_C), Display::Vertex_XY_C::m_pDeclaration, true); // This can fail
	
	if(pBuffer)
	{
		for(S32 b = 0; b < m_Series.Size(); ++b)
		{
			C_Series* pSeries = (C_Series*)m_Series[b];
			// get the value as a number between 0 and 1 relative to the chart's axis
			bar.fValue = GetValueRelative(pSeries->fValue);

			pBuffer = DrawBar(this, pBuffer, bar, pSeries->colour);

			if(m_eOrientation == E_ChartOrientation_Horizontal)
				bar.fCumulativeX += bar.fValue * bar.fBarLength;
			else
				bar.fCumulativeY += bar.fValue * bar.fBarLength;
		}

		Display::Device()->EndVertices(NULL); // // TODO: Pass in end of written vertex data
	}
}
