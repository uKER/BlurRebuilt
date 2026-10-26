
#include "stdafx.h"

#include "Render/Debug/Charts/BarChartInterface.h"
#include "Render/Debug/Charts/Chart1DInterface.h"
#include "Display/Buffer/VertexDeclarations.h"
// use display's font rendering
#include "Display/Render/FontRender.h"

using namespace Render::Chart;

I_BarChart::I_BarChart() : m_fSpacing(0.0f)
{
};

I_BarChart::~I_BarChart()
{
};

MathsStorage::Float4 I_BarChart::GetBarCoordinates(const I_Chart1D* pChart, const BarParameters& parameters) const
{
	MathsStorage::Float4 coordinates;

	if(pChart->GetOrientation() == E_ChartOrientation_Horizontal)
	{
		coordinates.x = parameters.fCumulativeX + pChart->GetPosition().x + parameters.fLabelSize;
		coordinates.z = parameters.fCumulativeX + (parameters.fValue * parameters.fBarLength) + pChart->GetPosition().x + parameters.fLabelSize;
		coordinates.y = parameters.fCumulativeY + pChart->GetPosition().y;
		coordinates.w = parameters.fCumulativeY + parameters.fBarWidth + pChart->GetPosition().y;
	}
	else
	{
		coordinates.x = parameters.fCumulativeX + pChart->GetPosition().x;
		coordinates.z = parameters.fCumulativeX + parameters.fBarWidth + pChart->GetPosition().x;
		coordinates.y = pChart->GetPosition().y + pChart->GetSize().y - (parameters.fValue * parameters.fBarLength) - parameters.fLabelSize - parameters.fCumulativeY;
		coordinates.w = pChart->GetSize().y + pChart->GetPosition().y - parameters.fLabelSize - parameters.fCumulativeY;
	}

	return coordinates;
}

void* I_BarChart::DrawBar(const I_Chart1D* pChart, void* pBuffer, const BarParameters& parameters, const Maths::C_RGBA_U8& colour) const
{
	MathsStorage::Float4 coordinates = GetBarCoordinates(pChart, parameters);
	
	Display::Vertex_XY_C* pVertices = (Display::Vertex_XY_C*)pBuffer;
	
	pVertices->x = coordinates.x;
	pVertices->y = coordinates.y;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	pVertices->x = coordinates.z;
	pVertices->y = coordinates.y;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	pVertices->x = coordinates.z;
	pVertices->y = coordinates.w;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	pVertices->x = coordinates.z;
	pVertices->y = coordinates.w;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	pVertices->x = coordinates.x;
	pVertices->y = coordinates.w;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	pVertices->x = coordinates.x;
	pVertices->y = coordinates.y;
	pVertices->c = Display::GPU_RGBA(colour);
	pVertices++;

	return (void*)pVertices;
}

void I_BarChart::DrawLabels(const I_Chart1D* pChart, const Util::C_String name, const BarParameters& parameters) const
{
	MathsStorage::Float4 coordinates = GetBarCoordinates(pChart, parameters);
	
	Util::C_String valueText;
	valueText.Printf("%.2f", parameters.fValue);
	
	if(pChart->GetUnits().Length() > 0)
		valueText = valueText + pChart->GetUnits();

	if(pChart->GetOrientation() == E_ChartOrientation_Horizontal)
	{
		F32 fFontSize = parameters.fBarWidth * 0.5f;
		if(fFontSize > pChart->GetLabelFontSize())
			fFontSize = pChart->GetLabelFontSize();

		F32 y = coordinates.y + (coordinates.w - coordinates.y - fFontSize) * 0.5f + fFontSize * 0.5f;
		F32 fTextLength = Display::FontRender()->GetSpacing(valueText.Chars(), fFontSize) + fFontSize * 0.5f;

		Display::FontRender()->Draw(valueText.Chars(), coordinates.z - fTextLength, y, fFontSize, fFontSize, Maths::RGBA(255, 255, 255, 255));
		Display::FontRender()->Draw(name, coordinates.x - parameters.fLabelSize, y, fFontSize, fFontSize, Maths::RGBA(255, 255, 255, 255));
	}
	else
	{
		F32 fFontSize = 0.025f;
		if(fFontSize > pChart->GetLabelFontSize())
			fFontSize = pChart->GetLabelFontSize();

		Display::FontRender()->Draw(valueText.Chars(), coordinates.x + fFontSize * 0.25f, coordinates.y - fFontSize * 0.5f, fFontSize, fFontSize, Maths::RGBA(255, 255, 255, 255));
		Display::FontRender()->Draw(name, coordinates.x + fFontSize * 0.25f, coordinates.w - fFontSize * 0.5f + parameters.fLabelSize, fFontSize, fFontSize, Maths::RGBA(255, 255, 255, 255));
	}
}


