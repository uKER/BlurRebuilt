#include "stdafx.h"
#include "renderingdata/context.h"
#include "renderingdata/target.h"
#include "renderingdata/debugtextures.h"
#include "renderingstatistics.h"
#include "btl/allocators/stackallocator.h"

#if !PLATFORM_SPU
#include "renderingdata/streaming.h"
#endif

#include "Display.h"
#include <fx/fx.h>
#include <fx/fxb.h>
#include <fx/I_Effect.h>
#include <fx/Effect.h>
#include <fx/Manager.h>
#include <fx/FxContext.h>
#include <fx/RenderStates.h>
#include <display/context/Context.h>
#include <display/DisplayToDisplay2.h>
 
//#pragma optimize("", off)

DECLARE_DEBUG_BOOL(g_renderPrimitives,"Graphics.RenderingData.Render.RenderPrimitives", true, Debug::Persist());
DECLARE_DEBUG_BOOL(g_renderInstances,"Graphics.RenderingData.Render.RenderInstances", true, Debug::Persist());
DECLARE_DEBUG_INT(g_renderInstanceMin,"Graphics.RenderingData.Render.RenderInstanceMin", -1, Debug::Persist());
DECLARE_DEBUG_INT(g_renderInstanceMax,"Graphics.RenderingData.Render.RenderInstanceMax", -1, Debug::Persist());
DECLARE_DEBUG_BOOL(g_renderOpaque,"Graphics.RenderingData.Render.RenderOpaque", true, Debug::Persist());
DECLARE_DEBUG_BOOL(g_renderAlphaTest,"Graphics.RenderingData.Render.RenderAlphaTest", true, Debug::Persist());
DECLARE_DEBUG_BOOL(g_renderOverlay,"Graphics.RenderingData.Render.RenderOverlay", true, Debug::Persist());
DECLARE_DEBUG_BOOL(g_renderBackFront,"Graphics.RenderingData.Render.RenderBackFront", true, Debug::Persist());
DECLARE_DEBUG_BOOL(g_debugColourEnable,"Graphics.RenderingData.Render.EnableDebugColour", false);
DECLARE_DEBUG_INT(g_debugColour,"Graphics.RenderingData.Render.DebugColour", -1, Debug::Persist(), Debug::Range(-1, 4));
DECLARE_DEBUG_INT(g_debugColourSeed,"Graphics.RenderingData.Render.DebugColourSeed", 0, Debug::Persist());
DECLARE_DEBUG_FLOAT(g_debugColourAlpha,"Graphics.RenderingData.Render.DebugColourAlpha", 0.f);
DECLARE_DEBUG_BOOL(g_showMatchingTextures,"Graphics.RenderingData.Render.ShowSelectedTextures", true);
DECLARE_DEBUG_BOOL(g_showMatchingBuffers,"Graphics.RenderingData.Render.ShowSelectedBuffers", true);
DECLARE_DEBUG_BOOL(g_showMatchingNodes,"Graphics.RenderingData.Render.ShowSelectedNodes", true);
DECLARE_DEBUG_INT(g_minIndexCount,"Graphics.RenderingData.Render.MinIndexCount", 0);

#if !PLATFORM_SPU && !CFG_SPEED
#define ENABLE_CHUNKDEBUG 1
//#define ENABLE_DEBUGCOLOUR 1
#endif

#if ENABLE_CHUNKDEBUG
extern Debug::Int g_chunkDebugIndex;
extern Debug::Int g_chunkDebugIndexOffset;
#endif

#if PLATFORM_SPU
void jqCheckStack();
#else
void jqCheckStack() {}
#endif

DECLARE_DEBUG_INT(g_dcbt, "Graphics.RenderingData.Render.DCBT", 0);

static U8 g_dest;

// Currently, turning this on reduces L2 misses reported by PIX 
// but makes no difference to the frame rate.
void DCBT_DEBUG(U32 offset, const void* base) {}
/*{
	if (g_dcbt == 0)
	{
		return;
	}
	else if (g_dcbt == 1)
	{
		DCBT(offset, (U8*)base);
	}
	else if (g_dcbt == 2)
	{
		if (base)
		{
			U8* addr = ((U8*) base) + offset;
			g_dest = *addr;
		}
	}
}*/

namespace Display
{
	extern const int g_iAniso[];
};

