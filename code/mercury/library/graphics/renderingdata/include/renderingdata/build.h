#ifndef RD_BUILD_H
#define RD_BUILD_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/map.h"
#include "btl/containers/set.h"
#include "node.h"
#include "renderlist.h"
#include "scene.h"
#include "renderstates.h"
#include "streamdata.h"
#include "model.h"

namespace TextureExport {class TextureManager;}
namespace ActTools {class Texture;}

namespace RenderingData {
namespace Build
{
	class ResourceBlock;
	class VertexFinder;
	class VertexBufferList;
	class LodMesh;

	// Write a class for a particular platform.
	template<typename T>
	class Writer
	{
	public:
		Writer(const T& t, PlatformType platformType) : m_t(t), m_platformType(platformType) {}

		const T& m_t;
		PlatformType m_platformType;
	};

	// Write a class for a particular platform and lod.
	template<typename T>
	class LodWriter
	{
	public:
		LodWriter(const T& t, PlatformType platformType, S32 lod = -1) : m_t(t), m_lod(lod), m_platformType(platformType) {}

		const T& m_t;
		PlatformType m_platformType;
		S32 m_lod;
	};

	///////////////////////////////////////////////////////

	// Texture to be exported.
	class Texture
	{
	public:
		Texture();

		// Name for debug (e.g. filename).
		// Index of the texture in the texture cache.
		// Hash value of texture export info, for texture sharing and reloading.
		void Initialise(const Btl::String& name, S32 cacheIndex, U32 hashValue, TextureExport::TextureManager* pTextureManager);

		const Btl::String& GetName() const;
		S32 GetCacheIndex() const;
		U32 GetHashValue() const;

		const SamplerState& GetSamplerState() const;
		SamplerState& GetSamplerState();

		// For streaming.
		// Scale = size in world space of one unit in texture space.
		// Result is the weighted average of all calls to the update function.
		// Weight = e.g. area of texture used by a triangle.
		void UpdateScale(float uScale, float vScale, float weight) const;
		float GetScaleU() const;
		float GetScaleV() const;

		TextureExport::TextureManager* GetTextureManager() const;

		FRIEND_SERIALISE(LodWriter<Texture>);
		FRIEND_SERIALISE(Writer<Texture>);

	private:
		void CalculateTextureScale() const;

		Btl::String m_name;
		SamplerState m_samplerState;
		S32 m_cacheIndex;
		S32 m_hashValue;
		TextureExport::TextureManager* m_pTextureManager;

		struct TextureScale
		{
			float scale;
			float weight;
		};

		friend bool operator < (const Texture::TextureScale& a, const Texture::TextureScale& b);

		mutable Btl::Vector<TextureScale> m_textureScaleU;
		mutable Btl::Vector<TextureScale> m_textureScaleV;
		mutable float m_uScale;
		mutable float m_vScale;
		mutable bool m_textureScaleCalculated;
	};

	bool operator == (const Texture& p1, const Texture& p2);
	bool operator != (const Texture& p1, const Texture& p2);
	bool operator < (const Texture& p1, const Texture& p2);

	///////////////////////////////////////////////////////

	class VertexBuffer
	{
	public:
		VertexBuffer();

		const VertexData& GetVertices() const;
		VertexData& GetVertices();

		S32 GetBatchID() const;
		S32 GetInstanceID() const;
		S32 GetStreamIndex() const;

		friend class Build::Scene;
		friend class ResourceBlock;

		FRIEND_SERIALISE(VertexBuffer)

	private:
		VertexData m_vertices;

		// Initially, each primitive uses a separate buffer.
		S32 m_batchID;
		S32 m_instanceID;
		S32 m_streamIndex;
		PlatformType m_platformType;
	};

	//////////////////////////////////////////////////////////

	class IndexBuffer
	{
	public:
		IndexBuffer() {}

		// Built with 32 bit indices, but saved as 16 bit if all indices are small enough.

		typedef Btl::Vector<U32> IndexData;

		const IndexData& GetIndices() const;
		IndexData& GetIndices();

