#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/archive.h"
#include "renderingdata/streaming.h"
#include "Display.h"

#include <System/Serialise/I_FileSystem.h>
#include <core/containers/containerserialisation.h>
#include <fx/fx.h>
#include <fx/RenderStates.h>
#include "fx/SamplerStates.h"
#include <system/serialise/C_MemoryStream.h>
#include "btl/containers/openhashmap.h"
#include "misc_shared/archive/serialiser.h"
#include "system\memory\blockallocator.h"

#include "renderingdata/noninterface/archive.inl"
#include <fx/RenderMutex.h>

//#pragma optimize ("", off)

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

namespace Sys {class I_File;}

namespace RenderingData {

template <class Archive>
inline void DeserialiseMaterial(Archive& ar, Material& material, const MaterialType& materialType)
{
	material.m_dataSize = materialType.GetDataSize();
	material.m_alignment = materialType.GetMaxAlign();

	if (material.m_dataSize > 0)
	{
		Allocate(material.m_pData, material.m_dataSize / 4, material.m_alignment);
		U8* pData = (U8*) Pointer(material.m_pData);
		assert(pData); // to do: handle out of memory

		for (int e = 0; e < materialType.GetNumElements(); e++)
		{
			for (int c = 0; c < materialType.GetElement(e).count; c++)
			{
				switch (materialType.GetElement(e).type)
				{
					case MATERIAL_ELEMENT_TYPE_S8:
						ar >> *(S8*) pData; pData += 1;
						break;
					case MATERIAL_ELEMENT_TYPE_U8:
						ar >> *(U8*) pData; pData += 1;
						break;
					case MATERIAL_ELEMENT_TYPE_S16:
						ar >> *(S16*) pData; pData += 2;
						break;
					case MATERIAL_ELEMENT_TYPE_U16:
						ar >> *(U16*) pData; pData += 2;
						break;
					case MATERIAL_ELEMENT_TYPE_S32:
						ar >> *(S32*) pData; pData += 4;
						break;
					case MATERIAL_ELEMENT_TYPE_U32:
						ar >> *(U32*) pData; pData += 4;
						break;
					case MATERIAL_ELEMENT_TYPE_F32:
						ar >> *(F32*) pData; pData += 4;
						break;
					case MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX:
						ar >> *(S32*) pData; pData += 4;
						break;
					case MATERIAL_ELEMENT_TYPE_TEXTURE_PTR:
#if defined(_WIN64)
						// [Blur2PC][x64] The element is pointer-sized in memory. Exported data stores
						// 4 bytes; TEXTURE_PTR is a runtime (dynamic material) type, so the value is not
						// a usable address and file layouts with it would carry 4-byte offsets.
						{
							S32 value;
							ar >> value;
							BIZ_ASSERT(!"TEXTURE_PTR element in exported material data");
							*(void**) pData = (void*)(INTPTR)value; pData += sizeof(void*);
						}
#else
						ar >> *(S32*) pData; pData += 4;
#endif
						break;
					case MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX:
					case MATERIAL_ELEMENT_TYPE_DATA_INDEX:
						ar >> *(MaterialIndex*) pData; pData += 4;
						break;
					default: 
						assert(0 && "Material element type corrupt.");
				}
			}
		}
	}
}

template <typename Archive>
void DeserialiseMaterialData(Archive& ar, MaterialListData& data, MaterialList& types, bool internalStorage)
{
	S32 numMaterials;
	ar & Arch::MakeNvp("NumMaterials", numMaterials);

	S32 dataSize;
	ar & dataSize;

	if (!internalStorage && Btl::StackAllocatorSystem::Instance().CurrentAllocator() == 0)
		internalStorage = true;

	if (internalStorage)
	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		data.m_autoStackAllocator.Set(p);
		p->Reserve(dataSize + numMaterials * sizeof(Material) + 16);
		p->SetGrowSize(0);
	}

	data.m_materials.Resize(numMaterials);
	
	for (int i = 0; i < numMaterials; i++)
	{
		S32 typeIndex;
		ar & Arch::MakeNvp("TypeIndex", typeIndex);

		data.m_materials[i].m_typeIndex = typeIndex;
		DeserialiseMaterial(ar, data.m_materials[i], types.GetMaterialType(typeIndex));
	}

	if (internalStorage)
		data.m_autoStackAllocator.End();
}

template <typename Archive>
void Deserialise(Archive& ar, MaterialList& t)
{
	ar & Arch::MakeNvp("MaterialTypes", t.m_materialTypes);
	DeserialiseMaterialData(ar, t, t, false);
}

