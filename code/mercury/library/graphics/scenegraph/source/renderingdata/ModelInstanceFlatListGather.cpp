#include "stdafx.h"

#include "SceneGraph/RenderingData/ModelInstanceFlatListGather.h"
#include "SceneGraph/ViewData.h"
#include "SceneGraph/RenderingData/ModelInstanceCullNode.h"
#include "SceneGraph/Enums.h"
#include <system/perf.h>

#ifdef PLATFORM_SPU
#include "System/Debug/Log/DebugLogSPU.h"
#include "../source/maths/hvec/VectorConst.cpp"
#endif

namespace SceneGraph
{


#ifndef PLATFORM_SPU
extern Debug::Bool g_UseLightCulling;
#else
bool g_UseLightCulling = true;
#endif

void ModelInstanceFlatListGather(Scheduler::Task* pTask)
{
	PERF_EVENT("SceneGraph.ModelInstanceListGather");
	
	ModelInstanceFlatListGatherData* pData = (ModelInstanceFlatListGatherData*)Spu::FetchAlloc(pTask->Input, sizeof(ModelInstanceFlatListGatherData));
	
	// fetch things onto SPU
#ifdef PLATFORM_SPU
	for(S32 i = 0; i < pData->viewData.Size(); ++i)
		pData->viewData[i] = (ViewData*)Spu::FetchAllocAsync(pData->viewData[i], sizeof(ViewData), 0);
	Spu::Wait(0);
	for(S32 i = 0; i < pData->viewData.Size(); ++i)
	{
		if(pData->viewData[i]->pLightSpheres)
			pData->viewData[i]->pLightSpheres = (const LightSphere*)Spu::FetchAllocAsync(pData->viewData[i]->pLightSpheres, sizeof(LightSphere) * pData->viewData[i]->lightSpheresCount, 0);
	}
#endif

	ModelInstanceFlatListGatherData::Input* pInputData = &pData->input;

	U32 num[MAX_FLATLIST_VIEWS];
	ModelInstanceFlatListGatherData::Output* pOutputEA[MAX_FLATLIST_VIEWS];
	ModelInstanceFlatListGatherData::Output::Result* pResults[MAX_FLATLIST_VIEWS];

	for(S32 v = 0; v < pData->outputs.Size(); ++v)
	{
		pOutputEA[v] = pData->outputs[v];
		num[v] = 0;

	#ifdef PLATFORM_SPU
		pResults[v] = (ModelInstanceFlatListGatherData::Output::Result*)Spu::Alloc(sizeof(ModelInstanceFlatListGatherData::Output::Result) * MAX_FLATLIST_RESULTS);
	#else
		pResults[v] = pOutputEA[v]->results;
	#endif
	}

	pInputData->ppCullNodes = (const C_ModelInstanceBasicCullNode**)Spu::FetchAlloc(pInputData->ppCullNodes, (sizeof(C_ModelInstanceBasicCullNode*) * pInputData->nodesCount + 15) & ~15);
	
#ifdef PLATFORM_SPU
	// allocate memory for all the cullnodes
	C_ModelInstanceBasicCullNode* pCullNodes = (C_ModelInstanceBasicCullNode*)Spu::Alloc(sizeof(C_ModelInstanceBasicCullNode) * pInputData->nodesCount);

	// fetch all cull nodes to local memory
	for(U32 i = 0; i < pInputData->nodesCount; ++i)
	{
		pInputData->ppCullNodes[i] = (const C_ModelInstanceBasicCullNode*)Spu::FetchAsync(&pCullNodes[i], pInputData->ppCullNodes[i], sizeof(C_ModelInstanceBasicCullNode), 0);
	}
	Spu::Wait(0);
#endif

	Maths::HF32 pixelCostHalf(128.0f);
	Maths::HF32 pixelCostMax(256.0f);

	for(U32 i = 0; i < pInputData->nodesCount; ++i)
	{
		const C_ModelInstanceBasicCullNode* pCullNode = pInputData->ppCullNodes[i];
		assert(pCullNode);
	
		Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&pCullNode->centre));
		Maths::HF32	radius(pCullNode->size.w);
		Maths::HF32	farDistance(pCullNode->clip.y);

