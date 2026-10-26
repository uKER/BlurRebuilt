#ifndef __VIS_CORE_H__
#define __VIS_CORE_H__

struct Vertex
{
	float3		vPos;
	FaceFlags	flags;

	friend ost::Engine& operator << (ost::Engine& ar, const Vertex &v)
	{
		ar << v.vPos << v.flags.bits;
		return ar;
	}

	friend ost::Engine& operator >> (ost::Engine& ar, Vertex &v)
	{
		ar >> v.vPos >> v.flags.bits;
		return ar;
	}
};

#endif
