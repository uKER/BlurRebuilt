
#include "stdafx.h"

#include "Render/Debug/Charts/ChartDefault1DInterface.h"

using namespace Render::Chart;

U32 I_ChartDefault1D::AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour)
{
	U32 iIndex = m_Series.Size();

	C_Series* pSeries = new C_Series;

	pSeries->name = name;
	pSeries->colour = colour;
	pSeries->fValue = 0.0f;

	m_Series.PushBack(pSeries);

	return iIndex;
}

void I_ChartDefault1D::SetValue(S32 i, F32 fValue)
{
	C_Series* pSeries = (C_Series*)m_Series[i];
	pSeries->fValue = fValue;
}

