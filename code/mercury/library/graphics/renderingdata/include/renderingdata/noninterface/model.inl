// Model 
inline Model::Model() : m_baseMaterialIndex(-1), m_instanceMaterialIndex(-1)
{
}

inline RenderObjectType Model::GetObjectType() const
{
	return RENDER_OBJECT_MODEL;
}

inline Model* Model::Clone() const
{
	return new Model(*this);
}

inline S32 Model::GetNumPrimitives() const
{
	return m_primitives.Size();
}

inline const Primitive& Model::GetPrimitive(S32 i) const
{
	return m_primitives[i];
}

inline S32 Model::GetNumStreams() const
{
	return m_streams.Size();
}

inline const ModelStream& Model::GetModelStream(S32 i) const
{
	return m_streams[i];
}

inline const Model* Model::GetPointer(const RenderObject* p)
{
	if (p && p->GetObjectType() == RENDER_OBJECT_MODEL)
		return (const Model*) p;
	else
		return 0;
}

inline Model* Model::GetPointer(RenderObject* p)
{
	if (p && p->GetObjectType() == RENDER_OBJECT_MODEL)
		return (Model*) p;
	else
		return 0;
}

inline void Model::SetBaseMaterialIndex(S32 i)
{
	m_baseMaterialIndex = i;
}

inline S32 Model::GetBaseMaterialIndex() const
{
	return m_baseMaterialIndex;
}

inline void Model::SetInstanceMaterialIndex(S32 i)
{
	m_instanceMaterialIndex = i;
}

inline S32 Model::GetInstanceMaterialIndex() const
{
	return m_instanceMaterialIndex;
}

// ModelStream
inline ModelStream::ModelStream()
{
}

inline S32 ModelStream::GetNumPrimitives() const
{
	return m_primitives.Size();
}

inline const PrimitiveStream& ModelStream::GetPrimitive(S32 i) const
{
	return m_primitives[i];
}

// Instance
inline Instance::Instance() : m_baseModelIndex(-1), m_materialIndex(-1), m_flags(0)
{
}

inline RenderObjectType Instance::GetObjectType() const
{
	return RENDER_OBJECT_INSTANCE;
}

inline Instance* Instance::Clone() const
{
	return new Instance(*this);
}

inline S32 Instance::GetBaseModelIndex() const
{
	return m_baseModelIndex;
}

inline S32 Instance::GetMaterialIndex() const
{
	return m_materialIndex;
}

inline S32 Instance::GetNumStreams() const
{
	return m_streams.Size();
}

inline const ModelStream& Instance::GetModelStream(S32 i) const
{
	return m_streams[i];
}

inline void Instance::SetBaseModelIndex(S32 i)
{
	m_baseModelIndex = i;
}

inline void Instance::SetMaterialIndex(S32 i)
{
	m_materialIndex = i;
}

inline U32 Instance::GetFlags() const
{
	return m_flags;
}

inline void Instance::SetFlags(U32 flags)
{
	m_flags = flags; 
}

// Instance Batch
inline RenderObjectType InstanceBatch::GetObjectType() const
{
	return RENDER_OBJECT_INSTANCE_BATCH;
}

inline InstanceBatch* InstanceBatch::Clone() const
{
	return new InstanceBatch(*this);
}
