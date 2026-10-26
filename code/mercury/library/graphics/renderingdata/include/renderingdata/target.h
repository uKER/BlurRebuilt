#ifndef RD_PLATFORM_TYPES_H
#define RD_PLATFORM_TYPES_H
#ifdef _MSC_VER
#pragma once
#endif

#ifndef RENDERINGDATA_ALLOW_INCLUDE
#error "Illegal include of renderingdata library internals.h"
#endif

#include "node.h"
#include "renderlist.h"
#include "scene.h"
#include "Display.h"
#include "renderstates.h"
#include "Display/Texture/TextureDesc.h"
#include "memcopyworker.h"
#include "renderingdata/EffectVariationCallback.h"
#if PLATFORM_SPU
#include "btl/allocators/stackallocator.h"
#endif
#include <fx/fx.h>
#include "renderingdata/primitivestream.h"

namespace FX
{
class I_Effect;
class Effect;
typedef U64 ImmediateParameter;
}

// Types created when loading scene data (not required for export).
namespace RenderingData {

namespace Target
{
	// Debug and stats
	void BeginFrame();
	void EndFrame();

	class Scene;
	class ResourceBlock;
	class StreamData;

	PlatformType GetPlatformType();

	template <typename T>
	class LodReader
	{
	public:
		LodReader(const T* front, const T* back, T& next, S32 lod) : m_pFront(front), m_pBack(back), m_next(next), m_lod(lod) {}

		const T* m_pFront;
		const T* m_pBack;
		T& m_next;
		S32 m_lod;
		ResourceIndex m_source;
	};

	template <typename T>
	struct ResourceAllocator
	{
		static T* Allocate(S32 n = 1);
		static void Free(T* p, S32 n = 1);

		template <typename U> 
		struct Rebind 
		{
			typedef ResourceAllocator<U> Other;
		};

		typedef T* PointerType;
	};

	class IndexBuffer 
	{
	public:
		IndexBuffer() {}
		~IndexBuffer();

		const Display2::C_IndexBuffer& GetData() const;

		FRIEND_SERIALISE(IndexBuffer);

	private:
		IndexBuffer(const IndexBuffer& texture);
		IndexBuffer& operator = (const IndexBuffer& texture);

		Display2::C_IndexBuffer m_data;
	};

	class VertexBuffer 
	{
	public:
		VertexBuffer() : m_vertexSize(0) {}
		~VertexBuffer();

		const Display2::C_VertexBuffer& GetData() const;
		S32 GetVertexSize() const;

		FRIEND_SERIALISE(VertexBuffer);

	private:
		VertexBuffer(const VertexBuffer& texture);
		VertexBuffer& operator = (const VertexBuffer& texture);

		Display2::C_VertexBuffer m_data;
		S32 m_vertexSize;
	};

	class Texture
	{
	public:
		Texture();
		~Texture();

		const Display::C_Texture2D& GetData() const;
		void Destroy();

		const char* GetName() const;
		U32 GetHashValue() const;
		S32 GetSamplerStateIndex() const;
		//const SamplerState& GetSamplerState() const;

		FRIEND_SERIALISE(LodReader<Texture>);
		FRIEND_SERIALISE(Texture);
		friend class ResourceBlock;
		friend class StreamData;
		bool Create(const char* fileName);
		U32 Discard() const; // returns true if the texture load failed

		Display::C_Texture2D m_data;

	private:
		Texture(const Texture& texture);
		Texture& operator = (const Texture& texture);

		typedef Btl::StringBase<char, Btl::StringBuffer<char, ResourceAllocator<char>, 16> > String;
		String m_name;
		U32 m_hashValue;
		S16 m_samplerStateIndex;
		U16 m_discard;

		struct HeaderData
		{
			S16 width;
			S16 height;
			U32 size;
			S16 chunk;
			S8 levels;
			S8 lod;
		};

		Btl::Vector<HeaderData, Btl::VectorParameters_Dynamic<ResourceAllocator<Btl::NullType> > > m_headers; // Keep all headers so we can step back down.
	};

	class VertexDeclaration 
	{
	public:
		const Display2::C_VertexDeclaration& GetData() const;

		FRIEND_SERIALISE(VertexDeclaration);

	private:
		Display2::C_VertexDeclaration* m_pData;
	};

