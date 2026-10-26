#ifndef RD_PLATFORM_CONTEXT_H
#define RD_PLATFORM_CONTEXT_H
#ifdef _MSC_VER
#pragma once
#endif

#include <Display/Device/DeviceContext.h>

#include "system/serialise/i_file.h"

#include "node.h"
#include "renderlist.h"
#include "scene.h"
#include "renderstates.h"
#include "renderlist.h"
#include "target.h"
#include "materialindex.h"
#include "renderflags.h"
#include "renderingdata\primitivestream.h"

namespace Display
{
class C_Context;
}

namespace FX
{
class I_Effect;
struct Parameter;
class Context;
}

// Types created when loading scene data (not required for export).
namespace RenderingData {
namespace Target
{

DECLARE_CONTEXT(Context, Context);

Context *GlobalContext();	 // return pointer to global rendering context

// temporary rendering data
class Context : public Display::I_Context
{
public:
	Context();

	~Context();

	Display::I_Context *New()
	{
#if !PLATFORM_SPU
		return new Context;
#endif
	}

	void Initialise();

	void CloneState(const Display::I_Context *pContext);

	void InvalidateState();

	void SetRenderFlags(const RenderFlags renderFlags);

	RenderFlags	GetRenderFlags() const { return m_renderFlags; }

	void RenderList(const RenderListNode* pRenderListNode);

	void RenderPrimitive(const S32 i, U32 techniqueBits = 0, S32 lod = 0);

	void RenderPrimitiveInstance(const S32 i, U32 techniqueBits = 0);

	void SetScene(const Scene *pScene);

	void SetPass(RenderPass renderPass);

	void SetNode(const RenderListNode* pNode, const void* pData = 0);

	// Offset is from the primitive index in the RenderListNode to the index in the primitive stream array.
	// This function should be called after SetNode().
	// Use SetDynamicVertexBuffers() to provide an array of vertex buffers. 
	void SetDynamicStreamData(const RenderingData::Target::PrimitiveStream* pPrimitiveStreams, S32 nPrimitives, S32 primitiveOffset);
	void SetDynamicVertexBuffers(const RenderingData::Target::VertexBufferData* pVertexBufferData, S32 nVertexBuffers);

#if !PLATFORM_SPU
	void SetDynamicMaterialListTypes(S32 listIndex, const RenderingData::MaterialList *pMtlList);
#endif

	void SetDynamicMaterialListData(S32 listIndex, const RenderingData::MaterialListData *pMtlList, const void* pData, S32 offset, S32 count = 0);

	void SetMaterial(const MaterialIndex& materialIndex);

	void ResetMaterials();

	void SetAbstract(bool a);

#if PLATFORM_SPU
	RenderListNode *GetLocalRenderListCopy() { return &m_localRenderListNodeCopy; }
#endif

private:

	void BeginRender();

	void EndRender();

	void RenderPrimitive(const RenderListNode::Primitive& primitive, S32 lod);

	bool SetPrimitiveStream(const RenderListNode::PrimitiveStream& primitiveStream);

	void SetDynamicPrimitiveStream(const RenderingData::Target::PrimitiveStream& primitiveStream);

	void SetBlendState(S32 i);

	void SetEffect(FX::Effect* pEffect, U32 techniqueBits);

	void SetVertexDeclaration(const Display2::C_VertexDeclaration* pvd);

	void Reset();

	void SetStreamData(S32 streamIndex, const Display2::C_VertexBuffer* pVertexBuffer, S32 vertexOffset, S32 frequency, S32 vertexSize, S32 vertexIndex);

	void SetIndexBuffer(const Display2::C_IndexBuffer* pIndexBuffer);

	void DrawIndexedPrimitive(Display::E_PrimitiveType primitiveType, S32 minIndex, S32 nVertices, S32 startIndex, S32 nIndices);

