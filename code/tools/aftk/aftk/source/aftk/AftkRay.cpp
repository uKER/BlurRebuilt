#include <stdafx.h>

using namespace Aftk;

namespace Aftk
{
	//Moller Trumbore triangle-intersection.
	inline void MollerTrumboreIntersect(
		C_RayTriangleResult& result, 
		const C_Ray& r,
		const Position3& p0, 
		const Position3& p1,
		const Position3& p2)
	{
		bool cull = false;
		F32 t, u, v;
		Vector3F32 edge1, edge2, tvec, pvec, qvec;
		F32 det,inv_det;
		// find vectors for two edges sharing p0
		edge1 = p1 - p0;
		edge2 = p2 - p0;
		
		// begin calculating determinant - also used to calculate U parameter.
		pvec = r.Direction() ^ edge2;
		
		//if determinant is near zero, ray lies in plane of triangle.
		det = edge1 | pvec;
		if (cull)
		{
			if (det < AFTK_EPSILON)
			{
				result.Set(false);
				return;
			}
			// calculate distance from p0 to ray origin
			tvec = r.Origin() - p0;
			
			// calculate U parameter and test bounds 
			u = tvec | pvec;
			if (u < 0.0 || u > det)
			{
				result.Set(false);
				return;
			}

			//prepare to test V parameter
			qvec = tvec ^ edge1;
			//calculate V parameter and test bounds

			v = (Vector3F32&)r.Origin() | qvec;
			if (v < 0.0 || u + v > det)
			{
				result.Set(false);
				return;
			}

			// calculate t, scale parameters, ray intersects triangle
			t = edge2 | qvec;
			inv_det = 1.0f / det;
			t *= inv_det;
			u *= inv_det;
			v *= inv_det;
		}
		else
		{
			// the non-culling branch
			if (det > -AFTK_EPSILON && det < AFTK_EPSILON)
			{
				result.Set(false);
				return;
			}

			inv_det = 1.0f / det;
			// calculate distance from p0 to ray origin 
			tvec = (Vector3F32&)r.Origin() - (Vector3F32&)p0;
			//calculate U parameter and test bounds.
			u = (tvec | pvec) * inv_det;
			if (u < 0.0 || u > 1.0f)
			{
				result.Set(false);
				return;
			}
			// prepare to test V parameter
			qvec = tvec ^ edge1;
			// calculate V parameter and test bounds
			v = ((Vector3F32&)r.Direction() | qvec) * inv_det;
			
			if (v < 0.0 || u + v > 1.0f)
			{
				result.Set(false);
				return;
			}

			// calculate t, ray intersects triangle
			t = (edge2 | qvec) * inv_det;
		}

		Vector3F32 I;
		I = (Vector3F32&)p0 * (1-(u+v));
		I += (Vector3F32&)p1 * u;
		I += (Vector3F32&)p2 * v;
		result.Set(true, (Position3&)I);
	}
}

inline void C_Ray::IntersectTriangle(
	C_RayTriangleResult& result, 
	const Position3& p0, 
	const Position3& p1,
	const Position3& p2) const
{
	MollerTrumboreIntersect(result, *this, p0, p1, p2);
}

