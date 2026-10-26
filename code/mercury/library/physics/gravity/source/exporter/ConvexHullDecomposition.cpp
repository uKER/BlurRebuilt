#include "stdafx.h"
#include "gravity/exporter/convexhulldecomposition.h"
#include "gravity/exporter/convexhullgeneration.h"
#include "gravity/exporter/chdbridge.h"


namespace Gravity
{

#pragma optimize("", off)
	C_ConvexHullMeshWrapper::~C_ConvexHullMeshWrapper()
	{
			if(m_ConvexHullMesh)
				delete m_ConvexHullMesh;
	}

	C_ConvexHullDecomposition::C_ConvexHullDecomposition(C_ConvexHullMesh* pBaseMesh): m_pBaseMesh(pBaseMesh), 
		m_MaxMeshWrapper(0), m_ConvexHullMeshes(NULL), m_Layer(0), m_ValidMeshCount(0)
	{
	}

	C_ConvexHullDecomposition::~C_ConvexHullDecomposition()
	{
		Reset();
	}

	void C_ConvexHullDecomposition::SetBaseMesh(C_ConvexHullMesh* pBaseMesh)
	{
		m_pBaseMesh = pBaseMesh;
	}

	void C_ConvexHullDecomposition::SetNumLayers(const U32 layer)
	{
		m_Layer = layer;

		if(m_ConvexHullMeshes)
		{
			/*for(U32 a = 0; a < m_MeshCount; ++a)
			{
				delete m_ConvexHullMeshes[a];
			}*/
			delete[] m_ConvexHullMeshes;
		}
		
		
		m_MaxMeshWrapper = Maths::Power<U32>(2, layer+1)-1;
		m_pBaseMesh = NULL;
		m_ConvexHullMeshes = new C_ConvexHullMeshWrapper[m_MaxMeshWrapper];
	}

	void C_ConvexHullDecomposition::Reset()
	{
		if(m_ConvexHullMeshes)
		{
			delete[] m_ConvexHullMeshes;
		}
		m_ConvexHullMeshes = NULL;
		m_MaxMeshWrapper = 0;
		m_Layer = 0;
		m_ValidMeshCount = 0;
		this->m_Bridges.m_numElements = 0;
		this->m_Edges.m_numElements = 0;
		this->m_Vertices.m_numElements =0;

	}

	//bool C_ConvexHullDecomposition::Decomposition(C_ConvexHullMesh* pBaseMesh, U32 count, Gravity::C_Point& startPoint, const Maths::C_Normal PCREF dir)
	//{
	//	using namespace Maths;

	//	if(count >= m_Layer )
	//	{
	//		m_ConvexHullMeshes[m_MeshCount++] = pBaseMesh;
	//		return true;
	//	}
	//	
	//	count++;

	//	const HF32 zero(Constants<HF32>::Zero());
	//	const HF32 one(Constants<HF32>::One());
	//
	//	if(startPoint.m_Facets.m_numElements == 0)
	//		return false;

	//	if(count == 3)
	//	{
	//		int bob =0;
	//	}

	//	C_Point supPoint = startPoint;
	//	C_Facet* pCurFacet = supPoint.m_Facets[0].m_pFacet;
	//	const U32 startGlobalIndex = pBaseMesh->m_Points[pCurFacet->m_indice[supPoint.m_Facets[0].m_Index]];
	//	const HF32 planeDist = (startPoint.m_Point|dir);
	//	S32 curGlobalIndex = startGlobalIndex;
	//	SharedPhysics::SortedArray<S32> silhouette;
	//	SharedPhysics::SortedArray<S32> rejectPoints;
	//	SharedPhysics::SortedArray<S32> silhouetteTrack;


	//	do
	//	{

	//		silhouette.Insert(&curGlobalIndex);
	//		silhouetteTrack.AddToEnd(&curGlobalIndex);
	//		S32 finalIndex = -1;
	//		S32 minIndex = -1;
	//		HF32 dist(Constants<HF32>::Max());
	//		HF32 minDist(zero);
	//		for(S32 i=0; i< supPoint.m_Facets.m_numElements; ++i)
	//		{
	//			C_PointFacetStructure& pointFacetInform = supPoint.m_Facets[i];
	//			if(!pointFacetInform.m_pFacet->m_Visited)
	//			{
	//				C_Facet* pCurFacet = pointFacetInform.m_pFacet;
	//				const U32 curFacetLocalIndex = pointFacetInform.m_Index;
	//				const U32 curFacetLocalIndex0 = Select(curFacetLocalIndex == 0, 1, Select(curFacetLocalIndex == 1, 0, 1));
	//				const U32 curFacetLocalIndex1 = Select(curFacetLocalIndex == 0, 2, Select(curFacetLocalIndex == 1, 2, 0));
	//				
	//				S32 globalIndex0 = pBaseMesh->m_Points[pCurFacet->m_indice[curFacetLocalIndex0]];
	//				S32 globalIndex1 = pBaseMesh->m_Points[pCurFacet->m_indice[curFacetLocalIndex1]];

	//				const HVec3 adjPoint0 = pBaseMesh->m_SupportPoints[globalIndex0].m_Point;
	//				const HVec3 adjPoint1 = pBaseMesh->m_SupportPoints[globalIndex1].m_Point;
	//				const HF32 dist0 = Abs((adjPoint0|dir) - planeDist);
	//				const HF32 dist1 = Abs((adjPoint1|dir) - planeDist);
	//				S32* index0 = rejectPoints.Find(&globalIndex0);
	//				S32* index1 = rejectPoints.Find(&globalIndex1);

	//				if((index0 == NULL) && (index1 != NULL))// || silhouette.Find(&globalIndex0))
	//				{
	//					minDist = dist1;
	//					minIndex = globalIndex1;
	//				}
	//				else if((index1 == NULL) && (index0!=NULL))// || silhouette.Find(&globalIndex1))
	//				{
	//					minDist = dist0;
	//					minIndex = globalIndex0;
	//					
	//				}
	//				else if(index1!=NULL && index0!=NULL)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					minDist = Min(dist0, dist1);
	//					minIndex = Select((bool)dist0.LT(dist1), globalIndex0, globalIndex1);
	//				}

	//				if(index0 && !index1)// || silhouette.Find(&globalIndex0))
	//				{
	//					minDist = dist1;
	//					minIndex = globalIndex1;
	//				}
	//				else if(index1 && !index0)// || silhouette.Find(&globalIndex1))
	//				{
	//					minDist = dist0;
	//					minIndex = globalIndex0;
	//					
	//				}
	//				else if(index0 && index1)
	//				{
	//					continue;
	//				}
	//				else
	//				{
	//					minDist = Min(dist0, dist1);
	//					minIndex = Select((bool)dist0.LT(dist1), globalIndex0, globalIndex1);
	//				}


	//				if(minDist.LT(dist))
	//				{
	//					//finalFacet = pCurFacet;
	//					finalIndex = minIndex;
	//					dist = minDist;
	//				}
	//			
	//			}
	//		}
	//		if(finalIndex == -1)
	//			break;

