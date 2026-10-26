#include "stdafx.h"

using namespace AftkUt;

Aftk::C_Status C_Utility::ReparentInstance(
	Aftk::C_SceneNode* pInstance, 
	Aftk::C_SceneNode* pInstanceSource)
{
	if (pInstance->HasType(Aftk::E_ObjType_SceneInstance))
	{
		((Aftk::C_SceneInstanceNode*)pInstance)->SetSourceNodePtr(pInstanceSource);
	}
	else if (pInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
	{
		//Only mesh instancing is allowed in this case.
		if (pInstanceSource->HasType(Aftk::E_ObjType_SceneMesh))
			((Aftk::C_SceneMeshRemappedInstanceNode*)pInstance)->SetSourceNodePtr(
				(Aftk::C_SceneMeshNode*)pInstanceSource);
		else
		{
			((Aftk::C_SceneMeshRemappedInstanceNode*)pInstance)->SetSourceNodePtr(0);
			LOGFATAL(g_Log, "Invalid remapped mesh instance %s encountered", pInstance->GetPath().c_str());
			return Aftk::C_Status::E_Status_Failure;
		}
	}
	else
	{
		//Unknown instance type.
		LOGFATAL(g_Log, "Unknown instance type.", pInstance->GetPath().c_str());
		return Aftk::C_Status::E_Status_Failure;
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_SceneNode* C_Utility::GetInstanceSourceNodePtr(Aftk::C_SceneNode* pNode)
{
	switch(pNode->GetType())
	{
		default:
		break;
		
		case Aftk::E_ObjType_SceneInstance:
		{
			Aftk::C_SceneInstanceNode* pInstanceNode = (Aftk::C_SceneInstanceNode*)pNode;
			return pInstanceNode->GetSourceNodePtr();
		}
		break; 

		case Aftk::E_ObjType_SceneMeshRemappedInstance:
		{
			Aftk::C_SceneMeshRemappedInstanceNode* pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*)pNode;
			return pInstanceNode->GetSourceNodePtr();
		}
		break; 
	}
	
	return pNode;
}

const Aftk::C_SceneNode* C_Utility::GetInstanceSourceNodePtr(const Aftk::C_SceneNode* pNode)
{
	return GetInstanceSourceNodePtr(const_cast<Aftk::C_SceneNode*>(pNode));
}
