#include "stdafx.h"

#include "Dynamic/Renderer.h"
#include "Dynamic/Interface.h"
#include "Dynamic/ManagerInterface.h"
#include "Dynamic/Node.h"
#include "Dynamic/TextureCoords.h"
#include "Dynamic/Shaders/Constants.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/Block.h"

#include "Render/Utils/Globals.h"
#include "Render/State/RenderState.h"
#include "Render/State/Fog.h"
#include "Render/View/ViewManager.h"

#include "SceneGraph/Visitor.h"
#include "SceneGraph/View.h"

#define SCENEGRAPH_ALLOW_INCLUDE	// only for Renderer.cpp
#define FX_ALLOW_INCLUDE
#include "SceneGraph/SceneGraphImpl.h"
#include "SceneGraph/Context.h"

#include <system\DebugLog.h>
#include <system\memory.h>
#include <system\Debug\I_DebugRender.h>

#include <Display/Bundler/Bundler.h>
#include <Display/Context/Context.h>

#include <fx/FxContext.h>

namespace DynamicEffect
{
	
Debug::Logger &g_Log = Debug::GetLogger("mercury.render.dynamic");

Debug::Float g_debugDynamicEffectTimeScale("Graphics.Dynamic.TimeScale", 1.0f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));
Debug::Float g_debugDynamicEffectLodFadeStart("Graphics.Dynamic.LodFadeStart", 0.8f, Debug::Range(0.f, 1.0f), Debug::Step(0.05f));

Debug::Bool g_debugDynamicEffectDebugInfo("Graphics.Dynamic.DebugInfo.Show", false);
Debug::Bool g_debugDynamicEffectDebugInfoPrintTextureNames("Graphics.Dynamic.DebugInfo.PrintTextureNames", false);
Debug::Int g_debugDynamicEffectDebugInfoShowEffect("Graphics.Dynamic.DebugInfo.Effect", 0, Debug::Range(0, 200), Debug::Step(1));

Debug::Bool g_debugDynamicEffectDebugGraphics("Graphics.Dynamic.DebugGraphics.Render", false);
Debug::Bool g_debugDynamicEffectDebugGraphicsDepthEnable("Graphics.Dynamic.DebugGraphics.DepthEnable", true);
Debug::Bool g_debugDynamicEffectDebugGraphicsBlockBounds("Graphics.Dynamic.DebugGraphics.BlockBounds", true);
Debug::Bool g_debugDynamicEffectDebugGraphicsLights("Graphics.Dynamic.DebugGraphics.Lights", true);
Debug::Bool g_debugDynamicEffectDebugGraphicsBlockBounds2D("Graphics.Dynamic.DebugGraphics.BlockBounds2D", false);

Debug::Bool g_debugDynamicEffectUseJobQueue("Graphics.Dynamic.Build.UseJobQueue", true);

Debug::Bool g_debugDynamicEffectSortEffects("Graphics.Dynamic.Gather.SortEffects", true);
Debug::Bool g_debugDynamicEffectCullMode("Graphics.Dynamic.Gather.UseCullMode", true);
Debug::Bool g_debugDynamicEffectStateManager("Graphics.Dynamic.Gather.UseStateManager", true);
Debug::Bool g_debugDynamicEffectBatch("Graphics.Dynamic.Gather.UseBatching", true);
Debug::Int g_debugDynamicEffectDrawBatches("Graphics.Dynamic.Gather.MaxDrawBatches", 500, Debug::Range(0, 500), Debug::Step(1));
Debug::Int g_debugDynamicEffectBatchMax("Graphics.Dynamic.Gather.MaxBlocksPerBatch", 10, Debug::Range(0, 50), Debug::Step(1));

Debug::Bool g_debugDynamicEffectUseBlending("Graphics.Dynamic.Materials.UseBlending", true);
Debug::Bool g_debugDynamicEffectWireframe("Graphics.Dynamic.Materials.Wireframe", false);

Debug::Bool g_debugDynamicEffectRemoveUnusedTextures("Graphics.Dynamic.Materials.RemoveUnusedTextures", false);

C_Renderer::C_Renderer(SceneGraph::C_SceneGraph* pSceneGraph) : Display::I_Renderer("DynamicEffect"),
																I_GatherNode(SceneGraph::E_GatherNodeType_Unknown, pSceneGraph),
																SceneGraph::T_Renderer<C_View>(pSceneGraph)
{
	MEMORY_TRACKER_PUSH("graphics.dynamic");

	m_pExternalTextures = NULL;
	m_Lights.Reserve(16);
	SetAssetPath("Textures/");

	m_BufferSize = 512 * 1024;	// 512k buffers
	m_BufferCount = 16; // 16 x 512k buffers
	m_BlocksCount = 128; // 128 blocks

	m_fTime = 0.0f;
	m_fCycle = 0.0f;
		
	m_bPaused = false;
	m_bSRGB = false;
	m_SamplerUsage.SetEnabled(0, 0);
	m_SamplerUsage.SetEnabled(0, 1);
	m_SamplerUsage.SetEnabled(0, 2);

	m_JobQueuePolicyHandle = -1;

	m_pRenderState = &Render::C_RenderState::Instance();

	U32 index = C_TextureCoords::Instance().GetTextureCoordinateIndex(E_Atlas_Default1x1);

	// set all scene graph pass mappings to invalid
	for(S32 i = 0; i < m_DeToSgPassMapping.Size(); ++i)
	{
		m_DeToSgPassMapping[i] = -1;
	}

	MEMORY_TRACKER_POP("graphics.dynamic");
}