		for(S32 v = 0; v < pData->outputs.Size(); ++v)
		{
			const ViewData* pViewData = pData->viewData[v];
			const Display::C_ViewFrustum* pViewFrustum = &pViewData->viewFrustum;
			
			// flag cull
			if(pViewData->typeFlag == (pViewData->typeFlag & pCullNode->GetVisibilityFlags()))
			{
				// viewParams.X = farClip, viewParams.Y = depthScale, viewParams.Z = instanceDrawScale
				Maths::HVec4 viewParams(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pViewData->farClip));
				Maths::HF32 farClip(pViewData->farClip);
				Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&pViewData->forwardZ));
				Maths::HF32 depthScale = pViewData->depthScale;

				Maths::HF32 viewFarDistance = farDistance * (pViewData->modelInstanceDrawDistanceScale);
				viewFarDistance = Maths::Min(viewFarDistance, viewParams.X() + radius);

				// sphere cull
				if(pViewFrustum->CullSphere(centreW, radius, viewFarDistance) || 
				   !(pViewData->cullFlags & E_CullMethod_Frustum))	// Is frustum culling disabled for the view?
				{
					ModelInstanceFlatListGatherData::Output::Result& result = pResults[v][num[v]];

					result.pModelInstance = pCullNode->GetModelInstance();
					assert(result.pModelInstance);
					assert((U32)result.pModelInstance != 0xfefefefe);

					bool visible = true;
							
					// this is specialist culling for a lighting prepass view
					// we cull based on whether the object overlaps a light and needs to be included in the prepass
					if(pViewData->lightSpheresCount && g_UseLightCulling)
					{
						visible = false;

						// get the bounds of the object in unit sphere space relative to camera
						Maths::HF32 objectSphereRadius(radius);
						Maths::HVec3 objectSpherePosition(centreW);
						Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&pViewData->forwardZ));
						Maths::HF32 depth = (centreW | forward) - objectSphereRadius;
						
						Maths::HVec3 objectUnitSpherePosition = objectSpherePosition - pViewData->viewSettings3D.GetCameraPosition();
						Maths::HF32 length = Maths::Length(objectUnitSpherePosition);
						objectUnitSpherePosition /= length;
						Maths::HF32 objectUnitSphereRadius = objectSphereRadius / length;

						for(U32 i = 0; i < pViewData->lightSpheresCount; ++i)
						{
							Maths::HVec4 lightUnitSphere(Maths::Load<Maths::HVec4>(&pViewData->pLightSpheres[i].unitSphere));
							Maths::HVec4 lightSphere(Maths::Load<Maths::HVec4>(&pViewData->pLightSpheres[i].sphere));
							Maths::HVec3 lightSpherePosition(lightSphere);
							Maths::HVec3 lightUnitSpherePosition(lightUnitSphere);
							Maths::HVec4 lightDepth(Maths::Load<Maths::HVec4>(&pViewData->pLightSpheres[i].depth));

							Maths::HF32 combinedUnitRadiusSq = lightUnitSphere.W() + objectUnitSphereRadius;
							combinedUnitRadiusSq = combinedUnitRadiusSq * combinedUnitRadiusSq;

							Maths::HF32 combinedRadiusSq = lightSphere.W() + objectSphereRadius;
							combinedRadiusSq = combinedRadiusSq * combinedRadiusSq;

							Maths::HF32 lengthBetweenUnitSpheresSq = Maths::Length2(objectUnitSpherePosition - lightUnitSpherePosition);
							Maths::HF32 lengthBetweenSpheresSq = Maths::Length2(objectSpherePosition - lightSpherePosition);

							// if spheres overlap and depth is equal or nearer then we need to render
							if(lengthBetweenSpheresSq < combinedRadiusSq || (lengthBetweenUnitSpheresSq < combinedUnitRadiusSq && depth < lightDepth.X()))
							{
								visible = true;
								break;
							}
						}
					}
					
					if(visible)
					{
						F32 depthf = 0.0f;
						F32 depthScaledf = 0.0f;
						F32 screenSizef = 0.0f;
						result.volatileInfo.data = 0;


						if((pCullNode->GetVolatileInfoFlags() & E_VolatileNodeInfo_Distance) || pViewData->distanceSubtechniqueEnable)
						{
							Maths::HF32 depth = (centreW | forward);
							Maths::HF32 depthScaled = depth * depthScale;
							
							depthScaledf = depthScaled;
							depth -= radius;
							depthf = depth;
						}
						if(pCullNode->GetVolatileInfoFlags() & E_VolatileNodeInfo_ScreenSize)
						{
							Maths::HF32 depth = (centreW | forward);
							Maths::HVec4 coord(radius, Maths::Constants<Maths::HF32>::Zero(), depth, Maths::Constants<Maths::HF32>::One());
							coord = coord * pViewData->viewSettings3D.GetCameraToDevice();
							Maths::HF32 screenSize = Maths::Abs(coord.X() / coord.W()) * pixelCostHalf;
							screenSize = Maths::Clamp(screenSize, Maths::Constants<Maths::HF32>::Zero(), pixelCostMax);
							screenSizef = screenSize;
						}


						if((pCullNode->GetVolatileInfoFlags() & E_VolatileNodeInfo_Distance) || pViewData->distanceSubtechniqueEnable)
						{
							result.volatileInfo.distance = (U32)depthScaledf;
							result.volatileInfo.distanceSubtechnique = (pViewData->distanceSubtechniqueEnable && depthf > pViewData->distanceSubtechniqueDistance) ? 1 : 0;
						}
						if(pCullNode->GetVolatileInfoFlags() & E_VolatileNodeInfo_ScreenSize)
						{
							result.volatileInfo.screenSize = (U32)screenSizef;
						}

						result.passIndex = pCullNode->GetPassIndex();
						++num[v];
					}
				}
			}
		}
	}

#ifdef PLATFORM_SPU
	for(S32 v = 0; v < pData->outputs.Size(); ++v)
	{
		Spu::Set(&pOutputEA[v]->numResults, num[v]);
		// store results
		{
			U32 storeSize = sizeof(ModelInstanceFlatListGatherData::Output::Result) * num[v];
			U32 storeSizeMod = (storeSize + 15) & ~15;
			Spu::StoreAsync((void*)pOutputEA[v]->results, (void*)pResults[v], storeSizeMod, 0);
		}
	}
	Spu::Wait(0);
#else
	for(S32 v = 0; v < pData->outputs.Size(); ++v)
		pOutputEA[v]->numResults = num[v];
#endif
}

}

JQ_DECLARE_SPU_MODULE(ModelInstanceFlatListGatherModule, SceneGraph::ModelInstanceFlatListGather, scenegraph_modelinstanceflatlistgather, "scenegraph.scheduler");



