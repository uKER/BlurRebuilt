#ifndef RENDER_CHART_BARCHARTINTERFACE_H
#define RENDER_CHART_BARCHARTINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Chart
{

class I_Chart1D;

struct BarParameters
{
	F32 fValue; 
	F32 fLabelSize;
	F32 fBarLength;
	F32 fBarWidth;
	F32 fCumulativeX;
	F32 fCumulativeY;
};

class I_BarChart
{
public:
	I_BarChart();
	~I_BarChart();

	// virtual methods
	void			SetSpacing(F32 fSpacing);

protected:
	void*			DrawBar(const I_Chart1D* pChart, void* pBuffer, const BarParameters& parameters, const Maths::C_RGBA_U8& colour) const;
	void			DrawLabels(const I_Chart1D* pChart, const Util::C_String name, const BarParameters& parameters) const;
	F32				m_fSpacing;
private:
	MathsStorage::Float4 GetBarCoordinates(const I_Chart1D* pChart, const BarParameters& parameters) const;

};

FORCE_INLINE void I_BarChart::SetSpacing(F32 fSpacing)
{
	m_fSpacing = fSpacing;
}

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART_BARCHARTINTERFACE_H