namespace RenderingData
{
namespace Target
{

DebugTextures g_DebugTextures;

#if ENABLE_DEBUGCOLOUR
extern FX::ImmediateParameter g_pFXDebugColour;
#endif

DEFINE_CONTEXT(Context, Context);

Context::Context()
{
	m_pScene = NULL;
	m_pNodeSource = NULL;
	m_pNodeCurrent = NULL;

	memset(&m_dynamicStreamData, 0, sizeof(m_dynamicStreamData));

	m_renderFlags.data = 0xFFFFFFFF;

	m_abstract = false;

#if PLATFORM_SPU
#if CFG_DEBUG
	m_allocators[0].Initialise(1024 * 32);
#elif CFG_RELEASE
	m_allocators[0].Initialise(1024 * 56);
#else
	m_allocators[0].Initialise(1024 * 64);
#endif
	m_allocators[1].Initialise(1024 * 2);
	m_allocators[2].Initialise(1024 * 2);
	m_allocators[3].Initialise(1024 * 2);

	{
		static U8 p[2048];
		m_cache.Initialise(2048, p);
	}

#endif
}

Context::~Context()
{
}

void Context::Initialise()
{
	m_pDisplay = Display::GetContext(m_pParent);
	m_pFX = FX::GetContext(m_pParent);

	// dependant on Fx and Display
	m_Flags = m_Id | m_pFX->GetId() | m_pDisplay->GetId();

#if PLATFORM_SPU
	m_pVtxBuffer = NULL;
	m_pIdxBuffer = NULL;
#endif
}

void Context::CloneState(const I_Context *pContext)
{
	InvalidateState();

	if(pContext)
	{
		m_renderFlags.data = Spu::Get(&((Context*)pContext)->m_renderFlags.data);
	}
}

void Context::InvalidateState()
{
	SetScene(0);	
	SetNode(0);

	m_state.pVertexDeclaration = 0;
	m_state.pIndexBuffer = 0;
	m_state.pEffect = 0;
	m_state.renderPass = (RenderPass)-1;
	m_state.techniqueBits = -1;
	m_state.blendState = -1;

	RenderingStatistics::Instance().BeginContext();
}

void Context::SetRenderFlags(const RenderFlags renderFlags)
{
	m_renderFlags = renderFlags;
}

void Context::SetDynamicStreamData(const RenderingData::Target::PrimitiveStream* pPrimitiveStreams, S32 nPrimitives, S32 primitiveOffset)
{
	m_dynamicStreamData.pPrimitiveStreams = pPrimitiveStreams;
	m_dynamicStreamData.nPrimitiveStreams = nPrimitives;
	m_dynamicStreamData.primitiveOffset = primitiveOffset;
}

void Context::SetDynamicVertexBuffers(const RenderingData::Target::VertexBufferData* pVertexBufferData, S32 nVertexBuffers)
{
	m_dynamicStreamData.pVertexBuffers = pVertexBufferData;
	m_dynamicStreamData.nVertexBuffers = nVertexBuffers;
}

void Context::RenderList(const RenderListNode* pRenderListNode)
{
	PERF_EVENT("RenderingData::Context::RenderList");

	if (m_state.renderPass == RenderingData::RENDER_PASS_OPAQUE && !g_renderOpaque) return;
	if (m_state.renderPass == RenderingData::RENDER_PASS_ALPHA_TEST && !g_renderAlphaTest) return;
	if (m_state.renderPass == RenderingData::RENDER_PASS_OVERLAY && !g_renderOverlay) return;
	if (m_state.renderPass == RenderingData::RENDER_PASS_BACK_FRONT && !g_renderBackFront) return;

	SetNode(pRenderListNode);

	const RenderListNode::RenderPassInfo& renderPassInfo(pRenderListNode->GetRenderPassInfo(m_state.renderPass));

	if (g_renderPrimitives)
	{
		PERF_EVENT("RenderingData::Context::RenderList::Primitives");

		for (int i = renderPassInfo.primitiveOffset; i < renderPassInfo.primitiveOffset + renderPassInfo.nPrimitives; i++)
		{
			DCBT_DEBUG(128, &pRenderListNode->m_renderData.primitives[i]);
			RenderPrimitive(i);
		}
	}

	if (g_renderInstances)
	{
		PERF_EVENT("RenderingData::Context::RenderList::Instances");

		for (int i = renderPassInfo.instanceOffset; i < renderPassInfo.instanceOffset + renderPassInfo.nInstances; i++)
		{
			if (g_renderInstanceMin >= 0 && (i < g_renderInstanceMin || i > g_renderInstanceMax))
				continue;

			DCBT_DEBUG(128, &pRenderListNode->m_renderData.instances[i]);
			RenderPrimitiveInstance(i);
		}
	}
}


//int g_debugLod = 0;
DECLARE_DEBUG_INT(g_debugLod,"Graphics.RenderingData.Render.LOD", 0, Debug::Range(0, 50));

void Context::RenderPrimitive(const S32 i, U32 techniqueBits, S32 lod)
{
	RenderingStatistics::Instance().BeginPrimitive(i);

	const RenderListNode::Primitive& primitive = m_pNodeCurrent->m_renderData.primitives[i];
	DCBT_DEBUG(128, &primitive);

	S32 effect = primitive.effect;

	assert(m_pNodeCurrent->m_renderDataTarget.effects[effect]);

#ifndef CFG_SPEED
	if (g_debugLod > 0)
		lod = g_debugLod-1;
#endif

	FX::Effect* pEffect = m_pNodeCurrent->m_renderDataTarget.effects[effect];

	if (m_state.pEffect != pEffect
		|| m_state.techniqueBits != techniqueBits)
		SetEffect(pEffect, techniqueBits);

	if (m_instanceMaterial.data != primitive.instanceMaterial.data)
	{
		m_instanceMaterial = primitive.instanceMaterial;
		m_tempMaterial = false;
		SetMaterial(primitive.instanceMaterial);
#ifndef CFG_SPEED
		m_shouldRender.instanceMaterial = m_tempMaterial;
#endif
	}

#if ENABLE_DEBUGCOLOUR
	if (g_debugColourEnable && g_pFXDebugColour != -1)
	{
		U32 hash = g_debugColourSeed;

		if (g_debugColour == 0)
			hash = Btl::Hash(m_pNodeSource, hash);
		else if (g_debugColour == 1)
			hash = Btl::Hash(&primitive, hash);
		else if (g_debugColour == 2)
			hash = Btl::Hash(primitive.indexBufferIndex, hash);
		else if (g_debugColour == 3)
			hash = Btl::Hash(m_pNodeCurrent->GetLocation().GetBlock(), hash);
		else if (g_debugColour == 4)
			hash = Btl::Hash(primitive.primitiveStreams[0].material.data, hash);

		float r = (hash & 0x000000ff) / 255.f;
		float g = ((hash & 0x0000ff00) >> 8) / 255.f;
		float b = ((hash & 0x00ff0000) >> 16) / 255.f;
		float a = g_debugColourAlpha;
		MathsStorage::Float4 c(r, g, b, a);
		m_pFX->SetParameterImmediate(g_pFXDebugColour, &c);
	}

	for (int stream = 0; stream < MAX_STREAMS; stream++)
		m_shouldRender.streams[stream].active = false;
#endif

	const Display2::C_VertexDeclaration* pvd = m_pNodeCurrent->m_renderDataTarget.vertexDeclarations[primitive.vertexDeclaration];

	if (m_state.pVertexDeclaration != pvd)
		SetVertexDeclaration(pvd);

	if (m_dynamicStreamData.pPrimitiveStreams)
	{
		S32 index = i - m_dynamicStreamData.primitiveOffset;
		assert(index > -1 && index < m_dynamicStreamData.nPrimitiveStreams);
		const RenderingData::Target::PrimitiveStream& stream(m_dynamicStreamData.pPrimitiveStreams[i - m_dynamicStreamData.primitiveOffset]);
		assert(stream.vertexBuffer > -1 && stream.vertexBuffer < m_dynamicStreamData.nVertexBuffers);
		SetDynamicPrimitiveStream(stream);
	}

	RenderPrimitive(primitive, lod);

#if (!CFG_SPEED && !PLATFORM_SPU)
	if (g_DebugTextures.m_pSelectedTexture && m_shouldRender.Test())
		m_pScene->GetStreamData().AddDebugBounds(m_pNodeCurrent->m_sceneGraphData.primitiveBounds[i]);
#endif
}

void Context::RenderPrimitiveInstance(const S32 i, U32 techniqueBits)
{
	RenderingStatistics::Instance().BeginInstance(i);

	const RenderListNode::Instance& instance = m_pNodeCurrent->m_renderData.instances[i];
	DCBT_DEBUG(128, &instance);

	S32 basePrimitiveIndex = instance.primitiveIndex;
	const RenderListNode::Primitive& primitive = m_pNodeCurrent->m_renderData.basePrimitives[basePrimitiveIndex];
	
	S32 lod = g_debugLod; // to do: set externally
	S32 effect = primitive.effect;

	assert(m_pNodeCurrent->m_renderDataTarget.effects[primitive.effect]);

	FX::Effect* pEffect = m_pNodeCurrent->m_renderDataTarget.effects[effect];

	if (m_state.pEffect != pEffect
		|| m_state.techniqueBits != techniqueBits)
		SetEffect(pEffect, techniqueBits);

	if (m_instanceMaterial.data != instance.material.data)
	{
		m_instanceMaterial = instance.material;
		m_tempMaterial = false;
		SetMaterial(instance.material);
#ifndef CFG_SPEED
		m_shouldRender.instanceMaterial = m_tempMaterial;
#endif
	}

#if ENABLE_DEBUGCOLOUR
	if (g_pFXDebugColour != -1)
	{
		U32 hash = g_debugColourSeed;

		if (g_debugColour == 0)
			hash = Btl::Hash(m_pNodeSource, hash);
		else if (g_debugColour == 3)
			hash = Btl::Hash(m_pNodeCurrent->GetLocation().GetBlock(), hash);
	
		float r = (hash & 0x000000ff) / 255.f;
		float g = ((hash & 0x0000ff00) >> 8) / 255.f;
		float b = ((hash & 0x00ff0000) >> 16) / 255.f;
		MathsStorage::Float4 c(r, g, b, 0.f);
		m_pFX->SetParameterImmediate(g_pFXDebugColour, &c);
	}

	for (S32 stream = 0; stream < MAX_STREAMS; stream++)
		m_shouldRender.streams[stream].active = false;
#endif

	const Display2::C_VertexDeclaration* pvd = m_pNodeCurrent->m_renderDataTarget.vertexDeclarations[primitive.vertexDeclaration];

	if (m_state.pVertexDeclaration != pvd)
		SetVertexDeclaration(pvd);
	
	for (int stream = 0; stream < instance.primitiveStreams.Size(); stream++)
	{
		if (!SetPrimitiveStream(instance.primitiveStreams[stream]))
			continue;
	}

	RenderPrimitive(primitive, lod);

#if (!CFG_SPEED && !PLATFORM_SPU)
	if (g_DebugTextures.m_pSelectedTexture && m_shouldRender.Test())
		m_pScene->GetStreamData().AddDebugBounds(m_pNodeCurrent->m_sceneGraphData.instanceBounds[i]);
#endif
}

#ifndef CFG_SPEED
bool Context::ShouldRender::Test() const
{
	if (always) return true;

	if (node && g_showMatchingNodes) return true;
	if (baseMaterial && g_showMatchingTextures) return true;
	if (instanceMaterial && g_showMatchingTextures) return true;
	if (indexBuffer && g_showMatchingBuffers) return true;

	for (int i = 0; i < MAX_STREAMS; i++)
	{
		if (streams[i].active)
		{
			if (streams[i].material && g_showMatchingTextures) return true;
			if (streams[i].vertexBuffer && g_showMatchingBuffers) return true;
		}
	}

	return false;
}
#endif

void Context::RenderPrimitive(const RenderListNode::Primitive& primitive, S32 lod)
{
	lod = Maths::Min(primitive.lods.Size() - 1, lod);

	DCBT_DEBUG(128, &primitive);
	DCBT_DEBUG(128, &primitive.lods[lod]);
	DCBT_DEBUG(128, &primitive.primitiveStreams[0]);

	const Display2::C_IndexBuffer* pIndexBuffer = m_pNodeCurrent->m_renderDataTarget.indexBuffers[primitive.indexBufferIndex];
	
	if (!pIndexBuffer || primitive.lods[lod].nVertices == 0)
		return;

	if (m_baseMaterial.data != primitive.baseMaterial.data)
	{
		m_baseMaterial = primitive.baseMaterial;
		m_tempMaterial = false;
		SetMaterial(primitive.baseMaterial);
#ifndef CFG_SPEED
		m_shouldRender.baseMaterial = m_tempMaterial;
#endif
	}

#if ENABLE_CHUNKDEBUG
	if (!m_pNodeCurrent->indexBufferDebug.Empty())
		m_shouldRender.indexBuffer = (g_chunkDebugIndex > -1 && m_pNodeCurrent->indexBufferDebug[primitive.indexBufferIndex] == g_chunkDebugIndex + g_chunkDebugIndexOffset);
#endif

	if (m_state.pIndexBuffer != pIndexBuffer)
		SetIndexBuffer(pIndexBuffer);

	if (m_state.blendState != primitive.blendState)
		SetBlendState(primitive.blendState);

	for (int stream = 0; stream < primitive.primitiveStreams.Size(); stream++)
	{
		if (!SetPrimitiveStream(primitive.primitiveStreams[stream]))
			return;
	}

#ifndef CFG_SPEED
#if !PLATFORM_SPU
	if (!m_shouldRender.Test())
		return;
#endif
#endif

	if (primitive.effect != -1)
		m_pFX->Commit();

	DrawIndexedPrimitive((Display::E_PrimitiveType) primitive.primitiveType, primitive.lods[lod].minIndex, primitive.lods[lod].nVertices, primitive.lods[lod].offset, primitive.lods[lod].nIndices);
}

bool Context::SetPrimitiveStream(const RenderListNode::PrimitiveStream& primitiveStream)
{
	DCBT_DEBUG(128, &primitiveStream);
	S32 stream = primitiveStream.streamIndex;

	if (stream == -1)
		return true;

#if ENABLE_CHUNKDEBUG
	if (!m_pNodeCurrent->vertexBufferDebug.Empty())
	{
		m_shouldRender.streams[stream].vertexBuffer = (g_chunkDebugIndex > -1 && m_pNodeCurrent->vertexBufferDebug[primitiveStream.vertexBuffer] == g_chunkDebugIndex + g_chunkDebugIndexOffset);
		m_shouldRender.streams[stream].active = true;
	}
#endif

	if (m_streamData[stream].vertexBuffer != primitiveStream.vertexBuffer
		|| m_streamData[stream].vertexOffset != primitiveStream.vertexOffset
		|| m_streamData[stream].frequency != primitiveStream.frequency)
	{
		const RenderingData::Target::VertexBufferData& vertexBufferData = m_pNodeCurrent->m_renderDataTarget.vertexBuffers[primitiveStream.vertexBuffer];

		if (vertexBufferData.pBuffer)
		{
			SetStreamData(primitiveStream.streamIndex, vertexBufferData.pBuffer, primitiveStream.vertexOffset, primitiveStream.frequency, vertexBufferData.vertexSize, primitiveStream.vertexBuffer);
		}
		else
		{
			if (vertexBufferData.vertexSize > 0)
				return false;
		}
	}

	if (m_streamData[stream].material.data != primitiveStream.material.data)
	{
		m_tempMaterial = false;
		SetMaterial(primitiveStream.material);
		m_streamData[stream].material = primitiveStream.material;
#ifndef CFG_SPEED
		m_shouldRender.streams[stream].material = m_tempMaterial;
#endif
	}

	return true;
}

void Context::SetDynamicPrimitiveStream(const RenderingData::Target::PrimitiveStream& primitiveStream)
{
	S32 stream = primitiveStream.streamIndex;

	if (m_streamData[stream].vertexBuffer != primitiveStream.vertexBuffer
		|| m_streamData[stream].vertexOffset != primitiveStream.vertexOffset)
	{
		const RenderingData::Target::VertexBufferData& vertexBufferData = m_dynamicStreamData.pVertexBuffers[primitiveStream.vertexBuffer];
		
		if (vertexBufferData.pBuffer)
			SetStreamData(primitiveStream.streamIndex, vertexBufferData.pBuffer, primitiveStream.vertexOffset, 0, vertexBufferData.vertexSize, primitiveStream.vertexBuffer);
	}	
}


void Context::SetNode(const RenderListNode* pNode, const void* pData)
{
	if (m_pNodeSource == pNode)
		return;

	m_state.techniqueBits = -1;
	m_pNodeSource = pNode;

	if (!pNode)
		return;

	RenderingStatistics::Instance().SetNode(pNode);

	m_materialData.Resize(Maths::Max(m_materialData.Size(), 1));

#if !PLATFORM_SPU

	if (0)//!m_abstract)
	{
		//m_localRenderListNodeCopy.CopyRenderData(*pNode);
		//m_pNodeCurrent = &m_localRenderListNodeCopy;
	}
	else
	{
		m_pNodeCurrent = pNode;
	}

	m_materialData[0].pTypes = &pNode->m_constantMaterialTypes;

	if (m_pScene)
	{
		assert(m_pScene->IsActive(pNode));
	}

#ifndef CFG_SPEED
	if (g_chunkDebugIndex > -1 && m_pNodeCurrent->m_chunkDebug == g_chunkDebugIndex + g_chunkDebugIndexOffset)
		m_shouldRender.node = true;
	else
		m_shouldRender.node = false;
#endif

#else

	m_allocators[0].Reset();
	m_localRenderListNodeCopy.SpuFetch(pData, &m_allocators[0], &m_cache);
	m_pNodeCurrent = &m_localRenderListNodeCopy;

#endif

	m_pFX->BindSharedParameters(&m_pNodeCurrent->m_renderDataTarget.m_SharedFXUsage, &m_pNodeCurrent->m_renderDataTarget.m_SharedFXMask);

	ResetMaterials();

	m_materialData[0].pDataSource = &m_pNodeCurrent->m_constantMaterialData;
	m_materialData[0].pDataCurrent = &m_pNodeCurrent->m_constantMaterialData;
	m_materialData[0].offset = 0;
	m_materialData[0].count = 0;

	memset(&m_dynamicStreamData, 0, sizeof(m_dynamicStreamData));

#if ENABLE_CHUNKDEBUG
	m_shouldRender.always = (g_chunkDebugIndex == -1) && (g_DebugTextures.m_pSelectedTexture == 0);
#endif

#if !PLATFORM_SPU
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderData.primitives.First());
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderData.basePrimitives.First());
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderData.instances.First());
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderDataTarget.indexBuffers.First());
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderDataTarget.vertexBuffers.First());
	DCBT_DEBUG(0, m_pNodeCurrent->m_renderDataTarget.textures.First());
