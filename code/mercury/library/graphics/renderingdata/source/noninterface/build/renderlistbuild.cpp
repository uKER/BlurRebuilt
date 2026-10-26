#include "stdafx.h"
#include "renderingdata/build.h"

#include <algorithm>
#include "btl/containers/algorithm.h"
#include "btl/containers/map.h"
#include "primitivesorting.h" 

//#pragma optimize("", off)

namespace RenderingData {
namespace Build
{

class RenderListBuilder
{
public:
	RenderListBuilder() : m_pScene(0), m_nPvsCells(0), m_nRooms(0), m_separatePasses(false) {}

	void SetScene(Scene* pScene, bool separatePasses) {m_pScene = pScene; m_separatePasses = separatePasses;}

	bool Empty() {return m_primitives.Empty() && m_instances.Empty() && m_instanceBatches.Empty();}

	void AddModel(const Model* p, const CullNode* pNode);
	void AddInstance(const Instance* p, const CullNode* pNode);
	void AddInstanceBatch(const InstanceBatch* p, const CullNode* pNode);

	void Finish();

	struct SourceBase
	{
		const CullNode* cullNode;
	};

	struct PrimitiveSource : public SourceBase
	{
		const Model* pModel;
		S32 index;
	};

	struct InstanceSource : public SourceBase
	{
		const Instance* pInstance;
		const Model* pBaseModel;
		S32 primitiveIndex;
	};

	struct InstanceBatchSource : public SourceBase
	{
		const InstanceBatch* pInstanceBatch;
	};

	struct RenderListNode
	{
		Btl::AutoPointer<RenderingData::Build::RenderListNode> pNode;

		MaterialList constantMaterials;
		Btl::Map<S32, MaterialList> localDynamicMaterialLists;

		struct ResourceMap
		{
			Btl::Map<ResourceIndex, S32> map;
			Btl::Vector<ResourceIndex>* pVector;

			S32 Insert(const ResourceIndex& r)
			{
				Btl::Map<ResourceIndex, S32>::InsertionReturnType i = map.Insert(r);
				if (i.inserted)
				{
					pVector->PushBack(r);
					i.iterator->GetValue() = pVector->Size() - 1;
				}
				return i.iterator->GetValue();
			}
		};

		ResourceMap textures;
		ResourceMap indexBuffers;
		ResourceMap vertexBuffers;

		Btl::Vector<RenderingData::RenderListNode::RoomObjectMapper> vMap;
	};

	Btl::Vector<RenderListNode> m_renderLists;

private:
	void AddInstanceToRenderList(const InstanceSource& instanceSource, S32 baseIndex, S32 primitiveIndex, S32 pass);
	void AddPrimitiveToRenderList(const PrimitiveSource& primitiveSource, S32 pass);
	S32 AddPrimitiveToRenderList(const Model* pModel, S32 primitiveIndex, 
														Btl::Vector<RenderingData::RenderListNode::Primitive>& primitives, S32 pass);
	void CalculateRenderListBounds(RenderingData::RenderListNode* pNode);

	MaterialIndex InsertMaterial(S32 materialIndex, S32 pass);
	S32 InsertMaterial(S32 materialIndex, MaterialList& dstList, S32 pass);
	void CreateRenderList(S32 pass);

	Scene* m_pScene;
	S32 m_nPvsCells;
	S32 m_nRooms;
	bool m_separatePasses;

