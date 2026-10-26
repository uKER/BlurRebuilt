#include "Common/XtPch.h"
#include "XtGameImpl.h"
#include "Xt/XtStd.h"

// Basic types

XT_TYPE(bool, "bool")		XT_ATOM(kXtBool)	XT_END_TYPE

XT_TYPE(XtI8, "i8")			XT_ATOM(kXtI8)		XT_END_TYPE
XT_TYPE(XtI16, "i16")		XT_ATOM(kXtI16)		XT_END_TYPE
XT_TYPE(XtI32, "i32")		XT_ATOM(kXtI32)		XT_END_TYPE
XT_TYPE(XtI64, "i64")		XT_ATOM(kXtI64)		XT_END_TYPE

XT_TYPE(XtU8, "u8")			XT_ATOM(kXtU8)		XT_END_TYPE
XT_TYPE(XtU16, "u16")		XT_ATOM(kXtU16)		XT_END_TYPE
XT_TYPE(XtU32, "u32")		XT_ATOM(kXtU32)		XT_END_TYPE
XT_TYPE(XtU64, "u64")		XT_ATOM(kXtU64)		XT_END_TYPE

XT_TYPE(XtF32, "f32")		XT_ATOM(kXtF32)		XT_END_TYPE
XT_TYPE(XtF64, "f64")		XT_ATOM(kXtF64)		XT_END_TYPE

XT_TYPE(XtSz8,		"sz8")		XT_ATOM(kXtSz8)		XT_END_TYPE
XT_TYPE(XtSz16,		"sz16")		XT_ATOM(kXtSz16)	XT_END_TYPE
XT_TYPE(XtLocId,	"LocId")	XT_ATOM(kXtLocId)	XT_END_TYPE

// Vector

XT_TYPE(XtVec2, "Vec2")
	XT_FIELD(vx, "vx")
	XT_FIELD(vy, "vy")
XT_END_TYPE

XT_TYPE(XtVec3, "Vec3")
	XT_FIELD(vx, "vx")
	XT_FIELD(vy, "vy")
	XT_FIELD(vz, "vz")
XT_END_TYPE

XT_TYPE(XtVec4, "Vec4")
	XT_FIELD(vx, "vx")
	XT_FIELD(vy, "vy")
	XT_FIELD(vz, "vz")
	XT_FIELD(vw, "vw")
XT_END_TYPE

// Matrix

XT_TYPE(XtMat22, "Mat22")
	XT_FIELD(ax, "ax")
	XT_FIELD(ay, "ay")
XT_END_TYPE

XT_TYPE(XtMat32, "Mat32")
	XT_FIELD(ax, "ax")
	XT_FIELD(ay, "ay")
	XT_FIELD(az, "az")
XT_END_TYPE

XT_TYPE(XtMat33, "Mat33")
	XT_FIELD(ax, "ax")
	XT_FIELD(ay, "ay")
	XT_FIELD(az, "az")
XT_END_TYPE

XT_TYPE(XtMat43, "Mat43")
	XT_FIELD(ax, "ax")
	XT_FIELD(ay, "ay")
	XT_FIELD(az, "az")
	XT_FIELD(aw, "aw")
XT_END_TYPE

XT_TYPE(XtMat44, "Mat44")
	XT_FIELD(ax, "ax")
	XT_FIELD(ay, "ay")
	XT_FIELD(az, "az")
	XT_FIELD(aw, "aw")
XT_END_TYPE

// Colour

XT_TYPE(XtRgb, "Rgb")
	XT_FIELD(r, "r")
	XT_FIELD(g, "g")
	XT_FIELD(b, "b")
XT_END_TYPE

XT_TYPE(XtRgba, "Rgba")
	XT_FIELD(r, "r")
	XT_FIELD(g, "g")
	XT_FIELD(b, "b")
	XT_FIELD(a, "a")
XT_END_TYPE

XT_TYPE(XtHsv, "Hsv")
	XT_FIELD(h, "h")
	XT_FIELD(s, "s")
	XT_FIELD(v, "v")
XT_END_TYPE

XT_TYPE(XtHsva, "Hsva")
	XT_FIELD(h, "h")
	XT_FIELD(s, "s")
	XT_FIELD(v, "v")
	XT_FIELD(a, "a")
