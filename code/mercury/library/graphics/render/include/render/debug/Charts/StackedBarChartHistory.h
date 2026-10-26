#ifndef RENDER_CHART_STACKEDBARCHARTHISTORY_H
#define RENDER_CHART_STACKEDBARCHARTHISTORY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "BarChartInterface.h"
#include "ChartHistory1DInterface.h"

namespace Render
{
namespace Chart
{

class C_StackedBarChartHistory : public I_BarChart, public I_ChartHistory1D
{
public:
	C_StackedBarChartHistory() {};
	virtual ~C_StackedBarChartHistory() {};

protected:
	// virtual methods
	virtual void	RenderChart() const;

private:
};

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART_STACKEDBARCHARTHISTORY_H
