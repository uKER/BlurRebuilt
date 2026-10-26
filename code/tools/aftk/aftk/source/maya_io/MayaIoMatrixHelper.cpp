#include "stdafx.h"
#include "MayaIoMatrixHelper.h"

using namespace MayaIo; 

#pragma optimize("", off)

bool MatrixHelper::SetMatrix(
	Aftk::C_SceneNode* pAftkSceneNode, 
	const MDagPath& transformDagPath,
	F32 absNumericRange)
{
	MStatus mStatus;

	MFnTransform fnTransform(transformDagPath, &mStatus);

	if (!mStatus) return false;

	Aftk::Vector3F32 cumulativeTranslation	= Aftk::Vector3F32(0,0,0);
	Aftk::Vector3F32 cumulativeRotation		= Aftk::Vector3F32(0,0,0);
	Aftk::Vector3F32 cumulativeScale		= Aftk::Vector3F32(1.f,1.f,1.f);
	Aftk::Vector3F32 cumulativeShear		= Aftk::Vector3F32(0,0,0);
	MEulerRotation::RotationOrder eulerRotationOrder;

	//ROTATION
	{
		MEulerRotation eulerRotation;
		mStatus = fnTransform.getRotation(eulerRotation);

		cumulativeRotation[0] = (F32)eulerRotation[0];
		cumulativeRotation[1] = (F32)eulerRotation[1];
		cumulativeRotation[2] = (F32)eulerRotation[2];

		eulerRotationOrder = 
			eulerRotation.order;
	}

	//TRANSLATION
	{
		Aftk::Vector3F32 localPosition;
		GetLocalPosition(localPosition, fnTransform);

		MVector translation;
		translation = fnTransform.getTranslation(MSpace::kTransform, &mStatus);
		cumulativeTranslation[0] = (F32)translation[0] + localPosition[0];
		cumulativeTranslation[1] = (F32)translation[1] + localPosition[1];
		cumulativeTranslation[2] = (F32)translation[2] + localPosition[2];
	}

	//SCALE
	{
		double scale[3];
		mStatus = fnTransform.getScale(scale);
		if (!mStatus) return false;

		cumulativeScale[0] *= (F32)scale[0];
		cumulativeScale[1] *= (F32)scale[1];
		cumulativeScale[2] *= (F32)scale[2];
	}

	//SHEAR
	{
		double shear[3];
		mStatus = fnTransform.getShear(shear);
		if (!mStatus) return false;

		cumulativeShear[0] += (F32)shear[0];
		cumulativeShear[1] += (F32)shear[1];
		cumulativeShear[2] += (F32)shear[2];
	}

	//SCALE PIVOT
	Aftk::Vector3F32 scalePivot;
	{
		MPoint mScalePivot = fnTransform.scalePivot(MSpace::kTransform, &mStatus);
		if (!mStatus) return false;
		scalePivot[0] = (F32)mScalePivot[0];
		scalePivot[1] = (F32)mScalePivot[1];
		scalePivot[2] = (F32)mScalePivot[2];
	}

	//SCALE PIVOT TRANSLATION
	Aftk::Vector3F32 scalePivotTranslation;
	{
		MPoint mScalePivotTranslation = fnTransform.scalePivotTranslation(MSpace::kTransform, &mStatus);
		if (!mStatus) return false;
		scalePivotTranslation[0] = (F32)mScalePivotTranslation[0];
		scalePivotTranslation[1] = (F32)mScalePivotTranslation[1];
		scalePivotTranslation[2] = (F32)mScalePivotTranslation[2];
	}

	//ROTATE PIVOT
	Aftk::Vector3F32 rotatePivot;
	{
		MPoint mRotatePivot = fnTransform.rotatePivot(MSpace::kTransform, &mStatus);
		if (!mStatus) return false;
		rotatePivot[0] = (F32)mRotatePivot[0];
		rotatePivot[1] = (F32)mRotatePivot[1];
		rotatePivot[2] = (F32)mRotatePivot[2];
	}

	//ROTATE PIVOT TRANSLATION
	Aftk::Vector3F32 rotatePivotTranslation;
	{
		MPoint mRotatePivotTranslation = fnTransform.rotatePivotTranslation(MSpace::kTransform, &mStatus);
		if (!mStatus) return false;
		rotatePivotTranslation[0] = (F32)mRotatePivotTranslation[0];
		rotatePivotTranslation[1] = (F32)mRotatePivotTranslation[1];
		rotatePivotTranslation[2] = (F32)mRotatePivotTranslation[2];
	}

	//ROTATE ORIENTATION
	Aftk::Matrix44 rotateOrientation;
	{
		MQuaternion mRotateOrientation = fnTransform.rotateOrientation(MSpace::kTransform, &mStatus);
		if (!mStatus) return false;

		MatrixHelper::MayaToAftkMatrix44(
			rotateOrientation, 
			mRotateOrientation.asMatrix(), 
			absNumericRange);
	}

	//Scale Pivot.
	pAftkSceneNode->SetScalePivot(scalePivot);
	//Scale.
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetScalePlug().GetChildPtr(0))->SetValue(cumulativeScale[0]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetScalePlug().GetChildPtr(1))->SetValue(cumulativeScale[1]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetScalePlug().GetChildPtr(2))->SetValue(cumulativeScale[2]);
	//Shear.
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetShearPlug().GetChildPtr(0))->SetValue(cumulativeShear[0]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetShearPlug().GetChildPtr(1))->SetValue(cumulativeShear[1]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetShearPlug().GetChildPtr(2))->SetValue(cumulativeShear[2]);
	//Scale pivot translation.
	pAftkSceneNode->SetScalePivotTranslation(scalePivotTranslation);
	//Rotate pivot.
	pAftkSceneNode->SetRotatePivot(rotatePivot);
	//Rotate orientation.
	pAftkSceneNode->SetRotateOrientation(rotateOrientation);
	//Rotation.
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetRotatePlug().GetChildPtr(0))->SetValue(cumulativeRotation[0]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetRotatePlug().GetChildPtr(1))->SetValue(cumulativeRotation[1]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetRotatePlug().GetChildPtr(2))->SetValue(cumulativeRotation[2]);
	//Rotate pivot translation.
	pAftkSceneNode->SetRotatePivotTranslation(rotatePivotTranslation);
	//Translation
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetTranslatePlug().GetChildPtr(0))->SetValue(cumulativeTranslation[0]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetTranslatePlug().GetChildPtr(1))->SetValue(cumulativeTranslation[1]);
	((Aftk::C_StaticPlug<F32>*)pAftkSceneNode->GetTranslatePlug().GetChildPtr(2))->SetValue(cumulativeTranslation[2]);
	//Set rotation order.
	pAftkSceneNode->SetRotationOrder(GetAftkRotationOrderMEulerRotation(eulerRotationOrder));

