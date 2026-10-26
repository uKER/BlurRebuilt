#include "stdafx.h"

#include "../MayaIoIGameImport.h"
#include "MayaIoShitstormGameImport.h"
#include "../../../MayaIoPolyFlagsHelper.h"

using namespace MayaIo;

C_ShitstormGameImport::~C_ShitstormGameImport()
{
}

Aftk::C_Status C_ShitstormGameImport::PreProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
//	PolyFlagsHelper::Validate("z:\\tools\\art_tools\\config\\PolyFlags_shooter.txt");
	
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_ShitstormGameImport::PostProcessScene(
	C_Importer* pImporter,
	const C_ImportOptions* pOptions)
{
	return Aftk::C_Status::E_Status_Success;
}


bool C_ShitstormGameImport::PreserveChildJointTransforms(
	C_Importer* pImporter,
	const MDagPath& path) const
{
	return true;
}

