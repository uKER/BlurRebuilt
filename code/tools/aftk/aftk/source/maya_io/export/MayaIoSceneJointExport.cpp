#include "stdafx.h"

#include "../MayaIoMatrixHelper.h"

using namespace MayaIo;

Aftk::C_Status C_SceneJointExport::Export(
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

	//We must create this node
	AFTK_ASSERT(create);

	MFnIkJoint fnJoint;
	mayaNewTransformObject = fnJoint.create(parentTransformObject, &mStatus);
	MCHECK_BEGIN << "Failed to create joint node" MCHECK_ERROR_RETURN_END_1;

	if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneJoint)
	{
		const Aftk::C_SceneJointNode *pAftkSceneJointNode = (const Aftk::C_SceneJointNode*)pAftkSceneNode;

#ifndef AFTK_LEAN_AND_MEAN
		{
			double tempVec3[3];
			const Aftk::Vector3F32& stiff = pAftkSceneJointNode->GetStiffness();
			tempVec3[0] = stiff.x;
			tempVec3[1] = stiff.y;
			tempVec3[2] = stiff.z;
			fnJoint.setStiffness(tempVec3);
		}
		{
			MQuaternion q;
			Aftk::Matrix44 orientation = pAftkSceneJointNode->GetOrientation();
			MatrixHelper::AftkToMayaMatrix44(orientation, (MMatrix)q);
			fnJoint.setOrientation(q);
		}
		{
			MQuaternion q;
			Aftk::Matrix44 scaleOrientation = pAftkSceneJointNode->GetOrientation();
			MatrixHelper::AftkToMayaMatrix44(scaleOrientation, (MMatrix)q);
			fnJoint.setScaleOrientation(q);
		}
		{
			double tempVec3[3];
			const Aftk::Vector3F32& pref = pAftkSceneJointNode->GetPreferredAngle();
			tempVec3[0] = Maths::RadToDeg(pref.x);
			tempVec3[1] = Maths::RadToDeg(pref.y);
			tempVec3[2] = Maths::RadToDeg(pref.z);
			fnJoint.setPreferedAngle(tempVec3);
		}
		{
			const Aftk::Vector3Bool dof = pAftkSceneJointNode->GetDegreesOfFreedom();
			fnJoint.setDegreesOfFreedom(dof.x, dof.y, dof.z);
		}
		{
			//joint radius
			MPlug radiusPlug = fnJoint.findPlug("radius", &mStatus);
			if (status)
				radiusPlug.setValue(pAftkSceneJointNode->GetRadius());
		}
		{
			const Aftk::Vector3F32 minrotatedamprange = pAftkSceneJointNode->GetMinRotateDampRange();
			fnJoint.setMinRotateDampXRange(Maths::RadToDeg(minrotatedamprange.x));
			fnJoint.setMinRotateDampYRange(Maths::RadToDeg(minrotatedamprange.y));
			fnJoint.setMinRotateDampZRange(Maths::RadToDeg(minrotatedamprange.z));
		}
		{
			const Aftk::Vector3F32 maxrotatedamprange = pAftkSceneJointNode->GetMaxRotateDampRange();
			fnJoint.setMaxRotateDampXRange(Maths::RadToDeg(maxrotatedamprange.x));
			fnJoint.setMaxRotateDampXRange(Maths::RadToDeg(maxrotatedamprange.y));
			fnJoint.setMaxRotateDampXRange(Maths::RadToDeg(maxrotatedamprange.z));
		}
		{
			const Aftk::Vector3F32 minrotatedampstrength = pAftkSceneJointNode->GetMinRotateDampStrength();
			fnJoint.setMinRotateDampXStrength(minrotatedampstrength.x);
			fnJoint.setMinRotateDampYStrength(minrotatedampstrength.y);
			fnJoint.setMinRotateDampZStrength(minrotatedampstrength.z);
		}
		{
			const Aftk::Vector3F32 maxrotatedampstrength = pAftkSceneJointNode->GetMaxRotateDampStrength();
			fnJoint.setMaxRotateDampXStrength(maxrotatedampstrength.x);
			fnJoint.setMaxRotateDampYStrength(maxrotatedampstrength.y);
			fnJoint.setMaxRotateDampZStrength(maxrotatedampstrength.z);
		}

		fnJoint.setLimit(MFnIkJoint::kTranslateMinX, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMinX));
		fnJoint.setLimit(MFnIkJoint::kTranslateMinY, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMinY));
		fnJoint.setLimit(MFnIkJoint::kTranslateMinZ, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMinZ));

		fnJoint.setLimit(MFnIkJoint::kTranslateMaxX, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMaxX));
		fnJoint.setLimit(MFnIkJoint::kTranslateMaxY, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMaxY));
		fnJoint.setLimit(MFnIkJoint::kTranslateMaxZ, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_TranslateMaxZ));

		fnJoint.setLimit(MFnIkJoint::kScaleMinX, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMinX));
		fnJoint.setLimit(MFnIkJoint::kScaleMinY, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMinY));
		fnJoint.setLimit(MFnIkJoint::kScaleMinZ, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMinZ));

		fnJoint.setLimit(MFnIkJoint::kScaleMaxX, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMaxX));
		fnJoint.setLimit(MFnIkJoint::kScaleMaxY, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMaxY));
		fnJoint.setLimit(MFnIkJoint::kScaleMaxZ, pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_ScaleMaxZ));

		fnJoint.setLimit(MFnIkJoint::kRotateMinX, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMinX)));
		fnJoint.setLimit(MFnIkJoint::kRotateMinY, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMinY)));
		fnJoint.setLimit(MFnIkJoint::kRotateMinZ, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMinZ)));

		fnJoint.setLimit(MFnIkJoint::kRotateMaxX, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMaxX)));
		fnJoint.setLimit(MFnIkJoint::kRotateMaxY, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMaxY)));
		fnJoint.setLimit(MFnIkJoint::kRotateMaxZ, Maths::RadToDeg(pAftkSceneJointNode->GetLimit(Aftk::E_LimitType_RotateMaxZ)));

		fnJoint.enableLimit(MFnIkJoint::kTranslateMinX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMinX));
		fnJoint.enableLimit(MFnIkJoint::kTranslateMinY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMinY));
		fnJoint.enableLimit(MFnIkJoint::kTranslateMinZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMinZ));

		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMaxX));
		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMaxY));
		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_TranslateMaxZ));

		fnJoint.enableLimit(MFnIkJoint::kScaleMinX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMinX));
		fnJoint.enableLimit(MFnIkJoint::kScaleMinY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMinY));
		fnJoint.enableLimit(MFnIkJoint::kScaleMinZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMinZ));

		fnJoint.enableLimit(MFnIkJoint::kScaleMaxX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMaxX));
		fnJoint.enableLimit(MFnIkJoint::kScaleMaxY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMaxY));
		fnJoint.enableLimit(MFnIkJoint::kScaleMaxZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_ScaleMaxZ));

		fnJoint.enableLimit(MFnIkJoint::kRotateMinX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMinX));
		fnJoint.enableLimit(MFnIkJoint::kRotateMinY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMinY));
		fnJoint.enableLimit(MFnIkJoint::kRotateMinZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMinZ));

		fnJoint.enableLimit(MFnIkJoint::kRotateMaxX, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMaxX));
		fnJoint.enableLimit(MFnIkJoint::kRotateMaxY, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMaxY));
		fnJoint.enableLimit(MFnIkJoint::kRotateMaxZ, pAftkSceneJointNode->IsLimited(Aftk::E_LimitType_RotateMaxZ));
