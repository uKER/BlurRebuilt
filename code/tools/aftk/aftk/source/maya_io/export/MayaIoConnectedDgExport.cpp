#include "stdafx.h"

using namespace MayaIo;


Aftk::C_Status C_ConnectedDgExport::Export(
	C_Exporter* pExporter,
	MDagPath& transformDagPath,
	MDagPath* pShapeDagPath,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	MStatus mStatus;


	U32 numPlugs = pAftkSceneNode->GetNumStaticPlugs();
	U32 numDynamicPlugs = pAftkSceneNode->GetNumDynamicPlugs();

	for (U32 i=0; i<numPlugs; ++i)
		ExportDirectlyConnectedPlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pAftkSceneNode->GetStaticPlugPtr(i));

	for (U32 i=0; i<numDynamicPlugs; ++i)
		ExportDirectlyConnectedPlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pAftkSceneNode->GetDynamicPlugPtr(i));

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_ConnectedDgExport::ExportDirectlyConnectedPlug(
	C_Exporter* pExporter,
	MDagPath& transformDagPath,
	MDagPath* pShapeDagPath,
	const Aftk::C_Scene *pAftkScene,
	const Aftk::I_Plug* pAftkPlug)
{
	MStatus mStatus;
	Aftk::C_Status status;

	if (pAftkPlug->IsCompound())
	{
		MDGModifier dgMod;

		if (pAftkPlug->GetName() == "translate")
		{
			MString plugNames[3] = {"translateX", "translateY", "translateZ"};
			for( U32 i=0; i<3; ++i )
			{
				MFnAnimCurve fnCurve;
				const Aftk::I_Plug *pChildPlug = pAftkPlug->GetChildPtr(i);
				Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pChildPlug->GetSourceConnectionPtr();

				if( !pAftkPlug || !pAftkAnimCurve )
					continue;

				fnCurve.create(MFnAnimCurve::kAnimCurveTL, 0, &mStatus);
				MCHECK_ERROR_RETURN_1;

				status = C_DgAnimationCurveExport::Export(pExporter, fnCurve, pAftkScene, pAftkAnimCurve);
				if (!status)
					return status;

				MPlug srcPlug;
				srcPlug = fnCurve.findPlug("output", &mStatus);
				MCHECK_ERROR_RETURN_1;

				MPlug dstPlug;
				MFnDependencyNode dstFnNode(transformDagPath.node(), &mStatus);
				MCHECK_ERROR_RETURN_1;

				dstPlug = dstFnNode.findPlug(plugNames[i], &mStatus);
				MCHECK_ERROR_RETURN_1;

				mStatus = dgMod.connect(srcPlug, dstPlug);
				MCHECK_ERROR_RETURN_1

			}
		}
		else if (pAftkPlug->GetName() == "rotate")
		{
			MString plugNames[3] = {"rotateX", "rotateY", "rotateZ"};
			for( U32 i=0; i<3; ++i )
			{
				MFnAnimCurve fnCurve;
				const Aftk::I_Plug *pChildPlug = pAftkPlug->GetChildPtr(i);
				Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pChildPlug->GetSourceConnectionPtr();

				if( !pAftkPlug || !pAftkAnimCurve )
					continue;

				fnCurve.create(MFnAnimCurve::kAnimCurveTA, 0, &mStatus);
				MCHECK_ERROR_RETURN_1;

				status = C_DgAnimationCurveExport::Export(pExporter, fnCurve, pAftkScene, pAftkAnimCurve);
				if (!status)
					return status;

				MPlug srcPlug;
				srcPlug = fnCurve.findPlug("output", &mStatus);
				MCHECK_ERROR_RETURN_1;

				MPlug dstPlug;
				MFnDependencyNode dstFnNode(transformDagPath.node(), &mStatus);
				MCHECK_ERROR_RETURN_1;

				dstPlug = dstFnNode.findPlug(plugNames[i], &mStatus);
				MCHECK_ERROR_RETURN_1;

				mStatus = dgMod.connect(srcPlug, dstPlug);
				MCHECK_ERROR_RETURN_1
			}
		}
		else if (pAftkPlug->GetName() == "scale")
		{
			MString plugNames[3] = {"scaleX", "scaleY", "scaleZ"};
			for( U32 i=0; i<3; ++i )
			{
				MFnAnimCurve fnCurve;
				const Aftk::I_Plug *pChildPlug = pAftkPlug->GetChildPtr(i);
				Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pChildPlug->GetSourceConnectionPtr();

				if( !pAftkPlug || !pAftkAnimCurve )
					continue;

				fnCurve.create(MFnAnimCurve::kAnimCurveTL, 0, &mStatus);
				MCHECK_ERROR_RETURN_1;

				status = C_DgAnimationCurveExport::Export(pExporter, fnCurve, pAftkScene, pAftkAnimCurve);
				if (!status)
					return status;

				MPlug srcPlug;
				srcPlug = fnCurve.findPlug("output", &mStatus);
				MCHECK_ERROR_RETURN_1;

				MPlug dstPlug;
				MFnDependencyNode dstFnNode(transformDagPath.node(), &mStatus);
				MCHECK_ERROR_RETURN_1;

				dstPlug = dstFnNode.findPlug(plugNames[i], &mStatus);
				MCHECK_ERROR_RETURN_1;

				mStatus = dgMod.connect(srcPlug, dstPlug);
				MCHECK_ERROR_RETURN_1
			}
		}
		else if (pAftkPlug->GetName() == "shear")
		{
			MString plugNames[3] = {"shearX", "shearY", "shearZ"};
			for( U32 i=0; i<3; ++i )
			{
				MFnAnimCurve fnCurve;
				const Aftk::I_Plug *pChildPlug = pAftkPlug->GetChildPtr(i);
				Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pChildPlug->GetSourceConnectionPtr();

				if( !pAftkPlug || !pAftkAnimCurve )
					continue;

				fnCurve.create(MFnAnimCurve::kAnimCurveTL, 0, &mStatus);
				MCHECK_ERROR_RETURN_1;

				status = C_DgAnimationCurveExport::Export(pExporter, fnCurve, pAftkScene, pAftkAnimCurve);
				if (!status)
					return status;

				MPlug srcPlug;
				srcPlug = fnCurve.findPlug("output", &mStatus);
				MCHECK_ERROR_RETURN_1;

				MPlug dstPlug;
				MFnDependencyNode dstFnNode(transformDagPath.node(), &mStatus);
				MCHECK_ERROR_RETURN_1;

				dstPlug = dstFnNode.findPlug(plugNames[i], &mStatus);
				MCHECK_ERROR_RETURN_1;

				mStatus = dgMod.connect(srcPlug, dstPlug);
				MCHECK_ERROR_RETURN_1
			}
		}
		else
		{
			status = ExportCompoundPlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pAftkPlug, MObject::kNullObj);
			if (!status)
				return status;
		}
		mStatus = dgMod.doIt();
		MCHECK_ERROR_RETURN_1
	}
	else
	{
		status = ExportSimplePlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pAftkPlug, MObject::kNullObj);
		if (!status)
			return status;
	}
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_ConnectedDgExport::ExportCompoundPlug(
	C_Exporter* pExporter,
	MDagPath& transformDagPath,
	MDagPath* pShapeDagPath,
	const Aftk::C_Scene *pAftkScene,
	const Aftk::I_Plug* pAftkPlug,
	MObject& parentAttrObj)
{
	MStatus mStatus;
	Aftk::C_Status status;

	assert(pAftkPlug->IsCompound());
	
	std::string longName = pAftkPlug->GetName();

	MFnCompoundAttribute fnCompoundAttr;
	MObject compoundAttrObj = fnCompoundAttr.create(MString(longName.c_str()), MString(longName.c_str()), &mStatus);
	MCHECK_ERROR_RETURN_1;

	for (U32 i=0; i<pAftkPlug->GetNumChildren(); ++i)
	{
		const Aftk::I_Plug *pChildPlug = pAftkPlug->GetChildPtr(i);
		if (pChildPlug->IsCompound())
		{
			std::string childLongName = pChildPlug->GetName();

			MFnCompoundAttribute fnChildCompoundAttr;
			MObject compoundChildAttrObj = fnChildCompoundAttr.create(MString(childLongName.c_str()), MString(childLongName.c_str()), &mStatus);
			MCHECK_ERROR_RETURN_1;
			status = ExportCompoundPlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pChildPlug, compoundChildAttrObj);
			if (!status)
				return status;

			mStatus = fnCompoundAttr.addChild(fnChildCompoundAttr.object());
			MCHECK_ERROR_RETURN_1
		}
		else
		{
			status = ExportSimplePlug(pExporter, transformDagPath, pShapeDagPath, pAftkScene, pChildPlug, compoundAttrObj);
			if (!status)
				return status;
		}
	}

	if (fnCompoundAttr.numChildren() > 0)
	{
		// hook up attribute to parent (could be dagPath, or a parent composite attribute)
		if (parentAttrObj != MObject::kNullObj)
		{
			MFnCompoundAttribute fnParentAttr(parentAttrObj, &mStatus);
			MCHECK_ERROR_RETURN_1
			mStatus = fnParentAttr.addChild(compoundAttrObj);
			MCHECK_ERROR_RETURN_1
		}
		else
		{
			MFnDependencyNode srcFnNode(transformDagPath.node(), &mStatus);
			mStatus = srcFnNode.addAttribute(compoundAttrObj, MFnDependencyNode::kLocalDynamicAttr);
			MCHECK_ERROR_RETURN_1;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_ConnectedDgExport::ExportSimplePlug(
	C_Exporter* pExporter,
	MDagPath& transformDagPath,
	MDagPath* pShapeDagPath,
	const Aftk::C_Scene *pAftkScene,
	const Aftk::I_Plug* pAftkPlug,
	MObject& parentAttrObj)
{
	MStatus mStatus;
	Aftk::C_Status status;

	assert(!pAftkPlug->IsCompound());


	const Aftk::I_Object *pSourceConnection = pAftkPlug->GetSourceConnectionPtr();
	if (pSourceConnection)
	{
		if (pSourceConnection->GetType() != Aftk::E_ObjType_AnimationCurve)
		{
			AFTK_DO_BEGIN << "Custom parameter " << pAftkPlug->GetName() << " is not an animation curve and will not be exported." AFTK_DO_WARNING_END;
			return Aftk::C_Status::E_Status_Success;
		}

		MFnNumericAttribute fnNumAttr;
		std::string childLongName = pAftkPlug->GetName();

		MObject childAttrObj = fnNumAttr.create(MString(childLongName.c_str()), MString(childLongName.c_str()), MFnNumericData::kFloat, 0.0f, &mStatus);
		MCHECK_ERROR_RETURN_1;

		MPlug srcPlug, dstPlug;
		if (parentAttrObj != MObject::kNullObj)
		{
			MFnCompoundAttribute fnParentAttr(parentAttrObj, &mStatus);
			MCHECK_ERROR_RETURN_1
			mStatus = fnParentAttr.addChild(childAttrObj);
			MCHECK_ERROR_RETURN_1

			MFnDependencyNode srcFnNode(transformDagPath.node(), &mStatus);
			MCHECK_ERROR_RETURN_1
			dstPlug = srcFnNode.findPlug(childAttrObj, &mStatus);
			MCHECK_ERROR_RETURN_1
		}
		else
		{
			MFnDependencyNode srcFnNode(transformDagPath.node(), &mStatus);
			MCHECK_ERROR_RETURN_1;
			mStatus = srcFnNode.addAttribute(childAttrObj, MFnDependencyNode::kLocalDynamicAttr);
			MCHECK_ERROR_RETURN_1
			dstPlug = srcFnNode.findPlug(childAttrObj, &mStatus);
			MCHECK_ERROR_RETURN_1
		}

		// Now create the curve ...
		MFnAnimCurve fnCurve;
		Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pSourceConnection;

		fnCurve.create(MFnAnimCurve::kAnimCurveTL, 0, &mStatus);
		MCHECK_ERROR_RETURN_1;

		status = C_DgAnimationCurveExport::Export(pExporter, fnCurve, pAftkScene, pAftkAnimCurve);
		if (!status)
			return status;

		fnCurve.setName(MString(childLongName.c_str()), &mStatus);
		MCHECK_ERROR_RETURN_1;

		srcPlug = fnCurve.findPlug("output", &mStatus);
		MCHECK_ERROR_RETURN_1;

		MDGModifier dgMod;
		mStatus = dgMod.connect(srcPlug, dstPlug);
		MCHECK_ERROR_RETURN_1

		mStatus = dgMod.doIt();
		MCHECK_ERROR_RETURN_1
	}

	// Export of other attribute types not needed (yet?)

	return Aftk::C_Status::E_Status_Success;

}