#endif
}

void Context::SetDynamicMaterialListData(S32 listIndex, const RenderingData::MaterialListData *pMtlList, const void* pData, S32 offset, S32 count)
{
	S32 index = listIndex + 1;

	m_materialData.Resize(Maths::Max(m_materialData.Size(), index + 1));

	if ((m_materialData[index].pDataSource != pMtlList) || (m_materialData[index].offset != offset) || (m_materialData[index].count != count))
	{
#if PLATFORM_SPU
		m_allocators[index].Reset();
		if (pMtlList && pData)
		{
			m_materialData[index].localDataCopy.SpuFetch(pData, &m_allocators[index], &m_cache, offset, count);
			m_materialData[index].pDataCurrent = &m_materialData[index].localDataCopy;
			if (!m_materialData[index].pDataCurrent)
				DebugBreak();
		}
#else
		m_materialData[index].pDataCurrent = pMtlList;
#endif
		m_materialData[index].offset = offset;
		m_materialData[index].count = count;
		m_materialData[index].pDataSource = pMtlList;

		ResetMaterials();
	}
}

#if !PLATFORM_SPU

void Context::SetDynamicMaterialListTypes(S32 listIndex, const RenderingData::MaterialList *pMtlList)
{
	S32 index = listIndex + 1;
	m_materialData.Resize(Maths::Max(m_materialData.Size(), index + 1));
	m_materialData[index].pTypes = pMtlList;
}

