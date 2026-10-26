// StreamManager
inline StreamManager& StreamManager::Instance()
{
	return s_instance;
}

// StreamData
inline bool StreamData::IsLoaded(S32 chunk) const
{
	return m_chunkState[chunk].loaded;
}

inline bool StreamData::IsCached(S32 chunk) const
{
	return m_chunkState[chunk].cached;
}

inline bool StreamData::NodesChanged() const
{
	return m_nodesChanged;
}

inline bool StreamData::BlocksChanged() const
{
	return m_blocksChanged > 0;
}

inline bool StreamData::CacheChanged() const
{
	return m_chunksChanged;
}

inline S32 StreamData::LoadedChunk() const
{
	return m_loadedChunk;
}

inline S32 StreamData::UnloadedChunk() const
{
	return m_unloadedChunk;
}

inline bool StreamData::MultipleChunksUpdated() const
{
	return m_multipleChunksUpdated;
}

// Scene
inline void Scene::SetDebugName(const char* pName)
{
	m_pDebugName = pName;
}

inline const char* Scene::GetDebugName() const 
{
	return m_pDebugName;
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

inline S32 Scene::GetNumVertexDeclarations() const
{
	return m_vertexDeclarations.Size();
}

inline const VertexDeclaration& Scene::GetVertexDeclaration(S32 i) const
{
	return m_vertexDeclarations[i];
}

inline S32 Scene::GetNumEffects() const
{
	return m_effects.Size();
}

inline const Effect& Scene::GetEffect(S32 i) const
{
	return m_effects[i];
}

inline Effect& Scene::GetEffect(S32 i)
{
	return m_effects[i];
}

inline StreamData& Scene::GetStreamData()
{
	return m_streamData;
}

inline const StreamData& Scene::GetStreamData() const
{
	return m_streamData;
}

inline void Scene::SetMaxLodToLoad(S32 i)
{
	m_maxLodToLoad = i;
}

inline S32 Scene::GetMaxLodToLoad() const
{
	return m_maxLodToLoad;
}

inline const FX::SharedBindingBits *Scene::GetSharedBindingUsage() const
{
	return &m_SharedFXUsage;
}

inline S32 Scene::GetNumDynamicMaterialLists() const
{
	return m_dynamicMaterialLists.Size();
}

inline const MaterialList* Scene::GetDynamicMaterialList(S32 i) const
{
	if (m_dynamicMaterialLists.Size() <= i)
		return 0;
	else
		return Pointer(m_dynamicMaterialLists[i]);
}

// ResourceBlock
inline ResourceBlock::ResourceBlock()
{
}

inline ResourceBlock::Resources::Resources() : lod(-1), pScene(0), discard(false)
{
}

inline S32 ResourceBlock::GetLod() const
{
	return m_resources.lod;
}

inline S32 ResourceBlock::GetNumVertexBuffers() const
{
	return m_resources.vertexBuffers.Size();
}

inline S32 ResourceBlock::GetNumIndexBuffers() const
{
	return m_resources.indexBuffers.Size();
}

inline S32 ResourceBlock::GetNumTextures() const
{
	return m_resources.textures.Size();
}

inline S32 ResourceBlock::GetNumNodes() const
{
	return m_resources.nodes.Size();
}

inline const VertexBuffer& ResourceBlock::GetVertexBuffer(S32 i) const
{
	assert(m_resources.vertexBuffers[i] != 0);
	return *m_resources.vertexBuffers[i];
}

inline const IndexBuffer& ResourceBlock::GetIndexBuffer(S32 i) const
{
	assert(m_resources.indexBuffers[i] != 0);
	return *m_resources.indexBuffers[i];
}

inline const Texture& ResourceBlock::GetTexture(S32 i) const
{
	assert(m_resources.textures[i] != 0);
	return *m_resources.textures[i];
}

inline const Node* ResourceBlock::GetNodePtr(S32 i) const
{
	return Pointer(m_resources.nodes[i]);
}

inline Node* ResourceBlock::GetNodePtr(S32 i)
{
	return Pointer(m_resources.nodes[i]);
}

inline void ResourceBlock::SetScenePtr(Scene* pScene)
{
	m_resources.pScene = pScene;
}

inline S32 ResourceBlock::GetMaxLodToLoad() const
{
	assert(m_resources.pScene);
	return m_resources.pScene->GetMaxLodToLoad();
}

// SharedTextureList
inline const Texture* SharedTextureList::GetTexture(S32 i)
{
	return m_textures[i];
}

// SharedTextureManager
inline SharedTextureManager& SharedTextureManager::Instance()
{
	return s_instance;
}

//extern Sys::MemoryBuffer g_resourceAllocator;
extern Memory::BlockAllocator g_resourceAllocator;
extern MT::Mutex g_resourceAllocatorMutex;

#ifndef CFG_SPEED
extern int g_resourceAllocatorLock;
#endif

// Resource Allocator
template<typename T>
T* ResourceAllocator<T>::Allocate(int n) 
{
	#ifndef CFG_SPEED
	BIZ_ASSERT(g_resourceAllocatorLock == 0);
	MT::AtomicIncrement(&g_resourceAllocatorLock);
	BIZ_ASSERT(g_resourceAllocatorLock == 1);
	#endif

	assert(n > 0);

	//g_resourceAllocatorMutex.Wait();
	T* p = (T*) g_resourceAllocator.Allocate(n * sizeof(T));
	//g_resourceAllocatorMutex.Release();

	BIZ_ASSERT_MSG(p, "size %i", n * sizeof(T));

	if (p)
	{
		for (int i = 0; i < n; i++)
			new (p + i) T;
	}

	#ifndef CFG_SPEED
	MT::AtomicDecrement(&g_resourceAllocatorLock);
	#endif

	return p;
}

template<typename T>
void ResourceAllocator<T>::Free(T* p, int n)
{
	if (p)
	{
		assert(n > 0);

		for (int i = 0; i < n; i++)
			(p + i)->~T();

		#ifndef CFG_SPEED
		BIZ_ASSERT(g_resourceAllocatorLock == 0);
		MT::AtomicIncrement(&g_resourceAllocatorLock);
		BIZ_ASSERT(g_resourceAllocatorLock == 1);
		#endif

		//g_resourceAllocatorMutex.Wait();
		g_resourceAllocator.Free((U8*)p);//, n * sizeof(T));
		//g_resourceAllocatorMutex.Release();

		#ifndef CFG_SPEED
		MT::AtomicDecrement(&g_resourceAllocatorLock);
		#endif
	}
}