		// Primitive type used for processing on export.
		// quad list -> triangle list (ps3, pc)
		void SetPrimitiveType(PrimitiveType primitiveType);
		PrimitiveType GetPrimitiveType() const;

		void GetMinIndexAndVertexCount(S32 offset, S32 nIndices, U32& minIndex, U32& vertexCount) const;

		// Lods
		// lod 0 = most detailed.
		// To begin with there is a single lod.
		// To create a new lod, call CreateLod() with the number of indices required.
		// The new indices are added to the buffer and initialised to the reset index.
		S32 CreateLod(S32 nIndices); // Returns the index of the lod that was created.
		S32 GetNumLods() const;
		S32 GetLodOffset(S32 lod) const;
		S32 GetLodSize(S32 lod) const;

		friend class VertexBufferList;
		friend class VertexFinder;
		friend class ResourceBlock;
		friend class Scene;

		static U32 ResetIndex32();
		static U16 ResetIndex16();

		FRIEND_SERIALISE(IndexBuffer)

	private:
		void InsertVertexBuffer(ResourceIndex vertexBufferIndex);
		void InsertVertexFormat(const VertexFormat& vertexFormat, S32 stream);

		Btl::Vector<U32> m_indices;
		Btl::Vector<ResourceIndex> m_vertexBuffers;
		Btl::Vector<U32> m_lodSizes;

		struct VertexFormatData
		{
			VertexFormat format;
			S32 streamIndex;
		};

		Btl::Vector<VertexFormatData> m_vertexFormatData;

		PrimitiveType m_primitiveType;
		PlatformType m_platformType;
	};

	/////////////////////////////////////////////////////////////

	// Write all lods of a resource block.
	typedef Writer<ResourceBlock> ResourceBlockWriter;

	// Write a single lod of a resource block.
	typedef LodWriter<ResourceBlock> ResourceBlockLodWriter;

	template <typename Archive> void Serialise(Archive& ar, ResourceBlockWriter& t);
	template <typename Archive> void Serialise(Archive& ar, ResourceBlockLodWriter& t);

	// Unit of streaming.
	class ResourceBlock
	{
	public:
		ResourceBlock();
		~ResourceBlock();
	
		S32							GetNumTextures() const;
		const Texture&				GetTexture(S32 i) const;

		S32							GetNumVertexBuffers() const;
		const VertexBuffer&			GetVertexBuffer(S32 i) const;

		S32							GetNumIndexBuffers() const;
		const IndexBuffer&			GetIndexBuffer(S32 i) const;

		S32							GetNumNodes() const;
		const Node*					GetNodePtr(S32 i) const;
		Node*						GetNodePtr(S32 i);
		S32							AddNode(Btl::AutoPointer<Node> p);
		void						ClearNode(S32 i);

		S32 GetSize(S32 lod, PlatformType platformType) const;
		bool Empty() const;

		// No facility to delete items since that would change the indices.
		// Clean-up of the scene could remove unused items.

		const Bounds& GetFullBounds() const;
		int GetNumBounds() const;
		const Bounds& GetSubBounds(int i) const;
		const Btl::BitVector& GetPvsBits() const;
		const Btl::Set<S32>& GetGroups() const;
		const Btl::BitVector& GetPvsBits(S32 lod) const;
		const Btl::BitVector& GetRoomBits() const;
		U32 GetGroupBits() const;

		U32 GetRenderNodeHash() const;

		void CreateLodPvs();
		void UpdateLodPvs(S32 cell, float distance);

		void SetPlatformType(PlatformType platformType);

		friend class Scene;
		friend void Move(void* pDst, ResourceBlock* pSrc);
		FRIEND_SERIALISE(ResourceBlockLodWriter);

	private:
		ResourceBlock(const ResourceBlock& t);
		ResourceBlock& operator = (const ResourceBlock& t);

		void Clear();
		void ClearTextures();
		void Copy(const ResourceBlock& t);

