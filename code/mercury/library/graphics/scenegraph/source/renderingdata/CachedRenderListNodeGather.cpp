#include "stdafx.h"

#include "SceneGraph/RenderingData/CachedRenderListNodeGather.h"
#include "SceneGraph/ViewData.h"
#include "SceneGraph/Bounds.h"
#include "SceneGraph/Enums.h"
#include <system/perf.h>
#ifdef PLATFORM_SPU
#include "System/Debug/Log/DebugLogSPU.h"
#include "../source/maths/hvec/VectorConst.cpp"
#endif

namespace SceneGraph
{


#ifndef PLATFORM_SPU
Debug::Bool g_UseLightCulling("Graphics.SceneGraph.UseLightCulling", true);
#else
bool g_UseLightCulling = true;
#endif

#ifndef PLATFORM_SPU
	extern Debug::Logger& g_Log;
#endif

void CullBounds(const CachedRenderListNodeData::PassData::Input* pInput, CachedRenderListNodeData::PassData::Outputs& outputs, const U8* pPvs, const U8* pPortals, const CachedRenderListNodeData::Views& viewData, const U32 volatileInfoFlags)
{
	CachedRenderListNodeData::PassData::Output* pOutputEA[MAX_VIEWS];
	U32 num[MAX_VIEWS];
	U32 viewIndices[MAX_VIEWS];

	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		pOutputEA[v] = outputs[v];
		viewIndices[v] = Spu::Get(&outputs[v]->viewIndex);
		num[v] = 0;
	}

	CachedRenderListNodeData::PassData::Output::Result* pResults[MAX_VIEWS];
	
#ifdef PLATFORM_SPU
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		pResults[v] = (CachedRenderListNodeData::PassData::Output::Result*)Spu::Alloc(sizeof(CachedRenderListNodeData::PassData::Output::Result) * NUM_RESULTS);
	}
#else
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		pResults[v] = pOutputEA[v]->results;
	}
#endif

	const C_Bounds* pPrefetch = &pInput->pBounds[0];

	for(U32 i = 0; i < pInput->boundsCount; ++i)
	{
		DCBT(1024, pPrefetch);
		++pPrefetch;
		
		if(!pPortals || GetBitValue(pPortals, pInput->pIndices[i]))
		{
			// check pvs
			if(!pPvs || GetBitValue(pPvs, pInput->pIndices[i]))
			{
				const C_Bounds& bb = pInput->pBounds[i];

				// load centre, size radius, and near and far clips
				Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&bb.centre));
				Maths::HVec4 sizeRadius(Maths::Load<Maths::HVec4>(&bb.size));
				Maths::HVec4 clip(Maths::Load<Maths::HVec4>(&bb.clip));
				
				// get size
				Maths::HVec3 size(sizeRadius);

				for(S32 v = 0; v < outputs.Size(); ++v)
				{
					const ViewData* pViewData = viewData[viewIndices[v]];
					const Display::C_ViewFrustum* pViewFrustum = &pViewData->viewFrustum;
					CachedRenderListNodeData::PassData::Output::Result& result = pResults[v][num[v]];

					// check for flags exclusion
					U32 typeFlag = pViewData->typeFlag;
					if(typeFlag == (typeFlag & pInput->pFlags[i]))
					{
						// viewParams.X = farClip, viewParams.Y = depthScale, viewParams.Z = instanceDrawScale, W = forwardMultiplier
						Maths::HVec4 viewParams(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pViewData->farClip));

						// get a value of either 1.0 or instanceDrawScale depending on whether we are using draw scales for this pass
						Maths::HF32 drawScale = viewParams.Z();

						// scale clip vector by draw scale
						Maths::HVec4 viewClip = clip * drawScale;
						viewClip.X(viewClip.X());
						viewClip.Y(Maths::Min(viewClip.Y(), viewParams.X() * viewParams.W()));

						if(pViewFrustum->CullAxisBox(centreW, Maths::HVec4(size), viewClip) || !(pViewData->cullFlags & E_CullMethod_Frustum))
						{
							bool visible = true;
							
							// this is specialist culling for a lighting prepass view
							// we cull based on whether the object overlaps a light and needs to be included in the prepass
							if(pViewData->lightSpheresCount && g_UseLightCulling)
							{
								visible = false;

								// get the bounds of the object in unit sphere space relative to camera
								Maths::HF32 objectSphereRadius(sizeRadius.W());
								Maths::HVec3 objectSpherePosition(centreW);
								Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&pViewData->forwardZ));
								Maths::HF32 depth = (centreW | forward) - objectSphereRadius;
								
								Maths::HVec3 objectUnitSpherePosition = objectSpherePosition - pViewData->viewSettings3D.GetCameraPosition();
								Maths::HF32 length = Maths::Length(objectUnitSpherePosition);
								objectUnitSpherePosition /= length;
								Maths::HF32 objectUnitSphereRadius = objectSphereRadius / length;

								// if we are inside the sphere its always visible
								if(length < objectSphereRadius && pViewData->lightSpheresCount)
									visible = true;
								else
								{
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
							}
							
							if(visible)
							{
								result.index = pInput->pIndices[i];
								++num[v];
							}
						}
					}
				}
			}
		}
	}

	// store results back to main memory
