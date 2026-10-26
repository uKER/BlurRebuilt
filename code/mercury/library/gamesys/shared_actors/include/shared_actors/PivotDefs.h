
#ifndef _PIVOTDEF_H
#define _PIVOTDEF_H

#include <xt/XtGame.h>



namespace BasePhysicalObject
{

	enum E_PivotRotationType
	{
		E_PivotRotationType_XAxis,
		E_PivotRotationType_YAxis,
		E_PivotRotationType_ZAxis,
		E_PivotRotationType_XYAxis,
		E_PivotRotationType_XZAxis,
		E_PivotRotationType_YZAxis,
		E_PivotRotationType_XYZAxis,
		E_PivotRotationType_ExternalSet,
		E_PivotRotationType_Scripted,
		E_PivotRotationType_AutoRotation,
	};


	struct C_PivotDef
	{
		XtSz8					m_Name;
		E_PivotRotationType		m_PivotType;
		XtVec3					m_MinRotation;
		XtVec3					m_MaxRotation;
		F32						m_AccelScale;
		F32						m_VelScale;
		F32						m_GravScale;
		F32						m_AutoRotationRPM;
	};


	struct C_PivotInfo
	{
		Maths::HVec3	m_LocalVel;
		Maths::HVec3	m_LocalAccel;
		F32				m_TimeStep;
	};


} // namespace BasePhysicalObject


#endif
