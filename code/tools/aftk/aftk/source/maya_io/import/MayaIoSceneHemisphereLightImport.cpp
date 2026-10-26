#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneHemisphereLightImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Export the base AFTK scene node
	status = C_SceneLightImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		pOptions);
	CHECK_ERROR_RETURN;

	Aftk::C_SceneHemisphereLightNode* pAftkSceneHemisphereLightNode = (Aftk::C_SceneHemisphereLightNode*)pAftkSceneNode;

	MFnLight mayaLight(dagPath, &mStatus);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL MCHECK_ERROR_RETURN_END_1;

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Hemisphere Light C_Node " << mayaLight.name().asChar() << " id " << mayaLight.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

