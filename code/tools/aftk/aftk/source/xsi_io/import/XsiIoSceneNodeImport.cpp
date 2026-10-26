#include "stdafx.h"

#include "../XsiIoUtility.h"

#include "XsiIoSceneImport.h"
#include "XsiIoSceneNodeImport.h"
#include "XsiIoAnimationCurveImport.h"

#include <Aftk/AftkEnum.h>

using namespace XsiIo;

void SetAftkDynamicPlugValueFromXsiVariant(
	SI_TinyVariant *pVariant,
	Aftk::C_DynamicPlug *pAftkDynamicPlug)
{
	switch(pVariant->variantType)
	{
	case SI_VT_BOOL:
		pAftkDynamicPlug->SetValue((pVariant->boolVal==0) ? false : true);
		break;
	case SI_VT_BYTE:
		pAftkDynamicPlug->SetValue(*((S8*)(&pVariant->bVal)));
		break;
	case SI_VT_UBYTE:
		pAftkDynamicPlug->SetValue(pVariant->ubVal);
		break;
	case SI_VT_SHORT:
		pAftkDynamicPlug->SetValue(pVariant->sVal);
		break;
	case SI_VT_USHORT:
		pAftkDynamicPlug->SetValue(pVariant->usVal);
		break;
	case SI_VT_INT:
	case SI_VT_LONG:
		pAftkDynamicPlug->SetValue(pVariant->nVal);
		break;
	case SI_VT_UINT:
	case SI_VT_ULONG:
		pAftkDynamicPlug->SetValue(pVariant->unVal);
		break;
	case SI_VT_FLOAT:
		pAftkDynamicPlug->SetValue(pVariant->fVal);
		break;
	case SI_VT_DOUBLE:
		pAftkDynamicPlug->SetValue(pVariant->dVal);
		break;
	default:
		break;
	}
}


Aftk::C_Status C_SceneNodeImport::Import(		
	C_SceneImport* pSceneImport,
	CSLModel* pXsiNode, 
	Aftk::C_SceneNode* pAftkSceneNode)
{
	Aftk::C_Status status;
	pAftkSceneNode->SetName(pXsiNode->Name().GetText());

	if (pXsiNode->Transform())
	{
		XsiMatrix m = (XsiMatrix&) pXsiNode->Transform()->GetMatrix();
		pAftkSceneNode->SetLocalMatrix(m);
	}
	if (pXsiNode->GetBasePose())
	{
		XsiMatrix m = (XsiMatrix&) pXsiNode->GetBasePose()->GetMatrix();
		pAftkSceneNode->SetBasePose(m);
	}

	if (pXsiNode->GetCustomPSetCount())
	{
		CSLCustomPSet ** pXsiParamSetList = pXsiNode->GetCustomPSetList();
		for (U32 i=0; i<(U32)pXsiNode->GetCustomPSetCount(); ++i)
		{
			CSLCustomPSet *pXsiParamSet = pXsiParamSetList[i];

			//Add a new dynamic attribute(plug) to our scene node.
			Aftk::C_DynamicPlug* pAftkDynamicPlug = new Aftk::C_DynamicPlug();
			if (!pAftkDynamicPlug)
				return Aftk::C_Status::E_Status_OutOfMemory;

			pAftkSceneNode->AddPlugPtr(pAftkDynamicPlug);
			status = XsiCustomPSetToAftkDynamicPlug(pSceneImport, pAftkSceneNode, pXsiParamSet, pAftkDynamicPlug);
			if (!status)
				return status;

			//Clean up if no attributes of the set have been exported
			if (pAftkDynamicPlug->GetNumChildren() == 0)
				pAftkSceneNode->DeletePlug(pAftkDynamicPlug->GetLogicalIndex());
		}
	}

	status = pSceneImport->AddToAftkXsiSceneNodeMapping(pAftkSceneNode, pXsiNode);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
	status = pSceneImport->AddToXsiAftkSceneNodeMapping(pXsiNode, pAftkSceneNode);
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_SceneNodeImport::XsiCustomPSetToAftkDynamicPlug(
	C_SceneImport *pSceneImport,
	Aftk::C_SceneNode* pAftkSceneNode,
	CSLCustomPSet *pXsiParamSet,
	Aftk::C_DynamicPlug *pAftkDynamicPlug)
{

	std::string plugName(pXsiParamSet->GetName());
	{
		std::size_t separator = plugName.rfind(".");
		if (separator != std::string::npos)
			plugName = plugName.substr(separator+1);
	}
	pAftkDynamicPlug->SetName(plugName);

	CSLVariantParameter **pXsiParamList = pXsiParamSet->GetParameterList();
	for (U32 i=0; i<(U32)pXsiParamSet->GetParameterCount(); ++i)
	{
		CSLVariantParameter *pXsiParam = pXsiParamList[i];

		CSLFCurve *pCurve = pXsiParamSet->GetParameterFCurve(pXsiParam->GetName());
		if (pCurve)
		{
			//Add a new child dynamic attribute
			Aftk::C_DynamicPlug* pAftkChildDynamicPlug = new Aftk::C_DynamicPlug();
			if (!pAftkDynamicPlug)
				return Aftk::C_Status::E_Status_OutOfMemory;

			pAftkDynamicPlug->AddChildPtr(pAftkChildDynamicPlug);
			pAftkChildDynamicPlug->SetName(pXsiParam->GetName());

			//Create a new anim curve in our scene.
			Aftk::C_AnimationCurveNode* pAftkAnimCurve = 
				new Aftk::C_AnimationCurveNode(pAftkSceneNode->GetScenePtr());

			//Do the import here ... may want to perform this step together with the import of other curves
			C_AnimationCurveImport::Import(pSceneImport, pCurve, pAftkAnimCurve);
			pAftkChildDynamicPlug->SetSourceConnectionPtr(pAftkAnimCurve);
			pAftkAnimCurve->SetName(pXsiParam->GetName());
		}
		else
		{
			//No curve ... simply set value
			//SI_TinyVariant *pVariant = pXsiParam->GetValue();
			//SetAftkDynamicPlugValueFromXsiVariant(pVariant, pAftkChildDynamicPlug);
		}
	}

	return Aftk::C_Status::E_Status_Success;
}