template <typename Archive>
inline void Deserialise(Archive& ar, MaterialNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);
	ar & t.m_materialType;
	DeserialiseMaterial(ar, t.m_material, t.m_materialType);
	ar & t.m_textureSource;
}

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode& t)
{
	DESERIALISE_BASE_FREE(CullNode, ar, t);

	if (ar.GetVersion() < 2)
	{
		Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
		p->SetGrowSize(1024);

		ar & Arch::MakeNvp("RenderPass", t.m_sceneGraphData.renderPassInfo);
		ar & Arch::MakeNvp("PrimitiveBounds", t.m_sceneGraphData.primitiveBounds);
		ar & Arch::MakeNvp("PrimitivePvs", t.m_sceneGraphData.primitivePvs.bits);

		// TODO: [Stove] Remove the test variable
		if (ar.GetVersion() == 1)
		{
			U32 iTest = 0xd15e45ed;
			ar & Arch::MakeNvp("Test", iTest);
			ar & Arch::MakeNvp("PortalRooms", t.m_sceneGraphData.vRoomIndices);
			ar & Arch::MakeNvp("PortalMap", t.m_sceneGraphData.vMap);
		}
		
		ar & Arch::MakeNvp("Primitives", t.m_renderData.primitives);
		ar & Arch::MakeNvp("InstanceBounds", t.m_sceneGraphData.instanceBounds);
		ar & Arch::MakeNvp("InstancePvs", t.m_sceneGraphData.instancePvs.bits);
		ar & Arch::MakeNvp("Instances", t.m_renderData.instances);
		ar & Arch::MakeNvp("InstanceBatchBounds", t.m_sceneGraphData.instanceBatchBounds);
		ar & Arch::MakeNvp("InstanceBatchPvs", t.m_sceneGraphData.instanceBatchPvs.bits);
		ar & Arch::MakeNvp("InstanceBatches", t.m_renderData.instanceBatches);
		ar & Arch::MakeNvp("BasePrimitives", t.m_renderData.basePrimitives);
		ar & Arch::MakeNvp("MaterialOutput", t.m_constantMaterialTypes);

		{
			t.m_sceneGraphData.dynamicMaterialLists.InitialiseSize(2);
			ar & t.m_sceneGraphData.dynamicMaterialLists[0];
			ar & t.m_sceneGraphData.dynamicMaterialLists[1];
		}

		ar & Arch::MakeNvp("IndexBufferSource", t.m_streamerData.indexBufferSource);
		ar & Arch::MakeNvp("VertexBufferSource", t.m_streamerData.vertexBufferSource);
		ar & Arch::MakeNvp("TextureSource", t.m_streamerData.textureSource);
		ar & Arch::MakeNvp("ChunkData", t.m_streamerData.chunkData);

		Btl::StackAllocatorSystem::Instance().End(p);
	 	t.Reallocate(); // to do: arrange archive to avoid having to do this. 
		Btl::StackAllocatorSystem::Instance().Free(p);
	}
	else
	{
		ar & Arch::MakeNvp("ConstantMaterialTypes", t.m_constantMaterialTypes);

		{
			Btl::DynamicStackAllocator* pTemp = Btl::StackAllocatorSystem::Instance().Begin();

			{
				RenderListNode::RenderData renderData;

				ar & Arch::MakeNvp("Primitives", renderData.primitives);
				ar & Arch::MakeNvp("Instances", renderData.instances);
				ar & Arch::MakeNvp("InstanceBatches", renderData.instanceBatches);
				ar & Arch::MakeNvp("BasePrimitives", renderData.basePrimitives);

				Btl::StackAllocatorSystem::Instance().End(pTemp);

				Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
				t.m_autoStackAllocatorRender.Set(p);
				p->Reserve(pTemp->BytesUsed()); 
				p->SetGrowSize(0);
				t.m_renderData = renderData;
				t.m_autoStackAllocatorRender.End();
			}

			Btl::StackAllocatorSystem::Instance().Free(pTemp);
		}

		{
			DeserialiseMaterialData(ar, t.m_constantMaterialData, t.m_constantMaterialTypes, true);
		}

		{
			Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
			t.m_autoStackAllocatorDefault.Set(p);

			ar & Arch::MakeNvp("RenderPass", t.m_sceneGraphData.renderPassInfo);
			ar & Arch::MakeNvp("PrimitiveBounds", t.m_sceneGraphData.primitiveBounds);
			ar & Arch::MakeNvp("InstanceBounds", t.m_sceneGraphData.instanceBounds);
			ar & Arch::MakeNvp("InstanceBatchBounds", t.m_sceneGraphData.instanceBatchBounds);
			ar & Arch::MakeNvp("PrimitivePvs", t.m_sceneGraphData.primitivePvs);
			ar & Arch::MakeNvp("InstancePvs", t.m_sceneGraphData.instancePvs);
			ar & Arch::MakeNvp("InstanceBatchPvs", t.m_sceneGraphData.instanceBatchPvs);
			ar & Arch::MakeNvp("PortalRooms", t.m_sceneGraphData.vRoomIndices);
			ar & Arch::MakeNvp("PortalMap", t.m_sceneGraphData.vMap);

			if (ar.GetVersion() == 3)
			{
				t.m_sceneGraphData.dynamicMaterialLists.InitialiseSize(2);
				ar & t.m_sceneGraphData.dynamicMaterialLists[0];
				ar & t.m_sceneGraphData.dynamicMaterialLists[1];
			}
			else if (ar.GetVersion() > 3)
			{
				ar & t.m_sceneGraphData.dynamicMaterialLists;
			}

			ar & Arch::MakeNvp("IndexBufferSource", t.m_streamerData.indexBufferSource);
			ar & Arch::MakeNvp("VertexBufferSource", t.m_streamerData.vertexBufferSource);
			ar & Arch::MakeNvp("TextureSource", t.m_streamerData.textureSource);
			ar & Arch::MakeNvp("ChunkData", t.m_streamerData.chunkData);

			t.m_autoStackAllocatorDefault.End();
		}
	}
}

namespace Target
{

template <typename Archive>
inline void Deserialise(Archive& ar, IndexBuffer& t);

template <typename Archive>
inline void Deserialise(Archive& ar, VertexBuffer& t);

template <typename Archive>
inline void Deserialise(Archive& ar, Texture& t);

template <typename Archive>
inline void Deserialise(Archive& ar, SharedTextureList& t);

template <typename Archive>
inline void Deserialise(Archive& ar, VertexDeclaration& t);

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode& t)
{
	assert(0);
}

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode& t)
{
	PERF_EVENT("Deserialise RenderListNode");

	DESERIALISE_BASE_FREE(RenderingData::RenderListNode, ar, t)
}

template <typename Archive>
inline void Deserialise(Archive& ar, Effect& t)
{
	DESERIALISE_BASE_FREE(RenderingData::Effect, ar, t)
}

template <typename Archive>
inline void Deserialise(Archive& ar, StreamData& t)
{
	Btl::DynamicStackAllocator* p = Btl::StackAllocatorSystem::Instance().Begin();
	t.m_autoStackAllocator.Set(p);
	DESERIALISE_BASE_FREE(RenderingData::StreamData, ar, t)
	Btl::StackAllocatorSystem::Instance().End(p);
}

