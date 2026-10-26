#ifndef APP_PREPROCESS_MORPH_TARGET_PROCESSOR_H
#define APP_PREPROCESS_MORPH_TARGET_PROCESSOR_H

#ifdef _MSC_VER
#pragma once
#endif

#include "aftk/Aftk.h"
#include "misc_shared/archive/btl.h"
#include <algorithm>

namespace Aftk { class C_Scene; }

namespace MeshDeform
{

namespace MorphTarget
{

/* 
===============================================================
					class C_MorphTargetProcessor

NOTES: 
===============================================================
*/
class C_MorphTargetProcessor
{
public:
	
	static C_MorphTargetProcessor& Instance() { return s_Instance; }

	C_MorphTargetProcessor() : m_pAftkScene(NULL), m_pCurrentMeshNode(NULL) {}

	void Process(Aftk::C_Scene* p_pAftkScene, Util::C_String p_Tag);

	void AddMorphTarget(Aftk::C_SceneMeshNode* p_pMeshNode) { m_pMorphTargets.PushBack(p_pMeshNode); }

	void SetCurrentMeshNode(Aftk::C_SceneMeshNode* p_pMeshNode) { m_pCurrentMeshNode = p_pMeshNode; }

public:

	static C_MorphTargetProcessor		s_Instance;

	Aftk::C_Scene*						m_pAftkScene;

	Util::C_String						m_Tag;

	Btl::Vector<Aftk::C_SceneMeshNode*>	m_pMorphTargets;

	Aftk::C_SceneMeshNode*				m_pCurrentMeshNode;
};

/* 
===============================================================
					class C_GetMorphTargets

NOTES: Class to visit Aftk nodes and identify morph targets
===============================================================
*/
class C_GetMorphTargets
{
public:
	C_GetMorphTargets(C_MorphTargetProcessor* p_pProcessor, Util::C_String p_Tag);

	bool IsInteresting(Aftk::C_SceneNode* p_pSceneNode);

	Aftk::C_Status Visit(Aftk::C_SceneNode* p_pNode, C_NullMemoryManager& memoryManager);

	Util::C_String				m_Tag;

	C_MorphTargetProcessor*		m_pProcessor;
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
	C_FindMeshNode(C_MorphTargetProcessor* p_pProcessor, Util::C_String p_Name)
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

	Util::C_String				m_Name;

	C_MorphTargetProcessor*		m_pProcessor;
};

/* 
===============================================================
					class C_DeleteMeshNode

NOTES: Class to delete a mesh node by name from the Aftk scene
===============================================================
*/
class C_DeleteMeshNode
{
public:
	C_DeleteMeshNode(Util::C_String p_Name) 
	{
		m_Name = p_Name;
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
		if (name == m_Name)
		{
			p_pNode->GetScenePtr()->DeleteSceneNodeAndReparentChildren(p_pNode, true);
			return Aftk::C_Status::E_Status_Failure;
		}

		return Aftk::C_Status::E_Status_Success;
	}

	Util::C_String				m_Name;
};

} // namespace MorphTarget

} // namespace MeshDeform

#endif




