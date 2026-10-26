#ifndef XT_STD_H
#define XT_STD_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Xt/XtCoreTypes.h"

// Vector

struct XtVec2
{
	float vx, vy;

public:
	inline explicit XtVec2(float xy = 0.0f) :
		vx(xy),
		vy(xy)
	{
	}

	inline XtVec2(float vx_, float vy_) :
		vx(vx_),
		vy(vy_)
	{
	}
};

struct XtVec3
{
	float vx, vy, vz;

public:
	inline explicit XtVec3(float xyz = 0.0f) :
		vx(xyz),
		vy(xyz),
		vz(xyz)
	{
	}

	inline XtVec3(float vx_, float vy_, float vz_) :
		vx(vx_),
		vy(vy_),
		vz(vz_)
	{
	}
};

struct XtVec4
{
	float vx, vy, vz, vw;

public:
	inline explicit XtVec4(float xyzw = 0.0f) :
		vx(xyzw),
		vy(xyzw),
		vz(xyzw),
		vw(xyzw)
	{
	}

	inline XtVec4(float vx_, float vy_, float vz_, float vw_) :
		vx(vx_),
		vy(vy_),
		vz(vz_),
		vw(vw_)
	{
	}
};

// Matrix

struct XtMat22
{
	XtVec2 ax;
	XtVec2 ay;

public:
	inline XtMat22() :
		ax(1.0f, 0.0f),
		ay(0.0f, 1.0f)
	{
	}

	inline XtMat22(const XtVec2& ax_, const XtVec2& ay_) :
		ax(ax_),
		ay(ay_)
	{
	}
};

struct XtMat32
{
	XtVec2 ax;
	XtVec2 ay;
	XtVec2 az;

public:
	inline XtMat32() :
		ax(1.0f, 0.0f),
		ay(0.0f, 1.0f)
	{
	}

	inline XtMat32(const XtVec2& ax_, const XtVec2& ay_, const XtVec2& az_) :
		ax(ax_),
		ay(ay_),
		az(az_)
	{
	}
};

struct XtMat33
{
	XtVec3 ax;
	XtVec3 ay;
	XtVec3 az;

public:
	inline XtMat33() :
		ax(1.0f, 0.0f, 0.0f),
		ay(0.0f, 1.0f, 0.0f),
		az(0.0f, 0.0f, 1.0f)
	{
	}

	inline XtMat33(const XtVec3& ax_, const XtVec3& ay_, const XtVec3& az_) :
		ax(ax_),
		ay(ay_),
		az(az_)
	{
	}
};

struct XtMat43
{
	XtVec3 ax;
	XtVec3 ay;
	XtVec3 az;
	XtVec3 aw;

public:
	inline XtMat43() :
		ax(1.0f, 0.0f, 0.0f),
		ay(0.0f, 1.0f, 0.0f),
		az(0.0f, 0.0f, 1.0f)
	{
	}

	inline XtMat43(const XtVec3& ax_, const XtVec3& ay_, const XtVec3& az_, const XtVec3& aw_) :
		ax(ax_),
		ay(ay_),
		az(az_),
		aw(aw_)
	{
	}
};

struct XtMat44
{
	XtVec4 ax;
	XtVec4 ay;
	XtVec4 az;
	XtVec4 aw;

public:
	inline XtMat44() :
		ax(1.0f, 0.0f, 0.0f, 0.0f),
		ay(0.0f, 1.0f, 0.0f, 0.0f),
		az(0.0f, 0.0f, 1.0f, 0.0f),
		aw(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	inline XtMat44(const XtVec4& ax_, const XtVec4& ay_, const XtVec4& az_, const XtVec4& aw_) :
		ax(ax_),
		ay(ay_),
		az(az_),
		aw(aw_)
	{
	}
};

// Colour

struct XtRgb
{
	float r, g, b;

public:
	inline explicit XtRgb(float rgb = 1.0f) :
		r(rgb),
		g(rgb),
		b(rgb)
	{
	}

	inline XtRgb(float r_, float g_, float b_) :
		r(r_),
		g(g_),
		b(b_)
	{
	}
};

struct XtRgba
{
	float r, g, b, a;

public:
	inline explicit XtRgba(float rgba = 1.0f) :
		r(rgba),
		g(rgba),
		b(rgba),
		a(rgba)
	{
	}

	inline XtRgba(float r_, float g_, float b_, float a_) :
		r(r_),
		g(g_),
		b(b_),
		a(a_)
	{
	}
};

struct XtHsv
{
	float h, s, v;

public:
	inline explicit XtHsv() :
		h(0.0f),
		s(0.0f),
		v(1.0f)
	{
	}