template <typename Archive>
void Deserialise(Archive& ar, MaterialList& t)
{
	DESERIALISE_BASE_FREE(const RenderingData::MaterialList, ar, t);
}

void ActivateHandlers2();

template <typename Archive>
inline void Deserialise(Archive& ar, Scene& t)
{	
	FX::LockSerialiseMutex();

	ActivateHandlers2();

	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());

	DESERIALISE_BASE_FREE(RenderingData::Scene, ar, t)

	ar & t.m_vertexDeclarations;
	ar & t.m_effects;

	S32 n; ar & n;
	t.m_resourceBlocks.Resize(n);

	for (int i = 0; i < n; i++)
		t.m_resourceBlocks[i].SetScenePtr(&t);

	ar & t.m_sharedTextures;

	ar & t.m_streamData;
	t.m_streamData.SetScenePtr(&t);

	if (ar.GetVersion() > 0)
		ar & t.m_dynamicMaterialLists;

	FX::ReleaseSerialiseMutex();

}

void Scene::OnLoad(EffectVariationCallbackFunc pCallback)
{
	for (int i = 0; i < m_effects.Size(); i++)
	{
		m_effects[i].OnLoad(pCallback);
	}

	UpdateResourcePointers();
}

template <typename Archive>
void Deserialise(Archive& ar, LodReader<ResourceBlock::Resources>& t);

// Non-streamed resource block loading.
template <typename Archive>
inline void Deserialise(Archive& ar, ResourceBlock& t)
{
	g_resourceAllocatorMutex.Wait();

	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());
	t.m_back.discard = false;

	S32 numLods;
	ar & numLods;

	for (int i = 0; i < numLods; i++)
	{
		//FX::LockSerialiseMutex();
		ResourceBlock::Resources resources;
		resources.pScene = t.m_resources.pScene;

		if (i < t.GetMaxLodToLoad())
		{
			LodReader<ResourceBlock::Resources> blockLod(0, &t.m_back, resources, i);
			ar & blockLod;
			ResourceBlock::SwapNew(t.m_back, resources);
		}
		else
		{
			LodReader<ResourceBlock::Resources> blockLod(0, 0, resources, i);
			ar & blockLod;
		}

		//FX::ReleaseSerialiseMutex();
	}

	//FX::LockSerialiseMutex();
	ResourceBlock::SwapNew(t.m_resources, t.m_back);
	//FX::ReleaseSerialiseMutex();

	g_resourceAllocatorMutex.Release();
}

// get a pointer to the Effect that corresponds to the name of the effect
void Effect::OnLoad(EffectVariationCallbackFunc pCallback)
{
	Btl::String name;

	if (pCallback)
		name = (*pCallback)(GetName().Chars());
	else
		name = (GetName() + "b").Chars();

	Btl::String sPath = FX::GetShaderPath();

	m_pFX = FX::LoadEffect((sPath + name).Chars());
	if (!m_pFX)
		m_pFX = FX::LoadEffect((Btl::String("shaders\\") + name).Chars());	// try to load the missing shader from the old location

	if (!m_pFX)
	{
//		BIZ_ASSERT_MSG(m_pFX, "A shader '%s' failed to load - please make sure you have the latest shaders", name.Chars());

		// attempt to load invalid.fx instead
		m_pFX = FX::LoadEffect((sPath + "invalid.fxb").Chars());
	}
	else
		LOGDEBUG( g_Log, "Loaded %s\n", name.Chars());
}

template <typename Archive>
void Deserialise(Archive& ar, IndexBuffer& t)
{
	//PERF_EVENT("Deserialise index buffer");

	S32 size;
	IndexType indexType;
	ar & size;
	ar & indexType;

	U32 bufferSize = 0;
	
	if (indexType == INDEX_TYPE_U16)
		bufferSize = size * 2;
	else if (indexType == INDEX_TYPE_U32)
		bufferSize = size * 4;
	else
		assert(0 && "Index type corrupt.");

#if defined (PLATFORM_PS3)
	// [LloydW] Put the index buffers into local memory as we're running out of main memory for the vehicle vertex buffers
	U32 flags = Display2::E_Flags_AlignSizeToBlockSize; // SPU DMAs aligned to 128 bytes...
#else
	U32 flags = 0;
#endif

	bool streamed = (ar.Stream().Name() == "MemoryStream");

	Display2::Device()->AllowOutOfMemory(streamed);
	
	Display2::Device()->CreateIndexBuffer(bufferSize, flags, 
										(indexType == INDEX_TYPE_U16) ? Display2::E_IndexFormat_16 : Display2::E_IndexFormat_32, 
										&t.m_data);

#if !CFG_GOLD && CFG_SPEED && PLATFORM_PS3
	if(!streamed && t.m_data.IsCreated() && (flags & Display2::E_Flags_System) && (t.m_data.GetFlags() & Display2::E_Flags_Video))
	{
		LOGWARN(g_Log, "Wanted system memory index buffer of size %i got video memory instead\n", bufferSize);
		BIZ_ASSERT_SPEED(0 && "Wanted system memory index buffer got video memory instead. Please tell LloydW asap."); // Only in speed as we reduce main graphics memory on other builds
	}
#endif
	
	Display2::Device()->AllowOutOfMemory(false);

	void* p = 0;

	if (t.m_data.IsCreated())
	{
#ifndef PLATFORM_PC
		p = t.m_data.GetAddress();
#else
		p = t.m_data.Lock();
#endif
		assert(p); // out of memory
	}

	if (ar.GetVersion() > 0) // using deserialiser
	{
		Arch::Deserialiser deserialiser;
		ar & deserialiser;

#ifndef PLATFORM_PC
		if (deserialiser.GetCompressionType() != Arch::NO_COMPRESSION)
		{
			deserialiser.BeginDecompression((U8*)p, bufferSize, StreamManager::Instance().GetPolicy(), streamed ? StreamManager::Instance().GetDecompressionGroup() : -1);
		}
		else if (streamed)
		{
			StreamManager::Instance().AsynchCopy(p, deserialiser.Stream().m_pCurBuffer, bufferSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);
		}
		else
#endif
		{
			deserialiser.Read(p, bufferSize);
		}
	}
	else
	{
		if (streamed)
		{
			StreamManager::Instance().AsynchCopy(p, ar.Stream().m_pCurBuffer, bufferSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);
			ar.Read(0, bufferSize);
		}
		else
		{
			ar.Read(p, bufferSize);
		}
	}

#ifdef PLATFORM_PC
	if (p)
		t.m_data.Unlock();
#endif
}

