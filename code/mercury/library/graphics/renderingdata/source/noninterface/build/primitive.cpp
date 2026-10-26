#include "stdafx.h"
#include "renderingdata/primitive.h"

//#pragma optimize("", off)

namespace RenderingData
{

void PrimitiveInfo::GenerateSortCode( const U8 decalOrder )
{
	//assert(base.effect >= 0 && base.effect <= 0xffff);
	//assert(base.resourceBlockIndex >= 0 && base.resourceBlockIndex <= 0xffff);
	//assert(streams[0].materialIndex >= 0 && streams[0].materialIndex < 0xffff);
	//base.sortCode = (U64(decalOrder) << 48) + (U64(base.effect) << 32) + (base.resourceBlockIndex << 16) + streams[0].materialIndex;
	base.sortCode = decalOrder;
}

void PrimitiveInfo::GenerateSortCode( const U8 decalOrder, S32 iModelMtlIdx)
{
	//assert(base.effect >= 0 && base.effect <= 0xffff);
	//assert(iModelMtlIdx >= 0 && iModelMtlIdx <= 0xffff);
	//assert(streams[0].materialIndex >= 0 && streams[0].materialIndex < 0xffff);
	//base.sortCode = (U64(decalOrder) << 48) + (U64(base.effect) << 32) + (iModelMtlIdx << 16) + streams[0].materialIndex;
	base.sortCode = decalOrder;
}


void PrimitiveInfo::CalculateBoundsFromVertices()
{
	base.bounds.SetAABBEmpty();

	for (int stream = 0; stream < streams.Size(); stream++)
	{
		const VertexFormat& vertexFormat = streams[stream].vertices[0].GetVertexFormat();
		
		for (int e = 0; e < vertexFormat.GetNumVertexElements(); e++)
		{
			if (vertexFormat.GetVertexElement(e).usage == VERTEX_ELEMENT_USAGE_POSITION)
			{
				if (vertexFormat.GetVertexElement(e).GetNumF32s() == 3)
				{
					for (int v = 0; v < streams[stream].vertices.Size(); v++)
					{
						const Point3& p = streams[stream].vertices[v].GetData<Point3>(e);
						base.bounds.SetUnion(p);
					}
				}
			}
		}
	}
}

void PrimitiveInfo::RemapUVs(float maxUV)
{
	for (int stream = 0; stream < streams.Size(); stream++)
	{
		const VertexFormat& vertexFormat = streams[stream].vertices[0].GetVertexFormat();
		
		for (int e = 0; e < vertexFormat.GetNumVertexElements(); e++)
		{
			if (vertexFormat.GetVertexElement(e).usage == VERTEX_ELEMENT_USAGE_TEXCOORD)
			{
				for (int f = 0; f < vertexFormat.GetVertexElement(e).GetNumF32s(); f++)
				{
					float min = FLT_MAX;
					float max = -FLT_MAX;

					for (int v = 0; v < streams[stream].vertices.Size(); v++)
					{
						const U8* p = &streams[stream].vertices[v].GetData<U8>(e) + vertexFormat.GetVertexElement(e).GetF32Offset(f);
						float t = *(float*) p;
						min = Maths::Min(min, t); 
						max = Maths::Max(max, t); 
					}

					S32 n = 0;

					if (min > 0.f)
						n = S32(min / maxUV) + 1;
					else if (max < 0.f)
						n = S32(max / maxUV) - 1;

					if (n != 0)
					{
						for (int v = 0; v < streams[stream].vertices.Size(); v++)
						{
							const U8* p = &streams[stream].vertices[v].GetData<U8>(e) + vertexFormat.GetVertexElement(e).GetF32Offset(f);
							float* t = (float*) p;
							*t = *t - n * maxUV;
						}
					}
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////

bool operator == (const BatchIDGenerator::BaseInfo& t1, const BatchIDGenerator::BaseInfo& t2)
{
	if (t1.primitiveType != t2.primitiveType) return false;
	if (t1.resourceBlockIndex != t2.resourceBlockIndex) return false;
	if (t1.effect != t2.effect) return false;
	if (t1.sortCode != t2.sortCode) return false;
	if (t1.flags != t2.flags) return false;
	if (t1.sortType != t2.sortType) return false;
	if (t1.blendState != t2.blendState) return false;
	return true;
}

bool operator == (const BatchIDGenerator::StreamInfo& t1, const BatchIDGenerator::StreamInfo& t2)
{
	if (t1.resourceBlockIndex != t2.resourceBlockIndex) return false;
	if (t1.streamIndex != t2.streamIndex) return false;
	if (t1.materialIndex != t2.materialIndex) return false;
	if (t1.vertexFormat != t2.vertexFormat) return false;
	return true;
}

U32 Hash(const BatchIDGenerator::BaseInfo& t)
{
	U32 hash = 0;
	hash = Btl::Hash(t.resourceBlockIndex, hash);
	hash = Btl::Hash(t.effect, hash);
	hash = Btl::Hash(t.sortCode, hash);
	hash = Btl::Hash(t.flags, hash);
	hash = Btl::Hash(t.sortType, hash);
	hash = Hash(t.blendState, hash);
	return hash;
}

U32 Hash(const BatchIDGenerator::StreamInfo& t)
{
	U32 hash = 0;
	hash = Btl::Hash(t.resourceBlockIndex, hash);
	hash = Btl::Hash(t.streamIndex, hash);
	hash = Btl::Hash(t.materialIndex, hash);
	return hash;
}

BatchIDGenerator::BatchIDGenerator() : m_uniqueIndexCount(0)
{
}

S32 BatchIDGenerator::GetBatchID(const PrimitiveInfo& info, S32 baseModelID, S32 previousIndex)
{
	S32 index = previousIndex;

	if (previousIndex == -1)
	{
		BaseInfo baseInfo;
		baseInfo.effect = info.base.effect;
		baseInfo.flags = info.base.flags;
		baseInfo.primitiveType = info.primitiveType;
		baseInfo.sortCode = info.base.sortCode;
		baseInfo.sortType = info.base.sortType;
		baseInfo.resourceBlockIndex = info.base.resourceBlockIndex;
		baseInfo.blendState = info.base.blendState;

		index = GetBatchID(baseInfo, baseModelID);
	}

	for (int i = 0; i < info.streams.Size(); i++)
	{
		StreamInfo streamInfo;
		streamInfo.resourceBlockIndex = info.streams[i].resourceBlockIndex;
		streamInfo.materialIndex = info.streams[i].materialIndex;
		streamInfo.streamIndex = info.streams[i].streamIndex;
		streamInfo.vertexFormat = info.streams[i].vertices.GetVertexFormat();

		index = GetBatchID(streamInfo, index);
	}

	if (index > 10000)
		int breakhere = 0;

	return index;
}

S32 BatchIDGenerator::GetBatchID(const BaseInfo& base, S32 baseModelID)
{
	BaseMap::InsertionReturnType result = m_baseMap.Insert(Btl::MakePair(base, baseModelID));

	if (result.inserted)
		result.iterator->GetValue() = m_uniqueIndexCount++;

	return result.iterator->GetValue();
}

S32 BatchIDGenerator::GetBatchID(const StreamInfo& stream, S32 previousIndex)
{
	StreamMap::InsertionReturnType result = m_streamMap.Insert(Btl::MakePair(stream, previousIndex));

	if (result.inserted)
		result.iterator->GetValue() = m_uniqueIndexCount++;

	return result.iterator->GetValue();
}

}