/*
	std::cout << scalePivot[0] << " " << scalePivot[1] << " " << scalePivot[2] << std::endl;
	std::cout << cumulativeScale[0] << " " << cumulativeScale[1] << " " << cumulativeScale[2] << std::endl;
	std::cout << cumulativeShear[0] << " " << cumulativeShear[1] << " " << cumulativeShear[2] << std::endl;
	std::cout << scalePivotTranslation[0] << " " << scalePivotTranslation[1] << " " << scalePivotTranslation[2] << std::endl;
	std::cout << cumulativeRotation[0] << " " << cumulativeRotation[1] << " " << cumulativeRotation[2] << std::endl;
	std::cout << cumulativeTranslation[0] << " " << cumulativeTranslation[1] << " " << cumulativeTranslation[2] << std::endl;

	std::cout << std::endl;
	std::cout << rotateOrientation[0][0] << " " << rotateOrientation[0][1] << " " << rotateOrientation[0][2] << " " <<  rotateOrientation[0][3] << std::endl;
	std::cout << rotateOrientation[1][0] << " " << rotateOrientation[1][1] << " " << rotateOrientation[1][2] << " " <<  rotateOrientation[1][3] << std::endl;
	std::cout << rotateOrientation[2][0] << " " << rotateOrientation[2][1] << " " << rotateOrientation[2][2] << " " <<  rotateOrientation[2][3] << std::endl;
	std::cout << rotateOrientation[3][0] << " " << rotateOrientation[3][1] << " " << rotateOrientation[3][2] << " " <<  rotateOrientation[3][3] << std::endl;
	std::cout << std::endl;

	std::cout << out[0][0] << " " << out[0][1] << " " << out[0][2] << " " <<  out[0][3] << std::endl;
	std::cout << out[1][0] << " " << out[1][1] << " " << out[1][2] << " " <<  out[1][3] << std::endl;
	std::cout << out[2][0] << " " << out[2][1] << " " << out[2][2] << " " <<  out[2][3] << std::endl;
	std::cout << out[3][0] << " " << out[3][1] << " " << out[3][2] << " " <<  out[3][3] << std::endl;

	std::cout << mMatrix[0][0] << " " << mMatrix[0][1] << " " << mMatrix[0][2] << " " <<  mMatrix[0][3] << std::endl;
	std::cout << mMatrix[1][0] << " " << mMatrix[1][1] << " " << mMatrix[1][2] << " " <<  mMatrix[1][3] << std::endl;
	std::cout << mMatrix[2][0] << " " << mMatrix[2][1] << " " << mMatrix[2][2] << " " <<  mMatrix[2][3] << std::endl;
	std::cout << mMatrix[3][0] << " " << mMatrix[3][1] << " " << mMatrix[3][2] << " " <<  mMatrix[3][3] << std::endl;
*/
	return true;
}