template <typename Archive>
void Deserialise(Archive& ar, VertexBuffer& t)
{
	//PERF_EVENT("Deserialise vertex buffer");

	S32 size;
	VertexFormat vertexFormat;
	ar & vertexFormat;
	ar & size;

	U32 bufferSize = size * vertexFormat.GetDataSize();

#if defined (PLATFORM_PS3)
	U32 flags = Display2::E_Flags_System | Display2::E_Flags_AlignSizeToBlockSize; // SPU DMAs aligned to 128 bytes...
#else
	U32 flags = 0;
#endif

	bool streamed = (ar.Stream().Name() == "MemoryStream");

	Display2::Device()->AllowOutOfMemory(streamed);

	{
		//PERF_EVENT("Create vertex buffer");

		Display2::Device()->CreateVertexBuffer(bufferSize, flags, &t.m_data);

#if !CFG_GOLD && CFG_SPEED && PLATFORM_PS3
		if(!streamed && t.m_data.IsCreated() && (flags & Display2::E_Flags_System) && (t.m_data.GetFlags() & Display2::E_Flags_Video))
		{
			LOGWARN(g_Log, "Wanted system memory vertex buffer of size %i got video memory instead\n", bufferSize);
			BIZ_ASSERT_SPEED(0 && "Wanted system memory vertex buffer got video memory instead. Please tell LloydW asap."); // Only in speed as we reduce main graphics memory on other builds
		}
#endif
	}
	
	Display2::Device()->AllowOutOfMemory(false);

	void* p = 0;
	
	if (t.m_data.IsCreated())
	{
#ifndef PLATFORM_PC
		p = t.m_data.GetAddress();
#else
		p = t.m_data.Lock();
#endif
		assert(p); // out of memory
	}

	if (ar.GetVersion() > 0) // using deserialiser
	{
		//PERF_EVENT("Read vertex buffer");

		Arch::Deserialiser deserialiser;
		ar & deserialiser;

#ifndef PLATFORM_PC
		if (deserialiser.GetCompressionType() != Arch::NO_COMPRESSION)
		{
			deserialiser.BeginDecompression((U8*)p, bufferSize, StreamManager::Instance().GetPolicy(), streamed ? StreamManager::Instance().GetDecompressionGroup() : -1);
		}
		else if (streamed)
		{
			StreamManager::Instance().AsynchCopy(p, deserialiser.Stream().m_pCurBuffer, bufferSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);
		}
		else
#endif
		{
			deserialiser.Read(p, bufferSize);
		}
	}
	else
	{
		if (streamed)
		{
			StreamManager::Instance().AsynchCopy(p, ar.Stream().m_pCurBuffer, bufferSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);
			ar.Read(0, bufferSize);
		}
		else
		{
			ar.Read(p, bufferSize);
		}
	}

	if (p)
	{
#ifdef PLATFORM_PC
		t.m_data.Unlock();
#endif

		t.m_vertexSize = vertexFormat.GetDataSize();
	}
}

Display2::E_VertexElementType GetDisplayVertexElementType(VertexElementType type)
{
	switch (type)
	{
		case VERTEX_ELEMENT_TYPE_S16_2: return Display2::E_VertexElementType_S16_2;
		case VERTEX_ELEMENT_TYPE_S16_4: return Display2::E_VertexElementType_S16_4;
		case VERTEX_ELEMENT_TYPE_S16_2N: return Display2::E_VertexElementType_S16_2N;
		case VERTEX_ELEMENT_TYPE_S16_4N: assert(0); return Display2::E_VertexElementType_Unknown;
		case VERTEX_ELEMENT_TYPE_F32_1: return Display2::E_VertexElementType_F32_1;
		case VERTEX_ELEMENT_TYPE_F32_2: return Display2::E_VertexElementType_F32_2;
		case VERTEX_ELEMENT_TYPE_F32_3: return Display2::E_VertexElementType_F32_3;
		case VERTEX_ELEMENT_TYPE_F32_4: return Display2::E_VertexElementType_F32_4;
		case VERTEX_ELEMENT_TYPE_F16_2: return Display2::E_VertexElementType_F16_2;
		case VERTEX_ELEMENT_TYPE_F16_4: return Display2::E_VertexElementType_F16_4;
		case VERTEX_ELEMENT_TYPE_U8_4: return Display2::E_VertexElementType_U8_4;
		case VERTEX_ELEMENT_TYPE_U8_4N: return Display2::E_VertexElementType_U8_4N;
#if !PLATFORM_PC
		case VERTEX_ELEMENT_TYPE_SHHD_3N: return Display2::E_VertexElementType_SHHD_3N;
#endif
		//case VERTEX_ELEMENT_TYPE_SDDD_3N: return Display2::E_VertexElementType_SDDD_3N;
		default: assert(0); return Display2::E_VertexElementType_Unknown;
	}
}

