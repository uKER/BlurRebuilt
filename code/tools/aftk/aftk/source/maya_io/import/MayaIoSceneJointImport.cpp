#include "stdafx.h"
#include "../MayaIoMatrixHelper.h"

#include <maya/MCommandResult.h>

using namespace MayaIo;

Aftk::C_Status C_SceneJointImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	//Import the base AFTK scene node, this includes the bind pose.
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneJointNode* pAftkSceneJointNode = (Aftk::C_SceneJointNode*)pAftkSceneNode;

	status = C_LimitMixinImport::Import(
		pImporter,
		dagPath,
		(Aftk::C_LimitMixin*)pAftkSceneJointNode,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	MFnIkJoint fnJoint(dagPath, &mStatus);
	if (!mStatus)
	{
		if (pOptions->strict)
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_FAIL AFTK_DO_ERROR_RETURN_END_0;
		}
		else
		{
			AFTK_DO_BEGIN << TRANS_ACQUIRE_FN_WARN AFTK_DO_WARNING_END;
			pImporter->AddDebris(pAftkSceneNode);
			return Aftk::C_Status::E_Status_Success;
		}
	}
#ifndef AFTK_LEAN_AND_MEAN
	double tempVec3[3];
	fnJoint.getStiffness(tempVec3);

	pAftkSceneJointNode->SetStiffness(
		Aftk::Vector3F32((F32)(tempVec3[0]), (F32)(tempVec3[1]), (F32)(tempVec3[2])));

	{
		MQuaternion q;
		fnJoint.getOrientation(q);
		Aftk::Matrix44 orientation;
		if (!MatrixHelper::MayaToAftkMatrix44(orientation, q.asMatrix(), pOptions->absNumericRange))
			AFTK_DO_BEGIN << "Failed to import matrix." AFTK_DO_ERROR_RETURN_END_1;
		pAftkSceneJointNode->SetOrientation(orientation);
	}
	{
		MQuaternion q;
		fnJoint.getScaleOrientation(q);
		Aftk::Matrix44 orientation;
		if (!MatrixHelper::MayaToAftkMatrix44(orientation, q.asMatrix(), pOptions->absNumericRange))
			AFTK_DO_BEGIN << "Failed to import matrix." AFTK_DO_ERROR_RETURN_END_1;
		pAftkSceneJointNode->SetScaleOrientation(orientation);
	}
	fnJoint.getPreferedAngle(tempVec3);
    pAftkSceneJointNode->SetPreferredAngle(
		Aftk::Vector3F32(
		(F32)(tempVec3[0]), 
		(F32)(tempVec3[1]), 
		(F32)(tempVec3[2])));

	Aftk::Vector3Bool dof;
	fnJoint.getDegreesOfFreedom(dof[0], dof[1], dof[2]);
	pAftkSceneJointNode->SetDegreesOfFreedom(dof);

	//Query radius attribute.
	{
		MPlug plug = fnJoint.findPlug("radius", &mStatus);
		MCHECK_ERROR_RETURN_1;

		F32 result;
		mStatus = plug.getValue(result);
		MCHECK_ERROR_RETURN_1;

		pAftkSceneJointNode->SetRadius(result);
	}

	pAftkSceneJointNode->SetMinRotateDampRange(
		Aftk::Vector3F32(
		Maths::DegToRad((F32)fnJoint.minRotateDampXRange()), 
		Maths::DegToRad((F32)fnJoint.minRotateDampYRange()), 
		Maths::DegToRad((F32)fnJoint.minRotateDampZRange())));

	pAftkSceneJointNode->SetMaxRotateDampRange(
		Aftk::Vector3F32(
		Maths::DegToRad((F32)fnJoint.maxRotateDampXRange()), 
		Maths::DegToRad((F32)fnJoint.maxRotateDampYRange()), 
		Maths::DegToRad((F32)fnJoint.maxRotateDampZRange())));

	pAftkSceneJointNode->SetMinRotateDampStrength(
		Aftk::Vector3F32(
		Maths::DegToRad((F32)fnJoint.minRotateDampXStrength()), 
		Maths::DegToRad((F32)fnJoint.minRotateDampYStrength()), 
		Maths::DegToRad((F32)fnJoint.minRotateDampZStrength())));

	pAftkSceneJointNode->SetMaxRotateDampStrength(
		Aftk::Vector3F32(
		Maths::DegToRad((F32)fnJoint.maxRotateDampXStrength()), 
		Maths::DegToRad((F32)fnJoint.maxRotateDampYStrength()),
		Maths::DegToRad((F32)fnJoint.maxRotateDampZStrength())));

#endif

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Joint C_Node " << fnJoint.name().asChar() << " id " << fnJoint.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneJointImport::GetSSC(
	const MDagPath &dagPath,
	bool& ssc)
{
	MStatus mStatus;
	Aftk::C_Status status;

	MFnIkJoint fnJoint(dagPath, &mStatus);
	MCHECK_ERROR_RETURN_1

	MPlug plug = fnJoint.findPlug("segmentScaleCompensate", &mStatus);
	MCHECK_ERROR_RETURN_1;
	mStatus = plug.getValue(ssc);
	MCHECK_ERROR_RETURN_1;

	return Aftk::C_Status::E_Status_Success;
}

void C_SceneJointImport::SetSSC(
	Aftk::C_SceneNode* pAftkSceneNode,
	bool ssc)
{
	Aftk::C_SceneJointNode* pAftkSceneJointNode = 
		(Aftk::C_SceneJointNode*)pAftkSceneNode;
	
	pAftkSceneJointNode->SetSegmentScaleCompensation(ssc);
}