bool MatrixHelper::GetMatrix(
	MMatrix& mMatrix,
	Aftk::E_RotationOrder& rotationOrder,
	const MDagPath& transformDagPath, 
	F32 absNumericRange)
{
	MStatus mStatus;
	MFnTransform fnTransform(transformDagPath, &mStatus);
	if (!mStatus) 
		return false;

	Aftk::Vector3F32 localPosition;
	MDagPath shapePath(transformDagPath);
	mStatus = shapePath.extendToShape();
	if (mStatus)
	{
		MFnDagNode fnShape(shapePath);
		if (!GetLocalPosition(localPosition, fnShape))
			return false;
	}
	else
	{
		localPosition[0] = 0.f;
		localPosition[1] = 0.f;
		localPosition[2] = 0.f;
	}

	MMatrix mOffset;
	mOffset.setToIdentity();
	mOffset[3][0] = localPosition[0];
	mOffset[3][1] = localPosition[1];
	mOffset[3][2] = localPosition[2];

	MTransformationMatrix mTransformationMatrix = 
		fnTransform.transformation();
	mMatrix = mOffset * mTransformationMatrix.asMatrix();

	rotationOrder = GetAftkRotationOrderMTransformationMatrix(mTransformationMatrix.rotationOrder());
	return true;
}

bool MatrixHelper::GetLocalPosition(Aftk::Vector3F32& localPosition, const MFnDagNode& fnDagNode)
{
	MStatus mStatus;
	MPlug localPositionPlug = fnDagNode.findPlug("localPosition", &mStatus);
	if (mStatus)
	{
		MPlug x = localPositionPlug.child(0, &mStatus);
		if (!mStatus) return false;
		MPlug y = localPositionPlug.child(1, &mStatus);
		if (!mStatus) return false;
		MPlug z = localPositionPlug.child(2, &mStatus);
		if (!mStatus) return false;
		x.getValue(localPosition[0]);
		y.getValue(localPosition[1]);
		z.getValue(localPosition[2]);
	}
	else
	{
		localPosition[0] = 0.f;
		localPosition[1] = 0.f;
		localPosition[2] = 0.f;
	}

	return true;
}

bool MatrixHelper::MayaToAftkMatrix44(
	Aftk::Matrix44& aftkMatrix, 
	const MMatrix& mayaMatrix,
	F32 absNumericRange)
{
	F32 tempMatrix1[4][4];
	mayaMatrix.get(tempMatrix1);

	bool invalid = false;
	if (!IsNumberValid(tempMatrix1[0][0], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[0][1], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[0][2], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[0][3], absNumericRange))
		invalid = true;

	if (!IsNumberValid(tempMatrix1[1][0], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[1][1], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[1][2], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[1][3], absNumericRange))
		invalid = true;

	if (!IsNumberValid(tempMatrix1[2][0], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[2][1], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[2][2], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[2][3], absNumericRange))
		invalid = true;

	if (!IsNumberValid(tempMatrix1[3][0], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[3][1], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[3][2], absNumericRange))
		invalid = true;
	if (!IsNumberValid(tempMatrix1[3][3], absNumericRange))
		invalid = true;

	if (invalid)
	{
		UtMaths::SetIdentity(&aftkMatrix);
		return false;
	}

	Aftk::Vector4F32 ax1(tempMatrix1[0][0],tempMatrix1[0][1],tempMatrix1[0][2],tempMatrix1[0][3]);
	Aftk::Vector4F32 ay1(tempMatrix1[1][0],tempMatrix1[1][1],tempMatrix1[1][2],tempMatrix1[0][3]);
	Aftk::Vector4F32 az1(tempMatrix1[2][0],tempMatrix1[2][1],tempMatrix1[2][2],tempMatrix1[0][3]);
	// translation vector in both maya and CMat44 lies along the bottom row of the matrix
	Aftk::Vector4F32 aw1(
		tempMatrix1[3][0],
		tempMatrix1[3][1],
		tempMatrix1[3][2],
		tempMatrix1[3][3]);

	aftkMatrix[0] = ax1;
	aftkMatrix[1] = ay1;
	aftkMatrix[2] = az1;
	aftkMatrix[3] = aw1;

	return true;
}

void MatrixHelper::AftkToMayaMatrix44(
	const Aftk::Matrix44& aftkMatrix, 
	MMatrix& mayaMatrix)
{
	mayaMatrix[0][0] = aftkMatrix[0][0];mayaMatrix[0][1] = aftkMatrix[0][1];mayaMatrix[0][2] = aftkMatrix[0][2];mayaMatrix[3][0] = aftkMatrix[3][0];
	mayaMatrix[1][0] = aftkMatrix[1][0];mayaMatrix[1][1] = aftkMatrix[1][1];mayaMatrix[1][2] = aftkMatrix[1][2];mayaMatrix[3][1] = aftkMatrix[3][1];
	mayaMatrix[2][0] = aftkMatrix[2][0];mayaMatrix[2][1] = aftkMatrix[2][1];mayaMatrix[2][2] = aftkMatrix[2][2];mayaMatrix[3][2] = aftkMatrix[3][2];
	mayaMatrix[3][0] = aftkMatrix[3][0];mayaMatrix[3][1] = aftkMatrix[3][1];mayaMatrix[3][2] = aftkMatrix[3][2];mayaMatrix[3][3] = aftkMatrix[3][3];
}
