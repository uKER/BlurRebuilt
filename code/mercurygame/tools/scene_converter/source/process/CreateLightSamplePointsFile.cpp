#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>
#include <converter_shared/lux_interface/LuxSampleList.h>

#include <data/DataTypes.h>
#include <data/ConversionOptions.h>
#include <data/AftkSceneData.h>
#include <process/CreateLightSamplePointsFile.h>
#include <spatial_hierarchy\SpatialDividerHeaders.h>
#include <Polyflags.h>
#include <converter_shared/helpers/AftkHelpers.h>

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

int DoesPolyNodeNeedLightSamples(Aftk::C_SceneNode *pNode, void *pContext, int iTriangle)
{
	if (!pNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return 0;
	
	const Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;

	const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iTriangle);

	Polyflags::C_Polyflags polyFlags( g_pInputConversionOptions->GetPolyflagConfig(), pPoly );

	// NoLightSamples has priority
	if (polyFlags.GetValue(E_Polyflag_NoLightSamples))
		return 0;

	// if collideable or forcelightsamples is set
	if (polyFlags.GetValue(E_Polyflag_CollideFlags) || polyFlags.GetValue(E_Polyflag_ForceLightSamples))
		return 1;
	else
		return 0;
}

int DoesPolyNodeNeedHighDensityLightSamples(Aftk::C_SceneNode *pNode, void *pContext, int iTriangle)
{
	if (!pNode->HasType(Aftk::E_ObjType_ISceneMesh))
		return 0;

	const Aftk::I_SceneMeshNode *pMesh = (Aftk::I_SceneMeshNode*)pNode;

	const Aftk::C_Polygon *pPoly = &pMesh->GetPolygon(iTriangle);

	Polyflags::C_Polyflags polyFlags( g_pInputConversionOptions->GetPolyflagConfig(), pPoly );

	if (polyFlags.GetValue(E_Polyflag_LowDensityLightSamples))
		return 0;

	return DoesPolyNodeNeedLightSamples(pNode, pContext, iTriangle);
}

int DoesPolyNodeNeedLowDensityLightSamples(Aftk::C_SceneNode *pNode, void *pContext, int iTriangle)
{
	return DoesPolyNodeNeedLightSamples(pNode, pContext, iTriangle);
}

bool DoesObjectNodeNeedLightSamples(Aftk::C_SceneNode *pAftkRefNode)
{
	assert(g_pInputConversionOptions);
	Objectflags::C_Objectflags objectFlags( g_pInputConversionOptions->GetObjectflagConfig(), pAftkRefNode );

	if( objectFlags.GetValue(E_Objectflag_NoLightSamples))
		return false;

	return true;
}

bool C_ProcessCreateLightSamplePointsFile::AddSamplePoints( LuxSampleList::C_SampleList* pSampleList )
{
	const C_DataConversionOptions* pInputConversionOptions = (C_DataConversionOptions*)GetInput( E_GraphData_ConversionOptions );
	BIZ_ASSERT(pInputConversionOptions);

	C_DataAftkScene* pInputPreprocessedAftk = (C_DataAftkScene*)GetInput( E_GraphData_PreprocessedAftk );
	BIZ_ASSERT(pInputPreprocessedAftk);
	if (!pInputPreprocessedAftk->GetAftkScene().GetRootSceneNodePtr())
		return true;

	//Add one and return
	//pSampleList->AddSamplePoint( Maths::HVec3( 0.0f, 0.0f, 0.0f ) );
	//return true;

	
	using namespace Maths;

	// TODO : Add sample points in a sensible way

	/*for (S32 x = -10; x < 10; x++)
		for (S32 y = -10; y < 10; y++)
			for (S32 z = -5; z < 5; z++)
				pSampleList->AddSamplePoint( Maths::HVec3( x, y, z ) );*/

	SD::SpatialHierarchy lowDensityCollision;
	SD::SpatialHierarchy highDensityCollision;

	highDensityCollision.SetCallbackFunction(DoesObjectNodeNeedLightSamples);
	highDensityCollision.SetTriangleCallbackFunction(DoesPolyNodeNeedHighDensityLightSamples, NULL);
	highDensityCollision.Build(&pInputPreprocessedAftk->GetAftkScene());

	lowDensityCollision.SetCallbackFunction(DoesObjectNodeNeedLightSamples);
	lowDensityCollision.SetTriangleCallbackFunction(DoesPolyNodeNeedLowDensityLightSamples, NULL);
	lowDensityCollision.Build(&pInputPreprocessedAftk->GetAftkScene());

	float fGridSpacing = pInputConversionOptions->GetLightingOptions().GetLightSampleGridSpacing();	// place a samplepoint every _n_ meters
	float fMinSamplePointYSpacing = pInputConversionOptions->GetToolConfig().GetLightSampleYSpacing();	// minimum amount of height between two samplepoints
	float fSamplePointHeightOffset = pInputConversionOptions->GetLightingOptions().GetLightSampleHeightOffset(); // height of the samplepoints above the ground
	S32 lightSampleMaxCount = pInputConversionOptions->GetToolConfig().GetLightSampleMaxCount();

	C_AxisBox lowDensityBbox = lowDensityCollision.GetBoundingBox();
	C_AxisBox highDensityBbox = highDensityCollision.GetBoundingBox();
	C_AxisBox bbox;
	bbox.SetInvalidExtents();
	bbox.UpdateExtents(lowDensityBbox.Min());
	bbox.UpdateExtents(lowDensityBbox.Max());
	bbox.UpdateExtents(highDensityBbox.Min());
	bbox.UpdateExtents(highDensityBbox.Max());

	S32 count = 0;
	HVec3 vCurPos;
	S32 iX, iZ;
	for (vCurPos.X(bbox.Min().X()), iX = 0; vCurPos.X() < bbox.Max().X(); vCurPos.X(vCurPos.X()+fGridSpacing), iX++)
	{
		for (vCurPos.Z(bbox.Min().Z()), iZ = 0; vCurPos.Z() < bbox.Max().Z(); vCurPos.Z(vCurPos.Z()+fGridSpacing), iZ++)
		{
			float fCurSpacing = fMinSamplePointYSpacing;
			for (vCurPos.Y(bbox.Max().Y()+fMinSamplePointYSpacing); vCurPos.Y() > (bbox.Min().Y()+1.f); )
			{
				// trace a ray down from here, and generate a samplepoint at the hit position
				SD::Ray ray;
				ray.bShadowRay = false;
				ray.SetRay(vCurPos, HVec3(0,-1.f,0));

				SD::RayTraceResult result;

				bool bIsHighDensitySample = false;
				if( (( iX % 4 ) != 0) || (( iZ % 4 ) != 0) )
				{
					bIsHighDensitySample = true;
				}

				if(bIsHighDensitySample)
					highDensityCollision.RayTrace(ray, result);
				else
					lowDensityCollision.RayTrace(ray, result);

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
						count++;
						if (count >= lightSampleMaxCount)
						{
							LOGWARN(Tools::g_log, "Light samples count is bigger than max count defined in tools config file(%i), some of the light samples will be discarded", lightSampleMaxCount);
							return true;
						}
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