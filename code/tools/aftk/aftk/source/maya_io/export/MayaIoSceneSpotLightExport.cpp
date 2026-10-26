#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneSpotLightExport::Export(
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

	//We must create this node
	AFTK_ASSERT(create);

	//Create directional light function set.
	MFnSpotLight spotLightFn;
	mayaNewTransformObject = spotLightFn.create(parentTransformObject, true, &mStatus);
	MCHECK_BEGIN << "Failed to create spot light node" MCHECK_ERROR_RETURN_END_1;
	
	//Get shape object.
	{
		MDagPath path;
		mStatus = spotLightFn.getPath(path);
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
		mStatus = path.extendToShape();
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
		mayaNewShapeObject = path.node(&mStatus);
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
	}

	Aftk::C_SceneSpotLightNode* pAftkSceneSpotLightNode = 
		(Aftk::C_SceneSpotLightNode*)pAftkSceneNode;

	//Copy across spot light proporties
	spotLightFn.setConeAngle(pAftkSceneSpotLightNode->GetConeAngle());
	spotLightFn.setDropOff(pAftkSceneSpotLightNode->GetDropOff());
	spotLightFn.setPenumbraAngle(pAftkSceneSpotLightNode->GetPenumbraAngle());

	return Aftk::C_Status::E_Status_Success;
}