		void SetScenePtr(Scene* pScene);
		Scene* GetScenePtr() const;
		VertexBuffer& GetVertexBuffer(S32 i);
		S32 InsertVertexBuffer(const VertexFormat& vertexFormat, S32 batchID, S32 instanceID, S32 streamIndex);
		IndexBuffer& GetIndexBuffer(S32 i);
		S32 AddIndexBuffer(PrimitiveType primitiveType);
		S32 InsertTexture(const Texture& texture);
		void RemoveEmptyNodes();

		// Block member variables.
		// Note: anything added here must be included in Move()

		Btl::Vector<Btl::AutoPointer<VertexBuffer> > m_vertexBuffers;
		Btl::Vector<Btl::AutoPointer<IndexBuffer> > m_indexBuffers;
		Btl::Vector<Texture> m_textures;
		Btl::Vector<Btl::AutoPointer<Node> > m_nodes;

		typedef Btl::Map<Texture, S32> TextureMap;
		TextureMap m_textureMap;

		Bounds m_fullBounds; // once calculated, bounds of everything contained in the block.
		Btl::Vector<Bounds> m_subBounds; // list of the bounds of everything using the block.
		Btl::BitVector m_pvsBits; // 1 bit per PVS zone
		Btl::Vector<Btl::BitVector> m_lodPvs; // 1 bit per PVS zone per lod
		Btl::Set<S32> m_groups; // Groups that use the block.
		U32 m_renderNodeHash; // Hash of all the render nodes that use the block.
		Btl::BitVector m_roomBits; // 1 bit per room.

		Scene* m_pScene;
		S32 m_blockIndex;
	};

	// for vector
	void Move(void* pDst, ResourceBlock* pSrc);
	void MoveArray(void* pDst, ResourceBlock* pSrc, int n);

	//////////////////////////////////////////////////////////////

	// Similar to a resource block but only contains textures, 
	// does not support lods and the scene only contains one list of shared textures.
	class TextureList
	{
	public:
		S32 GetNumTextures() const;
		const Texture& GetTexture(S32 i) const;

		friend class Scene;

		FRIEND_SERIALISE(Writer<TextureList>);

	private:
		S32 InsertTexture(const Texture& texture);
		void Clear();

		typedef Btl::Map<Texture, S32> TextureMap;
		TextureMap m_textureMap;

		Btl::Vector<Texture> m_textures;
	};

	//////////////////////////////////////////////////////////////

	// Extend material list to add and share materials.
	class MaterialList : public RenderingData::MaterialList
	{
	public:
		S32 InsertMaterialType(const MaterialType& t);
		S32 InsertMaterial(const Material& m);

		friend class Scene;

	private:
		void Clear();

		// Using map for searching as many materials are expected.
		typedef Btl::Map<Material, S32> MaterialMap;
		MaterialMap m_materialMap;
	};

	//////////////////////////////////////////////////////////////

	// Purely to match serialisation types with target.
	class Effect : public RenderingData::Effect
	{
	};

	class RenderListNode : public RenderingData::RenderListNode
	{
	public:
		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual RenderListNode* Clone() const;

		// If p is a RenderListNode, return a pointer to it, otherwise return a null pointer.
		static const RenderListNode* GetPointer(const Node* p);
		static RenderListNode* GetPointer(Node* p);

		VIRTUAL_SERIALISATION_KEY_DECL

		const Material& GetMaterial(const MaterialIndex& material) const;
		const MaterialType& GetMaterialType(const MaterialIndex& material) const;
	};

	//////////////////////////////////////////////////////////////

	class StreamData : public RenderingData::StreamData
	{
	public:
		StreamData();

		// Assign a lod of a block to a chunk.
		void AddBlockToChunk(S32 chunkIndex, S32 blockIndex, S32 lod);

		// Write all the stream data to a file (must be done before serialising the stream data).
		void WriteChunkData(Sys::I_File* pFile, PlatformType platformType);

		// Get the original (uncompressed) size of the GPU data in the block.
		S32 GetChunkSize(S32 chunkIndex, PlatformType platformType) const;

		// Get the original (uncompressed) size of the GPU data in the block.
		S32 GetItemSize(S32 chunkIndex, S32 itemIndex, PlatformType platformType) const;

