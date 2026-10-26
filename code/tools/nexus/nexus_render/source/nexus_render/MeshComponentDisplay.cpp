#include <nexus_render/stdafx.h>
#include <nexus_render/MeshComponentDisplay.h>
#include <nexus_render/AftkUtility.h>

using namespace NexusRender;

MeshComponentDisplay::MeshComponentDisplay()
{
	m_pMesh = 0;
	m_pAftkISceneMeshNode = 0;
}

MeshComponentDisplay::~MeshComponentDisplay()
{
}

Tu::Status MeshComponentDisplay::Create(IRenderMeshNode* pMesh)
{
	Tu::Status status;

	m_pMesh = pMesh;

	m_pAftkISceneMeshNode = 
		(Aftk::I_SceneMeshNode*)AftkUtility::GetMeshInterfacePtr(m_pMesh->GetAftkSceneNodePtr());
	if (!m_pAftkISceneMeshNode)
		return Tu::Status::SUCCESS;
	ASSERT(m_pAftkISceneMeshNode->HasType(Aftk::E_ObjType_ISceneMesh));

	m_dataPerLOD.Resize(m_pAftkISceneMeshNode->GetNumLodLevels());

	for (U32 iLod = 0; iLod < (U32)m_pAftkISceneMeshNode->GetNumLodLevels(); iLod++)
	{	
		U32 nPVsUsed = 0;
		//Count the number of expanded PVs used by the currently active LOD.
		for (U32 j = 0; j < m_pAftkISceneMeshNode->GetNumPolygons(iLod); j++)
		{
			const Aftk::C_Polygon* pPoly = &m_pAftkISceneMeshNode->GetPolygon(iLod, j);
			for (U32 k = 0; k < pPoly->GetNumPolygonVertices(); k++)
				//Ignore PVs with a negative index.
				if (pPoly->GetPolygonVertexIndex(k) >= 0)
					nPVsUsed++;
		}

		status = CreateNormalVertexBuffer(iLod, nPVsUsed);
		if (!status) return status;

		status = CreateTangentSpaceVertexBuffers(iLod, nPVsUsed);
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

void MeshComponentDisplay::Destroy()
{
	if (!m_pMesh)
		return;

	m_dataPerLOD.Destroy();

	m_pMesh = 0;
}

void MeshComponentDisplay::Update(bool evaluate)
{
	for (U32 iLod = 0; iLod < (U32)m_dataPerLOD.Size(); iLod++)
	{
		UpdateNormals(iLod, evaluate);
	
		for (U32 jUVSet = 0; jUVSet < (U32)m_dataPerLOD[iLod].tangentSpaceVBs.Size(); jUVSet++)
			UpdateTangentSpace(iLod, jUVSet, evaluate);

		UpdateFaces(iLod, evaluate);
	}
}

void MeshComponentDisplay::UpdateTangentSpace(U32 lod, U16 set, bool evaluate)
{
//	return;
	if (set >= (U16)m_dataPerLOD[lod].tangentSpaceVBs.Size())
		return;

	if (m_dataPerLOD[lod].tangentSpaceVBs[set].GetNumVertices() == 0)
		return;

	//[TODO] prevent repeated extraction of scaling information.
	Maths::HMat43 m = (Maths::HMat43)m_pMesh->GetWorldMatrix();
	Maths::C_MatrixSRT srt = m.GetSRT();
	F32 scaling[] = {srt.Scale().X(),srt.Scale().Y(),srt.Scale().Z()};

	//Prevent div/0
	for (U32 i = 0; i < 3; i++)
	{
		if (scaling[i] < 1e-7f)
			scaling[i] = 1e-7f;
		scaling[i] = 1/scaling[i];
	}

	//Calculate distance to camera in world space.
	Maths::HVec3 cameraPos = m_pMesh->GetScenePtr()->GetCameraPtr()->GetPosition();
	Maths::HVec3 objPos(m.AW().X(), m.AW().Y(), m.AW().Z());
	F32 distance = objPos.Distance(cameraPos);
	F32 zoomScale = distance * 0.03f;

	m_dataPerLOD[lod].tangentSpaceVBs[set].Lock();
	U32 polyIdx = 0;
	U32 vtxIdx = 0;
	while (vtxIdx != (U32)m_dataPerLOD[lod].tangentSpaceVBs[set].GetNumVertices())
	{
		//Get the start of our D3D primitive
		D3DVertexPosCol* pVertex = 
			(D3DVertexPosCol*)m_dataPerLOD[lod].tangentSpaceVBs[set].GetVertexPtrByVertexIndex(vtxIdx);

		//Get the current polygon.
		const Aftk::C_Polygon* pPoly = &m_pAftkISceneMeshNode->GetPolygon(lod, polyIdx);
		for (U32 jPV = 0; jPV < pPoly->GetNumPolygonVertices(); jPV++)
		{
			U32 polyVertIndex = pPoly->GetPolygonVertexIndex(jPV);

			//Ignore PVs with a negative index.
			if (polyVertIndex < 0)
				continue;

			Maths::HVec3 pos;
			Maths::HVec3 norm;
			Maths::HVec3 tangent;
			Maths::HVec3 binormal;

			if (evaluate)
			{
				pos = Tu::TemplateToNightmare(
					m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexPosition(polyVertIndex));
				tangent = Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexTangent(set, polyVertIndex));
				binormal = Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexBinormal(set, polyVertIndex));
			}
			else
			{
				pos = Tu::TemplateToNightmare(
					m_pAftkISceneMeshNode->GetPolygonVertexPosition(polyVertIndex));
				tangent = 
					Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetPolygonVertexTangent(set, polyVertIndex));
				binormal = Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetPolygonVertexBinormal(set, polyVertIndex));
			}

			if (tangent.LengthSquared() + binormal.LengthSquared() > 1e-3f)
			{
				//Cross binormal and tangent, and normalise the result to give our tangent-space vertex normal.
				norm = binormal ^ tangent;
				norm.Normalise();
			}
			else
			{
				norm.X(0);
				norm.Y(0);
				norm.Z(0);
			}

			//Scale tangent
			tangent.X(tangent.X() * zoomScale * scaling[0]);
			tangent.Y(tangent.Y() * zoomScale * scaling[1]);
			tangent.Z(tangent.Z() * zoomScale * scaling[2]);				
			
			//Scale binormal
			binormal.X(binormal.X() * zoomScale * scaling[0]);
			binormal.Y(binormal.Y() * zoomScale * scaling[1]);
			binormal.Z(binormal.Z() * zoomScale * scaling[2]);

			//Scale normal
			norm.X(norm.X() * zoomScale * scaling[0]);
			norm.Y(norm.Y() * zoomScale * scaling[1]);
			norm.Z(norm.Z() * zoomScale * scaling[2]);

			//translate tangent
			tangent += pos;
			//translate binormal
			binormal += pos;
			//translate normal
			norm += pos;

			//Normal line (red)
			pVertex->SetPosition(pos[0], pos[1], pos[2]);
			pVertex->SetColour(255,0,0,0);
			pVertex++;

			pVertex->SetPosition(norm[0], norm[1], norm[2]);
			pVertex->SetColour(255,0,0,0);
			pVertex++;

			//Tangent line (green)
			pVertex->SetPosition(pos[0], pos[1], pos[2]);
			pVertex->SetColour(0,255,0,0);
			pVertex++;

			pVertex->SetPosition(tangent[0], tangent[1], tangent[2]);
			pVertex->SetColour(0,255,0,0);
			pVertex++;

			//Binormal line (blue)
			pVertex->SetPosition(pos[0], pos[1], pos[2]);
			pVertex->SetColour(0,0,255,0);
			pVertex++;

			pVertex->SetPosition(binormal[0], binormal[1], binormal[2]);
			pVertex->SetColour(0,0,255,0);
			pVertex++;
				
			vtxIdx+=6;
		}
		polyIdx++;
	}
	m_dataPerLOD[lod].tangentSpaceVBs[set].Unlock();
}

