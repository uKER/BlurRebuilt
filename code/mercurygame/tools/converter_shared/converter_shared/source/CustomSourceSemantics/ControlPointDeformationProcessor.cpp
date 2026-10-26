#include "stdafx.h"

#include <converter_shared/CustomSourceSemantics/ControlPointDeformationProcessor.h>

//#pragma optimize("", off)

namespace MeshDeform
{

namespace ControlPoint
{

/* 
===============================================================
					class C_ControlPoint

NOTES: 
===============================================================
*/
void C_ControlPoint::Sort()
{
	if (m_pControlPoints.Size() < 2)
		return;

	Maths::HVec3 start = Maths::HVec3(m_pControlPoints[0].first->GetWorldBounds().GetCentre().X(), m_pControlPoints[0].first->GetWorldBounds().GetCentre().Y(), m_pControlPoints[0].first->GetWorldBounds().GetCentre().Z());
	Maths::HVec3 end = Maths::HVec3(m_pControlPoints[m_pControlPoints.Size()-1].first->GetWorldBounds().GetCentre().X(), m_pControlPoints[m_pControlPoints.Size()-1].first->GetWorldBounds().GetCentre().Y(), m_pControlPoints[m_pControlPoints.Size()-1].first->GetWorldBounds().GetCentre().Z());
	Maths::HVec3 disp = end - start;
	m_Dimensions[0] = Maths::Abs(disp.X());
	m_Dimensions[1] = Maths::Abs(disp.Y());
	m_Dimensions[2] = Maths::Abs(disp.Z());
	m_MaxDimension = -1;

	if (m_Dimensions[0] > m_Dimensions[1])
	{
		if (m_Dimensions[2] > m_Dimensions[0])
			m_MaxDimension = 2;
		else
			m_MaxDimension = 0;
	}
	else
	{
		if (m_Dimensions[2] > m_Dimensions[1])
			m_MaxDimension = 2;
		else
			m_MaxDimension = 1;
	}

	for (int iCP = 0; iCP < m_pControlPoints.Size(); iCP++)
	{
		if (m_MaxDimension == 0)
			m_pControlPoints[iCP].first->SetName(m_pControlPoints[iCP].first->GetName() + "X");
		else if (m_MaxDimension == 1)
			m_pControlPoints[iCP].first->SetName(m_pControlPoints[iCP].first->GetName() + "Y");
		else if (m_MaxDimension == 2)
			m_pControlPoints[iCP].first->SetName(m_pControlPoints[iCP].first->GetName() + "Z");
	}
}

/* 
===============================================================
					class C_ControlPointProcessor

NOTES: 
===============================================================
*/

C_ControlPointProcessor C_ControlPointProcessor::s_Instance;

/*
===============
NAME: GetMapping

NOTE: CP to Part mapping
===============
*/
S32 C_ControlPointProcessor::GetMapping(Util::C_String p_Name)
{
	//Bumper front	0 -> 2
	//Bumper back	3 -> 5
	//Bonnet		6 -> 8
	//Boot			9 -> 11
	//Wing left		12 -> 14
	//Wing right	15 -> 17
	S32 iNum = -1;
	if (p_Name.ISearch("bone0") != -1)
		iNum = 0;
	else if (p_Name.ISearch("bone1") != -1)
		iNum = 1;
	else if (p_Name.ISearch("bone2") != -1)
		iNum = 2;

	S32 iPos = -1;
	if (p_Name.ISearch("bumper") != -1 && p_Name.ISearch("front") != -1)
		iPos = 0;
	else if (p_Name.ISearch("bumper") != -1 && p_Name.ISearch("back") != -1)
		iPos = 3;
	else if (p_Name.ISearch("bonnet") != -1)
		iPos = 6;
	else if (p_Name.ISearch("boot") != -1)
		iPos = 9;
	else if (p_Name.ISearch("wing") != -1 && p_Name.ISearch("left") != -1)
		iPos = 12;
	else if (p_Name.ISearch("wing") != -1 && p_Name.ISearch("right") != -1)
		iPos = 15;

	if (iNum == -1 || iPos == -1)
		return -1;

	return (iPos + iNum);
}

/*
===============
NAME: Process

NOTE: Pre-process the Aftk scene and prepare morph targets
===============
*/
void C_ControlPointProcessor::Process(Aftk::C_Scene* p_pAftkScene, Util::C_String p_Tag)
{	
	m_pAftkScene		= p_pAftkScene;
	m_Tag				= p_Tag.Lower();

	if (!m_pAftkScene)
		return;
	
	m_pSceneNodes.Clear();
	m_ControlPoints.Clear();

	//1) Find all control-point nodes in the scene and build a list
	C_GetAllControlPoints get(this, m_Tag);
	Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, get);

