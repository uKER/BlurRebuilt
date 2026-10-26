#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status SceneSpotLightImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pAftkImportOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Importing spot light.\n" AFTK_DO_DEBUG_END;
	#endif
	//Import the base AFTK non ambient light node
	status = C_SceneNonAmbientLightImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		pAftkImportOptions);
	AFTK_CHECK_ERROR_RETURN;
		
	Aftk::C_SceneSpotLightNode* pAftkSceneSpotLightNode = 
		(Aftk::C_SceneSpotLightNode*)pAftkSceneNode;

	MFnSpotLight mfnSpotLight(dagPath, &mStatus);
	if (!mStatus)
	{
		if (pAftkImportOptions->strict)
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_FAIL AFTK_DO_ERROR_RETURN_END_0;
		}
		else
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_WARN AFTK_DO_WARNING_END;
			pImporter->AddDebris(pAftkSceneNode);
			return Aftk::C_Status::E_Status_Success;
		}
	}

	pAftkSceneSpotLightNode->SetConeAngle((F32)mfnSpotLight.coneAngle());
	pAftkSceneSpotLightNode->SetPenumbraAngle((F32)mfnSpotLight.penumbraAngle());
	pAftkSceneSpotLightNode->SetDropOff((F32)mfnSpotLight.dropOff());

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Spot Light C_Node " << mfnSpotLight.name().asChar() << " id " << mfnSpotLight.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

