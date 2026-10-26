#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include "renderingdata/reloadabletextures.h"
#include "renderingstatistics.h"
#include "renderingdata/context.h"
#include "Display.h"

#include <fx/fx.h>
#include <fx/fxb.h>
#include <fx/I_Effect.h>
#include <fx/Effect.h>
#include <fx/Manager.h>
#include <fx/ImmediateParameter.h>

#include "system/scheduler/cache.h"

//#pragma optimize ("", off)

Debug::Bool g_wireframe("Graphics.RenderingData.Render.RenderWireframe", false, Debug::Persist());

extern Debug::Int g_chunkDebugIndex;
extern Debug::Int g_chunkDebugIndexOffset;

namespace RenderingData {
namespace Target
{

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

SharedTextureManager SharedTextureManager::s_instance;

FX::ImmediateParameter g_pFXDebugColour;

///////////////////////////////////////////////////////////////////////

void BeginFrame()
{
	RenderingStatistics::Instance().BeginFrame();
}

void EndFrame()
{
	RenderingStatistics::Instance().EndFrame();
}

////////////////////////////////////////////////////////////////////////

PlatformType GetPlatformType()
{
#if PLATFORM_PS3
	return PLATFORM_TYPE_PS3;
#elif PLATFORM_PC
	return PLATFORM_TYPE_PC;
#else
	assert(0);
	return PLATFORM_TYPE_UNKNOWN;
#endif
}

////////////////////////////////////////////////////////////////////////
// RenderListNode

const RenderListNode* RenderListNode::GetPointer(const Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST_PLATFORM))
		return (const RenderListNode*) p;
	else
		return 0;
}

RenderListNode* RenderListNode::GetPointer(Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST_PLATFORM))
		return (RenderListNode*) p;
	else
		return 0;
}

S32 MaterialData::GetSize() const
{
	S32 size = GetNumTypes() * sizeof(MaterialTypeData);
	size += 8; // max alignment

	for (int t = 0; t < GetNumTypes(); t++)
		size += GetNumParameters(t) * sizeof(MaterialParameterData);

	return size;
}

S32 MaterialData::CalculateSize(const RenderingData::MaterialList* pList) const
{
	S32 size = pList->GetNumMaterialTypes() * sizeof(MaterialTypeData);
	size += 8; // max alignment

	for (int t = 0; t < pList->GetNumMaterialTypes(); t++)
		size += pList->GetMaterialType(t).GetNumElements() * sizeof(MaterialParameterData);

	return size;
}

void MaterialData::InitialiseMaterialList(const Scene& scene, const RenderingData::MaterialList* pList, bool internalStorage)
{
	m_data.types.Destroy();
	m_autoStackAllocator.Free();
	S32 size = CalculateSize(pList);

	if (internalStorage)
	{
		Btl::DynamicStackAllocator* pAllocator = Btl::StackAllocatorSystem::Instance().Begin();
		pAllocator->Reserve(size);
		pAllocator->SetGrowSize(0);
		m_autoStackAllocator.Set(pAllocator);
	}

	SetNumMaterialTypes(pList->GetNumMaterialTypes());

	for (int t = 0; t < pList->GetNumMaterialTypes(); t++)
		SetNumParameters(t, pList->GetMaterialType(t).GetNumElements());

	m_data.FXIndex = !FX::g_Manager.GetReloadCount();
	UpdateParameters(scene, pList);

	if (internalStorage)
	{
		assert(m_autoStackAllocator.BytesUsed() <= size);
		m_autoStackAllocator.End();
	}
}

