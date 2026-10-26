#ifndef RENDER_CHARTHISTORY1D_INTERFACE_H
#define RENDER_CHARTHISTORY1D_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Chart1DInterface.h"

namespace Render
{
namespace Chart
{

class C_SeriesHistory : public I_Series
{
public:
	C_SeriesHistory(): fScale(1.0f) {};
	virtual ~C_SeriesHistory() {};

	U32										iWrap;
	Ut::UtVector<Ut::DynamicVector<F32> >	fValues;
	F32										fScale;
};

class I_ChartHistory1D : public I_Chart1D
{
public:
	I_ChartHistory1D() : m_iLastUpdate(0), m_bSynchronise(false) { SetHistorySize(32); }
	virtual ~I_ChartHistory1D() {}

	virtual U32		AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour);
	U32				AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour, U32 iWrap);
	virtual void	RemoveSeries(S32 i);
	virtual void	SetValue(S32 i, F32 value);
	void			Update();

	void			SetHistorySize(U32 nHistoryValues);
	void			SetSynchronise(bool bSynchronise) { m_bSynchronise = bSynchronise; }

	U32				GetHistorySize() const { return m_nHistorySize; }

	// Returns the iWrap member of the last series to be updated, or 0 if the chart has no series.
	U32				GetLastWrap() const;

	// Series scaling currently only implemented for C_LineChart.
	void			SetSeriesScale(S32 i, F32 fScale);

protected:
	U32				m_iLastUpdate;
	U32				m_nHistorySize;
	bool			m_bSynchronise;
};

FORCE_INLINE U32 I_ChartHistory1D::GetLastWrap() const
{
	if (m_Series.Size() == 0)
		return 0;
	else
		return static_cast<C_SeriesHistory*>(m_Series[m_iLastUpdate])->iWrap;
}

FORCE_INLINE void I_ChartHistory1D::SetSeriesScale(S32 i, F32 fScale)
{
	static_cast<C_SeriesHistory*>(m_Series[i])->fScale = fScale;
}

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART1D_INTERFACE_H
