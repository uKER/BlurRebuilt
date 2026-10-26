#ifndef NETTRANSPORT_SEGMENTPOOL_H
#define NETTRANSPORT_SEGMENTPOOL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "netTransport_Pool.h"
#include "netTransport_Segment.h"

namespace netTransport
{
	BD_REFERENCE(SegmentPool);
	class SegmentPool: public Pool<Segment>, public bdReferencable
	{
	public:
		SegmentPool(int size)
		: Pool<Segment>(size)
		{}
	};
}

#endif //NETTRANSPORT_SEGMENTPOOL_H