	//2) For each control-point, find the corresponding mesh
	for (S32 iCP = 0; iCP < m_pSceneNodes.Size(); iCP++)
	{
		//Already processed this one
		if (m_pSceneNodes[iCP].first)
			continue;

		Util::C_String name = m_pSceneNodes[iCP].second->GetName().c_str();
		if (name.Length() < 6)
			continue;

		char meshName[MAX_PATH];
		memset(meshName, 0, MAX_PATH);
		char searchName[MAX_PATH];
		name.ConvertToLower();
		strncpy(meshName, name.Chars(), name.Length() - 6);
		_snprintf_s(searchName, lengthof(meshName), _TRUNCATE, "%s", meshName);

		m_pCurrentMeshNode = NULL;

		C_FindMeshNode findMeshNode(this, searchName);
		Aftk::C_SceneNodeVisit::Traverse(m_pAftkScene, findMeshNode);

		if (!m_pCurrentMeshNode)
			continue;

		//Check we haven't already found the source mesh for this control point
		C_ControlPoint* pControlPont = NULL;
		for (S32 iNode = 0; iNode < m_ControlPoints.Size(); iNode++)
		{
			if (m_pCurrentMeshNode == m_ControlPoints[iNode].m_pMeshNode)
			{
				pControlPont = &m_ControlPoints[iNode];
				break;
			}
		}
		//Haven't processed this mesh yet, so add a new control point
		if (!pControlPont)
		{
			m_ControlPoints.PushBack();
			m_ControlPoints.Back().m_pMeshNode = m_pCurrentMeshNode;
			m_ControlPoints.Back().m_pControlPoints.PushBack();
			m_ControlPoints.Back().m_pControlPoints.Back().first = m_pSceneNodes[iCP].second;
			m_ControlPoints.Back().m_pControlPoints.Back().second = GetMapping(m_pSceneNodes[iCP].second->GetName().c_str());
			m_pSceneNodes[iCP].first = true;
			pControlPont = &m_ControlPoints.Back();
		}

		//Oh dear
		if (!pControlPont)
			continue;

		//Go through all control-point scene-nodes found and add it to the current control-point
		for (S32 iNode2 = 0; iNode2 < m_pSceneNodes.Size(); iNode2++)
		{
			//Already added this one
			if (m_pSceneNodes[iNode2].first)
				continue;

			name = m_pSceneNodes[iNode2].second->GetName().c_str();
			name.ConvertToLower();
			if (name.ISearch(meshName) != -1)
			{
				pControlPont->m_pControlPoints.PushBack();
				pControlPont->m_pControlPoints.Back().first = m_pSceneNodes[iNode2].second;
				pControlPont->m_pControlPoints.Back().second = GetMapping(m_pSceneNodes[iNode2].second->GetName().c_str());
				m_pSceneNodes[iNode2].first = true;
			}
		}
	}
	
	//Sort the bad boys
	for (S32 iCP = 0; iCP < m_ControlPoints.Size(); iCP++)
		m_ControlPoints[iCP].Sort();

