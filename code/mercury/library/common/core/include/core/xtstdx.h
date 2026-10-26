#ifndef XT_STDX_H
#define XT_STDX_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtStd.h>
#include <Core/Maths.h>

// ** Load **

// XtVector*->HVec*

inline Maths::HVec2 Load(const XtVec2& v)
{
	return Maths::Load<Maths::HVec2>((const MathsStorage::Float2*)&v);
}

inline Maths::HVec3 Load(const XtVec3& v)
{
	return Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&v);
}

inline Maths::HVec4 Load(const XtVec4& v)
{
	return Maths::Load<Maths::HVec4>((const MathsStorage::Float4*)&v);
}

// XtMat*->HMat*

inline Maths::HMat33 Load(const XtMat33& v)
{
	return Maths::HMat33(
			Load(v.ax),
			Load(v.ay),
			Load(v.az)
		);
}

inline Maths::HMat43 Load(const XtMat43& v)
{
	return Maths::HMat43(
			Load(v.ax),
			Load(v.ay),
			Load(v.az),
			Load(v.aw)
		);
}

inline Maths::HMat44 Load(const XtMat44& v)
{
	return Maths::HMat44(
			Load(v.ax),
			Load(v.ay),
			Load(v.az),
			Load(v.aw)
		);
}

// ** Store **

// HVec*->XtVector*

inline XtVec2 Store(const Maths::HVec2& v)
{
	return XtVec2(v.X(), v.Y());
}

inline XtVec3 Store(const Maths::HVec3& v)
{
	return XtVec3(v.X(), v.Y(), v.Z());
}

inline XtVec4 Store(const Maths::HVec4& v)
{
	return XtVec4(v.X(), v.Y(), v.Z(), v.W());
}

// HMat*->XtMat*

inline XtMat33 Store(const Maths::HMat33& v)
{
	return XtMat33(
			Store(v.AX()),
			Store(v.AY()),
			Store(v.AZ())
		);
}

inline XtMat43 Store(const Maths::HMat43& v)
{
	return XtMat43(
			Store(v.AX()),
			Store(v.AY()),
			Store(v.AZ()),
			Store(v.AW())
		);
}

inline XtMat44 Store(const Maths::HMat44& v)
{
	return XtMat44(
			Store(v.AX()),
			Store(v.AY()),
			Store(v.AZ()),
			Store(v.AW())
		);
}

#endif // XT_STD_H