void MaterialData::UpdateParameters(const Scene& scene, const RenderingData::MaterialList *pList)
{
	assert(m_data.FXIndex > -1);

	int nReload = FX::g_Manager.GetReloadCount();
	if (m_data.FXIndex == nReload)
		return;

	m_data.FXIndex = nReload;
	g_pFXDebugColour = -1;
	FX::ParameterHandle h = FX::GetSharedParameter("g_vDebugColour");
	if (h != -1)
	{
		FX::Parameter *pParam = FX::GetParameterPtr(h);
		if (pParam->Data != NULL)
			g_pFXDebugColour = FX::GetImmediateParameter(NULL, pParam);
	}

	for (int t = 0; t < pList->GetNumMaterialTypes(); t++)
	{
		const MaterialType& materialType = pList->GetMaterialType(t);

		m_data.types[t].dataSize = materialType.GetDataSize();

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			FX::ImmediateParameter pParameter;

			MaterialParameterData& parameter = GetParameter(t, e);
			parameter.type = materialType.GetElement(e).type;
			parameter.offset = materialType.GetOffset(e, 0);
			parameter.count = materialType.GetElement(e).count;

			U32 iMaxSize = UINT_MAX;
			const Btl::String& name = materialType.GetElement(e).name;

			// HACK: this parameter is g_mPalette (skinning matrix palette) specify a maximum size
			//       this clamps the data upload size to the source data size indicated by the number of material parameters
			if (parameter.type == MATERIAL_ELEMENT_TYPE_DATA_INDEX && !stricmp(name.Chars(), "g_mPalette"))
				iMaxSize = 12*4 * parameter.count;

			if (materialType.GetEffect() > -1)
				pParameter = scene.GetEffect(materialType.GetEffect()).GetParameter(name, iMaxSize);
			else
				pParameter = FX::GetImmediateParameter(NULL, FX::GetParameterPtr(FX::GetSharedParameter(name.Chars())), iMaxSize, scene.GetSharedBindingUsage());

			parameter.fxParameterData = pParameter;
		}
	}
}

/////////////////////////////////////////////////////////////////////
// Scene

Scene::Scene() 
{
	m_streamData.m_pScene = this;
	m_pDebugName = 0;
	m_maxLodToLoad = INT_MAX;
	m_iFXReloadIndex = 0;
}

Scene::~Scene()
{
	g_resourceAllocatorMutex.Wait();
	m_resourceBlocks.Destroy();
	g_resourceAllocatorMutex.Release();
}

const Node* Scene::GetNodePtr(const ResourceIndex& i) const
{
	return const_cast<Scene*>(this)->GetNodePtr(i);
}

Node* Scene::GetNodePtr(const ResourceIndex& i)
{
	S32 block = i.GetBlock();
	S32 index = i.GetIndex();

	if (block > -1 && m_resourceBlocks[block].GetLod() > -1)
		return m_resourceBlocks[block].GetNodePtr(index);
	else if (block == -1 && index == 0)
		return &m_rootNode;
	else
		return 0;
}

S32 Scene::FindEffect(const Btl::String& name) const
{
	for (int i = 0; i < m_effects.Size(); i++)
		if (m_effects[i].GetName() == name)
			return i;

	return -1;
}

void Scene::UpdateResourcePointers()
{
	PERF_EVENT("Scene::UpdateResourcePointers");

	U32 iFXReloadCount = FX::g_Manager.GetReloadCount();
	if (m_iFXReloadIndex != iFXReloadCount)
	{
		m_iFXReloadIndex = iFXReloadCount;
		CalculateGlobalBindings();
	}

	for (int i = 0; i < m_activeNodes.Size(); i++)
	{
		if (m_activeNodes[i].cachedType == NODE_TYPE_RENDER_LIST_PLATFORM)
			UpdateResourcePointers(*m_activeNodes[i].pNode);
	}
}

