#include "stdafx.h"
#include "renderingdata/build.h"
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>

#include <system/DebugLog.h>

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

namespace RenderingData {
namespace Build
{

// Move all the textures from block 0 to new blocks.
// Create one block for each combination of scene elements that uses textures.
void Scene::RemapTextures()
{
	ProcessNodes(&GetRootNode(), &Scene::RemapTextures_GetTextureUsage);
	RemapTextures_CreateBlocks();
	ProcessNodes(&GetRootNode(), &Scene::RemapTextures_MoveTextures);
}

void Scene::RemapTextures_GetTextureUsage(Node& node)
{
	// Find all the materials in the render node.
	if (RenderListNode* pRenderListNode = RenderListNode::GetPointer(&node))
	{
		// Primitives.
		for (int i = 0; i < pRenderListNode->m_renderData.primitives.Size(); i++)
		{
			RenderListNode::Primitive* pPrimitive = &pRenderListNode->m_renderData.primitives[i];

			// Model material.
			MaterialIndex baseMaterialIndex = pRenderListNode->m_renderData.primitives[i].baseMaterial;
			RemapTextures_GetTextureUsage(*pRenderListNode, baseMaterialIndex, pPrimitive);
			
			MaterialIndex instanceMaterialIndex = pRenderListNode->m_renderData.primitives[i].instanceMaterial;
			RemapTextures_GetTextureUsage(*pRenderListNode, instanceMaterialIndex, pPrimitive);

			// Stream materials.
			for (int j = 0; j < pRenderListNode->m_renderData.primitives[i].primitiveStreams.Size(); j++)
			{
				MaterialIndex materialIndex = pRenderListNode->m_renderData.primitives[i].primitiveStreams[j].material;
				RemapTextures_GetTextureUsage(*pRenderListNode, materialIndex, pPrimitive);
			}
		}

		// Base primitives.
		for (int i = 0; i < pRenderListNode->m_renderData.basePrimitives.Size(); i++)
		{
			MaterialIndex materialIndex = pRenderListNode->m_renderData.basePrimitives[i].baseMaterial;
			RenderListNode::Primitive* pPrimitive = &pRenderListNode->m_renderData.basePrimitives[i];
			RemapTextures_GetTextureUsage(*pRenderListNode, materialIndex, pPrimitive);

			// Stream materials.
			for (int j = 0; j < pRenderListNode->m_renderData.basePrimitives[i].primitiveStreams.Size(); j++)
			{
				materialIndex = pRenderListNode->m_renderData.basePrimitives[i].primitiveStreams[j].material;
				RemapTextures_GetTextureUsage(*pRenderListNode, materialIndex, pPrimitive);
			}
		}

		// Instances.
		for (int i = 0; i < pRenderListNode->m_renderData.instances.Size(); i++)
		{
			// Instance material.
			MaterialIndex materialIndex = pRenderListNode->m_renderData.instances[i].material;
			RenderListNode::Instance* pInstance = &pRenderListNode->m_renderData.instances[i];
			RemapTextures_GetTextureUsage(*pRenderListNode, materialIndex, pInstance);
			
			// Stream materials.
			for (int j = 0; j < pRenderListNode->m_renderData.instances[i].primitiveStreams.Size(); j++)
			{
				materialIndex = pRenderListNode->m_renderData.instances[i].primitiveStreams[j].material;
				RemapTextures_GetTextureUsage(*pRenderListNode, materialIndex, pInstance);
			}
		}

		// To do: instance batches.
	}
	else if (MaterialNode* pMaterialNode = MaterialNode::GetPointer(&node))
	{
		Material& material = pMaterialNode->GetMaterial();
		const MaterialType& materialType = pMaterialNode->GetMaterialType();

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
			{	
				S32 offset = materialType.GetOffset(e, 0);
				S32 index = material.GetData<S32>(offset);
				ResourceIndex resourceIndex = GetTexture(index);

				// Point the material data at the entry in the texture array.
				material.SetData<S32>(offset, pMaterialNode->m_textureSource.Size());
				pMaterialNode->m_textureSource.PushBack(resourceIndex);

				if (resourceIndex.GetBlock() == 0) // Only textures in block 0 are remapped.
				{
					// Record this use of the texture.
					m_textureUsageMap[resourceIndex].Insert(&node);
				}
			}
		}
	}
}

void Scene::RemapTextures_GetTextureUsage(RenderListNode& node, MaterialIndex materialIndex, void* id)
{
	if (materialIndex.listIndex == 0)
	{
		const Material& material = node.GetMaterial(materialIndex);
		const MaterialType& materialType = node.GetMaterialType(materialIndex);

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX)
			{
				for (int i = 0; i < materialType.GetElement(e).count; i++)
				{		
					S32 offset = materialType.GetOffset(e, i);
					MaterialIndex subMaterial = material.GetData<MaterialIndex>(offset);
					RemapTextures_GetTextureUsage(node, subMaterial, id);
				}
			}
			else if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
			{	
				S32 offset = materialType.GetOffset(e, 0);
				S32 index = material.GetData<S32>(offset);
				ResourceIndex resourceIndex = node.m_streamerData.textureSource[index];

				if (resourceIndex.GetBlock() == 0) // Only textures in block 0 are remapped.
				{
					// Record this use of the texture.
					m_textureUsageMap[resourceIndex].Insert(id);
				}
			}
		}
	}
}

