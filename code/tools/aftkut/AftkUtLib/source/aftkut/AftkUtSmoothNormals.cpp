#include "stdafx.h"

namespace AftkUt
{
	Aftk::C_Status C_SmoothNormals::Smooth(
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		F32 angularTolerance)
	{
		Aftk::C_Status status;

		if (!C_SmoothingHelper::ShouldSmooth(pAftkSceneMeshNode))
			return Aftk::C_Status::E_Status_Success;

		RadialEdgeMesh::C_Mesh* pRadialMesh = new RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode);
		pRadialMesh->Build(status, false);
		if (!status) return status;

		status = Smooth(
			pRadialMesh,
			pAftkSceneMeshNode,
			angularTolerance);

		delete pRadialMesh;

		return status;
	}

	Aftk::C_Status C_SmoothNormals::Smooth(
		const RadialEdgeMesh::C_Mesh* pRadialMesh,
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		F32 angularTolerance)
	{
		if (angularTolerance > 180.f)
			angularTolerance = 180.f;
		if (angularTolerance < 0.f)
		{
			angularTolerance = 0.f;
		}
		angularTolerance = cosf(DegToRad(angularTolerance));

		//Perform pre-pass to make 2D pre-alloced array.
		//[Vertex relative PV index] dimensioned to max number of pvs per vertex [Vertex relative PV index] 
		//dimensioned to the max number of pvs per vertex -1. 
		//This is used as: [PV index][a list of pvs that can be smoothed with this PV, not including ourselves - hence max size -1]

		//Worst case - we have one smoothing group per pv.
		U32 maxPVs = 0;
		{
			RadialEdgeMesh::ListVert* pVertex = pRadialMesh->GetFirstVertex();
			while (pVertex)
			{
				if (pVertex->m_data.NumPolygonVertices() > maxPVs)
				{
					maxPVs = pVertex->m_data.NumPolygonVertices();
				}
				pVertex = pVertex->GetNext();
			}
		}

		//Worst case - we have one smoothing group per pv.
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector< Ut::Pair<S32,F32> > > > > smoothingGroups;
		smoothingGroups.Reserve(maxPVs);
		//Enough storage has been allocated to potentially store one smoothing group per PV.

		RadialEdgeMesh::ListVert* pVertex = pRadialMesh->GetFirstVertex();
		while (pVertex)
		{
			RadialEdgeMesh::ListPolyVert* pPolyVert = pVertex->m_data.GetFirstPolygonVertex();
			U32 iPolyVert = 0;

			smoothingGroups.Clear();
			while (pPolyVert)
			{
				U32 pv0Index = pPolyVert->m_data.GetAftkIndex();
				if ((pv0Index >= pAftkSceneMeshNode->GetNumPolygonVertices()))
				{
					//Skip invalid polygon vertex.
					pPolyVert = pPolyVert->GetNext();
					continue;
				}
				const Aftk::C_PolygonVertex* pPV0 = &pAftkSceneMeshNode->GetPolygonVertex(pv0Index);
				if (pPV0->GetNormalIndex() == -1)
				{
					pPolyVert = pPolyVert->GetNext();
					continue;
				}

				const Aftk::C_Polygon& polygon = pAftkSceneMeshNode->GetPolygon(pPolyVert->m_data.GetPolygon()->GetAftkIndex());
				if (!C_SmoothingHelper::ShouldSmooth(polygon))
				{
					pPolyVert = pPolyVert->GetNext();
					continue;
				}

				const Aftk::Normal3& n0 = pAftkSceneMeshNode->GetVertexNormal(pPV0->GetNormalIndex());
				
				bool smoothed = false;
				//See if we can add ourselves to any existing smoothing groups.
				for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
				{
					//Can we add to this group?
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][0].first;
					const Aftk::C_PolygonVertex* pPV1 = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);

					const Aftk::Normal3& n1 = 
						pAftkSceneMeshNode->GetVertexNormal(pPV1->GetNormalIndex());
						
					if (C_SmoothingHelper::CanSmooth(
						(Aftk::Vector3F32&)n0, 
						(Aftk::Vector3F32&)n1,
						1.0f,
						angularTolerance,
						true))
					{
						F32 w = 0.f;
						Aftk::Vector3F32 e1,e2;

						if (C_SmoothingHelper::GetEdgeVectors(polygon, pv0Index, e1, e2))
							w = C_SmoothingHelper::GetVertexWeight(e1, e2);

						//Add to smoothing group.
						smoothingGroups[jGroup].PushBack(Ut::Pair<S32,F32>(pv0Index, w));
						smoothed = true;
						break;
					}
				}

				//If we get here then we can't fit into any existing set, create a new one.
				if (!smoothed)
				{
					F32 w = 0.f;
					Aftk::Vector3F32 e1,e2;

					if (C_SmoothingHelper::GetEdgeVectors(polygon, pv0Index, e1, e2))
						w = C_SmoothingHelper::GetVertexWeight(e1, e2);

					smoothingGroups.PushBack();
					smoothingGroups[smoothingGroups.Size()-1].PushBack(Ut::Pair<S32,F32>(pv0Index, w));
				}

				iPolyVert++;
				pPolyVert = pPolyVert->GetNext();
			}

			//Okay, by the time we get here, we've got our populated smoothing groups. 
			//Perform the smoothing then!
			for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
			{
				U32 newNormalIndex = pAftkSceneMeshNode->AddVertexNormal((Aftk::Normal3&)Aftk::Vector3F32(0,0,0));
				Aftk::Normal3* pSmoothNorm = &pAftkSceneMeshNode->GetVertexNormal(newNormalIndex);
				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);
					const Aftk::Normal3& n = 
						pAftkSceneMeshNode->GetVertexNormal(pPV->GetNormalIndex());

					(Aftk::Vector3F32&)*pSmoothNorm = ((Aftk::Vector3F32&)*pSmoothNorm) + smoothingGroups[jGroup][kPV].second*(Aftk::Vector3F32&)n;
				}
				//Normalise our final normal.
				UtMaths::SafeNormalise((Aftk::Vector3F32*)pSmoothNorm);

				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);

					//Set new PV normal
					pPV->SetNormalIndex(newNormalIndex);
				}
			}

			pVertex = pVertex->GetNext();
		}

		//remove any redundant normals
		pAftkSceneMeshNode->RepairVertexNormals();
		return Aftk::C_Status::E_Status_Success;
	}
}
