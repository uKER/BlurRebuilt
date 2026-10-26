// Texture
inline Texture::Texture()
{
	m_cacheIndex = -1;
	m_pTextureManager = 0;
	m_textureScaleCalculated = false;
}

inline void Texture::Initialise(const Btl::String& name, S32 cacheIndex, U32 hashValue, TextureExport::TextureManager* pTextureManager)
{
	assert(m_pTextureManager == 0);

	m_name = name;
	m_cacheIndex = cacheIndex;
	m_hashValue = hashValue;
	m_pTextureManager = pTextureManager;
}

inline const Btl::String& Texture::GetName() const
{
	return m_name;
}

inline U32 Texture::GetHashValue() const
{
	return m_hashValue;
}		

inline S32 Texture::GetCacheIndex() const
{
	return m_cacheIndex;
}

inline const SamplerState& Texture::GetSamplerState() const
{
	return m_samplerState;
}

inline SamplerState& Texture::GetSamplerState()
{
	return m_samplerState;
}

inline TextureExport::TextureManager* Texture::GetTextureManager() const
{
	return m_pTextureManager;
}

inline bool operator == (const Texture& p1, const Texture& p2)
{
	return p1.GetCacheIndex() == p2.GetCacheIndex()
			&& p1.GetSamplerState() == p2.GetSamplerState();
}

inline bool operator != (const Texture& p1, const Texture& p2)
{
	return p1.GetCacheIndex() != p2.GetCacheIndex()
			|| p1.GetSamplerState() != p2.GetSamplerState();
}

inline bool operator < (const Texture& p1, const Texture& p2)
{
	if (p1.GetCacheIndex() < p2.GetCacheIndex()) return true;
	if (p1.GetCacheIndex() > p2.GetCacheIndex()) return false;
	if (p1.GetHashValue() < p2.GetHashValue()) return true;
	if (p1.GetHashValue() > p2.GetHashValue()) return false;
	if (p1.GetName() < p2.GetName()) return true;
	if (p1.GetName() > p2.GetName()) return false;
	return p1.GetSamplerState() < p2.GetSamplerState();
}

// ResourceBlock
inline ResourceBlock::ResourceBlock() : m_pScene(0), m_blockIndex(-1), m_renderNodeHash(0)
{
}

inline ResourceBlock::~ResourceBlock()
{
	int breakhere = 0;
}

inline S32 ResourceBlock::GetNumNodes() const 
{
	return m_nodes.Size();
}

inline const Node* ResourceBlock::GetNodePtr(S32 i) const 
{
	return &*m_nodes[i];
}

inline Node* ResourceBlock::GetNodePtr(S32 i) 
{
	return &*m_nodes[i];
}

inline S32 ResourceBlock::AddNode(Btl::AutoPointer<Node> p) 
{
	m_nodes.PushBack(p);
	m_nodes.Back()->SetScenePtr(m_pScene);
	m_nodes.Back()->SetLocation(m_blockIndex, m_nodes.Size() - 1);
	return m_nodes.Size() - 1;
}

inline void ResourceBlock::ClearNode(S32 i)
{
	Free(m_nodes[i]);
}

inline S32 ResourceBlock::GetNumVertexBuffers() const
{
	return m_vertexBuffers.Size();
}

inline bool ResourceBlock::Empty() const
{
	return m_vertexBuffers.Size() == 0
		&& m_indexBuffers.Size() == 0
		&& m_textures.Size() == 0
		&& m_nodes.Size() == 0;
}

inline const VertexBuffer& ResourceBlock::GetVertexBuffer(S32 i) const
{
	return *m_vertexBuffers[i];
}

inline VertexBuffer& ResourceBlock::GetVertexBuffer(S32 i)
{
	return *m_vertexBuffers[i];
}

inline S32 ResourceBlock::GetNumIndexBuffers() const
{
	return m_indexBuffers.Size();
}

inline const IndexBuffer& ResourceBlock::GetIndexBuffer(S32 i) const
{
	return *m_indexBuffers[i];
}

inline IndexBuffer& ResourceBlock::GetIndexBuffer(S32 i)
{
	return *m_indexBuffers[i];
}

inline S32 ResourceBlock::AddIndexBuffer(PrimitiveType primitiveType)
{
	m_indexBuffers.PushBack();
	Allocate(m_indexBuffers.Back());
	m_indexBuffers.Back()->SetPrimitiveType(primitiveType);
	return m_indexBuffers.Size() - 1;
}

inline S32 ResourceBlock::GetNumTextures() const
{
	return m_textures.Size();
}

inline const Texture& ResourceBlock::GetTexture(S32 i) const
{
	return m_textures[i];
}

inline void ResourceBlock::SetScenePtr(Scene* pScene)
{
	m_pScene = pScene;
}

