#include "StdAfx.h"

#include "../XsiIoUtility.h"
#include "XsiIoSceneNodeExport.h"
#include "XsiIoSceneExport.h"
#include "XsiIoAnimationCurveExport.h"

using namespace XsiIo;

Aftk::C_Status C_SceneNodeExport::Export(
	C_SceneExport *pExporter,
	CSLModel *pXsiNode,
	const Aftk::C_Scene *pAftkScene,
	const Aftk::C_SceneNode *pAftkSceneNode)
{
	Aftk::C_Status status;

	AFTK_ASSERT( pExporter );
	AFTK_ASSERT( pXsiNode );
	AFTK_ASSERT( pAftkScene );
	AFTK_ASSERT( pAftkSceneNode );

	pXsiNode->Name().SetText( pAftkSceneNode->GetName().c_str() );

	status = pExporter->AddToAftkXsiSceneNodeMapping( pAftkSceneNode, pXsiNode );
	AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;

	XsiMatrix basePoseMatrix, localMatrix;

	basePoseMatrix = (XsiMatrix&) ( pAftkSceneNode->GetBasePose() );

	CSLTransform* pBasePose = pXsiNode->CreateBasePose();
	if( !pBasePose )
		return Aftk::C_Status( Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi base pose" );
	pBasePose->SetMatrix( basePoseMatrix );
	pBasePose->ComputeGlobalMatrix();

	{
		// Even though we've just set the basepose matrix, XSI insists on writing the rotation vector in radians.
		// Get rotation vector back from XSI and convert it into degrees
		CSIBCVector3D rotation = pBasePose->GetEulerRotation();

		pBasePose->SetEulerRotation( CSIBCVector3D( Maths::RadToDeg( rotation.GetX() ),
													Maths::RadToDeg( rotation.GetY() ),
													Maths::RadToDeg( rotation.GetZ() ) ) );
	}

	localMatrix = (XsiMatrix&) ( pAftkSceneNode->GetLocalMatrix() );

	CSLTransform *pTransform = pXsiNode->Transform();
	pTransform->SetMatrix( localMatrix );
	pTransform->ComputeGlobalMatrix();

	{
		CSIBCVector3D rotation = pTransform->GetEulerRotation();
		pTransform->SetEulerRotation( CSIBCVector3D( Maths::RadToDeg( rotation.GetX() ),
													 Maths::RadToDeg( rotation.GetY() ),
													 Maths::RadToDeg( rotation.GetZ() ) ) );
	}

#if 0
	CSLVisibility * visibility = pXsiNode->Visibility();
	visibility->SetVisibility( 1 );
#endif

	U32 numPlugs=pAftkSceneNode->GetNumDynamicPlugs();
	if (numPlugs>0)
	{
		for (U32 i=0; i<numPlugs; ++i)
		{
			const Aftk::C_DynamicPlug *pAftkDynamicPlug = pAftkSceneNode->GetDynamicPlugPtr(i);

			CSLCustomPSet *pXsiParamSet = pXsiNode->AddCustomPSet();
			if (!pXsiParamSet)
				return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi custom parameter set");

			status = AftkDynamicPlugToXsiCustomPSet(pExporter, pXsiNode, pAftkSceneNode, pXsiParamSet, pAftkDynamicPlug);
			if (!status)
				return status;
		}
	}
	return Aftk::C_Status::E_Status_Success;
}


Aftk::C_Status C_SceneNodeExport::AftkDynamicPlugToXsiCustomPSet(
	C_SceneExport* pExporter,
	CSLModel *pXsiNode,
	const Aftk::C_SceneNode* pAftkSceneNode,
	CSLCustomPSet *pXsiParamSet,
	const Aftk::I_Plug* pAftkPlug)
{

	Aftk::C_Status status;
	std::string plugName = pAftkPlug->GetName();
	pXsiParamSet->SetName((char*)plugName.c_str());


	if (pAftkPlug->IsCompound())
	{
		for (U32 i=0; i<pAftkPlug->GetNumChildren(); ++i)
		{
			const Aftk::I_Plug *pAftkChildPlug = pAftkPlug->GetChildPtr(i);

			CSLVariantParameter *pXsiParam = pXsiParamSet->AddParameter();
			if (!pXsiParam)
				return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi custom parameter");

			ExportParameter(pExporter, pXsiNode, pAftkSceneNode, pXsiParamSet, pXsiParam, pAftkChildPlug);
		}
		if (pXsiParamSet->GetParameterCount() == 0)
			pXsiNode->RemoveCustomPSet(pXsiParamSet);
	}
	else
	{
		//we've lost the parameter set -> parameter relationship, probably after going into maya
		//Put the parameter into its own parameter set
		CSLVariantParameter *pXsiParam = pXsiParamSet->AddParameter();
		if (!pXsiParam)
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi custom parameter");

		ExportParameter(pExporter, pXsiNode, pAftkSceneNode, pXsiParamSet, pXsiParam, pAftkPlug);
		if (pXsiParamSet->GetParameterCount() == 0)
			pXsiNode->RemoveCustomPSet(pXsiParamSet);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneNodeExport::ExportParameter(
	C_SceneExport* pExporter,
	CSLModel *pXsiNode,
	const Aftk::C_SceneNode* pAftkSceneNode,
	CSLCustomPSet *pXsiParamSet,
	CSLVariantParameter *pXsiParam,
	const Aftk::I_Plug* pAftkPlug)
{

	Aftk::C_Status status;

	pXsiParam->SetName((char*)pAftkPlug->GetName().c_str());

	const Aftk::I_Object *pSourceConnection = pAftkPlug->GetSourceConnectionPtr();
	if (pSourceConnection)
	{
		if (pSourceConnection->GetType() != Aftk::E_ObjType_AnimationCurve)
		{
			//AFTK_DO_BEGIN << "Custom parameter " << pAftkDynamicPlug->GetName() << " is not an animation curve and will not be exported." AFTK_DO_WARNING_END;
			pXsiParamSet->RemoveParameter(pXsiParam);
			return Aftk::C_Status::E_Status_Success;
		}

		Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pSourceConnection;

		CSLTemplate::EFCurveInterpolationType eInterpolationType =
			( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Linear ) ? CSLTemplate::SI_LINEAR :
			( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Hermite ) ? CSLTemplate::SI_HERMITE :
			CSLTemplate::SI_LINEAR;

		CSLFCurve *pXsiFCurve = pXsiParamSet->CreateParameterFCurve((char*)pAftkPlug->GetName().c_str(), eInterpolationType);
		pXsiFCurve->ConnectParameter(pXsiParam);
		if (!pXsiFCurve)
			return Aftk::C_Status(Aftk::C_Status::E_Status_OutOfMemory, "Failed to create Xsi custom parameter curve");

		status = C_AnimationCurveExport::Export(pExporter, pXsiFCurve, pAftkAnimCurve, pAftkSceneNode);
		if (!status)
			return status;

		SI_TinyVariant variantValue;
		variantValue.variantType = SI_VT_FLOAT;
		variantValue.fVal = 0.0f;
		pXsiParam->SetValue(&variantValue);
	}
	else
	{
		//Other attributes if/when required
		pXsiParamSet->RemoveParameter(pXsiParam);
	}

	return Aftk::C_Status::E_Status_Success;
}