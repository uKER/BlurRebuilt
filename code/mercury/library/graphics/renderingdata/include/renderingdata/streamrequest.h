#ifndef RD_STREAMREQUEST_H
#define RD_STREAMREQUEST_H
#ifdef _MSC_VER
#pragma once
#endif

#include "Display/View/ViewFrustum.h"

namespace RenderingData {
namespace Target
{
	class RequestInfo
	{
	public:
		RequestInfo() : drawDistance(FLT_MAX), pvsCell(-1), groupMask(0), fovY(1.f), aspect(1.f), resolution(1.f) {}

		Maths::HMat44 cameraToWorld; // directional only
		Maths::HVec3 cameraPosition; // non-directional only
		float drawDistance;
		S32 pvsCell;
		U32 groupMask;
		float fovY; // directional only
		float aspect; // directional only
		float resolution;
		bool directional;
		bool mainView;
		Btl::Vector<U16> visibleRooms;
	};

	class ALIGN16 StreamerRequestWorker
	{
	public:
		class RequestInfo : public RenderingData::Target::RequestInfo
		{
		public:
			mutable Display::C_ViewFrustum view;
			mutable const U8* pvsData;
			mutable const U8* roomData;
		};

		const RequestInfo* m_pRequests;
		S32 m_nRequests;
		const Bounds* m_pChunkBounds;
		S32 m_nChunks;
		bool* m_pResults;
		S32 m_nResults;

		void Process() const;

#if !PLATFORM_SPU
		Btl::Vector<RequestInfo> m_requests;
		Btl::Vector<Btl::BitVector> m_roomData;
#endif
	};
}}

#endif