	inline ConstMaterialReference GetMaterial(const MaterialIndex& material) const
	{
		return m_materialData[material.listIndex].pDataCurrent->GetMaterial(material.index + m_materialData[material.listIndex].offset);
	}

#if PLATFORM_SPU
	void FetchTexture(void **ppTexture, int iTag);
	void FetchTexture(int iTex);
#else
	void FetchTexture(void **ppTexture, int iTag) {}
	void FetchTexture(int iTex) {}
#endif

	RenderFlags						m_renderFlags;

// The scene may be used for debug purposes, but not on SPU.
#if !PLATFORM_SPU
	const Scene*					m_pScene;
#else
	const void*						m_pScene;
#endif

	const RenderListNode* m_pNodeSource;
	const RenderListNode* m_pNodeCurrent;
	MaterialIndex m_baseMaterial;
	MaterialIndex m_instanceMaterial;

	struct State
	{
		const Display2::C_VertexDeclaration*	pVertexDeclaration;
		const Display2::C_IndexBuffer*			pIndexBuffer;
		const FX::Effect* 						pEffect;
		RenderPass								renderPass;
		U32 									techniqueBits;
		S32										blendState;
	};

	State m_state;

	const static int MAX_STREAMS = 4;

	struct StreamData
	{
		void Reset();

		MaterialIndex					material;
		S32								vertexBuffer;
		S32								vertexOffset;
		S32								frequency;
	};

	Btl::Vector<StreamData, Btl::VectorParameters_Array<MAX_STREAMS> >
										m_streamData;

#ifndef CFG_SPEED
	struct ShouldRender
	{
		ShouldRender() {memset(this, 0, sizeof(*this));}

		bool always;
		bool baseMaterial;
		bool instanceMaterial;
		bool indexBuffer;
		bool node;

		struct Stream
		{
			bool active;
			bool material;
			bool vertexBuffer;
		};

		Btl::Vector<Stream, Btl::VectorParameters_Array<MAX_STREAMS> > streams;

		bool Test() const;
	};

	ShouldRender						m_shouldRender;
#endif

	bool m_tempMaterial;

	Display::C_Context*					m_pDisplay;
	FX::Context*						m_pFX;
	
#if PLATFORM_SPU
	Btl::Vector<Btl::StackAllocatorSimple<>, Btl::VectorParameters_Array<4> > m_allocators;
	RenderListNode m_localRenderListNodeCopy;
#endif

	struct MaterialData
	{
		MaterialData() : pTypes(0), pDataSource(0), pDataCurrent(0), offset(0), count(0) {}

#if PLATFORM_SPU
		RenderingData::MaterialListData localDataCopy;
#endif

		const RenderingData::MaterialList* pTypes;
		const RenderingData::MaterialListData* pDataSource;
		const RenderingData::MaterialListData* pDataCurrent;
		S32 offset;
		S32 count;
	};

	Btl::Vector<MaterialData, Btl::VectorParameters_Static<15> > m_materialData;

#if PLATFORM_SPU
	Spu::CircularCache m_cache;
#endif
	
	bool m_abstract;

	struct DynamicStreamData
	{
		const RenderingData::Target::PrimitiveStream* pPrimitiveStreams;
		S32 nPrimitiveStreams;
		S32 primitiveOffset;
		const RenderingData::Target::VertexBufferData* pVertexBuffers;
		S32 nVertexBuffers;
	};

	DynamicStreamData m_dynamicStreamData;

#if PLATFORM_SPU
	// for the SPU, cache the current vertex buffer, index buffer and vertex declaration on the context
	Display2::C_VertexBuffer			m_VtxBuffer;
	const Display2::C_VertexBuffer*		m_pVtxBuffer;
	Display2::C_IndexBuffer				m_IdxBuffer;
	const Display2::C_IndexBuffer*		m_pIdxBuffer;
	Display2::C_VertexDeclaration		m_VtxDecl;
#endif
};

}}

#endif
