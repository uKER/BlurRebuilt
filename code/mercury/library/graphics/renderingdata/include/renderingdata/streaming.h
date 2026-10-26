#ifndef RD_STREAMING_H
#define RD_STREAMING_H
#ifdef _MSC_VER
#pragma once
#endif

#ifndef RENDERINGDATA_ALLOW_INCLUDE
#error "Illegal include of renderingdata library internals.h"
#endif

#include "btl/containers/list.h"
#include "btl/containers/chainedhashmap.h"
#include "node.h"
#include "renderlist.h"
#include "scene.h"
#include "Display.h"
#include "renderstates.h"
#include "streamdata.h"
#include "system/memory/memorybuffer.h"
#include "Display/Texture/TextureDesc.h"
#include "system/scheduler/jobqueue.h"
#include "memcopyworker.h"
#include "renderingdata/EffectVariationCallback.h"
#include "streamrequest.h"
#include <fx/fx.h>
#include "system\memory\blockallocator.h"

// Types created when loading scene data (not required for export).
namespace RenderingData {
namespace Target
{
	// Contains resources in platform-specific format.
	class ResourceBlock
	{
	public:
		ResourceBlock();

		S32 GetLod() const;

		S32 GetNumVertexBuffers() const;
		S32 GetNumIndexBuffers() const;
		S32 GetNumTextures() const;
		S32 GetNumNodes() const;

		S32 GetMaxLodToLoad() const;

		const VertexBuffer& GetVertexBuffer(S32 i) const;
		const IndexBuffer& GetIndexBuffer(S32 i) const;
		const Texture& GetTexture(S32 i) const;
		const Node* GetNodePtr(S32 i) const;
		Node* GetNodePtr(S32 i);

		void SetScenePtr(Scene* pScene);
		void SetChunk(S32 chunkIndex); // debug

		FRIEND_SERIALISE(ResourceBlock)

		struct Resources
		{
			Resources();
			~Resources();

			void Destroy();
			void RemoveFromDevice();
			bool Empty() const;

			// Vector of auto-pointers with a custom allocator.
			template <typename T>
			struct VectorType
			{
				typedef Btl::Vector<Btl::AutoPointer<T, ResourceAllocator<T> >, Btl::VectorParameters_Dynamic<ResourceAllocator<Btl::NullType> > > Result;
			};

			VectorType<VertexBuffer>::Result vertexBuffers;
			VectorType<IndexBuffer>::Result indexBuffers;
			VectorType<Texture>::Result textures;
			Btl::Vector<Btl::AutoPointer<Node> > nodes; // Serialisation Handler uses standard allocator.
			S32 lod;
			Scene* pScene;
			bool discard;
		};

		static void SwapNew(Resources& dest, Resources& source); // Swap new resources from source.
		static void ReduceLod(const Resources* pFront, const Resources& pBack, Resources& dest);
		
		void Destroy(); // Blocking clear of front and back.

	private:
		FRIEND_SERIALISE(LodReader<ResourceBlock::Resources>) // Loads into back.
		friend class StreamData;

		Resources m_back;
		Resources m_resources;

		// debug
		struct StreamDebug
		{
			S32 chunk;
			S32 lod;
			bool load;
		};

		Btl::Vector<StreamDebug> m_streamDebug;
	};

	void Swap(ResourceBlock::Resources& a, ResourceBlock::Resources& b);

	struct TextureInfo
	{
		enum E_Flag
		{
			E_Flag_Shared = 1 << 0,
		};

		const Texture*	pTexture;
		U32				flags;
	};

	typedef Btl::Vector<TextureInfo> TextureInfoList;

	class SharedTextureList
	{
	public:
		SharedTextureList() {}
		~SharedTextureList();

		const Texture* GetTexture(S32 i);

		void AccumulateTextureInfo(TextureInfoList& ioTextureInfoList) const;

		FRIEND_SERIALISE(SharedTextureList)

	private:
		Btl::Vector<const Texture*> m_textures;
	};

	struct SharedTextureData
	{
		friend class SharedTextureManager;
		SharedTextureData() : referenceCount(0), potentialShareCount(0) {}
		Texture *GetTexture() { return &texture; }
	private:
		Texture texture;
		S32 referenceCount;
		S32 potentialShareCount;
	};

	class SharedTextureManager
	{
	public:
		static SharedTextureManager& Instance();

