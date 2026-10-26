#ifndef RENDER_CHART_INTERFACE_H
#define RENDER_CHART_INTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Chart
{

class C_ChartRenderer;

enum E_ChartSpace
{
	E_ChartSpace_Relative,
	E_ChartSpace_Absolute
};

enum E_ChartOrientation
{
	E_ChartOrientation_Horizontal,
	E_ChartOrientation_Vertical
};

class I_Series
{
public:
	I_Series(): bShow(true) {};
	virtual ~I_Series() {};

	Util::C_String							name;
	Maths::C_RGBA_U8						colour;
	bool									bShow;
};

class I_Chart
{
public:
	I_Chart();
	virtual ~I_Chart();

	// virtual methods to be implemented
	virtual U32		AddSeries(const Util::C_String& name, const Maths::C_RGBA_U8& colour) = 0;
	virtual void	RemoveSeries(S32 i);
	virtual void	Update() {};	// not many charts needs to update
	// normal methods
	void			Render() const;
	void			RenderDeferred() const;
	
	void			SetPosition(F32 x, F32 y); 
	void			SetSize(F32 x, F32 y);
	void			SetSpace(E_ChartSpace eSpace);
	void			SetOrientation(E_ChartOrientation eOrientation);
	void			SetAnnotated(bool bAnnotated, F32 fLabelSpacing, F32 fLabelFontSize);
	void			SetShowBorder(bool bShowBorder);
	void			SetShowGrid(bool bShowGrid) {m_bShowGrid = bShowGrid;}
	void			SetSeriesColour(S32 i, const Maths::C_RGBA_U8& colour);

	// Series hiding currently only implemented for C_LineChart.
	void			SetShowSeries(S32 i, bool bShow) {m_Series[i]->bShow = bShow;}

	const MathsStorage::Float2&		GetPosition() const { return m_vPosition; }
	const MathsStorage::Float2&		GetSize() const { return m_vSize; }
	E_ChartOrientation				GetOrientation() const { return m_eOrientation; }
	F32								GetLabelFontSize() const { return m_fLabelFontSize; }
	F32								GetLabelSpacing() const { return m_fLabelSpacing; }
	S32								GetSeriesCount() const { return m_Series.Size(); }

	friend class C_ChartRenderer;

protected:

	void RenderBorder() const;
	virtual void RenderGrid() const {}
	virtual void RenderChart() const = 0;

	MathsStorage::Float2							m_vPosition;
	MathsStorage::Float2							m_vSize;
	E_ChartSpace									m_eSpace;
	E_ChartOrientation								m_eOrientation;
	F32												m_fLabelSpacing;
	F32												m_fLabelFontSize;
	bool											m_bAnnotated;
	bool											m_bShowBorder;
	bool											m_bShowGrid;
	Ut::UtVector<Ut::DynamicVector<I_Series*> >		m_Series;
};

FORCE_INLINE void I_Chart::SetPosition(F32 x, F32 y)
{
	m_vPosition = MathsStorage::Float2(x, y);
}

FORCE_INLINE void I_Chart::SetSize(F32 x, F32 y)
{
	m_vSize = MathsStorage::Float2(x, y);
}

FORCE_INLINE void I_Chart::SetSpace(E_ChartSpace eSpace)
{
	m_eSpace = eSpace;
}

FORCE_INLINE void I_Chart::SetOrientation(E_ChartOrientation eOrientation)
{
	m_eOrientation = eOrientation;
}

FORCE_INLINE void I_Chart::SetAnnotated(bool bAnnotated, F32 fLabelSpacing, F32 fLabelFontSize)
{
	m_fLabelSpacing = fLabelSpacing;
	m_fLabelFontSize = fLabelFontSize;
	m_bAnnotated = bAnnotated;
}

FORCE_INLINE void I_Chart::SetShowBorder(bool bShowBorder)
{
	m_bShowBorder = bShowBorder;
}

FORCE_INLINE void I_Chart::SetSeriesColour(S32 i, const Maths::C_RGBA_U8& colour)
{
	m_Series[i]->colour = colour;
}

} // namespace Chart
} // namespace Render

#endif // RENDER_CHART_INTERFACE_H
