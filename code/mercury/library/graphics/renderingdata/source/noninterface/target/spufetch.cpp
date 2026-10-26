#include "stdafx.h"
#include "renderingdata/context.h"
#include "renderingdata/target.h"
#include "btl/allocators/stackallocator.h"

namespace RenderingData
{

#if !PLATFORM_SPU

const void* MaterialListData::GetDmaData() const
{
	if (m_autoStackAllocator.GetAllocatorPtr() && m_autoStackAllocator.GetAllocatorPtr()->GetNumAllocators() == 1)
	{
		m_dmaData.buffer.pMain = m_autoStackAllocator.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		m_dmaData.buffer.size = m_autoStackAllocator.GetAllocatorPtr()->BytesUsed();
	}
	else
	{
		assert(m_materials.Size() == 0);
		m_dmaData.buffer.pMain = 0;
		m_dmaData.buffer.size = 0;
	}

	m_dmaData.materials = &m_materials;
	return &m_dmaData;
}

#else

void Material::OffsetLocation(S32 offset)
{
	Btl::OffsetLocation(m_pData, offset);
}

void MaterialListData::SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator, Spu::CircularCache* pCache, const S32 offset, const S32 count)
{
	this->~MaterialListData();
	new (this) MaterialListData();

	Spu::Fetch(&m_dmaData, pData, sizeof(m_dmaData));

	// Get the material pointers.
	{
		Spu::Fetch(&m_materials, m_dmaData.materials, sizeof(m_materials));
		U32 size = 0;
		
		if (count > 0)
			size = count * sizeof(Material);
		else
			size = m_materials.Size() * sizeof(Material);
		
		U8 *pData = (U8*)(m_materials.First() + offset);
		U32 iPad = ((U32)pData)&0xf;
		pData -= iPad;
		size += iPad;
		size = Btl::AlignUp(size, 16);

		U8* p = pAllocator->Allocate<U8>(size);
		if (!p)
			DebugBreak();	// OUT OF MEMORY IN SPU RENDER TASK
		Spu::Fetch(p, pData, size);
		S32 offsetLocation = p - pData;
		m_materials.OffsetLocation(offsetLocation);
	}

	m_pCache = pCache;
}

#endif

void RenderListNode::RenderData::OffsetLocation(S32 offset)
{
	primitives.OffsetLocation(offset);
	instances.OffsetLocation(offset);
	instanceBatches.OffsetLocation(offset);
	basePrimitives.OffsetLocation(offset);

	for (int i = 0; i < primitives.Size(); i++)
		primitives[i].OffsetLocation(offset);

	for (int i = 0; i < instances.Size(); i++)
		instances[i].OffsetLocation(offset);

	for (int i = 0; i < instanceBatches.Size(); i++)
		instanceBatches[i].OffsetLocation(offset);

	for (int i = 0; i < basePrimitives.Size(); i++)
		basePrimitives[i].OffsetLocation(offset);
}

void RenderListNode::Primitive::OffsetLocation(S32 offset)
{
	lods.OffsetLocation(offset);
	primitiveStreams.OffsetLocation(offset);
}

void RenderListNode::Instance::OffsetLocation(S32 offset)
{
	primitiveStreams.OffsetLocation(offset);
}

void RenderListNode::InstanceBatch::OffsetLocation(S32 offset)
{
	instance.OffsetLocation(offset);
}

namespace Target
{

void MaterialData::OffsetLocation(S32 offset)
{
	m_data.types.OffsetLocation(offset);

	for (int i = 0; i < m_data.types.Size(); i++)
		m_data.types[i].OffsetLocation(offset);
}

void MaterialTypeData::OffsetLocation(S32 offset)
{
	parameters.OffsetLocation(offset);
}

void RenderListNode::RenderDataTarget::OffsetLocation(S32 offset)
{
	indexBuffers.OffsetLocation(offset);
	vertexBuffers.OffsetLocation(offset);
	textures.OffsetLocation(offset);
	effects.OffsetLocation(offset);
	vertexDeclarations.OffsetLocation(offset);
	primitiveLods.OffsetLocation(offset);
	primitiveTechniques.OffsetLocation(offset);
}

#if !PLATFORM_SPU

void RenderListNode::CopyRenderData(const RenderListNode& node)
{
	this->~RenderListNode();
	new (this) RenderListNode();

	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		m_autoStackAllocatorRender.Set(p);

		if (node.m_autoStackAllocatorRender.GetAllocatorPtr()->GetNumAllocators() == 1)
		{
			S32 originalSize = node.m_autoStackAllocatorRender.BytesUsed();
			U8* pDst = p->Allocate<U8>(originalSize);	
			const U8* pSrc = node.m_autoStackAllocatorRender.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		
			memcpy(pDst, pSrc, originalSize);
			memcpy(&m_renderData, &node.m_renderData, sizeof(m_renderData));

			m_renderData.OffsetLocation(pDst - pSrc);
		}
		else
		{
			m_renderData = node.m_renderData;
		}

		m_autoStackAllocatorRender.End();
	}

	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		m_autoStackAllocatorRenderTarget.Set(p);

