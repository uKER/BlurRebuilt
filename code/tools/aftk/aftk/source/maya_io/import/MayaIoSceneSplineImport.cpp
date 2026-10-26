#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneSplineImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	#ifdef DEBUG_PRINT
	AFTK_DO_BEGIN << "Exporting NURBS curve.\n" AFTK_DO_DEBUG_END;
	#endif
	//Export the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_BEGIN << "Could not export scene node." AFTK_CHECK_ERROR_RETURN_END_0;

	MFnNurbsCurve mayaNurbsCurve(dagPath, &mStatus);
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

	Aftk::C_SceneSplineNode* pAftkSceneSplineNode = (Aftk::C_SceneSplineNode*)pAftkSceneNode;

	// set spline type
	pAftkSceneSplineNode->SetSplineType(Aftk::E_SplineType_Nurbs);

	// set spline form
	switch (mayaNurbsCurve.form())
	{
		case MFnNurbsCurve::kOpen:
			pAftkSceneSplineNode->SetSplineForm(Aftk::E_SplineForm_Open);
			break;

		case MFnNurbsCurve::kClosed:
			pAftkSceneSplineNode->SetSplineForm(Aftk::E_SplineForm_Closed);
			break;

		case MFnNurbsCurve::kPeriodic:
			pAftkSceneSplineNode->SetSplineForm(Aftk::E_SplineForm_Periodic);
			break;

		default:
			pAftkSceneSplineNode->SetSplineForm(Aftk::E_SplineForm_Open);
			break;
	}

	pAftkSceneSplineNode->SetDegree((U32)mayaNurbsCurve.degree());

	U32 numCvs = mayaNurbsCurve.numCVs(&mStatus);
	MCHECK_BEGIN << "Could not acquire num cv points." MCHECK_ERROR_RETURN_END_1;

	// copy control vertices
	pAftkSceneSplineNode->SetCVInitialCapacity(numCvs);
	for (U32 i = 0; i < numCvs; i++)
	{
		MPoint mayaPoint;
		mayaNurbsCurve.getCV(i, mayaPoint, MSpace::kObject);

		static const double e = 0.0001;
		if (abs(mayaPoint.x) < e) 
		{
			mayaPoint.x = 0;
		}
		if (abs(mayaPoint.y) < e) 
		{
			mayaPoint.y = 0;
		}
		if (abs(mayaPoint.z) < e) 
		{
			mayaPoint.z = 0;
		}

		Aftk::Position3 cv(
			(F32)mayaPoint.x, 
			(F32)mayaPoint.y, 
			(F32)mayaPoint.z);

		pAftkSceneSplineNode->AddCV(cv);
	}

	MDoubleArray knots;
	mStatus = mayaNurbsCurve.getKnots(knots);
	MCHECK_BEGIN << "Could not acquire knots." MCHECK_ERROR_RETURN_END_1;

	pAftkSceneSplineNode->SetKnotInitialCapacity(mayaNurbsCurve.numKnots());
	for (U32 i = 0; i < (U32)knots.length(); i++)
	{
		pAftkSceneSplineNode->AddKnot((F32)knots[i]);
	}

	//Todo - how do we find out if a curve is rational? Assume false, as most NURBSs are
	pAftkSceneSplineNode->SetIsRational(false);

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Spline C_Node " << mayaNurbsCurve.name().asChar() << " id " << mayaNurbsCurve.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