void C_Ray::IntersectMesh(
	C_RayMeshResult& result, 
	C_SceneNode* pNode,
	U32 lodLevel,
	bool evaluate,
	bool findClosest) const
{
	result.Set(Position3(),0,-1, -1);
	bool foundClosestIntersection = false;
	F32 closestIntersection;
	Matrix44 worldMatrix = pNode->GetWorldMatrix();

	I_SceneMeshNode* pMesh = 0;
	Aftk::C_RayMeshResult rayResult;

	if (pNode->GetType() == E_ObjType_SceneInstance)
	{
		C_SceneNode* pSourceNode = 
			((C_SceneInstanceNode*)pNode)->GetSourceNodePtr();
		if (pSourceNode->HasType(E_ObjType_ISceneMesh))
		{
			pMesh = (I_SceneMeshNode*)pSourceNode;
		}
	}
	else if (pNode->HasType(E_ObjType_ISceneMesh))
	{
		pMesh = (I_SceneMeshNode*)pNode;
	}

	if (!pMesh)
	{
		result.Set(Position3(),0,-1, -1);
		return;
	}

	for (U32 iPoly = 0; iPoly < pMesh->GetNumPolygons(lodLevel); iPoly++)
	{
		const C_Polygon& poly = pMesh->GetPolygon(lodLevel, iPoly);

		for (U32 jTri = 0; jTri < poly.GetNumTriangles(); jTri++)
		{
			U32 trianglePVIndexList[3];
			poly.GetTrianglePolygonVertexIndices(jTri, trianglePVIndexList);


			Position3 positions[3];
			if (evaluate)
			{
				positions[0] = 
					ApplyMatrix(worldMatrix, pMesh->GetEvaluatedPolygonVertexPosition(trianglePVIndexList[0]));
				positions[1] = 
					ApplyMatrix(worldMatrix, pMesh->GetEvaluatedPolygonVertexPosition(trianglePVIndexList[1]));
				positions[2] = 
					ApplyMatrix(worldMatrix, pMesh->GetEvaluatedPolygonVertexPosition(trianglePVIndexList[2]));
			}
			else
			{
				positions[0] = 
					ApplyMatrix(worldMatrix, pMesh->GetPolygonVertexPosition(trianglePVIndexList[0]));
				positions[1] = 
					ApplyMatrix(worldMatrix, pMesh->GetPolygonVertexPosition(trianglePVIndexList[1]));
				positions[2] = 
					ApplyMatrix(worldMatrix, pMesh->GetPolygonVertexPosition(trianglePVIndexList[2]));
			}

			trianglePVIndexList[0] = poly.GetPolygonVertexIndex(0);
			trianglePVIndexList[1] = poly.GetPolygonVertexIndex(1);
			trianglePVIndexList[2] = poly.GetPolygonVertexIndex(2);

			C_RayTriangleResult triResult;
			IntersectTriangle(
				triResult, positions[0], positions[1], positions[2]);
			if (triResult.Hit())
			{
				if (triResult.Hit())
				{
					if (findClosest)
					{
						Aftk::Vector3F32 delta = 
							(Aftk::Vector3F32&)triResult.GetIntersectionPoint() - (Aftk::Vector3F32&)m_origin;

						F32 distanceSq = delta[0]*delta[0] + delta[1]*delta[1] + delta[2]*delta[2];
						if (!foundClosestIntersection || distanceSq < closestIntersection)
						{
							rayResult.Set(
								triResult.GetIntersectionPoint(),
								pMesh, 
								iPoly,
								jTri);
							closestIntersection = distanceSq;
							foundClosestIntersection = true;
						}
					}
					else
					{
						//Return upon first hit.
						result.Set(
							triResult.GetIntersectionPoint(),
							pMesh, 
							iPoly,
							jTri);
						return;
					}
				}
			}
		}
	}

	result = rayResult;
}

void C_Ray::IntersectPlane(
	C_RayPlaneResult& result,
	const Position3& p0, 
	const Position3& p1,
	const Position3& p2) const
{
	Vector3F32 N = ((Vector3F32&)p1 - (Vector3F32&)p0) ^ ((Vector3F32&)p2 - (Vector3F32&)p0);
	UtMaths::SafeNormalise(&N);
	
    F32 a = -(N | (Vector3F32&)m_origin - (Vector3F32&)p0);
    F32 b = N | (Vector3F32&)m_direction;
	//Prevent a /0.
    if (fabsf(b) < AFTK_EPSILON)
	{
		result.Set(false);
		return;
    }

    //Find the distance along the ray direction at which we intersect our triangle.
    F32 d = a / b;
    if (d < 0.0)   
	{
		//C_Ray is aimed in the opposite direction to the triangle
		result.Set(false);
		return;
	}
 
	//We are intersecting the triangle.
	result.Set(true, d);
}