C_Renderer::~C_Renderer()
{
	DestroyRenderer();

	// kill all effects first, since effects can own emitters (super effects)
	while(m_Effects.Begin() != m_Effects.End())
	{
		I_DynamicEffect* pEffect = m_Effects.Begin()->GetValue();
		delete pEffect;
	}

	while(m_Managers.Begin() != m_Managers.End())
	{
		I_DynamicEffectManager* pManager = m_Managers.Begin()->GetValue();
		delete pManager;
	}
}

bool C_Renderer::OnCreate()
{
	RESOURCE_MEMORY_ID("DynamicEffect");

	// create white texture
	if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_WhiteTexture))
		return false;
	else
	{
		Display::C_LockedRect rect = m_WhiteTexture.Lock(0);
		if(rect.pBits)
		{
			U32 *pData = reinterpret_cast<U32*>(rect.pBits);
			*pData = 0xffffffff;
			m_WhiteTexture.Unlock(0);
		}
	}

	#ifndef CFG_GOLD
	m_WhiteTexture.SetFileName("_DynamicEffectWhiteTexture");
	#endif

	// create grey texture
	if (!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_GreyTexture))
		return false;
	else
	{
		Display::C_LockedRect rect = m_GreyTexture.Lock(0);
		if(rect.pBits)
		{
			U32 *pData = reinterpret_cast<U32*>(rect.pBits);
			*pData = 0xff808080;
			m_GreyTexture.Unlock(0);
		}
	}

	#ifndef CFG_GOLD
	m_GreyTexture.SetFileName("_DynamicEffectGreyTexture");
	#endif

	bool bCreated = true;
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		bCreated = bCreated && pIterator->GetValue()->Create();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		bCreated = bCreated && pManager->Create();
	}

	// diffuse map
	m_DiffuseSamplerState.SetDefaultStates();
	m_DiffuseSamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
	m_DiffuseSamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);
#if PLATFORM_PS3
	m_DiffuseSamplerState.SetTextureGamma(m_bSRGB ? Display2::E_TextureGamma_R | Display2::E_TextureGamma_G | Display2::E_TextureGamma_B : 0);
#endif

	// normal map
	m_NormalSamplerState.SetDefaultStates();
	m_NormalSamplerState.SetTextureFilter(Display2::E_TextureFilter_Linear);
	m_NormalSamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);

	// depth buffer
	m_DepthSamplerState.SetDefaultStates();
	m_DepthSamplerState.SetTextureFilter(Display2::E_TextureFilter_Point);
	m_DepthSamplerState.SetTextureAddress(Display2::E_TextureAddress_Clamp);

	return bCreated;
}

void C_Renderer::OnDestroy()
{
	Display::Device()->Destroy(&m_WhiteTexture);
	Display::Device()->Destroy(&m_GreyTexture);
	
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		pIterator->GetValue()->Destroy();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->Destroy();
	}

	RemoveTextures();
	assert(m_Textures.Begin() == m_Textures.End());
}

bool C_Renderer::OnCreateVolatile()
{
	m_DynamicVertexBuffer.ReserveBlocks(m_BlocksCount);
	m_DynamicVertexBuffer.SetVertexBufferSize(m_BufferSize, m_BufferCount);
	bool bCreated = m_DynamicVertexBuffer.OnCreateVolatile();
	
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		bCreated = bCreated && pIterator->GetValue()->CreateVolatile();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		bCreated = bCreated && pManager->CreateVolatile();
	}

	return bCreated;
}

bool C_Renderer::OnInitialise()
{
	bool bInitialised = true;
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		bInitialised = bInitialised && pIterator->GetValue()->Initialise();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		bInitialised = bInitialised && pManager->Initialise();
	}

	return bInitialised;
}

void C_Renderer::OnShutdown()
{
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		pIterator->GetValue()->Shutdown();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->Shutdown();
	}
}

void C_Renderer::OnDestroyVolatile()
{
	m_DynamicVertexBuffer.OnDestroyVolatile();

	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		pIterator->GetValue()->DestroyVolatile();
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->DestroyVolatile();
	}
}

void C_Renderer::ReserveVertexBuffers(const U32 bufferSize, const U32 bufferCount, const U32 blocksCount)
{
	m_BufferSize = bufferSize;
	m_BufferCount = bufferCount;
	m_BlocksCount = blocksCount;
}

void C_Renderer::Accept(SceneGraph::I_Visitor& visitor) const
{
	for(Managers::ConstIterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->Accept(visitor);
	}

	visitor.Visit(*this);
}

void C_Renderer::OnGather(const SceneGraph::C_View& view) const
{
	// pass cull
	if(view.RenderPassEnabled(m_PassCull.RenderPasses()))
	{
		// node type cull
		if(view.RenderNodeEnabled(m_NodeFlag))
		{
			PERF_EVENT("DynamicEffect.Gather");
			{
				for(S32 i = 0; i < m_EffectsSorted.Size(); ++i)
				{
					I_DynamicEffect* pEffect = m_EffectsSorted[i].pEffect;
					// only do the virtual call if emitter has some blocks
					if(pEffect->GetNumBlocks() > 0)
						pEffect->Gather(view);
				}
			}

			{
				for(Managers::ConstIterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
				{
					I_DynamicEffectManager* pManager = *pIterator;
					pManager->Gather(view);
				}
			}
		}
	}
}

void C_Renderer::CleanEffect(const I_DynamicEffect *pEffect)
{
	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->CleanEffect(pEffect);
	}
}