void Scene::RemapTextures_CreateBlocks()
{
	typedef Btl::Map<Btl::Set<void*>, Btl::Vector<ResourceIndex> > BlockMap;
	BlockMap blockMap;

	// Turn the texture usage data around to get a list of textures in each block.
	for (TextureUsageMap::Iterator it = m_textureUsageMap.Begin(); it != m_textureUsageMap.End(); ++it)
		blockMap[it->GetValue()].PushBack(it->GetKey());

	// Create remapping data for the textures.
	for (BlockMap::Iterator it = blockMap.Begin(); it != blockMap.End(); ++it)
	{
		S32 blockIndex = AddResourceBlock();
		S32 size = 0;

		for (int i = 0; i < it->GetValue().Size(); i++)
		{
			// Don't make the blocks too large.
			if (size >= 1024 * 1024)
			{
				blockIndex = AddResourceBlock();
				size = 0;
			}

			ResourceIndex oldIndex = it->GetValue()[i];
			const Texture& oldTexture(m_resourceBlocks[0].GetTexture(oldIndex.GetIndex()));

			S32 textureIndex = m_resourceBlocks[blockIndex].InsertTexture(oldTexture);
			ResourceIndex newIndex = ResourceIndex::BlockIndex(blockIndex, textureIndex);
			m_bufferRemap[oldIndex] = BufferRemapData(newIndex);

			{
				// to do: use texture info cache
				TextureExport::TextureInfo info;
				ActTools::Texture& texture = *oldTexture.GetTextureManager()->GetTexture(oldTexture.GetCacheIndex(), info);
				size += texture.GetSize();
			}
		}
	}

	ProcessNodes(&GetRootNode(), &Scene::RemapTextures_MoveTextures);
	m_bufferRemap.Clear();

	m_resourceBlocks[0].ClearTextures();
}

