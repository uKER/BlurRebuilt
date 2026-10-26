#include "stdafx.h"

using namespace MayaIo;


Aftk::C_Status C_DgAnimationCurveExport::Export(
	C_Exporter *pExporter,
	MFnAnimCurve& fnAnimCurve,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_AnimationCurveNode* pAftkAnimCurveNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	fnAnimCurve.setName(MString(pAftkAnimCurveNode->GetName().c_str()));
	status = C_DgNodeExport::Export(pExporter, fnAnimCurve, pAftkScene, pAftkAnimCurveNode);
	if (!status)
		return status;

	Aftk::E_AnimCurveType type = pAftkAnimCurveNode->GetCurveType();

	switch (type)
	{
	case Aftk::E_AnimCurveType_Hermite:
		status = ExportHermiteCurve(fnAnimCurve, pAftkAnimCurveNode, pAftkScene);
		break;

	case Aftk::E_AnimCurveType_Linear:
	case Aftk::E_AnimCurveType_Unknown:
	default:
		status = ExportLinearCurve(fnAnimCurve, pAftkAnimCurveNode, pAftkScene);
	}


	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_DgAnimationCurveExport::ExportHermiteCurve(
	MFnAnimCurve& fnAnimCurve,
	const Aftk::C_AnimationCurveNode* pAftkAnimCurveNode,
	const Aftk::C_Scene* pAftkScene)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MTime oneSecond( 1, MTime::kSeconds );
	F64 frameRate = oneSecond.as(MTime::uiUnit());

	F32 ourFrameRate = pAftkScene->GetFrameRate();

	U32 numKeys = pAftkAnimCurveNode->GetNumKeys();
	for (U32 i=0; i<numKeys; ++i)
	{
		Aftk::C_HermiteKey key;

		if (!pAftkAnimCurveNode->GetKey(key, i))
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to get curve key value");

		if (fnAnimCurve.animCurveType() == MFnAnimCurve::kAnimCurveTA ||
			fnAnimCurve.animCurveType() == MFnAnimCurve::kAnimCurveTL)
		{
			MTime keyTime(key.GetX()/frameRate, MTime::kSeconds);
			fnAnimCurve.addKey(keyTime, key.GetValue(), MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, 0, &mStatus);
			MCHECK_ERROR_RETURN_1
		}
		else
		{
			fnAnimCurve.addKey(key.GetX(), key.GetValue(), MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, 0, &mStatus);
			MCHECK_ERROR_RETURN_1
		}

		F32 inTangentAngle = key.GetInTangentAngle();
		F32 outTangentAngle = key.GetOutTangentAngle();

		mStatus = fnAnimCurve.setAngle(i, MAngle(inTangentAngle, MAngle::kRadians), true);
		mStatus = fnAnimCurve.setAngle(i, MAngle(outTangentAngle, MAngle::kRadians), false);
/*
		Aftk::Vector2F32 inTangent = key.GetInTangentVector();
		Aftk::Vector2F32 outTangent = key.GetOutTangentVector();

		mStatus = fnAnimCurve.setTangent(i, inTangent.x, inTangent.y, true);
		MCHECK_ERROR_RETURN_1
		mStatus = fnAnimCurve.setTangent(i, outTangent.x, outTangent.y, false);
		MCHECK_ERROR_RETURN_1
*/
	}

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_DgAnimationCurveExport::ExportLinearCurve(
	MFnAnimCurve &fnAnimCurve,
	const Aftk::C_AnimationCurveNode *pAftkAnimCurveNode,
	const Aftk::C_Scene *pAftkScene)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MTime oneSecond( 1, MTime::kSeconds );
	F64 frameRate = oneSecond.as(MTime::uiUnit());

	F32 ourFrameRate = pAftkScene->GetFrameRate();

	U32 numKeys = pAftkAnimCurveNode->GetNumKeys();
	for (U32 i=0; i<numKeys; ++i)
	{
		Aftk::C_LinearKey key;

		if (!pAftkAnimCurveNode->GetKey(key, i))
			return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Failed to get curve key value");

		if (fnAnimCurve.animCurveType() == MFnAnimCurve::kAnimCurveTA ||
			fnAnimCurve.animCurveType() == MFnAnimCurve::kAnimCurveTL)
		{
			MTime keyTime(key.GetX() / ourFrameRate, MTime::kSeconds);
			fnAnimCurve.addKey(keyTime, key.GetValue(), MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, 0, &mStatus);
			MCHECK_ERROR_RETURN_1
		}
		else
		{
			fnAnimCurve.addKey(key.GetX(), key.GetValue(), MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, 0, &mStatus);
			MCHECK_ERROR_RETURN_1
		}
	}

	return Aftk::C_Status::E_Status_Success;





}