void C_Renderer::CleanNode(const C_Node* pNode)
{
	// when an external transformation is removed remove any blocks which are using it
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		I_DynamicEffect* pEffect = pIterator->GetValue();
		// only bother with virtual call if effect has any local space blocks
		if(pEffect->GetNumBlocksLocal() > 0)
			pEffect->Clean(pNode);
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->CleanNode(pNode);
	}
}

void C_Renderer::AddManager(I_DynamicEffectManager *pManager)
{
	assert(m_Managers.Find(pManager) == m_Managers.End());
	m_Managers.Insert(pManager);
}

void C_Renderer::RemoveManager(I_DynamicEffectManager *pManager)
{
	Managers::Iterator it = m_Managers.Find(pManager);
	assert(it != m_Managers.End());
	if(it != m_Managers.End())
		m_Managers.Erase(it);
}

C_Texture* C_Renderer::LoadTexture(const Util::C_String& filename)
{
	if(filename.Length() == 0)
		return NULL;

	RESOURCE_MEMORY_ID("DynamicEffect");

	Util::C_String filenameLower = filename.Lower();
	Textures::ConstIterator it;

	if(m_pExternalTextures)
	{
		it = m_pExternalTextures->Find(filenameLower);
		if(it != m_pExternalTextures->End())
		{
			C_Texture* pTexture = it->GetValue();
			pTexture->Acquire();

			return pTexture;
		}
	}
	
	it = m_Textures.Find(filenameLower);
	if(it != m_Textures.End())
	{
		// texture already exists, increase ref count and return pointer
		C_Texture* pTexture = it->GetValue();
		pTexture->Acquire();

		return pTexture;
	}
	else
	{
		// texture doesn't exist, try and load it here
		C_Texture* pTexture = 0;

		Display::ResourceBundler::I_ResourceBundle* pBundle = GetResourceBundle();
		if (pBundle)
		{
			pTexture = pBundle->GetTexture2D(filename);
		}

		if (!pTexture)
		{
			pTexture = new C_Texture;

			if (!Display::CreateTexture2DFromFile(GetAssetPath() + filenameLower + ".texture", pTexture))
			{
				if (!Display::CreateTexture2DFromFile(GetAssetPath() + filenameLower + ".dds", pTexture))
				{
					if (!Display::CreateTexture2DFromFile(GetAssetPath() + filenameLower + ".tga", pTexture))
					{
						delete pTexture;
						return NULL;
					}
					else
					{
						LOGWARN(g_Log, "DynamicEffect: Texture '%s' was loaded from a tga, please replace with a .texture or bundle.\n", filenameLower.Chars());
					}
				}
				else
				{
					LOGWARN(g_Log, "DynamicEffect: Texture '%s' was loaded from a dds, please replace with a .texture or bundle.\n", filenameLower.Chars());
				}
			}
		}

		if(pTexture->IsCreated())
		{
			m_Textures.Insert(Textures::DataType(filenameLower, pTexture));
		}
		else
		{
			delete pTexture;
			pTexture = NULL;
		}

		return pTexture;
	}
}

U32 C_Renderer::RemoveTextures()
{
	U32 nNum = 0;
	// go through textures and destroy any which aren't being used anymore
	for(Textures::Iterator pIterator = m_Textures.Begin(); pIterator != m_Textures.End(); )
	{
		Textures::Iterator pNext = pIterator;
		++pNext;
		C_Texture* pTexture = pIterator->GetValue();
		//Only remove non-bundled textures with a ref count of zero.
		if(pTexture->RefCount() == 0)
		{
			if(!(pTexture->GetFlags() & Display::E_Usage_Bundled))
			{
				Display::Device()->Destroy(pTexture);

				// make sure any I_DynamicEffects don't use this texture
				for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
				{
					I_DynamicEffect* pEffect = pIterator->GetValue();
					pEffect->Clean(pTexture);
				}

				delete pTexture;
			}

			m_Textures.Erase(pIterator);
			pIterator = m_Textures.Begin();
			++nNum;
		}
		else
			++pIterator;
	}

	return nNum;
}

I_DynamicEffect* C_Renderer::FindEffect(const U32 id)
{
	Effects::Iterator it = m_Effects.Find(id);
	if(it !=  m_Effects.End())
		return it->GetValue();
	else
		return NULL;
}

void C_Renderer::AddBindPixelConstantsEffect(I_DynamicEffect *pEffect)
{
	m_BindPixelConstantsEffects.PushBack(pEffect);
}

void C_Renderer::RemoveBindPixelConstantsEffect(I_DynamicEffect *pEffect)
{
	for(S32 i = 0; i < m_BindPixelConstantsEffects.Size(); ++i)
	{
		if(m_BindPixelConstantsEffects[i] == pEffect)
		{
			m_BindPixelConstantsEffects.EraseByIndex(i);
			return;
		}
	}
}

void C_Renderer::AddEffect(I_DynamicEffect *pEffect)
{
	if(pEffect->m_pRenderer == NULL)
		pEffect->m_pRenderer = this;

	// make sure this effect doesnt already exist
	assert(m_Effects.Find(pEffect->m_iIndex) == m_Effects.End());

	m_Effects.Insert(Effects::DataType(pEffect->m_iIndex, pEffect));

	EffectSortInfo sortInfo(pEffect, pEffect->m_iTypeIndex);
	m_EffectsSorted.PushBack(sortInfo);
	m_EffectSorter.Sort(m_EffectsSorted);
}

void C_Renderer::RemoveEffect(I_DynamicEffect *pEffect)
{
	Effects::Iterator it = m_Effects.Find(pEffect->m_iIndex);
	assert(it != m_Effects.End());
	if(it != m_Effects.End())
		m_Effects.Erase(it);

	for(S32 i = 0; i < m_EffectsSorted.Size(); )
	{
		if(pEffect == m_EffectsSorted[i].pEffect)
			m_EffectsSorted.EraseByIndex(i);
		else
			++i;
	}
}

