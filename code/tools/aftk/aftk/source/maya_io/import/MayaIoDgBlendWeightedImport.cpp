#include "Stdafx.h"
#include "MayaIoConnectedDgImport.h"

using namespace MayaIo;

Aftk::C_Status C_DgBlendWeightedImport::Import(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	MFnDependencyNode& fnDependencyNode,
	Aftk::C_Node* pAftkNode)
{
	Aftk::C_BlendWeightedNode* pAftkBlendWeightedNode = 
		(Aftk::C_BlendWeightedNode*)pAftkNode;

	Aftk::C_Status status = C_DgNodeImport::Import(
		pImporter,
		pAftkScene,
		fnDependencyNode,
		pAftkBlendWeightedNode);
	if (!status) return status;

	MStatus mStatus;

	MPlug blendWeightedInputPlug = fnDependencyNode.findPlug("input", &mStatus);
	MCHECK_ERROR_RETURN_1;
	MPlug blendWeightedWeightPlug = fnDependencyNode.findPlug("weight", &mStatus);
	MCHECK_ERROR_RETURN_1;

	//Loop thru inputs
	AFTK_ASSERT(blendWeightedInputPlug.isArray());
	AFTK_ASSERT(blendWeightedWeightPlug.isArray());

	Ut::Vector<Ut::Pair<Aftk::I_Object*, F32> > inputs;
	for (U32 i = 0; i < blendWeightedInputPlug.numConnectedElements(); i++)
	{
		MPlug connectedElement = 
			blendWeightedInputPlug.connectionByPhysicalIndex(i, &mStatus);
		MCHECK_ERROR_RETURN_1;

		Aftk::I_Object* pChainedObject = 
			C_ConnectedDgImport::ImportDirectlyConnectedPlug(
				pImporter,
				connectedElement,
				status);
			AFTK_CHECK_ERROR_RETURN;

		MPlug weightPlug = 
			blendWeightedWeightPlug.elementByPhysicalIndex(i, &mStatus);
		F32 weight(1.f);
		if (mStatus)
		{
			mStatus = weightPlug.getValue(weight);
			MCHECK_ERROR_RETURN_1;
		}
		inputs.PushBack(Ut::Pair<Aftk::I_Object*, F32>(pChainedObject, weight));
	}

	pAftkBlendWeightedNode->SetNumInputs(inputs.Size());
	for (U32 i = 0; i < (U32)inputs.Size(); i++)
	{
		pAftkBlendWeightedNode->GetInputPlug(i).SetSourceConnectionPtr(inputs[i].first);
		pAftkBlendWeightedNode->GetWeightPlug(i).SetValue(inputs[i].second);
	}

	return Aftk::C_Status::E_Status_Success;
}
