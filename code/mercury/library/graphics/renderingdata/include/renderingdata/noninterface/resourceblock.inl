// resource index

inline ResourceIndex::ResourceIndex() : m_block(-1), m_index(-1) 
{
}

inline ResourceIndex::ResourceIndex(S32 b, S32 i) : m_block(b), m_index(i) 
{
	assert(i > -1);
}

inline void ResourceIndex::SetBlockIndex(S32 b, S32 i) 
{
	assert(i > -1);
	m_block = b;
	m_index = i;
}

inline S32 ResourceIndex::GetBlock() const 
{
	return m_block;
}

inline S32 ResourceIndex::GetIndex() const 
{
	return m_index;
}

inline ResourceIndex ResourceIndex::Null()
{
	return ResourceIndex();
}

inline bool ResourceIndex::IsNull() const
{
	return m_index == -1;
}

inline bool ResourceIndex::IsShared() const
{
	return m_block == -1;
}

inline ResourceIndex ResourceIndex::BlockIndex(S32 b, S32 i)
{
	return ResourceIndex(b, i);
}

inline bool operator == (const ResourceIndex& r1, const ResourceIndex& r2)
{
	return r1.GetBlock() == r2.GetBlock() 
			&& r1.GetIndex() == r2.GetIndex();
}

inline bool operator != (const ResourceIndex& r1, const ResourceIndex& r2)
{
	return r1.GetBlock() != r2.GetBlock() 
			|| r1.GetIndex() != r2.GetIndex();
}

inline bool operator < (const ResourceIndex& r1, const ResourceIndex& r2)
{
	if  (r1.GetBlock() < r2.GetBlock()) return true;
	if  (r2.GetBlock() < r1.GetBlock()) return false;
	
	return r1.GetIndex() < r2.GetIndex();
}

inline U32 Hash(const RenderingData::ResourceIndex& r)
{
	U32 hash = Btl::Hash(r.GetBlock());
	return Btl::Hash(r.GetIndex(), hash);
}



