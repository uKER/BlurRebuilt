#ifndef __PHYSICS_I_TRIMESH_H__
#define __PHYSICS_I_TRIMESH_H__

#include "Impact\Shape.h"
#include "Impact\TriangleId.h"
#include "Impact\ConvexTri.h"

namespace Impact
{

typedef Ut::UtArrayVector<Maths::C_Vertex, 3> C_Triangle;

/// virtual interface for a representation of triangle mesh collision
class I_TriMesh : public Shape
{
public:
	virtual ~I_TriMesh() {}

	/// return the bounding box of the mesh
	virtual Maths::C_AxisBox	GetAABB(Body *pBody) = 0;

	/// return the triangle with index _i_
	virtual const C_Triangle	GetPolygon(const TriangleId &i) = 0;

	/// set a convex tri structure from one of the triangles in the spatial hierarchy
	virtual void				SetConvexTri(const TriangleId &i, ConvexTri *pTri)
	{
		pTri->SetFromPolygon( GetPolygon(i) );
	}

	/// perform a collision query against the spatial hierarchy
	virtual void				Query(const Maths::C_AxisBox &area, Ut::Vector<TriangleId> &indices, Maths::C_AxisBox &limit) = 0;
};

};

#endif
