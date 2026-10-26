#include "stdafx.h"
#include "../MayaIoMatrixHelper.h"

using namespace MayaIo;

Aftk::C_Status C_SceneNodeImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	bool setBasePose,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	// Apply proporties common to all shapes
	if (!pAftkSceneNode)
	{
		AFTK_DO_BEGIN << "Null scene node provided to AftkSceneNodeExport::Export()" AFTK_DO_ERROR_RETURN_END_1
	}

	MFnDagNode fnDagNode(dagPath, &mStatus);
	AFTK_CHECK_BEGIN << "Failed to get dag node from given dag path." AFTK_CHECK_ERROR_RETURN_END_1

	//Set node name
	pAftkSceneNode->SetName(fnDagNode.name().asChar());

//	AFTK_DO_BEGIN << "Setting transform for " << pAftkSceneNode->GetName() << "\n" AFTK_DO_DEBUG_END;

	//If there are any gaps in our hierarchy then we must use matrix decomposition in order to
	//set our transform.
	if (pExclusiveMatrix)
	{
		Aftk::Matrix44 aftkMatrix;
		MMatrix mMatrix;
		Aftk::E_RotationOrder rotationOrder;
		MatrixHelper::GetMatrix(
			mMatrix, 
			rotationOrder,
			dagPath, 
			pOptions->absNumericRange);
		MatrixHelper::MayaToAftkMatrix44(aftkMatrix, mMatrix, pOptions->absNumericRange);
		aftkMatrix = aftkMatrix * (*pExclusiveMatrix);
		pAftkSceneNode->SetRotationOrder(rotationOrder);
		pAftkSceneNode->SetLocalMatrix(aftkMatrix);
	}
	else
	{
		if (!MatrixHelper::SetMatrix(
			pAftkSceneNode, 
			dagPath, 
			pOptions->absNumericRange))
		{
			LOGERROR(Aftk::g_aftkMainLogger, "Failed to set matrix for [%s].\n",
				pAftkSceneNode->GetName());
			return Aftk::C_Status::E_Status_Failure;
		}
	}

	{
		MPlug mayaPlug = fnDagNode.findPlug("visibility", &mStatus);
		MCHECK_ERROR_RETURN_1
		bool value;
		mStatus = mayaPlug.getValue(value);
		MCHECK_ERROR_RETURN_1

		Aftk::C_StaticPlug<bool>& aftkPlug = pAftkSceneNode->GetVisibilityPlug();
		aftkPlug.SetValue(value);

		Aftk::I_Object* pObject = C_ConnectedDgImport::ImportDirectlyConnectedPlug(
			pImporter,
			mayaPlug,
			status);
		CHECK_ERROR_RETURN;
		if (pObject)
			aftkPlug.SetSourceConnectionPtr(pObject);
	}

	{
		MPlug mayaPlug = fnDagNode.findPlug("rotationInterpolation", &mStatus);
		MCHECK_ERROR_RETURN_1
		int value;
		mStatus = mayaPlug.getValue(value);
		MCHECK_ERROR_RETURN_1

		static int kQuatInterp = 2;
		if (value == kQuatInterp)
			pAftkSceneNode->SetRotationInterpolation(Aftk::E_RotationInterpolation_Quaternion);
		else
			pAftkSceneNode->SetRotationInterpolation(Aftk::E_RotationInterpolation_Independent);
	}

	//Don't add dynamic plugs to boned instances in order to save memory.
	if (!pImporter->IsBonedInstance(dagPath))
	{
		//Import from transform.
		status = AddDynamicPlugs(
			pImporter,
			fnDagNode,
			pAftkSceneNode);
		if (!status) return status;

		//Attempt to import from shape.
		MDagPath shapeDagPath = dagPath;
		mStatus = shapeDagPath.extendToShape();
		if (mStatus)
		{
			status = AddDynamicPlugs(
				pImporter,
				shapeDagPath,
				pAftkSceneNode);
			if (!status) return status;
		}
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Aftk::C_Scene C_Node " << pAftkSceneNode->GetName() << " id " << fnDagNode.typeId().id() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneNodeImport::AddDynamicPlugs(
	C_Importer* pImporter,
	const MFnDagNode& fnDagNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	MStatus mStatus;


	//Add any dynamic attributes to the given AFTK scene node.
	U32 attributeCount = fnDagNode.attributeCount(&mStatus);
	MCHECK_BEGIN << "Failed to get attribute count." MCHECK_ERROR_RETURN_END_1

	std::string tmpName;
	for (U32 i = 0; i < attributeCount; i++)
	{
		MStatus attrStatus;
		MObject attrObj = fnDagNode.attribute( i, &attrStatus );
		if (attrStatus && (!attrObj.isNull()))
		{
			MPlug attrPlug(fnDagNode.object(), attrObj);

			//If dynamic and not filtered.
			if (attrPlug.isDynamic())
			{
				tmpName = attrPlug.name().asChar();
//				OutputDebugString((tmpName + "\n").c_str());
				StripNodeNameFromPlugName(tmpName, tmpName);
				if (!pImporter->GetPlugFilter().Find(tmpName.c_str()))
				{
					//Don't override plugs already exported. In this way shape plugs do not override transform plugs.
					if (pAftkSceneNode->GetPlugPtr(tmpName))
						continue;
					//Add a new dynamic attribute(plug) to our scene node.
					Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
					if (!pAftkDynamicPlug)
						return Aftk::C_Status::E_Status_OutOfMemory;
					pAftkSceneNode->AddPlugPtr(pAftkDynamicPlug);

					//Populate our new dynamic attribute.
					status = MayaPlugToAftkDynamicPlug(
						pImporter,
						attrPlug,
						*pAftkDynamicPlug);
					AFTK_CHECK_ERROR_RETURN;
				}
			}
			else
			{
				tmpName = attrPlug.name().asChar();
//				OutputDebugString((tmpName + " static\n").c_str());
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

 Aftk::C_Status C_SceneNodeImport::ImportConnectivity(	
	C_Importer* pImporter,
	const MDagPath& transformPath,
	const MDagPath* pShapePath,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	MPlugArray drivenPlugs;
	MFnDependencyNode fnTransformNode(transformPath.node());

	fnTransformNode.getConnections(drivenPlugs);
	#ifdef _DEBUG
	AFTK_DO_BEGIN << drivenPlugs.length() << " connected plugs for transform " << fnTransformNode.name().asChar() << "\n" AFTK_DO_DEBUG_END;
	#endif
	if (pShapePath)
	{
		MFnDependencyNode fnShapeNode(pShapePath->node());
		MPlugArray drivenShapePlugs;
		fnShapeNode.getConnections(drivenShapePlugs);
		#ifdef _DEBUG
		AFTK_DO_BEGIN << drivenShapePlugs.length() << " connected plugs for shape " << fnShapeNode.name().asChar() << ".\n" AFTK_DO_DEBUG_END;
		#endif
		for (U32 i = 0; i < drivenShapePlugs.length(); i++)
			drivenPlugs.append(drivenShapePlugs[i]);
	}

	//Import direct animation data
	for (U32 i = 0; i < drivenPlugs.length(); i++)
	{
		std::string drivenPlugName = drivenPlugs[i].name().asChar();

		Aftk::I_Object* pConnectedObject = 
			C_ConnectedDgImport::ImportDirectlyConnectedPlug(
				pImporter,
				drivenPlugs[i],
				status);
			AFTK_CHECK_ERROR_RETURN;
		if (!pConnectedObject)
			continue;
	
		std::string basePlugName;
		StripNodeNameFromPlugName(drivenPlugName, basePlugName);
		
		//Examine static and dynamic plugs to see if we've an existing connection point.
		Aftk::I_Plug* pAftkTargetPlug = C_ConnectedDgImport::FindPlug(
			pAftkSceneNode, 
			basePlugName);

		if (!pAftkTargetPlug)
			continue;

		//Set top level plug connection.
		pAftkTargetPlug->SetSourceConnectionPtr(pConnectedObject);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneNodeImport::FindPlug(
	Aftk::C_SceneNode* pAftkSceneNode,
	const char* pPlugName)
{
	if (!strcmp(pPlugName, "translate"))
		return &pAftkSceneNode->GetTranslatePlug();
	if (!strcmp(pPlugName, "translateX"))
		return pAftkSceneNode->GetTranslatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "translateY"))
		return pAftkSceneNode->GetTranslatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "translateZ"))
		return pAftkSceneNode->GetTranslatePlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "scale"))
		return &pAftkSceneNode->GetScalePlug();
	if (!strcmp(pPlugName, "scaleX"))
		return pAftkSceneNode->GetScalePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "scaleY"))
		return pAftkSceneNode->GetScalePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "scaleZ"))
		return pAftkSceneNode->GetScalePlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "rotate"))
		return &pAftkSceneNode->GetRotatePlug();
	if (!strcmp(pPlugName, "rotateX"))
		return pAftkSceneNode->GetRotatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "rotateY"))
		return pAftkSceneNode->GetRotatePlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "rotateZ"))
		return pAftkSceneNode->GetRotatePlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "shear"))
		return &pAftkSceneNode->GetShearPlug();
	if (!strcmp(pPlugName, "shearX"))
		return pAftkSceneNode->GetTranslatePlug().GetChildPtr(0);
	if (!strcmp(pPlugName, "shearY"))
		return pAftkSceneNode->GetShearPlug().GetChildPtr(1);
	if (!strcmp(pPlugName, "shearZ"))
		return pAftkSceneNode->GetShearPlug().GetChildPtr(2);

	if (!strcmp(pPlugName, "worldMatrix[0]"))
		return &pAftkSceneNode->GetWorldMatrixPlug();

	//Check to see if the given plug name matches any of our dynamic plugs.
	for (U32 i = 0; i < pAftkSceneNode->GetNumDynamicPlugs(); i++)
	{
		Aftk::I_Plug* pPlug = pAftkSceneNode->GetDynamicPlugPtr(i);
		if (strstr(pPlug->GetName().c_str(), pPlugName))
			return pPlug;
	}

	return 0;
}