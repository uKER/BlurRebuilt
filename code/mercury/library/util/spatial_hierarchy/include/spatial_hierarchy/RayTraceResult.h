#ifndef RAY_TRACE_RESULT_H
#define RAY_TRACE_RESULT_H

struct TriAccel;

namespace SD
{

/// class containing the result of a ray that has been raytraced by the SpatialHierarchy class
class RayTraceResult
{
public:
	/// indicates whether the ray hit something
	/** the other parameters in this class will only be valid if the ray hit something,
	    and if the ray was not a shafow-ray. **/
	bool	bHit;

	/// indicates the index of the polygon that was hit
	int		iPolygon;
	/// global primitive ID of the object that was hit
	int		iGlobalPrim;
	/// global subprimitive ID of the object that was hit
	int		iGlobalSubPrim;
#ifndef DOXYGEN_SHOULD_SKIP_THIS
	/// indicates the index of the intermediate primitive that was hit (for internal use)
	int		iPrimitive;
#endif
	/// gives the pointer to the meshnode containing the polygon that was hit
	Aftk::I_SceneMeshNode *pMeshNode;
	/// contains the thee vertex positions of the triangle that was hit
	Float3	vVerts[3];
	/// barycentric co-ordinates of the intersection point on the triangle
	float	u,v;
	/// contains the distance to the hit point
	float	fDistance;
	/// contains the hit position in world space
	Float3	vHitPos;
	/// returns the normal of the surface at the hit point
	Float3	GetSurfaceNormal() const;

	TriAccel *pTri;

	/// returns the position of the hit point
	Float3	&GetPosition() { return vHitPos; };
};
}

#endif