	//		//mark all the facets contain the finalIndex and the startGlobalIndex
	//		for(S32 i=0; i< supPoint.m_Facets.m_numElements; ++i)
	//		{
	//			C_PointFacetStructure& pointFacetInform = supPoint.m_Facets[i];
	//			C_Facet* pCurFacet = pointFacetInform.m_pFacet;
	//			const U32 curFacetLocalIndex = pointFacetInform.m_Index;
	//			const U32 curFacetLocalIndex0 = Select(curFacetLocalIndex == 0, 1, Select(curFacetLocalIndex == 1, 0, 1));
	//			const U32 curFacetLocalIndex1 = Select(curFacetLocalIndex == 0, 2, Select(curFacetLocalIndex == 1, 2, 0));
	//			
	//			S32 globalIndex0 = pBaseMesh->m_Points[pCurFacet->m_indice[curFacetLocalIndex0]];
	//			S32 globalIndex1 = pBaseMesh->m_Points[pCurFacet->m_indice[curFacetLocalIndex1]];
	//			BIZ_ASSERT(globalIndex0 != globalIndex1);
	//			if(globalIndex0 == finalIndex)
	//			{
	//				pCurFacet->m_Visited = true;
	//				rejectPoints.Insert(&globalIndex1);
	//			}

	//			if(globalIndex1 == finalIndex)
	//			{
	//				pCurFacet->m_Visited = true;
	//				rejectPoints.Insert(&globalIndex0);
	//			}
	//			
	//		}
	//		//finalFacet->m_Visited = true;
	//		curGlobalIndex = finalIndex;
	//		supPoint = pBaseMesh->m_SupportPoints[curGlobalIndex];
	//		//silhouette.Insert(&curGlobalIndex);	

	//	}while(curGlobalIndex != startGlobalIndex);

	//	SharedPhysics::SortedArray<Maths::HVec3> point0;
	//	SharedPhysics::SortedArray<Maths::HVec3> point1;

	//	//sperate the mesh into two
	//	for(S32 i = 0; i< pBaseMesh->m_Points.m_numElements; ++i)
	//	{
	//		const U32 globalIndex = pBaseMesh->m_Points[i];
	//		HVec3 p = pBaseMesh->m_SupportPoints[globalIndex].m_Point;
	//		const HF32 signDist = (p|dir) - planeDist;
	//		if(signDist.GE(zero))
	//			point0.AddToEnd(&p);
	//		else
	//			point1.AddToEnd(&p);
	//	}

	//	//put the silhouette point into the two points set
	//	for(S32 i=0; i<silhouette.m_numElements; ++i)
	//	{
	//		HVec3 p = pBaseMesh->m_SupportPoints[silhouette[i]].m_Point;
	//		const HF32 signDist = (p|dir) - planeDist;
	//		if(signDist.GE(zero))
	//			point1.AddToEnd(&p);
	//		else
	//			point0.AddToEnd(&p);
	//	}

	//	if(point0.m_numElements <= 4 || point1.m_numElements <=4)
	//		return false;

	//	if(count == 3)
	//	{
	//		static bool first = true;
	//		Btl::String strRes;
	//		if(first)
	//		{
	//			DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(startPoint.m_Point, 0.25f, RGBA(0, 255, 255));
	//			pBaseMesh->DebugRender(DbUtil::DefaultDebugRenderSys()->Persistent(), Maths::RGBA(255, 0, 255), true);
	//		
	//			for(S32 i=0; i< silhouetteTrack.m_numElements; ++i)
	//			{
	//				HVec3 curPoint = pBaseMesh->m_SupportPoints[silhouetteTrack.m_pElems[i]].m_Point;
	//				HVec3 nextPoint;
	//				if(i == silhouette.m_numElements - 1)
	//					nextPoint = pBaseMesh->m_SupportPoints[silhouetteTrack.m_pElems[0]].m_Point;
	//				else
	//					nextPoint=pBaseMesh->m_SupportPoints[silhouetteTrack.m_pElems[i+1]].m_Point;
	//				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dStar(curPoint, 0.15f, Maths::RGBA(0, 255, 0));
	//				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dLine(curPoint, nextPoint, Maths::RGBA(255, 0, 0));
	//				
	//				strRes = Btl::String::Format("%i", i);
	//				DbUtil::DefaultDebugRenderSys()->Persistent()->Draw3dText(curPoint, strRes);
	//			}
	//		}
	//		first = false;
	//	}

	//	bool succeeded0 = false;
	//	bool succeeded1 = false;
	//	C_ConvexHullMesh* mesh0 = new C_ConvexHullMesh;
	//	Gravity::GenerationState state0 = mesh0->Build(point0);

	//	if(state0 == Gravity::E_Generation_Succeed)
	//	{
	//		mesh0->Remap();
	//		succeeded0 = Decomposition(mesh0, count, mesh0->m_SupportPoints[0], dir);
	//	}
	//	
	//	C_ConvexHullMesh* mesh1 = new C_ConvexHullMesh;
	//	Gravity::GenerationState state1 = mesh1->Build(point1);
	//	if(state1 == Gravity::E_Generation_Succeed)
	//	{		
	//		mesh1->Remap();
	//		succeeded1 = Decomposition(mesh1, count, mesh1->m_SupportPoints[0], dir);
	//	}
	//	if(!succeeded0)delete mesh0;
	//	if(!succeeded1)delete mesh1;
	//	return false;
	//}

	Maths::HVec3 C_ConvexHullDecomposition::AddStaddlePoint(C_ConvexHullMesh* pBaseMesh,  C_Facet* pAdjFacet, U32 adjEdge, const Maths::C_Normal PCREF dir, const Maths::HVec3 PCREF point)
	{
		//Check weather the adjFacets staddle the plane as well
		//C_Facet* pAdjFacet = pCurFacet->m_adjFacets[staddleEdge];
		using namespace Maths;
		S32 index0 = pBaseMesh->m_Points[pAdjFacet->m_indice[0]];
		S32 index1 = pBaseMesh->m_Points[pAdjFacet->m_indice[1]];
		S32 index2 = pBaseMesh->m_Points[pAdjFacet->m_indice[2]];

		//U32 adjEdge = pCurFacet->m_adjEdges[staddleEdge];
		const U32 i0 = Select(adjEdge == 0, index0, Select(adjEdge == 1, index1, index2));
		const U32 i1 = Select(adjEdge == 0, index1, Select(adjEdge == 1, index2, index0));
		const U32 i2 = Select(adjEdge == 0, index2, Select(adjEdge == 1, index0, index1));

		const HVec3 temp0 = pBaseMesh->m_SupportPoints[i0].m_Point;
		const HVec3 temp1 = pBaseMesh->m_SupportPoints[i1].m_Point;
		const HVec3 temp2 = pBaseMesh->m_SupportPoints[i2].m_Point;

		const HF32 d0 = (temp0 - point)|dir;
		const HF32 d1 = (temp1 - point)|dir;
		const HF32 d2 = (temp2 - point)|dir;

		//const HF32 s0 = d0*d1;
		const HF32 s1 = d0*d2;
		const HF32 s2 = d1*d2;

		const HVec3 tp0 = Select(s1.LT(Constants<HF32>::Zero()), temp0, temp1);

		//Calcualte the intersect point
		const HVec3 a(tp0 - point);
		const HF32 b(-(a|dir));

		const HVec3 v(temp2 - tp0);
		const HF32 t0 = b/(v|dir);
		return tp0 + t0*v;
		
	}

	/*U32 C_ConvexHullDecomposition::GetNumOfConvexHull()
	{
		for(U32 i=1; i < m_MaxMeshWrapper; ++i)
		{
			if(m_ConvexHullMeshes[i].state == E_Decomposition_Succeed)
			{
				m_ValidMeshCount++;
			}
		}
	}*/

