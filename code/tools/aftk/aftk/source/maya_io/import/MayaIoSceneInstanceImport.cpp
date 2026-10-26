#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneInstanceImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions,
	Aftk::C_SceneNode* pAftkSourceNode)
{
	Aftk::C_Status status;
	//Export the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;
		
	Aftk::C_SceneInstanceNode* pAftkSceneInstanceNode = (Aftk::C_SceneInstanceNode*)pAftkSceneNode;

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "New Instance [" << pAftkSceneInstanceNode->GetName() << "]\n" AFTK_DO_DEBUG_END;
	#endif

	pAftkSceneInstanceNode->SetSourceNodePtr(pAftkSourceNode);
	return Aftk::C_Status::E_Status_Success;
}

