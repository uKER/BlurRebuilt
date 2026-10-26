#include "stdafx.h"

#include "Stdafx.h"
#include "MayaIoConnectedDgImport.h"

using namespace MayaIo;

Aftk::C_Status C_DgDistanceDimImport::Import(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	MFnDependencyNode& fnDependencyNode,
	Aftk::C_Node* pAftkNode)
{
	Aftk::C_DistanceDimNode* pAftkDistanceDimNode = 
		(Aftk::C_DistanceDimNode*)pAftkNode;

	Aftk::C_Status status = C_DgNodeImport::Import(
		pImporter,
		pAftkScene,
		fnDependencyNode,
		pAftkDistanceDimNode);
	if (!status) return status;

	MStatus mStatus;

	MPlug mInputs[2];
	mInputs[0] = fnDependencyNode.findPlug("startPoint", &mStatus);
	MCHECK_ERROR_RETURN_1;
	mInputs[1] = fnDependencyNode.findPlug("endPoint", &mStatus);
	MCHECK_ERROR_RETURN_1;

	//Loop thru inputs
	for (U32 i = 0; i < 2; i++)
	{
		Aftk::I_Object* pChainedObject = 
			C_ConnectedDgImport::ImportDirectlyConnectedPlug(
				pImporter,
				mInputs[i],
				status);
			AFTK_CHECK_ERROR_RETURN;

		pAftkDistanceDimNode->SetInput(i, pChainedObject);
	}

	return Aftk::C_Status::E_Status_Success;
}

