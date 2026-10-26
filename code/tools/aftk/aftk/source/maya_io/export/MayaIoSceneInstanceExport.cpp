#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneInstanceExport::Export(
	bool create,
	C_Exporter* pExporter,
	MObject& parentTransformObject,
	MObject& mayaNewTransformObject,
	MObject& mayaNewShapeObject,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;

	//We must create this node
	AFTK_ASSERT(create);

	status = C_SceneNodeExport::Export(
		create,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;

	return Aftk::C_Status::E_Status_Success;
}