		if	(node.m_autoStackAllocatorRenderTarget.GetAllocatorPtr()->GetNumAllocators() == 1)
		{
			S32 originalSize = node.m_autoStackAllocatorRenderTarget.BytesUsed();
			U8* pDst = p->Allocate<U8>(originalSize);	
			const U8* pSrc = node.m_autoStackAllocatorRenderTarget.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		
			memcpy(pDst, pSrc, originalSize);
			memcpy(&m_renderDataTarget, &node.m_renderDataTarget, sizeof(m_renderDataTarget));

			m_renderDataTarget.OffsetLocation(pDst - pSrc);
		}
		else
		{
			m_renderData = node.m_renderData;
		}

		m_autoStackAllocatorRenderTarget.End();
	}

	m_constantMaterialData.CopyData(node.m_constantMaterialData);

	for (int i = 0; i < m_renderDataTarget.textures.Size(); i++)
	{
		assert(m_renderDataTarget.textures[i].pGpuData == node.m_renderDataTarget.textures[i].pGpuData);
	}

	for (int i = 0; i < node.m_materialData.Size(); i++)
	{
		m_materialData[i].CopyFrom(node.m_materialData[i], true);
	}
}

void MaterialData::CopyFrom(const MaterialData& other, bool internalStorage)
{
	m_data.types.Destroy();
	m_autoStackAllocator.Free();

	if (other.GetNumTypes() == 0)
		return;

	if (internalStorage)
	{
		Btl::DynamicStackAllocator* pAllocator = Btl::StackAllocatorSystem::Instance().Begin();
		m_autoStackAllocator.Set(pAllocator);

		if (other.m_autoStackAllocator.GetAllocatorPtr()->GetNumAllocators() == 1)
		{
			S32 size = other.m_autoStackAllocator.BytesUsed();
			U8* pDst = pAllocator->Allocate<U8>(size);
			const U8* pSrc = other.m_autoStackAllocator.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
			
			memcpy(pDst, pSrc, size);
			memcpy(&m_data, &other.m_data, sizeof(Data));

			OffsetLocation(pDst - pSrc);
		}
		else
		{
			*this = other;
		}

		m_autoStackAllocator.End();
	}
	else
	{
		*this = other;
	}
}

const void* RenderListNode::GetDmaData() const
{
	if (!m_dmaData.initialised)
	{
		m_dmaData.renderData.pMain = (U8*) &m_renderData;
		m_dmaData.renderData.size = sizeof(m_renderData);
		assert(m_autoStackAllocatorRender.GetAllocatorPtr()->GetNumAllocators() == 1);
		m_dmaData.renderDataBuffer.pMain = m_autoStackAllocatorRender.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		m_dmaData.renderDataBuffer.size = m_autoStackAllocatorRender.BytesUsed();

		m_dmaData.renderDataTarget.pMain = (U8*) &m_renderDataTarget;
		m_dmaData.renderDataTarget.size = sizeof(m_renderDataTarget);
		assert(m_autoStackAllocatorRenderTarget.GetAllocatorPtr()->GetNumAllocators() == 1);
		m_dmaData.renderDataTargetBuffer.pMain = m_autoStackAllocatorRenderTarget.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		m_dmaData.renderDataTargetBuffer.size = m_autoStackAllocatorRenderTarget.BytesUsed();
		
		m_dmaData.materialData.Resize(m_materialData.Size());

		for (int i = 0; i < m_materialData.Size(); i++)
			m_dmaData.materialData[i] = m_materialData[i].GetDmaData();

		m_dmaData.constantMaterialData = m_constantMaterialData.GetDmaData();

		m_dmaData.renderPassInfo.pMain = (U8*) &m_sceneGraphData.renderPassInfo;
		m_dmaData.renderPassInfo.size = sizeof(m_sceneGraphData.renderPassInfo);

		m_dmaData.pvsData.pMain = (U8*) &m_sceneGraphData.primitivePvs;
		m_dmaData.pvsData.size = sizeof(m_sceneGraphData.primitivePvs);

		m_dmaData.initialised = true;
	}

	return &m_dmaData;
}

