/////////////////////////////////////////////////////////////
// Curve
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Utils/Curve.h"
#include "Vehicle/Db/VehicleXt.h"

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

/* 
===============================================================
					class C_HermiteCurve

NOTES: Curve class
===============================================================
*/

/*
===============
NAME: C_HermiteCurve

NOTE: Constructor
===============
*/
C_HermiteCurve::C_HermiteCurve()
{
	m_Curve.Clear();
}

/*
===============
NAME: C_HermiteCurve

NOTE: Constructor
===============
*/
C_HermiteCurve::C_HermiteCurve(const XtCurve& curve)
{
	m_Curve.Clear();

	//Add in key frames
	for (S32 i = 0; i < curve.m_KeyFrames.count; i++)
	{
		const XtHermiteKeyFrame& keyFrame = curve.m_KeyFrames[i];

		m_Curve.AddKeyFrame(keyFrame.m_Time,
							keyFrame.m_Value,
							keyFrame.m_InTangent,
							keyFrame.m_OutTangent);
	}
}

} //namespace Vehicle
