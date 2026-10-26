#include "StdAfx.h"

#include "XsiIoAnimationExport.h"
#include "XsiIoSceneExport.h"
#include "XsiIoAnimationCurveExport.h"

using namespace XsiIo;

Aftk::C_Status C_AnimationExport::Export(
	C_SceneExport* pExporter,
	CSLModel* pXsiNode,
	const Aftk::C_Scene* pAftkScene,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
/*
	U32 nCurves = pAftkScene->GetNumDgNodes( Aftk::E_ObjType_AnimationCurve );
	if( nCurves == 0 )
		return Aftk::C_Status::E_Status_Success;
*/
	CSLTransform *pXsiTransform = pXsiNode->Transform();
	// Translation FCurve
	{
		for( U32 i=0; i<3; ++i )
		{
			const Aftk::I_Plug *pAftkPlug = pAftkSceneNode->GetTranslatePlug().GetChildPtr( i );
			Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pAftkPlug->GetSourceConnectionPtr();

			if( !pAftkPlug || !pAftkAnimCurve )
				continue;

			CSLTemplate::EFCurveInterpolationType eInterpolationType =
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Linear ) ? CSLTemplate::SI_LINEAR :
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Hermite ) ? CSLTemplate::SI_HERMITE :
				CSLTemplate::SI_LINEAR;

			CSLFCurve *pXsiFCurve = pXsiTransform->CreateFCurve( (CSLTemplate::EFCurveType) (CSLTemplate::SI_TRANSLATION_X + i), eInterpolationType );
			if( !pXsiFCurve )
				return Aftk::C_Status( Aftk::C_Status::E_Status_Success, "Failed to create animation curve" );

			Aftk::C_Status status = C_AnimationCurveExport::Export( pExporter, pXsiFCurve, pAftkAnimCurve, pAftkSceneNode );
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
		}
	}

	// Rotation FCurve
	{
		for( U32 i=0; i<3; ++i )
		{
			const Aftk::I_Plug *pAftkPlug = pAftkSceneNode->GetRotatePlug().GetChildPtr( i );
			Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pAftkPlug->GetSourceConnectionPtr();

			if( !pAftkPlug || !pAftkAnimCurve )
				continue;

			CSLTemplate::EFCurveInterpolationType eInterpolationType =
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Linear ) ? CSLTemplate::SI_LINEAR :
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Hermite ) ? CSLTemplate::SI_HERMITE :
				CSLTemplate::SI_LINEAR;

			CSLFCurve *pXsiFCurve = pXsiTransform->CreateFCurve( (CSLTemplate::EFCurveType) (CSLTemplate::SI_ROTATION_X + i), eInterpolationType );
			if( !pXsiFCurve )
				return Aftk::C_Status( Aftk::C_Status::E_Status_Success, "Failed to create animation curve" );

			Aftk::C_Status status = C_AnimationCurveExport::Export( pExporter, pXsiFCurve, pAftkAnimCurve, pAftkSceneNode );
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
		}
	}

	// Scale FCurve
	{
		for( U32 i=0; i<3; ++i )
		{
			const Aftk::I_Plug *pAftkPlug = pAftkSceneNode->GetScalePlug().GetChildPtr( i );
			Aftk::C_AnimationCurveNode *pAftkAnimCurve = (Aftk::C_AnimationCurveNode*) pAftkPlug->GetSourceConnectionPtr();

			if( !pAftkPlug || !pAftkAnimCurve )
				continue;

			CSLTemplate::EFCurveInterpolationType eInterpolationType =
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Linear ) ? CSLTemplate::SI_LINEAR :
				( pAftkAnimCurve->GetCurveType() == Aftk::E_AnimCurveType_Hermite ) ? CSLTemplate::SI_HERMITE :
				CSLTemplate::SI_LINEAR;

			CSLFCurve *pXsiFCurve = pXsiTransform->CreateFCurve( (CSLTemplate::EFCurveType) (CSLTemplate::SI_SCALING_X + i), eInterpolationType );
			if( !pXsiFCurve )
				return Aftk::C_Status( Aftk::C_Status::E_Status_Success, "Failed to create animation curve" );

			Aftk::C_Status status = C_AnimationCurveExport::Export( pExporter, pXsiFCurve, pAftkAnimCurve, pAftkSceneNode );
			AFTK_CHECK_BEGIN AFTK_CHECK_ERROR_RETURN_END;
		}
	}

	return Aftk::C_Status::E_Status_Success;
}