void MeshComponentDisplay::UpdateFaces(bool evaluate)
{
	for (U32 i = 0; i < m_pAftkISceneMeshNode->GetNumLodLevels(); i++)
		UpdateFaces(i, evaluate);
}

void MeshComponentDisplay::UpdateFaces(U32 lod, bool evaluate)
{
	if (m_dataPerLOD[lod].selectedFaceIndices.Size() == 0)
		return;

	m_dataPerLOD[lod].selectedFaceVB.Lock();
	U32 polyIdxIdx = 0;
	U32 polyIdx = 0;
	U32 vtxIdx = 0;

	const Aftk::Matrix44* pWorldMatrix;
	if (m_pAftkISceneMeshNode->HasType(Aftk::E_ObjType_SceneInstance))
		pWorldMatrix = &((Aftk::C_SceneInstanceNode*)m_pAftkISceneMeshNode)->GetSourceNodePtr()->GetWorldMatrix();
	else if (m_pAftkISceneMeshNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		pWorldMatrix = &((Aftk::C_SceneMeshRemappedInstanceNode*)m_pAftkISceneMeshNode)->GetSourceNodePtr()->GetWorldMatrix();
	else
		pWorldMatrix = &m_pAftkISceneMeshNode->GetWorldMatrix();

	while (
		vtxIdx != (U32)m_dataPerLOD[lod].selectedFaceVB.GetNumVertices() && 
		polyIdxIdx < (U32)m_dataPerLOD[lod].selectedFaceIndices.Size())
	{
		//Get the start of our D3D primitive
		D3DVertexPosCol* pVertex = 
			(D3DVertexPosCol*)m_dataPerLOD[lod].selectedFaceVB.GetVertexPtrByVertexIndex(vtxIdx);

		U32 polyIdx = m_dataPerLOD[lod].selectedFaceIndices[polyIdxIdx];
		//Get the current polygon.
		const Aftk::C_Polygon& poly = m_pAftkISceneMeshNode->GetPolygon(lod, polyIdx);
		
		for (Aftk::C_TriangleIterator triIt(poly, pWorldMatrix); triIt; triIt++)
		{
			const U32* aftkPvIndices = triIt;
			for (U32 jPV = 0; jPV < 3; jPV++)
			{
				U32 polyVertIndex0 = aftkPvIndices[jPV];
				
				//Ignore PVs with a negative index.
				if (polyVertIndex0 < 0)
					continue;

				Maths::HVec3 pos0;

				if (evaluate)
				{
					pos0 = Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexPosition(polyVertIndex0));
				}
				else
				{
					pos0 = Tu::TemplateToNightmare(
						m_pAftkISceneMeshNode->GetPolygonVertexPosition(polyVertIndex0));
				}

				//(maya-orange)
				pVertex->SetPosition(pos0[0], pos0[1], pos0[2]);
				pVertex->SetColour(228,172,121,190);
				pVertex++;
					
				vtxIdx++;
			}
		}
		polyIdxIdx++;
	}
	m_dataPerLOD[lod].selectedFaceVB.Unlock();
}