Display2::E_VertexElementUsage GetDisplayVertexElementUsage(VertexElementUsage usage)
{
	switch (usage)
	{
		case VERTEX_ELEMENT_USAGE_POSITION: return Display2::E_VertexElementUsage_Position;
		case VERTEX_ELEMENT_USAGE_BLENDWEIGHT: return Display2::E_VertexElementUsage_BlendWeight;
		case VERTEX_ELEMENT_USAGE_BLENDINDICES: return Display2::E_VertexElementUsage_BlendIndices;
		case VERTEX_ELEMENT_USAGE_NORMAL: return Display2::E_VertexElementUsage_Normal;
		case VERTEX_ELEMENT_USAGE_PSIZE: return Display2::E_VertexElementUsage_PSize;
		case VERTEX_ELEMENT_USAGE_TEXCOORD: return Display2::E_VertexElementUsage_TexCoord;
		case VERTEX_ELEMENT_USAGE_TANGENT: return Display2::E_VertexElementUsage_Tangent;
		case VERTEX_ELEMENT_USAGE_BINORMAL: return Display2::E_VertexElementUsage_Binormal;
		//case VERTEX_ELEMENT_USAGE_TESSFACTOR: return Display2::E_VertexElementUsage_TessFactor;
		case VERTEX_ELEMENT_USAGE_COLOR: return Display2::E_VertexElementUsage_Colour;
		//case VERTEX_ELEMENT_USAGE_SAMPLE: return Display2::E_VertexElementUsage_Sample;
		default: assert(0); return Display2::E_VertexElementUsage_Unknown;
	}
}

template <typename Archive>
void Deserialise(Archive& ar, VertexDeclaration& t)
{
	Btl::Vector<Display2::C_VertexElement> elements;

	S32 nElements;
	ar & nElements;

	elements.Resize(nElements + 1);

	for (int i = 0; i < nElements; i++)
	{
		RenderingData::VertexDeclaration::VertexElement element;
		ar & element;

		elements[i].SetStream(element.stream);
		elements[i].SetOffset(element.offset);
		elements[i].SetType(GetDisplayVertexElementType(element.type));
		elements[i].SetUsage(GetDisplayVertexElementUsage(element.usage));
		elements[i].SetUsageIndex(element.usageIndex);
	}

	elements[nElements] = GPU_VERTEX_ELEMENT_END();

	t.m_pData = Display2::Device()->CreateVertexDeclaration(&elements[0]);
}

template <typename Archive>
void Deserialise(Archive& ar, LodReader<ResourceBlock::Resources>& t)
{
	PERF_EVENT("Deserialise resource block");

	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());

	ResourceBlock::Resources& next(t.m_next);
	next.discard = false;

	S32 lod;
	ar & lod;
	assert(lod == t.m_lod);
	assert(t.m_pBack == 0 || lod == t.m_pBack->lod + 1);

	if (lod == 0)
	{
		//PERF_EVENT("Deserialise lod 0");

		ar & next.vertexBuffers;
		ar & next.indexBuffers;
		ar & next.nodes;

		for (int i = 0; i < next.vertexBuffers.Size(); i++)
		{
			if (!next.vertexBuffers[i]->GetData().IsCreated())
				next.discard = true;
		}

		for (int i = 0; i < next.indexBuffers.Size(); i++)
		{
			if (!next.indexBuffers[i]->GetData().IsCreated())
				next.discard = true;
		}

		for (int i = 0; i < next.nodes.Size(); i++)
		{
			Node* pNode = Pointer(next.nodes[i]);
			if (pNode) pNode->SetScenePtr(next.pScene);
		}
	}

	{
		//PERF_EVENT("Deserialise textures");

		S32 nTextures;
		ar & nTextures;
		next.textures.Resize(nTextures);

		assert(nTextures <= 16);

		for (int i = 0; i < nTextures; i++)
		{
			//PERF_EVENT("Deserialise texture");

			const Texture* pFront = 0;

			if (t.m_pBack && !t.m_pBack->Empty())
				pFront = Pointer(t.m_pBack->textures[i]);

			if (t.m_pFront && (pFront == 0 || !pFront->GetData().IsCreated()))
				pFront = Pointer(t.m_pFront->textures[i]);

			Allocate(next.textures[i]);
			LodReader<Texture> textureLod(pFront, 0, *next.textures[i], t.m_lod);
			ar & textureLod;

			/*if (textureLod.m_source.GetIndex() > -1)
			{
				Texture& src = *next.pScene->GetResourceBlock(textureLod.m_source.GetBlock()).m_back.textures[textureLod.m_source.GetIndex()];
				Texture& dst = textureLod.m_next;
				
				U8* address = src.m_data.GetAddress();

				Display2::E_TextureFormat format = src.m_data.GetFormat();
				assert(format == Display2::E_TextureFormat_DXT1 || format == Display2::E_TextureFormat_DXT5);
				S32 tile = (format == Display2::E_TextureFormat_DXT1) ? 8192 : 16384;

				dst.m_data.SetAddress(address + tile / 2);
			}*/

			if (next.textures[i]->Discard())
				next.discard = true;
		}
	}

	next.lod = t.m_lod;
	assert(next.lod > -1);
}

Display::E_TextureAddress GetDisplayTextureAddress(TextureAddress textureAddress)
{
	switch (textureAddress)
	{
		case TEXTURE_ADDRESS_WRAP: return Display::E_TextureAddress_Wrap;
		case TEXTURE_ADDRESS_CLAMP: return Display::E_TextureAddress_Clamp;
		case TEXTURE_ADDRESS_MIRROR: return Display::E_TextureAddress_Mirror;
		case TEXTURE_ADDRESS_BORDER: return Display::E_TextureAddress_Border;
		default: return Display::E_TextureAddress_Wrap;
	}
}

