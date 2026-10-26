#include "stdafx.h"
#include "renderingdata/target.h"
#include "renderingdata/streamrequest.h"
#include "core/maths/conversion.h"
#include <system/scheduler/jobqueue.h>

//#pragma optimize ("", off)

DECLARE_DEBUG_BOOL(g_useStreamerPvs,"Graphics.RenderingData.Streaming.Requests.UsePvs", true);
DECLARE_DEBUG_BOOL(g_useStreamerPortals,"Graphics.RenderingData.Streaming.Requests.UsePortals", false);
DECLARE_DEBUG_FLOAT(g_maxStreamingDistance,"Graphics.RenderingData.Streaming.Requests.MaxDistance", 10000.f, Debug::Range(0.f, 10000.f));
DECLARE_DEBUG_FLOAT(g_streamerResolutionScale,"Graphics.RenderingData.Streaming.Requests.ResolutionScale", 1.f, Debug::Range(0.f, 1.f));
DECLARE_DEBUG_FLOAT(g_streamerNearClip,"Graphics.RenderingData.Streaming.Requests.StreamerNearClip", 1.f);

namespace Sys {class I_File;}

namespace RenderingData {
namespace Target 
{

bool BoxContains(const Maths::HVec3& p, const Maths::HVec3& min, const Maths::HVec3& max)
{
	Maths::Vector3_Bool b1 = p > min;
	Maths::Vector3_Bool b2 = p < max;
	Maths::Vector3_Bool r = (b1 & b2);
	return (X(r) & Y(r) & Z(r));
}

void ProcessRequest(const StreamerRequestWorker::RequestInfo* pRequest, S32 nRequests,
					const Bounds* pChunkBounds, S32 nChunks,
					bool* pResult)
{
	for (int ir = 0; ir < nRequests; ir++)
	{
		const StreamerRequestWorker::RequestInfo& request(pRequest[ir]);

		if (request.pvsData)
			request.pvsData = (U8*) Spu::FetchAllocAlign(request.pvsData, ((nChunks - 1) >> 3) + 1); 

		if (request.roomData)
			request.roomData = (U8*) Spu::FetchAllocAlign(request.roomData, ((nChunks - 1) >> 3) + 1); 
	}

	for (int ic = 0; ic < nChunks; ic++)
	{
		for (int ir = 0; ir < nRequests; ir++)
		{
			const StreamerRequestWorker::RequestInfo& request(pRequest[ir]);

			if (g_useStreamerPvs && request.pvsData)
			{
				U8 pvsBits = request.pvsData[ic >> 3];
					
				if (pvsBits == 0 || (pvsBits != 255 && !((pvsBits >> (ic % 8)) & 1)))
					continue;
			}

			if (g_useStreamerPortals && request.roomData)
			{
				U8 roomBits = request.roomData[ic >> 3];
					
				if (roomBits == 0 || (roomBits != 255 && !((roomBits >> (ic % 8)) & 1)))
					continue;
			}

			const Bounds& bb = pChunkBounds[ic];

			Maths::HVec3 minimum = Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMin);
			Maths::HVec3 maximum = Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&bb.aabbMax);
			Maths::HF32 dmax = Maths::HF32(bb.distanceMax) * g_streamerResolutionScale;
			Maths::HF32 d = Maths::Min(g_maxStreamingDistance, dmax * (request.resolution / (request.fovY * request.aspect)));
			Maths::HVec3 d3(d, d, d);

			if (d > g_streamerNearClip)
			{
				if (!request.directional)
				{
					if (BoxContains(request.cameraPosition, minimum - d3, maximum + d3))
					{
						pResult[ic] = true;
						break;
					}
				}
				else
				{
					if (BoxContains(Maths::HVec3(request.cameraToWorld.AW()), minimum - d3, maximum + d3))
					{
						Maths::HVec3 centre = (minimum + maximum) * Maths::Constants<Maths::HF32>::Half();
						Maths::HVec3 size = maximum - centre;
						Maths::HVec4 centreW(centre);
						centreW.W(Maths::Constants<Maths::HF32>::One());

						Maths::HVec4 clip(	bb.distanceMin, // near
											Maths::Min(d, Maths::HF32(request.drawDistance)), // far
											Maths::Constants<Maths::HF32>::One(), 
											Maths::Constants<Maths::HF32>::One());

						if (request.view.CullAxisBox(centreW, Maths::HVec4(size), clip))
						{
							pResult[ic] = true;
							break;
						}
					}
				}
			}
		}
	}
}

void StreamerRequestWorker::Process() const
{
	PERF_EVENT("StreamerRequestWorker");

	const RequestInfo* pRequests = (const RequestInfo*)Spu::FetchAllocAsync(m_pRequests, sizeof(RequestInfo) * m_nRequests, 0);
	const Bounds* pChunkBounds = (const Bounds*)Spu::FetchAllocAsync(m_pChunkBounds, sizeof(Bounds) * m_nChunks, 0);
	bool* pResults = (bool*)Spu::FetchAllocAsync(m_pResults, sizeof(bool) * m_nResults, 0);
	Spu::Wait(0);

	ProcessRequest(pRequests, m_nRequests, pChunkBounds, m_nChunks, pResults);

	Spu::Store(m_pResults, pResults, sizeof(bool) * m_nResults);
}

}}

void StreamerRequestTask(Scheduler::Task* pTask)
{
	const RenderingData::Target::StreamerRequestWorker* p = (const RenderingData::Target::StreamerRequestWorker*) Spu::FetchAllocAsync(*(void**)pTask->ParamData, sizeof(RenderingData::Target::StreamerRequestWorker), 0);	// [Blur2PC][x64] a pointer-sized parameter
	Spu::Wait(0);
	p->Process();
}

//JQ_DECLARE_PPU_MODULE(StreamerRequestModule, StreamerRequestTask)
JQ_DECLARE_SPU_MODULE(StreamerRequestModule, StreamerRequestTask, streamrequestworker, "Graphics.RenderingData.StreamerRequest");
