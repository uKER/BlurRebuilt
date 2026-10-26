#include "stdafx.h"
#include "MayaIoSceneConstraintImport.h"

using namespace MayaIo;

Aftk::C_Status C_SceneConstraintImport::Import(
	C_Importer* pImporter,
	const MDagPath &transformDagPath,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;
	//Import the base AFTK scene node
	status = C_SceneNodeImport::Import(
		pImporter,
		transformDagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	Aftk::C_SceneConstraintNode* pAftkSceneConstraintNode = 
		(Aftk::C_SceneConstraintNode*)pAftkSceneNode;

	MFnDagNode fnTransform(transformDagPath, &mStatus);
	MDagPath shapeDagPath = transformDagPath;
	if (!mStatus || !shapeDagPath.extendToShape())
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
	MFnDagNode fnShape(shapeDagPath, &mStatus);
	CHECK_ERROR_RETURN;

	{
		MPlug attrPlug = fnShape.findPlug("constraintType", &mStatus);
		if (mStatus)
		{
			Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
			assert(pAftkPlug);
			pAftkSceneNode->AddPlugPtr(pAftkPlug);

			status = MayaPlugToAftkDynamicPlug(
				pImporter,
				attrPlug, 
				*pAftkPlug);
			CHECK_ERROR_RETURN;
		}
	}
	{
		MPlug attrPlug = fnShape.findPlug("axis", &mStatus);
		if (mStatus)
		{
			Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
			assert(pAftkPlug);
			pAftkSceneNode->AddPlugPtr(pAftkPlug);

			status = MayaPlugToAftkDynamicPlug(
				pImporter,
				attrPlug, 
				*pAftkPlug);
			CHECK_ERROR_RETURN;
		}
	}

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Constraint " << fnTransform.name().asChar() << "\n" AFTK_DO_DEBUG_END;
	#endif

	return Aftk::C_Status::E_Status_Success;
}

Aftk::I_Plug* C_SceneConstraintImport::FindPlug(
	Aftk::C_SceneNode* pAftkNode,
	const char* pPlugName)
{
	return C_SceneNodeImport::FindPlug(
		pAftkNode, 
		pPlugName);
}

Aftk::C_Status C_SceneConstraintImport::ResolveConnectionsSceneWide(	
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	Aftk::C_SceneNodeIterator it(pAftkScene);

	for (; it; it++)
	{
		Aftk::C_SceneNode* pAftkSceneNode = it;
		if (pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneConstraint)
		{
			Aftk::C_SceneConstraintNode* pConstraint = (Aftk::C_SceneConstraintNode*)pAftkSceneNode;

			MDagPath transformDagPath;
			if (!pImporter->GetAftkMayaMapping().GetMayaDagPathFromAftkSceneNode(
				pAftkSceneNode, 
				&transformDagPath))
				continue;

			MFnDagNode fnTransform(transformDagPath, &mStatus);
			MDagPath shapeDagPath = transformDagPath;
			if (!mStatus || !shapeDagPath.extendToShape())
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
			MFnDagNode fnShape(shapeDagPath, &mStatus);
			CHECK_ERROR_RETURN;

			{
				MPlug attrPlug = fnShape.findPlug("inWorldMatrix1", &mStatus);
				if (mStatus)
				{
					Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
					assert(pAftkPlug);
					pAftkSceneNode->AddPlugPtr(pAftkPlug);

					status = MayaPlugToAftkDynamicPlug(
						pImporter,
						attrPlug, 
						*pAftkPlug);
					CHECK_ERROR_RETURN;
				}
			}
			{
				MPlug attrPlug = fnShape.findPlug("inWorldMatrix2", &mStatus);
				if (mStatus)
				{
					Aftk::C_DynamicPlug* pAftkPlug = new Aftk::C_DynamicPlug();
					assert(pAftkPlug);
					pAftkSceneNode->AddPlugPtr(pAftkPlug);

					status = MayaPlugToAftkDynamicPlug(
						pImporter,
						attrPlug, 
						*pAftkPlug);
					CHECK_ERROR_RETURN;
				}
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

