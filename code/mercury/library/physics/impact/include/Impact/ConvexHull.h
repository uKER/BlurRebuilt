#ifndef __IMPACT_CONVEXHULL_H__
#define __IMPACT_CONVEXHULL_H__

#include "Impact\ConvexBase.h"
#include "Impact\Shape.h"

namespace Impact
{

class C_ClipPolygon;
class ConvexHullInstance;

/// representation of a convex hull for collision purposes
/// this class contains the static (shared) data
class ConvexHull : public ConvexBase, public BaseShape
{
	friend class ConvexHullInstance;
public:
	/// constructor
							ConvexHull();

	/// destructor
							~ConvexHull();

	/// create an instance of this Shape, to attach to a body
	virtual	Shape			*Instance();

	/// add a point to the convex hull
	void					AddPoint(const Maths::HVec3 &v);

	/// generate hull data from the given list of points
	void					Build();

	/// get the local-space AABB
	const Maths::C_AxisBox	&GetAABB() const;

	/// retrieve the given polygon
	void					GetPolygon(int i, C_ClipPolygon &poly) const;

	/// set center of mass
	void					SetCenterOfMass(const Maths::HVec3 &v);

	/// get center of mass
	const Maths::HVec3		&GetCenterOfMass() const;

private:
	/// generate the plane list from the point list
	void					CalculatePlanes();

	/// generate the polygon list from the plane list
	void					CalculatePolygons();

	/// for each plane, calculate a list of planes that bound it
	void					CalculateSidePlanes();

	/// helper function for _CalculateSidePlanes_
	int						FindMatchingEdge(int iVtx1, int iVtx2, int iOrgPlane);

protected:
	typedef Ut::Vector<int>	Polygon;

	Ut::Vector<Polygon>		m_SidePlanes;
	Ut::Vector<Polygon>		m_Polygons;
	Maths::C_AxisBox		m_AABB;
	Maths::HVec3			m_vOriginOffset;	// offset between the center of mass and the origin of the model
};

/// instance of a convex hull containing instance specific data such as world position, world space points, etc.
class ConvexHullInstance : public ConvexBase
{
	friend class ConvexHull;
public:
	/// get side plane idx
	int						GetSidePlaneIdx(int iSupportingPlane, int iSidePlane) const;

	/// get the side-plane index ptr for a given plane index
	const Maths::C_Plane	&GetSidePlane(int iSupportingPlane, int iSidePlane) const;

	/// return the number of side-planes for the given plane
	int						NumSidePlanes(int iPlane) const;

	/// retrieve the given polygon
	void					GetPolygon(int i, C_ClipPolygon &poly) const;

	/// update the convex hull - called by ConvexHullCollision
	void					ApplyTransform();

	/// returns 'true' if this objects wants to be deleted by it's body when the body itself is deleted
	bool					WantDelete();

	/// get the current world-space axis-aligned bounding box
	Maths::C_AxisBox		GetAABB(Body *pBody);

	/// returns the parent convex hull
	const ConvexHull		*GetParent() const;

protected:
	/// construct given a parent _ConvexHull_
							ConvexHullInstance(ConvexHull *pParent);
private:
	/// get the current world-space AABB for a given position and rotation
	Maths::C_AxisBox		GetAABB(const Maths::HVec3 &vPos, const Maths::C_Quaternion &qRot);

	const ConvexHull		*m_pParent;

};

};

#endif
