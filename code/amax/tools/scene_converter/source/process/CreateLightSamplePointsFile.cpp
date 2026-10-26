#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/lux_interface/LuxSampleList.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <process/CreateLightSamplePointsFile.h>
#include <spatial_hierarchy\SpatialDividerHeaders.h>
#include <Polyflags.h>

//Global access to DosNodeNeedLighting callback
const C_DataConversionOptions* g_pInputConversionOptions;

bool C_ProcessCreateLightSamplePointsFile::Process( Progress::I_ProgressBar* pProgress )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	assert(pInputConversionOptions);

	g_pInputConversionOptions = pInputConversionOptions;

	bool success = true;

	LuxSampleList::C_SampleList sampleList;

	success = AddSamplePoints(&sampleList);
	if(!success)
		return false;

	const Util::C_String samplePointsFilename = pInputConversionOptions->GetLightSamplePointsFilename();
	success = sampleList.WriteSamplePointsFile( samplePointsFilename );
	if(!success)
		return false;

	return success;
}

bool DoesNodeNeedLighting(Aftk::C_SceneNode *pAftkRefNode)
{
	assert(g_pInputConversionOptions);
	Objectflags::C_Objectflags objectFlags( g_pInputConversionOptions->GetObjectflagConfig(), pAftkRefNode );

	if( objectFlags.GetValue(E_Objectflag_DontLight))
		return false;
	else
		return true;
}



bool C_ProcessCreateLightSamplePointsFile::AddSamplePoints( LuxSampleList::C_SampleList* pSampleList )
{
	C_DataAftkScene* pInputPreprocessedAftk = (C_DataAftkScene*)GetInput( E_GraphData_PreprocessedAftk );
	assert(pInputPreprocessedAftk);


	using namespace Maths;

	// TODO : Add sample points in a sensible way

	/*for (S32 x = -20; x < 20; x++)
		for (S32 y = -20; y < 20; y++)
			for (S32 z = -20; z < 20; z++)
				pSampleList->AddSamplePoint( Maths::HVec3( x, y, z ) );*/

	SD::SpatialHierarchy collision;

	collision.SetCallbackFunction(DoesNodeNeedLighting);
	collision.Build(&pInputPreprocessedAftk->GetAftkScene());

	float fGridSpacing = 1.0f;				// place a samplepoint every _n_ meters
	float fMinSamplePointYSpacing = 1.1f;	// minimum amount of height between two samplepoints
	float fSamplePointHeightOffset = 1.f;	// height of the samplepoints above the ground

	C_AxisBox bbox = collision.GetBoundingBox();
	HVec3 vCurPos;
	for (vCurPos.X(bbox.Min().X()); vCurPos.X() < bbox.Max().X(); vCurPos.X(vCurPos.X()+fGridSpacing))
	{
		for (vCurPos.Z(bbox.Min().Z()); vCurPos.Z() < bbox.Max().Z(); vCurPos.Z(vCurPos.Z()+fGridSpacing))
		{
			float fCurSpacing = fMinSamplePointYSpacing;
			for (vCurPos.Y(bbox.Max().Y()+fMinSamplePointYSpacing); vCurPos.Y() > (bbox.Min().Y()+1.f); )
			{
				// trace a ray down from here, and generate a samplepoint at the hit position
				SD::Ray ray;
				ray.bShadowRay = false;
				ray.SetRay(vCurPos, HVec3(0,-1.f,0));

				SD::RayTraceResult result;

				collision.RayTrace(ray, result);
				if (result.bHit)
				{
					// only put a samplepoint here if there is at least 2m of unused space above it
					fCurSpacing += result.fDistance;
					if (fCurSpacing >= fMinSamplePointYSpacing)
					{
						//						Float3 vNormal = result.GetSurfaceNormal();
						//						if (vNormal.y > 0.7f)
						//						{
						pSampleList->AddSamplePoint( HVec3(&result.vHitPos) + HVec3(0,fSamplePointHeightOffset,0) );
						//						}
					}
					fCurSpacing = 0;
					vCurPos.Y(result.vHitPos.y - fMinSamplePointYSpacing );
				}
				else
					break;
			}
		}
	}

	return true;
}

const Util::C_String C_ProcessCreateLightSamplePointsFile::GetName() const
{
	return "C_ProcessCreateLightSamplePointsFile";
}