inline Scene* ResourceBlock::GetScenePtr() const
{
	return m_pScene;
}

inline const Bounds& ResourceBlock::GetFullBounds() const
{
	return m_fullBounds;
}

inline const Bounds& ResourceBlock::GetSubBounds(int i) const
{
	return m_subBounds[i];
}

inline int ResourceBlock::GetNumBounds() const
{
	return m_subBounds.Size();
}

inline const Btl::BitVector& ResourceBlock::GetPvsBits() const
{
	return m_pvsBits;
}

inline const Btl::BitVector& ResourceBlock::GetPvsBits(S32 lod) const
{
	if (m_lodPvs.Size() > 0)
		return m_lodPvs[lod];
	else
		return m_pvsBits;
}

inline const Btl::BitVector& ResourceBlock::GetRoomBits() const
{
	return m_roomBits;
}

inline void Move(void* pDst, ResourceBlock* pSrc) 
{
	ResourceBlock* p = (ResourceBlock*) pDst;
	Btl::Move(&p->m_vertexBuffers, &pSrc->m_vertexBuffers);
	Btl::Move(&p->m_indexBuffers, &pSrc->m_indexBuffers);
	Btl::Move(&p->m_textures, &pSrc->m_textures);
	Btl::Move(&p->m_nodes, &pSrc->m_nodes);
	Btl::Move(&p->m_textureMap, &pSrc->m_textureMap);
	Btl::Move(&p->m_fullBounds, &pSrc->m_fullBounds);
	Btl::Move(&p->m_subBounds, &pSrc->m_subBounds);
	Btl::Move(&p->m_pvsBits, &pSrc->m_pvsBits);
	Btl::Move(&p->m_lodPvs, &pSrc->m_lodPvs);
	Btl::Move(&p->m_groups, &pSrc->m_groups);
	Btl::Move(&p->m_renderNodeHash, &pSrc->m_renderNodeHash);
	Btl::Move(&p->m_pScene, &pSrc->m_pScene);
	Btl::Move(&p->m_blockIndex, &pSrc->m_blockIndex);
	Btl::Move(&p->m_roomBits, &pSrc->m_roomBits);
}

inline void MoveArray(void* pDst, ResourceBlock* pSrc, int n) 
{
	for (int i = 0; i < n; i++)
		Move(((ResourceBlock*)pDst) + i, pSrc + i);
}

inline void ResourceBlock::RemoveEmptyNodes()
{
	Btl::Remove(m_nodes, Btl::AutoPointer<Node>());
}

inline U32 ResourceBlock::GetRenderNodeHash() const
{
	return m_renderNodeHash;
}

inline const Btl::Set<S32>& ResourceBlock::GetGroups() const
{
	return m_groups;
}

// TextureList
inline S32 TextureList::GetNumTextures() const
{
	return m_textures.Size();
}

inline const Texture& TextureList::GetTexture(S32 i) const
{
	return m_textures[i];
}

inline void TextureList::Clear()
{
	m_textures.Clear();
	m_textureMap.Clear();
}

// Scene
inline Scene::Scene()
{
	m_exportData.platformType = PLATFORM_TYPE_UNKNOWN;
	m_exportData.exportComplete = false;
	m_exportData.lodsToGenerate = 0;
	m_exportData.reduceVertices = false;
	m_streamData.m_pScene = this;
}

inline const RootNode& Scene::GetRootNode() const
{
	return m_rootNode;
}

inline RootNode& Scene::GetRootNode()
{
	return m_rootNode;
}

inline S32 Scene::GetNumResourceBlocks() const 
{
	return m_resourceBlocks.Size();
}

inline const ResourceBlock& Scene::GetResourceBlock(S32 i) const 
{
	return m_resourceBlocks[i];
}

inline ResourceBlock& Scene::GetResourceBlock(S32 i) 
{
	return m_resourceBlocks[i];
}

inline const VertexBuffer& Scene::GetVertexBuffer(const ResourceIndex& i) const
{
	return m_resourceBlocks[i.GetBlock()].GetVertexBuffer(i.GetIndex());
}

inline const IndexBuffer& Scene::GetIndexBuffer(const ResourceIndex& i) const
{
	return m_resourceBlocks[i.GetBlock()].GetIndexBuffer(i.GetIndex());
}

inline VertexBuffer& Scene::GetVertexBuffer(const ResourceIndex& i)
{
	return m_resourceBlocks[i.GetBlock()].GetVertexBuffer(i.GetIndex());
}

inline IndexBuffer& Scene::GetIndexBuffer(const ResourceIndex& i)
{
	return m_resourceBlocks[i.GetBlock()].GetIndexBuffer(i.GetIndex());
}