		const Texture* InsertTexture(const char* name, U32 hashValue);
		void ReleaseTexture(const Texture* p);

		SharedTextureData* FindTexture(const char* name, U32 hashValue);
		void IncreaseRefCount(SharedTextureData *pTexture);
		void DecreaseRefCount(SharedTextureData *pTexture);

		void CheckPotentialTextureSharing(const char* name, U32 hashValue);
		S32 GetTotalPotentialSharing() const;
		void DumpPotentialTextureSharing() const;
		
		S32 GetCurrentSharing(const Texture* p);

	private:
		SharedTextureManager() {}
		static SharedTextureManager s_instance;

		typedef Btl::List<SharedTextureData> TextureList;
		TextureList m_textureList;
	};

	struct TexturePairs;

	class ChunkAllocator
	{
	public:
		static U8* Allocate(S32 n);
		static void Free(U8* p);

		typedef U8* PointerType;

		template <typename U> 
		struct Rebind 
		{
			typedef ChunkAllocator Other;
		};

		typedef Btl::Map<U8*, U32, Btl::DynamicPool<> > MemoryMap;

	private:
		static MemoryMap s_usedMap;
	};

	// Handles the streaming for a scene.
	class StreamData : public RenderingData::StreamData
	{
	public:
		StreamData();
		~StreamData();

		void OpenStreamFile(const Btl::String& filename);
		void CloseStreamFile();

		void ClearRequests(); // Clear all cache and load requests.
		void RequestAll(); // Request to load all chunks.
		
		void Request(const RequestInfo* pRequest, S32 nRequests);
		void RequestLowest(S32 bytesToRequest, S32 groupIndex = -1); // Request lowest lod data.

		bool UpdateMain(); // Returns true unless loading failed.
		void LoadAllRequested(); // Update until all requested chunks are loaded.
		void FlushTasks(); // Flush requests to make sure they are ready for the update.

		void SetJobQueuePolicy(Scheduler::PolicyHandle policyHandle);

		void DisplayDebug();
		void DebugInfo() const;
		void DebugSizeInfo() const;

		bool IsLoaded(S32 chunk) const;
		bool IsCached(S32 chunk) const;

		U64 GetTotalUsed() const;
		S32 LoadedChunk() const; // Chunk that was loaded in the last update, or -1.
		S32 UnloadedChunk() const; // Chunk that was unloaded in the last update, or -1.
		bool MultipleChunksUpdated() const; // If LoadAllRequested() was called.

		bool Busy() const; // Returns true if anything is currently being loaded.
		
		bool NodesChanged() const; // Whether any nodes changed in the last update.
		bool BlocksChanged() const; // Whether any blocks changed in the last update.
		bool CacheChanged() const; // Whether any chunks changed in the last update.

		// Get a bitfield indicating which chunks are visible.
		Btl::BitVector& GetPVSInfo(S32 zone);

		void SetScenePtr(Scene* pScene);

		friend class Scene;
		friend class StreamerRequestWorker;

		FRIEND_SERIALISE(StreamData);

		struct DebugInfoStats
		{
			U64 compressedBytesInFile;
			U64 uncompressedBytesInFile;
			U64 bytesLoaded;
			U64 bytesRequested;
			U64 bytesLoadedAndRequested;
			U64 bytesRequestedPvs;
			U64 bytesCached;
			S32 nChunksInFile;
			S32 nChunksLoaded;
			S32 nChunksRequested;
			S32 nChunksCached;
		};

		void DebugInfo(DebugInfoStats& stats) const;
		void AddDebugBounds(const Bounds& bounds) const;

	private:
		StreamData(const StreamData& streamData);
		StreamData& operator = (const StreamData& streamData);

		void SetCacheChunk(S32 chunk, bool cache); // Load a chunk from the file into memory.
		void SetLoadChunk(S32 chunk, bool load); // Load the blocks in a chunk from the cache.
		void SetLoadChunk(S32 chunk);

		bool CacheChunk(S32 chunk);
		void LoadChunk(S32 chunk);
		bool UnloadChunk(S32 chunk);

		void UpdateChunkActivity();
		void ProcessChunks();
		void ProcessLoadChunk(S32 chunk);
		void ProcessUnloadChunk(S32 chunk);
		void SwapChunks();
		void DeleteChunks();
		void DeleteBlocks();
		
