#ifndef RENDER_CHART_STACKEDBARCHART_H
#define RENDER_CHART_STACKEDBARCHART_H

#ifdef _MSC_VER
#pragma once
#endif

#include "BarChartInterface.h"
#include "ChartDefault1DInterface.h"

namespace Render
{
namespace Chart
{

class C_StackedBarChart : public I_BarChart, public I_ChartDefault1D
{
public:
	C_StackedBarChart() {};
	virtual ~C_StackedBarChart() {};

protected:
	// virtual methods
	virtual void	RenderChart() const;

private:
};

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART_STACKEDBARCHART_H