	void C_ConvexHullDecomposition::SetFailFlags(U32 index)
	{
		if(index < m_MaxMeshWrapper)
		{
			m_ConvexHullMeshes[index].state = E_Decomposition_Fail;
			SetFailFlags(index * 2 + 1);
			SetFailFlags(index * 2 + 2);
		}
	}


	C_DecompositionState C_ConvexHullDecomposition::Decomposition(C_ConvexHullMesh* pBaseMesh, U32 count, U32 parentIndex, Gravity::C_Point& startPoint, const Maths::C_Normal PCREF dir)
	{
		using namespace Maths;


		if(count >= m_Layer )
		{
			//m_ConvexHullMeshes[m_MeshCount++] = pBaseMesh;
			
			return E_Decomposition_Succeed;
		}
		
		count++;

		const HF32 zero(Constants<HF32>::Zero());
		const HF32 one(Constants<HF32>::One());
		const HF32 planeDist = (startPoint.m_Point|dir);
		const HF32 epsilon(Constants<HF32>::Epsilon());
	
	

		SharedPhysics::SortedArray<Maths::HVec3> point0;
		SharedPhysics::SortedArray<Maths::HVec3> point1;

		//sperate the mesh into two
		for(S32 i = 0; i< pBaseMesh->m_Points.m_numElements; ++i)
		{
			const U32 globalIndex = pBaseMesh->m_Points[i];
			HVec3 p = pBaseMesh->m_SupportPoints[globalIndex].m_Point;
			const HF32 signDist = (p|dir) - planeDist;
			if(signDist.GE(zero))
				point0.AddToEnd(&p);
			else
				point1.AddToEnd(&p);
		}

		if(point0.m_numElements <= 4 || point1.m_numElements <=4)
			return E_Decomposition_Succeed;

		for(S32 i=0; i<pBaseMesh->m_Facets.m_numElements; ++i)
		{
			C_Facet* pCurFacet = pBaseMesh->m_Facets[i];
			
		//	if(!pCurFacet->m_Discarded)
			//if(!pCurFacet->m_Visited)
			{
				//pCurFacet->m_Visited = true;
				S32 globalIndex0 = pBaseMesh->m_Points[pCurFacet->m_indice[0]];
				S32 globalIndex1 = pBaseMesh->m_Points[pCurFacet->m_indice[1]];
				S32 globalIndex2 = pBaseMesh->m_Points[pCurFacet->m_indice[2]];

				HVec3 adjPoint0 = pBaseMesh->m_SupportPoints[globalIndex0].m_Point;
				HVec3 adjPoint1 = pBaseMesh->m_SupportPoints[globalIndex1].m_Point;
				HVec3 adjPoint2 = pBaseMesh->m_SupportPoints[globalIndex2].m_Point;
				const HF32 dist0 = (adjPoint0|dir) - planeDist;
				const HF32 dist1 = (adjPoint1|dir) - planeDist;
				const HF32 dist2 = (adjPoint2|dir) - planeDist;
			/*	const HF32 dist0 = (adjPoint0 - startPoint.m_Point)|dir;
				const HF32 dist1 = (adjPoint1 - startPoint.m_Point)|dir;
				const HF32 dist2 = (adjPoint2 - startPoint.m_Point)|dir;*/

				const HF32 sign0(dist0*dist1);//edge0
				const HF32 sign1(dist1*dist2);//edge1
				const HF32 sign2(dist0*dist2);//edge2
				//check whether the facet is staddle the plane
				if(sign0.LT(zero) | sign1.LT(zero) | sign2.LT(zero))
				{
					

					//point in the differenct same 
					const HVec3 tp0 = Select(sign0.GE(zero), adjPoint2, Select(sign1.GE(zero), adjPoint0, adjPoint1));
					//points in the same side
					const HVec3 tp1 = Select(sign0.GE(zero), adjPoint0, Select(sign1.GE(zero), adjPoint1, adjPoint2));
					const HVec3 tp2 = Select(sign0.GE(zero), adjPoint1, Select(sign1.GE(zero), adjPoint2, adjPoint0));

					/*const HF32 dist0 = (tp0|dir) - planeDist;
					const HF32 dist1 = (tp1|dir) - planeDist;
					const HF32 dist2 = (tp2|dir) - planeDist;

					const HF32 sum1(Abs(dist0 - dist1));
					const HF32 sum2(Abs(dist0 - dist2));

					const HF32 absDist(Abs(dist0));
					const HVec3 v(tp1 - tp0);
					const HVec3 w(tp2 - tp0);
					HVec3 p0 = tp0 + v*(absDist/sum1);
					HVec3 p1 = tp0 + w*(absDist/sum2);*/



					//calculate the points intersect with the plane
					/*const HVec3 a(tp0 - startPoint.m_Point);
					const HF32 length = dir.m_V.Length();
					const HF32 b(-(a|dir));*/
					const HF32 b(planeDist - (tp0|dir));
					//const HF32 b((startPoint.m_Point- tp0)|dir);
				
					const HVec3 v(tp1 - tp0);
					const HF32 t0 = b/(v|dir);
					HVec3 p0= tp0 + t0*v;

					const HVec3 w(tp2 - tp0);
					const HF32 t1 = b/(w|dir);
					HVec3 p1= tp0 + t1*w;


					/*BIZ_ASSERT(pCurFacet->m_indice[0] == pCurFacet->m_adjFacets[0]->m_indice[incMod3(pCurFacet->m_adjEdges[0])]);
					BIZ_ASSERT(pCurFacet->m_indice[incMod3(0)] == pCurFacet->m_adjFacets[0]->m_indice[pCurFacet->m_adjEdges[0]]);
					BIZ_ASSERT(pCurFacet->m_indice[1] == pCurFacet->m_adjFacets[1]->m_indice[incMod3(pCurFacet->m_adjEdges[1])]);
					BIZ_ASSERT(pCurFacet->m_indice[incMod3(1)] == pCurFacet->m_adjFacets[1]->m_indice[pCurFacet->m_adjEdges[1]]);
					BIZ_ASSERT(pCurFacet->m_indice[2] == pCurFacet->m_adjFacets[2]->m_indice[incMod3(pCurFacet->m_adjEdges[2])]);
					BIZ_ASSERT(pCurFacet->m_indice[incMod3(2)] == pCurFacet->m_adjFacets[2]->m_indice[pCurFacet->m_adjEdges[2]]);*/
					//Check weather the adjFacets staddle the plane as well
					/*C_Facet* pFacet = pCurFacet->m_adjFacets[0];
					if(!pFacet->m_Visited && sign0.LT(zero))
					{
						HVec3 staddlePoint = AddStaddlePoint(pBaseMesh,  pFacet, pCurFacet->m_adjEdges[0], dir, startPoint.m_Point);
						point0.AddToEnd(&staddlePoint);
						point1.AddToEnd(&staddlePoint);
					}
					pFacet->m_Visited = true;

					pFacet = pCurFacet->m_adjFacets[1];
					if(!pFacet->m_Visited && sign1.LT(zero))
					{
						HVec3 staddlePoint = AddStaddlePoint(pBaseMesh,  pFacet, pCurFacet->m_adjEdges[1], dir, startPoint.m_Point);
						point0.AddToEnd(&staddlePoint);
						point1.AddToEnd(&staddlePoint);
					}
					pFacet->m_Visited = true;

					pFacet =pCurFacet->m_adjFacets[2];
					if(!pFacet->m_Visited && sign2.LT(zero))
					{
						HVec3 staddlePoint = AddStaddlePoint(pBaseMesh,  pFacet, pCurFacet->m_adjEdges[2], dir, startPoint.m_Point);
						point0.AddToEnd(&staddlePoint);
						point1.AddToEnd(&staddlePoint);
					}
					pFacet->m_Visited = true;*/


					point0.AddToEnd(&p0);
					point0.AddToEnd(&p1);
					point1.AddToEnd(&p0);
					point1.AddToEnd(&p1);
				}
			}
		}


		if(point0.m_numElements <= 4 || point1.m_numElements <=4)
			return E_Decomposition_Succeed;	

		//C_ConvexHullMesh* mesh0 = new C_ConvexHullMesh;
		U32 childIndex0 = parentIndex*2+1;
		U32 childIndex1 = parentIndex*2 +2;
		C_ConvexHullMesh* mesh0 = new C_ConvexHullMesh;
		m_ConvexHullMeshes[childIndex0].m_ConvexHullMesh = mesh0;
		if(mesh0->Build(point0) == Gravity::E_Generation_Succeed)
		{
			mesh0->RemapFacets();
			m_ConvexHullMeshes[childIndex0].state = Decomposition(mesh0, count, childIndex0, mesh0->m_SupportPoints[0], dir);
			if(m_ConvexHullMeshes[childIndex0].state == E_Decomposition_Fail)
			{
				//m_ConvexHullMeshes[childIndex1].state = E_Decomposition_Fail;
				SetFailFlags(childIndex1);
				return E_Decomposition_Succeed;
			}
		}
		else
		{
			//convex hull generation code fail
			m_ConvexHullMeshes[childIndex0].state = E_Decomposition_Fail;
			SetFailFlags(childIndex1);
			return E_Decomposition_Succeed;//take the parent's convex hull as the final convex hull
		}
		
		//C_ConvexHullMesh* mesh1 = new C_ConvexHullMesh;
		
		C_ConvexHullMesh* mesh1 = new C_ConvexHullMesh;
		m_ConvexHullMeshes[childIndex1].m_ConvexHullMesh = mesh1;
		if(mesh1->Build(point1) == Gravity::E_Generation_Succeed)
		{		
			mesh1->RemapFacets();
			m_ConvexHullMeshes[childIndex1].state = Decomposition(mesh1, count, childIndex1, mesh1->m_SupportPoints[0], dir);
			if(m_ConvexHullMeshes[childIndex1].state == E_Decomposition_Fail)
			{
				SetFailFlags(childIndex0);
				return E_Decomposition_Succeed;
			}
		}
		else
		{
			m_ConvexHullMeshes[childIndex1].state = E_Decomposition_Fail;
			SetFailFlags(childIndex0);
			return E_Decomposition_Succeed;//take the parent's convex hull as the final convex hull
		}
		
		if(m_ConvexHullMeshes[childIndex0].state == E_Decomposition_Succeed && m_ConvexHullMeshes[childIndex1].state == E_Decomposition_Succeed)
		{
			m_ValidMeshCount+=2;
		}
		return E_Decomposition_Media;
	}