XT_END_TYPE

XT_TYPE(XtLinRgb, "LinRgb")
	XT_FIELD(r, "r")
	XT_FIELD(g, "g")
	XT_FIELD(b, "b")
XT_END_TYPE

XT_TYPE(XtLinRgba, "LinRgba")
	XT_FIELD(r, "r")
	XT_FIELD(g, "g")
	XT_FIELD(b, "b")
	XT_FIELD(a, "a")
XT_END_TYPE

// Range

#define ANNOTATE_RANGE(name) XT_TYPE(Xt ## name, #name) XT_FIELD(from, "from") XT_FIELD(to, "to") XT_END_TYPE

ANNOTATE_RANGE(RangeI8)
ANNOTATE_RANGE(RangeI16)
ANNOTATE_RANGE(RangeI32)
ANNOTATE_RANGE(RangeI64)

ANNOTATE_RANGE(RangeU8)
ANNOTATE_RANGE(RangeU16)
ANNOTATE_RANGE(RangeU32)
ANNOTATE_RANGE(RangeU64)

ANNOTATE_RANGE(RangeF32)
ANNOTATE_RANGE(RangeF64)

ANNOTATE_RANGE(RangeVec2)
ANNOTATE_RANGE(RangeVec3)
ANNOTATE_RANGE(RangeVec4)

ANNOTATE_RANGE(RangeRgb)
ANNOTATE_RANGE(RangeRgba)
ANNOTATE_RANGE(RangeHsv)
ANNOTATE_RANGE(RangeHsva)
ANNOTATE_RANGE(RangeLinRgb)
ANNOTATE_RANGE(RangeLinRgba)

// Dispersion

#define ANNOTATE_DISPERSION(name) XT_TYPE(Xt ## name, #name) XT_FIELD(mean, "mean") XT_FIELD(d, "d") XT_END_TYPE

ANNOTATE_DISPERSION(DispersionI8)
ANNOTATE_DISPERSION(DispersionI16)
ANNOTATE_DISPERSION(DispersionI32)
ANNOTATE_DISPERSION(DispersionI64)

ANNOTATE_DISPERSION(DispersionU8)
ANNOTATE_DISPERSION(DispersionU16)
ANNOTATE_DISPERSION(DispersionU32)
ANNOTATE_DISPERSION(DispersionU64)

ANNOTATE_DISPERSION(DispersionF32)
ANNOTATE_DISPERSION(DispersionF64)

ANNOTATE_DISPERSION(DispersionVec2)
ANNOTATE_DISPERSION(DispersionVec3)
ANNOTATE_DISPERSION(DispersionVec4)

// Spline

XT_TYPE(XtSplineReparam, "SplineReparam")
	XT_OPTION(XtSplineReparamDefault,	"default")
	XT_OPTION(XtSplineReparamNone,		"none")
	XT_OPTION(XtSplineReparamLow,		"low")
	XT_OPTION(XtSplineReparamMedium,	"medium")
	XT_OPTION(XtSplineReparamHigh,		"high")
XT_END_TYPE

XT_TYPE(XtKnotData,	"KnotData")
	XT_FIELD(distance,	"distance")
XT_END_TYPE

XT_TYPE(XtKnotVec3,	"KnotVec3")
	XT_FIELD(position,		"position")
	XT_FIELD(tangentIn,		"tangentIn")
	XT_FIELD(tangentOut,	"tangentOut")
	XT_FIELD(tangentScale,	"tangentScale")
//	XT_FIELD(knotData,		"knotData")
XT_END_TYPE

XT_TYPE(XtSplineVec3, "SplineVec3")
	XT_FIELD(reparamRes,	"reparamRes")
	XT_FIELD(closed,		"closed")
	XT_FIELD(knots,			"knots")
	XT_FIELD(knotData,		"knotData")
XT_END_TYPE

XT_TYPE(XtBoundingBox, "BoundingBox")
	XT_FIELD(matrix,	"Orientation")
	XT_FIELD(size,		"Size")
XT_END_TYPE

XT_TYPE(XtRectangle, "Rectangle")
	XT_FIELD(width,		"Width")
	XT_FIELD(height,	"Height")
XT_END_TYPE

XT_TYPE(XtSphere, "Sphere")
	XT_FIELD(position, "Position")
	XT_FIELD(radius,	"Radius")
