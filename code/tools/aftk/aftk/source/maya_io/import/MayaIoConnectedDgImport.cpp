#include "stdafx.h"
#include "MayaIoSceneConstraintImport.h"

using namespace MayaIo;

//#pragma optimize("", off)

Aftk::I_Plug* C_ConnectedDgImport::FindPlug(
	Aftk::C_SceneNode* pAftkSceneNode,
	const std::string& plugName)
{
	switch(pAftkSceneNode->GetType())
	{
	case Aftk::E_ObjType_SceneCamera:
		return C_SceneCameraImport::FindPlug(
			(Aftk::C_SceneCameraNode*)pAftkSceneNode,
			plugName.c_str());
	case Aftk::E_ObjType_SceneConstraint:
		return C_SceneConstraintImport::FindPlug(
			(Aftk::C_SceneConstraintNode*)pAftkSceneNode,
			plugName.c_str());
	case Aftk::E_ObjType_SceneAimConstraint:
		return C_SceneAimConstraintImport::FindPlug(
			(Aftk::C_SceneAimConstraintNode*)pAftkSceneNode,
			plugName.c_str());
	case Aftk::E_ObjType_SceneLookAtConstraint:
		return C_SceneLookAtConstraintImport::FindPlug(
			(Aftk::C_SceneLookAtConstraintNode*)pAftkSceneNode,
			plugName.c_str());
	case Aftk::E_ObjType_ScenePointConstraint:
		return C_ScenePointConstraintImport::FindPlug(
			(Aftk::C_ScenePointConstraintNode*)pAftkSceneNode,
			plugName.c_str());
	case Aftk::E_ObjType_SceneOrientConstraint:
		return C_SceneOrientConstraintImport::FindPlug(
			(Aftk::C_SceneOrientConstraintNode*)pAftkSceneNode,
			plugName.c_str());

	default:
		return C_SceneNodeImport::FindPlug(
			pAftkSceneNode,
			plugName.c_str());
	};
	return 0;
}