void Scene::ProcessNodes(const Node* pNode, void (*f)(const Node& node)) const
{
	if (pNode)
	{
		(*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessNodes(Node* pNode, void (*f)(Node& node))
{
	if (pNode)
	{
		(*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessNodes(Node* pNode, void (Scene::*f)(Node& node))
{
	if (pNode)
	{
		(this->*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessNodes(const Node* pNode, void (Scene::*f)(const Node& node) const) const
{
	if (pNode)
	{
		(this->*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::Initialise(RenderListNode& node)
{
	// Re-initialise the render data (clears the allocator and fixed-size vectors).
	{
		node.m_renderDataTarget.~RenderDataTarget();
		node.m_autoStackAllocatorRenderTarget.Free();
		new (&node.m_renderDataTarget) RenderListNode::RenderDataTarget();
	}

	node.m_renderDataTarget.m_SharedFXUsage = m_SharedFXUsage;
	node.m_renderDataTarget.m_SharedFXMask = m_SharedFXMask;

	Btl::DynamicStackAllocator* pTemp = Btl::StackAllocatorSystem::Instance().Begin();

	// Initialise lods.
	{
		node.m_renderDataTarget.primitiveLods.InitialiseSize(node.m_renderData.primitives.Size());

		for (int i = 0; i < node.m_renderDataTarget.primitiveLods.Size(); i++)
			node.m_renderDataTarget.primitiveLods[i] = 0;
	}

	// Initialise techniques.
	{
		node.m_renderDataTarget.primitiveTechniques.InitialiseSize(node.m_renderData.primitives.Size());

		for (int i = 0; i < node.m_renderDataTarget.primitiveTechniques.Size(); i++)
			node.m_renderDataTarget.primitiveTechniques[i] = 0;
	}

	// Set up the resource pointers.
	node.m_renderDataTarget.indexBuffers.InitialiseSize(node.m_streamerData.indexBufferSource.Size());
	node.m_renderDataTarget.vertexBuffers.InitialiseSize(node.m_streamerData.vertexBufferSource.Size());
	node.m_renderDataTarget.textures.InitialiseSize(node.m_streamerData.textureSource.Size());

#if !CFG_SPEED
	node.textureDebug.Resize(node.m_streamerData.textureSource.Size());
	node.indexBufferDebug.Resize(node.m_streamerData.indexBufferSource.Size());
	node.vertexBufferDebug.Resize(node.m_streamerData.vertexBufferSource.Size());
#endif

	for (int i = 0; i < node.m_streamerData.indexBufferSource.Size(); i++)
		node.m_renderDataTarget.indexBuffers[i] = 0;

	for (int i = 0; i < node.m_streamerData.vertexBufferSource.Size(); i++)
	{
		ResourceIndex r = node.m_streamerData.vertexBufferSource[i];

		if (r.GetBlock() > -1)
		{
			node.m_renderDataTarget.vertexBuffers[i].pBuffer = 0;
			node.m_renderDataTarget.vertexBuffers[i].vertexSize = 1; // Indicates that the buffer should exist.
		}
		else
		{
			node.m_renderDataTarget.vertexBuffers[i].pBuffer = 0;
			node.m_renderDataTarget.vertexBuffers[i].vertexSize = 0;
		}
	}

	for (int i = 0; i < node.m_streamerData.textureSource.Size(); i++)
		node.m_renderDataTarget.textures[i].pGpuData = 0;

	// Load resources when no chunks.
	if (node.m_streamerData.chunkData.Empty())
	{
		for (int i = 0; i < node.m_streamerData.indexBufferSource.Size(); i++)
		{
			ResourceIndex r = node.m_streamerData.indexBufferSource[i];

			if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
				node.m_renderDataTarget.indexBuffers[i] = &GetResourceBlock(r.GetBlock()).GetIndexBuffer(r.GetIndex()).GetData();
			else
				node.m_renderDataTarget.indexBuffers[i] = 0;
		}

		for (int i = 0; i < node.m_streamerData.vertexBufferSource.Size(); i++)
		{
			ResourceIndex r = node.m_streamerData.vertexBufferSource[i];

			if (r.GetBlock() > -1 && GetResourceBlock(r.GetBlock()).GetLod() > -1)
			{
				node.m_renderDataTarget.vertexBuffers[i].pBuffer = &GetResourceBlock(r.GetBlock()).GetVertexBuffer(r.GetIndex()).GetData();
				node.m_renderDataTarget.vertexBuffers[i].vertexSize = GetResourceBlock(r.GetBlock()).GetVertexBuffer(r.GetIndex()).GetVertexSize();
			}
		}

		for (int i = 0; i < node.m_streamerData.textureSource.Size(); i++)
		{
			ResourceIndex r = node.m_streamerData.textureSource[i];

			if (r.GetBlock() > -1)
			{
				if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
				{
					const Texture& texture = GetResourceBlock(r.GetBlock()).GetTexture(r.GetIndex());
					node.m_renderDataTarget.textures[i].pGpuData = texture.GetData().GetGPUTexture();
					node.m_renderDataTarget.textures[i].iSamplerState = texture.GetSamplerStateIndex();
				}
				else
					node.m_renderDataTarget.textures[i].pGpuData = 0;
			}

		}
	}

	// Get shared textures.
	for (int i = 0; i < node.m_streamerData.textureSource.Size(); i++)
	{
		ResourceIndex r = node.m_streamerData.textureSource[i];

		if (r.GetBlock() == -1)
		{
			const Texture* pTexture = m_sharedTextures.GetTexture(r.GetIndex());
			node.m_renderDataTarget.textures[i].pGpuData = pTexture->GetData().GetGPUTexture();
			node.m_renderDataTarget.textures[i].iSamplerState = pTexture->GetSamplerStateIndex();
			assert(pTexture->GetSamplerStateIndex() > -1);
		}
	}

	node.m_streamerDataTarget.chunkState.Resize(node.m_streamerData.chunkData.Size());
	node.m_streamerDataTarget.chunkIndices.Resize(node.m_streamerData.chunkData.Size());

	for (int i = 0; i < node.m_streamerDataTarget.chunkIndices.Size(); i++)
		node.m_streamerDataTarget.chunkIndices[i] = node.m_streamerData.chunkData[i].chunkIndex;

	node.m_streamerDataTarget.chunkState.ClearBits();

	// Initialise constant materials
	node.m_materialData.Resize(1 + Maths::Max(node.m_sceneGraphData.dynamicMaterialLists.Size(), node.GetScenePtr()->GetNumDynamicMaterialLists()));
	node.m_materialData[0].InitialiseMaterialList(*this, &node.m_constantMaterialTypes);

	node.m_streamerDataTarget.initialised = true;
	node.m_streamerDataTarget.textureReloadIndex = 0;

	node.m_renderDataTarget.effects.InitialiseSize(m_effects.Size());

	for (int i = 0; i < m_effects.Size(); i++)
		node.m_renderDataTarget.effects[i] = m_effects[i].GetEffect();

	node.m_renderDataTarget.vertexDeclarations.InitialiseSize(m_vertexDeclarations.Size());

	for (int i = 0; i < m_vertexDeclarations.Size(); i++)
		node.m_renderDataTarget.vertexDeclarations[i] = &m_vertexDeclarations[i].GetData();

	Btl::StackAllocatorSystem::Instance().End(pTemp);

	// Move all the data into a single buffer.
	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		node.m_autoStackAllocatorRenderTarget.Set(p);
		p->Reserve(pTemp->BytesUsed() + 16); 
		p->SetGrowSize(0);

		RenderListNode::RenderDataTarget renderDataTarget(node.m_renderDataTarget);
		Btl::SwapByMemcopy(renderDataTarget, node.m_renderDataTarget);

		node.m_autoStackAllocatorRenderTarget.End();
	}

	Btl::StackAllocatorSystem::Instance().Free(pTemp);
}


// calculate which shared parameters are global for this scene, and calculate
// which shared parameters would be blatted over when uploading these shared parameters
void Scene::CalculateGlobalBindings()
{
	m_SharedFXUsage.ClearBits();
	m_SharedFXMask.ClearBits();

	for (int i=0; i<m_effects.Size(); i++)
		FX::GetSharedBindingUsage(m_effects[i].GetEffect(), &m_SharedFXUsage, &m_SharedFXMask);
	for (int i=0; i<m_SharedFXUsage.Size(); i++)
		m_SharedFXUsage[i] = m_SharedFXUsage[i] & !m_SharedFXMask[i];

	// TODO: Instead of masking out all of the bindings that conflict for all of the effects, just mask out the ones that conflict
    //        with the global bindings left after applying the mask (i.e. recalculate SharedFXMask at this point)
}

void Scene::UpdateResourcePointers(Node& node)
{
	RenderListNode* pRenderListNode = RenderListNode::GetPointer(&node);
	assert(pRenderListNode);

	bool checkAllChunks = m_streamData.MultipleChunksUpdated();

	//PERF_EVENT("Scene::UpdateResourcePointers");

	// If this is the first update of the node, initialise it.
	// (or if the texture reloader has just been disabled)
	if (!pRenderListNode->m_streamerDataTarget.initialised
		|| pRenderListNode->m_streamerDataTarget.textureReloadIndex > ReloadableTextureManager::Instance().GetReloadCount())
	{
		Initialise(*pRenderListNode);
		checkAllChunks = true;
	}

	// If any chunks have been loaded or unloaded since the last update, copy the resource pointers from the blocks.
	if (checkAllChunks || m_streamData.LoadedChunk() > -1 || m_streamData.UnloadedChunk() > -1)
	{
		for (int ic = 0; ic < pRenderListNode->m_streamerData.chunkData.Size(); ic++)
		{
			S32 chunkIndex = pRenderListNode->m_streamerDataTarget.chunkIndices[ic];
			bool update;

			if (checkAllChunks)
				update = (pRenderListNode->m_streamerDataTarget.chunkState[ic] != m_streamData.IsLoaded(pRenderListNode->m_streamerDataTarget.chunkIndices[ic]));
			else
				update = (chunkIndex == m_streamData.LoadedChunk() || chunkIndex == m_streamData.UnloadedChunk());
			
			assert(update || pRenderListNode->m_streamerDataTarget.chunkState[ic] == m_streamData.IsLoaded(pRenderListNode->m_streamerDataTarget.chunkIndices[ic]));

			if (update)
			{
				const RenderListNode::ChunkData& chunkData = pRenderListNode->m_streamerData.chunkData[ic];

				pRenderListNode->m_streamerDataTarget.chunkState[ic] = m_streamData.IsLoaded(chunkData.chunkIndex);

				for (int i = 0; i < chunkData.indexBuffers.Size(); i++)
				{
					S32 indexInList = chunkData.indexBuffers[i].indexInList;
					ResourceIndex r = pRenderListNode->m_streamerData.indexBufferSource[indexInList];

					if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
						pRenderListNode->m_renderDataTarget.indexBuffers[indexInList] = &GetResourceBlock(r.GetBlock()).GetIndexBuffer(r.GetIndex()).GetData();
					else
						pRenderListNode->m_renderDataTarget.indexBuffers[indexInList] = 0;

	#ifndef CFG_SPEED
					pRenderListNode->indexBufferDebug[indexInList] = chunkData.chunkIndex;
	#endif
				}

				for (int i = 0; i < chunkData.vertexBuffers.Size(); i++)
				{
					S32 indexInList = chunkData.vertexBuffers[i].indexInList;
					ResourceIndex r = pRenderListNode->m_streamerData.vertexBufferSource[indexInList];

					if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
					{
						pRenderListNode->m_renderDataTarget.vertexBuffers[indexInList].pBuffer = &GetResourceBlock(r.GetBlock()).GetVertexBuffer(r.GetIndex()).GetData();
						pRenderListNode->m_renderDataTarget.vertexBuffers[indexInList].vertexSize = GetResourceBlock(r.GetBlock()).GetVertexBuffer(r.GetIndex()).GetVertexSize();
					}
					else
					{
						pRenderListNode->m_renderDataTarget.vertexBuffers[indexInList].pBuffer = 0;
						pRenderListNode->m_renderDataTarget.vertexBuffers[indexInList].vertexSize = 1;
					}

	#ifndef CFG_SPEED
					pRenderListNode->vertexBufferDebug[indexInList] = chunkData.chunkIndex;
	#endif
				}

				for (int i = 0; i < chunkData.textures.Size(); i++)
				{
					S32 indexInList = chunkData.textures[i].indexInList;
					ResourceIndex r = pRenderListNode->m_streamerData.textureSource[indexInList];

					assert(r.GetBlock() > -1);

					if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
					{
						const Texture& texture = GetResourceBlock(r.GetBlock()).GetTexture(r.GetIndex());
						pRenderListNode->m_renderDataTarget.textures[indexInList].pGpuData = texture.GetData().GetGPUTexture();
						pRenderListNode->m_renderDataTarget.textures[indexInList].iSamplerState = texture.GetSamplerStateIndex();
						assert(texture.GetSamplerStateIndex() > -1);

						// See if the texture reloader has this texture.
						if (const Texture* pTextureReloaded = ReloadableTextureManager::Instance().GetTexturePtr(texture.GetName()))
						{
							void* pTexture = (void*) pTextureReloaded->GetData().GetGPUTexture();
							pRenderListNode->m_renderDataTarget.textures[indexInList].pGpuData = pTexture;
							ReloadableTextureManager::Instance().CopyFlags((Texture*)pTextureReloaded, &texture);
						}
					}
					else
						pRenderListNode->m_renderDataTarget.textures[indexInList].pGpuData = 0;

	#ifndef CFG_SPEED
					// debug
					if (pRenderListNode->m_streamerDataTarget.chunkState[ic] == true)
						pRenderListNode->textureDebug[indexInList].chunks.PushBack(chunkData.chunkIndex);
					else
						Btl::RemoveNonOrdered(pRenderListNode->textureDebug[indexInList].chunks, chunkData.chunkIndex);
	#endif
				}
			}
		}
	}

	// Update anything in the texture reloader.
	if (pRenderListNode->m_streamerDataTarget.textureReloadIndex < ReloadableTextureManager::Instance().GetReloadCount())
	{
		pRenderListNode->m_streamerDataTarget.textureReloadIndex = ReloadableTextureManager::Instance().GetReloadCount();

		for (int i = 0; i < pRenderListNode->m_streamerData.textureSource.Size(); i++)
		{
			ResourceIndex r = pRenderListNode->m_streamerData.textureSource[i];
			const Texture* pTextureOriginal = 0;

			if (r.GetBlock() > -1) // streamed
			{
				if (GetResourceBlock(r.GetBlock()).GetLod() > -1)
					pTextureOriginal = &GetResourceBlock(r.GetBlock()).GetTexture(r.GetIndex());
			}
			else // shared
			{
				pTextureOriginal = m_sharedTextures.GetTexture(r.GetIndex());
			}

			if (pTextureOriginal)
			{
				if (const Texture* pTextureReloaded = ReloadableTextureManager::Instance().GetTexturePtr(pTextureOriginal->GetName()))
				{
					pRenderListNode->m_renderDataTarget.textures[i].pGpuData = pTextureReloaded->GetData().GetGPUTexture();
					ReloadableTextureManager::Instance().CopyFlags((Texture*)pTextureReloaded, pTextureOriginal);
				}
			}
		}
	}
}

void Scene::AccumulateTextureInfo(TextureInfoList& ioTextureInfoList) const
{
	// TODO: streamed textures?

	m_sharedTextures.AccumulateTextureInfo(ioTextureInfoList);
}

/////////////////////////////////////////////////////////////////////////////
// RenderListNode

// Temporary until this is done on export.
void RenderListNode::Reallocate()
{
	PERF_EVENT("RenderListNode::Reallocate");

	RenderingData::MaterialList constantMaterials;

	// Move all the data not used for rendering into the default allocator on the node.
	if (m_autoStackAllocatorDefault.BytesUsed() == 0)
	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		m_autoStackAllocatorDefault.Set(p);

		StreamerData streamerData(m_streamerData);
		SceneGraphData sceneGraphData(m_sceneGraphData);
		constantMaterials.CopyTypes(m_constantMaterialTypes);

		Btl::SwapByMemcopy(streamerData, m_streamerData);
		Btl::SwapByMemcopy(sceneGraphData, m_sceneGraphData);
		Btl::SwapByMemcopy(constantMaterials, m_constantMaterialTypes);

		m_autoStackAllocatorDefault.End();
	}

	// Move all the data used for rendering into a single buffer.
	if (m_autoStackAllocatorRender.BytesUsed() == 0)
	{
		m_constantMaterialData.CopyData(constantMaterials);

		Btl::DynamicStackAllocator* pTemp = Btl::StackAllocatorSystem::Instance().Begin();
			
		// First move it into a temporary buffer to find out how big it is.
		{
			RenderData renderData(m_renderData);
			Btl::SwapByMemcopy(renderData, m_renderData);
		}

		Btl::StackAllocatorSystem::Instance().End(pTemp);

		// Copy to the final buffer.
		{
			Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
			m_autoStackAllocatorRender.Set(p);
			p->Reserve(pTemp->BytesUsed()); 
			p->SetGrowSize(0);

			RenderData renderData(m_renderData);
			Btl::SwapByMemcopy(renderData, m_renderData);

			m_autoStackAllocatorRender.End();
		}

		Btl::StackAllocatorSystem::Instance().Free(pTemp);
	}
}


/////////////////////////////////////////////////////////////////////////////
// Effect

Effect::Effect()
{
	m_pFX = NULL;
}

Effect::~Effect()
{
	FX::ReleaseEffect(m_pFX);
}

FX::ImmediateParameter Effect::GetParameter(const Btl::String& name, U32 cMaxSize) const
{
	if (m_pFX)
		return FX::GetImmediateParameter(m_pFX, m_pFX->GetParameter(name.Chars()), cMaxSize);
	else
	{
		FX::ImmediateParameter rtn = {0};
		return rtn;
	}
}


}}