void Scene::RemapTextures_MoveTextures(Node& node)
{
	if (RenderListNode* pRenderListNode = RenderListNode::GetPointer(&node))
	{
		for (int i = 0; i < pRenderListNode->m_streamerData.textureSource.Size(); i++)
		{
			ResourceIndex oldIndex = pRenderListNode->m_streamerData.textureSource[i];
			
			if (oldIndex.GetBlock() == 0)
			{
				BufferRemap::Iterator f = m_bufferRemap.Find(oldIndex);
				
				if (f != m_bufferRemap.End())
				{
					ResourceIndex newIndex = f->GetValue().newIndex;
					pRenderListNode->m_streamerData.textureSource[i] = newIndex;
				}
				else
					assert(0);
			}
		}
	}
	else if (MaterialNode* pMaterialNode = MaterialNode::GetPointer(&node))
	{
		for (int i = 0; i < pMaterialNode->m_textureSource.Size(); i++)
		{
			ResourceIndex oldIndex = pMaterialNode->m_textureSource[i];
			
			if (oldIndex.GetBlock() == 0)
			{
				BufferRemap::Iterator f = m_bufferRemap.Find(oldIndex);
				
				if (f != m_bufferRemap.End())
				{
					ResourceIndex newIndex = f->GetValue().newIndex;
					pMaterialNode->m_textureSource[i] = newIndex;
				}
				else
					assert(0);
			}
		}
	}
}

void Scene::GetTextureBounds(Node& node)
{
	// Find all the materials in the render node.
	if (RenderListNode* pRenderListNode = RenderListNode::GetPointer(&node))
	{
		S32 groupIndex = pRenderListNode->GetGroupIndex();

		// Primitives.
		for (int i = 0; i < pRenderListNode->m_renderData.primitives.Size(); i++)
		{
			const Btl::BitVector& pvsBits = pRenderListNode->m_streamerData.cellPvsPrimitives[i];
			const Btl::BitVector& roomBits = pRenderListNode->m_streamerData.primitiveRooms[i];

			// Model material.
			RenderListNode::Primitive* pPrimitive = &pRenderListNode->m_renderData.primitives[i];

			MaterialIndex baseMaterialIndex = pRenderListNode->m_renderData.primitives[i].baseMaterial;
			GetTextureBounds(*pRenderListNode, baseMaterialIndex, pRenderListNode->m_sceneGraphData.primitiveBounds[i], pvsBits, groupIndex, roomBits);
			
			MaterialIndex instanceMaterialIndex = pRenderListNode->m_renderData.primitives[i].instanceMaterial;
			GetTextureBounds(*pRenderListNode, instanceMaterialIndex, pRenderListNode->m_sceneGraphData.primitiveBounds[i], pvsBits, groupIndex, roomBits);

			// Stream materials.
			for (int j = 0; j < pRenderListNode->m_renderData.primitives[i].primitiveStreams.Size(); j++)
			{
				MaterialIndex materialIndex = pRenderListNode->m_renderData.primitives[i].primitiveStreams[j].material;
				GetTextureBounds(*pRenderListNode, materialIndex, pRenderListNode->m_sceneGraphData.primitiveBounds[i], pvsBits, groupIndex, roomBits);
			}
		}

		// Instances.
		for (int i = 0; i < pRenderListNode->m_renderData.instances.Size(); i++)
		{
			const Btl::BitVector& pvsBits = pRenderListNode->m_streamerData.cellPvsInstances[i];
			const Btl::BitVector& roomBits = pRenderListNode->m_streamerData.instanceRooms[i];

			// Instance material.
			MaterialIndex materialIndex = pRenderListNode->m_renderData.instances[i].material;
			RenderListNode::Instance* pInstance = &pRenderListNode->m_renderData.instances[i];
			GetTextureBounds(*pRenderListNode, materialIndex, pRenderListNode->m_sceneGraphData.instanceBounds[i], pvsBits, groupIndex, roomBits);

			// Stream materials.
			for (int j = 0; j < pRenderListNode->m_renderData.instances[i].primitiveStreams.Size(); j++)
			{
				materialIndex = pRenderListNode->m_renderData.instances[i].primitiveStreams[j].material;
				GetTextureBounds(*pRenderListNode, materialIndex, pRenderListNode->m_sceneGraphData.instanceBounds[i], pvsBits, groupIndex, roomBits);
			}

			// Base primitive.
			{
				S32 baseIndex = pRenderListNode->m_renderData.instances[i].primitiveIndex;
				MaterialIndex materialIndex = pRenderListNode->m_renderData.basePrimitives[baseIndex].baseMaterial;
				RenderListNode::Primitive* pPrimitive = &pRenderListNode->m_renderData.basePrimitives[baseIndex];
				GetTextureBounds(*pRenderListNode, materialIndex, pRenderListNode->m_sceneGraphData.instanceBounds[i], pvsBits, groupIndex, roomBits);

				// Stream materials.
				for (int j = 0; j < pRenderListNode->m_renderData.basePrimitives[baseIndex].primitiveStreams.Size(); j++)
				{
					materialIndex = pRenderListNode->m_renderData.basePrimitives[baseIndex].primitiveStreams[j].material;
					GetTextureBounds(*pRenderListNode, materialIndex, pRenderListNode->m_sceneGraphData.instanceBounds[i], pvsBits, groupIndex, roomBits);
				}
			}
		}

		// To do: instance batches.
	}
	else if (MaterialNode* pMaterialNode = MaterialNode::GetPointer(&node))
	{
		S32 groupIndex = pMaterialNode->GetGroupIndex();

		const Bounds& bounds = pMaterialNode->GetBounds();
		const Btl::BitVector& pvsBits = pMaterialNode->GetPvsBits();
		const Btl::BitVector& roomBits = pMaterialNode->GetPortalBits();

		const Material& material = pMaterialNode->GetMaterial();
		const MaterialType& materialType = pMaterialNode->GetMaterialType();

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
			{	
				S32 offset = materialType.GetOffset(e, 0);
				S32 index = material.GetData<S32>(offset);
				ResourceIndex resourceIndex = pMaterialNode->m_textureSource[index];

				if (!resourceIndex.IsNull() && !resourceIndex.IsShared())
					UpdateResourceBlockBounds(resourceIndex.GetBlock(), bounds, Btl::Hash(&node), pvsBits, groupIndex, roomBits);
			}
		}
	}
}

