#ifndef NETTRANSPORT_TRAFFICANALYSER_H
#define NETTRANSPORT_TRAFFICANALYSER_H

#ifdef _MSC_VER
#pragma once
#endif


#include "netTime/netTime.h"
#include "netTransport/netTransport_TrafficLogger.h"

#include <system/debug/I_DebugRender.h>
#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>

class netPacketHeader;
BD_REFERENCE(bdAddrHandle);

namespace netTransport
{
	class Area
	{
	public:
		Area();
		void setRectangle(const Maths::HVec2& centre, const Maths::HVec2& size);
		void setBounds(const Maths::HVec2& min, const Maths::HVec2& max);
		bool valid() const;
		void extendTo(const Maths::HVec2& v);
		void extendTo(const Area& area);
		void invalidate();
		void expand(const Maths::HVec2& border);

		
		Maths::HVec2   centre() const { return (m_max + m_min) * 0.5f; }
		Maths::HVec2 halfsize() const { return (m_max - m_min) * 0.5f; }
		Maths::HVec2     size() const { return (m_max - m_min); }
		Maths::HVec2      min() const { return m_min; }
		Maths::HVec2      max() const { return m_max; }

		Maths::HVec2 m_min;
		Maths::HVec2 m_max;
	};

	BD_REFERENCE(TrafficLog);
	class TrafficLog: public bdReferencable
	{
	public:
		TrafficLog(const char* name);
		void clear();
		void record(TrafficSample& report);
		void update();
		float getBandwidth() const; // bandwidth usage in KBytes / second.
		float getCap() const;	
		void render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float width, float height);

	private:
		TrafficSample* report(int ptr);
		void calculatePeak();
		void calculateAverage();
		
		void renderChart	(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h);
		void renderPie		(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float size, float h);
		void renderFlow		(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float width, float height);
		void renderPortion	(DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA colour, const Maths::HVec2& cen, float rad, float a, float da);
		void renderBar		(DbUtil::I_DebugRenderPtr debugRenderPtr, const TrafficSample* report, float maxRate, float x, float y, float barwidth, float barheight);
	
		const char* m_name;
		const char* categoryName(int i) const;
		unsigned int categoryColour(int i) const;
		
		enum { MAX_REPORTS = 256 };
		enum { FLOW_TIME_SPAN = 6 * 1000 * 1000 };
		TrafficSample	m_reports[MAX_REPORTS];
		int				m_reportsPtr;
		
		TrafficSample	m_average;
		TrafficSample	m_peak;

		// render bounding boxes of various bits
		Area m_pieArea;
		Area m_flowArea;
		Area m_chartArea;
		Area m_area;
	};
};

#endif //NETTRANSPORT_TRAFFICANALYSER_H