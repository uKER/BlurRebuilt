#include "stdafx.h"

using namespace MayaIo;


Aftk::C_Status C_DgNodeExport::Export(
	C_Exporter *pExporter,
	MFnDependencyNode& fnDependencyNode,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_Node* pAftkNode)
{
	MStatus mStatus;

	fnDependencyNode.setName(pAftkNode->GetName().c_str(), &mStatus);
//	if (!mStatus)
//		return Aftk::C_Status::E_Status_Failure;
//	else
		return Aftk::C_Status::E_Status_Success;
		
}