	///////////////////////////////////////////////////////////////

	class MaterialListData;

	// Need to create materials on the target, but without sharing.
	class MaterialList : public RenderingData::MaterialList
	{
	public:
		S32 AddMaterialType(const MaterialType& t);
		S32 AddMaterial(S32 typeIndex);
		Material& GetMaterial(S32 i);

		void ClearAll();
		void ClearMaterials();

		MaterialList() {}
		MaterialList(const MaterialList& other);
		MaterialList& operator = (const MaterialList& other);

	private:
		const RenderingData::MaterialList::VectorType& GetMaterials() const;

		friend class RenderingData::Target::MaterialListData;
	};

	// Just material data, no types.
	class MaterialListData : public RenderingData::MaterialListData
	{
	public:
		MaterialListData() {}
		Material& GetMaterial(S32 i);

	private:
		MaterialListData(const MaterialListData& other);
		MaterialListData& operator = (const MaterialListData& other);
	};

	struct MaterialParameterData
	{
		FX::ImmediateParameter fxParameterData;
		MaterialElementType type;
		S16 count; 
		U16 offset;
	};

	struct MaterialTypeData
	{
#if !PLATFORM_SPU
		typedef Btl::VectorParameters_FixedSize<Btl::StackArrayAllocator<Btl::NullType> > VectorParameters;
#else
		typedef Btl::VectorParameters_FixedSize<> VectorParameters;
#endif
		Btl::Vector<MaterialParameterData, VectorParameters > parameters;
		S32 dataSize;

		void OffsetLocation(S32 offset);
	};

	class MaterialData
	{
	public:
		void InitialiseMaterialList(const Scene& scene, const RenderingData::MaterialList* p, bool internalStorage = true);
		void UpdateParameters(const Scene& scene, const RenderingData::MaterialList *pList);
		void RefreshFXParameters();

		void SetMaterialList(const RenderingData::MaterialList* p) const;
		const RenderingData::MaterialList* GetMaterialList() const;

		S32 GetNumTypes() const;
		S32 GetNumParameters(S32 type) const;
		const MaterialParameterData& GetParameter(S32 type, S32 element) const;
		S32 GetDataSize(S32 type) const;

		S32 GetSize() const;
		S32 CalculateSize(const RenderingData::MaterialList* pList = 0) const;
		void Prefetch() const;

#if !PLATFORM_SPU
		const void* GetDmaData() const;
		void CopyFrom(const MaterialData& other, bool internalStorage = true);
#else
		void SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator);
#endif
		void OffsetLocation(S32 offset);

	private:
		void SetNumMaterialTypes(S32 n);
		void SetNumParameters(S32 type, S32 n);
		MaterialParameterData& GetParameter(S32 type, S32 element);

#if !PLATFORM_SPU
		typedef Btl::VectorParameters_FixedSize<Btl::StackArrayAllocator<Btl::NullType> > VectorParameters;
#else
		typedef Btl::VectorParameters_FixedSize<> VectorParameters;
#endif

#if !PLATFORM_SPU
		AutoStackAllocator m_autoStackAllocator;
#endif
		struct ALIGN16 Data
		{
			Data() : FXIndex(-1) {}

			Btl::Vector<MaterialTypeData, VectorParameters> types;
			S32 FXIndex; // Index of the FX system when the shaders were last updated.
		};

		Data m_data;

		struct ALIGN16 DmaData
		{
			SrcData data;
			SrcData buffer;
		};

		mutable DmaData m_dmaData;
	};

	class RenderListNode : public RenderingData::RenderListNode
	{
		friend class Context;
	public:
		RenderListNode();

		virtual NodeType GetNodeType() const;
		virtual bool IsNode(NodeType t) const;
		virtual RenderListNode* Clone() const;

		// If p is a RenderListNode, return a pointer to it, otherwise return a null pointer.
		static const RenderListNode* GetPointer(const Node* p);
		static RenderListNode* GetPointer(Node* p);

		S32 GetNumDynamicMaterialLists() const;
		const RenderingData::MaterialList& GetDynamicMaterialList(S32 i) const;
		void InitialiseDynamicMaterialList(S32 i, const MaterialList* pExternalMaterialList);

		Scene* GetScenePtr() const;
		void SetChunk(S32 chunk);