		friend class Scene;

	private:
		void Finish();
		void WriteChunkData(Sys::I_File* pFile, S32 chunkIndex);
		void AlignPosition(Sys::I_File* pFile, U64 align);
		void GetRequiredBlocks(Node* pNode, Btl::Set<S32>& blocks);
		void AddRequirement(S32 chunkIndex, S32 requiredChunk);
		void CalculateDependencies();
		void CalculateTexturePairing(S32 chunkIndex);
		void CalculateTexturePairing(const ResourceBlock& block, S32 lod);
		void CalculateTexturePairing(const Texture& texture, S32 lod);
		
		Scene* m_pScene;
		PlatformType m_platformType;
		bool m_chunkDataReady;
		bool m_streamDataReady;
		Btl::Vector<S32> m_orderedChunks;
	};

	/////////////////////////////////////////////////////////////////

	struct SceneSizeInfo
	{
		S32 nRenderLists;
		S32 nPrimitives;
		S32 nBaseInstancePrimitives;
		S32 nInstancePrimitives;

		S32 nMaterials;
		S32 nEffectMaterials;
		S32 nSharedMaterials;
		S32 nMaterialTypes;

		S32 nBlocks;
		S32 nVertexBuffers;
		S32 nIndexBuffers;
		S32 nVertices;
		S32 nIndices;
		S32 nStreamedTextures;
		S32 nSharedTextures;

		S32 vertexDataSize;
		S32 indexDataSize;
		S32 streamedTextureSize;
		S32 sharedTextureSize;
		S32 totalStreamedSize;
		S32 totalTextureSize;
		S32 materialDataSize;
		S32 totalSize;
	};

	//////////////////////////////////////////////////////////////

	// Write the scene for a particular platform.
	typedef Writer<Scene> SceneWriter;

	template <typename Archive> void Serialise(Archive& ar, SceneWriter& t);

	// Main scene class containing everything.
	class Scene : public RenderingData::Scene
	{
	public:
		Scene();
		~Scene() {}

		Scene(const Scene& scene);
		Scene& operator = (const Scene& scene);

		// Set the name of the scene.
		// (GetName() in base class)
		void						SetName(const Btl::String& s);

		// The root node is fixed and other nodes must be added as sub-nodes.
		const RootNode&				GetRootNode() const;
		RootNode&					GetRootNode();

		// Convert a resource index to a node pointer.
		const Node*					GetNodePtr(const ResourceIndex& i) const;
		Node*						GetNodePtr(const ResourceIndex& i);

		// Manage resource blocks.
		S32							GetNumResourceBlocks() const;
		const ResourceBlock&		GetResourceBlock(S32 i) const;
		ResourceBlock&				GetResourceBlock(S32 i);
		ResourceBlock&				InsertResourceBlock(S32 i);
		S32							AddResourceBlock();

		// Get resources.
		const VertexBuffer&			GetVertexBuffer(const ResourceIndex& i) const;
		const IndexBuffer&			GetIndexBuffer(const ResourceIndex& i) const; 
		const Texture&				GetTexture(const ResourceIndex& i) const;

		// Base models (for instances).
		S32							GetNumBaseModels() const;
		const Model&				GetBaseModel(S32 i) const;
		Model&						GetBaseModel(S32 i);
		S32							AddBaseModel();

		// Effects.
		S32							GetNumEffects() const;
		const Effect&				GetEffect(S32 i) const;
		S32							FindEffect(const Btl::String& name) const;
		S32							AddEffect(const Effect& effect);

		// Insert a texture into a block.
		// (allows the texture to be streamed)
		S32							InsertStreamedTexture(const Texture& texture, S32 blockIndex);

		// Insert a shared texture.
		// (allows sharing with other scenes)
		S32							InsertSharedTexture(const Texture& texture);
		
		const ResourceIndex&		GetTexture(S32 textureIndex) const;

		// Materials.
		const MaterialList&			GetMaterialList() const;
		MaterialList&				GetMaterialList();
		void						CreateGlobalDynamicMaterialList(S32 i);
		const MaterialList*			GetDynamicMaterialList(S32 i) const;
		MaterialList*				GetDynamicMaterialList(S32 i);

