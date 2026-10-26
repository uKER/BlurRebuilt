#ifndef RENDER_CHARTDEFAULT1D_INTERFACE_H
#define RENDER_CHARTDEFAULT1D_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Chart1DInterface.h"

namespace Render
{
namespace Chart
{

class C_Series : public I_Series
{
public:
	C_Series() {};
	virtual ~C_Series() {};

	F32		fValue;
};

class I_ChartDefault1D : public I_Chart1D
{
public:
	I_ChartDefault1D() {};
	virtual ~I_ChartDefault1D() {};

	virtual U32		AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour);
	virtual void	SetValue(S32 i, F32 value);

protected:
};

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART1D_INTERFACE_H
