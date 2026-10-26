// MaterialType
inline void MaterialType::SetName(const Btl::String& s)
{
	m_name = s;
}

inline void MaterialType::SetDynamicListIndex(S32 listIndex)
{
	m_listIndex = listIndex + 1;
}

inline void MaterialType::SetEffect(S32 effect)
{
	m_effect = effect;
}

inline const Btl::String& MaterialType::GetName() const
{
	return m_name;
}

inline S32 MaterialType::GetNumElements() const
{
	return m_elements.Size();
}

inline const MaterialElement& MaterialType::GetElement(S32 i) const
{
	return m_elements[i];
}

inline S32 MaterialType::GetMaterialListIndex() const
{
	return m_listIndex;
}

inline S32 MaterialType::GetEffect() const
{
	return m_effect;
}

inline S32 MaterialType::GetOffset(S32 element, S32 index) const
{
	S32 size = materialDataTypeSizes[m_elements[element].type];
	return m_offsets[element] + index * size;
}

inline S32 MaterialType::GetMaxAlign() const
{
	return m_maxAlign;
}

// Material
inline Material::Material() 
{
	m_typeIndex = (U16)-1;
	m_dataSize = (U16)-1;
	m_alignment = 0;
}

inline S32 Material::GetTypeIndex() const
{
	return m_typeIndex;
}

inline U8* Material::GetData()
{
	return (U8*) Pointer(m_pData);
}

inline U8* Material::GetData(S32 offset)
{
	return (((U8*)Pointer(m_pData)) + offset);
}

inline const U8* Material::GetData() const
{
	return (U8*) Pointer(m_pData);
}

inline const U8* Material::GetData(S32 offset) const
{
	return (((U8*) Pointer(m_pData)) + offset);
}

inline void Material::SetData(S32 offset, const void* p, U32 size)
{
	assert(offset >= 0 && offset + size <= GetDataSize());
	memcpy(((U8*) Pointer(m_pData)) + offset, p, size);
}

inline void Material::SetData(const void* p, U32 size)
{
	assert(GetDataSize() == size);
	memcpy((U8*) Pointer(m_pData), p, Maths::Min(size, GetDataSize()));
}

template<typename T> 
inline void Material::SetData(S32 offset, const T& t)
{
	assert(offset >= 0 && offset + sizeof(T) <= GetDataSize());
	memcpy(((U8*) Pointer(m_pData)) + offset, &t, sizeof(T));
}

template<typename T> 
inline const T& Material::GetData(S32 offset) const
{
	assert(offset >= 0 && offset + sizeof(T) <= GetDataSize());
	return (T&)*((U8*) Pointer(m_pData) + offset);
}

inline U32 Material::GetDataSize() const
{
	return m_dataSize;
}

// MaterialList
inline S32 MaterialList::GetNumMaterialTypes() const
{
	return m_materialTypes.Size();
}

inline const MaterialType& MaterialList::GetMaterialType(S32 i) const
{
	return m_materialTypes[i];
}

inline const MaterialType& MaterialList::GetTypeOfMaterial(S32 i) const
{
	S32 typeIndex = m_materials[i].GetTypeIndex();
	return m_materialTypes[typeIndex];
}

inline void MaterialList::CopyTypes(const MaterialList& other)
{
	m_materialTypes = other.m_materialTypes;
	m_materials.Destroy();
}

// MaterialListData
inline MaterialListData::MaterialListData()
{
}

inline S32 MaterialListData::GetNumMaterials() const
{
	return m_materials.Size();
}

// Material Node
inline NodeType MaterialNode::GetNodeType() const
{
	return NODE_TYPE_MATERIAL;
}

inline bool MaterialNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_MATERIAL) return true;
	else return CullNode::IsNode(t);
}

inline MaterialNode* MaterialNode::Clone() const
{
	return new MaterialNode(*this);
}

inline void MaterialNode::CreateMaterial(const MaterialType& t)
{
	m_materialType = t;

	m_material.m_dataSize = (U16)m_materialType.GetDataSize();
	m_material.m_dataSize = Btl::AlignUp(m_material.m_dataSize, 4);

	if (m_material.m_dataSize > 0)
	{
		Allocate(m_material.m_pData, m_material.m_dataSize / 4, m_material.m_alignment);
		memset(Pointer(m_material.m_pData), 0, m_material.m_dataSize);
	}
}

