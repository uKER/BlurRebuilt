////////////////////////////////////////////////////////////////////
// RenderingData archive functions shared between converter and game.

namespace RenderingData
{

////////////////////////////////////////////////////////////////////////
// Scene
////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, PlatformType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(PlatformType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, PlatformType& t)
{
	switch (t)
	{
		case PLATFORM_TYPE_360: ar << "U16"; return;
		case PLATFORM_TYPE_PS3: ar << "PS3"; return;
		case PLATFORM_TYPE_PC: ar << "PC"; return;
		default:
			BIZ_ASSERT_FAIL("PlatformType corrupt");
			return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, Scene& t)
{
	ActivateHandlers();

	ar & Arch::MakeNvp("Name", t.m_name);
	ar & Arch::MakeNvp("RootNode", t.m_rootNode);
}

AUTO_DESERIALISE_FREE(Scene)

/////////////////////////////////////////////////////////////////////////
// Node
/////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, Bounds& t)
{
	ar & Arch::MakeNvp("AabbMin", t.aabbMin);
	ar & Arch::MakeNvp("AabbMax", t.aabbMax);
	ar & Arch::MakeNvp("DistanceMin", t.distanceMin);
	ar & Arch::MakeNvp("DistanceMax", t.distanceMax);
}

AUTO_DESERIALISE_FREE(Bounds)	

template <typename Archive>
inline void Serialise(Archive& ar, Node& t)
{
	ar & Arch::MakeNvp("Name", t.m_name);
	ar & Arch::MakeNvp("SubNodes", t.m_subNodes);
	ar & Arch::MakeNvp("Parent", t.m_parent);
	ar & Arch::MakeNvp("Location", t.m_location);
}

AUTO_DESERIALISE_FREE(Node)

template <typename Archive>
inline void Serialise(Archive& ar, CullNode& t)
{
	SERIALISE_BASE_FREE(Node, ar, t);
	ar & Arch::MakeNvp("Bounds", t.m_bounds);
	ar & Arch::MakeNvp("PvsBits", t.m_pvsBits);

	if (ar.GetVersion() > 0)
		ar & Arch::MakeNvp("GroupIndex", t.m_groupIndex);
	else
		t.m_groupIndex = -1;

	if (ar.GetVersion() > 1)
		ar & Arch::MakeNvp("PortalBits", t.m_portalBits);
}

AUTO_DESERIALISE_FREE(CullNode)

template <typename Archive>
inline void Serialise(Archive& ar, RootNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);
}

AUTO_DESERIALISE_FREE(RootNode)

template <typename Archive>
inline void Serialise(Archive& ar, GroupNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);
	ar & Arch::MakeNvp("GroupName", t.m_groupName);
}

AUTO_DESERIALISE_FREE(GroupNode)

template <typename Archive>
inline void Serialise(Archive& ar, LodNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);
	ar & Arch::MakeNvp("LodThresholds", t.m_lodThresholds);
}

AUTO_DESERIALISE_FREE(LodNode)

///////////////////////////////////////////////////////////////////////////
// Resource Index
///////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, ResourceIndex& t)
{
	ar & Arch::MakeNvp("Block", t.m_block);
	ar & Arch::MakeNvp("Index", t.m_index);
}

AUTO_DESERIALISE_FREE(ResourceIndex)

