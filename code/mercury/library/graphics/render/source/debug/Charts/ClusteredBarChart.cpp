
#include "stdafx.h"

#include "Render/Debug/Charts/ClusteredBarChart.h"

#include "Display/Buffer/VertexDeclarations.h"
// use display's font rendering
#include "Display/Render/FontRender.h"

using namespace Render::Chart;

void C_ClusteredBarChart::RenderChart() const
{
	BarParameters bar;
	bar.fCumulativeX = 0.0f;	// cumulative spacing
	bar.fCumulativeY = 0.0f;	// cumulative spacing

	if(m_eOrientation == E_ChartOrientation_Horizontal)
	{
		bar.fLabelSize = m_vSize.x * m_fLabelSpacing;
		bar.fBarLength = m_vSize.x * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = (m_vSize.y - (m_Series.Size() - 1) * m_fSpacing)/(F32)m_Series.Size();
	}
	else
	{
		bar.fLabelSize = m_vSize.y * m_fLabelSpacing;
		bar.fBarLength = m_vSize.y * (1.0f - m_fLabelSpacing);
		bar.fBarWidth = (m_vSize.x - (m_Series.Size() - 1) * m_fSpacing)/(F32)m_Series.Size();
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
				bar.fCumulativeY += bar.fBarWidth + m_fSpacing;
			else
				bar.fCumulativeX += bar.fBarWidth + m_fSpacing;
		}
		Display::Device()->EndVertices(NULL); // TODO: Pass in end of written vertex data
	}

	// draw labels if needed
	
	if(m_bAnnotated)
	{
		bar.fCumulativeX = 0.0f;
		bar.fCumulativeY = 0.0f;
		for(S32 b = 0; b < m_Series.Size(); ++b)
		{
			C_Series* pSeries = (C_Series*)m_Series[b];
			bar.fValue = GetValueRelative(pSeries->fValue);

			DrawLabels(this, pSeries->name, bar);
		
			if(m_eOrientation == E_ChartOrientation_Horizontal)
				bar.fCumulativeY += bar.fBarWidth + m_fSpacing;
			else
				bar.fCumulativeX += bar.fBarWidth + m_fSpacing;
		}
	}
}
