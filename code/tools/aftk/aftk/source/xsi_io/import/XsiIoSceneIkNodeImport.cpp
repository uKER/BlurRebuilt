#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoSceneIkNodeImport.h"

#include <Aftk/AftkEnum.h>

using namespace XsiIo;


Aftk::C_Status C_SceneIkNodeImport::Import(
	C_SceneImport* pSceneImport,
	CSLModel* pXsiNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	status = C_SceneNodeImport::Import(
		pSceneImport,
		pXsiNode,
		pAftkSceneNode);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	Aftk::C_SceneJointNode* pAftkSceneJointNode = (Aftk::C_SceneJointNode*)pAftkSceneNode;

	pAftkSceneJointNode->SetSegmentScaleCompensation(false);
#ifndef AFTK_LEAN_AND_MEAN
	pAftkSceneJointNode->SetRadius(0.5f);
	pAftkSceneJointNode->SetDegreesOfFreedom(Aftk::Vector3Bool(true,true,true));

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMinX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMinY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMinZ, false);

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMaxX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMaxY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_TranslateMaxZ, false);

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMinX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMinY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMinZ, false);

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxZ, false);

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMinX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMinY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMinZ, false);

	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMaxX, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMaxY, false);
	pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_ScaleMaxZ, false);

	CSLTemplate::ETemplateType type = pXsiNode->GetPrimitiveType();
	
	if (type == CSLTemplate::SI_IK_JOINT)
	{
		CSLIKJoint *pXsiJoint = (CSLIKJoint *)pXsiNode->Primitive();
		if (!pXsiJoint)
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to get XSI joint primitive");

		CSIBCVector3D tmp;
		SI_Bool active;
		
		tmp = pXsiJoint->GetPreferredRotation();
		pAftkSceneJointNode->SetPreferredAngle(Aftk::Vector3F32(tmp.GetX(), tmp.GetY(), tmp.GetZ()));

		F32 stiffness = pXsiJoint->GetStiffness();
		pAftkSceneJointNode->SetStiffness(Aftk::Vector3F32(stiffness, stiffness, stiffness));

		active = pXsiJoint->GetRotationLimitActivation();
		pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxX, (active==0) ? false: true );
		pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxZ, (active==0) ? false: true );
		pAftkSceneJointNode->SetIsLimited(Aftk::E_LimitType_RotateMaxY, (active==0) ? false: true );

		tmp = pXsiJoint->GetRotationLimitMinimum();
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMinX, Maths::DegToRad(tmp.GetX()));
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMinY, Maths::DegToRad(tmp.GetY()));
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMinZ, Maths::DegToRad(tmp.GetZ()));

		tmp = pXsiJoint->GetRotationLimitMaximum();
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMaxX, Maths::DegToRad(tmp.GetX()));
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMaxY, Maths::DegToRad(tmp.GetY()));
		pAftkSceneJointNode->SetLimit(Aftk::E_LimitType_RotateMaxZ, Maths::DegToRad(tmp.GetZ()));
	}

#endif
	return Aftk::C_Status::E_Status_Success;
}