XT_END_TYPE

// Library

XT_LIBRARY(Xt)

	// Basic

	XT_REGISTER_TYPE(bool)
	XT_REGISTER_TYPE(XtI8)
	XT_REGISTER_TYPE(XtI16)
	XT_REGISTER_TYPE(XtI32)
	XT_REGISTER_TYPE(XtI64)
	XT_REGISTER_TYPE(XtU8)
	XT_REGISTER_TYPE(XtU16)
	XT_REGISTER_TYPE(XtU32)
	XT_REGISTER_TYPE(XtU64)
	XT_REGISTER_TYPE(XtF32)
	XT_REGISTER_TYPE(XtF64)
	XT_REGISTER_TYPE(XtSz8)
	XT_REGISTER_TYPE(XtSz16)
	XT_REGISTER_TYPE(XtLocId)

	// Vector

	XT_REGISTER_TYPE(XtVec2)
	XT_REGISTER_TYPE(XtVec3)
	XT_REGISTER_TYPE(XtVec4)

	// Matrix

	XT_REGISTER_TYPE(XtMat22)
	XT_REGISTER_TYPE(XtMat32)
	XT_REGISTER_TYPE(XtMat33)
	XT_REGISTER_TYPE(XtMat43)
	XT_REGISTER_TYPE(XtMat44)

	// Colour

	XT_REGISTER_TYPE(XtRgb)
	XT_REGISTER_TYPE(XtRgba)
	XT_REGISTER_TYPE(XtHsv)
	XT_REGISTER_TYPE(XtHsva)
	XT_REGISTER_TYPE(XtLinRgb)
	XT_REGISTER_TYPE(XtLinRgba)

	// Range

	XT_REGISTER_TYPE(XtRangeI8)
	XT_REGISTER_TYPE(XtRangeI16)
	XT_REGISTER_TYPE(XtRangeI32)
	XT_REGISTER_TYPE(XtRangeI64)

	XT_REGISTER_TYPE(XtRangeU8)
	XT_REGISTER_TYPE(XtRangeU16)
	XT_REGISTER_TYPE(XtRangeU32)
	XT_REGISTER_TYPE(XtRangeU64)

	XT_REGISTER_TYPE(XtRangeF32)
	XT_REGISTER_TYPE(XtRangeF64)

	XT_REGISTER_TYPE(XtRangeVec2)
	XT_REGISTER_TYPE(XtRangeVec3)
	XT_REGISTER_TYPE(XtRangeVec4)

	XT_REGISTER_TYPE(XtRangeRgb)
	XT_REGISTER_TYPE(XtRangeRgba)
	XT_REGISTER_TYPE(XtRangeHsv)
	XT_REGISTER_TYPE(XtRangeHsva)
	XT_REGISTER_TYPE(XtRangeLinRgb)
	XT_REGISTER_TYPE(XtRangeLinRgba)

	// Dispersion

	XT_REGISTER_TYPE(XtDispersionI8)
	XT_REGISTER_TYPE(XtDispersionI16)
	XT_REGISTER_TYPE(XtDispersionI32)
	XT_REGISTER_TYPE(XtDispersionI64)

	XT_REGISTER_TYPE(XtDispersionU8)
	XT_REGISTER_TYPE(XtDispersionU16)
	XT_REGISTER_TYPE(XtDispersionU32)
	XT_REGISTER_TYPE(XtDispersionU64)

	XT_REGISTER_TYPE(XtDispersionF32)
	XT_REGISTER_TYPE(XtDispersionF64)

	XT_REGISTER_TYPE(XtDispersionVec2)
	XT_REGISTER_TYPE(XtDispersionVec3)
	XT_REGISTER_TYPE(XtDispersionVec4)

	// Spline

	XT_REGISTER_TYPE(XtSplineReparam)
	XT_REGISTER_TYPE(XtKnotData)
	XT_REGISTER_TYPE(XtKnotVec3)
	XT_REGISTER_TYPE(XtSplineVec3)
	
	// Misc
	
	XT_REGISTER_TYPE(XtBoundingBox)
	XT_REGISTER_TYPE(XtRectangle)
	XT_REGISTER_TYPE(XtSphere)

XT_END_LIBRARY
