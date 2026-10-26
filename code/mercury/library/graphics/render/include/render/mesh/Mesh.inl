namespace Render
{
namespace Mesh
{

INLINE const S32 C_Mesh::GetVertexDeclarationCount() const
{
	return m_VertexDeclarations.Size();
}
	
INLINE Display::C_VertexDeclaration* C_Mesh::GetVertexDeclaration(const S32 index)
{
	return index >= 0 && index < m_VertexDeclarations.Size() ? m_VertexDeclarations[index] : NULL;
}

INLINE const S32 C_Mesh::GetVertexBufferCount() const
{
	return m_VertexBuffers.Size();
}

INLINE Display::C_VertexBuffer* C_Mesh::GetVertexBuffer(const S32 index)
{
	return index >= 0 && index < m_VertexBuffers.Size() ? &m_VertexBuffers[index] : NULL;
}

INLINE const S32 C_Mesh::GetIndexBufferCount() const
{
	return m_IndexBuffers.Size();
}

INLINE Display::C_IndexBuffer* C_Mesh::GetIndexBuffer(const S32 index)
{
	return index >= 0 && index < m_IndexBuffers.Size() ? &m_IndexBuffers[index] : NULL;
}

INLINE const S32 C_Mesh::GetMaterialCount() const
{
	return m_Materials.Size();
}

INLINE C_Material* C_Mesh::GetMaterial(const S32 index)
{
	return index >= 0 && index < m_Materials.Size() ? &m_Materials[index] : NULL;
}

INLINE const S32 C_Mesh::GetBatchCount() const
{
	return m_Batches.Size();
}

INLINE C_Batch* C_Mesh::GetBatch(const S32 index)
{
	return index >= 0 && index < m_Batches.Size() ? &m_Batches[index] : NULL;
}

INLINE const S32 C_Mesh::GetInstanceCount() const
{
	return m_Instances.Size();
}

INLINE C_Instance* C_Mesh::GetInstance(const S32 index)
{
	return index >= 0 && index < m_Instances.Size() ? m_Instances[index] : NULL;
}

}; // namespace Mesh
}; // namespace Render
