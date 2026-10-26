#include "stdafx.h"
#include "renderingdata/renderlist.h"
#include "btl/containers/rlebitvector.h"

//#pragma optimize("",off)

namespace RenderingData
{

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");

/// Calculate bit vectors for primitives for a specified room bit vector.
void RenderListNode::CalculateVisibilityFromRoomBitVector(const Btl::BitVector& rkRoomSet) const
{
	if(!rkRoomSet.Size())
		return;	// No portal rooms visible.

	m_sceneGraphData.portalVisiblePrimitives.Resize(m_renderData.primitives.Size());
	m_sceneGraphData.portalVisibleInstances.Resize(m_renderData.instances.Size());
	m_sceneGraphData.portalVisiblePrimitives.ClearBits();
	m_sceneGraphData.portalVisibleInstances.ClearBits();
	
	for(U32 iCurrentRoom=0; iCurrentRoom<static_cast<U32>(m_sceneGraphData.vRoomIndices.Size()); ++iCurrentRoom)
	{
		const U32 iLookedUpVal = m_sceneGraphData.vRoomIndices[iCurrentRoom];

		if(rkRoomSet.GetBitValue(iLookedUpVal))
		{
			m_sceneGraphData.portalVisiblePrimitives |= m_sceneGraphData.vMap[iCurrentRoom].m_vPrimitives;
			m_sceneGraphData.portalVisibleInstances  |= m_sceneGraphData.vMap[iCurrentRoom].m_vInstances;
		}
	}
	return;
}

#if CFG_CONVERTER

void RenderListNode::PvsCellData::Build()
{
	if (!bits.Empty() && data.Empty())
	{
		nCells = bits.Size();
		cellSize = Btl::AlignUp((U32)bits[0].Size(), 8) / 8;

		if (1) // new version
		{
			offsets.Resize(nCells);
			S32 count = 0;
			S32 rleSize = 0;

			typedef Btl::Map<Btl::BitVector, S16> Map;
			Map map;

			for (int i = 0; i < nCells; i++)
			{
				Map::InsertionReturnType r = map.Insert(bits[i]);

				if (r.inserted)
				{
					S32 offset = cellSize * count;
					r.iterator->GetValue() = count;
					data.Resize(data.Size() + cellSize);
					U8* p = &data[offset];
					memcpy(p, bits[i].Data(), cellSize);
					count++;
				}

				offsets[i] = r.iterator->GetValue();

				//Btl::RleBitVector rle;
				//rle.Init(bits[i].Data(), bits[i].Size());
				//rleSize += rle.SizeInBytes();
			}

			//LOGDEBUG(g_Log, "RenderListNode PVS data: original size %i bytes, shared %i bytes, rle %i bytes.", nCells * cellSize, count * cellSize + nCells * 2, rleSize + nCells * 4);
		}
		else // old version
		{
			data.Resize(nCells * cellSize);

			for (int i = 0; i < nCells; i++)
			{
				U8* p = &data[i * cellSize];
				memcpy(p, bits[i].Data(), cellSize);
			}
		}
	}
}

#endif

}
