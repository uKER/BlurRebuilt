#include "Stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_DgNodeImport::Import(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	MFnDependencyNode& fnDependencyNode,
	Aftk::C_Node* pAftkNode)
{
	pAftkNode->SetName(fnDependencyNode.name().asChar());

	return Aftk::C_Status::E_Status_Success;
}