#ifdef PLATFORM_SPU
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		U32 storeSize = sizeof(CachedRenderListNodeData::PassData::Output::Result) * num[v];
		U32 storeSizeMod = (storeSize + 15) & ~15;
		Spu::StoreAsync((void*)pOutputEA[v]->results, (void*)pResults[v], storeSizeMod, 0);
		Spu::Set(&pOutputEA[v]->numResults, num[v]);
	}
	Spu::Wait(0);
#else
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		assert(num[v] < NUM_RESULTS);
		pOutputEA[v]->numResults = num[v];
	}
#endif
}

void CullBoundsDistance(const CachedRenderListNodeData::PassData::Input* pInput, CachedRenderListNodeData::PassData::Outputs& outputs, const U8* pPvs, const U8* pPortals, const CachedRenderListNodeData::Views& viewData, const U32 volatileInfoFlags)
{
	CachedRenderListNodeData::PassData::Output* pOutputEA[MAX_VIEWS];
	U32 num[MAX_VIEWS];
	U32 viewIndices[MAX_VIEWS];
	CachedRenderListNodeData::PassData::Output::Result* pResults[MAX_VIEWS];

	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		pOutputEA[v] = outputs[v];
		viewIndices[v] = Spu::Get(&outputs[v]->viewIndex);
		num[v] = 0;

	#ifdef PLATFORM_SPU
		pResults[v] = (CachedRenderListNodeData::PassData::Output::Result*)Spu::Alloc(sizeof(CachedRenderListNodeData::PassData::Output::Result) * NUM_RESULTS);
	#else
		pResults[v] = pOutputEA[v]->results;
	#endif
	}
	
	for(U32 i = 0; i < pInput->boundsCount; ++i)
	{
		DCBT(1024, &pInput->pBounds[i]);
		const C_Bounds& bb = pInput->pBounds[i];

		// check portals
		if(!pPortals || GetBitValue(pPortals, pInput->pIndices[i]))
		{
			// check pvs
			if(!pPvs || GetBitValue(pPvs, pInput->pIndices[i]))
			{
				// load centre, size radius, and near and far clips
				Maths::HVec4 centreW(Maths::Load<Maths::HVec4>(&bb.centre));
				Maths::HVec4 sizeRadius(Maths::Load<Maths::HVec4>(&bb.size));
				Maths::HVec4 clip(Maths::Load<Maths::HVec4>(&bb.clip));
				
				// get size
				Maths::HVec3 size(sizeRadius);

				for(S32 v = 0; v < outputs.Size(); ++v)
				{
					const ViewData* pViewData = viewData[viewIndices[v]];
					const Display::C_ViewFrustum* pViewFrustum = &pViewData->viewFrustum;
					CachedRenderListNodeData::PassData::Output::Result& result = pResults[v][num[v]];

					U32 typeFlag = pViewData->typeFlag;
					if(typeFlag == (typeFlag & pInput->pFlags[i]))
					{
						// viewParams.X = farClip, viewParams.Y = depthScale, viewParams.Z = instanceDrawScale, W = forwardMultiplier
						Maths::HVec4 viewParams(Maths::Load<Maths::HVec4>((MathsStorage::Float4A16*)&pViewData->farClip));

						Maths::HF32 drawScale = viewParams.Z();

						// scale clip vector by draw scale
						Maths::HVec4 viewClip = clip * drawScale;
						viewClip.X(viewClip.X());
						// set the max draw distance to be the minimum of draw distance and farclip
						viewClip.Y(Maths::Min(viewClip.Y(), viewParams.X() * viewParams.W()));

						// get depthscale and forward vectors for this view
						Maths::HF32 depthScale(viewParams.Y());
						Maths::HVec4 forward(Maths::Load<Maths::HVec4>(&pViewData->forwardZ));
				
						if(pViewFrustum->CullAxisBox(centreW, Maths::HVec4(size), viewClip) || !(pViewData->cullFlags & E_CullMethod_Frustum))
						{
							Maths::HF32 depth = (centreW | forward);
							Maths::HF32 depthScaled = depth * depthScale;
							depth -= sizeRadius.W();

							result.index = pInput->pIndices[i];
							

							result.volatileInfo.distanceSubtechnique = (pViewData->distanceSubtechniqueEnable && depth > pViewData->distanceSubtechniqueDistance) ? 1 : 0;
							result.volatileInfo.distance = (U32)depthScaled;

							++num[v];
						}
					}
				}
			}
		}
	}

	// store results back to main memory
