#ifndef RENDER_CHART1D_INTERFACE_H
#define RENDER_CHART1D_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ChartInterface.h"

namespace Render
{
namespace Chart
{

class I_Chart1D : public I_Chart
{
public:
	I_Chart1D() : m_fMin(0.0f), m_fMax(1.0f), m_fGridSpacing(0.1f) {};
	virtual ~I_Chart1D() {};

	void			SetAxisRange(F32 fMin, F32 fMax);
	void			SetUnits(const Util::C_String& units);
	virtual void	SetValue(S32 i, F32 fValue) = 0;
	void			SetGridSpacing(F32 fGridSpacing) {m_fGridSpacing = fGridSpacing;}

	const Util::C_String&		GetUnits() const { return m_Units; }

protected:

	virtual void RenderGrid() const;

	F32				GetValueRelative(F32 fValue) const;
	F32				m_fMin;
	F32				m_fMax;
	Util::C_String	m_Units;
	F32				m_fGridSpacing;
};

FORCE_INLINE void I_Chart1D::SetAxisRange(F32 fMin, F32 fMax)
{
	m_fMin = fMin;
	m_fMax = fMax;
}

FORCE_INLINE void I_Chart1D::SetUnits(const Util::C_String& units)
{
	m_Units = units;
}

FORCE_INLINE F32 I_Chart1D::GetValueRelative(F32 fValue) const
{
	fValue = (fValue - m_fMin)/(m_fMax - m_fMin);
	//if(m_eOrientation == E_ChartOrientation_Vertical)
	//	fValue = 1.0f - fValue;
	return Maths::Clamped(fValue, 0.0f, 1.0f);
}

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART1D_INTERFACE_H
