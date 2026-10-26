#ifndef RD_STREAMDATA_H
#define RD_STREAMDATA_H
#ifdef _MSC_VER
#pragma once
#endif

namespace RenderingData
{
	class StreamData;
	namespace Build {class Scene; class StreamData;}
	namespace Target {class Scene; class StreamData;}

	struct ChunkDefs
	{
#if CFG_CONVERTER 
		typedef Btl::VectorParameters_Dynamic<> VectorParameters;
#elif PLATFORM_SPU
		typedef Btl::VectorParameters_FixedSize<> VectorParameters;
#else
		typedef Btl::VectorParameters_FixedSize<Btl::StackArrayAllocator<Btl::NullType> > VectorParameters;
#endif
		typedef Btl::BitVectorTemplate<Btl::BitVectorBase<Btl::Vector<U8, VectorParameters> > > BitVector;
	};

	// A chunk contains resource data for a number of blocks at various lods.
	class ChunkInfo
	{
	public:
		ChunkInfo();

		struct Item
		{
			S32 block; // Index of the block.
			S32 lod; // Index of the lod.
			//S32 size; // Size of the item in the file.
		};

		S32 GetNumItems() const;
		const Item& GetItem(S32 index) const;

		S32 GetNumBlocks() const;
		S32 GetBlockIndex(S32 i) const;

		U64 GetOffset() const;
		U64 GetCompressedSize() const;
		U64 GetOriginalSize() const;

		const Bounds& GetBounds() const;

		FRIEND_SERIALISE(ChunkInfo)
		friend class Build::StreamData;
		friend class Target::StreamData;

	private:
		Bounds m_bounds; // Combined bounds of everything in the chunk.
		Btl::Vector<S32, ChunkDefs::VectorParameters> m_blocks; // List of blocks updated by the chunk.
		Btl::Vector<Item, ChunkDefs::VectorParameters> m_items; // Lods stored in the chunk.
		Btl::Vector<S32, ChunkDefs::VectorParameters> m_prerequisites; // Chunks that must be loaded before this one.
		Btl::Vector<S32, ChunkDefs::VectorParameters> m_dependants; // Chunks that must be unloaded before this one.
		U64 m_offset; // Offset of the chunk in the file.
		U64 m_compressedSize; // Size of the chunk in the file.
		U64 m_originalSize; // Size of the original (uncompressed) chunk.
	};

	bool operator < (const ChunkInfo::Item& a, const ChunkInfo::Item& b);

	class StreamData
	{
	public:
		StreamData();

		FRIEND_SERIALISE(StreamData)

		S32 GetNumChunks() const;
		const ChunkInfo& GetChunkInfo(S32 chunk) const;

		friend class Build::Scene;

	protected:
	#if !PLATFORM_SPU
		AutoStackAllocator m_autoStackAllocator;
	#endif

		Btl::Vector<ChunkInfo, ChunkDefs::VectorParameters> m_chunks;
		Btl::Vector<ChunkDefs::BitVector, ChunkDefs::VectorParameters> m_chunkPvs; // 1 bit per chunk, per pvs zone
		Btl::Vector<Bounds, ChunkDefs::VectorParameters> m_chunkBounds; // Same as bounds in the chunk.
		Btl::Vector<ChunkDefs::BitVector, ChunkDefs::VectorParameters> m_groups; // 1 bit per group, per chunk.
		Btl::Vector<ChunkDefs::BitVector, ChunkDefs::VectorParameters> m_chunkRooms; // 1 bit per chunk, per room.
		S32 m_chunkVersion;
	};

	#include "noninterface/streamdata.inl"
}

#endif