	//Now do the actual processing of vertices
	for (S32 iCP = 0; iCP < m_ControlPoints.Size(); iCP++)
	{
		C_ControlPoint* pControlPoint = &m_ControlPoints[iCP];
		Aftk::C_SceneMeshNode* pMeshNode = pControlPoint->m_pMeshNode;

		if (!pMeshNode)
			continue;

		int vertexDataChannelStart = pMeshNode->GetNumVertexDataChannels();

		//Set up the vertex data channel
		Aftk::C_VertexDataChannelInfo info;
		
		//Vertex channel information
		info.SetName("controlPointInfo");
		info.SetDataType(Aftk::E_GenericAttributeType_Vector4F32);		
		pMeshNode->AddVertexDataChannelInfo(info);

		int numVtx = pMeshNode->GetNumPolygonVertices();

		//Go through all the vertices and get the closest control points and work out weighting
		for (S32 iVtx = 0; iVtx < numVtx; iVtx++)
		{
			Aftk::C_PolygonVertex& vtx = pMeshNode->GetPolygonVertex(iVtx);
			Aftk::Position3 vtxPos = pMeshNode->GetPolygonVertexPosition(vtx.GetPositionIndex());
			Maths::HVec3 ax(pMeshNode->GetWorldMatrix().AX().x, pMeshNode->GetWorldMatrix().AX().y, pMeshNode->GetWorldMatrix().AX().z);
			Maths::HVec3 ay(pMeshNode->GetWorldMatrix().AY().x, pMeshNode->GetWorldMatrix().AY().y, pMeshNode->GetWorldMatrix().AY().z);
			Maths::HVec3 az(pMeshNode->GetWorldMatrix().AZ().x, pMeshNode->GetWorldMatrix().AZ().y, pMeshNode->GetWorldMatrix().AZ().z);
			Maths::HVec3 aw(pMeshNode->GetWorldBounds().GetCentre().x, pMeshNode->GetWorldBounds().GetCentre().y, pMeshNode->GetWorldBounds().GetCentre().z);
			Maths::HMat43 mat(ax, ay, az, aw);
			Maths::HVec3 vPos(vtxPos.x, vtxPos.y, vtxPos.z);
			vPos = vPos * mat;
			vtxPos = Aftk::Position3(vPos.X(), vPos.Y(), vPos.Z());
			
			S32 i0 = -1;
			S32 i1 = -1;
			float fVtxDist0 = 999999999999.9f;
			float fVtxDist1 = 999999999999.9f;

			//Find first closest
			for (S32 iPos = 0; iPos < pControlPoint->m_pControlPoints.Size(); iPos++)
			{
				Aftk::C_SceneNode* pSceneNode = pControlPoint->m_pControlPoints[iPos].first;
				
				Aftk::Position3 pos = pSceneNode->GetWorldBounds().GetCentre();
				Aftk::Vector3F32 vec = pos - vtxPos;
				float dimensions[3];
				dimensions[0] = Maths::Abs(vec.X());
				dimensions[1] = Maths::Abs(vec.Y());
				dimensions[2] = Maths::Abs(vec.Z());

				if (dimensions[pControlPoint->m_MaxDimension] < fVtxDist0)
				{
					fVtxDist0 = dimensions[pControlPoint->m_MaxDimension];
					i0 = iPos;
				}
			}
			//Find next closest
			for (S32 iPos = 0; iPos < pControlPoint->m_pControlPoints.Size(); iPos++)
			{
				if (iPos == i0)
					continue;
				Aftk::C_SceneNode* pSceneNode = pControlPoint->m_pControlPoints[iPos].first;
				
				Aftk::Position3 pos = pSceneNode->GetWorldBounds().GetCentre();
				Aftk::Vector3F32 vec = pos - vtxPos;
				float dimensions[3];
				dimensions[0] = Maths::Abs(vec.X());
				dimensions[1] = Maths::Abs(vec.Y());
				dimensions[2] = Maths::Abs(vec.Z());

				if (dimensions[pControlPoint->m_MaxDimension] < fVtxDist1)
				{
					fVtxDist1 = dimensions[pControlPoint->m_MaxDimension];
					i1 = iPos;					
				}
			}

			if (i0 == -1 || i1 == -1)
				continue;

			Aftk::Position3 pos = pControlPoint->m_pControlPoints[i0].first->GetWorldBounds().GetCentre();
			Aftk::Position3 pos1 = pControlPoint->m_pControlPoints[i1].first->GetWorldBounds().GetCentre();
			S32 index0 = pControlPoint->m_pControlPoints[i0].second;
			S32 index1 = pControlPoint->m_pControlPoints[i1].second;
			float vec[3];
			vec[0] = Maths::Abs(pos1.X() - pos.X());
			vec[1] = Maths::Abs(pos1.Y() - pos.Y());
			vec[2] = Maths::Abs(pos1.Z() - pos.Z());
			
			float distVec[3];
			distVec[0] = Maths::Abs(vtxPos.X() - pos.X());
			distVec[1] = Maths::Abs(vtxPos.Y() - pos.Y());
			distVec[2] = Maths::Abs(vtxPos.Z() - pos.Z());

			float fDist = distVec[pControlPoint->m_MaxDimension];
			float fRange = vec[pControlPoint->m_MaxDimension];
			//TODO: This is linear - make it evaluate a curve or something to bias the blend-weight
			float fBlendWeight = Maths::Clamp(fDist / fRange, 0.0f, 1.0f);
			//fBlendWeight = Maths::Sin(fBlendWeight * Maths::PiBy2) * 0.5f;
			Aftk::Vector4F32 info((float)(index0), (float)(index1), fBlendWeight*254.0f, 0.0f);
			vtx.SetData(vertexDataChannelStart, info);
		}
	}

	m_pSceneNodes.Clear();
	m_ControlPoints.Clear();
}

} // namespace ControlPoint

} //namespace MeshDeform
