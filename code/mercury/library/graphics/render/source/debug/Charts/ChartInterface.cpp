#include "stdafx.h"

#include "Render/Debug/Charts/ChartInterface.h"
#include "Render/Debug/Charts/ChartRenderer.h"
#include "Display/Buffer/VertexDeclarations.h"

using namespace Render::Chart;

I_Chart::I_Chart() :
	m_vPosition(MathsStorage::Float2(0.0f, 0.0f)), 
	m_vSize(MathsStorage::Float2(1.0f, 1.0f)),
	m_eSpace(E_ChartSpace_Relative),
	m_eOrientation(E_ChartOrientation_Horizontal),
	m_fLabelSpacing(0.0f),
	m_fLabelFontSize(0.0f),
	m_bAnnotated(false),
	m_bShowBorder(false),
	m_bShowGrid(false)
{
};

I_Chart::~I_Chart()
{
	for(S32 i = 0; i < m_Series.Size(); ++i)
	{
		delete m_Series[i];
	}

	m_Series.Clear();
};

void I_Chart::RemoveSeries(S32 i)
{
	delete m_Series[i];
	m_Series.EraseByIndex(i);
}

void I_Chart::Render() const
{
	if(ChartRendererExists())
	{
		ChartRenderer()->Prepare(this);

		if (m_bShowBorder)
			RenderBorder();

		if (m_bShowGrid)
			RenderGrid();

		if (m_Series.Size() > 0)
			RenderChart();
	}	
	else
		LOGWARN(g_Log, "Chart cannot be rendered without an instanciated ChartRenderer\n");
}

void I_Chart::RenderDeferred() const
{
	C_ChartRenderer::Instance().AddDeferredChart(this);
}


void I_Chart::RenderBorder() const
{
	Display::Vertex_XY_C* v = (Display::Vertex_XY_C*)Display::Device()->BeginVertices(
		Display::E_PrimitiveType_LineStrip, 
		5,
		sizeof(Display::Vertex_XY_C),
		Display::Vertex_XY_C::m_pDeclaration,
		true); // This can fail

	if(v)
	{
		U32 white = 0xffffffff;

		v->x = m_vPosition.x;				v->y = m_vPosition.y;				v->c = white;	v++;
		v->x = m_vPosition.x + m_vSize.x;	v->y = m_vPosition.y;				v->c = white;	v++;
		v->x = m_vPosition.x + m_vSize.x;	v->y = m_vPosition.y + m_vSize.y;	v->c = white;	v++;
		v->x = m_vPosition.x;				v->y = m_vPosition.y + m_vSize.y;	v->c = white;	v++;
		v->x = m_vPosition.x;				v->y = m_vPosition.y;				v->c = white;

		Display::Device()->EndVertices(v);
	}
}