void MeshComponentDisplay::UpdatePoints(U32 lod, bool evaluate)
{
}

void MeshComponentDisplay::DrawNormals(U32 lod)
{
	m_dataPerLOD[lod].normalVB.Draw(D3DPT_LINELIST);
}

void MeshComponentDisplay::DrawTangentSpace(U32 lod, U16 set)
{
	if ((U16)m_dataPerLOD[lod].tangentSpaceVBs.Size() > set)
		m_dataPerLOD[lod].tangentSpaceVBs[set].Draw(D3DPT_LINELIST);
}

void MeshComponentDisplay::DrawSelectedFaces(U32 lod)
{
	m_dataPerLOD[lod].selectedFaceVB.Draw(D3DPT_TRIANGLELIST);
}

void MeshComponentDisplay::DrawSelectedPoints(U32 lod)
{
}

Tu::Status MeshComponentDisplay::CreateNormalVertexBuffer(U32 lod, U32 nPVs)
{
	Tu::Status status;

	//Create VB wrapper.
	status = m_dataPerLOD[lod].normalVB.Create(
		sizeof(D3DVertexPosCol),
		nPVs*2,
		2);
	if (!status) return status;

	return Tu::Status::SUCCESS;
}

Tu::Status MeshComponentDisplay::CreateTangentSpaceVertexBuffers(U32 lod, U32 nPVs)
{
	Tu::Status status;

	//Create VB wrapper.
	m_dataPerLOD[lod].tangentSpaceVBs.Resize(m_pAftkISceneMeshNode->GetNumTextureCoordinateSets());
	for (U32 i = 0; i < (U32)m_dataPerLOD[lod].tangentSpaceVBs.Size(); i++)
	{
		status = m_dataPerLOD[lod].tangentSpaceVBs[i].Create(
			sizeof(D3DVertexPosCol),
			nPVs*6,
			2);
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status MeshComponentDisplay::CreateFaceVertexBuffer(U32 lod)
{
 	Tu::Status status;

	U32 nPVs = 0;

	//Count the number of expanded PVs used by the currently active LOD.
	for (U32 i = 0; i < (U32)m_dataPerLOD[lod].selectedFaceIndices.Size(); i++)
	{
		U32 faceIndex = m_dataPerLOD[lod].selectedFaceIndices[i];
		const Aftk::C_Polygon& poly = m_pAftkISceneMeshNode->GetPolygon(lod, faceIndex);
		Aftk::C_TriangleIterator triIt(poly, &poly.GetMeshNodePtr()->GetWorldMatrix());
		nPVs += triIt.GetNumTriangles()*3;
	}

	if (nPVs == 0)
		return Tu::Status::SUCCESS;

	//Create VB wrapper.
	status = m_dataPerLOD[lod].selectedFaceVB.Create(
		sizeof(D3DVertexPosCol),
		nPVs,
		3);
	if (!status) return status;

	return Tu::Status::SUCCESS;
}

Tu::Status MeshComponentDisplay::CreatePointVertexBuffer(U32 lod)
{
	return Tu::Status::SUCCESS;
}

void MeshComponentDisplay::UpdateNormals(U32 lod, bool evaluate)
{
	if (m_dataPerLOD[lod].normalVB.GetNumVertices() == 0)
		return;

	//[TODO] prevent repeated extraction of scaling information.
	Maths::HMat43 m = (Maths::HMat43)m_pMesh->GetWorldMatrix();
	Maths::C_MatrixSRT srt = m.GetSRT();
	F32 scaling[] = {srt.Scale().X(),srt.Scale().Y(),srt.Scale().Z()};

	//Prevent div/0
	for (U32 i = 0; i < 3; i++)
	{
		if (scaling[i] < 1e-7f)
			scaling[i] = 1e-7f;
		scaling[i] = 1/scaling[i];
	}

	//Calculate distance to camera in world space.
	Maths::HVec3 cameraPos = m_pMesh->GetScenePtr()->GetCameraPtr()->GetPosition();
	Maths::HVec3 objPos(m.AW().X(), m.AW().Y(), m.AW().Z());
	F32 distance = objPos.Distance(cameraPos);
	F32 zoomScale = distance * 0.03f;

	m_dataPerLOD[lod].normalVB.Lock();
	U32 polyIdx = 0;
	U32 vtxIdx = 0;
	while (vtxIdx != (U32)m_dataPerLOD[lod].normalVB.GetNumVertices())
	{
		//Get the start of our D3D primitive
		D3DVertexPosCol* pVertex = 
			(D3DVertexPosCol*)m_dataPerLOD[lod].normalVB.GetVertexPtrByVertexIndex(vtxIdx);

		//Get the current polygon.
		const Aftk::C_Polygon* pPoly = &m_pAftkISceneMeshNode->GetPolygon(lod, polyIdx);
		for (U32 jPV = 0; jPV < pPoly->GetNumPolygonVertices(); jPV++)
		{
			U32 polyVertIndex = pPoly->GetPolygonVertexIndex(jPV);

			//Ignore PVs with a negative index.
			if (polyVertIndex < 0)
				continue;

			Aftk::Position3 pos;
			Aftk::Normal3 norm;
			if (evaluate)
			{
				pos = m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexPosition(polyVertIndex);
				norm = m_pAftkISceneMeshNode->GetEvaluatedPolygonVertexNormal(polyVertIndex);
			}
			else
			{
				pos = m_pAftkISceneMeshNode->GetPolygonVertexPosition(polyVertIndex);
				norm = m_pAftkISceneMeshNode->GetPolygonVertexNormal(polyVertIndex);
			}

			norm[0] = norm[0] * zoomScale * scaling[0];
			norm[1] = norm[1] * zoomScale * scaling[1];
			norm[2] = norm[2] * zoomScale * scaling[2];
				
			//translate normal
			norm += pos;

			//Normal line (red)
			pVertex->SetPosition(pos[0], pos[1], pos[2]);
			pVertex->SetColour(255,0,0,0);
			pVertex++;

			pVertex->SetPosition(norm[0], norm[1], norm[2]);
			pVertex->SetColour(255,0,0,0);
			pVertex++;

			vtxIdx+=2;
		}
		polyIdx++;
	}
	m_dataPerLOD[lod].normalVB.Unlock();
}

void MeshComponentDisplay::SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selection)
{
	for (U32 iLod = 0; iLod < m_pAftkISceneMeshNode->GetNumLodLevels(); iLod++)
	{
		m_dataPerLOD[iLod].selectedFaceIndices.Clear();
		m_dataPerLOD[iLod].selectedFaceVB.Destroy();
	}

	for (Ut::UtSet<Nexus::C_ComponentSelection>::ConstIterator it = selection.Begin(); 
		it != selection.End(); 
		++it)
	{
		const Nexus::C_ComponentSelection& componentSelection = it->GetKey();
		m_dataPerLOD[componentSelection.lodLevel].selectedFaceIndices.PushBack(
			componentSelection.componentIndex);
	}

	for (U32 iLod = 0; iLod < m_pAftkISceneMeshNode->GetNumLodLevels(); iLod++)
	{
		CreateFaceVertexBuffer(iLod);
	}
}

void MeshComponentDisplay::SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selection)
{
}