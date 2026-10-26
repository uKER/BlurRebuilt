#include "Stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_DgUnitConversionImport::Import(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	MFnDependencyNode& fnDependencyNode,
	Aftk::C_Node* pAftkNode)
{
	MStatus mStatus;
	Aftk::C_UnitConversionNode* pAftkUnitConversionNode = 
		(Aftk::C_UnitConversionNode*)pAftkNode;

	Aftk::C_Status status = C_DgNodeImport::Import(
		pImporter,
		pAftkScene,
		fnDependencyNode,
		pAftkUnitConversionNode);
	if (!status) return status;

	MPlug factorPlug = fnDependencyNode.findPlug("conversionFactor", &mStatus);
	MCHECK_ERROR_RETURN_1;

	F32 factor;
	mStatus = factorPlug.getValue(factor);
	MCHECK_ERROR_RETURN_1;
	pAftkUnitConversionNode->SetFactor(factor);

	MPlug inputPlug = fnDependencyNode.findPlug("input", &mStatus);
	MCHECK_ERROR_RETURN_1;

	Aftk::I_Object* pChainedObject = 
		C_ConnectedDgImport::ImportDirectlyConnectedPlug(
			pImporter,
			inputPlug,
			status);
		AFTK_CHECK_ERROR_RETURN;

	pAftkUnitConversionNode->GetInputPlug().SetSourceConnectionPtr(pChainedObject);

	return Aftk::C_Status::E_Status_Success;
}