	Btl::Vector<PrimitiveSource> m_primitives;
	Btl::Vector<InstanceSource> m_instances;
	Btl::Vector<InstanceBatchSource> m_instanceBatches;
};

bool operator < (const RenderListBuilder::PrimitiveSource& s1, const RenderListBuilder::PrimitiveSource& s2)
{
	return s1.pModel->GetPrimitive(s1.index).sortCode < s2.pModel->GetPrimitive(s2.index).sortCode;
}

bool operator < (const RenderListBuilder::InstanceSource& s1, const RenderListBuilder::InstanceSource& s2)
{
	return s1.pBaseModel->GetPrimitive(s1.primitiveIndex).sortCode < s2.pBaseModel->GetPrimitive(s2.primitiveIndex).sortCode;
}

void Scene::BuildRenderLists(Node* pNode, bool separatePasses)
{
	if (pNode)
	{
		Btl::Map<S32, RenderListBuilder> renderListMap;

		if (pNode->GetName() == "Static")
			separatePasses = true;

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
		{
			Node* pSubNode = GetNodePtr(pNode->GetSubNode(i));

			S32 blockIndex = pNode->GetSubNode(i).GetBlock();

			RenderListBuilder& renderListBuild = renderListMap[blockIndex];
			renderListBuild.SetScene(this, separatePasses);

			if (pSubNode)
			{
				bool added = AddNodeToRenderList(pSubNode, renderListBuild);

				if (added)
				{
					// Move sub-nodes.
					for (int j = 0; j < pSubNode->GetNumSubNodes(); j++)
					{
						ResourceIndex r = pSubNode->GetSubNode(j);
						pNode->AddSubNode(r);
					}

					// Remove the node.
					pNode->ClearSubNode(i);
				}
			}
		}

		pNode->CleanSubNodes();

		S32 count = 0;

		for (Btl::Map<S32, RenderListBuilder>::Iterator it = renderListMap.Begin(); it != renderListMap.End(); it++)
		{
			RenderListBuilder& renderListBuild = it->GetValue();

			if (!renderListBuild.Empty())
			{
				count++;
				renderListBuild.Finish();

				for (int i = 0; i < RENDER_PASS_MAX; i++)
				{
					if (renderListBuild.m_renderLists[i].pNode != 0)
					{
						S32 block = it->GetKey();
						Btl::AutoPointer<RenderingData::Node> pNode2(renderListBuild.m_renderLists[i].pNode);
						S32 index = m_resourceBlocks[block].AddNode(pNode2);
						pNode->AddSubNode(ResourceIndex::BlockIndex(block, index));
					}
				}
			}
		}

		int breakhere = 0;
		
		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			BuildRenderLists(GetNodePtr(pNode->GetSubNode(i)), separatePasses);
	}
}

bool Scene::AddNodeToRenderList(const Node* pNode, RenderListBuilder& renderListBuild)
{
	if (const RenderNode* pRenderNode = RenderNode::GetPointer(pNode))
	{
		if (const RenderObject* pRenderObject = pRenderNode->GetRenderObjectPtr())
		{
			switch (pRenderObject->GetObjectType())
			{
				case RENDER_OBJECT_MODEL:
				{
					renderListBuild.AddModel((const Model*) pRenderObject, pRenderNode);
					return true;
				}
				case RENDER_OBJECT_INSTANCE:
				{
					renderListBuild.AddInstance((const Instance*) pRenderObject, pRenderNode);
					return true;
				}
				case RENDER_OBJECT_INSTANCE_BATCH:
				{
					renderListBuild.AddInstanceBatch((const InstanceBatch*) pRenderObject, pRenderNode);
					return true;
				}
				default:
					return false; // unknown object type, cannot remove
			}
		}

		return true;
	}

	return false;
}

void Scene::BuildChunkData()
{
	ProcessNodes(&GetRootNode(), &Scene::BuildChunkData);
}

// Build a list of resources in each chunk to allow updates when streaming.
void Scene::BuildChunkData(Node& node)
{
	RenderingData::RenderListNode* pNode = RenderingData::RenderListNode::GetPointer(&node);

	if (pNode)
	{
		Btl::Set<S32> blocks;

		for (int i = 0; i < pNode->m_streamerData.indexBufferSource.Size(); i++)
		{
			ResourceIndex r = pNode->m_streamerData.indexBufferSource[i];
			blocks.Insert(r.GetBlock());		
		}

		for (int i = 0; i < pNode->m_streamerData.vertexBufferSource.Size(); i++)
		{
			ResourceIndex r = pNode->m_streamerData.vertexBufferSource[i];
			blocks.Insert(r.GetBlock());	
		}

		for (int i = 0; i < pNode->m_streamerData.textureSource.Size(); i++)
		{
			ResourceIndex r = pNode->m_streamerData.textureSource[i];

			if (r.GetBlock() > -1)
				blocks.Insert(r.GetBlock());	
		}

		for (int chunk = 0; chunk < m_streamData.GetNumChunks(); chunk++)
		{
			const RenderingData::ChunkInfo& chunkInfo = m_streamData.GetChunkInfo(chunk);

			Btl::Set<S32>::Iterator pb1 = blocks.Begin();
			S32 ib2 = 0;

			while (pb1 != blocks.End() && ib2 < chunkInfo.GetNumBlocks())
			{
				if (*pb1 < chunkInfo.GetBlockIndex(ib2)) pb1++;
				else if (*pb1 > chunkInfo.GetBlockIndex(ib2)) ib2++;
				else
				{
					S32 blockIndex = *pb1;

					if (pNode->m_streamerData.chunkData.Size() == 0
						|| pNode->m_streamerData.chunkData.Back().chunkIndex != chunk)
					{
						pNode->m_streamerData.chunkData.PushBack();
						pNode->m_streamerData.chunkData.Back().chunkIndex = chunk;
					}

					RenderListNode::ChunkData& chunkData = pNode->m_streamerData.chunkData.Back();

					for (int i = 0; i < pNode->m_streamerData.indexBufferSource.Size(); i++)
					{
						if (pNode->m_streamerData.indexBufferSource[i].GetBlock() == blockIndex)
						{
							chunkData.indexBuffers.PushBack();
							chunkData.indexBuffers.Back().resourceIndex = pNode->m_streamerData.indexBufferSource[i];
							chunkData.indexBuffers.Back().indexInList = i;
						}
					}

					for (int i = 0; i < pNode->m_streamerData.vertexBufferSource.Size(); i++)
					{
						if (pNode->m_streamerData.vertexBufferSource[i].GetBlock() == blockIndex)
						{
							chunkData.vertexBuffers.PushBack();
							chunkData.vertexBuffers.Back().resourceIndex = pNode->m_streamerData.vertexBufferSource[i];
							chunkData.vertexBuffers.Back().indexInList = i;
						}
					}

					for (int i = 0; i < pNode->m_streamerData.textureSource.Size(); i++)
					{
						if (pNode->m_streamerData.textureSource[i].GetBlock() == blockIndex)
						{
							chunkData.textures.PushBack();
							chunkData.textures.Back().resourceIndex = pNode->m_streamerData.textureSource[i];
							chunkData.textures.Back().indexInList = i;
						}
					}

					pb1++; ib2++;
				}
			}
		}
	}
}

void RenderListBuilder::AddModel(const Model* p, const CullNode* pNode)
{
	for (int i = 0; i < p->GetNumPrimitives(); i++)
	{
		m_primitives.PushBack();
		m_primitives.Back().index = i;
		m_primitives.Back().pModel = p;
		m_primitives.Back().cullNode = pNode;
	}
}

void RenderListBuilder::AddInstance(const Instance* p, const CullNode* pNode)
{
	S32 baseModelIndex = p->GetBaseModelIndex();
	Model& baseModel = m_pScene->GetBaseModel(baseModelIndex);

	for (int i = 0; i < baseModel.GetNumPrimitives(); i++)
	{
		m_instances.PushBack();
		m_instances.Back().cullNode = pNode;
		m_instances.Back().pInstance = p;
		m_instances.Back().pBaseModel = &baseModel;
		m_instances.Back().primitiveIndex = i;
	}
}

void RenderListBuilder::AddInstanceBatch(const InstanceBatch* p, const CullNode* pNode)
{
}

RenderPass GetRenderPass(SortType sortType, BlendState blendState)
{
	if (sortType == SORT_TYPE_RACIFICATION)
		return RENDER_PASS_RACIFICATION;
	else if (sortType == SORT_TYPE_BACK_FRONT)
		return RENDER_PASS_BACK_FRONT;
	else if (sortType == SORT_TYPE_POST_RESOLVE)
		return RENDER_PASS_POST_RESOLVE;
	else if (blendState.GetAlphaBlendEnable())
		return RENDER_PASS_OVERLAY;
	else if (blendState.GetAlphaTestEnable() || blendState.GetAlphaToMaskEnable())
		return RENDER_PASS_ALPHA_TEST;
	else
		return RENDER_PASS_OPAQUE;
}

void RenderListBuilder::CreateRenderList(S32 pass)
{
	Allocate(m_renderLists[pass].pNode);
	m_renderLists[pass].indexBuffers.pVector = &m_renderLists[pass].pNode->m_streamerData.indexBufferSource;
	m_renderLists[pass].vertexBuffers.pVector = &m_renderLists[pass].pNode->m_streamerData.vertexBufferSource;
	m_renderLists[pass].textures.pVector = &m_renderLists[pass].pNode->m_streamerData.textureSource;
	m_renderLists[pass].pNode->m_sceneGraphData.renderPassInfo.Resize(RENDER_PASS_MAX);
}

void RenderListBuilder::Finish()
{
	if (m_primitives.Size() == 0 && m_instances.Size() == 0)
		return;

	m_renderLists.Resize(RENDER_PASS_MAX);

	std::stable_sort(m_primitives.Begin(), m_primitives.End());
	std::stable_sort(m_instances.Begin(), m_instances.End());

	if (m_primitives.Size() > 1)
	{
		PrimitiveSorter primitiveSorter;

		for (int i = 0; i < m_primitives.Size(); i++)
		{
			const Model* pModel = m_primitives[i].pModel;
			S32 index = m_primitives[i].index;
			const Primitive& primitive(pModel->GetPrimitive(index));

			PrimitiveSorter::State state;

			state.effect = primitive.effect;
			state.indexBuffer = Btl::Hash(primitive.indexBufferIndex.GetIndex(), primitive.indexBufferIndex.GetBlock());
			state.vertexDeclaration = primitive.vertexDeclaration;
			state.blendState = Hash(primitive.blendState, 0);
			state.baseMaterial = pModel->GetBaseMaterialIndex();
			state.instanceMaterial = pModel->GetInstanceMaterialIndex();
			state.sortCode = primitive.sortCode;

			for (int stream = 0; stream < pModel->GetNumStreams(); stream++)
			{
				const PrimitiveStream& primitiveStream = pModel->GetModelStream(stream).GetPrimitive(index);
				S32 streamIndex = primitiveStream.streamIndex;
				state.streams[streamIndex].vertexBuffer = Btl::Hash(primitiveStream.vertexBufferIndex.GetIndex(), primitiveStream.vertexBufferIndex.GetBlock());
				state.streams[streamIndex].material = primitiveStream.materialIndex;
			}

			primitiveSorter.AddPrimitive(state);
		}

		primitiveSorter.Process();

		Btl::Vector<PrimitiveSource> sortedPrimitives;

		for (int i = 0; i < primitiveSorter.GetSortedIndexArray().Size(); i++)
			sortedPrimitives.PushBack(m_primitives[primitiveSorter.GetSortedIndexArray()[i]]);

		Btl::Swap(sortedPrimitives, m_primitives);
	}

	if (m_instances.Size() > 1)
	{
		PrimitiveSorter primitiveSorter;

		for (int i = 0; i < m_instances.Size(); i++)
		{
			const Model* pModel = m_instances[i].pBaseModel;
			const Instance* pInstance = m_instances[i].pInstance;
			S32 index = m_instances[i].primitiveIndex;
			const Primitive& primitive(pModel->GetPrimitive(index));

			PrimitiveSorter::State state;

			state.effect = primitive.effect;
			state.indexBuffer = Btl::Hash(primitive.indexBufferIndex.GetIndex(), primitive.indexBufferIndex.GetBlock());
			state.vertexDeclaration = primitive.vertexDeclaration;
			state.blendState = Hash(primitive.blendState, 0);
			state.baseMaterial = pModel->GetBaseMaterialIndex();
			state.instanceMaterial = pInstance->GetMaterialIndex();
			state.sortCode = primitive.sortCode;

			for (int stream = 0; stream < pModel->GetNumStreams(); stream++)
			{
				const PrimitiveStream& primitiveStream = pModel->GetModelStream(stream).GetPrimitive(index);
				S32 streamIndex = primitiveStream.streamIndex;
				state.streams[streamIndex].vertexBuffer = Btl::Hash(primitiveStream.vertexBufferIndex.GetIndex(), primitiveStream.vertexBufferIndex.GetBlock());
				state.streams[streamIndex].material = primitiveStream.materialIndex;
			}

			for (int stream = 0; stream < pInstance->GetNumStreams(); stream++)
			{
				const PrimitiveStream& primitiveStream = pInstance->GetModelStream(stream).GetPrimitive(index);
				S32 streamIndex = primitiveStream.streamIndex;
				state.streams[streamIndex].vertexBuffer = Btl::Hash(primitiveStream.vertexBufferIndex.GetIndex(), primitiveStream.vertexBufferIndex.GetBlock());
				state.streams[streamIndex].material = primitiveStream.materialIndex;
			}

			primitiveSorter.AddPrimitive(state);
		}

		primitiveSorter.Process();

		Btl::Vector<InstanceSource> sortedPrimitives;

		for (int i = 0; i < primitiveSorter.GetSortedIndexArray().Size(); i++)
			sortedPrimitives.PushBack(m_instances[primitiveSorter.GetSortedIndexArray()[i]]);

		Btl::Swap(sortedPrimitives, m_instances);
	}

	std::stable_sort(m_primitives.Begin(), m_primitives.End());
	std::stable_sort(m_instances.Begin(), m_instances.End());

	for (int pass = 0; pass < RENDER_PASS_MAX; pass++)
	{
		S32 renderListIndex = m_separatePasses ? pass : 0;

		if (m_renderLists[renderListIndex].pNode != 0)
		{
			RenderingData::RenderListNode::RenderPassInfo& renderPassInfo(m_renderLists[renderListIndex].pNode->m_sceneGraphData.renderPassInfo[pass]);
			renderPassInfo.primitiveOffset = m_renderLists[renderListIndex].pNode->m_renderData.primitives.Size();
			renderPassInfo.instanceOffset = m_renderLists[renderListIndex].pNode->m_renderData.instances.Size();
			renderPassInfo.instanceBatchOffset = m_renderLists[renderListIndex].pNode->m_renderData.instanceBatches.Size();
		}

		for (int i = 0; i < m_primitives.Size(); i++)
		{
			SortType sortType = m_primitives[i].pModel->GetPrimitive(m_primitives[i].index).sortType;
			BlendState blendState = m_primitives[i].pModel->GetPrimitive(m_primitives[i].index).blendState;

			RenderPass renderPass = GetRenderPass(sortType, blendState);

			if (renderPass == pass)
			{
				if (m_renderLists[renderListIndex].pNode == 0)
					CreateRenderList(renderListIndex);

				AddPrimitiveToRenderList(m_primitives[i], renderListIndex);
			}
		}

		typedef Btl::Map<Btl::Pair<S32, S32>, S32> IndexMap;
		IndexMap indexMap;

		for (int i = 0; i < m_instances.Size(); i++)
		{
			S32 baseModelIndex = m_instances[i].pInstance->GetBaseModelIndex();
			S32 primitiveIndex = m_instances[i].primitiveIndex;

			Model& baseModel = m_pScene->GetBaseModel(baseModelIndex);

			const Primitive& oldPrimitive = baseModel.GetPrimitive(m_instances[i].primitiveIndex);
			SortType sortType = oldPrimitive.sortType;
			BlendState blendState = oldPrimitive.blendState;
			RenderPass renderPass = GetRenderPass(sortType, blendState);

			if (renderPass == pass)
			{	
				if (m_renderLists[renderListIndex].pNode == 0)
					CreateRenderList(renderListIndex);

				IndexMap::InsertionReturnType r = indexMap.Insert(Btl::MakePair(baseModelIndex, primitiveIndex));

				if (r.inserted) // Add the base primitive.
					r.iterator->GetValue() = AddPrimitiveToRenderList(&baseModel, primitiveIndex, m_renderLists[renderListIndex].pNode->m_renderData.basePrimitives, renderListIndex);

				AddInstanceToRenderList(m_instances[i], r.iterator->GetValue(), primitiveIndex, renderListIndex);
			}
		}

		if (m_renderLists[renderListIndex].pNode != 0)
		{
			RenderingData::RenderListNode::RenderPassInfo& renderPassInfo(m_renderLists[renderListIndex].pNode->m_sceneGraphData.renderPassInfo[pass]);
			renderPassInfo.nPrimitives = m_renderLists[renderListIndex].pNode->m_renderData.primitives.Size() - renderPassInfo.primitiveOffset;
			renderPassInfo.nInstances = m_renderLists[renderListIndex].pNode->m_renderData.instances.Size() - renderPassInfo.instanceOffset;
			renderPassInfo.nInstanceBatches = m_renderLists[renderListIndex].pNode->m_renderData.instanceBatches.Size() - renderPassInfo.instanceBatchOffset;
		}
	}
	
	for (int i = 0; i < m_renderLists.Size(); i++)
	{
		if (m_renderLists[i].pNode != 0)
		{
			m_renderLists[i].pNode->m_constantMaterialTypes.CopyTypes(m_renderLists[i].constantMaterials);
			m_renderLists[i].pNode->m_constantMaterialData.CopyData(m_renderLists[i].constantMaterials);

			for (Btl::Map<S32, MaterialList>::Iterator pML = m_renderLists[i].localDynamicMaterialLists.Begin(); pML != m_renderLists[i].localDynamicMaterialLists.End(); ++pML)
			{
				S32 index = pML->GetKey();

				if (index > -1)
				{
					S32 size = m_renderLists[i].pNode->m_sceneGraphData.dynamicMaterialLists.Size();
					m_renderLists[i].pNode->m_sceneGraphData.dynamicMaterialLists.Resize(Maths::Max(size, index + 1));
					m_renderLists[i].pNode->m_sceneGraphData.dynamicMaterialLists[index] = pML->GetValue();
				}
			}

			CalculateRenderListBounds(&*m_renderLists[i].pNode);

			for (int room = 0; room < m_renderLists[i].vMap.Size(); room++)
			{
				RenderingData::RenderListNode::RoomObjectMapper& rom(m_renderLists[i].vMap[room]);

				if (rom.m_vPrimitives.AnyBitsSet() || rom.m_vInstances.AnyBitsSet())
				{
					m_renderLists[i].pNode->m_sceneGraphData.vMap.PushBack();
					m_renderLists[i].pNode->m_sceneGraphData.vMap.Back() = rom;
					m_renderLists[i].pNode->m_sceneGraphData.vRoomIndices.PushBack(room);
				}
			}
		}
	}
}

void RenderListBuilder::AddInstanceToRenderList(const InstanceSource& instanceSource, S32 baseIndex, S32 primitiveIndex, S32 pass)
{
	m_renderLists[pass].pNode->m_sceneGraphData.instanceBounds.PushBack(instanceSource.cullNode->GetBounds());
	m_renderLists[pass].pNode->m_renderData.instances.PushBack();
	RenderingData::RenderListNode::Instance& instance = m_renderLists[pass].pNode->m_renderData.instances.Back();

	// pvs
	{
		S32 nCells = instanceSource.cullNode->GetPvsBits().Size();
		m_nPvsCells = Maths::Max(nCells, m_nPvsCells);
		m_renderLists[pass].pNode->m_sceneGraphData.instancePvs.bits.Resize(nCells);

		m_renderLists[pass].pNode->m_streamerData.cellPvsInstances.PushBack(instanceSource.cullNode->GetPvsBits());

		for (int i = 0; i < nCells; i++)
		{
			bool visible = instanceSource.cullNode->GetPvsBits()[i];
			m_renderLists[pass].pNode->m_sceneGraphData.instancePvs.bits[i].PushBack(visible);
		}
	}
	
	// portals
	{
		S32 iNumberOfRooms = instanceSource.cullNode->GetPortalBits().Size();
		m_nRooms = Maths::Max(iNumberOfRooms, m_nRooms);

		m_renderLists[pass].pNode->m_streamerData.instanceRooms.PushBack(instanceSource.cullNode->GetPortalBits());

		m_renderLists[pass].vMap.Resize(iNumberOfRooms);

		for(S32 i=0; i<iNumberOfRooms; ++i)
		{
			bool b = instanceSource.cullNode->GetPortalBits().GetBitValue(i);

			m_renderLists[pass].vMap[i].m_vInstances.PushBack(b);
		}
	}

	instance.material = InsertMaterial(instanceSource.pInstance->GetMaterialIndex(), pass);
	instance.primitiveIndex = baseIndex;
	instance.flags = instanceSource.pInstance->GetFlags();

	for (int stream = 0; stream < instanceSource.pInstance->GetNumStreams(); stream++)
	{
		const PrimitiveStream& primitiveStream = instanceSource.pInstance->GetModelStream(stream).GetPrimitive(primitiveIndex);
		instance.primitiveStreams.PushBack();
		RenderingData::RenderListNode::PrimitiveStream& primitiveStreamRL = instance.primitiveStreams.Back();
		primitiveStreamRL.streamIndex = primitiveStream.streamIndex;
		primitiveStreamRL.vertexOffset = primitiveStream.vertexOffset;
		primitiveStreamRL.frequency = primitiveStream.frequency;
		primitiveStreamRL.vertexBuffer = m_renderLists[pass].vertexBuffers.Insert(primitiveStream.vertexBufferIndex);
		primitiveStreamRL.material = InsertMaterial(primitiveStream.materialIndex, pass);
	}
}

void RenderListBuilder::AddPrimitiveToRenderList(const PrimitiveSource& primitiveSource, S32 pass)
{
	Bounds bounds = primitiveSource.cullNode->GetBounds();
	bounds.SetIntersection(primitiveSource.pModel->GetPrimitive(primitiveSource.index).bounds);

	m_renderLists[pass].pNode->m_sceneGraphData.primitiveBounds.PushBack(bounds);

	// pvs
	{
		S32 nCells = primitiveSource.cullNode->GetPvsBits().Size();
		m_nPvsCells = Maths::Max(nCells, m_nPvsCells);
		m_renderLists[pass].pNode->m_sceneGraphData.primitivePvs.bits.Resize(nCells);

		m_renderLists[pass].pNode->m_streamerData.cellPvsPrimitives.PushBack(primitiveSource.cullNode->GetPvsBits());

		for (int i = 0; i < nCells; i++)
		{
			bool visible = primitiveSource.cullNode->GetPvsBits()[i];
			m_renderLists[pass].pNode->m_sceneGraphData.primitivePvs.bits[i].PushBack(visible);
		}
	}

	AddPrimitiveToRenderList(primitiveSource.pModel, primitiveSource.index, m_renderLists[pass].pNode->m_renderData.primitives, pass);
	
	// portals
	{
		S32 iNumberOfRooms = primitiveSource.cullNode->GetPortalBits().Size();
		m_nRooms = Maths::Max(iNumberOfRooms, m_nRooms);

		m_renderLists[pass].pNode->m_streamerData.primitiveRooms.PushBack(primitiveSource.cullNode->GetPortalBits());

		m_renderLists[pass].vMap.Resize(iNumberOfRooms);

		for(S32 i=0; i<iNumberOfRooms; ++i)
		{
			bool b = primitiveSource.cullNode->GetPortalBits()[i];

			m_renderLists[pass].vMap[i].m_vPrimitives.PushBack(b);
		}
	}
}

S32 RenderListBuilder::AddPrimitiveToRenderList(const Model* pModel, S32 primitiveIndex, 
												 Btl::Vector<RenderingData::RenderListNode::Primitive>& primitives, S32 pass)
{
	primitives.PushBack();
	RenderingData::RenderListNode::Primitive& primitiveRL = primitives.Back();
	const Primitive& primitive = pModel->GetPrimitive(primitiveIndex);
	primitiveRL.effect = primitive.effect;
	primitiveRL.vertexDeclaration = primitive.vertexDeclaration;
	primitiveRL.primitiveType = primitive.primitiveType;
	primitiveRL.flags = primitive.flags;
	primitiveRL.baseMaterial = InsertMaterial(pModel->GetBaseMaterialIndex(), pass);
	primitiveRL.instanceMaterial = InsertMaterial(pModel->GetInstanceMaterialIndex(), pass);
	primitiveRL.blendState = primitive.blendState;

	for (int lod = 0; lod < primitive.lods.Size(); lod++)
	{
		primitiveRL.lods.PushBack();
		primitiveRL.lods[lod].nIndices = primitive.lods[lod].nIndices;
		primitiveRL.lods[lod].offset = primitive.lods[lod].offset;

		U32 minIndex, nVertices;
		const_cast<const Scene*>(m_pScene)->GetIndexBuffer(primitive.indexBufferIndex).GetMinIndexAndVertexCount(primitive.lods[lod].offset, primitive.lods[lod].nIndices, minIndex, nVertices);

		primitiveRL.lods[lod].minIndex = minIndex;
		primitiveRL.lods[lod].nVertices = nVertices;
	}

	primitiveRL.indexBufferIndex = m_renderLists[pass].indexBuffers.Insert(primitive.indexBufferIndex);
	
	for (int stream = 0; stream < pModel->GetNumStreams(); stream++)
	{
		const PrimitiveStream& primitiveStream = pModel->GetModelStream(stream).GetPrimitive(primitiveIndex);
		primitiveRL.primitiveStreams.PushBack();
		RenderingData::RenderListNode::PrimitiveStream& primitiveStreamRL = primitiveRL.primitiveStreams.Back();
		primitiveStreamRL.streamIndex = primitiveStream.streamIndex;
		primitiveStreamRL.vertexOffset = primitiveStream.vertexOffset;
		primitiveStreamRL.frequency = primitiveStream.frequency;
		primitiveStreamRL.vertexBuffer = m_renderLists[pass].vertexBuffers.Insert(primitiveStream.vertexBufferIndex);
		primitiveStreamRL.material = InsertMaterial(primitiveStream.materialIndex, pass);
	}

	return primitives.Size() - 1;
}

void RenderListBuilder::CalculateRenderListBounds(RenderingData::RenderListNode* pNode)
{
	// bounds
	{
		Bounds renderListBounds;
		renderListBounds.SetEmpty();

		for(S32 i = 0; i < pNode->m_sceneGraphData.primitiveBounds.Size(); ++i)
		{
			Bounds& bounds = pNode->m_sceneGraphData.primitiveBounds[i];
			renderListBounds.SetUnion(bounds);
		}

		for(S32 i = 0; i < pNode->m_sceneGraphData.instanceBounds.Size(); ++i)
		{
			Bounds& bounds = pNode->m_sceneGraphData.instanceBounds[i];
			renderListBounds.SetUnion(bounds);
		}
	
		pNode->SetBounds(renderListBounds);
	}

	// pvs
	{
		pNode->GetPvsBits().Resize(m_nPvsCells);
		pNode->GetPvsBits().ClearBits();

		for(S32 i = 0; i < pNode->m_streamerData.cellPvsPrimitives.Size(); ++i)
		{
			pNode->GetPvsBits() |= pNode->m_streamerData.cellPvsPrimitives[i];
		}

		for(S32 i = 0; i < pNode->m_streamerData.cellPvsInstances.Size(); ++i)
		{
			pNode->GetPvsBits() |= pNode->m_streamerData.cellPvsInstances[i];
		}
	}

	// portals
	{
		pNode->GetPortalBits().Resize(m_nRooms);
		pNode->GetPortalBits().ClearBits();

		for(S32 i = 0; i < pNode->m_streamerData.primitiveRooms.Size(); ++i)
		{
			pNode->GetPortalBits() |= pNode->m_streamerData.primitiveRooms[i];
		}

		for(S32 i = 0; i < pNode->m_streamerData.instanceRooms.Size(); ++i)
		{
			pNode->GetPortalBits() |= pNode->m_streamerData.instanceRooms[i];
		}
	}
}

S32 RenderListBuilder::InsertMaterial(S32 materialIndex, MaterialList& dstList, S32 pass)
{
	Material newMaterial;

	if (materialIndex > -1)
	{
		const Material& material = m_pScene->GetMaterialList().GetMaterial(materialIndex);
		const MaterialType& materialType = m_pScene->GetMaterialList().GetMaterialType(material.GetTypeIndex());

		S32 typeIndex = dstList.InsertMaterialType(materialType);
		dstList.CreateMaterial(newMaterial, typeIndex);
		newMaterial.SetData(material.GetData(), material.GetDataSize());

		// Convert sub-materials and remap the indices.
		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX
				|| materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_DATA_INDEX)
			{
				for (int i = 0; i < materialType.GetElement(e).count; i++)
				{
					S32 offset = materialType.GetOffset(e, i);
					S32 subIndex = material.GetData<S32>(offset);
					MaterialIndex newSubIndex = InsertMaterial(subIndex, pass);
					newMaterial.SetData(offset, newSubIndex);
				}
			}
			else if (materialType.GetElement(e).type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
			{
				for (int i = 0; i < materialType.GetElement(e).count; i++)
				{
					S32 offset = materialType.GetOffset(e, i);
					S32 sceneTextureIndex = material.GetData<S32>(offset);
					S32 renderListTextureIndex = m_renderLists[pass].textures.Insert(m_pScene->GetTexture(sceneTextureIndex));
					newMaterial.SetData(offset, renderListTextureIndex);
				}
			}
		}
	}
	else
	{
		// Create empty material.
		MaterialType materialType;
		S32 typeIndex = dstList.InsertMaterialType(materialType);
		dstList.CreateMaterial(newMaterial, typeIndex);
	}

