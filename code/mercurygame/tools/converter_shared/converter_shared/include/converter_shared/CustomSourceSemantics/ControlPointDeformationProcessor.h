#ifndef APP_PREPROCESS_CONTROL_POINT_PROCESSOR_H
#define APP_PREPROCESS_CONTROL_POINT_PROCESSOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "aftk/Aftk.h"
#include "misc_shared/archive/btl.h"
#include <algorithm>

namespace Aftk { class C_Scene; }

namespace MeshDeform
{

namespace ControlPoint
{

class C_ControlPoint
{
public:
	
	C_ControlPoint() 
	{ 
		m_pMeshNode		= NULL; 		
		m_MaxDimension	= -1;
		memset(m_Dimensions, 0, sizeof(float)*3);
	}

	void Sort();

public:

	Aftk::C_SceneMeshNode*			m_pMeshNode;

	float							m_Dimensions[3];
	int								m_MaxDimension;

	Btl::Vector<Btl::Pair<Aftk::C_SceneNode*, S32> >	m_pControlPoints;
};

/* 
===============================================================
					class C_ControlPointProcessor

NOTES: 
===============================================================
*/
class C_ControlPointProcessor
{
public:
	
	static C_ControlPointProcessor& Instance() { return s_Instance; }

	C_ControlPointProcessor() : m_pAftkScene(NULL) {}

	void Process(Aftk::C_Scene* p_pAftkScene, Util::C_String p_Tag);

	void AddControlPoint(Aftk::C_SceneNode* p_pNode) { m_pSceneNodes.PushBack(); m_pSceneNodes.Back().first = false; m_pSceneNodes.Back().second = p_pNode; }

	void SetCurrentMeshNode(Aftk::C_SceneMeshNode* p_pMeshNode) { m_pCurrentMeshNode = p_pMeshNode; }

	S32 GetMapping(Util::C_String p_Name);

public:

	static C_ControlPointProcessor	s_Instance;

	Aftk::C_Scene*							m_pAftkScene;

	Util::C_String							m_Tag;

	Btl::Vector< Btl::Pair<bool, Aftk::C_SceneNode*> >	m_pSceneNodes;
	
	
	Btl::Vector<C_ControlPoint>			m_ControlPoints;

	Aftk::C_SceneMeshNode*				m_pCurrentMeshNode;
};

/* 
===============================================================
					class C_GetDynamicMorphTargets

NOTES: Class to visit Aftk nodes and identify dynamic morph targets
===============================================================
*/
class C_GetAllControlPoints
{
public:
	C_GetAllControlPoints(C_ControlPointProcessor* p_pProcessor, Util::C_String p_Tag)
	{
		m_pProcessor	= p_pProcessor;
		m_Tag			= p_Tag.Lower();
	}

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
	{
		if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneNode)
			return true;

		return false;
	}

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
	{
		if (p_pNode->GetType() != Aftk::E_ObjType_SceneNode)
		{
			//Ignore this node.
			return Aftk::C_Status::E_Status_Success;
		}

		Aftk::C_SceneNode* pNode = (Aftk::C_SceneNode*)p_pNode;
		Util::C_String name = pNode->GetName().c_str();

		if (name.ISearch(m_Tag) != -1)
		{
			if (name.Length() > 2)
			{
				if (name.Chars()[0] != 'm' && name.Chars()[1] != '_')
				{
					pNode->SetName("m_" + pNode->GetName());
				}
			}
			m_pProcessor->AddControlPoint(pNode);
		}

		return Aftk::C_Status::E_Status_Success;
	}

	Util::C_String					m_Tag;

	C_ControlPointProcessor*	m_pProcessor;
};

/* 
===============================================================
					class C_FindMeshNode

NOTES: Class to find a node by name and pass it to processor
===============================================================
*/
class C_FindMeshNode
{
public:
	C_FindMeshNode(C_ControlPointProcessor* p_pProcessor, Util::C_String p_Name)
	{
		m_Name			= p_Name;
		m_pProcessor	= p_pProcessor;
	}

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode)
	{
		if (p_pSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			return true;

		return false;
	}

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager)
	{
		if (p_pNode->GetType() != Aftk::E_ObjType_SceneMesh)
		{
			//Ignore this node.
			return Aftk::C_Status::E_Status_Success;
		}

		Aftk::C_SceneMeshNode* pMesh = (Aftk::C_SceneMeshNode*)p_pNode;
		Util::C_String name = pMesh->GetName().c_str();
		if (name.Lower() == m_Name)
			m_pProcessor->SetCurrentMeshNode(pMesh);

		return Aftk::C_Status::E_Status_Success;
	}

	Util::C_String					m_Name;

	C_ControlPointProcessor*	m_pProcessor;
};

} //namespace ControlPoint

} //namespace MeshDeform

#endif