		void SwapChunk(S32 chunk);

		void AddTextures(ResourceBlock::Resources& resources, int chunk);
		void RemoveTextures(const ResourceBlock::Resources& resources);
		void Discard(S32 chunk);
		void AllocateStepDownBuffer();

		void BeginTasks();

		void SetPvsCellSizeDebug();
		bool CellRequiresChunk(S32 cell, S32 chunk) const;

		void FixupPairedTextures(const TexturePairs& pairs);

		Scene* m_pScene;
		Sys::I_File* m_pFile;
		typedef Btl::AutoPointer<U8, ChunkAllocator> AutoChunkPointer;
		Btl::Vector<AutoChunkPointer> m_cachedChunks;
		typedef Btl::List<S32, Btl::ListParameters_Pool<> > ChunkList;
		ChunkList m_cachedChunksLRU;
		
		enum ChunkActivity
		{
			STATE_IDLE,
			STATE_ASYNCH_CACHE,
			STATE_ASYNCH_LOAD,
			STATE_ASYNCH_UNLOAD,
			STATE_PENDING_SWAP_IN,
			STATE_PENDING_SWAP_OUT
		};

		struct ChunkState
		{
			ChunkActivity activity : 4;
			bool shouldCache : 1;
			bool shouldLoad : 1;
			bool cached : 1;
			bool loaded : 1;
			bool disable : 1;
		};

		struct ChunkDebug
		{
			S32 loadCount;
			float totalMBytes;
			S32 framesSinceLoad;
			S32 framesSinceUnload;
		};

		S32 m_failedCount;
		S32 m_lastUnloadFailedCount;
		bool m_cacheFailed;
		bool m_updateBlocked;
		S32 m_requestCount;
		Btl::Vector<ChunkState> m_chunkState;
		Btl::Vector<ChunkDebug> m_chunkDebug;
		Btl::Vector<bool> m_requestResults;
		typedef Btl::Vector<ResourceBlock::Resources> ResourceVector;
		Btl::Vector<ResourceVector, Btl::VectorParameters_Array<2> > m_garbage;
		Time::C_Time m_updateStartTime;
		
		S32 m_chunkToCache;
		S32 m_nextChunkToLoad;
		S32 m_chunkToUnload;
		S32 m_chunkToSwapIn;
		S32 m_chunkToSwapOut;
		S32 m_loadedChunk;
		S32 m_unloadedChunk;
		S32 m_nUpdatedChunks;

		bool m_nodesChanged;
		S32 m_blocksChanged;
		bool m_chunksChanged;
		bool m_multipleChunksUpdated;
		S32 m_debugPVSCell;
		U32 m_lastChunkProcessType;
		bool m_loadChunks;
		bool m_unloadChunks;

		StreamerRequestWorker m_streamRequestWorker;

		Btl::List<U8*> m_stepDownBuffers;

		// debug
		Maths::HVec3 m_cameraPosition;
		Maths::HVec3 m_cameraDirection;
		typedef Btl::ChainedHashMap<const void*, S32> TextureChunkMap;
		TextureChunkMap m_textureChunkMap;
		Btl::Vector<U64> m_cellSizes;
		mutable Btl::Vector<Bounds> m_debugBounds;
		mutable MT::Lock m_debugLock;
	};

	// Streaming of multiple scenes.
	class StreamManager
	{
	public:
		static StreamManager& Instance();

		void Create();
		typedef Btl::AutoPointer<U8, ChunkAllocator> AutoChunkPointer;

		bool IsLoadingChunk() const;

		bool LoadChunk(S32 chunkIndex, Sys::I_File* pFile, U64 offset, U64 size);
		AutoChunkPointer GetChunkData(S32& index, bool& loadFailed);
		void ClearRequests();
		S32 GetNumRequests() const;

		void Update();
		void WaitForChunk();

		void FlushRequests();

		enum AsynchCopyType
		{
			ASYNCH_COPY_SYSTEM_TO_GPU,
			ASYNCH_COPY_GPU_TO_GPU
		};

		void BeginCopyList();
		void AsynchCopy(void* pDst, const void* pSrc, S32 size, AsynchCopyType type);
		void SetPreLoad(bool preLoad);

