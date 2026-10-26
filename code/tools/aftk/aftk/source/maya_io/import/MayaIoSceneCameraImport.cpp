#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneCameraImport::Import(
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
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	// store location of new AFTK node
	Aftk::C_SceneCameraNode* pAftkSceneCameraNode = (Aftk::C_SceneCameraNode*)pAftkSceneNode;

	// Create the appropriate node type
	MFnDagNode mFnDagNode(dagPath, &mStatus);
	AFTK_CHECK_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL AFTK_CHECK_ERROR_RETURN_END_1;

	MFnCamera mFnCamera(dagPath, &mStatus);
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

	Aftk::C_SceneCameraNode* pAftkCamera = (Aftk::C_SceneCameraNode*)pAftkSceneNode;

	pAftkCamera->GetFocalLengthPlug().SetValue((F32)mFnCamera.focalLength());
	pAftkCamera->GetCentreOfInterestPlug().SetValue((F32)mFnCamera.centerOfInterest());
	pAftkCamera->GetHorizontalFilmAperturePlug().SetValue((F32)mFnCamera.horizontalFilmAperture()*Aftk::kInchesToMillimeters);
	pAftkCamera->GetVerticalFilmAperturePlug().SetValue((F32)mFnCamera.verticalFilmAperture()*Aftk::kInchesToMillimeters);
	pAftkCamera->GetShutterAnglePlug().SetValue((F32)mFnCamera.shutterAngle());
	pAftkCamera->GetNearClipPlanePlug().SetValue((F32)mFnCamera.nearClippingPlane());
	pAftkCamera->GetFarClipPlanePlug().SetValue((F32)mFnCamera.farClippingPlane());
	pAftkCamera->GetOrthographicWidthPlug().SetValue((F32)mFnCamera.orthoWidth());
	pAftkCamera->GetFocusDistancePlug().SetValue((F32)mFnCamera.focusDistance());
	pAftkCamera->GetFStopPlug().SetValue((F32)mFnCamera.fStop());
	pAftkCamera->SetIsOrthographic(mFnCamera.isOrtho());
	pAftkCamera->SetDepthOfFieldEnabled(mFnCamera.isDepthOfField());

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneCameraImport::FindPlug(
	Aftk::C_SceneCameraNode* pAftkCamera,
	const char* pPlugName)
{
	Aftk::I_Plug* pPlug = C_SceneNodeImport::FindPlug(
		pAftkCamera, 
		pPlugName);
	if (pPlug)
		return pPlug; 

	if (!strcmp(pPlugName, "focalLength"))
		return &pAftkCamera->GetFocalLengthPlug();

	if (!strcmp(pPlugName, "centerOfInterest"))
		return &pAftkCamera->GetCentreOfInterestPlug();

	if (!strcmp(pPlugName, "horizontalFilmAperture"))
		return &pAftkCamera->GetHorizontalFilmAperturePlug();

	if (!strcmp(pPlugName, "verticalFilmAperture"))
		return &pAftkCamera->GetVerticalFilmAperturePlug();

	if (!strcmp(pPlugName, "nearClipPlane"))
		return &pAftkCamera->GetNearClipPlanePlug();

	if (!strcmp(pPlugName, "farClipPlane"))
		return &pAftkCamera->GetFarClipPlanePlug();

	if (!strcmp(pPlugName, "shutterAngle"))
		return &pAftkCamera->GetShutterAnglePlug();

	if (!strcmp(pPlugName, "orthographicWidth"))
		return &pAftkCamera->GetOrthographicWidthPlug();

	if (!strcmp(pPlugName, "focusDistance"))
		return &pAftkCamera->GetFocusDistancePlug();

	if (!strcmp(pPlugName, "fStop"))
		return &pAftkCamera->GetFStopPlug();

	return 0;
}

Aftk::C_Status C_SceneCameraImport::PostProcess(	
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	const C_ImportOptions* pOptions)
{
	for (Aftk::C_SceneNodeIterator it(pAftkScene); it; ++it)
	{
		Aftk::C_SceneNode* pSceneNode = it;
		if (!pSceneNode->HasType(Aftk::E_ObjType_SceneCamera))
			continue;

		Aftk::C_SceneCameraNode* pCamera = (Aftk::C_SceneCameraNode*)pSceneNode;
		{
			Aftk::I_Object* pSource = 
				pCamera->GetHorizontalFilmAperturePlug().GetSourceConnectionPtr();
			if (pSource)
			{
				Aftk::C_UnitConversionNode* pUnitConv = new Aftk::C_UnitConversionNode(pAftkScene);
				pUnitConv->SetFactor(Aftk::kInchesToMillimeters);

				pUnitConv->GetInputPlug().SetSourceConnectionPtr(pSource);
				pCamera->GetHorizontalFilmAperturePlug().SetSourceConnectionPtr(pUnitConv);
			}
		}

		{
			Aftk::I_Object* pSource = 
				pCamera->GetVerticalFilmAperturePlug().GetSourceConnectionPtr();
			if (pSource)
			{
				Aftk::C_UnitConversionNode* pUnitConv = new Aftk::C_UnitConversionNode(pAftkScene);
				pUnitConv->SetFactor(Aftk::kInchesToMillimeters);

				pUnitConv->GetInputPlug().SetSourceConnectionPtr(pSource);
				pCamera->GetVerticalFilmAperturePlug().SetSourceConnectionPtr(pUnitConv);
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}
