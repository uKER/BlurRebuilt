/////////////////////////////////////////////////////////////
// Curve
/////////////////////////////////////////////////////////////

#ifndef __VEHICLE_CURVE_H__
#define __VEHICLE_CURVE_H__

#include <misc_shared/Curve/HermiteCurve.h>

namespace Vehicle
{

struct XtCurve;

/* 
===============================================================
					class C_HermiteCurve

NOTES: Curve class
===============================================================
*/
class C_HermiteCurve
{
public:

	//Default - empty
	C_HermiteCurve();

	//Construct from an XtCurve from Sorcery
	C_HermiteCurve(const XtCurve& curve);

	float Evaluate(float p_Time) const { return m_Curve.Valid() ? m_Curve.Evaluate(p_Time) : 0.0f; }

	void Clear() { m_Curve.Clear(); }

private:

	Curve::C_HermiteCurve		m_Curve;
};

} //namespace Vehicle

#endif