#endif

void Context::SetPass(RenderPass renderPass)
{
	if(m_renderFlags.setPass && m_state.renderPass != renderPass)
	{
		m_state.renderPass = renderPass;

		m_pDisplay->SetAlphaTestFunc(Display::E_CmpFunc_Greater);

		// temp
		switch (renderPass)
		{
			case RENDER_PASS_OPAQUE:
			case RENDER_PASS_POST_RESOLVE:
			case RENDER_PASS_ALPHA_TEST:
			{
				m_pDisplay->SetDepthEnable(Display::E_Bool_True);
				m_pDisplay->SetBlendEnable(Display::E_Bool_False);
				m_pDisplay->SetDepthWriteEnable(Display::E_Bool_True);
				break;
			}
			case RENDER_PASS_OVERLAY:
			case RENDER_PASS_RACIFICATION:
			case RENDER_PASS_BACK_FRONT:
			{
				m_pDisplay->SetDepthEnable(Display::E_Bool_True);
				m_pDisplay->SetBlendEnable(Display::E_Bool_True);
				m_pDisplay->SetDepthWriteEnable(Display::E_Bool_False);
				break;
			}
			case RENDER_PASS_MAX:	// custom pass
				break;
			default:
				assert(0); // unrecognised pass type
				break;
		}
	}
}

