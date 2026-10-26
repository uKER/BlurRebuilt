#include "stdafx.h"
#include <maya/MCommandResult.h>

using namespace MayaIo;

Aftk::C_Status C_LimitMixinImport::Import(
	C_Importer* pImport,
	const MDagPath &dagPath,
	Aftk::C_LimitMixin* pLimitMixin,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MFnDagNode fnDagNode(dagPath, &mStatus);
	MCHECK_ERROR_RETURN_1;

	MCommandResult result;
	MDoubleArray resDouble;
	MDoubleArray resInt;
	MString cmd;


	// Rotation limits

	cmd = "transformLimits -rx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMinX, (F32)Maths::DegToRad(resDouble[0]));
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMaxX, (F32)Maths::DegToRad(resDouble[1]));
	}

	cmd = "transformLimits -ry 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMinY, (F32)Maths::DegToRad(resDouble[0]));
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMaxY, (F32)Maths::DegToRad(resDouble[1]));
	}

	cmd = "transformLimits -rz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;

	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMinZ, (F32)Maths::DegToRad(resDouble[0]));
		pLimitMixin->SetLimit(Aftk::E_LimitType_RotateMaxZ, (F32)Maths::DegToRad(resDouble[1]));
	}

	cmd = "transformLimits -erx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;

	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMinX, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMaxX, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -ery 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMinY, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMaxY, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -erz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMinZ, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_RotateMaxZ, resInt[1] == 0 ? false : true);
	}


	// Translation limits

	cmd = "transformLimits -tx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMinX, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMaxX, (F32)resDouble[1]);
	}

	cmd = "transformLimits -ty 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMinY, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMaxY, (F32)resDouble[1]);
	}

	cmd = "transformLimits -tz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;

	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMinZ, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_TranslateMaxZ, (F32)resDouble[1]);
	}

	cmd = "transformLimits -etx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;

	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMinX, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMaxX, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -ety 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMinY, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMaxY, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -etz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMinZ, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_TranslateMaxZ, resInt[1] == 0 ? false : true);
	}

	// Scale limits

	cmd = "transformLimits -sx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMinX, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMaxX, (F32)resDouble[1]);
	}

	cmd = "transformLimits -sy 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMinY, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMaxY, (F32)resDouble[1]);
	}

	cmd = "transformLimits -sz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;

	mStatus = result.getResult(resDouble);
	MCHECK_ERROR_RETURN_1;
	if (resDouble.length() == 2) 
	{
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMinZ, (F32)resDouble[0]);
		pLimitMixin->SetLimit(Aftk::E_LimitType_ScaleMaxZ, (F32)resDouble[1]);
	}

	cmd = "transformLimits -esx 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;

	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMinX, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMaxX, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -esy 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMinY, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMaxY, resInt[1] == 0 ? false : true);
	}

	cmd = "transformLimits -esz 0 0 -q ";
	cmd += dagPath.fullPathName();
	mStatus = MGlobal::executeCommand(cmd, result, false, false);
	MCHECK_ERROR_RETURN_1;
	
	mStatus = result.getResult(resInt);
	MCHECK_ERROR_RETURN_1;
	if (resInt.length() == 2) 
	{
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMinZ, resInt[0] == 0 ? false : true);
		pLimitMixin->SetIsLimited(Aftk::E_LimitType_ScaleMaxZ, resInt[1] == 0 ? false : true);
	}

	return Aftk::C_Status::E_Status_Success;
}