		void SetJobQueuePolicy(Scheduler::PolicyHandle policyHandle);
		Scheduler::PolicyHandle GetPolicy() const;
		Scheduler::GroupHandle GetDecompressionGroup() const;
		Scheduler::GroupHandle GetRequestGroup() const;

		void BeginTasks();
		void FlushTasks();
		bool TasksRunning() const;
		bool PreLoad() const {return m_preLoad;}
		void BeginGPUCopyTasks();

		// Set the maximum amount of GPU memory that can be used by the streamer.
		void SetMemoryLimit(S32 bytes);
		S32 GetMemoryLimit() const;
		S32 MaxChunkSize() const;

		void Destroy();
		
	private:
		StreamManager();
		~StreamManager();

		static StreamManager s_instance;

		bool m_preLoad;
		bool m_loadingChunk;
		bool m_Destroyed;
		//S32 m_chunkLoaded;
		//AutoChunkPointer m_pChunkData;
		MemCopyWorker m_memCopyWorker;
		S32 m_memoryLimit;

		Scheduler::PolicyHandle m_JobQueuePolicy;
		Scheduler::GroupHandle m_JobQueueGroupDecompression;
		Scheduler::GroupHandle m_JobQueueGroupGPUCopy;
		Scheduler::GroupHandle m_JobQueueGroupRequest;
	};

	// Scene containing platform-specific resources.
	class Scene : public RenderingData::Scene
	{
		friend class Context;
	public:
		Scene();
		~Scene();

		// debug
		void SetDebugName(const char* pName);
		const char* GetDebugName() const;

		// streaming
		void UpdateResourcePointers();

		// hierarchy
		const RootNode& GetRootNode() const;
		RootNode& GetRootNode();
		const Node* GetNodePtr(const ResourceIndex& i) const;
		Node* GetNodePtr(const ResourceIndex& i);

		// resources
		S32 GetNumResourceBlocks() const;
		const ResourceBlock& GetResourceBlock(S32 i) const;
		ResourceBlock& GetResourceBlock(S32 i);

		// vertex declarations
		S32 GetNumVertexDeclarations() const;
		const VertexDeclaration& GetVertexDeclaration(S32 i) const;

		// effects
		S32 GetNumEffects() const;
		const Effect& GetEffect(S32 i) const;
		Effect& GetEffect(S32 i);
		S32 FindEffect(const Btl::String& name) const;

		// streaming
		StreamData& GetStreamData();
		const StreamData& GetStreamData() const;

		void SetMaxLodToLoad(S32 i);
		S32 GetMaxLodToLoad() const;

		S32 GetNumDynamicMaterialLists() const;
		const MaterialList* GetDynamicMaterialList(S32 i) const;

		void ProcessNodes(const Node* pNode, void (*f)(const Node& node)) const;
		void ProcessNodes(Node* pNode, void (*f)(Node& node));
		void ProcessNodes(const Node* pNode, void (Scene::*f)(const Node& node) const) const;
		void ProcessNodes(Node* pNode, void (Scene::*f)(Node& node));

		void AccumulateTextureInfo(TextureInfoList& ioTextureInfoList) const;

		const FX::SharedBindingBits *GetSharedBindingUsage() const;

		void OnLoad(EffectVariationCallbackFunc pCallback = NULL);
		FRIEND_SERIALISE(Scene);

	private:
		Scene(const Scene& scene);
		Scene& operator = (const Scene& scene);

		void UpdateResourcePointers(Node& node);
		void Initialise(RenderListNode& node);
		void CalculateGlobalBindings();

		// scene level data
		Btl::Vector<ResourceBlock> m_resourceBlocks;
		Btl::Vector<VertexDeclaration> m_vertexDeclarations;
		Btl::Vector<Effect> m_effects;
		Btl::Vector<Btl::AutoPointer<MaterialList> > m_dynamicMaterialLists;
		SharedTextureList m_sharedTextures;
		StreamData m_streamData;

		S32 m_maxLodToLoad;
		FX::SharedBindingBits m_SharedFXUsage, m_SharedFXMask;

		U32 m_iFXReloadIndex;
		const char* m_pDebugName;

		// No models or materials since they are stored in render lists.
		// All models are converted to render lists before reaching the game.
	};

#ifndef CFG_CONVERTER
#include "noninterface/streaming.inl"
#endif

}}

SERIALISATION_VERSION(RenderingData::Target::Scene, 1, "RenderingData::Target::Scene")

#endif
