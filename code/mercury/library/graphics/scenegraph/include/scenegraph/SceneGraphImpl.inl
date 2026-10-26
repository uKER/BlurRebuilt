#ifndef SCENE_GRAPH_INL_H
#define SCENE_GRAPH_INL_H

namespace SceneGraph
{

inline const C_RenderPass* C_SceneGraph::GetRenderPass(const S32 passType) const
{
	const Btl::Vector<C_RenderPass> *p = &m_RenderPasses;
	Spu::MemBaseMarker();
	Spu::FetchAlloc(p);
	return &p->At(passType);
}

inline void C_SceneGraph::GetDynamicMaterialList(const RenderingData::Target::MaterialList** ppOutMaterialList, const RenderingData::Target::MaterialList** ppOutMaterialListDmaData,
													  const S16 id, S32 &offset, S32 &count) const
{
#ifdef PLATFORM_SPU
	// TODO: [rthomas]	Use Spu::Fetch etc. in here for consistency with rest of material related thingies
	const uint32_t size = (sizeof(C_CachedStaticNodeInternalMaterialData) + 0xf) & ~0xf;
	uint32_t ea = cellDmaGetUint32((uint64_t) &m_pCachedStaticNodeInternalMaterialData, 0, 0, 0);
	if (!ea)
		return;

	U8 ALIGN16 vPool[size];
	cellDmaGet((uint32_t)&vPool[0], (uint64_t)ea, size, 0, 0, 0);
	cellDmaWaitTagStatusAll(1<<0);
	C_CachedStaticNodeInternalMaterialData* pMaterialData = (C_CachedStaticNodeInternalMaterialData*)&vPool[0];
#else
	C_CachedStaticNodeInternalMaterialData* pMaterialData = m_pCachedStaticNodeInternalMaterialData;
#endif

	if (!pMaterialData || !pMaterialData->m_pMaterialList)
		return;

#if PLATFORM_SPU
	assert(pMaterialData->m_pMaterialListDmaData);
#endif

	*ppOutMaterialList = pMaterialData->m_pMaterialList;
	*ppOutMaterialListDmaData = (const RenderingData::Target::MaterialList*)pMaterialData->m_pMaterialListDmaData;

	if (!pMaterialData->m_pPaletteMap)
		return;

	count = pMaterialData->m_MaterialListSize;

	if (id < 0)
	{
		offset = 0;
		return;
	}

#ifdef PLATFORM_SPU
	volatile U32 index = static_cast<U32>(id) & ~0xf;

	U8 ALIGN16 vBuff[16];
	cellDmaGet((uint32_t)&vBuff[0], (uint64_t)(pMaterialData->m_pPaletteMap + index), 16, 0, 0, 0);
	cellDmaWaitTagStatusAll(1<<0);

	offset = vBuff[id - index] * count;
#else
	offset = pMaterialData->m_pPaletteMap[id] * count;
#endif
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_INL_H