	return dstList.InsertMaterial(newMaterial);
}

MaterialIndex RenderListBuilder::InsertMaterial(S32 materialIndex, S32 pass)
{
	MaterialIndex materialRL;

	if (materialIndex > -1)
	{
		const Material& material = m_pScene->GetMaterialList().GetMaterial(materialIndex);
		const MaterialType& materialType = m_pScene->GetMaterialList().GetMaterialType(material.GetTypeIndex());
		S32 listIndex = materialType.GetMaterialListIndex();
		materialRL.listIndex = listIndex;

		MaterialList* pMaterialList;

		if (listIndex == 0)
			pMaterialList = &m_renderLists[pass].constantMaterials;
		else if (m_pScene->GetDynamicMaterialList(listIndex - 1))
			pMaterialList = m_pScene->GetDynamicMaterialList(listIndex - 1);
		else
			pMaterialList = &m_renderLists[pass].localDynamicMaterialLists[listIndex - 1];

		materialRL.index = InsertMaterial(materialIndex, *pMaterialList, pass);
	}
	else
	{
		materialRL.listIndex = 0;
		materialRL.index = InsertMaterial(materialIndex, m_renderLists[pass].constantMaterials, pass);
	}

	return materialRL;
}

const Material& RenderListNode::GetMaterial(const MaterialIndex& material) const
{
	if (material.listIndex == 0)
		return m_constantMaterialData.GetMaterial(material.index);
	else
		return m_sceneGraphData.dynamicMaterialLists[material.listIndex - 1].GetMaterial(material.index);
}

const MaterialType& RenderListNode::GetMaterialType(const MaterialIndex& material) const
{
	if (material.listIndex == 0)
	{
		S32 type = m_constantMaterialData.GetMaterial(material.index).GetTypeIndex();
		return m_constantMaterialTypes.GetMaterialType(type);
	}
	else
		return m_sceneGraphData.dynamicMaterialLists[material.listIndex - 1].GetTypeOfMaterial(material.index);
}

}}
