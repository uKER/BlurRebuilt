#include "stdafx.h"
#include "XsiIoAnimationCurveExport.h"
#include "XsiIoSceneExport.h"

using namespace XsiIo;

Aftk::C_Status C_AnimationCurveExport::Export(
	C_SceneExport* pExporter,
	CSLFCurve* pXsiAnimCurve,
	const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	switch( pXsiAnimCurve->GetInterpolationType() )
	{
		case CSLTemplate::SI_HERMITE:
			return ExportHermiteCurve( pExporter, pXsiAnimCurve, pAftkAnimCurve, pAftkSceneNode );
		case CSLTemplate::SI_LINEAR:
		default:
			return ExportLinearCurve( pExporter, pXsiAnimCurve, pAftkAnimCurve, pAftkSceneNode );
	}
}

Aftk::C_Status C_AnimationCurveExport::ExportLinearCurve(
	C_SceneExport* pExporter,
	CSLFCurve* pXsiAnimCurve,
	const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
	U32 nKeys = pAftkAnimCurve->GetNumKeys();
	CSLBaseFCurve::CSLLinearKeyArray *pXsiLinearKeyList = pXsiAnimCurve->GetLinearKeyList();
	if( !pXsiLinearKeyList )
		return Aftk::C_Status( Aftk::C_Status::E_Status_Failure, "Failed to get list of xsi linear key frames" );

	pXsiLinearKeyList->Resize( nKeys );

	for( U32 i=0; i<nKeys; ++i )
	{
		Aftk::C_LinearKey aftkLinearKey;
		bool status = pAftkAnimCurve->GetKey( aftkLinearKey, i );
		if( !status )
			return Aftk::C_Status( Aftk::C_Status::E_Status_Failure, "Failed to get animation key frame from aftk" );

		(*pXsiLinearKeyList)[i].m_fTime = aftkLinearKey.GetX();

		if(		pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_X  ||
				pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Y  ||
				pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Z )
				(*pXsiLinearKeyList)[i].m_fValue = Maths::RadToDeg( aftkLinearKey.GetValue() );
		else
			(*pXsiLinearKeyList)[i].m_fValue = aftkLinearKey.GetValue();
	}
	return Aftk::C_Status::E_Status_Success; 
}

Aftk::C_Status C_AnimationCurveExport::ExportHermiteCurve(
	C_SceneExport* pExporter,
	CSLFCurve* pXsiAnimCurve,
	const Aftk::C_AnimationCurveNode* pAftkAnimCurve,
	const Aftk::C_SceneNode* pAftkSceneNode)
{
/*
	F32 numFrames = pExporter->GetAftkScene()->GetDuration();
	F32 frameRate = pExporter->GetAftkScene()->GetFrameRate();
	F32 frameStart = pExporter->GetAftkScene()->GetStartTime();
	F32 frameEnd = pExporter->GetAftkScene()->GetEndTime();

	F32 frameStep = numFrames/frameRate;
*/

	U32 nKeys = pAftkAnimCurve->GetNumKeys();
	CSLBaseFCurve::CSLHermiteKeyArray *pXsiHermiteKeyList = pXsiAnimCurve->GetHermiteKeyList();
	if( !pXsiHermiteKeyList )
		return Aftk::C_Status( Aftk::C_Status::E_Status_Failure, "Failed to get list of xsi hermite key frames" );

	pXsiHermiteKeyList->Resize( nKeys );

	for( U32 i=0; i<nKeys; ++i )
	{
		Aftk::C_HermiteKey aftkHermiteKey;
		bool status = pAftkAnimCurve->GetKey( aftkHermiteKey, i );
		if( !status )
			return Aftk::C_Status( Aftk::C_Status::E_Status_Failure, "Failed to get animation key frame from aftk" );

		(*pXsiHermiteKeyList)[i].m_fTime = aftkHermiteKey.GetX();

		if(		pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_X  ||
				pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Y  ||
				pXsiAnimCurve->GetFCurveType() == CSLTemplate::SI_ROTATION_Z )
			(*pXsiHermiteKeyList)[i].m_fValue = Maths::RadToDeg( aftkHermiteKey.GetValue() );
		else
			(*pXsiHermiteKeyList)[i].m_fValue = aftkHermiteKey.GetValue();
/*
		(*pXsiHermiteKeyList)[i].m_fInTangent = Maths::RadToDeg( aftkHermiteKey.GetInTangentAngle() );
		(*pXsiHermiteKeyList)[i].m_fOutTangent = Maths::RadToDeg( aftkHermiteKey.GetOutTangentAngle() );
*/
		(*pXsiHermiteKeyList)[i].m_fInTangent = tan(aftkHermiteKey.GetInTangentAngle());
		(*pXsiHermiteKeyList)[i].m_fOutTangent = tan(aftkHermiteKey.GetOutTangentAngle());
	}

	return Aftk::C_Status::E_Status_Success; 
}