void C_Renderer::SetRenderPassMapping(E_RenderPassIndex renderPassIndex, const S32 passType)
{
	m_DeToSgPassMapping[renderPassIndex] = passType;
	BuildRenderPasses();
}

S32 C_Renderer::GetInverseRenderPassMapping(S32 renderPass) const
{
	return m_SgToDePassMapping[renderPass];
}


S32 C_Renderer::GetRenderPassMapping(E_RenderPassIndex renderPassIndex) const
{
	return m_DeToSgPassMapping[renderPassIndex];
}

void C_Renderer::BuildRenderPasses()
{
	m_PassCull = SceneGraph::C_PassCull();

	// resize the mapping array from scene graph render pass to RenderingData pass
	m_SgToDePassMapping.Resize(SceneGraph::I_Renderer::m_pSceneGraph->GetNumRenderPasses());
	for(S32 i = 0; i < m_SgToDePassMapping.Size(); ++i)
	{
		m_SgToDePassMapping[i] = -1;
	}

	for(S32 i = 0; i < E_RenderPassIndex_Count; ++i)
	{
		S32 renderPass = m_DeToSgPassMapping[i];
		if(renderPass > -1)
		{
			m_PassCull.EnableRenderPass( SceneGraph::Flag(renderPass) );
			m_SgToDePassMapping[renderPass] = i;
		}
	}
}