		// Vertex declarations.
		S32							GetNumVertexDeclarations() const;
		const VertexDeclaration&	GetVertexDeclaration(S32 i) const;

		// Streaming
		const StreamData&			GetStreamData() const;
		StreamData&					GetStreamData();

		ResourceIndex				AddRenderNode(S32 blockIndex);
		ResourceIndex				AddCullNode(S32 blockIndex);

		// Fill the vector with the texture indices used on the primitive.
		void						GetTextureIndices(const PrimitiveInfo& primitive, Btl::Vector<S32>& textures) const;

		// Add a primitive to a model.
		void						AddPrimitive(PrimitiveInfo& primitive, Model* pModel);

		// Primitives added to an instance must be in exactly the same sequence
		// as the primitives in the base model.
		void						AddPrimitive(PrimitiveInfo& primitive, Instance* pInstance);

		void						SetLodGenerationOptions(S32 lodsToGenerate, float reductionFactor = 0.5f, S32 minVertices = 8, bool reduceVertices = false);

		// Called after everything is added.
		// Will apply optimisations for the platform type specified, or generic processing if it is unknown. 
		void						Finish(PlatformType platformType);

		// Details of the data stored in the scene. 
		void						CalculateSizeInfo() const;
		const SceneSizeInfo&		GetSizeInfo() const;

		friend class VertexFinder;
		FRIEND_SERIALISE(SceneWriter);
		friend class StreamData;

	private:
		S32 InsertVertexDeclaration(const VertexDeclaration& vertexDeclaration);
		VertexBuffer& GetVertexBuffer(const ResourceIndex& i);
		IndexBuffer& GetIndexBuffer(const ResourceIndex& i); 
		void AddPrimitive(PrimitiveInfo& primitive, Model* pModel, Instance* pInstance);
		void VerifyPrimitive(const PrimitiveInfo& primitive) const;
		void BuildRenderLists(Node* pNode, bool separatePasses = false);
		bool AddNodeToRenderList(const Node* pNode, RenderListBuilder& renderListBuild);
		void AddModelToRenderList(const Model& model, RenderListBuilder& renderListBuild);
		void AddInstanceToRenderList(const Instance& instance, RenderListBuilder& renderListBuild);
		void AddInstanceBatchToRenderList(const InstanceBatch& instanceBatch, RenderListBuilder& renderListBuild);
		void BuildChunkData();
		void BuildChunkData(Node& node);
		void Clean();
		void CleanNode(Node& node);
		void Clear();
		void ShareVertices();
		void CombineIndexBuffers();
		void CombineVertexBuffers();
		void BuildStrips();
		void Copy(const Scene& scene);
		void CalculateBoundsAndPvs();
		void CalculateBoundsAndPvs(Node& node, Bounds& bounds, Btl::BitVector& pvsBits, S32 groupIndex, Btl::BitVector& roomBits);
		void PrintHierarchy(const Node& node, S32 level) const;
		void ChangeVertexElementType(VertexElementType originalType, VertexElementType finalType);
		void ReverseU8_4();
		void UpdateResourceBlockBounds(S32 blockIndex, const Bounds& bounds, U32 renderNodeHash, const Btl::BitVector& pvsBits, S32 groupIndex, const Btl::BitVector& roomBits);
		void PropagateGroupIndex(Node& node, S32 groupIndex);

		typedef void (Scene::*ModelFunctionPtr)(Model& p);
		typedef void (Scene::*InstanceFunctionPtr)(Instance& p);
		typedef void (Scene::*PrimitiveStreamFunctionPtr)(PrimitiveStream& p);
		typedef void (Scene::*PrimitiveFunctionPtr)(Primitive& p);
		typedef void (Scene::*NodeFunctionPtr)(Node& p);
		typedef void (Scene::*ConstNodeFunctionPtr)(const Node& p) const;

