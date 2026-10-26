#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include "renderingdata/archive.h"
#include "renderingdata/noninterface/archive.inl"
#include <Framework/application/CoreSystems.h>
#include <System/Serialise/I_FileSystem.h>
#include "system/serialise/C_MemoryStream.h"
#include "System/Debug/I_DebugRender.h"
#include "core/maths/conversion.h"
#include "btl/containers/algorithm.h"
#include "System/Memory/MemoryTracker.h"
#include "renderingdata/debugtextures.h"
#include "world_render/pvs.h"
#include "streamingstatistics.h"

//#pragma optimize ("", off)

static Debug::Logger &g_LogDebug = Debug::GetLogger("graphics.renderingdata.streamer.memory");
static Debug::Logger &g_LogActivity = Debug::GetLogger("graphics.renderingdata.streamer.activity");

Debug::Bool g_streamDataDebug("Graphics.RenderingData.Streaming.Debug.ShowDebug", false);
Debug::Bool g_streamDataOnlyLoaded("Graphics.RenderingData.Streaming.Debug.OnlyShowLoaded", false);
Debug::Int g_showPvsCellsAbove("Graphics.RenderingData.Streaming.Debug.ShowPvsCellsAbove_Mb", 0);
Debug::Int g_chunkDebugIndex("Graphics.RenderingData.Streaming.Debug.ChunkDebugIndex", -1, Debug::Range(-1, 100));
Debug::Int g_chunkDebugIndexOffset("Graphics.RenderingData.Streaming.Debug.ChunkDebugIndexOffset", 0);

EXTERN_DEBUG_FLOAT (g_maxStreamingDistance);

