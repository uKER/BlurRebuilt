#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneLightExport::Export(
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
		AFTK_DO_BEGIN << "AftkSceneLightNodeImport cannot be created." AFTK_DO_ERROR_RETURN_END_1;
	}

	Aftk::C_SceneLightNode* pAftkSceneLightNode = (Aftk::C_SceneLightNode*)pAftkSceneNode;

	//Acquire function set from existing node
	MFnLight lightFn(mayaNewShapeObject, &mStatus);
	MCHECK_BEGIN << "Failed to acquire light node" MCHECK_ERROR_RETURN_END_1;
	
	//Set the light's intensity
	lightFn.setIntensity(pAftkSceneLightNode->GetIntensity());

	//Set the colour of this light
	const Aftk::Vector4F32* pAftkColour4 = &pAftkSceneLightNode->GetColour();
	MColor mayaColour((*pAftkColour4)[0], (*pAftkColour4)[1], (*pAftkColour4)[2], (*pAftkColour4)[3]);
	lightFn.setColor(mayaColour);

	status = C_SceneNodeExport::Export(
		false,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;

	return Aftk::C_Status::E_Status_Success;
}

