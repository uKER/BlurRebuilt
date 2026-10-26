#include "Stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneDirectionalLightExport::Export(
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
	MFnDirectionalLight directionalLightFn;
	mayaNewTransformObject = directionalLightFn.create(parentTransformObject, true, &mStatus);
	MCHECK_BEGIN << "Failed to create directional light node" MCHECK_ERROR_RETURN_END_1;

	//Get shape object.
	{
		MDagPath path;
		mStatus = directionalLightFn.getPath(path);
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
		mStatus = path.extendToShape();
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
		mayaNewShapeObject = path.node(&mStatus);
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
	}

	//Import base-class proporties.
	status = C_SceneNonAmbientLightExport::Export(
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