	inline XtHsv(float h_, float s_, float v_) :
		h(h_),
		s(s_),
		v(v_)
	{
	}
};

struct XtHsva
{
	float h, s, v, a;

public:
	inline explicit XtHsva() :
		h(0.0f),
		s(0.0f),
		v(1.0f),
		a(1.0f)
	{
	}

	inline XtHsva(float h_, float s_, float v_, float a_) :
		h(h_),
		s(s_),
		v(v_),
		a(a_)
	{
	}
};

struct XtLinRgb
{
	float r, g, b;

public:
	inline explicit XtLinRgb(float rgb = 1.0f) :
		r(rgb),
		g(rgb),
		b(rgb)
	{
	}

	inline XtLinRgb(float r_, float g_, float b_) :
		r(r_),
		g(g_),
		b(b_)
	{
	}
};

struct XtLinRgba
{
	float r, g, b, a;

public:
	inline explicit XtLinRgba(float rgba = 1.0f) :
		r(rgba),
		g(rgba),
		b(rgba),
		a(rgba)
	{
	}

	inline XtLinRgba(float r_, float g_, float b_, float a_) :
		r(r_),
		g(g_),
		b(b_),
		a(a_)
	{
	}
};

// Range

template<typename Type>
struct XtRangeT
{
	Type from;
	Type to;
};

struct XtRangeI8 : public XtRangeT<XtI8> { };
struct XtRangeI16 : public XtRangeT<XtI16> { };
struct XtRangeI32 : public XtRangeT<XtI32> { };
struct XtRangeI64 : public XtRangeT<XtI64> { };

struct XtRangeU8 : public XtRangeT<XtU8> { };
struct XtRangeU16 : public XtRangeT<XtU16> { };
struct XtRangeU32 : public XtRangeT<XtU32> { };
struct XtRangeU64 : public XtRangeT<XtU64> { };

struct XtRangeF32 : public XtRangeT<XtF32> { };
struct XtRangeF64 : public XtRangeT<XtF64> { };

struct XtRangeVec2 : public XtRangeT<XtVec2> { };
struct XtRangeVec3 : public XtRangeT<XtVec3> { };
struct XtRangeVec4 : public XtRangeT<XtVec4> { };

struct XtRangeRgb : public XtRangeT<XtRgb> { };
struct XtRangeRgba : public XtRangeT<XtRgba> { };
struct XtRangeHsv : public XtRangeT<XtHsv> { };
struct XtRangeHsva : public XtRangeT<XtHsva> { };
struct XtRangeLinRgb : public XtRangeT<XtLinRgb> { };
struct XtRangeLinRgba : public XtRangeT<XtLinRgba> { };

// Dispersion

template<typename Type>
struct XtDispersionT
{
	Type mean;
	Type d;
};

struct XtDispersionI8 : public XtDispersionT<XtI8> { };
struct XtDispersionI16 : public XtDispersionT<XtI16> { };
struct XtDispersionI32 : public XtDispersionT<XtI32> { };
struct XtDispersionI64 : public XtDispersionT<XtI64> { };

struct XtDispersionU8 : public XtDispersionT<XtU8> { };
struct XtDispersionU16 : public XtDispersionT<XtU16> { };
struct XtDispersionU32 : public XtDispersionT<XtU32> { };
struct XtDispersionU64 : public XtDispersionT<XtU64> { };

struct XtDispersionF32 : public XtDispersionT<XtF32> { };
struct XtDispersionF64 : public XtDispersionT<XtF64> { };

struct XtDispersionVec2 : public XtDispersionT<XtVec2> { };
struct XtDispersionVec3 : public XtDispersionT<XtVec3> { };
struct XtDispersionVec4 : public XtDispersionT<XtVec4> { };

// Spline

enum XtSplineReparam
{
	XtSplineReparamDefault = 0,
	XtSplineReparamNone,
	XtSplineReparamLow,
	XtSplineReparamMedium,
	XtSplineReparamHigh,
};

struct XtKnotData
{
	XtF32		distance;
};

struct XtKnotVec3
{
	XtVec3		position;
	XtVec3		tangentIn;
	XtVec3		tangentOut;
	XtF32		tangentScale;
};

struct XtSplineVec3
{
	XtSplineReparam	reparamRes;
	bool closed;

	XtArray<XtKnotVec3>		knots;
	XtArray<XtKnotData*>	knotData;
};

// Misc

struct XtBoundingBox
{
	XtMat43	matrix;
	XtVec3	size;
};

struct XtRectangle
{
	XtF32 width;
	XtF32 height;
};

class XtSphere	
{
public:
	XtVec3	position;
	XtF32	radius;
};

#endif // XT_STD_H