void Context::SetMaterial(const MaterialIndex& materialIndex)
{
	if (!m_materialData[materialIndex.listIndex].pDataSource)
		return;

	ConstMaterialReference material = GetMaterial(materialIndex);
	//const U8 *pMtlData = material.GetData();

	//DCBT_DEBUG(0,pMtlData);
	//DCBT_DEBUG(128,pMtlData);
	//DCBT_DEBUG(128,&material);

	const RenderingData::Target::MaterialData& materialData = m_pNodeCurrent->m_materialData[materialIndex.listIndex];
	S32 typeIndex = material.GetTypeIndex();

#if !CFG_SPEED && !PLATFORM_SPU
	const MaterialType& materialType = m_materialData[materialIndex.listIndex].pTypes->GetMaterialType(typeIndex);

	// this is required for reloadable shaders
	const_cast<RenderingData::Target::MaterialData&>(materialData).UpdateParameters(*m_pScene, m_materialData[materialIndex.listIndex].pTypes);

	assert(materialType.GetNumElements() == materialData.GetNumParameters(typeIndex));
#endif

	RenderingStatistics::Instance().BeginMaterial();

	for (int e = 0; e < materialData.GetNumParameters(typeIndex); e++)
	{
		const MaterialParameterData& parameterData = materialData.GetParameter(typeIndex, e);

		RenderingData::MaterialElementType type = parameterData.type;

#if !CFG_SPEED && !PLATFORM_SPU
		RenderingStatistics::Instance().SetParameter(materialType.GetElement(e).name.Chars(), material.GetData(materialType.GetOffset(e, 0)), materialType.GetElementDataSize(e));
#endif

		if (type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
		{
			// Set a texture parameter.
			//assert(materialType.GetElement(e).count == 1);

			S32 offset = parameterData.offset;
			S32 index = material.GetData<S32>(offset);
			const void* pTexture = m_pNodeCurrent->m_renderDataTarget.textures[index].pGpuData;
			Spu::MemBaseMarker marker;
			DCBT_DEBUG(128, &m_pNodeCurrent->m_renderDataTarget.textures[index]);

			if (pTexture)
			{
				FX::ImmediateParameter pParameter = parameterData.fxParameterData;

				m_pFX->SetTextureParameterImmediate(pParameter, pTexture, m_pNodeCurrent->m_renderDataTarget.textures[index].iSamplerState);

#if !CFG_SPEED && !PLATFORM_SPU
				if (!m_pNodeCurrent->textureDebug.Empty())
				{
					if (g_chunkDebugIndex > -1)
						for (int i = 0; i < m_pNodeCurrent->textureDebug[index].chunks.Size(); i++)
							if (m_pNodeCurrent->textureDebug[index].chunks[i] == g_chunkDebugIndex + g_chunkDebugIndexOffset)
								m_tempMaterial = true;

					if (pTexture && g_DebugTextures.m_pSelectedTexture == pTexture)
						m_tempMaterial = true;
				}
#endif

				RenderingStatistics::Instance().SetTexture();
			}

		}
		else if (type < MATERIAL_ELEMENT_TYPE_BASIC_MAX)
		{
			// Basic material, set shader parameter immediately.
			S32 offset = parameterData.offset;
			FX::ImmediateParameter p = parameterData.fxParameterData;
			m_pFX->SetParameterImmediate(p, material.GetData() + offset);
		}
		else if (type == MATERIAL_ELEMENT_TYPE_TEXTURE_PTR)
		{
			// Basic material, set shader parameter immediately.
			S32 offset = parameterData.offset;
			FX::ImmediateParameter p = parameterData.fxParameterData;
			m_pFX->SetParameterImmediate(p, material.GetData() + offset);
		}
		else if (type == MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX)
		{
			// Recursively set other materials.
			for (int i = 0; i < parameterData.count; i++)
			{		
				S32 offset = parameterData.offset + materialDataTypeSizes[parameterData.type] * i;
				MaterialIndex subMaterial = material.GetData<MaterialIndex>(offset);

				{
					material.Release();
					SetMaterial(subMaterial);
					material.Bind(GetMaterial(materialIndex));
				}
			}
		}
		else if (type == MATERIAL_ELEMENT_TYPE_DATA_INDEX)
		{
			int nMtl = parameterData.count;

			// Set the parameter with data from another material.
			if (nMtl == 1)
			{
				S32 offset = parameterData.offset;
				MaterialIndex materialDataIndex = material.GetData<MaterialIndex>(offset);

				material.Release();

				if (m_materialData.Size() > materialDataIndex.listIndex && m_materialData[materialDataIndex.listIndex].pDataCurrent && 
					m_materialData[materialDataIndex.listIndex].pDataCurrent->GetNumMaterials() > materialDataIndex.index
					)
				{
					ConstMaterialReference material2 = GetMaterial(materialDataIndex);
					const U8 *pMtlData2 = material2.GetData();

					FX::ImmediateParameter p = parameterData.fxParameterData;
					
					const RenderingData::Target::MaterialData& materialData2 = m_pNodeCurrent->m_materialData[materialDataIndex.listIndex];
					if (((parameterData.fxParameterData>>32)&0xffffff) == 0xffffff &&
						 (parameterData.fxParameterData&0x10000000L))
						m_pFX->SetTextureParameterImmediate(parameterData.fxParameterData, *(void**)pMtlData2, 0);	// non-shared parameter
					else
						m_pFX->SetParameterImmediate(parameterData.fxParameterData, pMtlData2);						// shared parameter
				}
				material.Bind(GetMaterial(materialIndex));
			}
			else
			{
				FX::ImmediateParameter p = parameterData.fxParameterData;

				// this code is used for gathering bone matrices into a matrix palette for skinning
				F32 *pBuffer = (F32*)alloca(nMtl*12*sizeof(F32));
				F32 *pCurBuffer = pBuffer;

				jqCheckStack();

				// alloca a buffer to temporarily store the gathered material data in
				for (int i=0; i<nMtl; i++)
				{
					S32 offset = parameterData.offset + materialDataTypeSizes[parameterData.type] * i;
					MaterialIndex materialDataIndex = material.GetData<MaterialIndex>(offset);

					material.Release();
					{
						// TODO: [rthomas]	This is a temp fix until I can work out what is causing this materialDataIndex to be out of range
						if (materialDataIndex.listIndex < m_materialData.Size() && m_materialData[materialDataIndex.listIndex].pDataCurrent != NULL)
						{
							if ((materialDataIndex.index + m_materialData[materialDataIndex.listIndex].offset) < m_materialData[materialDataIndex.listIndex].pDataCurrent->GetNumMaterials())
							{
								ConstMaterialReference material2 = GetMaterial(materialDataIndex);
								if (material2.GetData())
									memcpy(pCurBuffer, material2.GetData(), sizeof(F32)*12);
								else
									memset(pCurBuffer, 0, sizeof(F32)*12);
							}
							else
							{
								memset(pCurBuffer, 0, sizeof(F32)*12);
							}
						}
						else
						{
							memset(pCurBuffer, 0, sizeof(F32)*12);
						}
						pCurBuffer += 12;
					}
					material.Bind(GetMaterial(materialIndex));
				}

				m_pFX->SetParameterImmediate(p, pBuffer);
			}
		}
	}
 
	RenderingStatistics::Instance().EndMaterial();
}

void Context::SetAbstract(bool a)
{
	m_abstract = a;
}

void Context::SetScene(const Scene *pScene)
{
	// check if scene has changed
	if(pScene != m_pScene)
	{
		m_pScene = pScene;
#if !PLATFORM_SPU
		if (pScene)
			RenderingStatistics::Instance().BeginScene(pScene->GetDebugName());
#endif
	}
}

void Context::BeginRender()
{
}

void Context::EndRender()
{
	if (m_state.pEffect)
		m_pFX->EndPass();

	SetIndexBuffer(0);

	for (int i = 0; i < m_streamData.Size(); i++)
		SetStreamData(i, 0, 0, 0, 0, -1);

	m_pScene = NULL;
}

void Context::ResetMaterials()
{
	for (int i = 0; i < m_streamData.Size(); i++)
		m_streamData[i].Reset();

	m_instanceMaterial.data = -1;
	m_baseMaterial.data = -1;
}

void Context::StreamData::Reset()
{
	material.listIndex = -1;
	vertexBuffer = -1;
	vertexOffset = -1;
	frequency = -1;
}

void Context::SetBlendState(S32 i)
{
	m_state.blendState = i;
	m_pFX->SetRenderState(i);
	RenderingStatistics::Instance().SetBlendState();
}

void Context::SetEffect(FX::Effect* pEffect, U32 techniqueBits)
{
	if (m_state.pEffect)
		m_pFX->EndPass();

	if (pEffect)
	{
		S32 technique = m_pFX->TechniqueFromTechniqueHandle(pEffect, techniqueBits);
		m_pFX->BeginPass(pEffect, technique);

		RenderingStatistics::Instance().SetEffect(pEffect);
	}

	m_state.pEffect = pEffect;
	m_state.techniqueBits = techniqueBits;

	for (int i = 0; i < m_streamData.Size(); i++)
	{
		m_streamData[i].material.listIndex = -1;
		m_streamData[i].material.index = -1;
		if (m_streamData[i].vertexBuffer > -1)
			SetStreamData(i, 0, 0, 0, 0, -1);
	}
}

void Context::SetVertexDeclaration(const Display2::C_VertexDeclaration* pvd)
{
	m_state.pVertexDeclaration = pvd;

#if PLATFORM_SPU
	Spu::Fetch(&m_VtxDecl, pvd, sizeof(Display2::C_VertexDeclaration));
	m_pDisplay->SetVertexDeclaration(&m_VtxDecl);
#else
	m_pDisplay->SetVertexDeclaration(pvd);
#endif
	RenderingStatistics::Instance().SetVertexDeclaration((U32)(INTPTR)pvd);	// [Blur2PC][x64] statistics key: the low 32 bits
}

void Context::SetStreamData(S32 streamIndex, const Display2::C_VertexBuffer* pVertexBuffer, S32 vertexOffset, S32 frequency, S32 vertexSize, S32 vertexBufferIndex)
{
	m_streamData[streamIndex].vertexBuffer = vertexBufferIndex;
	m_streamData[streamIndex].vertexOffset = vertexOffset;
	m_streamData[streamIndex].frequency = frequency;

	if(pVertexBuffer)
	{
#if PLATFORM_SPU
		if(pVertexBuffer != m_pVtxBuffer)
		{
			m_pVtxBuffer = pVertexBuffer;
			Spu::Fetch(&m_VtxBuffer, pVertexBuffer, sizeof(Display2::C_VertexBuffer));
		}
		m_pDisplay->SetVertexBuffer(streamIndex, &m_VtxBuffer, vertexOffset, vertexSize);
#else
		m_pDisplay->SetVertexBuffer(streamIndex, pVertexBuffer, vertexOffset, vertexSize);
#endif
	}
	RenderingStatistics::Instance().SetVertexBuffer((U32)(INTPTR)pVertexBuffer);	// [Blur2PC][x64] statistics key: the low 32 bits
}

void Context::SetIndexBuffer(const Display2::C_IndexBuffer* pIndexBuffer)
{
	m_state.pIndexBuffer = pIndexBuffer;
	if(pIndexBuffer)
	{
#if PLATFORM_SPU
		if(pIndexBuffer != m_pIdxBuffer)
		{
			m_pIdxBuffer = pIndexBuffer;
			Spu::Fetch(&m_IdxBuffer, pIndexBuffer, sizeof(Display2::C_IndexBuffer));
		}
		m_pDisplay->SetIndexBuffer(&m_IdxBuffer);
#else
		m_pDisplay->SetIndexBuffer(pIndexBuffer);
#endif
	}

	RenderingStatistics::Instance().SetIndexBuffer((U32)(INTPTR)pIndexBuffer);	// [Blur2PC][x64] statistics key: the low 32 bits
}

void Context::DrawIndexedPrimitive(Display::E_PrimitiveType primitiveType, S32 minIndex, S32 nVertices, S32 startIndex, S32 nIndices)
{
#if !CFG_SPEED && !PLATFORM_SPU
	if (g_DebugTextures.bEnable)
	{
		if (g_DebugTextures.m_pTexture0)
		{
			m_pDisplay->SetTexture(0, g_DebugTextures.m_pTexture0->GetGPUTexture());
		}
		if (g_DebugTextures.m_pTexture1)
		{
			m_pDisplay->SetTexture(1, g_DebugTextures.m_pTexture1->GetGPUTexture());
		}
		if (g_DebugTextures.m_pTexture2)
		{
			m_pDisplay->SetTexture(2, g_DebugTextures.m_pTexture2->GetGPUTexture());
		}
		if (g_DebugTextures.m_pTexture3)
		{
			m_pDisplay->SetTexture(3, g_DebugTextures.m_pTexture3->GetGPUTexture());
		}
	}

	RenderingStatistics::Instance().DrawPrimitive(nIndices);

	if (nIndices < g_minIndexCount)
		return;
#endif

	m_pDisplay->DrawIndexedPrimitive(ToPrimitiveType2(primitiveType), 0, minIndex, nVertices, startIndex, nIndices);
}

} // namespace

} // namespace

void ContextRenderTask(Scheduler::Task* pTask)
{
}

//JQ_DECLARE_PPU_MODULE(ContextRenderModule, ContextRenderTask);
//JQ_DECLARE_DUAL_MODULE(ContextRenderModule, ContextRenderTask, contextrender, "Graphics.RenderingData.Memcopy");
