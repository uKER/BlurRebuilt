#ifndef __PHYSICS_CONVEXTRI_H__
#define __PHYSICS_CONVEXTRI_H__

#include "Impact\ConvexBase.h"

namespace Impact
{

typedef Ut::UtArrayVector<Maths::C_Vertex, 3> C_Triangle;

/// representation of a triangle for collision purposes
class ConvexTri : public ConvexBase
{
public:
	/// create from a polygon (for trimesh collision)
	void						SetFromPolygon(const C_Triangle &poly);

	/// get the side-plane index ptr for a given plane index (_iSupportingPlane_ is ignored)
	const Maths::C_Plane		&GetSidePlane(int iSupportingPlane, int iSidePlane) const;

	/// return the number of side-planes for the given plane
	int							NumSidePlanes(int iPlane) const;

	/// get reference to points array from ConvexBase
	Ut::Vector<Maths::HVec3>	&GetPoints();
	
	/// get reference to planes array from ConvexBase
	Ut::Vector<Maths::C_Plane>	&GetPlanes();

	/// get reference to sideplanes array from ConvexBase
	Ut::UtArrayVector<Maths::C_Plane,3>	&GetSidePlanes();

private:
	Ut::UtArrayVector<Maths::C_Plane,3>	m_SidePlanes;
};

};

#endif
