#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_ScenePointLightImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;

	#ifdef DEBUG_PRINT
	AFTK_DO_BEGIN << "Exporting point light.\n" AFTK_DO_DEBUG_END;
	#endif
	//Export the base AFTK non ambient light node
	status = C_SceneNonAmbientLightImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	#ifdef _DEBUG
	MFnPointLight mfnPointLightNode(dagPath);	
	AFTK_DO_BEGIN << "Point light node " << mfnPointLightNode.name().asChar() << " id " << mfnPointLightNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