namespace RenderingData {
namespace Target 
{

void StreamData::AddTextures(ResourceBlock::Resources& resources, int chunk)
{
#ifndef CFG_SPEED
	for (int i = 0; i < resources.textures.Size(); i++)
	{
		if (resources.textures[i] != 0)
		{
			const void* p = resources.textures[i]->GetData().GetGPUTexture();

			if (p)
			{
				if (chunk > -1)
					resources.textures[i]->m_headers.Back().chunk = chunk;
				else
					chunk = resources.textures[i]->m_headers.Back().chunk;

				m_textureChunkMap[p] = chunk;
			}
		}
	}
#endif
}

void StreamData::RemoveTextures(const ResourceBlock::Resources& resources)
{
#ifndef CFG_SPEED
	for (int i = 0; i < resources.textures.Size(); i++)
	{
		if (resources.textures[i] != 0)
		{
			const void* p = resources.textures[i]->GetData().GetGPUTexture();
			if (p) m_textureChunkMap.Erase(p);
		}
	}
#endif
}

void StreamData::SetPvsCellSizeDebug()
{
	if (g_showPvsCellsAbove > 0)
	{
		Btl::BitVector v;
		v.Resize(m_chunkPvs.Size());

		if (m_cellSizes.Size() == 0)
		{
			m_cellSizes.Resize(m_chunkPvs.Size());

			for (int cell = 0; cell < m_cellSizes.Size(); cell++)
			{
				m_cellSizes[cell] = 0;

				for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
				{
					if (CellRequiresChunk(cell, chunk))
						m_cellSizes[cell] += m_chunks[chunk].m_originalSize;
				}
			}
		}

		for (int cell = 0; cell < m_cellSizes.Size(); cell++)
		{
			v[cell] = (m_cellSizes[cell] > g_showPvsCellsAbove * 1024 * 1024);
		}

		Pvs::Target::PvsData::SetDebugBits(v);
	}
}

void StreamData::AddDebugBounds(const Bounds& bounds) const
{
	m_debugLock.Enter();
	m_debugBounds.PushBack(bounds);
	m_debugLock.Exit();
}

void StreamData::DisplayDebug()
{
	SetPvsCellSizeDebug();

	if (!g_streamDataDebug)
	{
		if (g_showPvsCellsAbove == 0)
			Pvs::Target::PvsData::SetDebugBits(Btl::BitVector());

		return;
	}

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(m_cameraPosition - Maths::HVec3(1.f, 0.f, 0.f), m_cameraPosition + Maths::HVec3(1.f, 0.f, 0.f), Maths::RGBA(1.f, 0.f, 0.f));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(m_cameraPosition - Maths::HVec3(0.f, 1.f, 0.f), m_cameraPosition + Maths::HVec3(0.f, 1.f, 0.f), Maths::RGBA(1.f, 0.f, 0.f));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(m_cameraPosition - Maths::HVec3(0.f, 0.f, 1.f), m_cameraPosition + Maths::HVec3(0.f, 0.f, 1.f), Maths::RGBA(1.f, 0.f, 0.f));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(m_cameraPosition, m_cameraPosition + m_cameraDirection * (float) g_maxStreamingDistance, Maths::RGBA(1.f, 0.f, 0.f));

	S32 debugChunkIndex = -1;

	// Find the chunk containing the selected texture.
	if (g_chunkDebugIndex > -1)
	{
		debugChunkIndex = g_chunkDebugIndex + g_chunkDebugIndexOffset;
	}
	else if (g_DebugTextures.m_pSelectedTexture)
	{
		TextureChunkMap::Iterator r = m_textureChunkMap.Find(g_DebugTextures.m_pSelectedTexture);

		if (r != m_textureChunkMap.End())
			debugChunkIndex = r->GetValue();
	}

	// Set the pvs bits for the debug render.
	if (debugChunkIndex > -1 && debugChunkIndex < m_chunks.Size())
	{
		Btl::BitVector v;
		v.Resize(m_chunkPvs.Size());

		for (int i = 0; i < m_chunkPvs.Size(); i++)
			v[i] = (bool) m_chunkPvs[i][debugChunkIndex];

		Pvs::Target::PvsData::SetDebugBits(v);
	}
	else
	{
		Pvs::Target::PvsData::SetDebugBits(Btl::BitVector());
	}

	for (int i = 0; i < m_chunks.Size(); i++)
	{
		if (i == debugChunkIndex || debugChunkIndex < 0)
		{
			const Bounds& bb = m_chunks[i].GetBounds();

			if (!g_streamDataOnlyLoaded || m_chunkState[i].loaded)
			{
				Maths::HVec3 min, max;
				Maths::Convert(min, MakeVector(bb.aabbMin));
				Maths::Convert(max, MakeVector(bb.aabbMax));

				Maths::RGBA colour;
				
				if (m_chunkState[i].shouldLoad && m_chunkState[i].loaded)
					colour = Maths::RGBA(1.f, 1.f, 0.f);
				else if (m_chunkState[i].shouldLoad && !m_chunkState[i].loaded)
					colour = Maths::RGBA(0.f, 1.f, 0.f);
				else if (!m_chunkState[i].shouldLoad && m_chunkState[i].loaded)
					colour = Maths::RGBA(1.f, 0.f, 0.f);
				else
					colour = Maths::RGBA(0.f, 0.f, 1.f);

				DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBox(min, max, Maths::HMat43(), colour);
				DbUtil::DefaultDebugRenderSys()->Immediate()->SetStates(false);
			}
		}
	}

	for (int i = 0; i < m_debugBounds.Size(); i++)
	{
		const Bounds& bb = m_debugBounds[i];

		Maths::HVec3 min, max;
		Maths::Convert(min, MakeVector(bb.aabbMin));
		Maths::Convert(max, MakeVector(bb.aabbMax));

		Maths::RGBA colour = Maths::RGBA(1.f, 0.f, 1.f);

		DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dBox(min, max, Maths::HMat43(), colour);
		DbUtil::DefaultDebugRenderSys()->Immediate()->SetStates(false);
	}

	DbUtil::DefaultDebugRenderSys()->Immediate()->SetStates(true);

	for (int i = 0; i < m_chunks.Size(); i++)
	{
		if (i == debugChunkIndex || debugChunkIndex < 0)
		{
			const Bounds& bb = m_chunks[i].GetBounds();

			if (!g_streamDataOnlyLoaded || m_chunkState[i].loaded)
			{
				Maths::HVec3 min, max;
				Maths::Convert(min, MakeVector(bb.aabbMin));
				Maths::Convert(max, MakeVector(bb.aabbMax));

				Util::C_String text;
				text.Printf("%i", i);
				DbUtil::DefaultDebugRenderSys()->Immediate()->Draw3dText((min + max) * 0.5f, text);
			}
		}
	}
}

void StreamData::DebugSizeInfo() const
{
	S32 vertexDataSize = 0;
	S32 indexDataSize = 0;
	S32 textureDataSize = 0;
	S32 activeTextureDataSize = 0;

	S32 nVertexBuffers = 0;
	S32 nIndexBuffers = 0;
	S32 nTextures = 0;

	for (int b = 0; b < m_pScene->GetNumResourceBlocks(); b++)
	{
		const ResourceBlock& block = m_pScene->GetResourceBlock(b);

		nVertexBuffers += block.GetNumVertexBuffers();
		nIndexBuffers += block.GetNumIndexBuffers();
		nTextures += block.GetNumTextures();

		for (int i = 0; i < block.GetNumVertexBuffers(); i++)
		{
			vertexDataSize += block.GetVertexBuffer(i).GetData().GetSize();
		}

		for (int i = 0; i < block.GetNumIndexBuffers(); i++)
		{
			indexDataSize += block.GetIndexBuffer(i).GetData().GetSize();
		}

		for (int i = 0; i < block.GetNumTextures(); i++)
		{
			textureDataSize += block.GetTexture(i).GetData().GetSize();
			activeTextureDataSize += Display::GetActiveDataSize(&block.GetTexture(i).GetData());
		}
	}

	Memory::BlockAllocator::Stats* stats = g_resourceAllocator.GetStats();
	S32 allocMisc = stats->m_iUsedMem;
	S32 totalMisc = stats->m_iTotalMem;
	//S32 allocMisc = g_resourceAllocator.GetTotalAllocated();
	//S32 totalMisc = g_resourceAllocator.GetTotalSize();

	LOGDEBUG(g_LogDebug, "Vertex data: %i k (%i), Index data: %i k (%i), Texture data: %i k (%i, %i k), Misc: %i / %i k", 
								vertexDataSize / 1024, nVertexBuffers, 
								indexDataSize / 1024, nIndexBuffers, 
								textureDataSize / 1024, nTextures, activeTextureDataSize / 1024,
								allocMisc / 1024, totalMisc / 1024);
}

void StreamData::DebugInfo(DebugInfoStats& stats) const
{
	memset(&stats, 0, sizeof(stats));

	for (int chunk = 0; chunk < m_chunks.Size(); chunk++)
	{
		if (m_chunkState[chunk].loaded)
		{
			stats.nChunksLoaded++;
			stats.bytesLoaded += m_chunks[chunk].GetOriginalSize();
		}

		if (m_chunkState[chunk].cached)
		{
			stats.nChunksCached++;
			stats.bytesCached += m_chunks[chunk].GetCompressedSize();
		}

		if (m_chunkState[chunk].shouldLoad)
		{
			stats.nChunksRequested++;
			stats.bytesRequested += m_chunks[chunk].GetOriginalSize();
		}

		if (m_chunkState[chunk].loaded && m_chunkState[chunk].shouldLoad)
		{
			stats.bytesLoadedAndRequested += m_chunks[chunk].GetOriginalSize();
		}

		if (m_debugPVSCell > -1 && m_debugPVSCell < m_chunkPvs.Size())
		{
			if (m_chunkState[chunk].shouldLoad && m_chunkPvs[m_debugPVSCell][chunk])
			{
				stats.bytesRequestedPvs += m_chunks[chunk].GetOriginalSize();
			}
		}

		stats.nChunksInFile++;
		stats.compressedBytesInFile += m_chunks[chunk].GetCompressedSize();
		stats.uncompressedBytesInFile += m_chunks[chunk].GetOriginalSize();
	}
}

void StreamData::DebugInfo() const
{
	if (Debug::GetEffectiveLevel(g_LogDebug) == DebugLevel::Debug)
	{
		DebugInfoStats stats;

		DebugInfo(stats);

		LOGDEBUG(g_LogDebug, "Chunks Loaded: %i / %i", stats.nChunksLoaded, stats.nChunksInFile);
		LOGDEBUG(g_LogDebug, "Kb Requested: %lld (%lld, cell %i) / %lld (%i %%)", stats.bytesRequested / 1024, stats.bytesRequestedPvs / 1024, m_debugPVSCell, stats.uncompressedBytesInFile / 1024, U32((float(stats.bytesRequested) / float(stats.uncompressedBytesInFile)) * 100.f));
		LOGDEBUG(g_LogDebug, "Kb Loaded: %lld / %lld (%i %%)", stats.bytesLoaded / 1024, stats.bytesRequested / 1024, U32((float(stats.bytesLoadedAndRequested) / float(stats.bytesRequested)) * 100.f));
		LOGDEBUG(g_LogDebug, "Kb Cached: %lld / %lld", stats.bytesCached / 1024, stats.compressedBytesInFile / 1024);
		
		DebugSizeInfo();

#if defined(PLATFORM_PS3)
	Display::Device()->GetVideoMemory()->PrintDebug(g_LogDebug);
#endif
	}

	SharedTextureManager::Instance().DumpPotentialTextureSharing();
}

}}