Display::E_TextureFilter GetDisplayTextureFilter(TextureFilter textureFilter)
{
	switch (textureFilter)
	{
		case TEXTURE_FILTER_POINT: return Display::E_TextureFilter_Point;
		case TEXTURE_FILTER_LINEAR: return Display::E_TextureFilter_Linear;
		case TEXTURE_FILTER_ANISOTROPIC: return Display::E_TextureFilter_Anisotropic;
		default: return Display::E_TextureFilter_Linear;
	}
}

S32 GetSamplerStateIndex(const SamplerState& samplerState)
{
	typedef Btl::OpenHashMap<SamplerState, S32> SamplerStateMap;
	static SamplerStateMap s_samplerStateMap;
	SamplerStateMap::InsertionReturnType r = s_samplerStateMap.Insert(samplerState);

	if (r.inserted)
	{
		S32 i = FX::g_SamplerStateMgr.CreateSamplerState(	GetDisplayTextureAddress(samplerState.GetAddressU()),
															GetDisplayTextureAddress(samplerState.GetAddressV()), 
															GetDisplayTextureAddress(samplerState.GetAddressW()),
															GetDisplayTextureFilter(samplerState.GetMipFilter()),
															GetDisplayTextureFilter(samplerState.GetMinFilter()),
															GetDisplayTextureFilter(samplerState.GetMagFilter()),
															Maths::Clamped(samplerState.GetMaxAnisotropy(), 1, 16),
															samplerState.GetSRGB() == 1, samplerState.GetMipMapLodBias());
		r.iterator->GetValue() = i;
	}

	return r.iterator->GetValue();
}


template <typename Archive>
void Deserialise(Archive& ar, LodReader<Texture>& t)
{
	//PERF_EVENT("Deserialise texture");

	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());
	assert(t.m_pBack == 0);

	if (t.m_lod == 0)
	{
		//PERF_EVENT("Deserialise texture states");

		assert(t.m_pFront == 0);

		ar & t.m_next.m_name;

		SamplerState samplerState;
		ar & samplerState;
		t.m_next.m_samplerStateIndex = GetSamplerStateIndex(samplerState);

		ar & t.m_next.m_hashValue;

#if !CFG_SPEED
		SharedTextureManager::Instance().CheckPotentialTextureSharing(t.m_next.m_name.Chars(), t.m_next.m_hashValue);
#endif
	}

	U32 present;
	ar >> present;

	bool paired = (present > 1);

	if (present)
	{
		S32 headerSize;
		ar & headerSize;

		if (t.m_lod == 5)
			int breakhere = 0;

		if (headerSize > 0)
		{
			//PERF_EVENT("Create texture header");

			if (t.m_pFront)
			{
				//PERF_EVENT("Copy texture headers");

				t.m_next.m_headers.Reserve(t.m_pFront->m_headers.Size() + 1);
				t.m_next.m_headers = t.m_pFront->m_headers;
				t.m_next.m_hashValue = t.m_pFront->m_hashValue;
				t.m_next.m_name = t.m_pFront->m_name;
				t.m_next.m_samplerStateIndex = t.m_pFront->m_samplerStateIndex;
			}

#if defined(DEVICE_PS3GCM)
			static const U32 HEADER_SIZE = sizeof(CellGcmTexture) + 12;
#else	
			static const U32 HEADER_SIZE = sizeof(Display::TextureDesc);
#endif

			{
				//PERF_EVENT("Create texture");

				U8 pHeaderData[HEADER_SIZE];
				assert(headerSize <= HEADER_SIZE);
				ar.Read(pHeaderData, headerSize);

				Sys::C_MemoryStream stream(pHeaderData, headerSize);
#if defined (PLATFORM_PS3)
				t.m_next.m_data.SetFlags( t.m_next.m_data.GetFlags() | Display::E_Usage_AlignSizeToBlockSize ); // SPU DMAs aligned to 128 bytes...
#endif
				Display::CreateTexture2D(&t.m_next.m_data, &stream);
				t.m_next.m_data.SetFlags( t.m_next.m_data.GetFlags() | Display::E_Usage_DoNotReload | Display::E_Usage_Streamed );

				if (paired)
					t.m_next.m_data.SetFlags( t.m_next.m_data.GetFlags() | Display::E_Usage_Indirect );
				else if (!t.m_next.m_data.IsCreated())
					t.m_next.m_discard = true;

#ifndef CFG_SPEED
				#if TRACK_DISPLAY2_RESOURCES
				if(t.m_next.m_data.IsCreated())
					Display2::Device()->SetTextureName(&t.m_next.m_data, t.m_next.GetName());
				#endif
#endif

				t.m_next.m_headers.PushBack();
				Texture::HeaderData& header = t.m_next.m_headers.Back();
				header.lod = t.m_lod;
				header.width = Display2::GetWidth2D(t.m_next.m_data);
				header.height = Display2::GetHeight2D(t.m_next.m_data);
				header.levels = Display2::GetLevels(t.m_next.m_data);
				header.size = t.m_next.m_data.GetSize();
			}
		}

		if (!paired)
		{
			bool streamed = (ar.Stream().Name() == "MemoryStream");

			if (ar.GetVersion() == 0)
			{
				S32 dataSize;
				ar & dataSize;

				if (dataSize > 0)
				{
					{
	#ifdef PLATFORM_PC
						Display::LoadTextureData2D(&t.m_next.m_data, &ar.Stream(), dataSize);
	#else
						if (!t.m_next.m_discard)
							StreamManager::Instance().AsynchCopy(t.m_next.m_data.GetAddress(), ar.Stream().m_pCurBuffer, dataSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);

						ar.Stream().Read(0, dataSize);
	#endif
					}
				}
			}
			else
			{
				//PERF_EVENT("Read texture data");

				Arch::Deserialiser deserialiser;
				ar & deserialiser;

				if (!t.m_next.m_discard)
				{
					S32 bufferSize = deserialiser.GetSize();

#ifndef PLATFORM_PC
					void* p = t.m_next.m_data.GetAddress();

					if (deserialiser.GetCompressionType() != Arch::NO_COMPRESSION)
					{
						deserialiser.BeginDecompression((U8*)p, bufferSize, StreamManager::Instance().GetPolicy(), streamed ? StreamManager::Instance().GetDecompressionGroup() : -1);
					}
					else if (streamed)
					{
						StreamManager::Instance().AsynchCopy(p, deserialiser.Stream().m_pCurBuffer, bufferSize, StreamManager::ASYNCH_COPY_SYSTEM_TO_GPU);
					}
					else
					{
						deserialiser.Read(p, bufferSize);
					}
#else
					Display::LoadTextureData2D(&t.m_next.m_data, &deserialiser.Stream(), bufferSize);
#endif
				}
			}

			// Copy existing mip levels.
			if (t.m_pFront && !t.m_next.m_discard)
			{
				//PERF_EVENT("Copy texture data");
#ifdef PLATFORM_PC
				Display::CopyTextureData2D(&t.m_next.m_data, &t.m_pFront->m_data);
#else
				Display::C_Texture2D *pDst = &t.m_next.m_data;
				const Display::C_Texture2D *pSrc = &t.m_pFront->m_data;
				int size = Maths::Min(pSrc->GetSize(), pDst->GetSize());
				int dstOffset = Maths::Max((S32) pDst->GetSize() - (S32) pSrc->GetSize(), (S32) 0);
				int srcOffset = Maths::Max((S32) pSrc->GetSize() - (S32) pDst->GetSize(), (S32) 0);

				if (streamed)
				{
					StreamManager::Instance().AsynchCopy(pDst->GetAddress() + dstOffset, pSrc->GetAddress() + srcOffset, size, StreamManager::ASYNCH_COPY_GPU_TO_GPU);
				}
				else
				{
					memcpy(pDst->GetAddress() + dstOffset, pSrc->GetAddress() + srcOffset, size);
				}

#endif
			}
		}
		else
		{
			ar & t.m_source;
		}
	}
}

