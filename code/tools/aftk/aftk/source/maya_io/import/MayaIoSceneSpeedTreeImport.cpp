#include "stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_SceneSpeedTreeImport::Import(
	C_Importer* pImporter,
	const MDagPath &dagPath,
	const MObject& speedTreeNodeConst,
	const Aftk::Matrix44* pExclusiveMatrix,
	Aftk::C_Scene* pAftkScene,
	Aftk::C_SceneNode* pAftkSceneNode,
	const C_ImportOptions* pOptions)
{
	Aftk::C_Status status;
	MStatus mStatus;

	status = C_SceneNodeImport::Import(
		pImporter,
		dagPath,
		pExclusiveMatrix,
		pAftkScene,
		pAftkSceneNode,
		true,
		pOptions);
	AFTK_CHECK_ERROR_RETURN;

	MDagPath shapeDagPath = dagPath;

	if (!shapeDagPath.extendToShape())
		AFTK_DO_BEGIN << "Failed to extend to shape." AFTK_DO_ERROR_RETURN_END_1;

	// Create the appropriate shape node type
	MFnDagNode mFnShapeDagNode(shapeDagPath, &mStatus);

	MFnDependencyNode speedTreeNode(speedTreeNodeConst);

	C_PlugImportFilter plugImportFilter;
	Aftk::C_BlindDataNode bd;

	if (pAftkSceneNode->GetBlindDataResourcePtr())
	{
		bd = pAftkSceneNode->GetBlindDataResourcePtr()->GetContent();
	}

	{
		Aftk::C_GenericAttribute genericAttr;
		genericAttr.SetName("SPTData");
		bd.AddAttribute(genericAttr);
	}

	for (U32 attrPhysicalIdx = 0; attrPhysicalIdx < speedTreeNode.attributeCount(); attrPhysicalIdx++)
	{
		Aftk::C_GenericAttribute genericAttr;
		MPlug attrPlug(
			speedTreeNodeConst, 
speedTreeNode.attribute(attrPhysicalIdx, &mStatus));
		AFTK_CHECK_BEGIN << TRANS_ACQUIRE_PLUG_FAIL AFTK_CHECK_ERROR_RETURN_END_1;

		std::string strippedPlugName;
		StripNodeNameFromPlugName(attrPlug.name().asChar(), strippedPlugName);

		if (strippedPlugName == "Seed")
		{
			int attrInt;
			attrPlug.getValue(attrInt);
			genericAttr.SetName("Seed");
			genericAttr.SetValue(attrInt);
			bd.AddAttribute(genericAttr);
			#ifdef _DEBUG
			AFTK_DO_BEGIN  << genericAttr.GetName() <<  "Int dyn attr. " << attrInt << "\n" AFTK_DO_DEBUG_END;
			#endif
		}
		else if (strippedPlugName == "Size")
		{
			F32 attrF;
			attrPlug.getValue(attrF);
			genericAttr.SetName(strippedPlugName);
			genericAttr.SetValue(attrF);
			bd.AddAttribute(genericAttr);
			#ifdef _DEBUG
			AFTK_DO_BEGIN  << genericAttr.GetName() <<  "F dyn attr. " << attrF << "\n" AFTK_DO_DEBUG_END;
			#endif
		}
		else if (strippedPlugName == "SizeVariance")
		{
			F32 attrF;
			attrPlug.getValue(attrF);
			genericAttr.SetName(strippedPlugName);
			genericAttr.SetValue(attrF);
			bd.AddAttribute(genericAttr);
			#ifdef _DEBUG
			AFTK_DO_BEGIN  << genericAttr.GetName() <<  "F dyn attr. " << attrF << "\n" AFTK_DO_DEBUG_END;
			#endif
		}	
		else if (strippedPlugName == "DrawAsSpike")
		{
			int attrInt;
			attrPlug.getValue(attrInt);
			genericAttr.SetName(strippedPlugName);
			if (attrInt == 0)
			{
				genericAttr.SetValue(bool(false));
			}
			else
			{
				genericAttr.SetValue(bool(true));
			}
			bd.AddAttribute(genericAttr);
			#ifdef _DEBUG
			AFTK_DO_BEGIN << genericAttr.GetName() << " Int dyn attr. " << attrInt << "\n" AFTK_DO_DEBUG_END;
			#endif
		}
		else if (strippedPlugName == "Filename")
		{
			MString attrString;
			attrPlug.getValue(attrString);
			std::string stdString = attrString.asChar();
			genericAttr.SetName(strippedPlugName);
			genericAttr.SetValue(stdString);
			#ifdef _DEBUG
			AFTK_DO_BEGIN  << genericAttr.GetName() <<  "String dyn attr. " << stdString << "\n" AFTK_DO_DEBUG_END;
			#endif
			bd.AddAttribute(genericAttr);
		}
		else
		{
			if (plugImportFilter.Find(strippedPlugName))
			{
				continue;
			}
			if (
				(strippedPlugName == "CameraRotation")
				|| 
				(strippedPlugName == "CameraRotation0")
				|| 
				(strippedPlugName == "CameraRotation1")
				|| 
				(strippedPlugName == "CameraRotation2")
				|| 
				(strippedPlugName == "SavedSPTData")
				|| 
				(strippedPlugName == "OutputMesh")
				)
			{
				continue;
			}
			//Export any attributes we don't expect
			MayaPlugToAftkGenericAttribute(attrPlug, genericAttr);
			bd.AddAttribute(genericAttr);
		}
	}

	const Aftk::C_SharedResource< Aftk::C_BlindDataNode >* pBlindData;
	pBlindData = pAftkScene->AddBlindDataNode(bd);
	pAftkSceneNode->SetBlindDataResourcePtr(pBlindData);

	return Aftk::C_Status::E_Status_Success;
}