const void* MaterialData::GetDmaData() const
{
	m_dmaData.data.pMain = (U8*) &m_data;
	m_dmaData.data.size = sizeof(m_data);

	if (m_autoStackAllocator.GetAllocatorPtr())
	{
		assert(m_autoStackAllocator.GetAllocatorPtr()->GetNumAllocators() == 1);
		m_dmaData.buffer.pMain = m_autoStackAllocator.GetAllocatorPtr()->GetAllocator(0).GetBuffer();
		m_dmaData.buffer.size = m_autoStackAllocator.BytesUsed();
	}
	else
	{
		m_dmaData.buffer.pMain = 0;
		m_dmaData.buffer.size = 0;
	}

	return &m_dmaData;
}

#else

void RenderListNode::Reallocate()
{
}

void RenderListNode::SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator, Spu::CircularCache* pCache)
{
	this->~RenderListNode();
	new (this) RenderListNode();

	//DebugBreak();

	DmaData& dmaData(m_dmaData);
	Spu::Fetch(&dmaData, pData, sizeof(dmaData));
	Btl::SpuFetch(dmaData.materialData);

	if (dmaData.renderDataBuffer.pMain)
	{
		Spu::Fetch(&m_renderData, dmaData.renderData.pMain, dmaData.renderData.size);
	
		U32 size = Btl::AlignUp(dmaData.renderDataBuffer.size, 16);
		U8* p = pAllocator->Allocate<U8>(size);
		if (!p)
			DebugBreak();
		Spu::Fetch(p, dmaData.renderDataBuffer.pMain, size);
		m_renderData.OffsetLocation(p - dmaData.renderDataBuffer.pMain);
	}

	if (dmaData.renderDataTargetBuffer.pMain)
	{
		Spu::Fetch(&m_renderDataTarget, dmaData.renderDataTarget.pMain, dmaData.renderDataTarget.size);

		U32 size = Btl::AlignUp(dmaData.renderDataTargetBuffer.size, 16);
		U8* p = pAllocator->Allocate<U8>(size);
		if (!p)
			DebugBreak();
		Spu::Fetch(p, dmaData.renderDataTargetBuffer.pMain, size);
		m_renderDataTarget.OffsetLocation(p - dmaData.renderDataTargetBuffer.pMain);
	}

	m_constantMaterialData.SpuFetch(dmaData.constantMaterialData, pAllocator, pCache);

	m_materialData.Resize(dmaData.materialData.Size());

	for (int i = 0; i < m_materialData.Size(); i++)
		m_materialData[i].SpuFetch(dmaData.materialData[i], pAllocator);
}

void RenderListNode::SpuFetchRenderPassInfo(const void* pData)
{
	DmaData& dmaData(m_dmaData);
	Spu::Fetch(&dmaData, pData, sizeof(dmaData));

	const void* pMain = m_dmaData.renderPassInfo.pMain;
	U32 size = m_dmaData.renderPassInfo.size;
	assert(pMain && size == sizeof(m_sceneGraphData.renderPassInfo));

	Spu::Fetch(&m_sceneGraphData.renderPassInfo, pMain, m_dmaData.renderPassInfo.size);
	Btl::SpuFetch(m_sceneGraphData.renderPassInfo);
}

void RenderListNode::SpuFetchPvsInfo(const void* pData)
{
	DmaData& dmaData(m_dmaData);
	Spu::Fetch(&dmaData, pData, sizeof(dmaData));

	const void* pMain = m_dmaData.pvsData.pMain;
	U32 size = m_dmaData.pvsData.size;
	assert(pMain && size == sizeof(m_sceneGraphData.primitivePvs));

	Spu::Fetch(&m_sceneGraphData.primitivePvs, pMain, size);
}

void MaterialData::SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator)
{
	DmaData& dmaData(m_dmaData);
	Spu::Fetch(&dmaData, pData, sizeof(dmaData));

	Spu::Fetch(&m_data, dmaData.data.pMain, dmaData.data.size);

	if (dmaData.buffer.pMain)
	{
		U32 size = Btl::AlignUp(dmaData.buffer.size, 16);
		U8* p = pAllocator->Allocate<U8>(size);
		if (!p)
			DebugBreak();
		Spu::Fetch(p, dmaData.buffer.pMain, size);
		OffsetLocation(p - dmaData.buffer.pMain);
	}
}

#endif

} // namespace

const U8* RenderListNode::PvsCellData::GetPvsData(S32 cell) const
{
#if !PLATFORM_SPU

	if (!data.Empty())
	{
		if (offsets.Empty())
			return &data[cell * cellSize];
		else
			return &data[offsets[cell] * cellSize];
	}
	else if (!bits.Empty())
		return bits[cell].Data();
	else
		return 0;

#else

	if (!data.Empty())
	{
		if (offsets.Empty())
			return &data[cell * cellSize];
		else 
		{
			Spu::FetchLocal<S16> pOffset(&offsets[cell]);
			return &data[(*pOffset) * cellSize];
		}
	}

#endif
}

} // namespace
