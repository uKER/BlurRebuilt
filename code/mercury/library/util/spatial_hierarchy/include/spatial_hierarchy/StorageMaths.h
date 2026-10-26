#ifndef __SPATIALHIERARCHY_STORAGEMATHS_H__
#define __SPATIALHIERARCHY_STORAGEMATHS_H__

// some typedefs to easily use Maths-Storage classes
// and some functions to easily perform maths on them

typedef MathsStorage::Float2		Float2;
typedef MathsStorage::Float3		Float3;
typedef MathsStorage::Float4		Float4;
typedef MathsStorage::Box3			Box3;
typedef MathsStorage::MatrixRow44	Float44;

inline Float3 operator +(const Float3 &a, const Float3 &b)
{
	Float3 rtn;
	rtn.x = a.x + b.x;
	rtn.y = a.y + b.y;
	rtn.z = a.z + b.z;
	return rtn;
}

inline Float3 operator *(const Float3 &a, const Float3 &b)
{
	Float3 rtn;
	rtn.x = a.x * b.x;
	rtn.y = a.y * b.y;
	rtn.z = a.z * b.z;
	return rtn;
}

inline Float3 operator *(const Float3 &a, const float b)
{
	Float3 rtn;
	rtn.x = a.x * b;
	rtn.y = a.y * b;
	rtn.z = a.z * b;
	return rtn;
}

inline Float3 operator -(const Float3 &a, const Float3 &b)
{
	Float3 rtn;
	rtn.x = a.x - b.x;
	rtn.y = a.y - b.y;
	rtn.z = a.z - b.z;
	return rtn;
}

inline Float3 operator /(const float a, const Float3 &b)
{
	Float3 rtn;
	rtn.x = a / b.x;
	rtn.y = a / b.y;
	rtn.z = a / b.z;
	return rtn;
}

inline Float3 CrossProduct(const Float3 &a, const Float3 &b)
{
	Float3 rtn;
	rtn.x = a.y * b.z - a.z * b.y;
	rtn.y = a.z * b.x - a.x * b.z;
	rtn.z = a.x * b.y - a.y * b.x;
	return rtn;
}

inline float Length(const Float3 &a)
{
	return sqrtf(a.x*a.x + a.y*a.y + a.z*a.z);
}

inline void operator *=(Float3 &a, const float f)
{
	a.x *= f;
	a.y *= f;
	a.z *= f;
}

inline Float3 Normalize(const Float3 &a)
{
	float f = Length(a);
	Float3 rtn = a;
	rtn *= (1.f/f);
	return rtn;
}

inline float DotProduct(const Float3 &a, const Float3 &b)
{
	return a.x*b.x + a.y*b.y + a.z*b.z;
}

// clip ray against the an axis aligned bounding box
inline void ClipRayVsBBox(const Float3 &vRayOrg, const Float3 &vInvRayDir, const int *iRaydir, float *pfNear, float *pfFar, const Box3 &bbox)
{
	float dmax, dmin;
	float fNear = *pfNear;
	float fFar  = *pfFar;

	for (int iAxis=0; iAxis<3; iAxis++)
	{
		dmax = ( bbox.vmax.v[iAxis] - vRayOrg.v[iAxis] ) * vInvRayDir.v[iAxis];
		dmin = ( bbox.vmin.v[iAxis] - vRayOrg.v[iAxis] ) * vInvRayDir.v[iAxis];

		if (iRaydir[iAxis] >= 0)
		{
			fFar = Maths::Min( dmax, fFar );
			fNear = Maths::Max( dmin, fNear );
		}
		else
		{
			fFar = Maths::Min( dmin, fFar );
			fNear = Maths::Max( dmax, fNear );
		}
	}

	*pfNear = fNear;
	*pfFar  = fFar;
}

#ifdef CCXX_PERSIST_H_

namespace ost
{

inline Engine& operator << (Engine& ar, Float2 &v) { ar << v.x << v.y; return ar; }
inline Engine& operator >> (Engine& ar, Float2 &v) { ar >> v.x >> v.y; return ar; }

inline Engine& operator << (Engine& ar, Float3 &v) { ar << v.x << v.y << v.z; return ar; }
inline Engine& operator >> (Engine& ar, Float3 &v) { ar >> v.x >> v.y >> v.z; return ar; }

inline Engine& operator << (Engine& ar, Float4 &v) { ar << v.x << v.y << v.z << v.w; return ar; }
inline Engine& operator >> (Engine& ar, Float4 &v) { ar >> v.x >> v.y >> v.z >> v.w; return ar; }

inline Engine& operator << (Engine& ar, Box3 &b) { ar << b.vmin << b.vmax; return ar; }
inline Engine& operator >> (Engine& ar, Box3 &b) { ar >> b.vmin >> b.vmax; return ar; }

// serializing for HMat44 matrices
inline Engine& operator << (Engine &ar, Float44 &m)
{
	for (int i=0; i<16; i++)
		ar << m.v[i];
	return ar;
}

inline Engine& operator >> (Engine &ar, Float44 &m)
{
	for (int i=0; i<16; i++)
		ar >> m.v[i];
	return ar;
}

}; // namespace ost

#endif

#endif
