#include "stdafx.h"



using namespace MayaIo;

Aftk::C_Status C_SceneNonAmbientLightExport::Export(
	bool create,
	C_Exporter* pExporter,
	MObject& parentTransformObject,
	MObject& mayaNewTransformObject,
	MObject& mayaNewShapeObject,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	if (create)
	{
		AFTK_ASSERT(false);
		//YOU CANNOT CREATE INSTANCES OF THIS TYPE
		AFTK_DO_BEGIN << "Aftk::C_SceneNonAmbientLightNode cannot be created." AFTK_DO_ERROR_RETURN_END_1;
	}

	//Import base-class proporties.
	status = C_SceneLightExport::Export(
		false,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;
	Aftk::C_SceneNonAmbientLightNode* pAftkSceneNonAmbientLightNode = 
		(Aftk::C_SceneNonAmbientLightNode*)pAftkSceneNode;

	//Acquire function set from existing node
	MFnNonAmbientLight mayaNonAmbientLightNode(mayaNewShapeObject, &mStatus);
	MCHECK_BEGIN << "Failed to acquire non ambient light node" MCHECK_ERROR_RETURN_END_1

	//Set the decay type of this light
	switch (pAftkSceneNonAmbientLightNode->GetDecayType())
	{
		case Aftk::E_LightDecay_None:
			mayaNonAmbientLightNode.setDecayRate(0);		
			break;
		case Aftk::E_LightDecay_Linear:
			mayaNonAmbientLightNode.setDecayRate(1);
			break;
		case Aftk::E_LightDecay_Quadratic:
			mayaNonAmbientLightNode.setDecayRate(2);
			break;
		case Aftk::E_LightDecay_Cubic:
			mayaNonAmbientLightNode.setDecayRate(3);
			break;
		default:
			AFTK_DO_BEGIN << "Unknown light decay type encountered" AFTK_DO_WARNING_END;
			mayaNonAmbientLightNode.setDecayRate(0);
			break;
	}

	return Aftk::C_Status::E_Status_Success;
}