template <typename Archive>
void Deserialise(Archive& ar, Texture& t)
{
	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());

	ar & t.m_name;

	SamplerState samplerState;
	ar & samplerState;
	t.m_samplerStateIndex = GetSamplerStateIndex(samplerState);

	ar & t.m_hashValue;

	// read texture data
	S32 size;
	ar & size;

	if (size > 0)
	{
#if PLATFORM_PC
		U8* data = new U8[size];
		ar.Read(data, size);
		Sys::C_MemoryStream stream(data, size);
		Display::PCLoadTexture(&t.m_data, &stream);
		delete [] data;
#else
		Display::LoadTexture2D(&t.m_data, &ar.Stream());
#endif

		t.m_data.SetFlags( t.m_data.GetFlags() | Display::E_Usage_DoNotReload );
		
		if (t.m_data.IsCreated())
		{
			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetTextureName(&t.m_data, t.GetName());
			#endif
		}
		else
		{
			LOGWARN(g_Log, "Texture was not created (%s, %i bytes).", t.GetName(), size);
		}
	}
}

template <typename Archive>
inline void Deserialise(Archive& ar, SharedTextureList& t)
{
	g_resourceAllocatorMutex.Wait();

	PlatformType platformType; ar & platformType;
	assert(platformType == GetPlatformType());

	S32 nTextures;
	ar & nTextures;

	t.m_textures.Resize(nTextures);

	for (int i = 0; i < nTextures; i++)
	{
		Btl::String name;
		U32 hash;
		ar & name;
		ar & hash;

		t.m_textures[i] = SharedTextureManager::Instance().InsertTexture(name.Chars(), hash);
		
		if (!t.m_textures[i]->GetData().IsCreated())
		{
			ar & *t.m_textures[i];
		}
		else
		{
			Texture dummy;
			ar & dummy;
		}
	}

	g_resourceAllocatorMutex.Release();
}

Display::E_PrimitiveType GetDisplayPrimitiveType(PrimitiveType type)
{
	switch (type)
	{
		case PRIMITIVE_TYPE_TRIANGLE_LIST: return Display::E_PrimitiveType_TriangleList;
		case PRIMITIVE_TYPE_TRIANGLE_STRIP: return Display::E_PrimitiveType_TriangleStrip;
		case PRIMITIVE_TYPE_POINT_LIST: return Display::E_PrimitiveType_PointList;
		case PRIMITIVE_TYPE_QUAD_LIST: return Display::E_PrimitiveType_QuadList;
		case PRIMITIVE_TYPE_QUAD_STRIP: assert(0); return Display::E_PrimitiveType_Unknown;
		default: assert(0); return Display::E_PrimitiveType_Unknown;
	}
}

Display::E_Blend GetDisplayBlend(BlendFactor blend)
{
	switch (blend)
	{
		case BLEND_FACTOR_ZERO: return Display::E_Blend_Zero;
		case BLEND_FACTOR_ONE: return Display::E_Blend_One;
		case BLEND_FACTOR_SRCCOLOUR: return Display::E_Blend_SrcColour;
		case BLEND_FACTOR_INVSRCCOLOUR: return Display::E_Blend_InvSrcColour;
		case BLEND_FACTOR_SRCALPHA: return Display::E_Blend_SrcAlpha;
		case BLEND_FACTOR_INVSRCALPHA: return Display::E_Blend_InvSrcAlpha;
		case BLEND_FACTOR_DSTALPHA: return Display::E_Blend_DstAlpha;
		case BLEND_FACTOR_INVDSTALPHA: return Display::E_Blend_InvDstAlpha;
		case BLEND_FACTOR_DSTCOLOUR: return Display::E_Blend_DstColour;
		case BLEND_FACTOR_INVDSTCOLOUR: return Display::E_Blend_InvDstColour;
		default: assert(0); return Display::E_Blend_Unknown;
	}
}

