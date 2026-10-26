#include "stdafx.h"


using namespace MayaIo;

Aftk::C_SceneNode* C_SceneCustomLightImport::CreateSceneNode(
	const MDagPath &dagPath,
	Aftk::C_Scene* pAftkScene)
{
	MStatus status;

	Aftk::C_SceneNode* pAftkSceneNode = 0;

	MDagPath shapeDagPath = dagPath;
	status = shapeDagPath.extendToShape();
	if (!status)
	{
		AFTK_DO_BEGIN << TRANS_EXTEND_TO_SHAPE_FAIL AFTK_DO_ERROR_END_1;
		return 0;
	}

	MFnDagNode customLight(shapeDagPath, &status);
	if (!status)
	{
		AFTK_DO_BEGIN << TRANS_ACQUIRE_DAG_NODE_FAIL AFTK_DO_ERROR_END_1;
		return 0;
	}

    MPlug lightTypePlug = customLight.findPlug("LightType", &status);
	int lightType;
	lightTypePlug.getValue(lightType);

	switch(lightType)
	{
	//Point
	case 0:
		pAftkSceneNode = new Aftk::C_ScenePointLightNode(pAftkScene);
		break;
	//Directional
	case 1:
		pAftkSceneNode = new Aftk::C_SceneDirectionalLightNode(pAftkScene);
		break;
	//Spot
	case 2:
		pAftkSceneNode = new Aftk::C_SceneSpotLightNode(pAftkScene);
		break;
	//Hemisphere
	case 3:
		pAftkSceneNode = new Aftk::C_SceneHemisphereLightNode(pAftkScene);
		break;
	default:
		return 0;
		break;
	};

	if (!pAftkSceneNode)
	{
		AFTK_DO_BEGIN << "Failed to allocate memory for new custom light." AFTK_DO_ERROR_END_1;
		return 0;
	}

	return pAftkSceneNode;
}

Aftk::C_Status C_SceneCustomLightImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	if (!pAftkSceneNode)
	{
		AFTK_DO_BEGIN << "Aftk::C_Scene node pointer is null" AFTK_DO_ERROR_RETURN_END_1;
	}

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

	MDagPath shapeDagPath = dagPath;
	mStatus = shapeDagPath.extendToShape();
	MCHECK_BEGIN << "Failed find light shape." MCHECK_ERROR_RETURN_END_1;

	MFnDagNode customLight(shapeDagPath, &mStatus);
	MCHECK_BEGIN << "Failed acquire dag node." MCHECK_ERROR_RETURN_END_1;

	Aftk::C_SceneLightNode* pAftkLight = (Aftk::C_SceneLightNode*)pAftkSceneNode;

	MPlug colourPlug = customLight.findPlug("Colour", &mStatus);
	MCHECK_BEGIN << "Failed find Colour plug." MCHECK_ERROR_RETURN_END_1;
	MPlug colourR = colourPlug.child(0, &mStatus);
	MCHECK_BEGIN << "Failed find ColourR plug." MCHECK_ERROR_RETURN_END_1;
	MPlug colourG = colourPlug.child(1, &mStatus);
	MCHECK_BEGIN << "Failed find ColourG plug." MCHECK_ERROR_RETURN_END_1;
	MPlug colourB = colourPlug.child(2, &mStatus);
	MCHECK_BEGIN << "Failed find ColourB plug." MCHECK_ERROR_RETURN_END_1;

	Aftk::Vector4F32 lightColour;
	colourR.getValue(lightColour[0]);
	colourG.getValue(lightColour[1]);
	colourB.getValue(lightColour[2]);
	lightColour[3] = 0;

	pAftkLight->SetColour(lightColour);

	MPlug lightIntensityPlug = customLight.findPlug("Intensity", &mStatus);
	MCHECK_BEGIN << "Failed find Intensity plug." MCHECK_ERROR_RETURN_END_1;

	F32 lightIntensity;
	lightIntensityPlug.getValue(lightIntensity);

	pAftkLight->SetIntensity(lightIntensity);

	switch (pAftkSceneNode->GetType())
	{
		case Aftk::E_ObjType_ScenePointLight:
		{
			//No more proporties needed.
			Aftk::C_ScenePointLightNode* pAftkPointLight = (Aftk::C_ScenePointLightNode*)pAftkSceneNode;

			MPlug lightRadiusPlug = customLight.findPlug("Radius", &mStatus);
			MCHECK_BEGIN << "Failed find Radius plug." MCHECK_ERROR_RETURN_END_1;
			F32 lightRadius;
			lightRadiusPlug.getValue(lightRadius);

			pAftkPointLight->SetCutOffRadius(lightRadius);
			pAftkPointLight->SetDecayType(Aftk::E_LightDecay_Quadratic);
		}
		break;

		case Aftk::E_ObjType_SceneDirectionalLight:
		{
			Aftk::C_SceneDirectionalLightNode* pAftkDirectionalLight = (Aftk::C_SceneDirectionalLightNode*)pAftkSceneNode;
			pAftkDirectionalLight->SetDecayType(Aftk::E_LightDecay_Quadratic);
		}
		break;

		case Aftk::E_ObjType_SceneSpotLight:
		{
			Aftk::C_SceneSpotLightNode* pAftkSpotLight = (Aftk::C_SceneSpotLightNode*)pAftkSceneNode;

			MPlug lightPenumbraPlug = customLight.findPlug("Penumbra", &mStatus);
			AFTK_CHECK_BEGIN << "Failed find Penumbra plug." AFTK_CHECK_ERROR_RETURN_END_1;
			F32 lightPenumbra;
			lightPenumbraPlug.getValue(lightPenumbra);
			F32 coneAngle = lightPenumbra*2;
			pAftkSpotLight->SetConeAngle(Maths::DegToRad(coneAngle));

			MPlug lightUmbraPlug = customLight.findPlug("Umbra", &mStatus);
			AFTK_CHECK_BEGIN << "Failed find Umbra plug." AFTK_CHECK_ERROR_RETURN_END_1;
			F32 lightUmbra;
			lightUmbraPlug.getValue(lightUmbra);
			F32 penumbra = lightPenumbra - lightUmbra;
			pAftkSpotLight->SetPenumbraAngle(Maths::DegToRad(penumbra));

			MPlug lightRadiusPlug = customLight.findPlug("Radius", &mStatus);
			AFTK_CHECK_BEGIN << "Failed find Radius plug." AFTK_CHECK_ERROR_RETURN_END_1;
			F32 lightRadius;
			lightRadiusPlug.getValue(lightRadius);

            pAftkSpotLight->SetCutOffRadius(lightRadius);
			pAftkSpotLight->SetDecayType(Aftk::E_LightDecay_Quadratic);
		}
		break;

		case Aftk::E_ObjType_SceneHemisphereLight:
		{
			Aftk::C_SceneHemisphereLightNode* pAftkHemisphereLight = (Aftk::C_SceneHemisphereLightNode*)pAftkSceneNode;
		}
		break;

		default:
		AFTK_ASSERT(false);
		break;
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Custom Light C_Node " << customLight.name().asChar() << " id " << customLight.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

