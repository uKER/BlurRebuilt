#include "AftkRenderPch.h"

#include "common/Renderer.h"
#include "common/Scene.h"
#include "common/SceneGraphNode.h"

#include "mesh/MeshElements.h"
#include "mesh/Mesh.h"
#include "mesh/MeshSceneGraphNode.h"
#include "mesh/Ray.h"

using namespace AftkRender;

void AftkRender::IntersectMesh(
	const C_Ray& ray,
	MeshIntersection &result,
	const C_MeshSceneGraphNode *pSgNode)
{
	const C_MeshResource *pResource = pSgNode->GetMeshResource();
	if (!pResource)
		return;

	//early test against bounding box;
	{
		const Maths::C_AxisBox& aabbox = pSgNode->GetWorldAABBox();

		Maths::HF32 t0 = -1.f, t1 =-1.f;
		if(!Maths::Intersect(Maths::C_Line(Maths::C_Vertex(ray.m_Origin), Maths::C_Normal(ray.m_Direction), 1000.f), aabbox, t0, t1))
			return;
	}

	//transform ray into objects local space...
	Maths::HMat44 invWorld = pSgNode->GetWorldMatrix().Inverse();

	C_Ray localSpaceRay(ray);
	Maths::HVec4 p0(ray.m_Origin.X(), ray.m_Origin.Y(), ray.m_Origin.Z(), 1.f);
	Maths::HVec4 p1(ray.m_Origin.X()+ray.m_Direction.X(), ray.m_Origin.Y()+ray.m_Direction.Y(), ray.m_Origin.Z()+ray.m_Direction.Z(), 1.f);

	p0 = p0*invWorld;
	p1 = p1*invWorld;
	p1 -= p0;
	p1.Normalise();

	C_Ray localRay(Maths::HVec3(p0.X(), p0.Y(), p0.Z()), Maths::HVec3(p1.X(), p1.Y(), p1.Z()));

	//distance used to discriminate hit results...we want the nearest intersection point
	F32 distance = Maths::Constants<F32>::Max();
	for (U32 i=0; i<pResource->GetNumClusters(); ++i)
	{
		const C_MeshClusterResource* pClusterResource = pResource->GetCluster(i);

		const F32* pPositions = (const F32*)pClusterResource->GetVertexDataElement(E_ElementType_Position);
		if (!pPositions)
			continue;

		if (pClusterResource->GetNumIndexDataElements())
		{
			//using index data...
			for (U32 j=0; j<pClusterResource->GetNumIndexDataElements(); ++j)
			{
				const C_MeshClusterResource::IndexDataElement *pIndexElement = pClusterResource->GetIndexDataElement(j);				
				U32 nPrimitives = GetNumPrimitives(pIndexElement->type, pIndexElement->count);

				if (pIndexElement->type==E_PrimitiveType_Unknown || pIndexElement->type==E_PrimitiveType_PointList ||
					pIndexElement->type==E_PrimitiveType_LineList || pIndexElement->type==E_PrimitiveType_LineStrip)
					continue;

				const U32 *pIndices = pIndexElement->pData;
				for (U32 k=0; k<nPrimitives; ++k)
				{
					U32 polyIndices[4];
					GetPrimitiveIndices(pIndexElement->type, k, polyIndices);

					TriangleIntersection hitResult;
					localRay.IntersectTriangle(hitResult,
						Maths::HVec3(pPositions[pIndices[polyIndices[0]]*3], pPositions[pIndices[polyIndices[0]]*3+1], pPositions[pIndices[polyIndices[0]]*3+2]),
						Maths::HVec3(pPositions[pIndices[polyIndices[1]]*3], pPositions[pIndices[polyIndices[1]]*3+1], pPositions[pIndices[polyIndices[1]]*3+2]),
						Maths::HVec3(pPositions[pIndices[polyIndices[2]]*3], pPositions[pIndices[polyIndices[2]]*3+1], pPositions[pIndices[polyIndices[2]]*3+2]));

					if (hitResult.hit && hitResult.distance < distance)
					{
						result.cluster = i;
						result.polygon = k;
						result.batch = j;
						result.intersection = hitResult.intersection;
						distance = hitResult.distance;
					}

					if (pIndexElement->type == E_PrimitiveType_QuadList)
					{
						localRay.IntersectTriangle(hitResult,
							Maths::HVec3(pPositions[pIndices[polyIndices[0]]*3], pPositions[pIndices[polyIndices[0]]*3+1], pPositions[pIndices[polyIndices[0]]*3+2]),
							Maths::HVec3(pPositions[pIndices[polyIndices[2]]*3], pPositions[pIndices[polyIndices[2]]*3+1], pPositions[pIndices[polyIndices[2]]*3+2]),
							Maths::HVec3(pPositions[pIndices[polyIndices[3]]*3], pPositions[pIndices[polyIndices[3]]*3+1], pPositions[pIndices[polyIndices[3]]*3+2]));

						if (hitResult.hit && hitResult.distance < distance)
						{
							result.cluster = i;
							result.polygon = k;
							result.batch = j;
							result.intersection = hitResult.intersection;
							distance = hitResult.distance;
						}
					}
				}
			}
		}
		else
		{
			//no index data
			E_PrimitiveType primitiveType = pClusterResource->GetNonIndexedPrimitiveType();
			if (primitiveType==E_PrimitiveType_Unknown	|| primitiveType==E_PrimitiveType_PointList ||
				primitiveType==E_PrimitiveType_LineList	|| primitiveType==E_PrimitiveType_LineStrip)
				continue;

			U32 nVertices = pClusterResource->GetNumVertices();
			U32 nPrimitives = GetNumPrimitives(primitiveType, nVertices);

			for (U32 j=0; j<nPrimitives; ++j)
			{
				U32 indices[4];
				GetPrimitiveIndices(primitiveType, j, indices);

				TriangleIntersection hitResult;
				localRay.IntersectTriangle(hitResult,
					Maths::HVec3(pPositions[indices[0]*3], pPositions[indices[0]*3+1], pPositions[indices[0]*3+2]),
					Maths::HVec3(pPositions[indices[1]*3], pPositions[indices[1]*3+1], pPositions[indices[1]*3+2]),
					Maths::HVec3(pPositions[indices[2]*3], pPositions[indices[2]*3+1], pPositions[indices[2]*3+2]));

				if (hitResult.hit && hitResult.distance < distance)
				{
					result.cluster = i;
					result.polygon = j;
					result.intersection = hitResult.intersection;
					distance = hitResult.distance;
				}
				
				if (primitiveType == E_PrimitiveType_QuadList)
				{
					localRay.IntersectTriangle(hitResult,
						Maths::HVec3(pPositions[indices[0]*3], pPositions[indices[0]*3+1], pPositions[indices[0]*3+2]),
						Maths::HVec3(pPositions[indices[2]*3], pPositions[indices[2]*3+1], pPositions[indices[2]*3+2]),
						Maths::HVec3(pPositions[indices[3]*3], pPositions[indices[3]*3+1], pPositions[indices[3]*3+2]));

					if (hitResult.hit && hitResult.distance < distance)
					{
						result.cluster = i;
						result.polygon = j;
						result.intersection = hitResult.intersection;
						distance = hitResult.distance;
					}
				}
			}
		}
	}
}