inline const Texture& Scene::GetTexture(const ResourceIndex& i) const
{
	if (i.GetBlock() > -1)
		return m_resourceBlocks[i.GetBlock()].GetTexture(i.GetIndex());
	else
		return m_sharedTextures.GetTexture(i.GetIndex());
}

inline const MaterialList& Scene::GetMaterialList() const
{
	return m_materialList;
}

inline MaterialList& Scene::GetMaterialList()
{
	return m_materialList;
}

inline void Scene::CreateGlobalDynamicMaterialList(S32 i)
{
	assert(i > -1 && i < 15);
	m_dynamicMaterialLists.Resize(Maths::Max(m_dynamicMaterialLists.Size(), i + 1));

	if (m_dynamicMaterialLists[i] == 0)
		Allocate(m_dynamicMaterialLists[i]);
}

inline MaterialList* Scene::GetDynamicMaterialList(S32 i)
{
	if (m_dynamicMaterialLists.Size() <= i)
		return 0;
	else
		return Pointer(m_dynamicMaterialLists[i]);
}

inline const MaterialList* Scene::GetDynamicMaterialList(S32 i) const
{
	if (m_dynamicMaterialLists.Size() <= i)
		return 0;
	else
		return Pointer(m_dynamicMaterialLists[i]);
}

inline const StreamData& Scene::GetStreamData() const
{
	return m_streamData;
}

inline StreamData& Scene::GetStreamData()
{
	return m_streamData;
}

inline const ResourceIndex& Scene::GetTexture(S32 textureIndex) const
{
	return m_textures[textureIndex];
}

inline S32 Scene::GetNumVertexDeclarations() const
{
	return m_vertexDeclarations.Size();
}

inline const VertexDeclaration& Scene::GetVertexDeclaration(S32 i) const
{
	return m_vertexDeclarations[i];
}

// VertexBuffer
inline VertexBuffer::VertexBuffer() : m_batchID(-1), m_instanceID(-1), m_streamIndex(-1)
{
}

inline const VertexData& VertexBuffer::GetVertices() const
{
	return m_vertices;
}

inline VertexData& VertexBuffer::GetVertices()
{
	return m_vertices;
}

inline S32 VertexBuffer::GetBatchID() const
{
	return m_batchID;
}

inline S32 VertexBuffer::GetInstanceID() const
{
	return m_instanceID;
}

inline S32 VertexBuffer::GetStreamIndex() const
{
	return m_streamIndex;
}

// IndexBuffer
inline const IndexBuffer::IndexData& IndexBuffer::GetIndices() const
{
	return m_indices;
}

inline IndexBuffer::IndexData& IndexBuffer::GetIndices()
{
	return m_indices;
}

inline U32 IndexBuffer::ResetIndex32()
{
	return 0xffffff;
}

inline U16 IndexBuffer::ResetIndex16()
{
	return 0xffff;
}

inline void IndexBuffer::SetPrimitiveType(PrimitiveType primitiveType)
{
	m_primitiveType = primitiveType;
}

inline PrimitiveType IndexBuffer::GetPrimitiveType() const
{
	return m_primitiveType;
}

inline void IndexBuffer::InsertVertexBuffer(ResourceIndex vertexBufferIndex)
{
	for (int i = 0; i < m_vertexBuffers.Size(); i++)
		if (m_vertexBuffers[i] == vertexBufferIndex)
			return;

	m_vertexBuffers.PushBack();
	m_vertexBuffers.Back() = vertexBufferIndex;
}

inline S32 IndexBuffer::GetNumLods() const
{
	return Maths::Max(m_lodSizes.Size(), 1);
}

inline S32 IndexBuffer::GetLodOffset(S32 lod) const
{
	S32 offset = 0;

	for (int i = 0; i < lod; i++) 
		offset += m_lodSizes[i];

	return offset;
}

inline S32 IndexBuffer::GetLodSize(S32 lod) const
{
	if (lod == 0 && m_lodSizes.Empty())
		return m_indices.Size();
	else
		return m_lodSizes[lod];
}

// RenderListNode
inline NodeType RenderListNode::GetNodeType() const
{
	return NODE_TYPE_RENDER_LIST_BUILD;
}

inline bool RenderListNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_RENDER_LIST_BUILD) return true;
	else return RenderingData::RenderListNode::IsNode(t);
}

inline RenderListNode* RenderListNode::Clone() const
{
#if !PLATFORM_SPU
	return new RenderListNode(*this);
#endif
}

inline const RenderListNode* RenderListNode::GetPointer(const Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST_BUILD))
		return (const RenderListNode*) p;
	else
		return 0;
}

inline RenderListNode* RenderListNode::GetPointer(Node* p)
{
	if (p && p->IsNode(NODE_TYPE_RENDER_LIST_BUILD))
		return (RenderListNode*) p;
	else
		return 0;
}