		void ProcessModels(ModelFunctionPtr f);
		void ProcessModelNodes(Node* pNode, ModelFunctionPtr f);
		void ProcessPrimitives(PrimitiveFunctionPtr f);
		void ProcessPrimitiveStreams(PrimitiveStreamFunctionPtr f);
		void ProcessInstances(InstanceFunctionPtr f);
		void ProcessInstanceNodes(Node* pNode, InstanceFunctionPtr f);
		void ProcessNodes(Node* pNode, NodeFunctionPtr f);
		void ProcessNodes(const Node* pNode, ConstNodeFunctionPtr f) const;

        bool AddLodMeshTriangleVertexData( LodMesh* pLodMesh, const S32 triangleIndex, const VertexData& vertexData );
        S32  GenerateVertexBufferIndices( LodMesh* pLodMesh, Btl::Vector<U32>& indices, IndexBuffer const * pPrimitive );
		void CreateLodIndexBuffers( LodMesh* pLodMesh, int lod );
		void BuildLods();
		
		void BuildVertexDeclarations(Model& model);
		void RemapIndexBuffer(Primitive& primitive);
		void CreatePrimitiveLods(Primitive& stream);
		void RemapVertexBuffer(PrimitiveStream& stream);
		void RemapNodes(Node& node);
		void SetPrimitiveType(Primitive& primitive);
		void ProcessPrimitiveStreams(Model& model);
		void ProcessPrimitiveStreams(Instance& instance);
		void ProcessPrimitives(Model& model);
		void CountPrimitives(const Node& node) const;
		void RemapResourceIndex(ResourceIndex& resourceIndex);
		void RemapTextures();
		void RemapTextures_GetTextureUsage(Node& node);
		void RemapTextures_GetTextureUsage(RenderListNode& node, MaterialIndex materialIndex, void* id);
		void RemapTextures_CreateBlocks();
		void RemapTextures_MoveTextures(Node& node);
		void GetTextureBounds(Node& node);
		void GetTextureBounds(RenderListNode& node, MaterialIndex materialIndex, const Bounds& bounds, const Btl::BitVector& pvsBits, S32 groupIndex, const Btl::BitVector& roomBits);

		// scene level data
		Btl::Vector<ResourceBlock> m_resourceBlocks;
		Btl::Vector<VertexDeclaration> m_vertexDeclarations;
		Btl::Vector<Effect> m_effects;
		TextureList m_sharedTextures;
		StreamData m_streamData;

		// moved to render lists before writing
		Btl::Vector<Model> m_baseModels; // for instances
		MaterialList m_materialList;
		typedef Btl::Map<ResourceIndex, S32> TextureMap;
		TextureMap m_textureMap;
		Btl::Vector<ResourceIndex> m_textures;

		struct ExportData
		{
			mutable SceneSizeInfo sizeInfo;
			bool exportComplete;
			PlatformType platformType;

			// lods
			S32 lodsToGenerate;
			S32 minimumVertices;
			F32 vertexReductionRatio;
			bool reduceVertices;
		};

		ExportData m_exportData;

		// temporary
		PrimitiveFunctionPtr m_pPrimitiveFunction;
		PrimitiveStreamFunctionPtr m_pPrimitiveStreamFunction;

		struct BufferRemapData
		{
			BufferRemapData() {}
			BufferRemapData(const ResourceIndex r) : newIndex(r), offset(0) {}

			ResourceIndex newIndex;
			S32 offset;
		};

		typedef Btl::Map<ResourceIndex, BufferRemapData> BufferRemap;
		BufferRemap m_bufferRemap;

		typedef Btl::Map<ResourceIndex, Btl::Set<void*> > TextureUsageMap;
		TextureUsageMap m_textureUsageMap;
		
		Btl::Vector<Btl::AutoPointer<MaterialList> > m_dynamicMaterialLists;
	};

	////////////////////////////////////////////////////////////////////

	void ActivateHandlers();

	RenderPass GetRenderPass(SortType sortType, BlendState blendState);

#include "noninterface/build.inl"

}}

SERIALISATION_VERSION(RenderingData::Build::Writer<RenderingData::Build::Scene>, 1, "RenderingData::Build::Scene")

#endif