// this function examines the various bitfields and constructs whatever vertex data which can be shared between views
// this should be called at very start of frame after views have been set up
void C_Renderer::BeginFrame() const
{
	PERF_EVENT("DynamicEffect.BeginFrame");

	m_Lights.Clear();
	m_DynamicVertexBuffer.Begin();

	// BeginFrame
	{
		PERF_EVENT("DynamicEffect.Managers.BeginFrame");
		for(Managers::ConstIterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
		{
			I_DynamicEffectManager* pManager = *pIterator;
			pManager->BeginFrame();
		}
	}
	
	// call begin frame on all effects
	{
		PERF_EVENT("DynamicEffect.Effects.BeginFrame");
		for(Effects::ConstIterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
		{
			I_DynamicEffect* pEffect = pIterator->GetValue();
			// only do the virtual call if emitter has some blocks
			if(pEffect->Begin())
				pEffect->BeginFrame();
		}
	}
}

// SceneGraph integration
void C_Renderer::Build(const SceneGraph::C_View& view, const S32 renderPass, SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
{
	// combine particles into batched nodes where possible. replace existing nodes with composite nodes
	if(batch.m_NodeType == m_NodeType)
	{
		const DynamicEffect::C_View& dynamicView = GetView(view.GetIndex());

		// if the previous render pass index is different then we should switch buffers
		dynamicView.m_RenderPassIndex = m_SgToDePassMapping[renderPass];
		if(dynamicView.m_RenderPassIndexPrev != dynamicView.m_RenderPassIndex)
		{
			m_DynamicVertexBuffer.SwitchVertexBuffer();
			dynamicView.m_RenderPassIndexPrev = dynamicView.m_RenderPassIndex;
		}

		for(S32 i = batch.m_StartIndex; i <= batch.m_EndIndex; ++i)
		{
			if(dynamicView.m_CompositeRenderNodes.Size() == dynamicView.m_CompositeRenderNodes.Capacity())
			{
				dynamicView.m_CompositeRenderNodesResize = true;

				// set all remaining nodes to NULL since we cant build them
				while(i <= batch.m_EndIndex)
				{
					visibleNodes[i].SetNode(NULL);
					++i;
				}
			}
			else
			{
				dynamicView.m_CompositeRenderNodes.PushBack();
				C_CompositeRenderNode& compositeNode = dynamicView.m_CompositeRenderNodes.Back();
				compositeNode.m_Type = m_NodeType;

				SceneGraph::C_VisibleNode& node = visibleNodes[i];
				const C_RenderNode* pRenderNode = (C_RenderNode*)node.GetNode();

				compositeNode.lodFade = pRenderNode->m_vLodFade;
				compositeNode.pEffect = pRenderNode->m_pEffect;
				compositeNode.flags = pRenderNode->m_iFlags;
				U32 vertexDataSize = pRenderNode->m_VertexDataSize;
				
				compositeNode.blocks.PushBack(pRenderNode);
				node.SetNode(&compositeNode);
				
				++i;
				{
					S32 j = 1;
					const C_RenderNode* pNextRenderNode = i <= batch.m_EndIndex ? visibleNodes[i].GetNode<C_RenderNode>() : NULL;
					
					while(	pNextRenderNode &&
							compositeNode.pEffect == pNextRenderNode->m_pEffect &&
							compositeNode.lodFade.x == pNextRenderNode->m_vLodFade.x &&
							compositeNode.flags == pNextRenderNode->m_iFlags &&
							(vertexDataSize += pNextRenderNode->m_VertexDataSize) < m_BufferSize &&
							j < g_debugDynamicEffectBatchMax)
					{
						compositeNode.blocks.PushBack(pNextRenderNode);
						visibleNodes[i].SetNode(NULL);
						++i;
						++j;
						pNextRenderNode = i <= batch.m_EndIndex ? visibleNodes[i].GetNode<C_RenderNode>() : NULL;
					}
					--i;
				}

				compositeNode.pEffect->Build(dynamicView, compositeNode);
			}
		}
	}
}

void C_Renderer::EndBuild(const SceneGraph::C_View& view, const S32 renderPass) const
{
	// pass cull
	if(view.RenderNodeEnabled(m_NodeFlag))
	{
		// tell batcher that we have finished building
		const DynamicEffect::C_View& dynamicView = GetView(view.GetIndex());
		dynamicView.GetJobQueueDynamicVertexBufferBatcher().EndBuild(dynamicView);
	}
}

// SceneGraph integration
void C_Renderer::Render(const SceneGraph::C_View& view, SceneGraph::C_Context& context, const SceneGraph::C_VisibleNodes& visibleNodes, const SceneGraph::C_VisibleNodeBatch& batch) const
{
	PERF_EVENT("DynamicEffect.Render");

	assert(batch.m_NodeType == m_NodeType);
	assert(batch.m_EndIndex >= batch.m_StartIndex);

	// say that this uses display. this will cause a context reset for other stuff later
	Display::C_Context* pDisplay = Display::GetContext(&context);

	if(context.GetContextFlags() != pDisplay->GetFlags())
	{
#if PLATFORM_PS3
		pDisplay->SetVertexShaderConstantF(4, (F32*)&view.GetViewSettings3D().GetViewProjMatrix(), 4);
#endif
		context.InvalidateState();
	}
	context.SetContextFlags(pDisplay->GetFlags());

	// get the 
	S32 renderPass = m_SgToDePassMapping[context.GetRenderPassIndex()];
	const DynamicEffect::C_View& dynamicView = GetView(view.GetIndex());

	Bind(pDisplay);
	BindVertexConstants(view, pDisplay);

#ifndef PLATFORM_PS3
	BindPixelConstants(view, pDisplay);
#endif

	// TODO: get rid of this?
	switch(renderPass)
	{
		case E_RenderPassIndex_Opaque:
			BindOpaque(pDisplay);
			break;
		case E_RenderPassIndex_Decal:
			BindDecal(pDisplay);
			break;
		case E_RenderPassIndex_Transparent:
			BindTransparent(pDisplay);
			break;
		case E_RenderPassIndex_TwoDimensional:
			BindTwoDimensional(pDisplay);
			break;
	}

	const I_DynamicEffect* pPrevEffect = NULL;
	U32 prevFlags = 0xFFFFFFFF;
	//const I_DynamicEffectManager* pPrevManager = NULL;

	S32 start = batch.m_StartIndex;
	S32 end = batch.m_EndIndex;

	for(S32 i = start; i <= end - 1; ++i)
	{
		const C_CompositeRenderNode* pNode = visibleNodes[i].GetNode<C_CompositeRenderNode>();
		assert(pNode->GetType() == m_NodeType);
		
		U32 states;

		if(g_debugDynamicEffectStateManager)
		{
			// work out for whether the effect needs to bind itself, or its manager
			states = 0;
			const C_CompositeRenderNode* pNextNode = visibleNodes[i+1].GetNode<C_CompositeRenderNode>();
			const I_DynamicEffect* pEffect = pNode->pEffect;
			U32 flags = pNode->flags;
			const I_DynamicEffect* pNextEffect = pNextNode->pEffect;
			U32 nextFlags = pNextNode->flags;

			states |= pEffect != pPrevEffect ? E_States_BindEffect : 0;
			states |= pEffect != pNextEffect ? E_States_UnbindEffect : 0;
			states |= flags != prevFlags ? E_States_BindEffectVolatile : 0;
			states |= flags != nextFlags ? E_States_BindEffectVolatile : 0;
		
			prevFlags = flags;
			pPrevEffect = pEffect;
		}
		else
			states = 0xFFFFFFFF;

		Render(dynamicView, pDisplay, pNode, states);
	}

	// render at least 1 node
	{
		U32 states;
		const C_CompositeRenderNode* pNode = visibleNodes[end].GetNode<C_CompositeRenderNode>();

		if(g_debugDynamicEffectStateManager)
		{
			states = 0;
			const I_DynamicEffect* pEffect = pNode->pEffect;
			U32 flags = pNode->flags;

			states |= pEffect != pPrevEffect ? E_States_BindEffect : 0;
			states |= flags != prevFlags ? E_States_BindEffectVolatile : 0;
			states |= E_States_UnbindEffectVolatile;
			states |= E_States_UnbindEffect;
		}
		else
			states = 0xFFFFFFFF;

		Render(dynamicView, pDisplay, pNode, states);
	}

	switch(renderPass)
	{
		case E_RenderPassIndex_Opaque:
			UnbindOpaque(pDisplay);
			break;
		case E_RenderPassIndex_Decal:
			UnbindDecal(pDisplay);
			break;
		case E_RenderPassIndex_Transparent:
			UnbindTransparent(pDisplay);
			break;
		case E_RenderPassIndex_TwoDimensional:
			UnbindTwoDimensional(pDisplay);
			break;
	}

	Unbind(pDisplay);
}

void C_Renderer::Render(const C_View& view, Display::C_Context* pContext, const C_CompositeRenderNode* pRenderNode, const U32 states) const
{
	PERF_EVENT("RenderBatch");

	const I_DynamicEffect* pEffect = pRenderNode->pEffect;
	/*if(pRenderNode->pMatrix)
	{
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)pRenderNode->pMatrix, 4);
		//m_bIdentity = false;
	}
	else if(!m_bIdentity)
	{
		Display::C_Matrix identity;
		Display::MatrixSetIdentity(&identity);
		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&identity, 4);
		m_bIdentity = true;
	}*/

	F32 lodDistanceScale = view.GetView()->GetViewData().instanceDrawDistanceScaleRcp * view.GetView()->GetViewData().forwardMultiplier;
	MathsStorage::Float4 lodFade(pRenderNode->lodFade.x * lodDistanceScale, pRenderNode->lodFade.y, 0.0f, 0.0f);
	pContext->SetVertexShaderConstantF(CV_RENDER_SETTINGS_1, (F32*)&lodFade, 1);

	pEffect->Render(view, pContext, *pRenderNode, states);
}

void C_Renderer::OnInsert(SceneGraph::C_SceneGraph& sceneGraph)
{

}

void C_Renderer::OnRemove(SceneGraph::C_SceneGraph& sceneGraph)
{
	
}

void C_Renderer::BindVertexConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const
{
	for(Managers::ConstIterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->BindVertexConstants(view, pContext);
	}
}

void C_Renderer::BindPixelConstants(const SceneGraph::C_View& view, Display::C_Context* pContext) const
{
#ifndef PLATFORM_PS3
	// fogging
	MathsStorage::Float4 fogColour = m_pRenderState->Fog()->GetFogColour();
	MathsStorage::Float4 depthFogParam = m_pRenderState->Fog()->GetDepthFogShaderParameters();
	MathsStorage::Float4 heightFogParam = m_pRenderState->Fog()->GetHeightFogShaderParameters();

	assert(pContext);
	pContext->SetPixelShaderConstantF(CP_RENDER_FOG_COLOUR, (F32*)&fogColour, 1);
	pContext->SetPixelShaderConstantF(CP_RENDER_FOG_DEPTH_SETTINGS, (F32*)&depthFogParam, 1);
	pContext->SetPixelShaderConstantF(CP_RENDER_FOG_HEIGHT_SETTINGS, (F32*)&heightFogParam, 1);
#endif

	for(Managers::ConstIterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->BindPixelConstants(view, pContext);
	}

#ifdef PLATFORM_PS3
	// only PS3 needs to bind pixel constants right now, on 360 effects do it in their render function
	for(S32 i = 0; i < m_BindPixelConstantsEffects.Size(); ++i)
	{
		I_DynamicEffect* pEffect = m_BindPixelConstantsEffects[i];
		pEffect->BindPixelConstants(view, pContext);
	}
#endif
}

void C_Renderer::Bind(Display::C_Context* pContext) const
{
	pContext->SetTexture(0, NULL);
	pContext->SetSamplerState(0, &m_DiffuseSamplerState);

	pContext->SetSamplerState(1, &m_NormalSamplerState);

	if(pContext->GetDepthStencilTexture())
	{
		pContext->SetTexture(2, pContext->GetDepthStencilTexture()->GetGPUTexture());
		pContext->SetSamplerState(2, &m_DepthSamplerState);
	}
	
	pContext->SetSamplerUsageFlags(&m_SamplerUsage);

#ifndef CFG_GOLD
	if(g_debugDynamicEffectWireframe)
	{
		pContext->SetFillMode(Display::E_FillMode_Wireframe);
		pContext->SetCullMode(Display::E_CullMode_None);
	}
#endif

#ifdef PLATFORM_PS3
	pContext->SetVtxAttrUsageFlags(UINT_MAX);
#endif
}

void C_Renderer::Unbind(Display::C_Context* pContext) const
{
	pContext->SetTexture(0, NULL);
	pContext->SetCullMode(Display::E_CullMode_Default);

#ifndef CFG_GOLD
	if(g_debugDynamicEffectWireframe)
		pContext->SetFillMode(Display::E_FillMode_Solid);
#endif
}

void C_Renderer::BindOpaque(Display::C_Context* pContext) const
{
	pContext->SetDepthWriteEnable(Display::E_Bool_True);

#ifndef CFG_GOLD
	if(!g_debugDynamicEffectWireframe && g_debugDynamicEffectCullMode)
#endif
		pContext->SetCullMode(Display::E_CullMode_Default);

	//m_bIdentity = false;
}

void C_Renderer::UnbindOpaque(Display::C_Context* pContext) const
{
	// if we are doing luminance write then set blend mode unknown cause we overwrote the alpha writes to off
//	if(Display::Device()->GetLuminanceWrite() == Display::E_LuminanceWrite_Enabled)
		pContext->SetBlendMode(Display::E_BlendMode_Unknown);
}

void C_Renderer::BindDecal(Display::C_Context* pContext) const
{
	//m_bIdentity = false;
	pContext->SetDepthWriteEnable(Display::E_Bool_False);
	pContext->SetBlendEnable(Display::E_Bool_True);

#ifndef CFG_GOLD
	if(!g_debugDynamicEffectWireframe && g_debugDynamicEffectCullMode)
#endif
		pContext->SetCullMode(Display::E_CullMode_Default);
}


void C_Renderer::UnbindDecal(Display::C_Context* pContext) const
{
	pContext->SetBlendEnable(Display::E_Bool_False);
	pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
}

void C_Renderer::BindTransparent(Display::C_Context* pContext) const
{
	pContext->SetDepthWriteEnable(Display::E_Bool_False);
	pContext->SetCullMode(Display::E_CullMode_None);
	//m_bIdentity = false;
}

void C_Renderer::UnbindTransparent(Display::C_Context* pContext) const
{
}

void C_Renderer::BindTwoDimensional(Display::C_Context* pContext) const
{
	pContext->SetCullMode(Display::E_CullMode_Default);
	pContext->SetDepthEnable(Display::E_Bool_True);
	pContext->SetDepthWriteEnable(Display::E_Bool_True);
	pContext->SetDepthFunc(Display::E_CmpFunc_Always);

	MathsStorage::Float4 vClipVector(2.0f, -2.f, -1.f, 1.f);
	pContext->SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&vClipVector, 1);
}

void C_Renderer::UnbindTwoDimensional(Display::C_Context* pContext) const
{
	//const Display::C_ViewSettings2D viewSettings2D = view.GetViewSettings2D();
	//pContext->SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&viewSettings2D.GetClipTransform(), 1);
	pContext->SetDepthFunc(Display::E_CmpFunc_Greater);
}

bool C_Renderer::JobQueueActive() const
{
	return Scheduler::Active() && m_JobQueuePolicyHandle != -1 && g_debugDynamicEffectUseJobQueue;
}

void C_Renderer::Update(F32 fGameTime, F32 fFrameTime, bool bPaused)
{
	m_fFrameTime = fFrameTime * g_debugDynamicEffectTimeScale;
	m_bPaused = bPaused;

	if(m_bPaused || m_fFrameTime == 0.0f)
		return;

	PERF_EVENT("DynamicEffect.Update");

	// update frame time and frame render time
	m_fTime += m_fFrameTime;
	
	m_fFrameRate = 1.0f / m_fFrameTime;
	m_fFrameTimeRender = m_fFrameTime;
	
	m_fFrameTimeAdjustment = m_fFrameTime * 60.0f;
	if(m_fFrameTimeRender < 1.0f/60.0f)
		m_fFrameTimeRender = 1.0f/60.0f;

	m_vTiming = MathsStorage::Float4A16(m_fFrameTime, m_fFrameRate, m_fFrameTimeRender, m_fFrameTimeAdjustment);

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->Update(m_fFrameTime, m_fFrameRate);
	}

	U32 nElements = 0;
	U32 nBlocks = 0;
	U32 nBlocksLocal = 0;
	{
		PERF_EVENT("DynamicEffect.Effects");
		if(g_debugDynamicEffectSortEffects)
		{
			// n - 1 elements (prefetch)
			for(S32 i = 0; i < m_EffectsSorted.Size() - 1; ++i)
			{
				Maths::Prefetch( m_EffectsSorted[i+1].pEffect );
				I_DynamicEffect* pEffect = m_EffectsSorted[i].pEffect;
				if(pEffect->GetNumBlocks() > 0 || pEffect->AlwaysUpdate())
				{
					pEffect->Update();
					if(g_debugDynamicEffectDebugInfo)
					{
						nBlocks += pEffect->GetNumBlocks();
						nBlocksLocal += pEffect->GetNumBlocksLocal();
//						nElements += pEffect->GetNumElements();
					}
				}
			}
			// last element (no prefetch)
			if(m_EffectsSorted.Size() != 0)
			{
				I_DynamicEffect* pEffect = m_EffectsSorted.Back().pEffect;
				if(pEffect->GetNumBlocks() > 0 || pEffect->AlwaysUpdate())
				{
					pEffect->Update();
					if(g_debugDynamicEffectDebugInfo)
					{
						nBlocks += pEffect->GetNumBlocks();
						nBlocksLocal += pEffect->GetNumBlocksLocal();
//						nElements += pEffect->GetNumElements();
					}
				}
			}
		}
		else
		{
			for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
			{
				I_DynamicEffect* pEffect = pIterator->GetValue();
				if(pEffect->GetNumBlocks() > 0 || pEffect->AlwaysUpdate())
				{
					pEffect->Update();
					if(g_debugDynamicEffectDebugInfo)
					{
						nBlocks += pEffect->GetNumBlocks();
						nBlocksLocal += pEffect->GetNumBlocksLocal();
//						nElements += pEffect->GetNumElements();
					}
				}
			}
		}
	}

	if(g_debugDynamicEffectDebugInfo)
	{
		// update element count 4 times a sec
		m_fCycle += m_fFrameTime;
		if(m_fCycle > 0.25 || bPaused)
		{
			m_nBlocksLocal = nBlocksLocal;
			m_nBlocks = nBlocks;
			m_nElements = nElements;
			m_fCycle = 0.0f;
		}
	}

#ifndef CFG_GOLD
	if(g_debugDynamicEffectRemoveUnusedTextures)
	{
		g_debugDynamicEffectRemoveUnusedTextures = false;
		U32 nNumRemoved = RemoveTextures();
		LOGINFO(g_Log, "DynamicEffect: %u unused textures deallocated.\n", nNumRemoved);
	}
#endif
}

void C_Renderer::OnReset()
{
	U32 nLocalBlocks = 0;

	// finally reset all the currently active blocks
	for(Effects::Iterator pIterator = m_Effects.Begin(); pIterator != m_Effects.End(); pIterator++)
	{
		I_DynamicEffect* pEffect = pIterator->GetValue();
		// only bother with virtual call if effect has any local space blocks
		if(pEffect->GetNumBlocks() > 0)
		{
			nLocalBlocks += pEffect->GetNumBlocksLocal();
			pEffect->Reset();
		}
	}

	for(Managers::Iterator pIterator = m_Managers.Begin(); pIterator != m_Managers.End(); pIterator++)
	{
		I_DynamicEffectManager* pManager = *pIterator;
		pManager->Reset();
	}

	if(nLocalBlocks > 0)
		LOGINFO(g_Log, "DynamicEffectRenderer: %d blocks of local space dynamic effect blocks were not released by their game side owners\n", nLocalBlocks);
}

void C_Renderer::SetBlendMode(Display::C_Context* pContext, const E_BlendMode blendMode) const
{
#ifndef CFG_GOLD
	if(!g_debugDynamicEffectUseBlending || g_debugDynamicEffectWireframe)
	{
		pContext->SetBlendMode(Display::E_BlendMode_Solid);
		pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
		return;
	}
#endif

    switch (blendMode)
    {
		case E_BlendMode_Solid:
			pContext->SetBlendMode(Display::E_BlendMode_Solid);
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
			break;
		case E_BlendMode_Alpha:
		case E_BlendMode_AlphaA8:
		case E_BlendMode_AlphaAndAdditive:
			pContext->SetBlendState(Display::E_Blend_One, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_Zero, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add);
			pContext->SetAlphaTestState(Display::E_Bool_True, 0); 
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_True);
			break;
		case E_BlendMode_Additive:
		case E_BlendMode_AdditiveA8:
		case E_BlendMode_AlphaAdditive:
			pContext->SetBlendState(Display::E_Blend_One, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add, Display::E_Blend_Zero, Display::E_Blend_InvSrcAlpha, Display::E_BlendOp_Add);
			pContext->SetAlphaTestState(Display::E_Bool_False, 0); 
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_True);
			break;
		case E_BlendMode_AlphaSubtractive:
		case E_BlendMode_Subtractive:
		case E_BlendMode_SubtractiveA8:
			pContext->SetBlendMode(Display::E_BlendMode_Subtractive);
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
			break;
		case E_BlendMode_SolidAlphaTest:
			pContext->SetBlendMode(Display::E_BlendMode_SolidAlphaTest);
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
			break;
		case E_BlendMode_SolidAlphaMask:
			pContext->SetBlendMode(Display::E_BlendMode_SolidAlphaMask);
			pContext->SetBlendHighPrecisionEnable(Display::E_Bool_False);
			break;
	}
}