////////////////////////////////////////////////////////////////////////////
// Vertex Buffer
////////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, IndexType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(IndexType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, IndexType& t)
{
	switch (t)
	{
		case INDEX_TYPE_U16: ar << "U16"; return;
		case INDEX_TYPE_U32: ar << "U32"; return;
		default:
			BIZ_ASSERT_FAIL("IndexType corrupt");
			return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, VertexElementType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(VertexElementType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, VertexElementType& t)
{
	switch (t)
	{
		case VERTEX_ELEMENT_TYPE_S16_2: ar << "S16_2"; return;
		case VERTEX_ELEMENT_TYPE_S16_4: ar << "S16_4"; return;
		case VERTEX_ELEMENT_TYPE_S16_2N: ar << "S16_2N"; return;
		case VERTEX_ELEMENT_TYPE_S16_4N: ar << "S16_4N"; return;
		case VERTEX_ELEMENT_TYPE_F32_1: ar << "F32_1"; return;
		case VERTEX_ELEMENT_TYPE_F32_2: ar << "F32_2"; return;
		case VERTEX_ELEMENT_TYPE_F32_3: ar << "F32_3"; return;
		case VERTEX_ELEMENT_TYPE_F32_4: ar << "F32_4"; return;
		case VERTEX_ELEMENT_TYPE_F16_2: ar << "F32_4"; return;
		case VERTEX_ELEMENT_TYPE_F16_4: ar << "F16_4"; return;
		case VERTEX_ELEMENT_TYPE_U8_4: ar << "U8_4"; return;
		case VERTEX_ELEMENT_TYPE_U8_4N: ar << "U8_4N"; return;
		case VERTEX_ELEMENT_TYPE_SHHD_3N: ar << "SHHD_3N"; return;
		case VERTEX_ELEMENT_TYPE_SDDD_3N: ar << "SDDD_3N"; return;
		default:
			BIZ_ASSERT_FAIL("VertexElementType corrupt");
			return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, VertexElementMethod& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(VertexElementMethod)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, VertexElementMethod& t)
{
	switch (t)
	{
		case VERTEX_ELEMENT_METHOD_DEFAULT: ar << "DEFAULT"; return;
		default:
			BIZ_ASSERT_FAIL("VertexElementMethod corrupt");
			return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, VertexElementUsage& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(VertexElementUsage)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, VertexElementUsage& t)
{
	switch (t)
	{
		case VERTEX_ELEMENT_USAGE_POSITION: ar << "POSITION"; return;
		case VERTEX_ELEMENT_USAGE_BLENDWEIGHT: ar << "BLENDWEIGHT"; return;
		case VERTEX_ELEMENT_USAGE_BLENDINDICES: ar << "BLENDINDICES"; return;
		case VERTEX_ELEMENT_USAGE_NORMAL: ar << "NORMAL"; return;
		case VERTEX_ELEMENT_USAGE_PSIZE: ar << "PSIZE"; return;
		case VERTEX_ELEMENT_USAGE_TEXCOORD: ar << "TEXCOORD"; return;
		case VERTEX_ELEMENT_USAGE_TANGENT: ar << "TANGENT"; return;
		case VERTEX_ELEMENT_USAGE_BINORMAL: ar << "BINORMAL"; return;
		case VERTEX_ELEMENT_USAGE_TESSFACTOR: ar << "TESSFACTOR"; return;
		case VERTEX_ELEMENT_USAGE_COLOR: ar << "COLOR"; return;
		case VERTEX_ELEMENT_USAGE_SAMPLE: ar << "SAMPLE"; return;
		default:
			BIZ_ASSERT_FAIL("VertexElementUsage corrupt");
			return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, VertexFormat::VertexElement& t)
{
	ar & Arch::MakeNvp("Type", t.type);
	ar & Arch::MakeNvp("Method", t.method);
	ar & Arch::MakeNvp("Usage", t.usage);
	ar & Arch::MakeNvp("UsageIndex", t.usageIndex);
}

AUTO_DESERIALISE_FREE(VertexFormat::VertexElement)

template <typename Archive>
inline void Serialise(Archive& ar, VertexFormat& t)
{
	ar & Arch::MakeNvp("DataSize", t.m_dataSize);
	ar & Arch::MakeNvp("Elements", t.m_elements);
	ar & Arch::MakeNvp("Offsets", t.m_offsets);
}

AUTO_DESERIALISE_FREE(VertexFormat)

template <typename Archive>
inline void Serialise(Archive& ar, VertexDeclaration::VertexElement& t)
{
	ar & Arch::MakeNvp("Stream", t.stream);
	ar & Arch::MakeNvp("Offset", t.offset);
	ar & Arch::MakeNvp("Type", t.type);
	ar & Arch::MakeNvp("Method", t.method);
	ar & Arch::MakeNvp("Usage", t.usage);
	ar & Arch::MakeNvp("UsageIndex", t.usageIndex);
};

AUTO_DESERIALISE_FREE(VertexDeclaration::VertexElement)

template <typename Archive>
inline void Serialise(Archive& ar, VertexDeclaration& t)
{
	S32 nElements = t.GetNumVertexElements();
	ar & Arch::MakeNvp("Size", nElements);

	for (int i = 0; i < nElements; i++)
		ar & t.m_elements[i];
}

///////////////////////////////////////////////////////////////////////////
// Effect
///////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, Effect& t)
{
	ar & Arch::MakeNvp("Name", t.m_name);
}

AUTO_DESERIALISE_FREE(Effect)

/////////////////////////////////////////////////////////////////////////
// Material
//////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, MaterialIndex& t)
{
	S32 index = t.index;
	S32 listIndex = t.listIndex;

	ar & Arch::MakeNvp("Index", index);
	ar & Arch::MakeNvp("ProcessType", listIndex);
}

template <typename Archive>
inline void Deserialise(Archive& ar, MaterialIndex& t)
{
	S32 index;
	S32 listIndex;

	ar & index;
	ar & listIndex;

	t.index = index;
	t.listIndex = listIndex;
}

template <typename Archive>
inline void Serialise(Archive& ar, MaterialType& t)
{
	ar & Arch::MakeNvp("Name", t.m_name);
	ar & Arch::MakeNvp("MaterialElements", t.m_elements);
	ar & Arch::MakeNvp("DataSize", t.m_dataSize);
	ar & Arch::MakeNvp("MaxAlign", t.m_maxAlign);
	ar & Arch::MakeNvp("Offsets", t.m_offsets);
	ar & Arch::MakeNvp("Effect", t.m_effect);
	ar & Arch::MakeNvp("ListIndex", t.m_listIndex);
}

AUTO_DESERIALISE_FREE(MaterialType)

template <typename Archive>
inline void Serialise(Archive& ar, MaterialElement& t)
{
	ar & Arch::MakeNvp("Name", t.name);
	ar & Arch::MakeNvp("Type", t.type);
	ar & Arch::MakeNvp("Count", t.count);
}

AUTO_DESERIALISE_FREE(MaterialElement)

template <typename Archive>
inline void Serialise(Archive& ar, MaterialElementType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(MaterialElementType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, MaterialElementType& t)
{
	switch (t)
	{
		case MATERIAL_ELEMENT_TYPE_S8: ar << "S8"; return;
		case MATERIAL_ELEMENT_TYPE_U8: ar << "U8"; return;
		case MATERIAL_ELEMENT_TYPE_S16: ar << "S16"; return;
		case MATERIAL_ELEMENT_TYPE_U16: ar << "U16"; return;
		case MATERIAL_ELEMENT_TYPE_S32: ar << "S32"; return;
		case MATERIAL_ELEMENT_TYPE_U32: ar << "U32"; return;
		case MATERIAL_ELEMENT_TYPE_F32: ar << "F32"; return;
		case MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX: ar << "TEXTURE_INDEX"; return;
		case MATERIAL_ELEMENT_TYPE_TEXTURE_PTR: ar << "TEXTURE_PTR"; return;
		case MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX: ar << "MATERIAL_INDEX"; return;
		case MATERIAL_ELEMENT_TYPE_DATA_INDEX: ar << "DATA_INDEX"; return;
		default: assert(!"MaterialElementType corrupt"); return;
	}
}

template <typename Archive>
inline void SerialiseMaterial(Archive& ar, const Material& material, const MaterialType& materialType)
{
	const U8* pData = material.GetData();

	for (int e = 0; e < materialType.GetNumElements(); e++)
	{
		for (int c = 0; c < materialType.GetElement(e).count; c++)
		{
			switch (materialType.GetElement(e).type)
			{
				case MATERIAL_ELEMENT_TYPE_S8:
					ar << *(S8*) pData; pData += 1;
					break;
				case MATERIAL_ELEMENT_TYPE_U8:
					ar << *(U8*) pData; pData += 1;
					break;
				case MATERIAL_ELEMENT_TYPE_S16:
					ar << *(S16*) pData; pData += 2;
					break;
				case MATERIAL_ELEMENT_TYPE_U16:
					ar << *(U16*) pData; pData += 2;
					break;
				case MATERIAL_ELEMENT_TYPE_S32:
					ar << *(S32*) pData; pData += 4;
					break;
				case MATERIAL_ELEMENT_TYPE_U32:
					ar << *(U32*) pData; pData += 4;
					break;
				case MATERIAL_ELEMENT_TYPE_F32:
					ar << *(F32*) pData; pData += 4;
					break;
				case MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX:
					ar << *(S32*) pData; pData += 4;
					break;
				case MATERIAL_ELEMENT_TYPE_TEXTURE_PTR:
					ar << *(S32*) pData; pData += 4;
					break;
				case MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX:
				case MATERIAL_ELEMENT_TYPE_DATA_INDEX:
					ar << *(MaterialIndex*) pData; pData += 4;
					break;
				default: 
					assert(0 && "Material element type corrupt.");
			}
		}
	}
}

template <typename Archive>
void SerialiseMaterialData(Archive& ar, const MaterialListData& data, const MaterialList& types)
{
	S32 numMaterials = data.GetNumMaterials();
	ar & Arch::MakeNvp("NumMaterials", numMaterials);

	S32 dataSize = data.GetDataSize();
	ar & dataSize;
	
	for (int i = 0; i < data.GetNumMaterials(); i++)
	{
		S32 typeIndex = data.GetMaterial(i).GetTypeIndex();
		ar & Arch::MakeNvp("TypeIndex", typeIndex);

		if (!Btl::SameType<Archive, Arch::XmlOutputArchive>::result)
		{
			SerialiseMaterial(ar, data.GetMaterial(i), types.GetMaterialType(typeIndex));
		}
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, MaterialList& t)
{
	ar & Arch::MakeNvp("MaterialTypes", t.m_materialTypes);

	SerialiseMaterialData(ar, t, t);
}

template <typename Archive>
inline void Serialise(Archive& ar, MaterialNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);
	ar & Arch::MakeNvp("MaterialType", t.m_materialType);
	SerialiseMaterial(ar, t.m_material, t.m_materialType);
	ar & Arch::MakeNvp("TextureSource", t.m_textureSource);
}

///////////////////////////////////////////////////////////////////////
// Render States
///////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, BlendFactor& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(BlendFactor)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, BlendFactor& t)
{
	switch (t)
	{
		case BLEND_FACTOR_ZERO: ar << "Zero"; return;
		case BLEND_FACTOR_ONE: ar << "One"; return;
		case BLEND_FACTOR_SRCCOLOUR: ar << "SrcColour"; return;
		case BLEND_FACTOR_INVSRCCOLOUR: ar << "InvSrcColour"; return;
		case BLEND_FACTOR_SRCALPHA: ar << "SrcAlpha"; return;
		case BLEND_FACTOR_INVSRCALPHA: ar << "InvSrcAlpha"; return;
		case BLEND_FACTOR_DSTALPHA: ar << "DstAlpha"; return;
		case BLEND_FACTOR_INVDSTALPHA: ar << "InvDstAlpha"; return;
		case BLEND_FACTOR_DSTCOLOUR: ar << "DstColour"; return;
		case BLEND_FACTOR_INVDSTCOLOUR: ar << "InvDstColour"; return;
		default: assert(!"BlendFactor corrupt"); return;
	}
};

template <typename Archive>
inline void Serialise(Archive& ar, BlendOp& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(BlendOp)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, BlendOp& t) 
{
	switch (t)
	{
		case BLEND_OP_ADD: ar << "Add"; return;
		case BLEND_OP_SUBTRACT: ar << "Subtract"; return;
		case BLEND_OP_MIN: ar << "Min"; return;
		case BLEND_OP_MAX: ar << "Max"; return;
		case BLEND_OP_REVSUBTRACT: ar << "RevSubtract"; return;
		default: assert(!"BlendOp corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, CullMode& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(CullMode)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, CullMode& t) 
{
	switch (t)
	{
		case CULL_MODE_NONE: ar << "Node"; return;
		case CULL_MODE_NORMAL: ar << "Normal"; return;
		case CULL_MODE_REVERSED: ar << "Reversed"; return;
		default: assert(!"CullMode corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, BlendState& t)
{
	BlendFactor srcBlend = t.GetSrcBlend();
	BlendFactor destBlend = t.GetDestBlend();
	BlendOp blendOp = t.GetBlendOp();
	bool alphaBlendEnable = t.GetAlphaBlendEnable();
	bool alphaTestEnable = t.GetAlphaTestEnable();
	U8 alphaRef = t.GetAlphaRef();
	bool alphaToMaskEnable = t.GetAlphaToMaskEnable();
	CullMode cullMode = t.GetCullMode();
	S32 zBias = t.GetZBias();

	ar & Arch::MakeNvp("SrcBlend", srcBlend);
	ar & Arch::MakeNvp("DestBlend", destBlend);
	ar & Arch::MakeNvp("BlendOp", blendOp);
	ar & Arch::MakeNvp("AlphaBlendEnable", alphaBlendEnable);
	ar & Arch::MakeNvp("AlphaTestEnable", alphaTestEnable);
	ar & Arch::MakeNvp("AlphaRef", alphaRef);
	ar & Arch::MakeNvp("AlphaToMaskEnable", alphaToMaskEnable);
	ar & Arch::MakeNvp("CullMode", cullMode);
	ar & Arch::MakeNvp("ZBias", zBias);
}


template <typename Archive>
inline void Deserialise(Archive& ar, BlendState& t)
{
	BlendFactor srcBlend;
	BlendFactor destBlend;
	BlendOp blendOp;
	bool alphaBlendEnable;
	bool alphaTestEnable;
	U8 alphaRef;
	bool alphaToMaskEnable;
	CullMode cullMode = CULL_MODE_NORMAL;
	S32 zBias = 0;

	ar & srcBlend;
	ar & destBlend;
	ar & blendOp;
	ar & alphaBlendEnable;
	ar & alphaTestEnable;
	ar & alphaRef;
	ar & alphaToMaskEnable;
	ar & cullMode;
	ar & zBias;

	t.SetSrcBlend(srcBlend);
	t.SetDestBlend(destBlend);
	t.SetBlendOp(blendOp);
	t.SetAlphaBlendEnable(alphaBlendEnable);
	t.SetAlphaTestEnable(alphaTestEnable);
	t.SetAlphaRef(alphaRef);
	t.SetAlphaToMaskEnable(alphaToMaskEnable);
	t.SetCullMode(cullMode);
	t.SetZBias(zBias);
}

template <typename Archive>
inline void Serialise(Archive& ar, TextureAddress& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(TextureAddress)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, TextureAddress& t) 
{
	switch (t)
	{
		case TEXTURE_ADDRESS_WRAP: ar << "WRAP"; return;
		case TEXTURE_ADDRESS_MIRROR: ar << "MIRROR"; return;
		case TEXTURE_ADDRESS_CLAMP: ar << "CLAMP"; return;
		case TEXTURE_ADDRESS_BORDER: ar << "BORDER"; return;
		default: assert(!"TextureAddress corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, TextureFilter& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(TextureFilter)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, TextureFilter& t)
{
	switch (t)
	{
		case TEXTURE_FILTER_POINT: ar << "POINT"; return;
		case TEXTURE_FILTER_LINEAR: ar << "LINEAR"; return;
		case TEXTURE_FILTER_ANISOTROPIC: ar << "ANISOTROPIC"; return;
		default: assert(!"TextureFilter corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, SamplerState& t)
{	
	TextureAddress addressU = t.GetAddressU();
	TextureAddress addressV = t.GetAddressV();
	TextureAddress addressW = t.GetAddressW();
	TextureFilter magFilter = t.GetMagFilter();
	TextureFilter minFilter = t.GetMinFilter();
	TextureFilter mipFilter = t.GetMipFilter();
	S32 maxAnisotropy = t.GetMaxAnisotropy();
	S32 maxMipLevel = t.GetMaxMipLevel();
	S32 mipMapLodBias = t.GetMipMapLodBias();
	U32 sRGB = t.GetSRGB();

	ar & Arch::MakeNvp("AddressU", addressU);
	ar & Arch::MakeNvp("AddressV", addressV);
	ar & Arch::MakeNvp("AddressW", addressW);
	ar & Arch::MakeNvp("MagFilter", magFilter);
	ar & Arch::MakeNvp("MinFilter", minFilter);
	ar & Arch::MakeNvp("MipFilter", mipFilter);
	ar & Arch::MakeNvp("MaxAnisotropy", maxAnisotropy);
	ar & Arch::MakeNvp("MaxMipLevel", maxMipLevel);
	ar & Arch::MakeNvp("MipMapLodBias", mipMapLodBias);
	ar & Arch::MakeNvp("SRGB", sRGB);
}

template <typename Archive>
inline void Deserialise(Archive& ar, SamplerState& t)
{	
	TextureAddress addressU;
	TextureAddress addressV;
	TextureAddress addressW;
	TextureFilter magFilter;
	TextureFilter minFilter;
	TextureFilter mipFilter;
	S32 maxAnisotropy;
	S32 maxMipLevel;
	S32 mipMapLodBias;
	U32 sRGB = 0;

	ar & addressU;
	ar & addressV;
	ar & addressW;
	ar & magFilter;
	ar & minFilter;
	ar & mipFilter;
	ar & maxAnisotropy;
	ar & maxMipLevel;
	ar & mipMapLodBias;

	if (ar.GetVersion() > 0)
		ar & sRGB;

	t.SetAddressU(addressU);
	t.SetAddressV(addressV);
	t.SetAddressW(addressW);
	t.SetMagFilter(magFilter);
	t.SetMinFilter(minFilter);
	t.SetMipFilter(mipFilter);
	t.SetMaxAnisotropy(maxAnisotropy);
	t.SetMaxMipLevel(maxMipLevel);
	t.SetMipMapLodBias(mipMapLodBias);
	t.SetSRGB(sRGB);
}

///////////////////////////////////////////////////////////////////////////
// Render
///////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, SortType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(SortType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, SortType& t)
{	switch (t)
	{
		case SORT_TYPE_KEY: ar << "KEY"; return;
		case SORT_TYPE_BACK_FRONT: ar << "BACKFRONT"; return;
		default: assert(!"SortType corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, RenderPass& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(RenderPass)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, RenderPass& t)
{	switch (t)
	{
		case RENDER_PASS_OPAQUE: ar << "OPAQUE"; return;
		case RENDER_PASS_ALPHA_TEST: ar << "ALPHA_TEST"; return;
		case RENDER_PASS_OVERLAY: ar << "OVERLAY"; return;
		case RENDER_PASS_BACK_FRONT: ar << "BACK_FRONT"; return;
		case RENDER_PASS_POST_RESOLVE: ar << "RENDER_PASS_POST_RESOLVE"; return;
		default: assert(!"RenderPass corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, PrimitiveType& t)
{
	ar & (S32&) t;
}

AUTO_DESERIALISE_FREE(PrimitiveType)

template <>
inline void Serialise(Arch::XmlOutputArchive& ar, PrimitiveType& t)
{
	switch (t)
	{
		case PRIMITIVE_TYPE_TRIANGLE_LIST: ar << "TRIANGLE_LIST"; return;
		case PRIMITIVE_TYPE_TRIANGLE_STRIP: ar << "TRIANGLE_STRIP"; return;
		case PRIMITIVE_TYPE_POINT_LIST: ar << "POINT_LIST"; return;
		case PRIMITIVE_TYPE_QUAD_LIST: ar << "QUAD_LIST"; return;
		case PRIMITIVE_TYPE_QUAD_STRIP: ar << "QUAD_STRIP"; return;
		default: assert(!"TextureAddress corrupt"); return;
	}
}

template <typename Archive>
inline void Serialise(Archive& ar, RenderNode& t)
{
	ar & Arch::MakeNvp("Object", t.m_object);
}

AUTO_DESERIALISE_FREE(RenderNode)

template <typename Archive>
inline void Serialise(Archive& ar, RenderObject& t)
{
	assert(0 && "Cannot serialise render object.");
}

AUTO_DESERIALISE_FREE(RenderObject)

///////////////////////////////////////////////////////////////////////
// Render List
///////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::PrimitiveStream& t)
{
	S32 vertexBuffer = t.vertexBuffer;
	S32 streamIndex = t.streamIndex;

	ar & Arch::MakeNvp("VertexBuffer", vertexBuffer);
	ar & Arch::MakeNvp("StreamIndex", streamIndex);
	ar & Arch::MakeNvp("VertexOffset", t.vertexOffset);
	ar & Arch::MakeNvp("Frequency", t.frequency);
	ar & Arch::MakeNvp("Material", t.material);
}

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode::PrimitiveStream& t)
{
	S32 vertexBuffer, streamIndex;

	ar & Arch::MakeNvp("VertexBuffer", vertexBuffer);
	ar & Arch::MakeNvp("StreamIndex", streamIndex);
	ar & Arch::MakeNvp("VertexOffset", t.vertexOffset);
	ar & Arch::MakeNvp("Frequency", t.frequency);
	ar & Arch::MakeNvp("Material", t.material);

	t.vertexBuffer = vertexBuffer;
	t.streamIndex = streamIndex;
}

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::Primitive::Lod& t)
{
	ar & Arch::MakeNvp("Offset", t.offset);
	ar & Arch::MakeNvp("NIndices", t.nIndices);
	ar & Arch::MakeNvp("MinIndex", t.minIndex);
	ar & Arch::MakeNvp("NVertices", t.nVertices);
}

AUTO_DESERIALISE_FREE(RenderListNode::Primitive::Lod)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::Primitive& t)
{
	ar & Arch::MakeNvp("Effect", t.effect);
	ar & Arch::MakeNvp("VertexDeclaration", t.vertexDeclaration);
	ar & Arch::MakeNvp("PrimitiveType", (PrimitiveType&) t.primitiveType);
	ar & Arch::MakeNvp("IndexBufferIndex", t.indexBufferIndex);
	ar & Arch::MakeNvp("Flags", t.flags);
	ar & Arch::MakeNvp("BaseMaterial", t.baseMaterial);
	ar & Arch::MakeNvp("InstanceMaterial", t.instanceMaterial);
	ar & Arch::MakeNvp("BlendState", t.blendState);
	ar & Arch::MakeNvp("Lods", t.lods);
	ar & Arch::MakeNvp("PrimitiveStreams", t.primitiveStreams);
}

template <typename Archive>
inline void Deserialise(Archive& ar, RenderListNode::Primitive& t);

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::Instance& t)
{
	ar & Arch::MakeNvp("PrimitiveIndex", t.primitiveIndex);
	ar & Arch::MakeNvp("Material", t.material);
	ar & Arch::MakeNvp("PrimitiveStreams", t.primitiveStreams);
	ar & Arch::MakeNvp("Flags", t.flags);
}

AUTO_DESERIALISE_FREE(RenderListNode::Instance)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::InstanceBatch& t)
{
	ar & Arch::MakeNvp("Instance", t.instance);
	ar & Arch::MakeNvp("Stream", t.stream);
}

AUTO_DESERIALISE_FREE(RenderListNode::InstanceBatch)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::RenderPassInfo& t)
{
	ar & Arch::MakeNvp("Primitives", t.nPrimitives);
	ar & Arch::MakeNvp("PrimitiveOffset", t.primitiveOffset);
	ar & Arch::MakeNvp("NInstances", t.nInstances);
	ar & Arch::MakeNvp("InstanceOffset", t.instanceOffset);
	ar & Arch::MakeNvp("NInstanceBatches", t.nInstanceBatches);
	ar & Arch::MakeNvp("InstanceBatchOffset", t.instanceBatchOffset);
}

AUTO_DESERIALISE_FREE(RenderListNode::RenderPassInfo)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::ChunkData::ResourceInfo& t)
{
	ar & Arch::MakeNvp("ResourceIndex", t.resourceIndex);
	ar & Arch::MakeNvp("IndexInList", t.indexInList);
}

AUTO_DESERIALISE_FREE(RenderListNode::ChunkData::ResourceInfo)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::ChunkData& t)
{
	ar & Arch::MakeNvp("ChunkIndex", t.chunkIndex);
	ar & Arch::MakeNvp("IndexBuffers", t.indexBuffers);
	ar & Arch::MakeNvp("VertexBuffers", t.vertexBuffers);
	ar & Arch::MakeNvp("Textures", t.textures);
}

AUTO_DESERIALISE_FREE(RenderListNode::ChunkData)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::RoomObjectMapper& t)
{
	ar & Arch::MakeNvp("PrimitiveBits", t.m_vPrimitives);
	ar & Arch::MakeNvp("InstanceBits", t.m_vInstances);
}

AUTO_DESERIALISE_FREE(RenderListNode::RoomObjectMapper)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode::PvsCellData& t)
{
#if CFG_CONVERTER
	t.Build();
#endif

	ar & t.nCells;

	if (ar.GetVersion() > 0)
		ar & t.offsets;

	ar & t.cellSize;
	ar & t.data;
}

AUTO_DESERIALISE_FREE(RenderListNode::PvsCellData)

template <typename Archive>
inline void Serialise(Archive& ar, RenderListNode& t)
{
	SERIALISE_BASE_FREE(CullNode, ar, t);

	ar & Arch::MakeNvp("ConstantMaterialTypes", t.m_constantMaterialTypes);

	ar & Arch::MakeNvp("Primitives", t.m_renderData.primitives);
	ar & Arch::MakeNvp("Instances", t.m_renderData.instances);
	ar & Arch::MakeNvp("InstanceBatches", t.m_renderData.instanceBatches);
	ar & Arch::MakeNvp("BasePrimitives", t.m_renderData.basePrimitives);

	SerialiseMaterialData(ar, t.m_constantMaterialData, t.m_constantMaterialTypes);

	ar & Arch::MakeNvp("RenderPass", t.m_sceneGraphData.renderPassInfo);
	ar & Arch::MakeNvp("PrimitiveBounds", t.m_sceneGraphData.primitiveBounds);
	ar & Arch::MakeNvp("InstanceBounds", t.m_sceneGraphData.instanceBounds);
	ar & Arch::MakeNvp("InstanceBatchBounds", t.m_sceneGraphData.instanceBatchBounds);
	ar & Arch::MakeNvp("PrimitivePvs", t.m_sceneGraphData.primitivePvs);
	ar & Arch::MakeNvp("InstancePvs", t.m_sceneGraphData.instancePvs);
	ar & Arch::MakeNvp("InstanceBatchPvs", t.m_sceneGraphData.instanceBatchPvs);
	ar & Arch::MakeNvp("PortalRooms", t.m_sceneGraphData.vRoomIndices);
	ar & Arch::MakeNvp("PortalMap", t.m_sceneGraphData.vMap);
	ar & Arch::MakeNvp("DynamicMaterials", t.m_sceneGraphData.dynamicMaterialLists);

	ar & Arch::MakeNvp("IndexBufferSource", t.m_streamerData.indexBufferSource);
	ar & Arch::MakeNvp("VertexBufferSource", t.m_streamerData.vertexBufferSource);
	ar & Arch::MakeNvp("TextureSource", t.m_streamerData.textureSource);
	ar & Arch::MakeNvp("ChunkData", t.m_streamerData.chunkData);
}

///////////////////////////////////////////////////////////////////////////
// StreamData
///////////////////////////////////////////////////////////////////////////

template <typename Archive>
inline void Serialise(Archive& ar, ChunkInfo::Item& t)
{
	ar & Arch::MakeNvp("Block", t.block);
	ar & Arch::MakeNvp("Lod", t.lod); 
	//ar & Arch::MakeNvp("Size", t.size);
}

AUTO_DESERIALISE_FREE(ChunkInfo::Item)

template <typename Archive>
inline void Serialise(Archive& ar, ChunkInfo& t)
{
	ar & Arch::MakeNvp("Bounds", t.m_bounds);
	ar & Arch::MakeNvp("Items", t.m_items);
	ar & Arch::MakeNvp("Blocks", t.m_blocks); 
	ar & Arch::MakeNvp("Prerequisites", t.m_prerequisites); 
	ar & Arch::MakeNvp("Dependants", t.m_dependants); 
	ar & Arch::MakeNvp("Offset", t.m_offset);
	ar & Arch::MakeNvp("CompressedSize", t.m_compressedSize);

	if (ar.GetVersion() > 0)
		ar & Arch::MakeNvp("OriginalSize", t.m_originalSize);
	else
		t.m_originalSize = t.m_compressedSize;
}

AUTO_DESERIALISE_FREE(ChunkInfo)

template <typename Archive>
inline void Serialise(Archive& ar, StreamData& t)
{
	ar & Arch::MakeNvp("Chunks", t.m_chunks);
	ar & Arch::MakeNvp("ChunkPvs", t.m_chunkPvs);

	if (ar.GetVersion() > 0)
	{
		ar & Arch::MakeNvp("ChunkBounds", t.m_chunkBounds);
		ar & Arch::MakeNvp("Groups", t.m_groups);
	}

	if (ar.GetVersion() > 1)
		ar & Arch::MakeNvp("ChunkRooms", t.m_chunkRooms);

	if (ar.GetVersion() > 2)
		ar & t.m_chunkVersion;
	else
		t.m_chunkVersion = 0;
}

AUTO_DESERIALISE_FREE(StreamData)

}

///////////////////////////////////////////////////////////////////////
// Serialisation info (has to be outside namespace)
///////////////////////////////////////////////////////////////////////

// Scene
SERIALISATION_VERSION(RenderingData::Scene, 0, "RenderingData::Scene")

// Node
SERIALISATION_VERSION(RenderingData::Bounds, 0, "RenderingData::Bounds")
SERIALISATION_POLYMORPHIC(RenderingData::Node, 0, "RenderingData::Node")

SERIALISATION_POLYMORPHIC(RenderingData::RootNode, 0, "RenderingData::RootNode")
SERIALISATION_POLYMORPHIC(RenderingData::CullNode, 2, "RenderingData::CullNode")
SERIALISATION_POLYMORPHIC(RenderingData::GroupNode, 0, "RenderingData::GroupNode")
SERIALISATION_POLYMORPHIC(RenderingData::LodNode, 0, "RenderingData::LodNode")

// Resource Index
SERIALISATION_PRIMITIVE(RenderingData::ResourceIndex, "RenderingData::ResourceIndex")

// Vertex Buffer
SERIALISATION_PRIMITIVE(RenderingData::IndexType, "RenderingData::VertexElementType")
SERIALISATION_PRIMITIVE(RenderingData::VertexElementType, "RenderingData::VertexElementType")
SERIALISATION_PRIMITIVE(RenderingData::VertexElementMethod, "RenderingData::VertexElementMethod")
SERIALISATION_PRIMITIVE(RenderingData::VertexElementUsage, "RenderingData::VertexElementUsage")
SERIALISATION_VERSION(RenderingData::VertexFormat::VertexElement, 0, "RenderingData::VertexFormat::VertexElement")
SERIALISATION_VERSION(RenderingData::VertexFormat, 0, "RenderingData::VertexFormat")
SERIALISATION_VERSION(RenderingData::VertexDeclaration::VertexElement, 0, "RenderingData::VertexDeclaration::VertexElement")
SERIALISATION_VERSION(RenderingData::VertexDeclaration, 0, "RenderingData::VertexDeclaration")

// Effect
SERIALISATION_VERSION(RenderingData::Effect, 0, "RenderingData::Effect")

// DataTypes
SERIALISATION_VERSION(RenderingData::MaterialIndex, 0, "RenderingData::MaterialIndex")

// Material
SERIALISATION_VERSION(RenderingData::MaterialType, 0, "RenderingData::MaterialType")
SERIALISATION_VERSION(RenderingData::MaterialElement, 0, "RenderingData::MaterialElement")
SERIALISATION_PRIMITIVE(RenderingData::MaterialElementType, "RenderingData::MaterialElementType")
SERIALISATION_VERSION(RenderingData::MaterialList, 0, "RenderingData::MaterialList")
SERIALISATION_POLYMORPHIC(RenderingData::MaterialNode, 0, "RenderingData::MaterialNode")

// Render States
SERIALISATION_PRIMITIVE(RenderingData::BlendFactor, "RenderingData::BlendFactor")
SERIALISATION_PRIMITIVE(RenderingData::BlendOp, "RenderingData::BlendOp")
SERIALISATION_PRIMITIVE(RenderingData::TextureAddress, "RenderingData::TextureAddress")
SERIALISATION_PRIMITIVE(RenderingData::TextureFilter, "RenderingData::TextureFilter")
SERIALISATION_PRIMITIVE(RenderingData::CullMode, "RenderingData::CullMode")
SERIALISATION_VERSION(RenderingData::BlendState, 0, "RenderingData::BlendState")
SERIALISATION_VERSION(RenderingData::SamplerState, 1, "RenderingData::SamplerState")

// Render
SERIALISATION_PRIMITIVE(RenderingData::SortType, "RenderingData::SortType")
SERIALISATION_PRIMITIVE(RenderingData::RenderPass, "RenderingData::RenderPass")
SERIALISATION_PRIMITIVE(RenderingData::PrimitiveType, "RenderingData::PrimitiveType")
SERIALISATION_PRIMITIVE(RenderingData::RenderObjectType, "RenderingData::RenderObjectType")
SERIALISATION_POLYMORPHIC(RenderingData::RenderNode, 0, "RenderingData::RenderNode")

// RenderList
SERIALISATION_VERSION(RenderingData::RenderListNode::PrimitiveStream, 0, "RenderingData::RenderListNode::PrimitiveStream")
SERIALISATION_VERSION(RenderingData::RenderListNode::Primitive::Lod, 0, "RenderingData::RenderListNode::Primitive::Lod")
SERIALISATION_VERSION(RenderingData::RenderListNode::Primitive, 1, "RenderingData::RenderListNode::Primitive")
SERIALISATION_VERSION(RenderingData::RenderListNode::Instance, 0, "RenderingData::RenderListNode::Instance")
SERIALISATION_VERSION(RenderingData::RenderListNode::InstanceBatch, 0, "RenderingData::RenderListNode::InstanceBatch")
SERIALISATION_VERSION(RenderingData::RenderListNode::RenderPassInfo, 0, "RenderingData::RenderListNode::RenderPassInfo")
SERIALISATION_VERSION(RenderingData::RenderListNode::ChunkData, 0, "RenderingData::RenderListNode::ChunkData")
SERIALISATION_VERSION(RenderingData::RenderListNode::ChunkData::ResourceInfo, 0, "RenderingData::RenderListNode::ChunkData::ResourceInfo")
SERIALISATION_VERSION(RenderingData::RenderListNode::RoomObjectMapper, 0, "RenderingData::RenderListNode::RoomObjectMapper")
SERIALISATION_VERSION(RenderingData::RenderListNode::PvsCellData, 1, "RenderingData::RenderListNode::PvsCellData")
SERIALISATION_POLYMORPHIC(RenderingData::RenderListNode, 4, "RenderingData::RenderListNode") 

// StreamData
SERIALISATION_VERSION(RenderingData::ChunkInfo::Item, 0, "RenderingData::ChunkInfo::Item")
SERIALISATION_VERSION(RenderingData::ChunkInfo, 1, "RenderingData::ChunkInfo")
SERIALISATION_VERSION(RenderingData::StreamData, 3, "RenderingData::StreamData")

