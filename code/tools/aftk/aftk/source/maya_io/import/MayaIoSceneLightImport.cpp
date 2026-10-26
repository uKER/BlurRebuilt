#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneLightImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Import the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneLightNode* pAftkSceneLightNode = (Aftk::C_SceneLightNode*)pAftkSceneNode;

	MFnLight mayaLight(dagPath, &mStatus);
	if (!mStatus)
	{
		if (pOptions->strict)
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

	F32 intensity = mayaLight.intensity();
	pAftkSceneLightNode->SetIntensity(intensity);

	MColor mayaColor = mayaLight.color();
	Aftk::Vector4F32 aftkColour(
		mayaColor.r,
		mayaColor.g,
		mayaColor.b,
		mayaColor.a);

	pAftkSceneLightNode->SetColour(aftkColour);

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Light C_Node " << mayaLight.name().asChar() << " id " << mayaLight.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