#endif

	}
	else
	{
		fnJoint.enableLimit(MFnIkJoint::kTranslateMinX, false);
		fnJoint.enableLimit(MFnIkJoint::kTranslateMinY, false);
		fnJoint.enableLimit(MFnIkJoint::kTranslateMinZ, false);

		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxX, false);
		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxY, false);
		fnJoint.enableLimit(MFnIkJoint::kTranslateMaxZ, false);

		fnJoint.enableLimit(MFnIkJoint::kScaleMinX, false);
		fnJoint.enableLimit(MFnIkJoint::kScaleMinY, false);
		fnJoint.enableLimit(MFnIkJoint::kScaleMinZ, false);

		fnJoint.enableLimit(MFnIkJoint::kScaleMaxX, false);
		fnJoint.enableLimit(MFnIkJoint::kScaleMaxY, false);
		fnJoint.enableLimit(MFnIkJoint::kScaleMaxZ, false);

		fnJoint.enableLimit(MFnIkJoint::kRotateMinX, false);
		fnJoint.enableLimit(MFnIkJoint::kRotateMinY, false);
		fnJoint.enableLimit(MFnIkJoint::kRotateMinZ, false);

		fnJoint.enableLimit(MFnIkJoint::kRotateMaxX, false);
		fnJoint.enableLimit(MFnIkJoint::kRotateMaxY, false);
		fnJoint.enableLimit(MFnIkJoint::kRotateMaxZ, false);
	}

	status = C_SceneNodeExport::Export(
		false,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;

	//connect scale plugs for ssc
	//if (parentTransformObject.hasFn(MFn::kJoint))
	if (parentTransformObject.hasFn(MFn::kDependencyNode))
	{
		MFnDependencyNode fnParentJoint(parentTransformObject, &mStatus);
		MCHECK_ERROR_RETURN_1;

		MPlug sourcePlug = fnParentJoint.findPlug("scale", &mStatus);
		MCHECK_ERROR_RETURN_1;

		MPlug destinationPlug = fnJoint.findPlug("inverseScale", &mStatus);
		MCHECK_ERROR_RETURN_1;
		
		MDGModifier mdgModifier;
		mStatus = mdgModifier.connect(sourcePlug, destinationPlug); 
		MCHECK_ERROR_RETURN_1;
		mStatus = mdgModifier.doIt();
		MCHECK_ERROR_RETURN_1;
	}
	return Aftk::C_Status::E_Status_Success;
}

