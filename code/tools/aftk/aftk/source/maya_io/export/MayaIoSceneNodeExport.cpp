#include "stdafx.h"



using namespace MayaIo;

Aftk::C_Status C_SceneNodeExport::Export(
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

	AFTK_ASSERT(pAftkSceneNode);

	MFnTransform fnTransform;
	if (create)
	{
		mayaNewTransformObject = fnTransform.create(parentTransformObject, &mStatus);
		MCHECK_BEGIN << "Could not create transform node" MCHECK_ERROR_RETURN_END_1;
	}
	else
	{
		mStatus = fnTransform.setObject(mayaNewTransformObject);
		if (!mStatus)
		{
			int x = 5;
		}
		MCHECK_BEGIN MCHECK_ERROR_RETURN_END_1;
	}

	fnTransform.setName(MString(pAftkSceneNode->GetName().c_str()));

	//Copy transform matrix
	{
		//create a temporary matrix storing our data
		Aftk::Matrix44 aftkSceneNodeMatrix = pAftkSceneNode->GetLocalMatrix();

		F32 tempMatrix[4][4];
		tempMatrix[0][0] = aftkSceneNodeMatrix[0][0]; tempMatrix[0][1] = aftkSceneNodeMatrix[0][1]; tempMatrix[0][2] = aftkSceneNodeMatrix[0][2]; tempMatrix[0][3] = aftkSceneNodeMatrix[0][3];
		tempMatrix[1][0] = aftkSceneNodeMatrix[1][0]; tempMatrix[1][1] = aftkSceneNodeMatrix[1][1]; tempMatrix[1][2] = aftkSceneNodeMatrix[1][2]; tempMatrix[1][3] = aftkSceneNodeMatrix[1][3];
		tempMatrix[2][0] = aftkSceneNodeMatrix[2][0]; tempMatrix[2][1] = aftkSceneNodeMatrix[2][1]; tempMatrix[2][2] = aftkSceneNodeMatrix[2][2]; tempMatrix[2][3] = aftkSceneNodeMatrix[2][3];
		tempMatrix[3][0] = aftkSceneNodeMatrix[3][0]; tempMatrix[3][1] = aftkSceneNodeMatrix[3][1]; tempMatrix[3][2] = aftkSceneNodeMatrix[3][2]; tempMatrix[3][3] = aftkSceneNodeMatrix[3][3];

		//initialise an MMatrix from the raw data
		MMatrix mayaMatrix(tempMatrix);

		//initialise a MTransformationMatrix from our MMatrix
		MTransformationMatrix mayaTransformationMatrix(mayaMatrix);

		//initialise the MFnTransform from our new new object with the 
		//contents of our MTransformationMatrix.
		mStatus = fnTransform.set(mayaTransformationMatrix);
		MCHECK_BEGIN << "Could not set transform matrix" MCHECK_ERROR_RETURN_END_1
	}

	MDagPath mayaTransformDagPath;
	mStatus = fnTransform.getPath(mayaTransformDagPath);
	MCHECK_BEGIN << TRANS_ACQUIRE_DAG_PATH_FAIL MCHECK_ERROR_RETURN_END_1

	//Add our new transform to the importer's aftk<->maya mapping tables
	pExporter->AftkMayaMappingPtr()->SetAftkMayaSceneNodeMapping(pAftkSceneNode, mayaTransformDagPath);
	pExporter->AftkMayaMappingPtr()->SetMayaAftkSceneNodeMapping(mayaTransformDagPath, pAftkSceneNode);

	return Aftk::C_Status::E_Status_Success;
}