inline const MaterialType& MaterialNode::GetMaterialType() const
{
	return m_materialType;
}

inline const Material& MaterialNode::GetMaterial() const
{
	return m_material;
}

inline Material& MaterialNode::GetMaterial()
{
	return m_material;
}

inline const ResourceIndex& MaterialNode::GetTexture(S32 i) const
{
	return m_textureSource[i];
}

inline S32 MaterialNode::GetNumTextures() const
{
	return m_textureSource.Size();
}

inline const MaterialNode* MaterialNode::GetPointer(const Node* p)
{
	if (p && p->IsNode(NODE_TYPE_MATERIAL))
		return (const MaterialNode*) p;
	else
		return 0;
}

inline MaterialNode* MaterialNode::GetPointer(Node* p)
{
	if (p && p->IsNode(NODE_TYPE_MATERIAL))
		return (MaterialNode*) p;
	else
		return 0;
}

#if USE_MATERIAL_CACHE

// ConstMaterialReference
struct ConstMaterialReference_Temp
{
	ConstMaterialReference_Temp(const Material& material, const MaterialListData& materialList, const U32* pData) 
		: m_material(material), m_materialList(materialList), m_pData((U32*)pData) {}

	const U32* m_pData;
	const Material& m_material;
	const MaterialListData& m_materialList;
};

inline ConstMaterialReference::ConstMaterialReference(const Material& material, const MaterialListData& materialList, const U8* pData) 
	: m_material(material), m_materialList(materialList), m_pData((U32*)pData)
{
}

inline ConstMaterialReference::ConstMaterialReference(ConstMaterialReference& other)
	: m_material(other.m_material), m_materialList(other.m_materialList), m_pData(other.m_pData)
{
	other.m_pData = 0;
}

inline ConstMaterialReference::ConstMaterialReference(const ConstMaterialReference_Temp& temp)
	: m_material(temp.m_material), m_materialList(temp.m_materialList), m_pData(temp.m_pData)
{
}

inline ConstMaterialReference& ConstMaterialReference::operator = (ConstMaterialReference& other)
{
	Release();
	assert(&m_material == &other.m_material);
	m_pData = other.m_pData;
	other.m_pData = 0;
	return *this;
}

inline ConstMaterialReference& ConstMaterialReference::operator = (const ConstMaterialReference_Temp& temp)
{
	Release();
	m_pData = temp.m_pData;
	return *this;
}
 
inline ConstMaterialReference::operator ConstMaterialReference_Temp ()
{
	ConstMaterialReference_Temp temp(m_material, m_materialList, m_pData);
	m_pData = 0;
	return temp;
}

inline ConstMaterialReference::~ConstMaterialReference()
{
	Release();
}

inline void ConstMaterialReference::Release()
{
	if (m_pData)
		m_materialList.Release(*this);
}

inline void ConstMaterialReference::Bind(ConstMaterialReference& other)
{
	*this = other;
}

inline void ConstMaterialReference::Bind(const ConstMaterialReference_Temp& temp)
{
	*this = temp;
}

inline S32 ConstMaterialReference::GetTypeIndex() const
{
	return m_material.GetTypeIndex();
}

inline const U8* ConstMaterialReference::GetData() const
{
	return (U8*) m_pData;
}

inline const U8* ConstMaterialReference::GetData(S32 offset) const
{
	assert(m_pData);
	return (U8*) (m_pData + offset);
}

template<typename T> 
inline const T& ConstMaterialReference::GetData(S32 offset) const
{
	assert(m_pData);
	assert(offset >= 0 && offset + sizeof(T) <= GetDataSize());
	return (T&)*((U8*) m_pData + offset);
}

inline U32 ConstMaterialReference::GetDataSize() const
{
	return m_material.GetDataSize();
}

inline ConstMaterialReference MaterialListData::GetMaterial(S32 i) const
{
	const U8* p = m_pCache->CacheMemory(m_materials[i].GetData(), m_materials[i].GetDataSize());
	return ConstMaterialReference(m_materials[i], *this, p);
}

inline void MaterialListData::Release(ConstMaterialReference& m) const
{
	m_pCache->Release(m.GetData());
	m.m_pData = 0;
}

#else

inline const Material& MaterialListData::GetMaterial(S32 i) const
{
	return m_materials[i];
}

inline void MaterialListData::Release(ConstMaterialReference m) const
{
}

#endif




