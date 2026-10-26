#include "stdafx.h"

#include <Core/Maths/Hvec/New/IncludeGeneral.h>
#include <Core/Maths/Hvec/New/IncludeGeneral.inl>

#include "renderingdata/build.h"

#include "renderingdata/LodMesh.h"
#include "renderingdata/noninterface/LodMesh.inl"

#include <system/DebugLog.h>
#include <texture_export/texture.h>
#include "texture_export\TextureProcessing.h"
#include "renderingdata/lods.h"

//#pragma optimize("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

namespace RenderingData {
namespace Build
{

void Scene::Clear()
{
	m_resourceBlocks.Clear();
	m_vertexDeclarations.Clear();
	m_effects.Clear();
	m_sharedTextures.Clear();
	m_baseModels.Clear();
	m_materialList.Clear();
	m_textureMap.Clear();
	m_textures.Clear();
}

Scene::Scene(const Scene& scene)
{
	Copy(scene);
}

Scene& Scene::operator = (const Scene& scene)
{
	Clear();
	Copy(scene);
	return *this;
}

void Scene::Copy(const Scene& scene)
{
	static_cast<RenderingData::Scene&>(*this) = scene;

	m_vertexDeclarations = scene.m_vertexDeclarations;
	m_effects = scene.m_effects;
	m_sharedTextures = scene.m_sharedTextures;
	m_baseModels = scene.m_baseModels;
	m_materialList = scene.m_materialList;
	m_textureMap = scene.m_textureMap;
	m_textures = scene.m_textures;
	m_exportData = scene.m_exportData;
	
	m_resourceBlocks.Clear();
	m_resourceBlocks.Resize(scene.m_resourceBlocks.Size());

	for (int i = 0; i < m_resourceBlocks.Size(); i++)
	{
		m_resourceBlocks[i].SetScenePtr(this);
		m_resourceBlocks[i].m_blockIndex = i;
		m_resourceBlocks[i].Copy(scene.m_resourceBlocks[i]);
	}

	m_streamData.m_pScene = this;
}

void Scene::SetName(const Btl::String& s)
{
	m_name = s;
}

const Node* Scene::GetNodePtr(const ResourceIndex& i) const
{
	return const_cast<Scene*>(this)->GetNodePtr(i);
}

Node* Scene::GetNodePtr(const ResourceIndex& i)
{
	if (i.GetBlock() > -1)
		return m_resourceBlocks[i.GetBlock()].GetNodePtr(i.GetIndex());
	else if (i.GetBlock() == -1 && i.GetIndex() == 0)
		return &m_rootNode;
	else
		return 0;
}

S32 Scene::GetNumBaseModels() const
{
	return m_baseModels.Size();
}

const Model& Scene::GetBaseModel(S32 i) const
{
	return m_baseModels[i];
}

Model& Scene::GetBaseModel(S32 i)
{
	return m_baseModels[i];
}

S32 Scene::AddBaseModel()
{
	m_baseModels.PushBack();
	return m_baseModels.Size() - 1;
}

S32 Scene::InsertVertexDeclaration(const VertexDeclaration& vertexDeclaration)
{
	for (int i = 0; i < m_vertexDeclarations.Size(); i++)
		if (m_vertexDeclarations[i] == vertexDeclaration)
			return i;

	m_vertexDeclarations.PushBack(vertexDeclaration);
	return m_vertexDeclarations.Size() - 1;
}

// blocks
S32 Scene::AddResourceBlock() 
{
	m_resourceBlocks.PushBack();
	m_resourceBlocks.Back().SetScenePtr(this);
	m_resourceBlocks.Back().m_blockIndex = m_resourceBlocks.Size() - 1;
	return m_resourceBlocks.Size() - 1;
}

ResourceBlock& Scene::InsertResourceBlock(S32 i)
{
	while (m_resourceBlocks.Size() <= i)
		AddResourceBlock();

	return m_resourceBlocks[i];
}

// textures
S32 Scene::InsertStreamedTexture(const Texture& texture, S32 blockIndex)
{
	assert(!m_exportData.exportComplete);
	InsertResourceBlock(blockIndex);

	S32 i = m_resourceBlocks[blockIndex].InsertTexture(texture);
	ResourceIndex index = ResourceIndex::BlockIndex(blockIndex, i);

	TextureMap::InsertionReturnType r = m_textureMap.Insert(index);
	if (r.inserted)
	{
		m_textures.PushBack(index);
		r.iterator->GetValue() = m_textures.Size() - 1;
	}

	return r.iterator->GetValue();
}

S32 Scene::InsertSharedTexture(const Texture& texture)
{
	assert(!m_exportData.exportComplete);
	S32 i = m_sharedTextures.InsertTexture(texture);

	ResourceIndex index = ResourceIndex::BlockIndex(-1, i);

	TextureMap::InsertionReturnType r = m_textureMap.Insert(index);
	if (r.inserted)
	{
		m_textures.PushBack(index);
		r.iterator->GetValue() = m_textures.Size() - 1;
	}

	return r.iterator->GetValue();
}

// effects
S32 Scene::GetNumEffects() const
{
	return m_effects.Size();
}

const Effect& Scene::GetEffect(S32 i) const
{
	return m_effects[i];
}

S32 Scene::FindEffect(const Btl::String& name) const
{
	for (int i = 0; i < m_effects.Size(); i++)
		if (m_effects[i].GetName() == name)
			return i;

	return -1;
}

S32 Scene::AddEffect(const Effect& effect)
{
	m_effects.PushBack(effect);
	return m_effects.Size() - 1;
}

ResourceIndex Scene::AddRenderNode(S32 blockIndex)
{
	Btl::AutoPointer<RenderingData::RenderNode> pNode;
	Allocate(pNode);

	S32 nodeIndex = InsertResourceBlock(blockIndex).AddNode(pNode);
	RenderingData::ResourceIndex nodeResourceIndex = RenderingData::ResourceIndex::BlockIndex(blockIndex, nodeIndex);

	return nodeResourceIndex;
}

// Some testing of the primitive before adding it to the scene.
void Scene::VerifyPrimitive(const PrimitiveInfo& primitive) const
{
	assert(primitive.base.resourceBlockIndex > -1);
	assert(primitive.base.effect > -1);
	assert(primitive.batchID > -1);
	assert(primitive.streams.Size() > 0);

	for (int i = 0; i < primitive.streams.Size(); i++)
	{
		//assert(primitive.streams[i].streamIndex > -1);
		assert(primitive.streams[i].materialIndex > -1);

		// The first stream or any with a resource block index must contain vertices.
		if (primitive.streams[i].resourceBlockIndex > -1)
		{
			switch (primitive.primitiveType)
			{
				case PrimitiveInfo::PRIMITIVE_TYPE_POINT: 
					assert(primitive.streams[i].vertices.Size() == 1);
					break;
				case PrimitiveInfo::PRIMITIVE_TYPE_TRIANGLE: 
					assert(primitive.streams[i].vertices.Size() == 3);
					break;
				case PrimitiveInfo::PRIMITIVE_TYPE_QUAD: 
					assert(primitive.streams[i].vertices.Size() == 4);
					break;
				default:
					break;
			}
		}
		else
		{
			assert(primitive.streams[i].streamIndex > 0);
		}
	}
}

void Scene::GetTextureIndices(const PrimitiveInfo& primitive, Btl::Vector<S32>& textures) const
{
	textures.Clear();

	for (int s = 0; s < primitive.streams.Size(); s++)
	{
		S32 materialIndex = primitive.streams[s].materialIndex;

		if (materialIndex > -1)
		{
			S32 typeIndex = m_materialList.GetMaterial(materialIndex).GetTypeIndex();
			const MaterialType& type = m_materialList.GetMaterialType(typeIndex);

			for (int e = 0; e < type.GetNumElements(); e++)
			{
				const MaterialElement& materialElement = type.GetElement(e);

				if (materialElement.type == MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX)
				{
					for (int i = 0; i < materialElement.count; i++)
					{
						S32 offset = type.GetOffset(e, i);
						S32 textureIndex = m_materialList.GetMaterial(materialIndex).GetData<S32>(offset);

						textures.PushBack(textureIndex);
					}
				}
			}
		}
	}
}

void Scene::AddPrimitive(PrimitiveInfo& primitive, Model* pModel)
{
	VerifyPrimitive(primitive);
	AddPrimitive(primitive, pModel, 0);
}

void Scene::AddPrimitive(PrimitiveInfo& primitive, Instance* pInstance)
{
	AddPrimitive(primitive, &GetBaseModel(pInstance->GetBaseModelIndex()), pInstance);
}

void Scene::AddPrimitive(PrimitiveInfo& primitive, Model* pModel, Instance* pInstance)
{
	assert(!m_exportData.exportComplete);

	S32 modelPrimitiveIndex = -1;
	ResourceIndex indexBufferResourceIndex;
	ResourceIndex vertexBufferResourceIndex;

	PrimitiveType primitiveType = PRIMITIVE_TYPE_TRIANGLE_LIST;
	
	switch (primitive.primitiveType)
	{
		case PrimitiveInfo::PRIMITIVE_TYPE_POINT: 
			primitiveType = PRIMITIVE_TYPE_POINT_LIST;
			break;
		case PrimitiveInfo::PRIMITIVE_TYPE_TRIANGLE: 
			primitiveType = PRIMITIVE_TYPE_TRIANGLE_LIST;
			break;
		case PrimitiveInfo::PRIMITIVE_TYPE_QUAD: 
			primitiveType = PRIMITIVE_TYPE_QUAD_LIST;
			break;
		default: 
			assert(0);
	}

	for (int stream = 0; stream < primitive.streams.Size(); stream++)
	{
		// Get model stream.
		ModelStream* pModelStream = 0;

		if (pInstance)
		{
			if (pInstance->m_streams.Size() == stream) pInstance->m_streams.PushBack();
			pModelStream = &pInstance->m_streams[stream];
		}
		else
		{
			if (pModel->m_streams.Size() == stream) pModel->m_streams.PushBack();
			pModelStream = &pModel->m_streams[stream];
		}

		if (primitive.streams[stream].resourceBlockIndex > -1)
		{
			ResourceBlock& streamResourceBlock = InsertResourceBlock(primitive.streams[stream].resourceBlockIndex);

			// Find the vertex buffer in the current resource block
			S32 vertexBufferIndex = streamResourceBlock.InsertVertexBuffer(	primitive.streams[stream].vertices.GetVertexFormat(), 
																			primitive.batchID, primitive.streams[stream].instanceID,
																			primitive.streams[stream].streamIndex);

			vertexBufferResourceIndex = ResourceIndex::BlockIndex(primitive.streams[stream].resourceBlockIndex, vertexBufferIndex);

			// Add vertices to vertex buffers.
			VertexData& vertexData = streamResourceBlock.GetVertexBuffer(vertexBufferIndex).GetVertices();
			int i0 = vertexData.PushBack(primitive.streams[stream].vertices[0]);
			int i1 = vertexData.PushBack(primitive.streams[stream].vertices[1]);
			int i2 = vertexData.PushBack(primitive.streams[stream].vertices[2]);

			// If this is the base layer, add to the index buffer.
			if (stream == 0)
			{
				if (!pInstance)
				{
					modelPrimitiveIndex = pModel->InsertPrimitive(primitive.batchID, primitiveType, primitive.base);
					Primitive& modelPrimitive = pModel->m_primitives[modelPrimitiveIndex];
					
					if (modelPrimitive.indexBufferIndex.IsNull())
					{
						// Create an index buffer.
						ResourceBlock& modelResourceBlock = InsertResourceBlock(primitive.base.resourceBlockIndex);
						S32 i = modelResourceBlock.AddIndexBuffer(primitiveType);
						modelPrimitive.indexBufferIndex = ResourceIndex::BlockIndex(primitive.base.resourceBlockIndex, i);
					}

					indexBufferResourceIndex = modelPrimitive.indexBufferIndex;
					IndexBuffer::IndexData& indexData = GetIndexBuffer(modelPrimitive.indexBufferIndex).GetIndices();
					indexData.PushBack(i0);
					indexData.PushBack(i1);
					indexData.PushBack(i2);
					modelPrimitive.lods[0].nIndices += 3;

					pModel->m_primitives[modelPrimitiveIndex].bounds.SetUnion(primitive.base.bounds);
				}
				else
				{
					modelPrimitiveIndex = pModel->FindPrimitive(primitive.batchID);
					indexBufferResourceIndex = pModel->m_primitives[modelPrimitiveIndex].indexBufferIndex;
				}
			}

			// Add the vertex buffer to the index buffer's list.
			GetIndexBuffer(indexBufferResourceIndex).InsertVertexBuffer(vertexBufferResourceIndex);
		}
		else // No vertices, either dynamic vertex data or a material only stream.
		{
			vertexBufferResourceIndex = ResourceIndex::Null();

			// A vertex format indicates a dynamic vertex stream.
			if (primitive.streams[stream].vertices.GetVertexFormat().GetDataSize() > 0)
			{
				GetIndexBuffer(indexBufferResourceIndex).InsertVertexFormat(primitive.streams[stream].vertices.GetVertexFormat(), primitive.streams[stream].streamIndex);
			}
		}

		// Add to the model stream.
		pModelStream->InsertPrimitive(modelPrimitiveIndex, primitive.streams[stream].materialIndex, vertexBufferResourceIndex, primitive.streams[stream].streamIndex);
	}
}

// Called after everything is added.
void Scene::Finish(PlatformType platformType)
{
	assert(!m_exportData.exportComplete);
	LOGDEBUG(g_Log, "Scene::Finish");

	m_exportData.platformType = platformType;

	// processing
	{
		if (platformType == PLATFORM_TYPE_PC)
			ChangeVertexElementType(VERTEX_ELEMENT_TYPE_SHHD_3N, VERTEX_ELEMENT_TYPE_U8_4N);

		if (platformType == PLATFORM_TYPE_PS3)
			ReverseU8_4();

		ShareVertices();
		
		// lods
		//#if !defined( USER_JDENTON )
		if ( m_exportData.lodsToGenerate > 0 || m_exportData.reduceVertices)
		//#endif
		{
			BuildLods();
			ProcessPrimitives( &Scene::CreatePrimitiveLods );
		}
		
		BuildStrips();
		ShareVertices();
		ProcessModels(&Scene::BuildVertexDeclarations);
		CombineIndexBuffers();
		CombineVertexBuffers();
		BuildRenderLists(&GetRootNode());
		RemapTextures();
		CalculateBoundsAndPvs(); // update bounds and pvs on resource blocks and hierarchy 
		Clean();
	}

//#ifndef CFG_SPEED
	// debug
	//PrintHierarchy(GetRootNode(), 0); // [Jan] tidied up the export log file
	CalculateSizeInfo();
//#endif

	for (int i = 0; i < m_resourceBlocks.Size(); i++)
		m_resourceBlocks[i].SetPlatformType(m_exportData.platformType);

	m_exportData.exportComplete = true;
}

void Scene::CalculateBoundsAndPvs()
{
	for (int i = 0; i < m_resourceBlocks.Size(); i++)
		m_resourceBlocks[i].m_fullBounds.SetEmpty();

	PropagateGroupIndex(GetRootNode(), -1);

	ProcessNodes(&GetRootNode(), &Scene::GetTextureBounds);

	Bounds bounds;
	Btl::BitVector pvsBits, roomBits;
	CalculateBoundsAndPvs(GetRootNode(), bounds, pvsBits, -1, roomBits);
}

// Propagate group indices down the hierarchy.
void Scene::PropagateGroupIndex(Node& node, S32 groupIndex)
{
	// If it doesn't already have a group index, set to the groupIndex parameter.
	if (node.IsNode(NODE_TYPE_CULL))
	{
		CullNode* pCullNode = (CullNode*) &node;
		if (pCullNode->GetGroupIndex() == -1)
			pCullNode->SetGroupIndex(groupIndex);
	}

	for (int i = 0; i < node.GetNumSubNodes(); i++)
	{
		Node* pSubNode = GetNodePtr(node.GetSubNode(i));

		if (pSubNode)
		{
			S32 subGroupIndex = groupIndex;

			// See if the sub-node already has a group index.
			if (pSubNode->IsNode(NODE_TYPE_CULL))
			{
				CullNode* pCullNode = (CullNode*) pSubNode;
				if (pCullNode->GetGroupIndex() != -1)
					subGroupIndex = pCullNode->GetGroupIndex();
			}

			// Recursively set the group index.
			PropagateGroupIndex(*pSubNode, subGroupIndex);
		}
	}
}

// bounds and pvsBits passed up from sub-nodes.
// groupIndex passed down.
void Scene::CalculateBoundsAndPvs(Node& node, Bounds& bounds, Btl::BitVector& pvsBits, S32 groupIndex, Btl::BitVector& roomBits)
{
	if (node.IsNode(NODE_TYPE_CULL))
	{
		CullNode* pCullNode = (CullNode*) &node;
		groupIndex = pCullNode->GetGroupIndex();
	}

	bounds.SetEmpty();
	pvsBits.Clear();
	roomBits.Clear();

	for (int i = 0; i < node.GetNumSubNodes(); i++)
	{
		Node* pSubNode = GetNodePtr(node.GetSubNode(i));

		if (pSubNode)
		{
			Bounds subBounds;
			Btl::BitVector subPvsBits;
			Btl::BitVector subRoomBits;
			S32 subGroupIndex = groupIndex;

			CalculateBoundsAndPvs(*pSubNode, subBounds, subPvsBits, groupIndex, subRoomBits);
			bounds.SetUnion(subBounds);

			if (pvsBits.Empty())
				pvsBits = subPvsBits;
			else if (!subPvsBits.Empty())
				pvsBits |= subPvsBits;

			if (roomBits.Empty())
				roomBits = subRoomBits;
			else if (!subPvsBits.Empty())
				roomBits |= subRoomBits;

			if (pSubNode->IsNode(NODE_TYPE_CULL))
			{
				CullNode* pCullNode = (CullNode*) pSubNode;
				subGroupIndex = pCullNode->GetGroupIndex();
			}

			UpdateResourceBlockBounds(node.GetSubNode(i).GetBlock(), subBounds, Btl::Hash(pSubNode), subPvsBits, subGroupIndex, subRoomBits);
		}
	}

	// Some nodes have pre-calculated bounds.
	if (RenderListNode* pRenderListNode = RenderListNode::GetPointer(&node))
	{
		bounds.SetUnion(pRenderListNode->GetBounds());

		if (pvsBits.Empty())
			pvsBits = pRenderListNode->GetPvsBits();
		else
			pvsBits |= pRenderListNode->GetPvsBits();

		if (roomBits.Empty())
			roomBits = pRenderListNode->GetPortalBits();
		else
			roomBits |= pRenderListNode->GetPortalBits();

		// Blocks containing index and vertex buffers of the primitives.
		for (int i = 0; i < pRenderListNode->m_renderData.primitives.Size(); i++)
		{
			const Bounds& subBounds = pRenderListNode->m_sceneGraphData.primitiveBounds[i];
			const Btl::BitVector& subPvsBits = pRenderListNode->m_streamerData.cellPvsPrimitives[i];
			const Btl::BitVector& subRoomBits = pRenderListNode->m_streamerData.primitiveRooms[i];

			S32 indexBuffer = pRenderListNode->m_renderData.primitives[i].indexBufferIndex;
			ResourceIndex r = pRenderListNode->m_streamerData.indexBufferSource[indexBuffer];
			UpdateResourceBlockBounds(r.GetBlock(), subBounds, Btl::Hash(&node), subPvsBits, groupIndex, subRoomBits);

			for (int j = 0; j < pRenderListNode->m_renderData.primitives[i].primitiveStreams.Size(); j++)
			{
				S32 vertexBuffer = pRenderListNode->m_renderData.primitives[i].primitiveStreams[j].vertexBuffer;
				ResourceIndex r = pRenderListNode->m_streamerData.vertexBufferSource[vertexBuffer];

				if (r.GetBlock() > -1)
					UpdateResourceBlockBounds(r.GetBlock(), subBounds, Btl::Hash(&node), subPvsBits, groupIndex, subRoomBits);
			}
		}

		// Blocks containing index and vertex buffers of the instances.
		for (int i = 0; i < pRenderListNode->m_renderData.instances.Size(); i++)
		{
			const Bounds& subBounds = pRenderListNode->m_sceneGraphData.instanceBounds[i];
			const Btl::BitVector& subPvsBits = pRenderListNode->m_streamerData.cellPvsInstances[i];
			const Btl::BitVector& subRoomBits = pRenderListNode->m_streamerData.instanceRooms[i];

			S32 primitiveIndex = pRenderListNode->m_renderData.instances[i].primitiveIndex;
			S32 indexBuffer = pRenderListNode->m_renderData.basePrimitives[primitiveIndex].indexBufferIndex;
			ResourceIndex r = pRenderListNode->m_streamerData.indexBufferSource[indexBuffer];
			UpdateResourceBlockBounds(r.GetBlock(), subBounds, Btl::Hash(&node), subPvsBits, groupIndex, subRoomBits);

			for (int j = 0; j < pRenderListNode->m_renderData.instances[i].primitiveStreams.Size(); j++)
			{
				S32 vertexBuffer = pRenderListNode->m_renderData.instances[i].primitiveStreams[j].vertexBuffer;
				ResourceIndex r = pRenderListNode->m_streamerData.vertexBufferSource[vertexBuffer];

				if (r.GetBlock() > -1)
					UpdateResourceBlockBounds(r.GetBlock(), subBounds, Btl::Hash(&node), subPvsBits, groupIndex, subRoomBits);
			}
		}
	}

	if (MaterialNode* pMaterialNode = MaterialNode::GetPointer(&node))
	{
		bounds.SetUnion(pMaterialNode->GetBounds());
	}

	// Replace the original bounds.
	if (node.IsNode(NODE_TYPE_CULL))
	{
		CullNode* pCullNode = (CullNode*) &node;
		pCullNode->SetBounds(bounds);
		pCullNode->GetPvsBits() = pvsBits;
		pCullNode->GetPortalBits() = roomBits;
	}
}

void Scene::Clean()
{
	CleanNode(GetRootNode());

	// Remove render nodes from blocks.
	for (int block = 0; block < m_resourceBlocks.Size(); block++)
	{
		S32 count = 0;

		for (int node = 0; node < m_resourceBlocks[block].GetNumNodes(); node++)
		{
			if (RenderNode::GetPointer(m_resourceBlocks[block].GetNodePtr(node)))
				m_resourceBlocks[block].ClearNode(node);
			
			if (m_resourceBlocks[block].GetNodePtr(node))
				m_bufferRemap[ResourceIndex::BlockIndex(block, node)] = ResourceIndex::BlockIndex(block, count++);
		}

		m_resourceBlocks[block].RemoveEmptyNodes();
	}

	ProcessNodes(&GetRootNode(), &Scene::RemapNodes);
	m_bufferRemap.Clear();
}

void Scene::RemapResourceIndex(ResourceIndex& resourceIndex)
{
	if (resourceIndex.IsNull() || resourceIndex.IsShared())
		return;

	BufferRemap::Iterator f = m_bufferRemap.Find(resourceIndex);

	if (f != m_bufferRemap.End())
		resourceIndex = f->GetValue().newIndex;
	else
	{
		// [Jan] deprecated from assert to LOGWARN, to stop us receiving e-mails about it
		LOGWARN( g_Log, "Unexpected thing happened in scenebuild.cpp in Scene::RemapResourceIndex.\n");
	}
}

void Scene::RemapNodes(Node& node)
{
	RemapResourceIndex(node.m_parent);
	RemapResourceIndex(node.m_location);

	for (int i = 0; i < node.GetNumSubNodes(); i++)
		RemapResourceIndex(node.m_subNodes[i]);
}

void Scene::CleanNode(Node& node)
{
	for (int i = 0; i < node.GetNumSubNodes(); i++)
	{
		Node* pSubNode = GetNodePtr(node.GetSubNode(i));

		if (pSubNode)
			CleanNode(*pSubNode);
		else
			node.ClearSubNode(i);
	}

	// Remove sub-node cull nodes if they are the same size.
	if (node.IsNode(NODE_TYPE_CULL))
	{
		Bounds b1 = ((CullNode*)&node)->GetBounds();

		for (int i = 0; i < node.GetNumSubNodes(); i++)
		{
			Node* pSubNode = GetNodePtr(node.GetSubNode(i));

			if (pSubNode->GetNodeType() == NODE_TYPE_CULL
				&& pSubNode->GetName() == "")
			{
				Bounds b2 = ((CullNode*)pSubNode)->GetBounds();

				if (b1 == b2)
				{
					// Move sub-sub-nodes.
					for (int j = 0; j < pSubNode->GetNumSubNodes(); j++)
					{
						ResourceIndex r = pSubNode->ClearSubNode(j);
						node.AddSubNode(r);
					}

					// Delete the sub-node.
					ResourceIndex r = node.GetSubNode(i);
					node.ClearSubNode(i);
					GetResourceBlock(r.GetBlock()).ClearNode(r.GetIndex());
				}
			}
		}
	}

	node.CleanSubNodes();
}

void Scene::ProcessModels(ModelFunctionPtr f)
{
	for (int i = 0; i < m_baseModels.Size(); i++)
		(this->*f)(m_baseModels[i]);

	ProcessModelNodes(&GetRootNode(), f);
}

void Scene::ProcessModelNodes(Node* pNode, ModelFunctionPtr f)
{
	if (pNode)
	{
		if (const RenderNode* pRenderNode = RenderNode::GetPointer(pNode))
			if (const RenderObject* pRenderObject = pRenderNode->GetRenderObjectPtr())
				if (pRenderObject->GetObjectType() == RENDER_OBJECT_MODEL)
					(this->*f)(*(Model*) pRenderObject);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessModelNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessInstances(InstanceFunctionPtr f)
{
	ProcessInstanceNodes(&GetRootNode(), f);
}

void Scene::ProcessInstanceNodes(Node* pNode, InstanceFunctionPtr f)
{
	if (pNode)
	{
		if (const RenderNode* pRenderNode = RenderNode::GetPointer(pNode))
			if (const RenderObject* pRenderObject = pRenderNode->GetRenderObjectPtr())
				if (pRenderObject->GetObjectType() == RENDER_OBJECT_INSTANCE)
					(this->*f)(*(Instance*) pRenderObject);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessInstanceNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessNodes(const Node* pNode, ConstNodeFunctionPtr f) const
{
	if (pNode)
	{
		(this->*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}

void Scene::ProcessNodes(Node* pNode, NodeFunctionPtr f)
{
	if (pNode)
	{
		(this->*f)(*pNode);

		for (int i = 0; i < pNode->GetNumSubNodes(); i++)
			ProcessNodes(GetNodePtr(pNode->GetSubNode(i)), f);
	}
}


void Scene::ProcessPrimitiveStreams(Model& model)
{
	for (int stream = 0; stream < model.m_streams.Size(); stream++)
	{
		for (int primitive = 0; primitive < model.m_streams[stream].m_primitives.Size(); primitive++)
			(this->*m_pPrimitiveStreamFunction)(model.m_streams[stream].m_primitives[primitive]);
	}
}

void Scene::ProcessPrimitives(Model& model)
{
	for (int primitive = 0; primitive < model.m_primitives.Size(); primitive++)
	{
		(this->*m_pPrimitiveFunction)(model.m_primitives[primitive]);
	}
}

void Scene::ProcessPrimitiveStreams(Instance& instance)
{
	for (int stream = 0; stream < instance.m_streams.Size(); stream++)
	{
		for (int primitive = 0; primitive < instance.m_streams[stream].m_primitives.Size(); primitive++)
			(this->*m_pPrimitiveStreamFunction)(instance.m_streams[stream].m_primitives[primitive]);
	}
}

void Scene::ProcessPrimitiveStreams(PrimitiveStreamFunctionPtr f)
{
	m_pPrimitiveStreamFunction = f;

	ProcessModels(&Scene::ProcessPrimitiveStreams);
	ProcessInstances(&Scene::ProcessPrimitiveStreams);
	//ProcessInstanceBatches(&Scene::ProcessPrimitiveStreams);
}

void Scene::ProcessPrimitives(PrimitiveFunctionPtr f)
{
	m_pPrimitiveFunction = f;

	ProcessModels(&Scene::ProcessPrimitives);
}

//#if defined( USER_JDENTON ) && defined( CFG_ALLOW_OPTIMIZE_OFF )
//#pragma optimize( "", off )
//#endif

extern const char SELECTANY VertexElementUsageString[VERTEX_ELEMENT_USAGE_MAX][64] =
{
	"VERTEX_ELEMENT_USAGE_POSITION",
	"VERTEX_ELEMENT_USAGE_BLENDWEIGHT",
	"VERTEX_ELEMENT_USAGE_BLENDINDICES",
	"VERTEX_ELEMENT_USAGE_NORMAL",
	"VERTEX_ELEMENT_USAGE_PSIZE",
	"VERTEX_ELEMENT_USAGE_TEXCOORD",
	"VERTEX_ELEMENT_USAGE_TANGENT",
	"VERTEX_ELEMENT_USAGE_BINORMAL",
	"VERTEX_ELEMENT_USAGE_TESSFACTOR",
	"VERTEX_ELEMENT_USAGE_COLOR",
	"VERTEX_ELEMENT_USAGE_SAMPLE"
};

extern const char SELECTANY VertexElementTypeString[VERTEX_ELEMENT_TYPE_MAX][32] = 
{
	// Basic vertex types.
	"VERTEX_ELEMENT_TYPE_S16_2",
	"VERTEX_ELEMENT_TYPE_S16_4",
	"VERTEX_ELEMENT_TYPE_S16_2N",
	"VERTEX_ELEMENT_TYPE_S16_4N",
	"VERTEX_ELEMENT_TYPE_F32_1",
	"VERTEX_ELEMENT_TYPE_F32_2",
	"VERTEX_ELEMENT_TYPE_F32_3",
	"VERTEX_ELEMENT_TYPE_F32_4",
	"VERTEX_ELEMENT_TYPE_F16_2",
	"VERTEX_ELEMENT_TYPE_F16_4",
	"VERTEX_ELEMENT_TYPE_U8_4",
	"VERTEX_ELEMENT_TYPE_U8_4N",

	// Extended types (not supported on all platforms).
	"VERTEX_ELEMENT_TYPE_SHHD_3N", // 360, PS3
	"VERTEX_ELEMENT_TYPE_SDDD_3N"  // 360, PC
};

const Maths::Vec4F32 DecompressVertexElement( const VertexData& vertexData, const S32 elementIndex, const S32 vertexBufferIndex )
{
    using namespace DataTypes;
    using namespace Maths;
    
    const F32 zero( 0.0f );
    const VF32 vzero( Const<VF32>::Zero() );
    Vec4F32 result( Const<Vec4F32>::Zero() );
    
    const VertexFormat& vertexFormat = vertexData.GetVertexFormat();
    const VertexFormat::VertexElement& element = vertexFormat.GetVertexElement( elementIndex );
    switch ( element.type )
    {
	    case VERTEX_ELEMENT_TYPE_S16_2 :
	    {
	        const S16_2 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< S16_2 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), zero, zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_S16_4 :
	    {
	        const S16_4 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< S16_4 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_S16_2N :
	    {
	        const S16_2N data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< S16_2N >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), zero, zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_S16_4N :
	    {
	        const S16_4N data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< S16_4N >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F32_1 :
	    {
	        const F32_1 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F32_1 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), zero, zero, zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F32_2 :
	    {
	        const F32_2 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F32_2 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), zero, zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F32_3 :
	    {
	        const F32_3 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F32_3 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F32_4 :
	    {
	        const F32_4 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F32_4 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F16_2 :
	    {
	        const F16_2 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F16_2 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), zero, zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_F16_4 :
	    {
	        const F16_4 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< F16_4 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_U8_4 :
	    {
	        const U8_4 data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< U8_4 >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_U8_4N :
	    {
	        const U8_4N data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< U8_4N >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), (F32)data.W() );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_SHHD_3N :
	    {
	        const SHHD_3N data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< SHHD_3N >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), zero );
	    }
	    break;
	    
	    case VERTEX_ELEMENT_TYPE_SDDD_3N :
	    {
	        const SDDD_3N data( VertexData::ConstVertexReference( vertexData, vertexBufferIndex ).GetData< SDDD_3N >( elementIndex ) );
	        result = MakeVec4F32( (F32)data.X(), (F32)data.Y(), (F32)data.Z(), zero );
	    }
	    break;
	    
	    default: break;
    }
    
    #if defined( USER_JDENTON ) && defined( _DEBUG )
    if ( !Finite( result ) )
    {
        if ( !Finite( X( result ) ) )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field X is not finite! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetX( &result, vzero );
        }
        if ( !Finite( Y( result ) ) )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field Y is not finite! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetY( &result, vzero );
        }
        if ( !Finite( Z( result ) ) )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field Z is not finite! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetZ( &result, vzero );
        }
        if ( !Finite( W( result ) ) )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field W is not finite! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetW( &result, vzero );
        }
    }
    else
    {
        if ( GetF32( Abs( X( result ) ) ) > 1.0e15 )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field X is huge! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetX( &result, vzero );
        }
        if ( GetF32( Abs( Y( result ) ) ) > 1.0e15 )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field Y is huge! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetY( &result, vzero );
        }
        if ( GetF32( Abs( Z( result ) ) ) > 1.0e15 )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field Z is huge! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetZ( &result, vzero );
        }
        if ( GetF32( Abs( W( result ) ) ) > 1.0e15 )
        {
            LOGWARN( g_Log, "DecompressVertexElement(): Vertex buffer index %d, element %d, type '%s', usage '%s', field W is huge! Setting it to zero!", vertexBufferIndex, elementIndex, VertexElementTypeString[ element.type ], VertexElementUsageString[ element.usage ] );
            SetW( &result, vzero );
        }
    }
    #endif
    
    return result;
}

bool Scene::AddLodMeshTriangleVertexData( LodMesh* pLodMesh, const S32 triangleIndex, const VertexData& vertexData )
{
    using namespace DataTypes;
    using namespace Maths;
    
    bool degenerate = false;
    
    LodTriangle& triangle( pLodMesh->Triangle( triangleIndex ) );
    
    const VertexFormat& vertexFormat = vertexData.GetVertexFormat();
    
    const S32 vertexCount = vertexData.Size();
    //const S32 vertexSize = vertexFormat.GetDataSize();
    
    const S32 elementCount = vertexFormat.GetNumVertexElements();
    for ( S32 elementIndex = 0; elementIndex < elementCount; elementIndex++ )
    {
        const VertexFormat::VertexElement& element = vertexFormat.GetVertexElement( elementIndex );
        
        const Maths::Vec4F32 v0( DecompressVertexElement( vertexData, elementIndex, triangle.VBIndex(0) ) );
        const Maths::Vec4F32 v1( DecompressVertexElement( vertexData, elementIndex, triangle.VBIndex(1) ) );
        const Maths::Vec4F32 v2( DecompressVertexElement( vertexData, elementIndex, triangle.VBIndex(2) ) );
        
        BIZ_ASSERT( Finite( v0 ) );
        BIZ_ASSERT( Finite( v1 ) );
        BIZ_ASSERT( Finite( v2 ) );
        
        switch ( element.usage )
        {
		    case VERTEX_ELEMENT_USAGE_POSITION :
		    {
	            const Maths::Point3 point0( MakePoint3( v0 ) );
	            const Maths::Point3 point1( MakePoint3( v1 ) );
	            const Maths::Point3 point2( MakePoint3( v2 ) );

	            const S32 vertexIndex0 = pLodMesh->AddVertex( point0 );
	            const S32 vertexIndex1 = pLodMesh->AddVertex( point1 );
	            const S32 vertexIndex2 = pLodMesh->AddVertex( point2 );
	            
	            //BIZ_ASSERT( vertexIndex0 != vertexIndex1 );
	            //BIZ_ASSERT( vertexIndex0 != vertexIndex2 );
	            //BIZ_ASSERT( vertexIndex1 != vertexIndex2 );
	            if ( ( vertexIndex0 != vertexIndex1 ) &&
	                 ( vertexIndex0 != vertexIndex2 ) &&
	                 ( vertexIndex1 != vertexIndex2 ) )
	            {
                    const S32 edgeIndex0 = pLodMesh->AddEdge( vertexIndex0, vertexIndex1 );
                    const S32 edgeIndex1 = pLodMesh->AddEdge( vertexIndex1, vertexIndex2 );
                    const S32 edgeIndex2 = pLodMesh->AddEdge( vertexIndex2, vertexIndex0 );
    	            
                    BIZ_ASSERT( edgeIndex0 != edgeIndex1 );
                    BIZ_ASSERT( edgeIndex0 != edgeIndex2 );
                    BIZ_ASSERT( edgeIndex1 != edgeIndex2 );
    	            
                    triangle.SetVertexIndices( vertexIndex0, vertexIndex1, vertexIndex2 );
    	            
                    triangle.SetEdgeIndices( edgeIndex0, edgeIndex1, edgeIndex2 );
    	            
                    LodVertex& vertex0( pLodMesh->Vertex( vertexIndex0 ) );
                    LodVertex& vertex1( pLodMesh->Vertex( vertexIndex1 ) );
                    LodVertex& vertex2( pLodMesh->Vertex( vertexIndex2 ) );
    	            
                    LodEdge& edge0( pLodMesh->Edge( edgeIndex0 ) );
                    LodEdge& edge1( pLodMesh->Edge( edgeIndex1 ) );
                    LodEdge& edge2( pLodMesh->Edge( edgeIndex2 ) );
    	            
                    vertex0.AddTriangleIndex( triangleIndex );
                    vertex1.AddTriangleIndex( triangleIndex );
                    vertex2.AddTriangleIndex( triangleIndex );
    	            
                    edge0.AddTriangleIndex( triangleIndex );
                    edge1.AddTriangleIndex( triangleIndex );
                    edge2.AddTriangleIndex( triangleIndex );
	            }
	            else
	            {
	                degenerate = true;
	                //LOGWARN( g_Log, "Scene::AddLodMeshTriangleVertexData: Degenerate triangle in input!" );
	            }
		    }
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_BLENDWEIGHT :
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_BLENDINDICES :
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_NORMAL :
		    triangle.SetNormals( MakeVector3( v0 ), MakeVector3( v1 ), MakeVector3( v2 ) );
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_PSIZE :
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_TEXCOORD :
		    triangle.AddTexCoords( v0, v1, v2 );
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_TANGENT :
		    triangle.SetTangents( MakeVector3( v0 ), MakeVector3( v1 ), MakeVector3( v2 ) );
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_BINORMAL :
		    triangle.SetBiNormals( MakeVector3( v0 ), MakeVector3( v1 ), MakeVector3( v2 ) );
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_TESSFACTOR :
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_COLOR :
		    triangle.SetColours( MakeVector4( v0 ), MakeVector4( v1 ), MakeVector4( v2 ) );
		    break;
		    
		    case VERTEX_ELEMENT_USAGE_SAMPLE :
		    break;
		    
		    default:
		    break;
        }
    }
    
    return degenerate;
}

//#if defined( USER_JDENTON ) && defined( CFG_ALLOW_OPTIMIZE_OFF )
//#pragma optimize( "", on )
//#endif

S32 Scene::GenerateVertexBufferIndices( LodMesh* pLodMesh, Btl::Vector<U32>& indices, IndexBuffer const * pPrimitive )
{
    const S32 triangleCount( pLodMesh->TriangleCount() );
    for ( S32 triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++ )
    {
        LodTriangle& triangle( pLodMesh->Triangle( triangleIndex ) );
        if ( triangle.Primitive() == (void*)pPrimitive )
        {
            indices.PushBack( triangle.VBIndex( 0 ) );
            indices.PushBack( triangle.VBIndex( 1 ) );
            indices.PushBack( triangle.VBIndex( 2 ) );
        }
    }
    return indices.Size();
}

void Scene::CreateLodIndexBuffers( LodMesh* pLodMesh, int lod)
{
    #if !defined( _DEBUG )
    try
    #endif
    {
		for (int blockIndex = 0; blockIndex < m_resourceBlocks.Size(); blockIndex++)
		for (int indexBufferIndex = 0; indexBufferIndex < m_resourceBlocks[blockIndex].GetNumIndexBuffers(); indexBufferIndex++)
        //const S32 primitiveCount( model.GetNumPrimitives() );
        //for ( S32 primitiveIndex = 0; primitiveIndex < primitiveCount; primitiveIndex++ )
        {
            //const Primitive&    primitive( model.GetPrimitive( primitiveIndex ) );
            //const ResourceIndex indexBufferIndex( primitive.indexBufferIndex );
            Build::IndexBuffer& indexBuffer(  m_resourceBlocks[blockIndex].GetIndexBuffer( indexBufferIndex ) );
            const PrimitiveType primitiveType( indexBuffer.m_primitiveType );
            
            Build::VertexBuffer& vertexBuffer( GetVertexBuffer( indexBuffer.m_vertexBuffers[0] ) );
            const U32 vertexCount( vertexBuffer.GetVertices().Size() );
            
            IndexBuffer::IndexData& indexArray( indexBuffer.GetIndices() );
            
            #if defined( _DEBUG )
            for ( S32 i=0; i<indexBuffer.GetNumLods(); i++ )
            {
                const S32 lodOffset( indexBuffer.GetLodOffset( i ) );
                const S32 lodSize( indexBuffer.GetLodSize( i ) );
                for ( S32 j=0; j<lodSize; j++ )
                {
                    const U32 index( indexArray[ lodOffset + j ] );
                    BIZ_ASSERT( index < vertexCount );
                }
            }
            #endif
            
            if ( primitiveType == PRIMITIVE_TYPE_TRIANGLE_LIST )
            {
                Btl::Vector<U32> lodIndices;
                //const S32 lodIndexCount( lodMesh.GenerateVertexBufferIndices( lodIndices, &indexBuffer ) );
                const S32 lodIndexCount( GenerateVertexBufferIndices( pLodMesh, lodIndices, &indexBuffer ) );
                //if ( lodIndexCount > 0 )
				if (lod > 0)
                {
                    const S32 lodIndex( indexBuffer.CreateLod( lodIndexCount ) );
                    const S32 lodOffset( indexBuffer.GetLodOffset( lodIndex ) );
                    BIZ_ASSERT( lodIndexCount == indexBuffer.GetLodSize( lodIndex ) );
                    
                    for ( S32 i=0; i<lodIndexCount; i++ )
                    {
                        const U32 index( lodIndices[i] );
                        BIZ_ASSERT( index < vertexCount );
                        indexArray[ lodOffset + i ] = index;
                    }
                    
                    #if defined( _DEBUG )
                    for ( S32 i=0; i<indexBuffer.GetNumLods(); i++ )
                    {
                        const S32 lodOffset( indexBuffer.GetLodOffset( i ) );
                        const S32 lodSize( indexBuffer.GetLodSize( i ) );
                        for ( S32 j=0; j<lodSize; j++ )
                        {
                            const U32 index( indexArray[ lodOffset + j ] );
                            BIZ_ASSERT( index < vertexCount );
                        }
                    }
                    #endif
                }
				else if (lodIndexCount > 0 && lodIndexCount < indexArray.Size())
				{
					indexArray.Resize(lodIndexCount);

					for ( S32 i=0; i<lodIndexCount; i++ )
                    {
                        const U32 index( lodIndices[i] );
                        BIZ_ASSERT( index < vertexCount );
                        indexArray[i] = index;
                    }
				}
				else if (lodIndexCount == 0)
				{
					LOGDEBUG(g_Log, "Failed to generate reduced mesh, original = %i, new = %i", indexArray.Size(), lodIndexCount);
				}
            }
        }
    }
    #if !defined( _DEBUG )
    catch ( ... )
    {
        LOGWARN( g_Log, "Scene::CreateLodIndexBuffers(): Crashed!" );
    }
    #endif
}

void Scene::SetLodGenerationOptions(S32 lodsToGenerate, float reductionFactor, S32 minVertices, bool reduceVertices)
{
	m_exportData.lodsToGenerate = lodsToGenerate;
	m_exportData.minimumVertices = minVertices;
	m_exportData.vertexReductionRatio = reductionFactor;
	m_exportData.reduceVertices = reduceVertices;
}

void Scene::BuildLods()
{
    using namespace Maths;
    using namespace DataTypes;
    
    Btl::Vector<LodMesh> lodArray;
    
    #if !defined( _DEBUG )
    try
    #endif
    {
        // Build the root LodMesh from the Model
        
        LOGDEBUG( g_Log, "Scene::BuildLods(): Building root LOD mesh\n" );
        
        #if defined( USER_JDENTON )
        LARGE_INTEGER frequency, start, end;
        QueryPerformanceFrequency( &frequency );
        QueryPerformanceCounter( &start );
        #endif
        
        lodArray.Reserve( m_exportData.lodsToGenerate + 1 );
        lodArray.PushBack();
        //LodMesh* pRootLod = &lodArray.Back();
        
        LodMeshOptions options;
        
		options.minimumVertices = m_exportData.minimumVertices;
		options.vertexReductionRatio = m_exportData.vertexReductionRatio;

		if (m_exportData.reduceVertices)
		{	
			options.minEdgeScore = 0.99f;
			options.sharpnessPenalty = 100.0f;
			options.weldThreshold = 0.f;
			options.applyGradientPenalties = true;
			options.applyTriangleAreaPenalty = false;
			options.scalePenaltyByEdgeLength = false;
			options.textureDiscontinuityThreshold = 1.0f / 1024.0f;

			for ( S32 i=1; i<LodTriangle::m_MaxTexCoordCount; i++ )
				options.texCoordPriority[i] = 10.f;
		}
		else
		{
			options.minEdgeScore = 0.0f;
			options.sharpnessPenalty = 1000.0f;
			options.weldThreshold = 0.0001f;
			options.applyGradientPenalties = false;
			options.applyTriangleAreaPenalty = true;
			options.scalePenaltyByEdgeLength = true;
			options.textureDiscontinuityThreshold = 1.0f / 256.0f;
		}

		options.textureDiscontinuityPenalty = 10.0f; // 1.0f;
		options.materialBoundaryPenalty = 100.0f; // 100.0f;
		options.outlineEdgePenalty = 1.0e24f; // 1.0e24f; //1000000000.0f;
	    
		options.useViewDirection = false; // true;
		options.viewDirectionPenalty = 0.1f;
		options.viewDirection = Const<Maths::Normal3>::AxisZ();
	    
		options.colourPriority   = 1.0f / 255.0f;
		options.normalPriority   = 1.0f;
		options.tangentPriority  = 1.0f;
		options.binormalPriority = 1.0f;
        
        #if 1 && defined( USER_JDENTON )
        options.vertexReductionRatio = 0.95f;
        #endif
        
        lodArray.Front().SetOptions( options );
        
        AxisBox boundingBox;
        SetInvalidExtent( &boundingBox );
        
		for ( S32 blockIndex = 0; blockIndex < m_resourceBlocks.Size(); ++blockIndex )
		{
		    for ( S32 indexBufferIndex = 0; indexBufferIndex < m_resourceBlocks[blockIndex].GetNumIndexBuffers(); ++indexBufferIndex )
            {
                Build::IndexBuffer& indexBuffer(  m_resourceBlocks[blockIndex].GetIndexBuffer( indexBufferIndex ) );
                const PrimitiveType primitiveType( indexBuffer.m_primitiveType );
                
                if ( primitiveType == PRIMITIVE_TYPE_TRIANGLE_LIST )
                {
                    const IndexBuffer::IndexData& indexArray( indexBuffer.GetIndices() );
                    const S32 indexArraySize( indexArray.Size() );
                    const S32 vertexBufferCount( indexBuffer.m_vertexBuffers.Size() );
                    
                    for ( S32 vertexBufferIndex = 0; vertexBufferIndex < vertexBufferCount; vertexBufferIndex++ )
                    {
                        const VertexBuffer& vertexBuffer( GetVertexBuffer( indexBuffer.m_vertexBuffers[ vertexBufferIndex ] ) );
                        const VertexData&   vertexData( vertexBuffer.GetVertices() );
                        const VertexFormat& vertexFormat( vertexData.GetVertexFormat() );
                        const S32           vertexCount( vertexData.Size() );
                        const S32           elementCount( vertexFormat.GetNumVertexElements() );
                        
                        for ( S32 elementIndex = 0; elementIndex < elementCount; elementIndex++ )
                        {
                            const VertexFormat::VertexElement& element = vertexFormat.GetVertexElement( elementIndex );
                            if ( element.usage == VERTEX_ELEMENT_USAGE_POSITION )
                            {
                                for ( S32 vertexIndex=0; vertexIndex < vertexCount; vertexIndex++ )
                                {
	                                const Maths::Point3 p( MakePoint3( DecompressVertexElement( vertexData, elementIndex, vertexIndex ) ) );
	                                UpdateExtent( &boundingBox, p );
	                            }
                            }
                        }
                    }
                }
            }
        }
        
        #if defined( USER_JDENTON )
        LOGINFO( g_Log, "Bounding box min: %7.3f, %7.3f, %7.3f", GetF32( X( Min( boundingBox ) ) ), GetF32( Y( Min( boundingBox ) ) ), GetF32( Z( Min( boundingBox ) ) ) );
        LOGINFO( g_Log, "Bounding box max: %7.3f, %7.3f, %7.3f", GetF32( X( Max( boundingBox ) ) ), GetF32( Y( Max( boundingBox ) ) ), GetF32( Z( Max( boundingBox ) ) ) );
        #endif
        
        lodArray.Front().SetBoundingBox( boundingBox );
        
        S32 degenerateTriangles = 0;
        
		for ( S32 blockIndex = 0; blockIndex < m_resourceBlocks.Size(); ++blockIndex )
		{
		    for ( S32 indexBufferIndex = 0; indexBufferIndex < m_resourceBlocks[blockIndex].GetNumIndexBuffers(); ++indexBufferIndex )
            {
                Build::IndexBuffer& indexBuffer(  m_resourceBlocks[blockIndex].GetIndexBuffer( indexBufferIndex ) );
                const PrimitiveType primitiveType( indexBuffer.m_primitiveType );
                
                if ( primitiveType == PRIMITIVE_TYPE_TRIANGLE_LIST )
                {
                    const IndexBuffer::IndexData& indexArray( indexBuffer.GetIndices() );
                    const S32 indexArraySize( indexArray.Size() );
                    
                    Build::VertexBuffer& vertexBuffer( GetVertexBuffer( indexBuffer.m_vertexBuffers[0] ) );
                    const S32 vertexCount( vertexBuffer.GetVertices().Size() );
                    const S32 triangleCount( indexArraySize / 3 );
                    
                    for ( S32 indexArrayIndex=0; indexArrayIndex < indexArraySize; indexArrayIndex += 3 )
                    {
                        const U32 vbIndex0( indexArray[ indexArrayIndex + 0 ] );
                        const U32 vbIndex1( indexArray[ indexArrayIndex + 1 ] );
                        const U32 vbIndex2( indexArray[ indexArrayIndex + 2 ] );
                        
                        const S32 triangleIndex( lodArray.Front().AddTriangle( &indexBuffer, vertexCount, vbIndex0, vbIndex1, vbIndex2 ) );
                        
                        const S32 vertexBufferCount( indexBuffer.m_vertexBuffers.Size() );
                        for ( S32 vertexBufferIndex = 0; vertexBufferIndex < vertexBufferCount; vertexBufferIndex++ )
                        {
                            const VertexBuffer& vertexBuffer( GetVertexBuffer( indexBuffer.m_vertexBuffers[ vertexBufferIndex ] ) );
                            const VertexData&   vertexData( vertexBuffer.GetVertices() );
                            
                            degenerateTriangles += (S32)AddLodMeshTriangleVertexData( &lodArray.Front(), triangleIndex, vertexData );
                        }
                    }
                }
            }
        }
        
        if ( degenerateTriangles > 0 )
        {
            LOGDEBUG( g_Log, "Deleting %d degenerate triangles in input mesh", degenerateTriangles );
            
            S32 triangleIndex = 0;
            
            while ( triangleIndex < lodArray.Front().TriangleCount() )
            {
                LodTriangle& triangle( lodArray.Front().Triangle( triangleIndex ) );
                
                const S32 vertexIndex0( triangle.VertexIndex( 0 ) );
                const S32 vertexIndex1( triangle.VertexIndex( 1 ) );
                const S32 vertexIndex2( triangle.VertexIndex( 2 ) );
                
                if ( ( vertexIndex0 == vertexIndex1 ) ||
                     ( vertexIndex0 == vertexIndex2 ) ||
                     ( vertexIndex1 == vertexIndex2 ) )
                {
                    // NOTE: We don't increment 'triangleIndex' if we delete the triangle because
                    // the delete operation works by swapping the last element of the vector with
                    // the element to be deleted and then decrementing the size of the vector!
                    lodArray.Front().DeleteTriangleFromAdjacentVertices( triangleIndex );
                    lodArray.Front().DeleteTriangleFromAdjacentEdges( triangleIndex );
                    lodArray.Front().DeleteTriangle( triangleIndex );
                }
                else
                {
                    ++triangleIndex;
                }
            }
        }
        
        #if defined( _DEBUG )
        // Validate the built mesh
        lodArray.Front().Validate();
        #endif
        
        // Analyse the root LodMesh to generate data for the edge reduction error metric
        lodArray.Front().AnalyseMesh();

		S32 startIndex = (m_exportData.lodsToGenerate == 0) ? 0 : 1;
        
        // Generate the LodMesh LOD's
        #if 1 && defined( USER_JDENTON )
        for ( S32 i=1; i<=50; i++ )
        #else
        for ( S32 i=startIndex; i<=m_exportData.lodsToGenerate; i++ )
        #endif
        {
            if ( i == 0 || lodArray[i-1].VertexCount() > m_exportData.minimumVertices )
            {
				if (i > 0)
				{
					lodArray.PushBack();
					lodArray[i-1].Clone( &lodArray[i] );
				}
                lodArray[i].Reduce();
            }
            else
            {
                break;
            }
        }
        
        // Generate the Model LOD's from the LodMesh LOD's
        for ( S32 i=startIndex; i<lodArray.Size(); i++ )
        {
            CreateLodIndexBuffers( &lodArray[i], i );
        }
        
        // Finished
        lodArray.Destroy();
        
        #if defined( USER_JDENTON )
        QueryPerformanceCounter( &end );
        const F64 delta( (F64)( end.QuadPart - start.QuadPart ) );
        const F64 time( delta / (F64)frequency.QuadPart );
        LOGINFO( g_Log, "Scene::BuildLods(): Finished in %d minutes & %d seconds\n", ((S32)time) / 60, ((S32)time) % 60 );
        #endif
    }
    #if !defined( _DEBUG )
    catch ( ... )
    {
        LOGWARN( g_Log, "Scene::BuildLods(): Crashed!" );
    }
    #endif
    
    #if !defined( _DEBUG )
    try
    #endif
    {
        lodArray.Destroy();
    }
    #if !defined( _DEBUG )
    catch ( ... )
    {
        LOGWARN( g_Log, "Scene::BuildLods(): Crashed deleting lodArray!" );
    }
    #endif
}

void Scene::CreatePrimitiveLods(Primitive& primitive)
{
	const IndexBuffer& indexBuffer(GetIndexBuffer(primitive.indexBufferIndex));

	primitive.lods.Resize(indexBuffer.GetNumLods());

	for (int i = 1; i < primitive.lods.Size(); i++)
	{
		primitive.lods[i].nIndices = indexBuffer.GetLodSize(i);
		primitive.lods[i].offset = indexBuffer.GetLodOffset(i);
		BIZ_ASSERT(primitive.lods[i].offset + primitive.lods[i].nIndices <= GetIndexBuffer(primitive.indexBufferIndex).GetIndices().Size());
	
	}
}

void Scene::BuildVertexDeclarations(Model& model)
{
	for (int p = 0; p < model.m_primitives.Size(); p++)
	{
		VertexDeclaration vertexDeclaration;

		ResourceIndex indexBufferIndex = model.m_primitives[p].indexBufferIndex;

		const Build::IndexBuffer& indexBuffer(GetIndexBuffer(indexBufferIndex));

		Btl::Vector<U8, Btl::VectorParameters_Array<VERTEX_ELEMENT_USAGE_MAX> > usageCount;

		for (int i = 0; i < usageCount.Size(); i++)
			usageCount[i] = 0;

		for (int vb = 0; vb < indexBuffer.m_vertexBuffers.Size(); ++vb)
		{
			const VertexBuffer& vertexBuffer(GetVertexBuffer(indexBuffer.m_vertexBuffers[vb]));

			if (vertexBuffer.GetStreamIndex() > -1)
			{
				// only include each stream once
				bool match = false;
				for (int e = 0; e < vertexDeclaration.GetNumVertexElements(); e++)
					if (vertexDeclaration.GetVertexElement(e).stream == vertexBuffer.GetStreamIndex())
						match = true;
		
				if (!match)
				{
					const VertexFormat& vertexFormat(vertexBuffer.GetVertices().GetVertexFormat());

					for (int e = 0; e < vertexFormat.GetNumVertexElements(); e++)
					{
						S32 i = vertexDeclaration.AddVertexElement();
						VertexDeclaration::VertexElement& element(vertexDeclaration.GetVertexElement(i));

						element.stream = vertexBuffer.GetStreamIndex(); 
						element.offset = vertexFormat.GetOffset(e);
						element.type = vertexFormat.GetVertexElement(e).type;
						element.method = vertexFormat.GetVertexElement(e).method;
						element.usage = vertexFormat.GetVertexElement(e).usage;
						element.usageIndex = vertexFormat.GetVertexElement(e).usageIndex;
					}
				}
			}
		}

		for (int vb = 0; vb < indexBuffer.m_vertexFormatData.Size(); ++vb)
		{
			const IndexBuffer::VertexFormatData& vertexFormatData(indexBuffer.m_vertexFormatData[vb]);

			for (int e = 0; e < vertexDeclaration.GetNumVertexElements(); e++)
				if (vertexDeclaration.GetVertexElement(e).stream == vertexFormatData.streamIndex)
					BIZ_ASSERT_FAIL("Tried to add stream twice in vertex declaration");

			const VertexFormat& vertexFormat(vertexFormatData.format);

			for (int e = 0; e < vertexFormat.GetNumVertexElements(); e++)
			{
				S32 i = vertexDeclaration.AddVertexElement();
				VertexDeclaration::VertexElement& element(vertexDeclaration.GetVertexElement(i));

				element.stream = vertexFormatData.streamIndex; 
				element.offset = vertexFormat.GetOffset(e);
				element.type = vertexFormat.GetVertexElement(e).type;
				element.method = vertexFormat.GetVertexElement(e).method;
				element.usage = vertexFormat.GetVertexElement(e).usage;
				element.usageIndex = vertexFormat.GetVertexElement(e).usageIndex;
			}
		}

		S32 index = InsertVertexDeclaration(vertexDeclaration);
		model.m_primitives[p].vertexDeclaration = index;
	}
}

void Scene::PrintHierarchy(const Node& node, S32 level) const
{
	if (level == 0)
		LOGDEBUG(g_Log, "************ Tree **************");

	if (node.GetName() != "")
	{
		Btl::String output;

		for (int i = 0; i < level; i++)
			output += "\t";

		output += node.GetName();

		LOGDEBUG(g_Log, output.Chars());
	}

	if (node.IsNode(NODE_TYPE_CULL))
	{
		CullNode* pCullNode = (CullNode*) &node;
		Point3 max = pCullNode->GetBounds().aabbMax;
		Point3 min = pCullNode->GetBounds().aabbMin;

		Btl::String output;

		for (int i = 0; i < level; i++)
			output += "\t";

		output += "min (%0.2f, %0.2f, %0.2f), max (%0.2f, %0.2f, %0.2f).";

		LOGDEBUG(g_Log, output.Chars(), min.x, min.y, min.z, max.x, max.y, max.z);
	}

	for (int i = 0; i < node.GetNumSubNodes(); i++)
	{
		const Node* pSubNode = GetNodePtr(node.GetSubNode(i));

		if (pSubNode)
			PrintHierarchy(*pSubNode ,level + 1);
	}

	if (level == 0)
		LOGDEBUG(g_Log, "********************************");
}

void Scene::CalculateSizeInfo() const
{
	LOGDEBUG(g_Log, "******** Scene::CalculateSizeInfo ********\n");

	memset(&m_exportData.sizeInfo, 0, sizeof(m_exportData.sizeInfo));

	m_exportData.sizeInfo.nMaterials = m_materialList.GetNumMaterials();
	m_exportData.sizeInfo.nMaterialTypes = m_materialList.GetNumMaterialTypes();
	m_exportData.sizeInfo.nBlocks = GetNumResourceBlocks();

	for (int i = 0; i < m_materialList.GetNumMaterials(); i++)
	{
		if (m_materialList.GetMaterialType(m_materialList.GetMaterial(i).GetTypeIndex()).GetEffect() > -1)
			m_exportData.sizeInfo.nEffectMaterials++;
		else
			m_exportData.sizeInfo.nSharedMaterials++;

		m_exportData.sizeInfo.materialDataSize += m_materialList.GetMaterial(i).GetDataSize();
	}

	for (int blockIndex = 0; blockIndex < GetNumResourceBlocks(); blockIndex++)
	{
		const ResourceBlock& block = GetResourceBlock(blockIndex);

		m_exportData.sizeInfo.nVertexBuffers += block.GetNumVertexBuffers();
		m_exportData.sizeInfo.nIndexBuffers += block.GetNumIndexBuffers();
		m_exportData.sizeInfo.nStreamedTextures += block.GetNumTextures();

		for (int vb = 0; vb < block.GetNumVertexBuffers(); vb++)
		{
			S32 vertexSize = block.GetVertexBuffer(vb).GetVertices().GetVertexFormat().GetDataSize();
			S32 vertexBufferSize = block.GetVertexBuffer(vb).GetVertices().Size() * vertexSize;
			m_exportData.sizeInfo.vertexDataSize += vertexBufferSize;
			m_exportData.sizeInfo.nVertices += block.GetVertexBuffer(vb).GetVertices().Size();
		}

		for (int ib = 0; ib < block.GetNumIndexBuffers(); ib++)
		{
			S32 indexSize = 2; // Assume 16 bit
			S32 indexBufferSize = block.GetIndexBuffer(ib).GetIndices().Size() * indexSize;
			m_exportData.sizeInfo.indexDataSize += indexBufferSize;
			m_exportData.sizeInfo.nIndices += block.GetIndexBuffer(ib).GetIndices().Size();
		}

		for (int it = 0; it < block.GetNumTextures(); it++)
		{
			// to do: use texture info cache
			TextureExport::TextureInfo info;
			ActTools::Texture &texture = *block.GetTexture(it).GetTextureManager()->GetTexture(block.GetTexture(it).GetCacheIndex(), info);

			m_exportData.sizeInfo.streamedTextureSize += texture.GetSize();
		}
	}

	for (int i = 0; i < m_sharedTextures.GetNumTextures(); i++)
	{
		// to do: use texture info cache
		TextureExport::TextureInfo info;
		ActTools::Texture &texture = *m_sharedTextures.GetTexture(i).GetTextureManager()->GetTexture(m_sharedTextures.GetTexture(i).GetCacheIndex(), info);

		m_exportData.sizeInfo.sharedTextureSize += texture.GetSize();
	}

	m_exportData.sizeInfo.nSharedTextures = m_sharedTextures.GetNumTextures();
	m_exportData.sizeInfo.totalStreamedSize = m_exportData.sizeInfo.indexDataSize + m_exportData.sizeInfo.vertexDataSize + m_exportData.sizeInfo.streamedTextureSize;
	m_exportData.sizeInfo.totalSize = m_exportData.sizeInfo.totalStreamedSize + m_exportData.sizeInfo.sharedTextureSize + m_exportData.sizeInfo.materialDataSize;
	m_exportData.sizeInfo.totalTextureSize = m_exportData.sizeInfo.sharedTextureSize + m_exportData.sizeInfo.streamedTextureSize;

	ProcessNodes(&GetRootNode(), &Scene::CountPrimitives);

	LOGDEBUG(g_Log, "Streamed data size: %i bytes (v:%i i:%i t:%i) in %i blocks.\n", m_exportData.sizeInfo.totalStreamedSize, m_exportData.sizeInfo.vertexDataSize, m_exportData.sizeInfo.indexDataSize, m_exportData.sizeInfo.streamedTextureSize, m_exportData.sizeInfo.nBlocks);
	LOGDEBUG(g_Log, "Streamed count: %i vertices (%i buffers), %i indices (%i buffers) and %i textures.\n", m_exportData.sizeInfo.nVertices, m_exportData.sizeInfo.nVertexBuffers, m_exportData.sizeInfo.nIndices, m_exportData.sizeInfo.nIndexBuffers, m_exportData.sizeInfo.nStreamedTextures);
	LOGDEBUG(g_Log, "Shared data: % i bytes (%i textures)", m_exportData.sizeInfo.sharedTextureSize, m_exportData.sizeInfo.nSharedTextures);
	LOGDEBUG(g_Log, "Render lists: %i lists containing %i primitives, %i instances (%i base primitives)\n", m_exportData.sizeInfo.nRenderLists, m_exportData.sizeInfo.nPrimitives, m_exportData.sizeInfo.nInstancePrimitives, m_exportData.sizeInfo.nBaseInstancePrimitives);
	LOGDEBUG(g_Log, "Materials: %i materials (%i effect, %i shared), %i material types (%i bytes) \n", m_exportData.sizeInfo.nMaterials, m_exportData.sizeInfo.nEffectMaterials, m_exportData.sizeInfo.nSharedMaterials, m_exportData.sizeInfo.nMaterialTypes, m_exportData.sizeInfo.materialDataSize);
	LOGDEBUG(g_Log, "******************************************\n");
}

void Scene::CountPrimitives(const Node& node) const
{
	if (const RenderListNode* pRenderList = RenderListNode::GetPointer(&node))
	{
		m_exportData.sizeInfo.nRenderLists++;
		m_exportData.sizeInfo.nPrimitives += pRenderList->GetNumPrimitives();
		m_exportData.sizeInfo.nInstancePrimitives += pRenderList->GetNumInstances();
		m_exportData.sizeInfo.nBaseInstancePrimitives += pRenderList->GetNumBasePrimitives();
	}
}

}}