Display::E_BlendOp GetDisplayBlendOp(BlendOp blendOp)
{
	switch (blendOp)
	{
		case BLEND_OP_ADD: return Display::E_BlendOp_Add;
		case BLEND_OP_SUBTRACT: return Display::E_BlendOp_Subtract;
		case BLEND_OP_MIN: return Display::E_BlendOp_Min;
		case BLEND_OP_MAX: return Display::E_BlendOp_Max;
		case BLEND_OP_REVSUBTRACT: return Display::E_BlendOp_RevSubtract;
		default: assert(0); return Display::E_BlendOp_Unknown;
	}
}

Display::E_CullMode GetDisplayCullMode(CullMode cullMode)
{
	switch (cullMode)
	{
	case CULL_MODE_NORMAL:	return Display::E_CullMode_Default;
	case CULL_MODE_NONE:	return Display::E_CullMode_None;
	case CULL_MODE_REVERSED:return Display::E_CullMode_Reversed;
	default: assert(0); return Display::E_CullMode_Default;
	}
}

Display::E_Bool GetDisplayBool(bool b)
{
	if (b) return Display::E_Bool_True;
	else return Display::E_Bool_False;
}

} // Target ///////////////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode::Primitive& t)
{
	PrimitiveType primitiveType;
	BlendState blendState;

	ar & t.effect;
	ar & t.vertexDeclaration;
	ar & primitiveType;
	ar & t.indexBufferIndex;
	ar & t.flags;
	ar & t.baseMaterial;

	if (ar.GetVersion() > 0)
		ar & t.instanceMaterial;
	else
		t.instanceMaterial.data = -1;

	ar & blendState;
	ar & t.lods;
	ar & t.primitiveStreams;

	t.primitiveType = Target::GetDisplayPrimitiveType(primitiveType);

	FX::BlendState displayStates;
	displayStates.srcBlend = Target::GetDisplayBlend(blendState.GetSrcBlend());
	displayStates.destBlend = Target::GetDisplayBlend(blendState.GetDestBlend());
	displayStates.blendOp = Target::GetDisplayBlendOp(blendState.GetBlendOp());
	displayStates.srcBlendAlpha = Display::E_Blend_Zero;
	displayStates.destBlendAlpha = Display::E_Blend_InvSrcAlpha;
	displayStates.blendOpAlpha = Display::E_BlendOp_Add;
	displayStates.alphaRef = blendState.GetAlphaRef();
	displayStates.alphaToMaskEnable = Target::GetDisplayBool(blendState.GetAlphaToMaskEnable());
	displayStates.alphaTestEnable = Target::GetDisplayBool(blendState.GetAlphaTestEnable());
	displayStates.cullMode = Target::GetDisplayCullMode(blendState.GetCullMode());

#ifdef PLATFORM_PC
	// AlphaToMask not supported
	if (displayStates.alphaToMaskEnable == Display::E_Bool_True)
	{
		displayStates.alphaToMaskEnable = Display::E_Bool_False;
		displayStates.alphaRef = 128;
	}
#endif

	t.blendState = FX::g_RenderStateMgr.InsertRenderState(displayStates);
}

} // RenderingData //////////////////////////////////////////////////////////////////

SERIALISATION_VERSION(RenderingData::Target::Effect, 0, "RenderingData::Target::Effect")
SERIALISATION_VERSION(RenderingData::Target::VertexBuffer, 1, "RenderingData::Target::VertexBuffer")
SERIALISATION_VERSION(RenderingData::Target::IndexBuffer, 1, "RenderingData::Target::IndexBuffer")
SERIALISATION_VERSION(RenderingData::Target::LodReader<RenderingData::Target::Texture>, 2, "RenderingData::Target::TextureLod")
SERIALISATION_VERSION(RenderingData::Target::StreamData, 0, "RenderingData::Target::StreamData")
SERIALISATION_POLYMORPHIC(RenderingData::Target::RenderListNode, 0, "RenderingData::RenderListNode_Common")

namespace RenderingData
{

template void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, RenderListNode::Primitive& t);
template void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, RenderListNode::Primitive& t);

template void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, MaterialNode& t);
template void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, MaterialNode& t);

namespace Target
{

// The resource allocator holds:
// Texture objects (96 bytes)
// Texture headers (12 bytes)
// Texture names (only for debug / texture reload)
// Vertex buffer objects (40 bytes)
// Index buffer objects (36 bytes)
//Sys::MemoryBuffer g_resourceAllocator;
#if defined(_WIN64)
// [Blur2PC][x64] The retail areas fit the x86 object and pointer-array sizes; those at most
// double on x64 (a 13-entry vertex-buffer pointer array is 104 bytes), so the areas double too.
int g_sizes[] = {32, 96, 192, 0};
#else
int g_sizes[] = {16, 48, 96, 0}; 
#endif
Memory::BlockAllocator g_resourceAllocator(g_sizes, false);
MT::Mutex g_resourceAllocatorMutex;

#ifndef CFG_SPEED
int g_resourceAllocatorLock = 0;
#endif

void ActivateHandlers2()
{
	g_resourceAllocatorMutex.Wait();

	//if (g_resourceAllocator.BufferBegin() == 0)
	//	g_resourceAllocator.Initialise(1024 * 1024 * 2, 16, 8);

	g_resourceAllocatorMutex.Release();

	SERIALISATION_HANDLER(RenderingData::Target::RenderListNode)
}

SERIALISATION_KEY(RenderListNode)

template void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, Scene& t);
template void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, Scene& t);
template void Deserialise(Arch::BinaryInputArchive_t<Ut::NoEndianSwap>& ar, ResourceBlock& t);
template void Deserialise(Arch::BinaryInputArchive_t<Ut::DoEndianSwap>& ar, ResourceBlock& t);
}

}