void C_Ray::IntersectTriangle(
	TriangleIntersection &result,
	const Maths::HVec3& p0,
	const Maths::HVec3& p1,
	const Maths::HVec3& p2) const
{
	F32 t, u, v;

	Maths::HVec3 p0p1(p1-p0), p0p2(p2-p0);

	Maths::HVec3 pVec = m_Direction ^ p0p2;

	F32 det = p0p1 | pVec;
	if (fabsf(det) < Maths::Constants<F32>::Epsilon())
	{
		result.hit = false;
		return;
	}

	F32 invdet = 1.0f/det;

	Maths::HVec3 tVec(m_Origin - p0);
	u = (tVec|pVec)*invdet;

	if (u<0.f || u>1.f)
	{
		result.hit = false;
		return;
	}

	Maths::HVec3 qVec = tVec ^ p0p1;
	v = (m_Direction|qVec)*invdet;
	if (v<0.f || u+v>1.f)
	{
		result.hit = false;
		return;
	}

	t = (p0p2|qVec)*invdet;

	result.hit = true;
	result.distance = t;
	result.intersection	=	p0*(1-(u+v));
	result.intersection +=	p0p1*u;
	result.intersection +=	p0p2*v;
}

void C_Ray::IntersectPlane(
	PlaneIntersection &result,
	const Maths::HVec3& p0,
	const Maths::HVec3& p1,
	const Maths::HVec3& p2) const
{
	Maths::HVec3 p0p1(p1-p0), p0p2(p2-p0);
	Maths::HVec3 n = p0p1^p0p2;
	n.Normalise();

	F32 a = -(n|(m_Origin-p0));
	F32 b = n|m_Direction;

	if (fabsf(b) < Maths::Constants<F32>::Epsilon())
	{
		result.hit = false;
		return;
	}

	result.distance = a/b;
	result.hit = (result.distance < 0.0f);
}