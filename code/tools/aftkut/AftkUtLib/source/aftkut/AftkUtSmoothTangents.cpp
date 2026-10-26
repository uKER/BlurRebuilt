#include "stdafx.h"

namespace AftkUt
{
	Aftk::C_Status C_SmoothTangents::Smooth(
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		F32 magnitudeScale, 
		F32 angularTolerance,
		bool normalise)
	{
		Aftk::C_Status status;
		RadialEdgeMesh::C_Mesh* pRadialMesh = new RadialEdgeMesh::C_Mesh(pAftkSceneMeshNode);
		pRadialMesh->Build(status, false);
		if (!status) return status;

		status = Smooth(
			pRadialMesh,
			pAftkSceneMeshNode,
			magnitudeScale,
			angularTolerance,
			normalise);

		delete pRadialMesh;

		return status;
	}
	Aftk::C_Status C_SmoothTangents::Smooth(
		const RadialEdgeMesh::C_Mesh* pRadialMesh,
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		F32 magnitudeScale, 
		F32 angularTolerance,
		bool normalise)
	{
		Aftk::C_Status status;
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
		RadialEdgeMesh::ListVert* pVertex = pRadialMesh->GetFirstVertex();
		U32 maxPVs = 0;
		while (pVertex)
		{
			if (pVertex->m_data.NumPolygonVertices() > maxPVs)
			{
				maxPVs = pVertex->m_data.NumPolygonVertices();
			}
			pVertex = pVertex->GetNext();
		}

		for (U32 iSet = 0; iSet < pAftkSceneMeshNode->GetNumTextureCoordinateSets(); iSet++)
		{
			status = SmoothTangentsOnly(
				pAftkSceneMeshNode,
				pRadialMesh,
				iSet,
				maxPVs,
				magnitudeScale,
				angularTolerance,
				normalise);
			if (!status) return status;

			status = SmoothBinormalsOnly(
				pAftkSceneMeshNode,
				pRadialMesh,
				iSet,
				maxPVs,
				magnitudeScale,
				angularTolerance,
				normalise);
			if (!status) return status;
		}
	
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status C_SmoothTangents::SmoothTangentsOnly(
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const RadialEdgeMesh::C_Mesh* pRadialMesh,
		U32 iSet,
		U32 maxPVs,
		F32 magnitudeScale, 
		F32 angularTolerance,
		bool normalise)
	{
		Aftk::C_Status status;

		//Worst case - we have one smoothing group per pv.
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector<Ut::Pair<S32,F32> > > > > smoothingGroups;
		smoothingGroups.Reserve(maxPVs);
		//Enough storage has been allocated to potentially store one smoothing group per PV.

		RadialEdgeMesh::ListVert* pVertex = pRadialMesh->GetFirstVertex();
		while (pVertex)
		{
			RadialEdgeMesh::ListPolyVert* pPolyVert = pVertex->m_data.GetFirstPolygonVertex();

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
				if (pPV0->GetTangentIndex(iSet) == -1)
				{
					pPolyVert = pPolyVert->GetNext();
					continue;
				}
				const Aftk::C_Polygon& polygon = pAftkSceneMeshNode->GetPolygon(pPolyVert->m_data.GetPolygon()->GetAftkIndex());
				const Aftk::Vector3F32& t0 = pAftkSceneMeshNode->GetVertexTangent(iSet, pPV0->GetTangentIndex(iSet));
				
				bool smoothed = false;

				//See if we can add ourselves to any existing smoothing groups.
				for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
				{
					//Can we add to this group?
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][0].first;
					const Aftk::C_PolygonVertex* pPV1 = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);

					const Aftk::Vector3F32& t1 = 
						pAftkSceneMeshNode->GetVertexTangent(iSet,
							pPV1->GetTangentIndex(iSet));
						
					if (C_SmoothingHelper::CanSmooth(t0, t1, magnitudeScale, angularTolerance, normalise))
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

				pPolyVert = pPolyVert->GetNext();
			}

			//Okay, by the time we get here, we've got our populated smoothing groups. 
			//Perform the smoothing then!
			for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
			{
				U32 newTangentIndex = pAftkSceneMeshNode->AddVertexTangent(iSet, Aftk::Vector3F32(0,0,0));
				Aftk::Vector3F32* pSmoothTan = &pAftkSceneMeshNode->GetVertexTangent(iSet, newTangentIndex);
				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);
					const Aftk::Vector3F32& t = 
						pAftkSceneMeshNode->GetVertexTangent(iSet,pPV->GetTangentIndex(iSet));

					*pSmoothTan = *pSmoothTan + smoothingGroups[jGroup][kPV].second * t;
				}
				//Normalise our final tangent if needed.
				if (normalise)
					UtMaths::SafeNormalise(pSmoothTan);
				else
				{
					//Average
					(*pSmoothTan)[0] /= (F32)smoothingGroups[jGroup].Size();
					(*pSmoothTan)[1] /= (F32)smoothingGroups[jGroup].Size();
					(*pSmoothTan)[2] /= (F32)smoothingGroups[jGroup].Size();
				}

				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);

					//Set new PV tangent index
					pPV->SetTangentIndex(iSet, newTangentIndex);
				}
			}

			pVertex = pVertex->GetNext();
		}
		//remove any redundant tangents
		pAftkSceneMeshNode->RepairVertexTangents();
		return Aftk::C_Status::E_Status_Success;
	}

	Aftk::C_Status C_SmoothTangents::SmoothBinormalsOnly(
		Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
		const RadialEdgeMesh::C_Mesh* pRadialMesh,
		U32 iSet,
		U32 maxPVs,
		F32 magnitudeScale, 
		F32 angularTolerance,
		bool normalise)
	{
		Aftk::C_Status status;

		//Worst case - we have one smoothing group per pv.
		Ut::UtVector<Ut::DynamicVector<
			Ut::UtVector<Ut::DynamicVector< Ut::Pair<S32,F32> > > > > smoothingGroups;
		smoothingGroups.Reserve(maxPVs);
		//Enough storage has been allocated to potentially store one smoothing group per PV.

		RadialEdgeMesh::ListVert* pVertex = pRadialMesh->GetFirstVertex();
		while (pVertex)
		{
			RadialEdgeMesh::ListPolyVert* pPolyVert = pVertex->m_data.GetFirstPolygonVertex();

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

				const Aftk::Vector3F32& tempo = pPolyVert->m_data.GetTextureCoordinates(iSet).m_binormals;
				const Aftk::C_PolygonVertex* pPV0 = &pAftkSceneMeshNode->GetPolygonVertex(pv0Index);
				if (pPV0->GetBinormalIndex(iSet) == -1)
				{
					pPolyVert = pPolyVert->GetNext();
					continue;
				}
				const Aftk::C_Polygon& polygon = pAftkSceneMeshNode->GetPolygon(pPolyVert->m_data.GetPolygon()->GetAftkIndex());
				const Aftk::Vector3F32& b0 = pAftkSceneMeshNode->GetVertexBinormal(iSet, pPV0->GetBinormalIndex(iSet));
				
				bool smoothed = false;
				//See if we can add ourselves to any existing smoothing groups.
				for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
				{
					//Can we add to this group?
					U32 smoothingTestPVIndex = smoothingGroups[jGroup][0].first;
					const Aftk::C_PolygonVertex* pPV1 = &pAftkSceneMeshNode->GetPolygonVertex(smoothingTestPVIndex);

					const Aftk::Vector3F32& b1 = 
						pAftkSceneMeshNode->GetVertexBinormal(iSet,
							pPV1->GetBinormalIndex(iSet));
						
					if (C_SmoothingHelper::CanSmooth(b0, b1, magnitudeScale, angularTolerance, normalise))
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

				pPolyVert = pPolyVert->GetNext();
			}

			//Okay, by the time we get here, we've got our populated smoothing groups. 
			//Perform the smoothing then!
			for (U32 jGroup = 0; jGroup < (U32)smoothingGroups.Size(); jGroup++)
			{
				U32 newBinormalIndex = pAftkSceneMeshNode->AddVertexBinormal(iSet, Aftk::Vector3F32(0,0,0));
				Aftk::Vector3F32* pSmoothBin = &pAftkSceneMeshNode->GetVertexBinormal(iSet, newBinormalIndex);
				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingPVIndex);
					const Aftk::Vector3F32& b = 
						pAftkSceneMeshNode->GetVertexBinormal(iSet,pPV->GetBinormalIndex(iSet));

					*pSmoothBin = *pSmoothBin + smoothingGroups[jGroup][kPV].second * b;
				}
				//Normalise our final binormal if needed.
				if (normalise)
					UtMaths::SafeNormalise(pSmoothBin);
				else
				{
					//Average
					(*pSmoothBin)[0] /= (F32)smoothingGroups[jGroup].Size();
					(*pSmoothBin)[1] /= (F32)smoothingGroups[jGroup].Size();
					(*pSmoothBin)[2] /= (F32)smoothingGroups[jGroup].Size();
//					std::cout << "trap: " << trap << " " << (*pSmoothBin)[0] << " " << (*pSmoothBin)[1] << " " << (*pSmoothBin)[2] << " " << temp << std::endl;
				}

				for (U32 kPV = 0; kPV < (U32)smoothingGroups[jGroup].Size(); kPV++)
				{
					U32 smoothingPVIndex = smoothingGroups[jGroup][kPV].first;
					Aftk::C_PolygonVertex* pPV = &pAftkSceneMeshNode->GetPolygonVertex(smoothingPVIndex);

					//Set new PV binormal index
					pPV->SetBinormalIndex(iSet, newBinormalIndex);
				}
			}

			pVertex = pVertex->GetNext();
		}
		//remove any redundant binormals
		pAftkSceneMeshNode->RepairVertexBinormals();

		return Aftk::C_Status::E_Status_Success;
	}
}
