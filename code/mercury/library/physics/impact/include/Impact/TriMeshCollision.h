#ifndef __PHYSICS_TRIMESH_COLLISION_H__
#define __PHYSICS_TRIMESH_COLLISION_H__

#include "Impact\TriangleId.h"

namespace Impact
{

class I_TriMesh;

/// persistent collision data for one triangle
// TODO make this structure smaller. current size is 200+ bytes
class TriPersistentData : public HullPersistentData
{
public:
	TriPersistentData() : bCull(false), vBasePos(FLT_MAX, FLT_MAX, FLT_MAX), fMaxMoveDistance(0) {}

	// return the cached separating plane
	Maths::C_Plane GetSeparatingPlane();

	// cached triangle for colliding against
	ConvexTri tri;

	// persistent collision data for early rejection
	float fPlane; // projection of the triangle onto the separating plane (_HullPersistentData::persist.normal_)
	bool bCull;
	Maths::HVec3 vBasePos;
	float fMaxMoveDistance;
	float fMaxRotationAngle;
	TriangleId iTri;			// index of the triangle in the world mesh (for re-activation)
};

/// persistent data for a triangle that is in rejected state
struct RejectedTriData
{
	TriangleId iTri;			// index of the triangle in the world mesh (for re-activation)
	Maths::C_Plane vPlane;		// separating plane
};

/// calculates collision between a TriMesh and ConvexHull body
class TriMeshCollision : public I_PersistentData
{
public:
		TriMeshCollision();

	/// calculate contact points between a convex hull and a trimesh body
	int Collide(ContactVector &contacts, Body *p[2], Shape *pShape[2]);

private:
	/// incident polygon against the reference polygon and emit contact points
	template<typename T>
	int ClipPolyAndGenerateContacts(ContactVector &contacts, T *pRefHull, C_ClipPolygon &incidentPoly, int iRefPlane, int iFlip);

	/// collide one triangle with a convex hull
	int CollideTriMeshHull(ContactVector &contacts, ConvexHullInstance *pHull2, int &iCurTri);

	/// update polygon data from the trimesh body
	void UpdatePolygons(I_TriMesh *pBody1, const Maths::C_AxisBox &bbox);

	/// calculate maximum allowed movement before we have to re-test the separating plane
	void CalcMaxMoveDistanceAndRotationAngle(ConvexHullInstance *pHull, TriPersistentData *pPersist, int &iCurTri);

	/// update the far list, maxmovedistance etc.
	void UpdateFarList(I_TriMesh *pTriMesh, ConvexHullInstance *pHull2);

	/// vector containing the persistent polygon data
	Ut::Vector<TriPersistentData> m_Tris;

	/// vector of triangles that are further than _pHull->Radius()_ away
	Ut::Vector<RejectedTriData> m_FarTris;

	/// vector of triangles that are nearer than _pHull->Radius()_, but not colliding
//	Ut::Vector<RejectedTriData> m_NearTris;

	Maths::HVec3		m_vBasePos;
	float				m_fMaxMoveDistance2;
	float				m_fMaxRotDistance;
	Maths::C_AxisBox	m_QueryBox;
};

};

#endif
