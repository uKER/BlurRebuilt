#include "stdafx.h"

using namespace AftkUt;

bool IsBrokenNormal(
	const Aftk::Normal3& n)
{
	if (Maths::CheckBuggeredFloat(n.x) ||
		Maths::CheckBuggeredFloat(n.x) ||
		Maths::CheckBuggeredFloat(n.x))
		return true;

	if (Aftk::EqualityTestFVector(n, Aftk::Vector3F32(0,0,0)))
		return true;

	return false;
}


Aftk::C_Status C_RegenerateBrokenNormals::Regenerate(
	Aftk::C_SceneMeshNode *pSceneMeshNode,
	F32 threshold)
{
	//if (!C_SmoothingHelper::ShouldSmooth(pSceneMeshNode))
	//	return Aftk::C_Status::E_Status_Success;

#if 0
	RadialEdgeMesh::C_Mesh *pRadialMesh = new RadialEdgeMesh::C_Mesh(pSceneMeshNode);
	if (!pRadialMesh)
		return Aftk::C_Status::E_Status_OutOfMemory;

	pRadialMesh->Build(status, false);
	if (!status)
		return status;

	status = Regenerate(
		pRadialMesh,
		pSceneMeshNode,
		maxDeviationFromFaceNorm);

	delete pRadialMesh;

	return status;
#endif

	Ut::BitVector polygonVerticesProcessed;
	polygonVerticesProcessed.Resize(pSceneMeshNode->GetNumPolygonVertices());
	polygonVerticesProcessed.ClearBits();

	F32 angularThreshold = cosf(DegToRad(threshold));

	for (U32 iLod = 0; iLod<pSceneMeshNode->GetNumLodLevels(); ++iLod)
	{
		for (U32 jPoly=0; jPoly<pSceneMeshNode->GetNumPolygons(iLod); ++jPoly)
		{
			Aftk::C_Polygon& p1 = pSceneMeshNode->GetPolygon(iLod, jPoly);

			for (U32 kPolyVert=0; kPolyVert<p1.GetNumPolygonVertices(); ++kPolyVert)
			{
				U32 pv1Index = p1.GetPolygonVertexIndex(kPolyVert);
				if (polygonVerticesProcessed[pv1Index])
					continue;

				Aftk::C_PolygonVertex& pv1 = pSceneMeshNode->GetPolygonVertex(pv1Index);
				
				S32 pv1PosIndex = pv1.GetPositionIndex();
				if (pv1PosIndex == -1)
					continue;

				S32 pv1NormIndex = pv1.GetNormalIndex();
				if (pv1NormIndex == -1 || IsBrokenNormal(pSceneMeshNode->GetVertexNormal(pv1NormIndex)))
				{

					Ut::UtVector< Ut::DynamicVector<S32> > smoothingGroup;
					smoothingGroup.PushBack(pv1Index);

					Aftk::Normal3 p1Norm;
						p1Norm.x = p1Norm.y = p1Norm.z = 0.f;
						p1Norm += (p1.GetArea() * p1.GetNormal());
					Aftk::Normal3 smoothedNorm = p1Norm;

					if (C_SmoothingHelper::ShouldSmooth(p1))
					{
						//find polys that share this vertex
						//cant use aftk adjacency info as it only lists polys that share an edge.
						for (U32 jjPoly=0; jjPoly<pSceneMeshNode->GetNumPolygons(iLod); ++jjPoly)
						{
							if (jPoly == jjPoly)
								continue;

							Aftk::C_Polygon& p2 = pSceneMeshNode->GetPolygon(iLod, jjPoly);
							for (U32 kkPolyVert = 0; kkPolyVert < p2.GetNumPolygonVertices(); ++kkPolyVert)
							{
								U32 pv2Index = p2.GetPolygonVertexIndex(kkPolyVert);
								if (polygonVerticesProcessed[pv2Index])
									continue;

								Aftk::C_PolygonVertex& pv2 = pSceneMeshNode->GetPolygonVertex(pv2Index);

								S32 pv2PosIndex = p2.GetPolygonVertex(kkPolyVert).GetPositionIndex();
								if (pv2PosIndex == -1)
									continue;

								if (pv1PosIndex == pv2PosIndex ||
										Aftk::EqualityTestFVector( pSceneMeshNode->GetVertexPosition(pv1PosIndex), pSceneMeshNode->GetVertexPosition(pv2PosIndex)))
								{
									if (C_SmoothingHelper::ShouldSmooth(p2))
									{
										Aftk::Normal3 p2Norm;
										p2Norm.x = p2Norm.y = p2Norm.z = 0.f;

										if (C_SmoothingHelper::CanSmooth((Aftk::Vector3F32&)p1Norm, (Aftk::Vector3F32&)p2Norm, 1.f, angularThreshold, true))
										{
											smoothedNorm += (p2.GetArea() * p2.GetNormal());
											smoothingGroup.PushBack(pv2Index);
										}
									}
									break;
								}
							}
						}
					}

					UtMaths::SafeNormalise((Aftk::Vector3F32*)&smoothedNorm);
					for (U32 sgIndex = 0; sgIndex<(U32)smoothingGroup.Size(); ++sgIndex)
					{
						Aftk::C_PolygonVertex& pv = pSceneMeshNode->GetPolygonVertex(smoothingGroup[sgIndex]);

						S32 normIndex = pv.GetNormalIndex();
						if (normIndex == -1)
							normIndex = pSceneMeshNode->AddVertexNormal();

						pv.SetNormalIndex(normIndex);
						Aftk::Normal3& normal = pSceneMeshNode->GetVertexNormal(normIndex);
						normal = smoothedNorm;
					}
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;

}


Aftk::C_Status C_RegenerateBrokenNormals::Regenerate(
	RadialEdgeMesh::C_Mesh *pRadialMesh,
	Aftk::C_SceneMeshNode* pSceneMeshNode)
{
	RadialEdgeMesh::ListVert *pRadialVertex = pRadialMesh->GetFirstVertex();
	while (pRadialVertex)
	{
		Ut::UtVector< Ut::DynamicVector<RadialEdgeMesh::ListPolyVert*> > pRadialPVs;

		RadialEdgeMesh::ListPolyVert* pRadialPolyVert = pRadialVertex->m_data.GetFirstPolygonVertex();
		while (pRadialPolyVert)
		{
			U32 pvIndex = pRadialPolyVert->m_data.GetAftkIndex();

			if (pvIndex < 0 || pvIndex > pSceneMeshNode->GetNumPolygonVertices())
			{
				pRadialPolyVert = pRadialPolyVert->GetNext();
				continue;
			}

			Aftk::C_PolygonVertex& polyVert = pSceneMeshNode->GetPolygonVertex(pvIndex);
			S32 normIndex = polyVert.GetNormalIndex();
			if (normIndex != -1 && !IsBrokenNormal(pSceneMeshNode->GetVertexNormal(normIndex)))
			{
				pRadialPolyVert = pRadialPolyVert->GetNext();
				continue;
			}

			//if we get here, the normal is either not present or broken
			//add to list to be fixed later...
			pRadialPVs.PushBack(pRadialPolyVert);

			pRadialPolyVert = pRadialPolyVert->GetNext();
		}

		if (pRadialPVs.Size() > 0)
		{
			//Loop through and average polyface normals
			Aftk::Normal3 avgNormal;
			avgNormal.x = avgNormal.y = avgNormal.z = 0.f;


			pRadialPolyVert = pRadialVertex->m_data.GetFirstPolygonVertex();
			while (pRadialPolyVert)
			{
				RadialEdgeMesh::C_Polygon *pRadialPoly = pRadialPolyVert->m_data.GetPolygon();

				if (C_SmoothingHelper::ShouldSmooth(pSceneMeshNode->GetPolygon(pRadialPoly->GetAftkIndex())))
				{
					Aftk::Normal3 polyNormal;
					pRadialPoly->GetPolyNormal((Maths::HVec3&)polyNormal, &pRadialVertex->m_data);
					avgNormal += polyNormal;
				}

				pRadialPolyVert = pRadialPolyVert->GetNext();
			}

			UtMaths::SafeNormalise((Aftk::Vector3F32*)&avgNormal);
			Ut::UtVector< Ut::DynamicVector<RadialEdgeMesh::ListPolyVert*> >::Iterator it;
			for (it=pRadialPVs.Begin(); it!=pRadialPVs.End(); ++it)
			{
				U32 aftkPVIndex = (*it)->m_data.GetAftkIndex();
				Aftk::C_PolygonVertex& aftkPV = pSceneMeshNode->GetPolygonVertex(aftkPVIndex);
				S32 normIndex = aftkPV.GetNormalIndex();

				if (normIndex = -1)
					normIndex = pSceneMeshNode->AddVertexNormal();

				aftkPV.SetNormalIndex(normIndex);
				Aftk::Normal3& normal = pSceneMeshNode->GetVertexNormal(normIndex);

				if (C_SmoothingHelper::ShouldSmooth(pSceneMeshNode->GetPolygon((*it)->m_data.GetPolygon()->GetAftkIndex())))
					normal = avgNormal;
				else
					(*it)->m_data.GetPolygon()->GetPolyNormal((Maths::HVec3&)normal, &pRadialVertex->m_data);
			}
		}

		pRadialVertex = pRadialVertex->GetNext();
	}

	return Aftk::C_Status::E_Status_Success;
}