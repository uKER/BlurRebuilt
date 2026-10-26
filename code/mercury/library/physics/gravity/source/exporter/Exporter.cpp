#include "Stdafx.h"
#include "gravity/exporter/Exporter.h"
#include "gravity/exporter/GravityExporter.h"
#include "misc_shared/Geometry/ConvexHull.h"
#include "gravity/exporter/ConvexHullGeneration.h"
//could probably do with not referencing graphics code here, but not much else I can do at the moment!
#if CFG_CONVERTER
#include "renderingdata\LodMesh.h"
#endif

namespace Gravity
{

	extern Debug::Logger& g_Log;
	
	//Debug::Logger &g_Log = Debug::GetLogger("gravity.export");

	Gravity::SphericalJointDescriptor* C_Exporter::addSpherialJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF anchorB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::SphericalJointDescriptor* pDesc=new Gravity::SphericalJointDescriptor;
		pDesc->m_Id = constraintId;
		pDesc->m_PhysicsBodyAID=bodyA_id;
		pDesc->m_PhysicsBodyBID=bodyB_id;
		pDesc->m_AnchorPoint1=anchorA;
		pDesc->m_AnchorPoint2=anchorB;
		pDesc->m_PhysicsBodyAState=bodyAState;
		pDesc->m_PhysicsBodyBState=bodyBState;
		return pDesc;
	}


	Gravity::RevoluteJointDescriptor* C_Exporter::addRevoluteJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF anchorA, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF anchorB, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::RevoluteJointDescriptor* pDesc=new Gravity::RevoluteJointDescriptor;
		pDesc->m_Id = constraintId;
		pDesc->m_PhysicsBodyAID=bodyA_id;
		pDesc->m_PhysicsBodyBID=bodyB_id;
		pDesc->m_AnchorPoint1=anchorA;
		pDesc->m_AnchorPoint2=anchorB;
		pDesc->m_JointAxis1=jointAxisA;
		pDesc->m_JointAxis2=jointAxisB;
		pDesc->m_PhysicsBodyAState=bodyAState;
		pDesc->m_PhysicsBodyBState=bodyBState;
		return pDesc;
	}


	Gravity::PrismaticJointDescriptor* C_Exporter::addPrismaticJoint(const S32 constraintId, const S32 bodyA_id, const S32 bodyB_id, const Maths::HVec3 PCREF jointAxisA, const Maths::HVec3 PCREF jointAxisB, const U32 bodyAState, const U32 bodyBState)
	{
		Gravity::PrismaticJointDescriptor* pDesc=new Gravity::PrismaticJointDescriptor;
		pDesc->m_Id = constraintId;
		pDesc->m_PhysicsBodyAID=bodyA_id;
		pDesc->m_PhysicsBodyBID=bodyB_id;
		pDesc->m_JointAxis1=jointAxisA;
		pDesc->m_JointAxis2=jointAxisB;
		pDesc->m_PhysicsBodyAState=bodyAState;
		pDesc->m_PhysicsBodyBState=bodyBState;
		return pDesc;
	}


	Gravity::C_SphereDesc* C_Exporter::AddSphere(const S32 id, const Maths::HF32 PCREF radius, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_SphereDesc* pDesc = new Gravity::C_SphereDesc;
		pDesc->m_OriginalExportID=id;
		pDesc->m_Radius=radius;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		return pDesc;
	}


	Gravity::C_BoxDesc* C_Exporter::AddOBB(const S32 id, const Maths::HVec3 PCREF halfExtents, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_BoxDesc* pDesc=new Gravity::C_BoxDesc;
		pDesc->m_HalfExtents=halfExtents;
		Maths::HF32 margin(Maths::Min(halfExtents.X(), Maths::Min(halfExtents.Y(), halfExtents.Z())));
		margin *= 0.2f;
		pDesc->m_Margin = margin;
		pDesc->m_Epsilon = margin * 0.005f;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		return pDesc;
		
	}


	Gravity::C_CapsuleDesc* C_Exporter::AddCapsule(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		Gravity::C_CapsuleDesc* pDesc=new Gravity::C_CapsuleDesc;
		pDesc->m_Radius=radius;
		pDesc->m_Height=height;
		pDesc->m_Epsilon = Maths::Max(height, radius) * 0.005f;
		pDesc->m_Margin = radius;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		return pDesc;

	}


	Gravity::C_CylinderDesc* C_Exporter::AddCylinder(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{

		Gravity::C_CylinderDesc* pDesc=new Gravity::C_CylinderDesc;
		pDesc->m_Radius=radius;
		pDesc->m_Height=height;
		
		const Maths::HF32 margin(Maths::Min(radius, height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, height) * 0.001f);
		pDesc->m_Margin = margin;
		pDesc->m_Epsilon = epsilon;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		return pDesc;
	}

	Gravity::C_ConeDesc* C_Exporter::AddCone(const S32 id, const Maths::HF32 PCREF radius, const Maths::HF32 PCREF height, const U16 physicsMaterial, const U16 effectMaterial,  const U32 collisionFlag, const U32 nodeFlag, const F32 mass,
		const U32 userDataFlag)
	{

		Gravity::C_ConeDesc* pDesc=new Gravity::C_ConeDesc;
		pDesc->m_BaseRadius=radius;
		pDesc->m_Height=height;
		
		const Maths::HF32 margin(Maths::Min(radius, height) * 0.2f);
		const Maths::HF32 epsilon(Maths::Max(radius, height) * 0.001f);
		pDesc->m_Margin = margin;
		pDesc->m_Epsilon = epsilon;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag;
		return pDesc;
	}


	//Construct the triangle mesh for shootable, collidable and spline
	void C_Exporter::AddTriangleStaticMesh(Baking::AABBTree& tree, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*> & tris)
	{
		for(S32 j=0; j<tris.m_numElements; ++j)
		{
			C_ConverterTriangle* pTri=tris[j];
			S32 indexStart = tree.GetNumVertices();

			Baking::AABBBuildTreeTriangle tri;
			tri.m_IndexA = indexStart;
			tri.m_IndexB = indexStart + 1;
			tri.m_IndexC = indexStart + 2;
		
			tri.m_PhysicsMaterialIndex = pTri->m_PhysicsMaterialIndex;
			tri.m_EffectMaterialIndex  = pTri->m_EffectMaterialIndex;
			tri.m_CollisionFlags = pTri->m_CollisionFlag;
			tri.m_UserDataFlags= pTri->m_UserDataFlag;
			tree.AddTriangle(tri);
			tree.AddVertex(pTri->vertexA);
			tree.AddVertex(pTri->vertexB);
			tree.AddVertex(pTri->vertexC);
		}
		
	}


	void C_Exporter::AddTriangleMesh(Baking::AABBTree& tree, const Maths::HMat43 PCREF transform, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle* >& tris)
	{
		for(S32 j=0; j<tris.m_numElements; ++j)
		{
			C_ConverterTriangle* pTri=tris[j];
			S32 indexStart = tree.GetNumVertices();

			Baking::AABBBuildTreeTriangle tri;
			tri.m_IndexA = indexStart;
			tri.m_IndexB = indexStart + 1;
			tri.m_IndexC = indexStart + 2;

			const Maths::HMat43 inv_trans=transform.InverseRotTrans();
			Maths::HVec3 vertexA = pTri->vertexA * inv_trans ;
			Maths::HVec3 vertexB = pTri->vertexB * inv_trans ;
			Maths::HVec3 vertexC = pTri->vertexC * inv_trans ;
		
			tri.m_PhysicsMaterialIndex = pTri->m_PhysicsMaterialIndex;
			tri.m_EffectMaterialIndex  = pTri->m_EffectMaterialIndex;
			tri.m_CollisionFlags = pTri->m_CollisionFlag;
			tri.m_UserDataFlags= pTri->m_UserDataFlag;
			tree.AddTriangle(tri);
			tree.AddVertex(vertexA);
			tree.AddVertex(vertexB);
			tree.AddVertex(vertexC);
		}

	}

	Gravity::C_TriangleMeshDesc* C_Exporter::CreateTriangleStaticMesh()
	{
		return new Gravity::C_TriangleMeshDesc;

	}

	Gravity::C_ClothDesc* C_Exporter::CreateCloth()
	{
		return new Gravity::C_ClothDesc;
	}

	Gravity::C_DeformableDesc* C_Exporter::CreateDeformable()  
	{
		return new Gravity::C_DeformableDesc;
	}

	Gravity::C_FluidMeshDesc* C_Exporter::AddFluidMesh(const S32 id, const F32 depth, const F32 splitRatio, const F32 normThresh,  const U16 physicsMaterial, const U16 effectMaterial, const U32 nodeFlag,  const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_FluidMeshDesc* pDesc =new Gravity::C_FluidMeshDesc;
		pDesc->m_OriginalExportID = id;
		pDesc->m_Depth = depth;
		pDesc->m_PhysicsMaterialIndex = physicsMaterial;
		pDesc->m_EffectMaterialIndex = effectMaterial;
		pDesc->m_NodeFlag = nodeFlag;
		pDesc->m_Density = 300.f;
		pDesc->m_AngularDrag = 0.8f;
		pDesc->m_LinearDrag = 4.f;
		pDesc->m_MaxBuoyancy = 25.f;
		pDesc->m_Gravity = 9.8f;

		for(S32 j=0; j<tris.m_numElements; ++j)
		{
			C_ConverterTriangle* pTri=tris[j];
			S32 indexStart = pDesc->m_AABBTree->GetNumVertices();

			Baking::AABBBuildTreeTriangle tri;
			tri.m_IndexA = indexStart;
			tri.m_IndexB = indexStart + 1;
			tri.m_IndexC = indexStart + 2;
		
			tri.m_PhysicsMaterialIndex= pTri->m_PhysicsMaterialIndex;
			tri.m_EffectMaterialIndex = pTri->m_EffectMaterialIndex;
			tri.m_CollisionFlags = pTri->m_CollisionFlag;
			tri.m_UserDataFlags= pTri->m_UserDataFlag;
			pDesc->m_AABBTree->AddTriangle(tri);
			pDesc->m_AABBTree->AddVertex(pTri->vertexA);
			pDesc->m_AABBTree->AddVertex(pTri->vertexB);
			pDesc->m_AABBTree->AddVertex(pTri->vertexC);
		}
		pDesc->m_AABBTree->setSplitRatio(splitRatio);
		pDesc->m_AABBTree->BuildAABBTree(Maths::HF32(normThresh));

		pDesc->m_CollisionFlag = pDesc->GetCombinedCollisionFlags();

		return pDesc;
	}

	Gravity::C_TriangleMeshBaseDesc* C_Exporter::AddInstanceBase(const S32 id, const F32 splitRatio, const F32 normThresh, const SharedPhysics::SortedArray<Gravity::C_ConverterTriangle*>& tris)
	{
		Gravity::C_TriangleMeshBaseDesc* pDesc=new Gravity::C_TriangleMeshBaseDesc;
		pDesc->m_OriginalExportID = id;

		for(S32 j=0; j<tris.m_numElements; ++j)
		{
			C_ConverterTriangle* pTri=tris[j];
			S32 indexStart = pDesc->m_AABBTree->GetNumVertices();

			Baking::AABBBuildTreeTriangle tri;
			tri.m_IndexA = indexStart;
			tri.m_IndexB = indexStart + 1;
			tri.m_IndexC = indexStart + 2;
		
			tri.m_PhysicsMaterialIndex= pTri->m_PhysicsMaterialIndex;
			tri.m_EffectMaterialIndex = pTri->m_EffectMaterialIndex;
			tri.m_CollisionFlags = pTri->m_CollisionFlag;
			tri.m_UserDataFlags= pTri->m_UserDataFlag;
			pDesc->m_AABBTree->AddTriangle(tri);
			pDesc->m_AABBTree->AddVertex(pTri->vertexA);
			pDesc->m_AABBTree->AddVertex(pTri->vertexB);
			pDesc->m_AABBTree->AddVertex(pTri->vertexC);
		}
		pDesc->m_AABBTree->setSplitRatio(splitRatio);
		pDesc->m_AABBTree->BuildAABBTree(Maths::HF32(normThresh));

		return pDesc;
	}


	Gravity::C_InstancedTriangleMeshDesc* C_Exporter::AddInstance(const S32 id, const S32 baseNodeID)
	{
		Gravity::C_InstancedTriangleMeshDesc* pDesc=new Gravity::C_InstancedTriangleMeshDesc;
		pDesc->m_OriginalExportID = id;
		pDesc->m_BaseNodeId=baseNodeID;
		return pDesc;
	}

	enum E_PlaneSide
	{
		E_Back_Plane,
		E_Front_Plane,
		E_On_Plane
	};


	E_PlaneSide ClassifyPointToPlane(const Maths::HVec3& vertex, const Maths::C_Plane& plane)
	{
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
		const Maths::HF32 distance(plane.Distance(vertex));
		
		if (distance < -epsilon)
			return E_Back_Plane;
		if(distance > epsilon)
			return E_Front_Plane;
		return E_On_Plane;
	}

	Maths::HVec3 IntersectEdgeAgainstPlane(const Maths::HVec3 &a, const Maths::HVec3 &b, const Maths::C_Plane &p)
	{
		using namespace Maths;
		Maths::HVec3 rtn;

		// Compute t value at which the directed line ab intersects the plane
		HVec3 ab = b - a;
		HF32 fDist0 = (-p.DistanceToOrigin() - (p.Normal() | a));
		HF32 t = fDist0 / (p.Normal() | ab);

		// If t in [0..1] compute and return intersection point
		if (t >= 0.0f && t <= 1.0f)
		{
			rtn = a + (HF32)t * ab;
			return rtn;
		}

		assert(0);

		// Else t is +INF, -INF, NaN, or not in [0..1], so no intersection
		return rtn;
	}



	void SplitPolygon(const SharedPhysics::SortedArray<Maths::HVec3> &poly, const Maths::C_Plane &plane, SharedPhysics::SortedArray<Maths::HVec3> &backPoly)
	{
		using namespace Maths;

		backPoly.m_numElements = 0;;

		if (poly.m_numElements <= 0)
			return;

		// test all edges (a, b) starting with edge from last to first vertex
		int numVerts = poly.m_numElements;
		Maths::HVec3 aVert = poly[numVerts-1];
		//E_PlaneTest aSide = ClassifyPointToPlane(a, plane);
		E_PlaneSide aSide = ClassifyPointToPlane(aVert, plane);

		// loop over all edges given by vertex pair (n - 1, n)
		for (int n=0; n<numVerts; n++)
		{
			Maths::HVec3 bVert = poly[n];
			E_PlaneSide bSide = ClassifyPointToPlane(bVert, plane);

			if (bSide == E_Front_Plane)
			{
				if (aSide == E_Back_Plane)
				{
					// Edge (a, b) straddles, output intersection point to both sides
					HVec3 i = IntersectEdgeAgainstPlane(aVert, bVert, plane);

					backPoly.AddToEnd(&i);
				}
			}
			else if (bSide == E_Back_Plane)
			{
				if (aSide == E_Front_Plane)
				{
					// Edge (a, b) straddles plane, output intersection point to both sides
					HVec3 i = IntersectEdgeAgainstPlane(aVert, bVert, plane);
					backPoly.AddToEnd(&i);
				}
				else if (aSide == E_On_Plane)
				{
					// output a when edge (a, b) goes from 'on' to 'behind' the plane
					backPoly.AddToEnd(&aVert);
				}
				// in all three cases, output b to the back side
				backPoly.AddToEnd(&bVert);
			}
			else
			{
				// in one case, als output b to the back side
				if (aSide == E_Back_Plane)
				{
					backPoly.AddToEnd(&bVert);
				}
			}
			// keep b as starting point of next edge
			aVert = bVert;
			aSide = bSide;
		}
	}


	//void CalculatePolygons(Gravity::C_ConvexHullDesc* pDesc, SharedPhysics::SortedArray<Maths::C_Plane>& planes,
	//	const SharedPhysics::SortedArray<Maths::HVec3>& points, const Maths::HVec3 PCREF extents)
	//{
	//	using namespace Maths;

	//	SharedPhysics::SortedArray<Maths::HVec3> polygon, tmp;
	//	SharedPhysics::SortedArray<SharedPhysics::SortedArray<U32>*> polygons;
	//	

	//	for (int iPlane=0; iPlane<planes.m_numElements; iPlane++)
	//	{
	//		const C_Plane &plane = planes[iPlane];
	//		HVec3 vCentre = -plane.Normal() * plane.DistanceToOrigin();
	//		HVec3 p,q;
	//		const Maths::HVec3 norm(plane.Normal().m_V);
	//		norm.PerpendicularAxes(p, q);
	//		p = p * extents; // TODO: calculate the necessary scaling factor from the AABB
	//		q = q * extents;

	//		// create polygon in the plane
	//		polygon.m_numElements = 0;
	//		tmp.m_numElements = 0;
	//		Maths::HVec3 vtx;

	//		vtx = vCentre + p + q;		
	//		polygon.AddToEnd(&vtx);
	//		vtx = vCentre + p - q;		
	//		polygon.AddToEnd(&vtx);
	//		vtx = vCentre - p - q;		
	//		polygon.AddToEnd(&vtx);
	//		vtx = vCentre - p + q;		
	//		polygon.AddToEnd(&vtx);
	//		
	//		// clip the polygon against all the planes
	//		for (int i=0; i<planes.m_numElements; i++)
	//		{
	//			if (i == iPlane)
	//				continue;
	//			C_Plane clipPlane( planes[i] );
	//			SplitPolygon(polygon, clipPlane, tmp);
	//			polygon.m_numElements = 0;
	//			for(S32 p = 0; p < tmp.m_numElements; ++p)
	//			{
	//				polygon.AddToEnd(&tmp[p]);
	//			}
	//		}

	//		// replace the vertices of the polygon with indices into the point list
	//		SharedPhysics::SortedArray<U32> *idxpoly = new SharedPhysics::SortedArray<U32>;
	//		idxpoly->m_numElements = 0;
	//		//idxpoly.Resize(polygon.m_numElements);
	//		for (int i=0; i<polygon.m_numElements; i++)
	//		{
	//			U32 iBest = 0;
	//			float fDist = FLT_MAX;
	//			for (S32 j=0; j<points.m_numElements; j++)
	//			{
	//				HVec3 v = points[j] - polygon[i];
	//				float f = v.Length2();
	//				if (f < fDist)
	//				{
	//					iBest = (U32)j;
	//					fDist = f;
	//				}
	//			}

	//			idxpoly->AddToEnd(&iBest);
	//		}

	//		// remove duplicate vertices
	//		for (int i=0; i<idxpoly->m_numElements; i++)
	//		{
	//			if (idxpoly->m_pElems[i] == idxpoly->m_pElems[(i+1)%idxpoly->m_numElements])
	//			{
	//				idxpoly->Remove(i, 1);
	//				i--;
	//			}
	//		}

	//		// if the plane has no vertices for it's polygon, delete it
	//		if (idxpoly->m_numElements == 0)
	//		{
	//			//m_Planes.EraseByIndex(iPlane);
	//			planes.Remove(iPlane, 1);
	//			delete idxpoly;
	//			iPlane--;
	//			continue;
	//		}

	//		// store the polygon
	//		polygons.AddToEnd(&idxpoly);
	//	}

	//	//Last bit. We have index data, now let's build vertex list and triangle list...

	//	//(1) Create the unique vertex list...
	//	//(2) Create the polygons...
	//	//(3) Clear up the vertex data to avoid memory leaks...

	//	S32 currentIndex = 0;
	//	S32 *remapArray = new S32[points.m_numElements];

	//	for(S32 a = 0; a < points.m_numElements; ++a)
	//	{
	//		remapArray[a] = -1;
	//	}

	//	pDesc->m_Points.m_numElements = 0;

	//	for(S32 a = 0; a < polygons.m_numElements; ++a)
	//	{
	//		if(polygon.m_numElements >= 3)
	//		{
	//			SharedPhysics::SortedArray<U32> &idxpoly = *polygons[a];
	//			for(S32 b = 0; b < idxpoly.m_numElements; ++b)
	//			{
	//				U32 index = idxpoly[b];
	//				if(remapArray[index] == -1)
	//				{
	//					remapArray[index] = currentIndex++;
	//					pDesc->AddPoint(points[index]);
	//				}
	//			}

	//			Gravity::C_TriangleIndices inds;
	//			inds.m_IndexA = remapArray[idxpoly[0]];
	//			inds.m_IndexB = remapArray[idxpoly[1]];

	//			for(S32 b = 2; b < idxpoly.m_numElements; b++)
	//			{
	//				inds.m_IndexC = remapArray[idxpoly[b]];
	//				//Add the triangle
	//				pDesc->AddTriangle(inds);
	//				inds.m_IndexB = inds.m_IndexC;
	//			}
	//		}

	//	}

	//	for(S32 a = 0; a < polygons.m_numElements; ++a)
	//	{
	//		delete polygons[a];
	//	}
	//	delete [] remapArray;
	//}

	void CalculatePolygons(Gravity::C_ConvexHullDesc* pDesc, SharedPhysics::SortedArray<Maths::C_Plane>& planes,
		const SharedPhysics::SortedArray<Maths::HVec3>& points, const Maths::HVec3 PCREF extents)
	{
		using namespace Maths;

		SharedPhysics::SortedArray<Maths::HVec3> polygon, tmp;
		SharedPhysics::SortedArray<SharedPhysics::SortedArray<U32>*> polygons;
		const Maths::HF32 extLen(extents.Length());
		

		for (int iPlane=0; iPlane<planes.m_numElements; iPlane++)
		{
			const C_Plane &plane = planes[iPlane];
			HVec3 vCentre = -plane.Normal() * plane.DistanceToOrigin();
			HVec3 p,q;
			const Maths::HVec3 norm(plane.Normal().m_V);
			norm.PerpendicularAxes(p, q);
			p = p * extLen;//extents; 
			q = q * extLen;//extents;

			// create polygon in the plane
			polygon.m_numElements = 0;
			tmp.m_numElements = 0;
			Maths::HVec3 vtx;

			vtx = vCentre + p + q;		
			polygon.AddToEnd(&vtx);
			vtx = vCentre + p - q;		
			polygon.AddToEnd(&vtx);
			vtx = vCentre - p - q;		
			polygon.AddToEnd(&vtx);
			vtx = vCentre - p + q;		
			polygon.AddToEnd(&vtx);
			
			// clip the polygon against all the planes
			for (int i=0; i<planes.m_numElements; i++)
			{
				if (i == iPlane)
					continue;
				C_Plane clipPlane( planes[i] );
				SplitPolygon(polygon, clipPlane, tmp);
				polygon.m_numElements = 0;
				for(S32 p = 0; p < tmp.m_numElements; ++p)
				{
					polygon.AddToEnd(&tmp[p]);
				}
			}

			// replace the vertices of the polygon with indices into the point list
			SharedPhysics::SortedArray<U32> *idxpoly = new SharedPhysics::SortedArray<U32>;
			idxpoly->m_numElements = 0;
			//idxpoly.Resize(polygon.m_numElements);
			for (int i=0; i<polygon.m_numElements; i++)
			{
				U32 iBest = 0;
				float fDist = FLT_MAX;
				for (S32 j=0; j<points.m_numElements; j++)
				{
					HVec3 v = points[j] - polygon[i];
					float f = v.Length2();
					if (f < fDist)
					{
						iBest = (U32)j;
						fDist = f;
					}
				}

				bool bAdd = true;
				for(S32 p = 0; p < idxpoly->m_numElements; ++p)
				{
					if(idxpoly->m_pElems[p] == iBest)
					{
						bAdd = false;
						break;
					}
				}
				if(bAdd)
				{
					idxpoly->AddToEnd(&iBest);
				}
			}

			//// remove duplicate vertices
			//for (int i=0; i<idxpoly->m_numElements; i++)
			//{
			//	if (idxpoly->m_pElems[i] == idxpoly->m_pElems[(i+1)%idxpoly->m_numElements])
			//	{
			//		idxpoly->Remove(i, 1);
			//		i--;
			//	}
			//}

			// if the plane has no vertices for it's polygon, delete it
			if (idxpoly->m_numElements < 3)
			{
				//m_Planes.EraseByIndex(iPlane);
				planes.Remove(iPlane, 1);
				delete idxpoly;
				iPlane--;
				continue;
			}

			// store the polygon
			polygons.AddToEnd(&idxpoly);
		}

		//Last bit. We have index data, now let's build vertex list and triangle list...

		//(1) Create the unique vertex list...
		//(2) Create the polygons...
		//(3) Clear up the vertex data to avoid memory leaks...

		S32 currentIndex = 0;
		S32 *remapArray = new S32[points.m_numElements];

		for(S32 a = 0; a < points.m_numElements; ++a)
		{
			remapArray[a] = -1;
		}

		pDesc->m_Points.m_numElements = 0;
		pDesc->m_Triangles.m_numElements = 0;

		for(S32 a = 0; a < polygons.m_numElements; ++a)
		{
			if(polygon.m_numElements >= 3)
			{
				SharedPhysics::SortedArray<U32> &idxpoly = *polygons[a];
				for(S32 b = 0; b < idxpoly.m_numElements; ++b)
				{
					U32 index = idxpoly[b];
					if(remapArray[index] == -1)
					{
						remapArray[index] = currentIndex++;
						pDesc->AddPoint(points[index]);
					}
				}

				Gravity::C_TriangleIndices inds;
				inds.m_IndexA = remapArray[idxpoly[0]];
				inds.m_IndexC = remapArray[idxpoly[1]];

				for(S32 b = 2; b < idxpoly.m_numElements; b++)
				{
					inds.m_IndexB = remapArray[idxpoly[b]];
					//Add the triangle
					pDesc->AddTriangle(inds);
					inds.m_IndexC = inds.m_IndexB;
				}
			}

		}

		for(S32 a = 0; a < polygons.m_numElements; ++a)
		{
			delete polygons[a];
		}
		delete [] remapArray;
	}




	void ConstructBruteForceHull(Gravity::C_ConvexHullDesc* pDesc, const SharedPhysics::SortedArray<Maths::HVec3>& points)
	{

		Maths::HVec3 low(Maths::Constants<Maths::HVec3>::Max());
		Maths::HVec3 high(-Maths::Constants<Maths::HVec3>::Max());

		using namespace Maths;
		SharedPhysics::SortedArray<Maths::C_Plane> planes;
		for (int i1=0; i1<points.m_numElements; i1++)
		{
			low = Maths::Min(low, points[i1]);
			high = Maths::Max(high, points[i1]);
			for (int i2=i1+1; i2<points.m_numElements; i2++)
			{
				for (int i3=i2+1; i3<points.m_numElements; i3++)
				{
					/*if (i1 == i2 || i1 == i3 || i2 == i3)
						continue;*/

					C_Plane plane(points[i1], points[i2], points[i3]);

					// does the plane already exist?
					bool bAddPlane = true;
					for (int i=0; i<planes.m_numElements; i++)
					{
						HVec3 v = (HVec3)planes[i].Normal() - (HVec3)plane.Normal();
						if (v.Length() < 0.0000001f && 
							fabs(planes[i].DistanceToOrigin() - plane.DistanceToOrigin()) < 0.0001)
						{
							bAddPlane = false;
							break;
						}
					}
					if (!bAddPlane)
						continue;

					// do all points lie on one side of the plane?
					int iFront = 0, iBack = 0;
					for (int i=0; i<points.m_numElements; i++)
					{
						if (plane.Distance(points[i]) > 0.0001f)
						{
							iFront++;
						}
						else if (plane.Distance(points[i]) < -0.0001f)
						{
							iBack++;
						}
					}
					if (iBack == 0 && iFront>0)
					{
						plane = -plane;

						bAddPlane = true;
						for (int i=0; i<planes.m_numElements; i++)
						{
							HVec3 v = (HVec3)planes[i].Normal() - (HVec3)plane.Normal();
							if (v.Length() < 0.0000001f && 
								fabs(planes[i].DistanceToOrigin() - plane.DistanceToOrigin()) < 0.0001)
							{
								bAddPlane = false;
								break;
							}
						}
						if (!bAddPlane)
							continue;

					}
					else if (iFront == 0 && iBack>0)
					{
					}
					else
						continue;

					// add the plane
					planes.AddToEnd(&plane);
				}
			}
		}

		const Maths::HVec3 extents(high - low);


		//OK. Have the list of planes now!!! Now we need to construct the polygons...

		CalculatePolygons(pDesc, planes, points, extents);

	}


//#pragma optimize("", off)
//	Gravity::C_ConvexHullDesc* ConstructHull(Gravity::C_ConvexHullDesc* pDesc, const SharedPhysics::SortedArray<Maths::HVec3>& points)
//	{
//		ConvexHull::C_Mesh mesh;
//		for(S32 i=0; i<points.m_numElements; ++i)
//		{
//			//ConvexHull::C_Vertex vertex(&mesh, points[i]);
//			mesh.m_SourceVertexList.PushBack(points[i]);
//		
//		}
//
//		mesh.Build();
//
//		if(mesh.m_BuildState != ConvexHull::E_BuildState_Error)
//		{
//			Util::C_List<ConvexHull::C_Vertex>::C_Iterator iter(mesh.m_VertexList);
//			
//			//ConvexHull::C_Vertex* vert = iter.Begin();
//			pDesc->m_Triangles.m_numElements = 0;
//			pDesc->m_Points.m_numElements = 0;
//			for(S32 i = 0; i < mesh.m_VertexList.Size(); ++i, iter++)
//			{
//				ConvexHull::C_Vertex* vert(iter);
//				pDesc->AddPoint(vert->m_Pos);
//			}
//
//			if(pDesc->m_Points.m_numElements < 4)
//			{
//				delete pDesc;
//				return NULL;
//			}
//
//			
//
//			Util::C_List<ConvexHull::C_Polygon>::C_Iterator iter2(mesh.m_PolygonList);
//			for(S32 i = 0; i < mesh.m_PolygonList.Size(); ++i, iter2++)
//			{
//				ConvexHull::C_Polygon* poly = iter2;
//
//				ConvexHull::C_Vertex* firstPos, *prevPos, *currPos;
//
//				ConvexHull::C_VertexItem* pFirstVertexItem = poly->m_VertexItemList.First();
//				ConvexHull::C_VertexItem* pPrevVertexItem = pFirstVertexItem->Next();
//				ConvexHull::C_VertexItem* pCurrVertexItem = pPrevVertexItem->Next();
//
//				firstPos = (ConvexHull::C_Vertex*)(*pFirstVertexItem);
//				prevPos = (ConvexHull::C_Vertex*)(*pPrevVertexItem);
//
//				while(pCurrVertexItem)
//				{
//					currPos = (ConvexHull::C_Vertex*)(*pCurrVertexItem);
//
//					//Now add these to the triangle index list!
//
//					//debugRenderPtr->Draw3dTriangle(firstPos, currPos, prevPos, (Maths::RGBA)col);
//
//					Gravity::C_TriangleIndices inds;
//					BIZ_ASSERT(firstPos->m_Index < mesh.m_VertexList.Size());
//					BIZ_ASSERT(currPos->m_Index < mesh.m_VertexList.Size());
//					BIZ_ASSERT(prevPos->m_Index < mesh.m_VertexList.Size());
//
//					inds.m_IndexA = firstPos->m_Index;
//					inds.m_IndexB = currPos->m_Index;
//					inds.m_IndexC = prevPos->m_Index;
//					pDesc->AddTriangle(inds);
//
//					prevPos = currPos;
//					pPrevVertexItem = pCurrVertexItem;
//					pCurrVertexItem = pCurrVertexItem->Next();
//				}
//			}
//		}
//		else
//		{
//			LOGWARN(g_Log, "Quick Hull Failed on this convex hull! Falling back on brute force hull generation");
//			ConstructBruteForceHull(pDesc, points);
//		}
//		return pDesc;
//	}
//#pragma optimize("", on)

	Gravity::C_ConvexHullDesc* ConstructHull(Gravity::C_ConvexHullDesc* pDesc, const SharedPhysics::SortedArray<Maths::HVec3>& points)
	{

		if(points.m_numElements < 4)
		{
			delete pDesc;
			return NULL;
		}

		/*ConstructBruteForceHull(pDesc, points);

		if(points.m_numElements < 4)
		{
			delete pDesc;
			return NULL;
		}*/


		Gravity::C_ConvexHullMesh mesh;
		GenerationState state = mesh.Build(points);

		if(state == E_Generation_Fail)
		{
			LOGWARN(g_Log, "the model can't be made into a convex hull, no convex hull will be generated");
			//delete pDesc;
			//return NULL;
		}

		pDesc->m_Triangles.m_numElements = 0;
		pDesc->m_Points.m_numElements = 0;
		/*for(S32 i = 0; i < mesh.m_Points.m_numElements; ++i)
		{
			pDesc->AddPoint(mesh.m_SupportPoints[mesh.m_Points[i]].m_Point);
		}*/
		
		S32 index = 0;
		Maths::HVec3* pVertice = new Maths::HVec3[mesh.m_Points.m_numElements];
		S32 * remapAddress = new S32[mesh.m_SupportPoints.m_numElements];

		for(S32 a = 0; a < mesh.m_SupportPoints.m_numElements; ++a)
		{
			remapAddress[a] = -1;
		}


		for(S32 a = 0; a < mesh.m_Facets.m_numElements; ++a)
		{
			Gravity::C_Facet* pFacet = mesh.m_Facets[a];
			if(!pFacet->m_Discarded)
			{
				if(remapAddress[mesh.m_Points[pFacet->m_indice[0]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[0]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[0]]] = index++;
				}
				if(remapAddress[mesh.m_Points[pFacet->m_indice[1]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[1]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[1]]] = index++;
				}
				if(remapAddress[mesh.m_Points[pFacet->m_indice[2]]] == -1)
				{
					pVertice[index] = mesh.m_SupportPoints[mesh.m_Points[pFacet->m_indice[2]]].m_Point;
					remapAddress[mesh.m_Points[pFacet->m_indice[2]]] = index++;
				}

				//Remap facet indices...
				/*U32 oldIndexA = pFacet->m_indice[0];
				U32 oldIndexB = pFacet->m_indice[1];
				U32 oldIndexC = pFacet->m_indice[2];
				pFacet->m_indice[0] = (U32)remapAddress[m_Points[pFacet->m_indice[0]]];
				pFacet->m_indice[1] = (U32)remapAddress[m_Points[pFacet->m_indice[1]]];
				pFacet->m_indice[2] = (U32)remapAddress[m_Points[pFacet->m_indice[2]]];

				BIZ_ASSERT(pFacet->m_indice[0] < index);
				BIZ_ASSERT(pFacet->m_indice[1] < index);
				BIZ_ASSERT(pFacet->m_indice[2] < index);*/

			}
		}

		/*mesh.m_ResultPoints.Resize(index);
		mesh.m_ResultPoints.m_numElements = index;
		for(S32 a = 0; a < index; ++a)
		{
			mesh.m_ResultPoints[a] = pVertice[a];
		}*/

	

		/*for(S32 i = 0; i < mesh.m_ResultPoints.m_numElements; ++i)
		{
			pDesc->AddPoint(mesh.m_ResultPoints[i]);
		}*/

		if(index < 4)
		{
			delete pDesc;
			return NULL;
		}

		for(S32 i = 0; i < index; ++i)
		{
			pDesc->AddPoint(pVertice[i]);
		}
		
		for(S32 i = 0; i < mesh.m_Facets.m_numElements; ++i)
		{
			Gravity::C_Facet* facet = mesh.m_Facets[i];
			if(!facet->IsDistarded())
			{
				Gravity::C_TriangleIndices inds;
				inds.m_IndexA = remapAddress[mesh.m_Points[facet->m_indice[0]]];
				inds.m_IndexB = remapAddress[mesh.m_Points[facet->m_indice[2]]];
				inds.m_IndexC = remapAddress[mesh.m_Points[facet->m_indice[1]]];
				pDesc->AddTriangle(inds);
			}
		}
		
		delete [] pVertice;
		delete [] remapAddress;

	//	BIZ_ASSERT(pDesc->m_Points.m_numElements <=512 && pDesc->m_Triangles.m_numElements <=128);

		return pDesc;
	}




	Gravity::C_ConvexHullDesc* C_Exporter::AddConvexHull(const S32 id, const SharedPhysics::SortedArray<Maths::HVec3> & points, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass, const U32 userDataFlag)
	{
		using namespace Maths;
		Gravity::C_ConvexHullDesc* pDesc=new Gravity::C_ConvexHullDesc;

		pDesc = ConstructHull(pDesc, points); 

		if(pDesc == NULL)
			return NULL;

#if 0 && CFG_CONVERTER
		const U32 targetPolyCount = m_MaxConvexHullTriangles;
		if(pDesc->m_Triangles.m_numElements > targetPolyCount)
		{
			//LOGWARN(g_Log, "Warning - convex hull has %i polygons; Converter will reduce to approx %i polygons", pDesc->m_Triangles.m_numElements, targetPolyCount);
			LOGWARN(g_Log, "Warning - convex hull has %i polygons; Converter will reduce to %i polygons", pDesc->m_Triangles.m_numElements, targetPolyCount);

			RenderingData::Build::LodMesh lodMesh;
			//Add vertices / triangles into lod mesh...
			U32* remapVertexArray = new U32[pDesc->m_Points.m_numElements];

			//Reduction rate...
			//const F32 reductionRate = (F32)targetPolyCount/(F32)pDesc->m_Triangles.m_numElements;
			
			Maths::Vector3 m_Min(Maths::Constants<Maths::HVec3>::Max());
			Maths::Vector3 m_Max(-Maths::Constants<Maths::HVec3>::Max());

			for(S32 a = 0; a<pDesc->m_Points.m_numElements; ++a)
			{
				Maths::Vector3 point(pDesc->m_Points[a]);
				m_Min = Maths::Min(point, m_Min);
				m_Max = Maths::Max(point, m_Max);
			}

			Maths::AxisBox box;
			box.m_Min = m_Min;
			box.m_Max = m_Max;
			lodMesh.SetBoundingBox( box );


			for(S32 a = 0; a < pDesc->m_Points.m_numElements; ++a)
			{
				//Add in vertices + work out remaAddVertexp array...
				//remapVertexArray[a] = lodMesh.AddVertex(Maths::MakePoint3((Maths::Vector3)pDesc->m_Points[a]));
				remapVertexArray[a] = lodMesh.AddVertex((Maths::Point3)pDesc->m_Points[a]);
			}

		

			for(S32 a = 0; a < pDesc->m_Triangles.m_numElements; ++a)
			{
				//Does this need to be remapped?
				U32 edgeIndex0 = lodMesh.AddEdge(remapVertexArray[pDesc->m_Triangles[a].m_IndexA], remapVertexArray[pDesc->m_Triangles[a].m_IndexB]);
				U32 edgeIndex1 = lodMesh.AddEdge(remapVertexArray[pDesc->m_Triangles[a].m_IndexB], remapVertexArray[pDesc->m_Triangles[a].m_IndexC]);
				U32 edgeIndex2 = lodMesh.AddEdge(remapVertexArray[pDesc->m_Triangles[a].m_IndexA], remapVertexArray[pDesc->m_Triangles[a].m_IndexC]);
				U32 triangleIndex = lodMesh.AddTriangle(&pDesc->m_Triangles[0], pDesc->m_Points.m_numElements, remapVertexArray[pDesc->m_Triangles[a].m_IndexA],
					remapVertexArray[pDesc->m_Triangles[a].m_IndexB], remapVertexArray[pDesc->m_Triangles[a].m_IndexC]);

				RenderingData::Build::LodTriangle& triangle( lodMesh.Triangle( triangleIndex ) );
				triangle.SetVertexIndices( remapVertexArray[pDesc->m_Triangles[a].m_IndexA], remapVertexArray[pDesc->m_Triangles[a].m_IndexB], remapVertexArray[pDesc->m_Triangles[a].m_IndexC] );
	            triangle.SetEdgeIndices( edgeIndex0, edgeIndex1, edgeIndex2 );

				RenderingData::Build::LodVertex& vertex0( lodMesh.Vertex( remapVertexArray[pDesc->m_Triangles[a].m_IndexA] ) );
	            RenderingData::Build::LodVertex& vertex1( lodMesh.Vertex( remapVertexArray[pDesc->m_Triangles[a].m_IndexB] ) );
	            RenderingData::Build::LodVertex& vertex2( lodMesh.Vertex( remapVertexArray[pDesc->m_Triangles[a].m_IndexC] ) );
	            
	            RenderingData::Build::LodEdge& edge0( lodMesh.Edge( edgeIndex0 ) );
	            RenderingData::Build::LodEdge& edge1( lodMesh.Edge( edgeIndex1 ) );
	            RenderingData::Build::LodEdge& edge2( lodMesh.Edge( edgeIndex2 ) );
	            
                vertex0.AddTriangleIndex( triangleIndex );
                vertex1.AddTriangleIndex( triangleIndex );
                vertex2.AddTriangleIndex( triangleIndex );
	            
	            edge0.AddTriangleIndex( triangleIndex );
	            edge1.AddTriangleIndex( triangleIndex );
	            edge2.AddTriangleIndex( triangleIndex );
			}

			//OK. So now, we have to remove x triangles. Unfortunately, the algorithm removes vertices, not triangles, 
			//so there will be some work in getting the right results...For now, let's just remove 10% of vertices at a go...

			RenderingData::Build::LodMeshOptions options;
			//options.minimumVertices = 20; //Don't reduce past this...just too shallow detail otherwise...
			//options.vertexReductionRatio = reductionRate;
			options.minimumTriangles = targetPolyCount;
			options.triangleReductionRatio = 0.0f;
			options.reduceVertices = false;
			options.reduceTriangles = true;
			//options.preserveConvexHullIntegrity = true;
			//options.convexThreshold = 0.5f;
			//options.convexThreshold = Distance( MakePoint3( m_Min ), MakePoint3( m_Max ) ) * 0.1f;
			lodMesh.SetOptions( options );

			//U32 maxPasses = 3;
			//for(U32 a = 0; a < maxPasses; ++a)
			//{
			//	if(lodMesh.TriangleCount() < targetPolyCount || 
			//		lodMesh.VertexCount() < options.minimumVertices)
			//	{
			//		break;
			//	}
				lodMesh.AnalyseMesh();
				lodMesh.Reduce();
			//	//Reconstruct convex hull out of reduced vertex data (might not be convex anymore - although really should be)
			//}

			//Copy vertex data back into point array and reconstruct convex hull!
			SharedPhysics::SortedArray<Maths::HVec3> newPoints;
			for(S32 a = 0; a < lodMesh.VertexCount(); ++a)
			{
				RenderingData::Build::LodVertex& vertex0 = lodMesh.Vertex(a);
				Maths::HVec3 v(vertex0.GetVertex());
				newPoints.AddToEnd(&v);				
			}

		
			pDesc = ConstructHull(pDesc, newPoints); 
			if(pDesc == NULL)
				return NULL;
		}
#endif
 
		//BIZ_ASSERT(pDesc->m_Triangles.m_numElements <= 128  && pDesc->m_Points.m_numElements <= 512);

		Collision::C_CRAxisBox box(pDesc->GetBoundingBox());
		Maths::HF32 margin(Maths::Min(box.extents.X(), Maths::Min(box.extents.Y(), box.extents.Z())));
		margin *= 0.2f;
		pDesc->m_Margin = margin;
		pDesc->m_Epsilon = margin * 0.005f;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex = effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		pDesc->m_UserDataFlag = userDataFlag;
		return pDesc;

	}


	
	Gravity::C_CompositeDesc* C_Exporter::AddCompositeObjects(const S32 id, C_CompositeDesc* composite, const U16 physicsMaterial, const U16 effectMaterial, const U32 collisionFlag, const U32 nodeFlag, const F32 mass)
	{
		Gravity::C_CompositeDesc* pDesc=composite;
		pDesc->m_OriginalExportID=id;
		pDesc->m_PhysicsMaterialIndex=physicsMaterial;
		pDesc->m_EffectMaterialIndex =effectMaterial;
		pDesc->m_CollisionFlag=collisionFlag;
		pDesc->m_NodeFlag=nodeFlag;
		return pDesc;
	}
	
	Gravity::C_CompositeDesc* C_Exporter::AddCompositeObjects()
	{
		return new Gravity::C_CompositeDesc;
	}
};
