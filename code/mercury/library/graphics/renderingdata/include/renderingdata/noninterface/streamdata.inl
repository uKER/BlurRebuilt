// ChunkInfo
inline ChunkInfo::ChunkInfo() : m_offset(0), m_compressedSize(0), m_originalSize(0)
{
}

inline S32 ChunkInfo::GetNumItems() const
{
	return m_items.Size();
}

inline const ChunkInfo::Item& ChunkInfo::GetItem(S32 index) const
{
	return m_items[index];
}

inline S32 ChunkInfo::GetNumBlocks() const
{
	return m_blocks.Size();
}

inline S32 ChunkInfo::GetBlockIndex(S32 i) const
{
	return m_blocks[i];
}

inline U64 ChunkInfo::GetOffset() const
{
	return m_offset;
}

inline U64 ChunkInfo::GetCompressedSize() const
{
	return m_compressedSize;
}

inline U64 ChunkInfo::GetOriginalSize() const
{
	return m_originalSize;
}

inline const Bounds& ChunkInfo::GetBounds() const
{
	return m_bounds;
}

////////////////////////////////////////////////////////

inline bool operator < (const ChunkInfo::Item& a, const ChunkInfo::Item& b)
{
	if  (a.lod < b.lod) return true;
	else if (b.lod < a.lod) return false;
	if  (a.block < b.block) return true;
	else if (b.block < a.block) return false;
	return false;
}

/////////////////////////////////////////////////////////

inline StreamData::StreamData() 
{
	m_chunkVersion = 1;
}

inline S32 StreamData::GetNumChunks() const
{
	return m_chunks.Size();
}

inline const ChunkInfo& StreamData::GetChunkInfo(S32 chunk) const
{
	return m_chunks[chunk];
}


