
#ifndef UI_ADOBEPATH_H
#define UI_ADOBEPATH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Illustrator/UIAdobeElement.h"
#include "ui/Illustrator/UIAdobePoint.h"

#ifndef CFG_GOLD
extern Debug::Bool g_UiRenderSplineInfo;
#endif


namespace UI
{

/*-----------------------------------------------------------------------------
	Class : C_AdobePath

	Desc :	Animation paths from illustrator.
------------------------------------------------------------------------------*/
class C_AdobePath : public C_AdobeElement
{
private:
	typedef Btl::List< Maths::HVec2 > TCurvePointList;

public:
	C_AdobePath();

	~C_AdobePath();

	Maths::HVec3			GetPositionAt( const F32 distance, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const bool bOffset = false );

	const F32				GetRotationAt( const F32 distance, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const bool bOffset = false );

	const void				AddPoint( const C_AdobePoint& point );

	const void				CalculateLengths();

	const bool				EvaluateSpline( AdobePointList& pointList, const F32 fCurveStep, const bool bClosed );

	const bool				EvaluateSplineFixedSpacing( AdobePointList& pointList, const F32 fSpacing );

	const void				RemovePoints();

	/*-----------------------------------------------------------------------------
		Inline Functions
	------------------------------------------------------------------------------*/
	const Maths::HVec3		GetTopLeft() const	
							{
								return m_Bounds.Min(); 
							}

	const Maths::HVec3		GetCentre()	const	
							{ 
								return m_Bounds.Centre(); 
							}

	const F32				GetLength()	const
							{ 
								return m_TotalLength; 
							}
/*
#ifndef CFG_GOLD // for debug rendering
	void					RenderSplineInfo( const F32 distance, const U8 layer, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const U8 panel, const bool bOffset = false );
#endif
*/
private:
	const Maths::HVec3		GetPointAt( const F32 distance, const Maths::HVec2& offset );

	const F32				GetArcLength( const U32 index, F32 min, F32 max );	

	const F32				SubDivideLength( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3 );

	Maths::HVec2			SubDividePoints( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3,
											 Maths::HVec2& l1, Maths::HVec2& l2, Maths::HVec2& r1, Maths::HVec2& r2 );

	Maths::HVec2			SubDividePoints( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3 );

	const void				SubDivideArc( TCurvePointList& list, const Maths::HVec2 p0, const Maths::HVec2 p1, const Maths::HVec2 p2, const Maths::HVec2 p3, const F32& fSpacing );

	Maths::HVec2			MidPoint( Maths::HVec2& p1, Maths::HVec2& p2 );

public:
	static const U32		m_SplineCurveStep = 32;

	Maths::C_AxisBox		m_Bounds;		///< Currently used to scale the path.
	AdobePointList			m_Path;			///< Spline data.

private:
	F32						m_TotalLength;	///< Length of the curve.
	Maths::HVec3			m_LastPoint;
};

}; // namespace UI

#endif // UI_ADOBEPATH_H



