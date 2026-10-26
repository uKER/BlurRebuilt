#include "stdafx.h"


using namespace MayaIo;

Aftk::C_Status C_SceneSplineExport::Export(
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

	Aftk::C_SceneSplineNode* pAftkSceneSplineNode =
		(Aftk::C_SceneSplineNode*)pAftkSceneNode;

	status = C_SceneNodeExport::Export(
		true,
		pExporter,
		parentTransformObject,
		mayaNewTransformObject,
		mayaNewShapeObject,
		pAftkScene,
		pAftkSceneNode);
	AFTK_CHECK_ERROR_RETURN;

	//We must create this node
	AFTK_ASSERT(create == true);

	//Copy control vertices
	MPointArray cvs;
	mStatus = cvs.setLength(pAftkSceneSplineNode->GetNumCVs());
	MCHECK_ERROR_RETURN_1;
	for (U32 i = 0; i < pAftkSceneSplineNode->GetNumCVs(); i++)
	{
		const Aftk::Position3* pAftkCV = &pAftkSceneSplineNode->GetCV(i);
		MPoint p((*pAftkCV)[0], (*pAftkCV)[1], (*pAftkCV)[2]);
		mStatus = cvs.set(p, i);
		MCHECK_ERROR_RETURN_1;
	}

	if (pAftkSceneSplineNode->GetNumKnots() == 0)
	{
		//This spline has no knots! Generate a linear spline.
		pAftkSceneSplineNode->GenerateLinearFromCVs();
	}

	MDoubleArray knotSequences;
	mStatus = knotSequences.setLength(pAftkSceneSplineNode->GetNumKnots());
	MCHECK_ERROR_RETURN_1;
	for (U32 i = 0; i < pAftkSceneSplineNode->GetNumKnots(); i++)
	{
		mStatus = knotSequences.set(pAftkSceneSplineNode->GetKnot(i), i);
		MCHECK_ERROR_RETURN_1;
	}

	MFnNurbsCurve::Form form;

	// set spline form
	switch (pAftkSceneSplineNode->GetSplineForm())
	{
	case Aftk::E_SplineForm_Open:
		form = MFnNurbsCurve::kOpen;
		break;

	case Aftk::E_SplineForm_Closed:
		form = MFnNurbsCurve::kClosed;
		break;

	case Aftk::E_SplineForm_Periodic:
		form = MFnNurbsCurve::kPeriodic;
		break;

	default:
		form = MFnNurbsCurve::kOpen;
		break;
	}

	MFnNurbsCurve splineFn;
	mayaNewShapeObject = splineFn.create(
		cvs,
		knotSequences, 
		pAftkSceneSplineNode->GetDegree(), 
		form,
		false,
		pAftkSceneSplineNode->IsRational(),
		mayaNewTransformObject,
		&mStatus);
	MCHECK_BEGIN << "Failed to create spline node" MCHECK_ERROR_RETURN_END_1;
	
	return Aftk::C_Status::E_Status_Success;
}