void Scene::UpdateResourceBlockBounds(S32 blockIndex, const Bounds& bounds, U32 renderNodeHash, const Btl::BitVector& pvsBits, S32 groupIndex, const Btl::BitVector& roomBits)
{
	ResourceBlock& block(m_resourceBlocks[blockIndex]);
	block.m_subBounds.PushBack(bounds);
	block.m_fullBounds.SetUnion(bounds);
	block.m_renderNodeHash = Btl::Hash(renderNodeHash, block.m_renderNodeHash);
	
	if (block.m_pvsBits.Empty())
		block.m_pvsBits = pvsBits;
	else if (!pvsBits.Empty())
		block.m_pvsBits |= pvsBits;

	if (block.m_roomBits.Empty())
		block.m_roomBits = roomBits;
	else if (!roomBits.Empty())
		block.m_roomBits |= roomBits;

	block.m_groups.Insert(groupIndex);
}

void Scene::GetTextureBounds(RenderListNode& node, MaterialIndex materialIndex, const Bounds& bounds, const Btl::BitVector& pvsBits, S32 groupIndex, const Btl::BitVector& roomBits)
{
	if (materialIndex.listIndex == 0)
	{
		const Material& material = node.GetMaterial(materialIndex);
		const MaterialType& materialType = node.GetMaterialType(materialIndex);

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX)
			{
				for (int i = 0; i < materialType.GetElement(e).count; i++)
				{
					S32 offset = materialType.GetOffset(e, i);
					MaterialIndex subMaterial = material.GetData<MaterialIndex>(offset);
					GetTextureBounds(node, subMaterial, bounds, pvsBits, groupIndex, roomBits);
				}
			}
			else if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
			{	
				S32 offset = materialType.GetOffset(e, 0);
				S32 index = material.GetData<S32>(offset);
				ResourceIndex resourceIndex = node.m_streamerData.textureSource[index];

				if (!resourceIndex.IsNull() && !resourceIndex.IsShared())
					UpdateResourceBlockBounds(resourceIndex.GetBlock(), bounds, Btl::Hash(&node), pvsBits, groupIndex, roomBits);
			}
		}
	}
}


}}