		const RenderingData::MaterialListData& GetConstantMaterialListData() const;
		const RenderingData::MaterialList& GetConstantMaterialTypes() const;

#if !PLATFORM_SPU
		const void* GetDmaData() const;
		void CopyRenderData(const RenderListNode& node);
#else
		void SpuFetch(const void* pData, Btl::StackAllocatorSimple<>* pAllocator, Spu::CircularCache* pCache);
		void SpuFetchRenderPassInfo(const void* pData);
		void SpuFetchPvsInfo(const void* pData);
#endif
		void Reallocate();

		void RefreshFXParameters();

		friend class Scene;

		FRIEND_SERIALISE(RenderListNode);
		VIRTUAL_SERIALISATION_KEY_DECL;

		const VertexBufferData& GetVertexBuffers(const S32 iIndex) const
		{
			return m_renderDataTarget.vertexBuffers[iIndex];
		}
		
		const Display2::C_VertexDeclaration* GetVertexDeclaration(const S32 iIndex) const
		{
			return m_renderDataTarget.vertexDeclarations[iIndex];
		}

	protected:

		struct TextureData
		{
			S32 iSamplerState;
			const void* pGpuData;
		};

		struct ALIGN16 RenderDataTarget
		{
#if !PLATFORM_SPU
			typedef Btl::VectorParameters_FixedSize<Btl::StackArrayAllocator<Btl::NullType> > VectorParameters;
#else
			typedef Btl::VectorParameters_FixedSize<> VectorParameters;
#endif
			// Updated by the streaming system to point directly at the GPU resources.
			Btl::Vector<const Display2::C_IndexBuffer*, VectorParameters> indexBuffers;
			Btl::Vector<RenderingData::Target::VertexBufferData, VectorParameters> vertexBuffers;
			Btl::Vector<TextureData, VectorParameters> textures;
			Btl::Vector<FX::Effect*, VectorParameters> effects;
			Btl::Vector<const Display2::C_VertexDeclaration*, VectorParameters> vertexDeclarations;

			// Tells the renderer which lod/technique to use.
			Btl::Vector<U32, VectorParameters> primitiveLods;
			Btl::Vector<U32, VectorParameters> primitiveTechniques;

			FX::SharedBindingBits m_SharedFXUsage, m_SharedFXMask;

			void OffsetLocation(S32 offset);
		};
		
		struct StreamerDataTarget
		{
			Btl::Vector<S32> chunkIndices;
			Btl::BitVector chunkState;
			U32 textureReloadIndex;
			bool initialised;
		};

#if !PLATFORM_SPU
		AutoStackAllocator m_autoStackAllocatorRenderTarget;
#endif

		RenderDataTarget m_renderDataTarget;
		StreamerDataTarget m_streamerDataTarget;

		Btl::Vector<MaterialData, Btl::VectorParameters_Static<15> > m_materialData;

		struct ALIGN16 DmaData
		{
			DmaData() : initialised(false) {}

			SrcData renderData;
			SrcData renderDataBuffer;
			SrcData renderDataTarget;
			SrcData renderDataTargetBuffer;
			const void* constantMaterialData;
			Btl::Vector<const void*> materialData;
			SrcData renderPassInfo;
			SrcData pvsData;
			bool initialised;
		};

		mutable DmaData m_dmaData;

		// Debug
#if !CFG_SPEED
		struct TextureDebug
		{
			Btl::Vector<S32> chunks;
		};

		// Indices of the chunks containing the resources.
		S32 m_chunkDebug;
		Btl::Vector<S32> indexBufferDebug;
		Btl::Vector<S32> vertexBufferDebug;
		Btl::Vector<TextureDebug> textureDebug;
#endif
	};

	class Effect : public RenderingData::Effect
	{
	public:
		Effect();
		~Effect();

		bool IsLoaded() const;
		FX::ImmediateParameter GetParameter(const Btl::String& name, U32 cMaxSize) const;
		void OnLoad(EffectVariationCallbackFunc pFunc);
		FX::Effect *GetEffect() const { return (FX::Effect*)m_pFX; }

	private:
		FX::I_Effect *m_pFX;
	};

#ifndef CFG_CONVERTER
#include "noninterface/target.inl"
#endif

}}

#endif