Aftk::C_Status C_ConnectedDgImport::Import(
	C_Importer* pImporter,
	MDagPath& transformDagPath,
	MDagPath* pShapeDagPath,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MPlugArray animatedPlugs;
	MFnDependencyNode fnTransformNode(transformDagPath.node());

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Importing animated plugs for node [" << fnTransformNode.name().asChar() << "] of type [" << fnTransformNode.typeName().asChar() << "] ...\n" AFTK_DO_DEBUG_END;
	#endif

	fnTransformNode.getConnections(animatedPlugs);
	#ifdef _DEBUG
	AFTK_DO_BEGIN << animatedPlugs.length() << " connected plugs for transform " << fnTransformNode.name().asChar() << "\n" AFTK_DO_DEBUG_END;
	#endif
	if (pShapeDagPath)
	{
		MFnDependencyNode fnShapeNode(pShapeDagPath->node());
		MPlugArray animatedShapePlugs;
		fnShapeNode.getConnections(animatedShapePlugs);
		#ifdef _DEBUG
		AFTK_DO_BEGIN << animatedShapePlugs.length() << " connected plugs for shape " << fnShapeNode.name().asChar() << ".\n" AFTK_DO_DEBUG_END;
		#endif
		for (U32 i = 0; i < animatedShapePlugs.length(); i++)
			animatedPlugs.append(animatedShapePlugs[i]);
	}

	//Import direct animation data
	for (U32 i = 0; i < animatedPlugs.length(); i++)
	{
		std::string animatedPlugName = animatedPlugs[i].name().asChar();

		Aftk::I_Object* pConnectedObject = 
			ImportDirectlyConnectedPlug(
				pImporter,
				animatedPlugs[i],
				status);
			AFTK_CHECK_ERROR_RETURN;
		if (!pConnectedObject)
			continue;
	
		std::string basePlugName;
		StripNodeNameFromPlugName(animatedPlugName, basePlugName);
		//Examine static and dynamic plugs to see if we've an existing connection point.
		Aftk::I_Plug* pAftkTargetPlug = FindPlug(
			pAftkSceneNode, 
			basePlugName);

		//Create a dynamic target plug if no static plug was found.
		if (!pAftkTargetPlug)
		{
			Aftk::C_DynamicPlug* pAftkCustomTarget = new Aftk::C_DynamicPlug();
			if (!pAftkCustomTarget)
				return Aftk::C_Status::E_Status_OutOfMemory;

			pAftkSceneNode->AddPlugPtr(pAftkCustomTarget);

			pAftkCustomTarget->SetName(basePlugName);
			pAftkTargetPlug = pAftkCustomTarget;
		}

		//Set top level plug connection.
		pAftkTargetPlug->SetSourceConnectionPtr(pConnectedObject);
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "done.\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Node* C_ConnectedDgImport::CreateNode(
	C_Importer* pImporter,
	MFnDependencyNode& fnSourceNode)
{
	Aftk::C_Status status;
	const char* typeName = fnSourceNode.typeName().asChar();

	if (!strcmp(typeName, "blendWeighted"))
	{
		//Export blend weighted node.
		Aftk::C_Node* pAftkNode = new Aftk::C_BlendWeightedNode(pImporter->GetAftkScenePtr());
		if (!pAftkNode)	return 0;

		status = C_DgBlendWeightedImport::Import(
			pImporter,
			pImporter->GetAftkScenePtr(),
			fnSourceNode,
			(Aftk::C_BlendWeightedNode*)pAftkNode);
		if (!status) return 0;

		return pAftkNode;
	}
	if (!strcmp(typeName, "unitConversion"))
	{
		//Export unit conversion node.
		Aftk::C_Node* pAftkNode = new Aftk::C_UnitConversionNode(pImporter->GetAftkScenePtr());
		if (!pAftkNode)
		{
			status = Aftk::C_Status::E_Status_OutOfMemory;
			return 0;
		}

		status = C_DgUnitConversionImport::Import(
			pImporter,
			pImporter->GetAftkScenePtr(),
			fnSourceNode,
			(Aftk::C_UnitConversionNode*)pAftkNode);
		if (!status) return 0;

		return pAftkNode;
	}
	else if (fnSourceNode.object().hasFn(MFn::kAnimCurve))
	{
		//Export animation curve
		Aftk::C_Node* pAftkNode = new Aftk::C_AnimationCurveNode(pImporter->GetAftkScenePtr());
		if (!pAftkNode)
		{
			status = Aftk::C_Status::E_Status_OutOfMemory;
			return 0;
		}

		status = C_DgAnimationCurveImport::Import(
			pImporter,
			pImporter->GetAftkScenePtr(),
			fnSourceNode,
			(Aftk::C_AnimationCurveNode*)pAftkNode);
		if (!status) return 0;

		return pAftkNode;
	}
	else if (fnSourceNode.object().hasFn(MFn::kDistance))
	{
		Aftk::C_Node* pAftkNode = new Aftk::C_DistanceDimNode(pImporter->GetAftkScenePtr());
		if (!pAftkNode)	return 0;

		status = C_DgDistanceDimImport::Import(
			pImporter,
			pImporter->GetAftkScenePtr(),
			fnSourceNode,
			(Aftk::C_DistanceDimNode*)pAftkNode);
		if (!status) return 0;

		return pAftkNode;
	}

	return 0;
}

Aftk::I_Object* C_ConnectedDgImport::ImportDirectlyConnectedPlug(
	C_Importer* pImporter,
	const MPlug& plug,
	Aftk::C_Status& status)
{
	MStatus mStatus;

	Aftk::I_Object* pReturnObject = 0;

	MPlugArray mapConnections;
	plug.connectedTo(mapConnections, true, false);
	U32 nConnections = mapConnections.length();
	if (nConnections != 1)
		return 0;

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Plug: " << plug.name().asChar() << "\n" AFTK_DO_DEBUG_END;
	#endif

	//For the moment we're not supporting more than one source connection per-plug.
	//Is this a required or useful feature?

	for (U32 iConnection = 0; iConnection < mapConnections.length(); iConnection++)
	{
		MObject sourceObject = mapConnections[iConnection].node();
        MFnDependencyNode fnSourceNode(sourceObject);

		pReturnObject = CreateNode(		
			pImporter,
			fnSourceNode);
		if (pReturnObject)
			continue;

		const char* typeName = fnSourceNode.typeName().asChar();

		//If we're connected to a dag-node then it must be one already exported previously.
		if (sourceObject.hasFn(MFn::kTransform))
		{
			MFnDagNode sourceDagNode(mapConnections[iConnection].node(), &mStatus);
			MCHECK_ERROR_RETURN0_1;

			MDagPath sourceDagPath;
			mStatus = sourceDagNode.getPath(sourceDagPath);
			MCHECK_ERROR_RETURN0_1;

			Aftk::C_SceneNode* pSourceSceneNode;
			if (!pImporter->GetAftkMayaMapping().GetAftkSceneNodeFromMayaDagPath(
					sourceDagPath, 
					&pSourceSceneNode))
			{
				//For some reason our current plug was connected to a transform that we haven't
				//previously exported. Any example of how this could happen would be that the
				//transform was flagged as being non-exportable.
				AFTK_DO_BEGIN << "Failed to find Aftk scene node for dag path [" << sourceDagPath.fullPathName().asChar() << "]" AFTK_DO_WARNING_END;
				return 0;
			}

			//Find the plug we're to connect to on our transform.
			std::string animatedPlugName = mapConnections[iConnection].name().asChar();
			std::string basePlugName;
			StripNodeNameFromPlugName(animatedPlugName, basePlugName);
			if (basePlugName.find("parentMatrix") != std::string::npos ||
				basePlugName.find("parentInverseMatrix") != std::string::npos)
			{
				pReturnObject = pSourceSceneNode;
			}
			else
			{
				Aftk::I_Plug* pAftkSourcePlug = FindPlug(
					pSourceSceneNode, 
					basePlugName);
				if (!pAftkSourcePlug)
				{
					//We couldn't find the plug we were trying to connect to.
					status = Aftk::C_Status::E_Status_Success;
					return 0;
				}
			
				//Return the plug we're to connect to.
				pReturnObject = pAftkSourcePlug;
			}
		}
	}
	//For the moment, animated child plugs are not exported.

	status = Aftk::C_Status::E_Status_Success;
	return pReturnObject;
}