inline U32 Count(U32 n)
{
    U32 c = n;
    c -= (n>>1) & 033333333333;
    c -= (n>>2) & 011111111111;
    c = (c + (c>>3)) & 030707070707;
    return c % 63;
}

// returns 4 light indices packed into a U32
U32 C_Renderer::GetLightMask(const Maths::C_Sphere& blockSphere) const
{
	U32 numLights = 0;
	U32 lightIndices[4];

	for(S32 i = 0; i < m_Lights.Size() && i < 255 && numLights < 4; ++i)
	{
		Maths::C_Sphere lightSphere = m_Lights[i].GetBoundingSphere();
		if(lightSphere.Intersect(blockSphere))
		{
			lightIndices[numLights] = i;
			++numLights;
		}
	}

	while(numLights < 4)
	{
		lightIndices[numLights] = 0xFF;
		++numLights;
	}

	return lightIndices[0] + (lightIndices[1] << 8) + (lightIndices[2] << 16) + (lightIndices[3] << 24);
}

void C_Renderer::SetLightConstants1(Display::C_Context* pDisplay, U32 light) const
{
	MathsStorage::Float4A16 position = m_Lights[light].GetPosition();
	MathsStorage::Float4A16 colour = m_Lights[light].GetColour();

	pDisplay->SetVertexShaderConstantF(CV_RENDER_LIGHT_POSITIONS, (F32*)&position, 1);
	pDisplay->SetVertexShaderConstantF(CV_RENDER_LIGHT_COLOUR, (F32*)&colour, 1);
}