	bool C_ConvexHullDecomposition::CalculateBridges(C_ConvexHullMesh* pMesh, C_Facet* pCurFacet, const Maths::HF32 PCREF eps)
	{
		using namespace Maths;

		for(S32 j=0; j<m_Bridges.m_numElements; ++j)
		{
			C_Bridge& bridge = m_Bridges[j];
			//ensure that the maximum distance from the included facets to the supporting plane is less than eps
			const HVec3 a(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[0]));
			const HVec3 b(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[1]));
			const HVec3 c(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[2]));
			const HF32 dotNormal = bridge.m_SupportPlaneNormal | pCurFacet->m_Normal;
			const HF32 dist0 = (bridge.m_SupportPlaneNormal|a) - bridge.m_SupportPlaneDist;
			const HF32 dist1 = (bridge.m_SupportPlaneNormal|b) - bridge.m_SupportPlaneDist;
			const HF32 dist2 = (bridge.m_SupportPlaneNormal|c) - bridge.m_SupportPlaneDist;
			/*const HF32 dist0 = (a - bridge.m_Point)|bridge.m_SupportPlaneNormal;
			const HF32 dist1 = (b - bridge.m_Point)|bridge.m_SupportPlaneNormal;
			const HF32 dist2 = (c - bridge.m_Point)|bridge.m_SupportPlaneNormal;*/
			//BIZ_ASSERT(dist0.LE(zero) && dist1.LE(zero) && dist2.LE(zero));
			//if(Abs(dist0).LE(eps) & Abs(dist1).LE(eps) & Abs(dist2).LE(eps))
			if(dotNormal.GE(eps))
			{
				//add the facet to the existing bridge
				bridge.m_pCurFacets->AddToEnd(&pCurFacet);
				pCurFacet->m_BridgeIndex = j;
				return true;
			}
		}
		return false;
	}

	void C_ConvexHullDecomposition::ClusterEstimation(C_ConvexHullMesh* pMesh, C_Facet* pCurFacet, C_Bridge& bridge, const U32 bridgeIndex, const Maths::HF32 PCREF eps)
	{
		using namespace Maths;

		if(pCurFacet->m_Visited)
			return;

		const HVec3 a(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[0]));
		const HVec3 b(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[1]));
		const HVec3 c(pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[2]));
			
		const HF32 dotNormal = bridge.m_SupportPlaneNormal | pCurFacet->m_Normal;
		const HF32 dist0 = (bridge.m_SupportPlaneNormal|a) - bridge.m_SupportPlaneDist;
		const HF32 dist1 = (bridge.m_SupportPlaneNormal|b) - bridge.m_SupportPlaneDist;
		const HF32 dist2 = (bridge.m_SupportPlaneNormal|c) - bridge.m_SupportPlaneDist;
		if(dotNormal.GE(eps))
		{
			//add the facet to the existing bridge
			bridge.m_pCurFacets->AddToEnd(&pCurFacet);
			pCurFacet->m_BridgeIndex = bridgeIndex;
			pCurFacet->m_Visited = true;
			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[0], bridge, bridgeIndex, eps);
			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[1], bridge, bridgeIndex, eps);
			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[2], bridge, bridgeIndex, eps);
		}
	}

	//void C_ConvexHullDecomposition::ClusterEstimation(C_ConvexHullMesh* pMesh, C_Facet* pCurFacet, const Maths::HF32 PCREF eps)
	//{
	//	if(pCurFacet->m_Visited)
	//		return;

	//	pCurFacet->m_Visited = true;

	//	C_Bridge bridge;
	//	bridge.m_pCurFacets->AddToEnd(&pCurFacet);
	//	bridge.m_SupportPlaneNormal = pCurFacet->m_Normal;
	//	bridge.m_SupportPlaneDist = pCurFacet->m_Dist;
	//	const U32 index = m_Bridges.m_numElements;
	//	pCurFacet->m_BridgeIndex = index;
	//	m_Bridges.AddToEnd(&bridge);
	//	ClusterEstimation(pMesh, pCurFacet->m_adjFacets[0], m_Bridges[index], index, eps);
	//	ClusterEstimation(pMesh, pCurFacet->m_adjFacets[1], m_Bridges[index], index, eps);
	//	ClusterEstimation(pMesh, pCurFacet->m_adjFacets[2], m_Bridges[index], index, eps);
	//}


	void C_ConvexHullDecomposition::ClusterEstimation(C_ConvexHullMesh* pMesh, const Maths::HF32 PCREF eps)
	{
		using namespace Maths;
		//Get rid of all the discarded facets
		pMesh->RemapFacets();

		pMesh->ClearFacetsVisitedFlags();

		for(S32 i=0; i<pMesh->m_Facets.m_numElements; ++i)
		{
			C_Facet* pCurFacet = pMesh->m_Facets[i];
			//ClusterEstimation(pMesh, pMesh->m_Facets[i], eps);
			if(pCurFacet->m_Visited)
				continue;

			pCurFacet->m_Visited = true;

			C_Bridge bridge;
			bridge.m_pCurFacets->AddToEnd(&pCurFacet);
			bridge.m_SupportPlaneNormal = pCurFacet->m_Normal;
			bridge.m_SupportPlaneDist = pCurFacet->m_Dist;
			const U32 index = m_Bridges.m_numElements;
			pCurFacet->m_BridgeIndex = index;
			m_Bridges.AddToEnd(&bridge);

			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[0], m_Bridges[index], index, eps);
			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[1], m_Bridges[index], index, eps);
			ClusterEstimation(pMesh, pCurFacet->m_adjFacets[2], m_Bridges[index], index, eps);
		}
	}


	bool C_ConvexHullDecomposition::ClusterRefinement(C_ConvexHullMesh* pMesh, C_Facet* pFacet,C_Bridge& bridge, const U32 bridgeIndex, const Maths::HF32 PCREF eps)
	{
		using namespace Maths;

		if(pFacet->m_Visited)
			return true;

		const HVec3 a(pMesh->GetPointFromFacetLocalIndex(pFacet->m_indice[0]));
		const HVec3 b(pMesh->GetPointFromFacetLocalIndex(pFacet->m_indice[1]));
		const HVec3 c(pMesh->GetPointFromFacetLocalIndex(pFacet->m_indice[2]));
			
		const HF32 dotNormal = bridge.m_SupportPlaneNormal | pFacet->m_Normal;
		const HF32 dist0 = (bridge.m_SupportPlaneNormal|a) - bridge.m_SupportPlaneDist;
		const HF32 dist1 = (bridge.m_SupportPlaneNormal|b) - bridge.m_SupportPlaneDist;
		const HF32 dist2 = (bridge.m_SupportPlaneNormal|c) - bridge.m_SupportPlaneDist;
		if(dotNormal.GE(eps))
		{
			//add the facet to the list of preFacets in the existing bridge
			bridge.m_pPreFacets->AddToEnd(&pFacet);
			pFacet->m_BridgeIndex = bridgeIndex;
			pFacet->m_Visited = true;
			ClusterRefinement(pMesh, pFacet->m_adjFacets[0], bridge, bridgeIndex, eps);
			ClusterRefinement(pMesh, pFacet->m_adjFacets[1], bridge, bridgeIndex, eps);
			ClusterRefinement(pMesh, pFacet->m_adjFacets[2], bridge, bridgeIndex, eps);
			return true;
		}

		return false;

	}

	void C_ConvexHullDecomposition::ClusterRefinement(C_ConvexHullMesh* pMesh, const Maths::HF32 PCREF eps)
	{

		using namespace Maths;
		const HF32 zero(Maths::Constants<HF32>::Zero());
		const HVec3 zeroV(Maths::Constants<HVec3>::Zero());
		HVec3 averageNormal(zeroV);
		HF32 averageDist(zero);
		//refine the support plane(average the plane)
		bool bChanged = true;
		U32 MaxIterations = 12;
		while(bChanged && (MaxIterations--))
		{
			bChanged = false;

			//Loop through all the bridges, update planes and then re-insert facets into the new bridges. If any facet has changed from 
			//one bridge to another, set bChanged to true and continue round the loop. Shouldn't need to, but might want to, exit out after a maximum
			//number of iterations to avoid the case where an infinite loop might happen.

			//SanityCheck();
			for(S32 i=0; i<m_Bridges.m_numElements; ++i)
			{
				C_Bridge& bridge = m_Bridges[i];
				averageNormal = zeroV;
				averageDist = zero;
				
				for(S32 j=0; j<bridge.m_pCurFacets->m_numElements; ++j)
				{
					averageNormal += (*bridge.m_pCurFacets)[j]->m_Normal;
					averageDist +=(*bridge.m_pCurFacets)[j]->m_Dist;
				}
				bridge.m_SupportPlaneNormal = C_Normal(averageNormal);
				bridge.m_SupportPlaneDist = averageDist/(F32)bridge.m_pCurFacets->m_numElements;
				//bridge.m_pPreFacets->m_numElements = 0;
			}

			pMesh->ClearFacetsVisitedFlags();

			for(S32 j=0; j < pMesh->m_Facets.m_numElements; ++j)
			{
				C_Facet* pCurFacet = pMesh->m_Facets[j];
				if(!pCurFacet->m_Visited)
				{
					for(S32 i=0; i<m_Bridges.m_numElements; ++i)
					{
						C_Bridge& bridge = m_Bridges[i];
						if(bridge.m_pPreFacets->m_numElements == 0)
						{
							if(ClusterRefinement(pMesh, pCurFacet, bridge, i, eps))
								break;
						}
					}
				}
				SanityCheckPreFacets();
			}

		
		
			/*S32 count = 0;
			for(S32 i = 0; i < m_Bridges.m_numElements; ++i)
			{
				count +=m_Bridges[i].m_pPreFacets->m_numElements;
			}
			BIZ_ASSERT(count == pMesh->m_Facets.m_numElements);*/

			//Compare pre with current to see if they are different!!!
			for(S32 i = 0; i < m_Bridges.m_numElements && !bChanged; ++i)
			{
				C_Bridge& bridge = m_Bridges[i];

				if(bridge.m_pCurFacets->m_numElements != bridge.m_pPreFacets->m_numElements)
				{
					bChanged = true;
					break;
				}
				for(S32 i = 0; i < bridge.m_pCurFacets->m_numElements; ++i)
				{
					if((*bridge.m_pPreFacets)[i] != (*bridge.m_pCurFacets)[i])
					{
						bChanged = true;
						break;
					}
				}
			}

			//Swap the current and previous facets pointer
			for(S32 i=0; i< m_Bridges.m_numElements; ++i)
			{
				C_Bridge& bridge = m_Bridges[i];
				SharedPhysics::SortedArray<C_Facet*>* oldFacets = bridge.m_pCurFacets;
				bridge.m_pCurFacets = bridge.m_pPreFacets;
				bridge.m_pPreFacets = oldFacets;
				bridge.m_pPreFacets->m_numElements = 0;
			}
		}

		for(S32 i=0; i< m_Bridges.m_numElements; ++i)
		{
			if(m_Bridges[i].m_pCurFacets->m_numElements == 0)
			{
				m_Bridges.Remove(i, 1);
				--i;
			}
		}
	}

	/*void C_ConvexHullDecomposition::CalculatePockets(U32 bridgeIndex, C_ConvexHullMesh* pMesh, C_Facet* pFacet,  U32 _index, C_EdgeBuffer& edgeBuffer, C_Edge& stack)
	{
		using namespace Maths;

		S32 size = 1;
		stack[0].m_facet = this;
		stack[0].m_index = _index;

		C_Facet* next_facet = this;
		S32    next_index = _index;
		C_Facet* next2_facet = NULL;
		S32    next2_index = -1;

		while(size-- > 0)
		{
			C_Facet* const __restrict f = next_facet;
			const U32 index( next_index );

			if(f->m_Visited)
			{
				next_facet = next2_facet;
				next_index = next2_index;
				const C_Edge& next2_e = stack[ Max(size-2,0) ];
				next2_facet = next2_e.m_facet;
				next2_index = next2_e.m_index;
				
				continue;
			}

			if(f->m_BridgeIndex != bridgeIndex)
			{
				edgeBuffer.Insert(f, index);
				if(size!=0)
				{
					next_facet = next2_facet;
					next_index = next2_index;
					const C_Edge& next2_e = stack[ Max(size-2,0) ];
					next2_facet = next2_e.m_facet;
					next2_index = next2_e.m_index;
				}
			}
			else
			{
				f->m_Visited = true;

				const U32 next(incMod3(index));
				const U32 next2(incMod3(next));
				stack[size].m_facet   = f->m_adjFacets[next2];
				stack[size++].m_index = f->m_adjEdges[next2];
				stack[size].m_facet   = f->m_adjFacets[next];
				stack[size++].m_index = f->m_adjEdges[next];
				next_facet = f->m_adjFacets[next];
				next_index = f->m_adjEdges[next];
				next2_facet = f->m_adjFacets[next2];
				next2_index = f->m_adjEdges[next2];
			}
		}
	}*/

	void C_ConvexHullDecomposition::ConstructBridgesBoundary(C_ConvexHullMesh* pMesh, C_Bridge& bridge)
	{
		for(S32 k =0; k<bridge.m_pCurFacets->m_numElements; ++k)
		{
			C_Facet* pFacet = (*bridge.m_pCurFacets)[k];
			for(S32 j=0; j<3; ++j)
			{
				C_Facet* pAdjFacet = pFacet->m_adjFacets[j];
				if(pAdjFacet->m_BridgeIndex != pFacet->m_BridgeIndex)
				{
					U32 indice0 = pFacet->m_indice[j];
					U32 indice1 = pFacet->m_indice[incMod3(j)];
					C_GraphEdge edge;
					edge.m_VertexAIndex = pMesh->m_Points[indice0];
					edge.m_VertexBIndex = pMesh->m_Points[indice1];
					bridge.m_BridgeBoundary.AddToEnd(&edge);
				}
			}
		}
	}

	

	void C_ConvexHullDecomposition::GetShorestPath(const U32 vertexSourceIndex, const U32 vertexTargetIndex, SharedPhysics::SortedArray<C_GraphEdge>& shorestPath, const U32 brigdeBoundaryIndex)
	{
	
		if(vertexSourceIndex == vertexTargetIndex)
			return;

		SharedPhysics::C_S32SafeStack stack(2048);
		SharedPhysics::SortedArray<SharedPhysics::C_PointerWrapper<C_GraphVertex> > vertices(50);
		//U32 curIndex = vertexSourceIndex;

		C_GraphVertex* pVertexSource = &m_Vertices[vertexSourceIndex];
		C_GraphVertex* pVertexTarget = &m_Vertices[vertexTargetIndex];
	
		
		F32 max = Maths::Constants<F32>::Max(); 
		for(S32 i=0; i< m_Vertices.m_numElements; ++i)
		{
			m_Vertices[i].m_Dist = max;
			m_Vertices[i].m_pPreEdge = NULL;
			m_Vertices[i].m_Visited = false;
			m_Vertices[i].m_Discarded = false;
		}

		pVertexSource->m_Dist = 0.f;
		//pVertexSource->m_Visited = true;

		C_GraphVertex* pCurVertex = pVertexSource;
		bool found = false;

		while(pCurVertex)
		{

			pCurVertex->m_Visited = true;
			pCurVertex->m_Discarded = true;
			U32 curIndex = pCurVertex->m_Index;
			C_GraphEdge* pEdge = NULL;
			for(S32 i=0; i< pCurVertex->m_GraphEdges.m_numElements; ++i)
			{
				pEdge = pCurVertex->m_GraphEdges[i];
				U32 index = Maths::Select(pEdge->m_VertexAIndex== curIndex, pEdge->m_VertexBIndex, pEdge->m_VertexAIndex);
				C_GraphVertex* pVertex = &m_Vertices[index];

				if(!pVertex->m_Discarded)
				{
					if(index == vertexTargetIndex)
					{
						found = true;
						pVertex->m_pPreEdge = pEdge;
						pCurVertex = pVertex;
						curIndex = index;
						break;
					}
					F32 dist = pCurVertex->m_Dist + pEdge->m_sqDist;
					if(dist < pVertex->m_Dist)
					{
						pVertex->m_Dist = dist;
						pVertex->m_pPreEdge = pEdge;
						//make sure each vertex just insert one time in the vertices;
						if(!pVertex->m_Visited)
						{
							//sort vertex based on the dist
							SharedPhysics::C_PointerWrapper<C_GraphVertex> wrapper(pVertex);
							//vertices.Insert(&wrapper);
							vertices.AddToEnd(&wrapper);
							pVertex->m_Visited = true;
						}
					}
				}
			}

			if(found)
			{
				while(pEdge)
				{
					//shorestPath.AddToEnd(pEdge);
					shorestPath.InsertOverwrite(pEdge);
					curIndex = Maths::Select(pEdge->m_VertexAIndex== curIndex, pEdge->m_VertexBIndex, pEdge->m_VertexAIndex);
					pCurVertex = &m_Vertices[curIndex];
					pEdge = pCurVertex->m_pPreEdge;
				}
				return;
			}

			if(vertices.m_numElements == 0)
				return;

			//sorted the vertices by dist, the maximun dist will be in front  and the min dist will be on the back
			vertices.QuickSort(stack, 10);

			//get the min dist vertice out
			
			//curIndex = vertices.m_numElements - 1;
			pCurVertex = vertices[vertices.m_numElements - 1].m_pElement;
			vertices.m_numElements--;
			

		/*	F32 minDist = max;
			pCurVertex = NULL;

			for(S32 i=0; i< m_Vertices.m_numElements; ++i)
			{
				if(!m_Vertices[i].m_Discarded && minDist > m_Vertices[i].m_Dist)
				{
					minDist = m_Vertices[i].m_Dist;
					pCurVertex = &m_Vertices[i];
				}
			}*/
		}
	}

	void C_ConvexHullDecomposition::CalculatePockets(C_ConvexHullMesh* pMesh)
	{
		//Construct the bridges' boundary
		for(S32 i=0; i< m_Bridges.m_numElements; ++i)
		{
			C_Bridge& bridge = m_Bridges[i];
			
			ConstructBridgesBoundary(pMesh, bridge);

			//calculate the pockets
			for(S32 j=0; j<bridge.m_BridgeBoundary.m_numElements; ++j)
			{
				C_GraphEdge& edge = bridge.m_BridgeBoundary[j];
				//the vertex indice are corresponding to the convex hull mesh, not the model
				C_Point& pointA = pMesh->m_SupportPoints[edge.m_VertexAIndex];
				C_Point& pointB = pMesh->m_SupportPoints[edge.m_VertexBIndex];
				GetShorestPath(pointA.m_IndexInOriginalModel, pointB.m_IndexInOriginalModel, bridge.m_Pocket.m_PocketBoundary, j);
			}
		}
	}

	void C_ConvexHullDecomposition::CalculateKnots(Gravity::C_ConvexHullMesh *pMesh)
	{
		for(S32 i=0; i<m_Bridges.m_numElements; ++i)
		{
			C_Bridge& bridge = m_Bridges[i];
			C_Pocket& pocket = bridge.m_Pocket;
			for(S32 j=0; j<pocket.m_PocketBoundary.m_numElements; ++j)
			{
				C_GraphEdge& edge = pocket.m_PocketBoundary[j];
			}
		}
	}

	void C_ConvexHullDecomposition::DebugRender(DbUtil::I_DebugRender* pRender)
	{
		for(S32 i=0; i<m_Edges.m_numElements; ++i)
		{
			C_GraphEdge* edge = &m_Edges[i];
			C_GraphVertex& vertexA = m_Vertices[edge->m_VertexAIndex];
			C_GraphVertex& vertexB = m_Vertices[edge->m_VertexBIndex];
			pRender->Draw3dLine(vertexA.m_Position, vertexB.m_Position, Maths::RGBA(255, 255,255));
		}

		/*const Maths::HVec3 trans(1.f, 0.f, 0.f);
		for(S32 i=0; i<m_Triangles.m_numElements; ++i)
		{
			C_GraphTriangle& tri = m_Triangles[i];
			C_GraphVertex& vertexA = m_Vertices[tri.m_VertexA];
			C_GraphVertex& vertexB = m_Vertices[tri.m_VertexB];
			C_GraphVertex& vertexC = m_Vertices[tri.m_VertexC];
			pRender->Draw3dTriangleOutline(vertexA.m_Position + trans, vertexB.m_Position + trans, vertexC.m_Position + trans, Maths::RGBA(0, 255, 0));
		}*/
	}

	void C_ConvexHullDecomposition::SanityCheck()
	{
		using namespace Maths;
		for(S32 i=0; i<m_Bridges.m_numElements; ++i)
		{
			HVec3 average(Constants<HVec3>::Zero());
			const U32 colour = i % 24;
			C_Bridge& bridge = m_Bridges[i];
			bool found = false;
			if(bridge.m_pCurFacets->m_numElements != 1)
			{
				for(S32 k=0; k<bridge.m_pCurFacets->m_numElements; ++k)
				{
					C_Facet* pCurFacet = (*bridge.m_pCurFacets)[k];
					found = false;
					for(S32 j=0; j<bridge.m_pCurFacets->m_numElements; ++j)
					{
						if(k != j)
						{
							C_Facet* pFacet = (*bridge.m_pCurFacets)[j];
							if(pCurFacet->m_adjFacets[0] == pFacet || pCurFacet->m_adjFacets[1] == pFacet || pCurFacet->m_adjFacets[2] == pFacet)
							{
								found = true;
								break;
							}
						}
					}
					BIZ_ASSERT(found == true);
				}
			}
		}
	}

	void C_ConvexHullDecomposition::SanityCheckPreFacets()
	{
		using namespace Maths;
		for(S32 i=0; i<m_Bridges.m_numElements; ++i)
		{
			HVec3 average(Constants<HVec3>::Zero());
			const U32 colour = i % 24;
			C_Bridge& bridge = m_Bridges[i];
			bool found = false;
			if(bridge.m_pPreFacets->m_numElements != 1)
			{
				for(S32 k=0; k<bridge.m_pPreFacets->m_numElements; ++k)
				{
					C_Facet* pCurFacet = (*bridge.m_pPreFacets)[k];
					found = false;
					for(S32 j=0; j<bridge.m_pPreFacets->m_numElements; ++j)
					{
						if(k != j)
						{
							C_Facet* pFacet = (*bridge.m_pPreFacets)[j];
							if(pCurFacet->m_adjFacets[0] == pFacet || pCurFacet->m_adjFacets[1] == pFacet || pCurFacet->m_adjFacets[2] == pFacet)
							{
								found = true;
								break;
							}
						}
					}
					BIZ_ASSERT(found == true);
				}
			}
		}
	}

	void C_ConvexHullDecomposition::DrawShorestPath(DbUtil::I_DebugRender* pRender, SharedPhysics::SortedArray<Gravity::C_GraphEdge>& shorestPath)
	{
		for(S32 i=0; i<shorestPath.m_numElements; ++i)
		{
			Gravity::C_GraphEdge& edge = shorestPath[i];
			C_GraphVertex& vertexA = m_Vertices[edge.m_VertexAIndex];
			C_GraphVertex& vertexB = m_Vertices[edge.m_VertexBIndex];
			pRender->Draw3dLine(vertexA.m_Position, vertexB.m_Position, Maths::RGBA(0, 0, 255));
		}
	}

	void C_ConvexHullDecomposition::DrawBridges(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex)
	{
		using namespace Maths;
		HVec3 average(Constants<HVec3>::Zero());
		C_Bridge& bridge = m_Bridges[bridgeIndex];
		for(S32 j=0; j<bridge.m_pCurFacets->m_numElements; ++j)
		{
			C_Facet* pCurFacet = (*bridge.m_pCurFacets)[j];
			const HVec3 a = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[0]);
			const HVec3 b = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[1]);
			const HVec3 c = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[2]);
			average = (a + b +c)/3.f;
			pRender->Draw3dTriangle(a, b, c, RGBA(0, 0, 255));
			pRender->Draw3dTriangleOutline(a, b, c, RGBA(0, 255,0));
			//pRender->Draw3dArrow(average, pCurFacet->m_Normal, 0.05f, 0.02f, RGBA(255, 0, 0) );
		}

		//SanityCheck();

		//bridge.m_BridgeBoundary.m_numElements =0 ;
		//ConstructBridgesBoundary(pMesh, bridge);
		////edges in the convex hull in the pMesh
		//for(S32 j=0; j<bridge.m_BridgeBoundary.m_numElements; ++j)
		//{
		//	C_GraphEdge& edge = bridge.m_BridgeBoundary[j];
		//	const HVec3 a = pMesh->GetPointFromGlobalIndex(edge.m_VertexAIndex);
		//	const HVec3 b = pMesh->GetPointFromGlobalIndex(edge.m_VertexBIndex);
		//	pRender->Draw3dLine(a, b, RGBA(0, 0,0));
		//}
	}


	void C_ConvexHullDecomposition::DrawPockets(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex)
	{
		using namespace Maths;

		RGBA colours[24] = {Maths::RGBA(25, 25, 0), Maths::RGBA(0, 25, 25), Maths::RGBA(25, 0, 25), Maths::RGBA(25, 25, 25),
						Maths::RGBA(50, 50, 0), Maths::RGBA(0, 50, 50), Maths::RGBA(50, 0, 50), Maths::RGBA(50, 50, 50),
						Maths::RGBA(100, 100, 0), Maths::RGBA(0, 100, 100), Maths::RGBA(100, 0, 100), Maths::RGBA(100, 100, 100),
						Maths::RGBA(125, 125, 0), Maths::RGBA(0, 125, 125), Maths::RGBA(125, 0, 125), Maths::RGBA(125, 125, 125),
						Maths::RGBA(150, 150, 0), Maths::RGBA(0, 150, 150), Maths::RGBA(150, 0, 150), Maths::RGBA(150, 150, 150),
						Maths::RGBA(200, 200, 0), Maths::RGBA(0, 200, 200), Maths::RGBA(200, 0, 200), Maths::RGBA(200, 200, 200)};

		HVec3 average(Constants<HVec3>::Zero());
		C_Bridge& bridge = m_Bridges[bridgeIndex];
		C_Pocket& pocket = bridge.m_Pocket;
		for(S32 j=0; j<pocket.m_PocketBoundary.m_numElements; ++j)
		{
			Gravity::C_GraphEdge& edge = pocket.m_PocketBoundary[j];
			const HVec3 a = pMesh->GetPointFromGlobalIndex(edge.m_VertexAIndex);
			const HVec3 b = pMesh->GetPointFromGlobalIndex(edge.m_VertexBIndex);

			pRender->Draw3dLine(a, b, RGBA(0, 255, 0));

			/*for(S32 i=0; i<bridge.m_BridgeBoundary.m_numElements; ++i)
			{
				if(edge.m_BridgeBroundaryIndex == i)
				{
					const U32 colour = i % 24;
					pRender->Draw3dLine(a, b, colours[colour]);
					break;
				}
			}*/
		}
	}

	void C_ConvexHullDecomposition::DrawBridges(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender)
	{
		using namespace Maths;

		RGBA colours[24] = {Maths::RGBA(25, 25, 0), Maths::RGBA(0, 25, 25), Maths::RGBA(25, 0, 25), Maths::RGBA(25, 25, 25),
							Maths::RGBA(50, 50, 0), Maths::RGBA(0, 50, 50), Maths::RGBA(50, 0, 50), Maths::RGBA(50, 50, 50),
							Maths::RGBA(100, 100, 0), Maths::RGBA(0, 100, 100), Maths::RGBA(100, 0, 100), Maths::RGBA(100, 100, 100),
							Maths::RGBA(125, 125, 0), Maths::RGBA(0, 125, 125), Maths::RGBA(125, 0, 125), Maths::RGBA(125, 125, 125),
							Maths::RGBA(150, 150, 0), Maths::RGBA(0, 150, 150), Maths::RGBA(150, 0, 150), Maths::RGBA(150, 150, 150),
							Maths::RGBA(200, 200, 0), Maths::RGBA(0, 200, 200), Maths::RGBA(200, 0, 200), Maths::RGBA(200, 200, 200)};


		for(S32 i=0; i<m_Bridges.m_numElements; ++i)
		{
			HVec3 average(Constants<HVec3>::Zero());
			const U32 colour = i % 24;
			C_Bridge& bridge = m_Bridges[i];
			bool found = false;
			if(bridge.m_pCurFacets->m_numElements != 1)
			{
				for(S32 k=0; k<bridge.m_pCurFacets->m_numElements; ++k)
				{
					C_Facet* pCurFacet = (*bridge.m_pCurFacets)[k];
					found = false;
					for(S32 j=0; j<bridge.m_pCurFacets->m_numElements; ++j)
					{
						C_Facet* pFacet = (*bridge.m_pCurFacets)[j];
						if(pCurFacet->m_adjFacets[0] == pFacet)
						{
							found = true;
							break;
						}
						if(pCurFacet->m_adjFacets[1] == pFacet)
						{
							found = true;
							break;
						}
						if(pCurFacet->m_adjFacets[2] == pFacet)
						{
							found = true;
							break;
						}
						
					}
					BIZ_ASSERT(found == true);
				}
			}
			for(S32 j=0; j<bridge.m_pCurFacets->m_numElements; ++j)
			{
				//BIZ_ASSERT(bridge.m_pCurFacets->m_numElements != 1);
				
				C_Facet* pCurFacet = (*bridge.m_pCurFacets)[j];
				const HVec3 a = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[0]);
				const HVec3 b = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[1]);
				const HVec3 c = pMesh->GetPointFromFacetLocalIndex(pCurFacet->m_indice[2]);
				pRender->Draw3dTriangle(a, b, c, colours[colour]);

				//if(bridge.m_pCurFacets->m_numElements ==1)
				//{
				//	/*Btl::String str;
				//	str = Btl::String::Format("%i", i);
				//	pRender->Draw3dText(a, str.Chars());
				//	printf("%i\n", i);*/
				//	pRender->Draw3dTriangle(a, b, c, RGBA(255, 0, 0));
				//	//pRender->Draw3dArrow(a, bridge.m_SupportPlaneNormal, 0.05f, 0.01f, RGBA(0, 255, 0));
				//}
				//average += (a + b +c);
				
				//pRender->Draw3dTriangleOutline(a, b, c, colours[colour]);
			}

			//average = average / (bridge.m_pCurFacets->m_numElements *3);
			/*Btl::String str;
			average = average / (bridge.m_pCurFacets->m_numElements *3);
			str = Btl::String::Format("%i", i);
			pRender->Draw3dText(average, str.Chars());*/
			//pRender->Draw3dArrow(average , bridge.m_SupportPlaneNormal, 0.05f, 0.01f, RGBA(0, 255, 0));

			bridge.m_BridgeBoundary.m_numElements =0 ;
			ConstructBridgesBoundary(pMesh, bridge);
			//edges in the convex hull in the pMesh
			for(S32 j=0; j<bridge.m_BridgeBoundary.m_numElements; ++j)
			{
				C_GraphEdge& edge = bridge.m_BridgeBoundary[j];
				const HVec3 a = pMesh->GetPointFromGlobalIndex(edge.m_VertexAIndex);
				const HVec3 b = pMesh->GetPointFromGlobalIndex(edge.m_VertexBIndex);
				pRender->Draw3dLine(a, b, RGBA(255, 0,0));
			}
			
		}

	}

	void C_ConvexHullDecomposition::DrawBridgesBoundary(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender)
	{
		for(S32 i=0; i<m_Bridges.m_numElements; ++i)
		{
			C_Bridge& bridge = m_Bridges[i];
			bridge.m_BridgeBoundary.m_numElements =0 ;
			ConstructBridgesBoundary(pMesh, bridge);
			//edges in the convex hull in the pMesh
			for(S32 j=0; j<bridge.m_BridgeBoundary.m_numElements; ++j)
			{
				C_GraphEdge& edge = bridge.m_BridgeBoundary[j];
				const Maths::HVec3 a = pMesh->GetPointFromGlobalIndex(edge.m_VertexAIndex);
				const Maths::HVec3 b = pMesh->GetPointFromGlobalIndex(edge.m_VertexBIndex);
				pRender->Draw3dLine(a, b, Maths::RGBA(255, 0,0));
			}
		}
	}


	void C_ConvexHullDecomposition::DrawBridgesBoundary(C_ConvexHullMesh* pMesh, DbUtil::I_DebugRender* pRender, const U32 bridgeIndex)
	{
		using namespace Maths;
		C_Bridge& bridge = m_Bridges[bridgeIndex];
		bridge.m_BridgeBoundary.m_numElements =0 ;
		ConstructBridgesBoundary(pMesh, bridge);
		//edges in the convex hull in the pMesh
		for(S32 j=0; j<bridge.m_BridgeBoundary.m_numElements; ++j)
		{
			C_GraphEdge& edge = bridge.m_BridgeBoundary[j];
			const Maths::HVec3 a = pMesh->GetPointFromGlobalIndex(edge.m_VertexAIndex);
			const Maths::HVec3 b = pMesh->GetPointFromGlobalIndex(edge.m_VertexBIndex);
			pRender->Draw3dLine(a, b, Maths::RGBA(255, 0,0));
			pRender->Draw3dStar(a, 0.01f, RGBA(0, 155, 155));
			pRender->Draw3dStar(b, 0.02f, RGBA(155, 0, 155));
		}
	}
#pragma optimize("", on)
}

	