#ifdef PLATFORM_SPU
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		Spu::Set(&pOutputEA[v]->numResults, num[v]);
		U32 storeSize = sizeof(CachedRenderListNodeData::PassData::Output::Result) * num[v];
		U32 storeSizeMod = (storeSize + 15) & ~15;
		Spu::StoreAsync((void*)pOutputEA[v]->results, (void*)pResults[v], storeSizeMod, 0);
	}
	Spu::Wait(0);
#else
	for(S32 v = 0; v < outputs.Size(); ++v)
	{
		assert(num[v] < NUM_RESULTS);
		pOutputEA[v]->numResults = num[v];
	}
#endif
}

void GatherPassData(CachedRenderListNodeData::PassData* pData, const U8* pPvs, const U8* pPortals, const CachedRenderListNodeData::Views& viewData)
{
	Spu::MemBaseMarker marker;

	// fetch input data
	CachedRenderListNodeData::PassData::Input* pInput;
	{
		pData = (CachedRenderListNodeData::PassData*)Spu::FetchAlloc(pData, sizeof(CachedRenderListNodeData::PassData));

		pInput = &pData->input;

		// fetch aligned bounds
		U32 boundsSize = (sizeof(C_Bounds) * pInput->boundsCount + 15) & ~15;
		pInput->pBounds = (const C_Bounds*)Spu::FetchAllocAsync(pInput->pBounds, boundsSize, 0);

		// fetch aligned indices
		U32 u16Size = (sizeof(U16) * pInput->boundsCount + 15) & ~15;
		pInput->pIndices = (const U16*)Spu::FetchAllocAsync(pInput->pIndices, u16Size, 0);

		// fetch aligned flags
		pInput->pFlags = (const U16*)Spu::FetchAllocAsync(pInput->pFlags, u16Size, 0);
		Spu::Wait(0);
	}

	if(pData->input.volatileInfoFlags & E_VolatileNodeInfo_Distance)
		CullBoundsDistance(pInput, pData->outputs, pPvs, pPortals, viewData, pData->input.volatileInfoFlags);
	else
		CullBounds(pInput, pData->outputs, pPvs, pPortals, viewData, pData->input.volatileInfoFlags);
}

const U8* FetchBits(const U8* pSrc, const U32 count)
{
#ifdef PLATFORM_SPU
	U8* pBits = (U8*)(((U32)pSrc) & ~15);
	U32 offset = pSrc - pBits;
	U32 iSize = ((count + 7) / 8 + 15 + offset) & ~15;
	return (const U8*)Spu::FetchAllocAsync(pBits, iSize, 0) + offset;
#else
	return pSrc;
#endif
}

void CachedRenderListNodeGather(Scheduler::Task* pTask)
{
	PERF_EVENT("SceneGraph.RenderListGather");

	CachedRenderListNodeDataBatch* pBatch = (CachedRenderListNodeDataBatch*)Spu::FetchAlloc(pTask->Input, sizeof(CachedRenderListNodeDataBatch));
	
	for(S32 n = 0; n < pBatch->cachedRenderListNodeData.Size(); ++n)
	{
		Spu::MemBaseMarker marker;
		// fetch things onto SPU
		CachedRenderListNodeData* pData = (CachedRenderListNodeData*)Spu::FetchAlloc(pBatch->cachedRenderListNodeData[n], sizeof(CachedRenderListNodeData));
		
		// fetch all view data onto SPU
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

		if(pData->primitiveData.Size() != 0)
		{
			Spu::MemBaseMarker marker;

			// pvs data is for all passes so fetch all of it
			if(pData->pPrimitivePvs)
				pData->pPrimitivePvs = FetchBits(pData->pPrimitivePvs, pData->primitivePvsCount);

			// portal data is for all passes so fetch all of it
			if(pData->pPrimitivePortal)
				pData->pPrimitivePortal = FetchBits(pData->pPrimitivePortal, pData->primitivePortalCount);
				
			for(S32 i = 0; i < pData->primitiveData.Size(); ++i)
			{
				GatherPassData(pData->primitiveData[i], pData->pPrimitivePvs, pData->pPrimitivePortal, pData->viewData);
			}
		}

		if(pData->instanceData.Size() != 0)
		{
			Spu::MemBaseMarker marker;

			if(pData->pInstancePvs)
				pData->pInstancePvs = FetchBits(pData->pInstancePvs, pData->instancePvsCount);

			if(pData->pInstancePortal)
				pData->pInstancePortal = FetchBits(pData->pInstancePortal, pData->instancePortalCount);

			for(S32 i = 0; i < pData->instanceData.Size(); ++i)
			{
				GatherPassData(pData->instanceData[i], pData->pInstancePvs, pData->pInstancePortal, pData->viewData);
			}
		}
	}
}

}

JQ_DECLARE_SPU_MODULE(CachedRenderListNodeGatherModule, SceneGraph::CachedRenderListNodeGather, scenegraph_cachedrenderlistnodegather, "scenegraph.scheduler");
//JQ_DECLARE_PPU_MODULE(CachedRenderListNodeGatherModule, SceneGraph::CachedRenderListNodeGather);



