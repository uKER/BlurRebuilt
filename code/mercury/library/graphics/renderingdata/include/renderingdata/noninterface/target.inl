// IndexBuffer
inline const Display2::C_IndexBuffer& IndexBuffer::GetData() const
{
	return m_data;
}

// VertexBuffer
inline const Display2::C_VertexBuffer& VertexBuffer::GetData() const
{
	return m_data;
}

inline S32 VertexBuffer::GetVertexSize() const
{
	return m_vertexSize;
}

// Texture
inline Texture::Texture()
{
	m_discard = 0;
}

inline const Display::C_Texture2D& Texture::GetData() const
{
	return m_data;
}

inline const char* Texture::GetName() const
{
	return m_name.Chars();
}

inline U32 Texture::GetHashValue() const
{
	return m_hashValue;
}

/*inline const SamplerState& Texture::GetSamplerState() const
{
	return m_samplerState;
}*/

inline S32 Texture::GetSamplerStateIndex() const
{
	return m_samplerStateIndex;
}

inline U32 Texture::Discard() const
{
	return m_discard;
}

// VertexDeclaration
inline const Display2::C_VertexDeclaration& VertexDeclaration::GetData() const
{
	return *m_pData;
}

// MaterialList
inline Material& MaterialList::GetMaterial(S32 i)
{
	return m_materials[i];
}


// MaterialListData
inline Material& MaterialListData::GetMaterial(S32 i)
{
	return m_materials[i];
}

// Effect
inline bool Effect::IsLoaded() const
{
	return (m_pFX != 0);
}

// MaterialData
inline void MaterialData::SetNumMaterialTypes(S32 n)
{
	m_data.types.InitialiseSize(n);
}

inline void MaterialData::SetNumParameters(S32 type, S32 n)
{
	m_data.types[type].parameters.InitialiseSize(n);
}

inline const MaterialParameterData& MaterialData::GetParameter(S32 type, S32 element) const
{
	return m_data.types[type].parameters[element];
}

inline MaterialParameterData& MaterialData::GetParameter(S32 type, S32 element)
{
	return m_data.types[type].parameters[element];
}

inline S32 MaterialData::GetNumParameters(S32 type) const
{
	return m_data.types[type].parameters.Size();
}

inline S32 MaterialData::GetNumTypes() const
{
	return m_data.types.Size();
}

inline S32 MaterialData::GetDataSize(S32 type) const
{
	return m_data.types[type].dataSize;
}

// RenderListNode
inline RenderListNode::RenderListNode()
{
	m_streamerDataTarget.initialised = false;
#if !CFG_SPEED
	m_chunkDebug = -1;
#endif
}

inline NodeType RenderListNode::GetNodeType() const
{
	return NODE_TYPE_RENDER_LIST_PLATFORM;
}

inline bool RenderListNode::IsNode(NodeType t) const
{
	if (t == NODE_TYPE_RENDER_LIST_PLATFORM) return true;
	else return RenderingData::RenderListNode::IsNode(t);
}

inline RenderListNode* RenderListNode::Clone() const
{
#if !PLATFORM_SPU
	return new RenderListNode(*this);
#endif
}

inline S32 RenderListNode::GetNumDynamicMaterialLists() const
{
#if !PLATFORM_SPU
	return m_sceneGraphData.dynamicMaterialLists.Size();
#endif
}

inline const RenderingData::MaterialList& RenderListNode::GetDynamicMaterialList(S32 i) const
{
#if !PLATFORM_SPU
	return m_sceneGraphData.dynamicMaterialLists[i];
#endif
}

inline Scene* RenderListNode::GetScenePtr() const
{
	return (Scene*) Node::GetScenePtr();
}

inline const RenderingData::MaterialListData& RenderListNode::GetConstantMaterialListData() const
{
	return m_constantMaterialData;
}

#if !PLATFORM_SPU
inline const RenderingData::MaterialList& RenderListNode::GetConstantMaterialTypes() const
{
	return m_constantMaterialTypes;
}

inline void RenderListNode::InitialiseDynamicMaterialList(S32 i, const MaterialList* pExternalMaterialList)
{
	m_materialData[i + 1].InitialiseMaterialList(*GetScenePtr(), pExternalMaterialList);
	GetDmaData();
}

inline void RenderListNode::SetChunk(S32 chunk)
{
#if !CFG_SPEED
	m_chunkDebug = chunk;
#endif
}

#endif

