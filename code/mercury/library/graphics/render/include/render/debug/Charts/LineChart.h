#ifndef RENDER_CHART_LINE_H
#define RENDER_CHART_LINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ChartHistory1DInterface.h"

namespace Render
{
namespace Chart
{

class C_LineChart : public I_ChartHistory1D
{
public:
	C_LineChart();
	~C_LineChart();

protected:
	virtual void	RenderChart() const;
};

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART_LINE_H
