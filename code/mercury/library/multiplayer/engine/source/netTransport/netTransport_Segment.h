#ifndef NETTRANSPORT_SEGMENT_H
#define NETTRANSPORT_SEGMENT_H

#ifdef _MSC_VER
#pragma once
#endif


namespace netTransport
{
	enum { SEGMENT_SIZE = 1024 };
		
	class Segment
	{
	public:
		Segment()
		{
			m_next = NULL;
		}

		char m_buffer[SEGMENT_SIZE];
		Segment* m_next;
	};
};

#endif //NETTRANSPORT_SEGMENT_H