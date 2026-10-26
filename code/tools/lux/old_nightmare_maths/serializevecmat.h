#ifndef __SERIALIZE_VECMAT_H__
#define __SERIALIZE_VECMAT_H__

namespace ost
{

inline Engine& operator << (Engine& ar, float3 &v) { ar << v.x << v.y << v.z; return ar; }
inline Engine& operator >> (Engine& ar, float3 &v) { ar >> v.x >> v.y >> v.z; return ar; }

#ifdef VECMAT_COMPILED
// serializing for HMat44 matrices
inline Engine& operator << (Engine &ar, HMat44 &m)
{
	ar << m[0][0] << m[0][1] << m[0][2] << m[0][3];
	ar << m[1][0] << m[1][1] << m[1][2] << m[1][3];
	ar << m[2][0] << m[2][1] << m[2][2] << m[2][3];
	ar << m[3][0] << m[3][1] << m[3][2] << m[3][3];
	return ar;
}
inline Engine& operator >> (Engine &ar, HMat44 &m)
{
	ar >> m[0][0] >> m[0][1] >> m[0][2] >> m[0][3];
	ar >> m[1][0] >> m[1][1] >> m[1][2] >> m[1][3];
	ar >> m[2][0] >> m[2][1] >> m[2][2] >> m[2][3];
	ar >> m[3][0] >> m[3][1] >> m[3][2] >> m[3][3];
	return ar;
}
#else
// serializing for HMat44 matrices
inline Engine& operator << (Engine &ar, HMat44 &m)
{
	for (int i=0; i<16; i++)
		ar << m.v[i];
	return ar;
}
inline Engine& operator >> (Engine &ar, HMat44 &m)
{
	for (int i=0; i<16; i++)
		ar >> m.v[i];
	return ar;
}
#endif

inline Engine& operator << (Engine& ar, Aftk::Matrix44 &m) { for (int i=0; i<16; i++) ar << ((float*)&m)[i]; return ar; }
inline Engine& operator >> (Engine& ar, Aftk::Matrix44 &m) { for (int i=0; i<16; i++) ar >> ((float*)&m)[i]; return ar; }

inline Engine& operator << (Engine& ar, box3f &b) { ar << b.vmin << b.vmax; return ar; }
inline Engine& operator >> (Engine& ar, box3f &b) { ar >> b.vmin >> b.vmax; return ar; }

inline Engine& operator << (Engine& ar, float4 &v) { ar << v.x << v.y << v.z << v.w; return ar; }
inline Engine& operator >> (Engine& ar, float4 &v) { ar >> v.x >> v.y >> v.z >> v.w; return ar; }

inline Engine& operator << (Engine& ar, float2 &v) { ar << v.x << v.y; return ar; }
inline Engine& operator >> (Engine& ar, float2 &v) { ar >> v.x >> v.y; return ar; }

/*
inline Engine& operator << (Engine& ar, float3 &v) { ar << v.x << v.y << v.z; return ar; }
inline Engine& operator >> (Engine& ar, float3 &v) { ar >> v.x >> v.y >> v.z; return ar; }

// serializing for HMat44 matrices
inline Engine& operator << (Engine &ar, HMat44 &m)
{
	for (int i=0; i<16; i++)
		ar << m.v[i];
	return ar;
}
inline Engine& operator >> (Engine &ar, HMat44 &m)
{
	for (int i=0; i<16; i++)
		ar >> m.v[i];
	return ar;
}

inline Engine& operator << (Engine& ar, Aftk::Matrix44 &m) { for (int i=0; i<16; i++) ar << ((float*)&m)[i]; return ar; }
inline Engine& operator >> (Engine& ar, Aftk::Matrix44 &m) { for (int i=0; i<16; i++) ar >> ((float*)&m)[i]; return ar; }

inline Engine& operator << (Engine& ar, MathsStorage::Box3 &b) { ar << b.vmin << b.vmax; return ar; }
inline Engine& operator >> (Engine& ar, MathsStorage::Box3 &b) { ar >> b.vmin >> b.vmax; return ar; }

inline Engine& operator << (Engine& ar, float4 &v) { ar << v.x << v.y << v.z << v.w; return ar; }
inline Engine& operator >> (Engine& ar, float4 &v) { ar >> v.x >> v.y >> v.z >> v.w; return ar; }

inline Engine& operator << (Engine& ar, float2 &v) { ar << v.x << v.y; return ar; }
inline Engine& operator >> (Engine& ar, float2 &v) { ar >> v.x >> v.y; return ar; }

*/
}

#endif