void C_Renderer::SetLightConstants4(Display::C_Context* pDisplay, U32 lights) const
{
	// this function assumes a 4x point light shader is being used
	U32 positionConstant = CV_RENDER_LIGHT_POSITIONS;

	U32 numLights = 0;

	U32 lightIndices[4];
	MathsStorage::Float4 redColours;
	MathsStorage::Float4 greenColours;
	MathsStorage::Float4 blueColours;
	
	// get the 4 light indices which are packed into the U32
	lightIndices[0] = lights & ~0xFFFFFF00;
	lightIndices[1] = (lights >> 8) & ~0xFFFFFF00;
	lightIndices[2] = (lights >> 16) & ~0xFFFFFF00;
	lightIndices[3] = (lights >> 24) & ~0xFFFFFF00;

	for(S32 i = 0; i < 4; ++i)
	{
		// valid indices are 0 to 254
		if(lightIndices[i] != 0xFF)
		{
			MathsStorage::Float4A16 position = m_Lights[lightIndices[i]].GetPosition();
			position.w = 1.0f / m_Lights[lightIndices[i]].GetRadius().x;

			pDisplay->SetVertexShaderConstantF(positionConstant, (F32*)&position, 1);
			MathsStorage::Float4A16 colour = m_Lights[lightIndices[i]].GetColour();
			
			redColours.v[numLights] = colour.x;
			greenColours.v[numLights] = colour.y;
			blueColours.v[numLights] = colour.z;
			++numLights;
			++positionConstant;
		}
	}

	pDisplay->SetVertexShaderConstantF(CV_RENDER_LIGHTS_COLOURS_RED, (F32*)&redColours, 1);
	pDisplay->SetVertexShaderConstantF(CV_RENDER_LIGHTS_COLOURS_GREEN, (F32*)&greenColours, 1);
	pDisplay->SetVertexShaderConstantF(CV_RENDER_LIGHTS_COLOURS_BLUE, (F32*)&blueColours, 1);
}

void GetLodFade(MathsStorage::Float2& lodFade, const F32 drawDistance, const F32 fadeDistance)
{
	F32 fadeStart = fadeDistance != 0.0f ? fadeDistance : drawDistance * g_debugDynamicEffectLodFadeStart;
	F32 fadeEnd = drawDistance;
	lodFade.x = -1.0f / (fadeEnd - fadeStart);
	lodFade.y = 1.0f + fadeStart / (fadeEnd - fadeStart);
}

} // DynamicEffect

