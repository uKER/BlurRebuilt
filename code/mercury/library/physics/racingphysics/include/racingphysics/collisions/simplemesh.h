#ifndef RP_SIMPLE_MESH_H
#define RP_SIMPLE_MESH_H
#ifdef _MSC_VER
#pragma once
#endif

#include "system/serialise.h"
#include "btl/containers/vector.h"
#include "racingphysics/physics/geometry.h"

namespace RacingPhysics
{

class SimpleMesh
{
public:

	struct Triangle
	{
		RacingPhysics::Point3R v0, v1, v2;

		template <typename Archive> void Serialise(Archive& ar) const {ar << v0 << v1 << v2;}
		template <typename Archive> void Deserialise(Archive& ar) 
		{
			ar.operator >> (v0); // >> is ambiguous
			ar.operator >> (v1);
			ar.operator >> (v2);
		}
	};

	void AddTriangle(const Triangle& triangle) {m_triangles.PushBack(triangle);}
	void Clear() {m_triangles.Clear();}

	int GetNumTriangles() const {return m_triangles.Size();}
	const Triangle& GetTriangle(int i) const {return m_triangles[i];}

	// serialisation
	template <typename Archive>
	friend void Deserialise(Archive& ar, SimpleMesh& data);

	template <typename Archive>
	friend void Serialise(Archive& ar, const SimpleMesh& data);

private:
	Btl::Vector<Triangle> m_triangles;
};

////////////////////////////////////////////////////////

// serialisation
template <typename Archive>
void Deserialise(Archive& ar, SimpleMesh& data)
{
	data.Clear();
	ar >> data.m_triangles;
}

template <typename Archive>
void Serialise(Archive& ar, const SimpleMesh& data)
{
	ar << data.m_triangles;
}

}

#endif



