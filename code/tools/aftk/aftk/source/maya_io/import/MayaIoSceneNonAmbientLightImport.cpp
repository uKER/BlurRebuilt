#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneNonAmbientLightImport::Import(	
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	#ifdef DEBUG_PRINT
	AFTK_DO_BEGIN << "Exporting non ambient light.\n" AFTK_DO_DEBUG_END;
	#endif
	//Export the base AFTK light node
	status = C_SceneLightImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneNonAmbientLightNode* pAftkSceneNonAmbientLightNode = 
		(Aftk::C_SceneNonAmbientLightNode*)pAftkSceneNode;

	MFnNonAmbientLight mfnNonAmbientLight(dagPath);
	switch (mfnNonAmbientLight.decayRate())
	{
		case 0:
			pAftkSceneNonAmbientLightNode->SetDecayType(Aftk::E_LightDecay_None);
			break;
		case 1:
			pAftkSceneNonAmbientLightNode->SetDecayType(Aftk::E_LightDecay_Linear);
			break;
		case 2:
			pAftkSceneNonAmbientLightNode->SetDecayType(Aftk::E_LightDecay_Quadratic);
			break;
		case 3:
			pAftkSceneNonAmbientLightNode->SetDecayType(Aftk::E_LightDecay_Cubic);
			break;
		default:
			pAftkSceneNonAmbientLightNode->SetDecayType(Aftk::E_LightDecay_None);
			break;
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Non ambient light node " << mfnNonAmbientLight.name().asChar() << " id " << mfnNonAmbientLight.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

