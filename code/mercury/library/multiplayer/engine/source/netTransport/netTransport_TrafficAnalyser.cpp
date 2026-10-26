

#include "stdafx.h"
#include "netTransport_TrafficAnalyser.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

namespace netTransport
{
	Area::Area()
	{
		invalidate();
	}
	
	void Area::setRectangle(const Maths::HVec2& centre, const Maths::HVec2& size)
	{
		m_min = centre - size * 0.5f;
		m_max = centre + size * 0.5f;
		assert(valid());
	}
	void Area::setBounds(const Maths::HVec2& min, const Maths::HVec2& max)
	{
		m_min = min;
		m_max = max;
		assert(valid());
	}

	void Area::extendTo(const Area& area)
	{
		if(!area.valid()) return;
		extendTo(area.m_min);
		extendTo(area.m_max);
	}

	void Area::invalidate()
	{
		m_min = Maths::HVec2(0.0f, 0.0f);
		m_max = Maths::HVec2(-1.0f, -1.0f);
	}
		
	bool Area::valid() const 
	{ 
		return	(m_min.X() <= m_max.X()) && 
				(m_min.Y() <= m_max.Y()); 
	}

	void Area::extendTo(const Maths::HVec2& v)
	{
		if(!valid())
			m_min = m_max = v;

		if(v.X() < m_min.X()) m_min.X(v.X());
		if(v.X() > m_max.X()) m_max.X(v.X());
		if(v.Y() < m_min.Y()) m_min.Y(v.Y());
		if(v.Y() > m_max.Y()) m_max.Y(v.Y());
		assert(valid());
	}

	void Area::expand(const Maths::HVec2& border)
	{
		if(!valid())
			return;
		
		Maths::HVec2 c = centre();
		Maths::HVec2 s = size();
		setRectangle(c, s + border);		
	}

	TrafficLog::TrafficLog(const char* name)
	: m_name(name)
	{
		m_reportsPtr = 0;
	}

	void TrafficLog::clear()
	{
		m_average = TrafficSample();
		m_peak = TrafficSample();

		for(int i = 0; i < MAX_REPORTS; i ++)
		{
			m_reports[i] = TrafficSample();
		}		
		m_reportsPtr = 0;
	}

	void TrafficLog::update()
	{
		calculatePeak();
		calculateAverage();
	}

	void TrafficLog::record(TrafficSample& report)
	{
		m_reports[(m_reportsPtr % MAX_REPORTS)] = report;
		m_reportsPtr++;
	}

	void TrafficLog::calculatePeak()
	{
		m_peak = TrafficSample();

		netTime end = netClock::instance()->now();
		netTime start = (end < FLOW_TIME_SPAN)? 0 : end - FLOW_TIME_SPAN;

		int peakIndex = 0;

		for(int i = 0; i < MAX_REPORTS; i ++)
		{
			if(m_reports[i].m_bytes[TRAFFIC_TYPE_SOCKET] > m_peak.m_bytes[TRAFFIC_TYPE_SOCKET])
			{
				if(m_reports[i].m_timeEnd > start)
				{
					peakIndex = i;
				}
			}
			m_peak.peak(m_reports[i]);
		}
		m_peak.m_timeStart = m_reports[peakIndex].m_timeStart;
		m_peak.m_timeEnd = m_reports[peakIndex].m_timeEnd;
	}
	
	void TrafficLog::calculateAverage()
	{
		m_average = TrafficSample();

		int count = (m_reportsPtr > MAX_REPORTS)? MAX_REPORTS : m_reportsPtr;

		if(count <= 0) return;

		for(int i = 0; i  < count; i ++)
		{
			m_average += m_reports[i];
		}
		m_average /= count;
	}

	TrafficSample* TrafficLog::report(int ptr)
	{
		if(ptr < 0 || ptr > m_reportsPtr)
			return NULL;

		if(ptr <= m_reportsPtr - MAX_REPORTS)
			return NULL;
		
		int index = (ptr % MAX_REPORTS);

		return &(m_reports[index]);
	}
	
	void TrafficLog::render(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float w, float h)
	{
		float minx = x;
		float miny = y;		
		float midx = x + w * 0.5f;
		float midy = y + h * 0.35f;
		float maxx = x + w;
		float maxy = y + h;

		m_area.invalidate();
		m_area.extendTo(Maths::HVec2(x, y));

		renderChart(debugRenderPtr, minx, miny, (midx - minx), (maxy - miny));
		m_area.extendTo(m_chartArea);

		renderPie(debugRenderPtr, midx, miny, (maxx - midx), (midy - miny));
		m_area.extendTo(m_pieArea);

		renderFlow(debugRenderPtr, midx, midy, (maxx - midx), (maxy - midy));
		m_area.extendTo(m_flowArea);
	}

	const char* TrafficLog::categoryName(int i) const
	{
		// TO DO : user defined setup
		static const char* categories[] = 
		{
			"total",
			"forwarded",
			"header",
			"system",
			"reliable",
			"voice",
			"game",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
			"",
		};

		if(i < 0 || i >= TRAFFIC_TYPE_COUNT)
			return "";
		return categories[i];
	}

	unsigned int TrafficLog::categoryColour(int i) const
	{
		// colours
		static const unsigned int colours[] = 
		{
			0xffffffff, // "total",
			0xffcc0000,	// "forwarded",
			0xffffcc00, // "header",
			0xff9933cc, // "system",
			0xff6699ff, // "reliable",
			0xffccffcc, // "voice",
			0xff66ff66, // "game",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
			0x00000000, // "unused",
		};
		if(i < 0 || i >= TRAFFIC_TYPE_COUNT)
			return 0;
		return colours[i];
	}

	float TrafficLog::getCap() const
	{
		float capRate = m_average.limit();
		return capRate;
	}

	float TrafficLog::getBandwidth() const
	{
		float totalRate = (float)(m_average.m_bytes[TRAFFIC_TYPE_SOCKET]);
		return totalRate;
	}

	void TrafficLog::renderChart(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float width, float height)
	{
		float h = (height / 10.0f);
		if (h < 16.0f) h = 16.0f;
		if (h > 24.0f) h = 24.0f;
		float w = h;
	
		const float columns[] = { 6 * h, 3 * h, 3 * h, 3 * h, 3 * h, 3 * h };
		
		Maths::HVec2 min(x, y);
		Maths::HVec2 max;

		int c = 0;
		float cx = x;
		char str[128];
		Maths::RGBA white(0xffffffff);
	
		float totalRate = (float)(m_average.m_bytes[TRAFFIC_TYPE_SOCKET]) / 1024.0f;
		
		sprintf_s(str, sizeof(str), "%s [%.2f kB/S]", m_name, totalRate);
		debugRenderPtr->Draw2dText(Maths::HVec2(x, y), str, white, h);
		y += h;
		sprintf_s(str, sizeof(str), "cat.");
		debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
		cx += columns[c]; c++;

		sprintf_s(str, sizeof(str), "load");
		debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
		cx += columns[c]; c++;

		sprintf_s(str, sizeof(str), "kB/S");
		debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
		cx += columns[c]; c++;

		max.X(cx);
		y += h;
	
		for(int i = 0; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			c = 0;
			cx = x;

			const char* name = categoryName(i);
			unsigned int col = categoryColour(i);
			if(name == '\0' || col == 0)
				continue;

			// draw small colour box
			float border = 0.05f;
			Maths::HVec2 min(cx - h + h * border, y + h * border);
			Maths::HVec2 max(min.X() + h * (1.0f - border * 2.0f), min.Y() + h * (1.0f - border * 2.0f));
			debugRenderPtr->Draw2dBar(min, (max - min), Maths::RGBA(col));

			debugRenderPtr->Draw2dLine(Maths::HVec2(min.X(), min.Y()), Maths::HVec2(max.X(), min.Y()), white); 
			debugRenderPtr->Draw2dLine(Maths::HVec2(max.X(), min.Y()), Maths::HVec2(max.X(), max.Y()), white);
			debugRenderPtr->Draw2dLine(Maths::HVec2(max.X(), max.Y()), Maths::HVec2(min.X(), max.Y()), white);
			debugRenderPtr->Draw2dLine(Maths::HVec2(min.X(), max.Y()), Maths::HVec2(min.X(), min.Y()), white);

			float rate = (float)(m_average.m_bytes[i]) / 1024.0f;
			float ratio = (totalRate > 0.00001f)? rate / totalRate : 0.0f;

			// name of the type
			sprintf_s(str, sizeof(str), name);
			debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
			cx += columns[c]; c++;

			// percentage load
			sprintf_s(str, sizeof(str), "%.1f %%", ratio * 100.0f);
			debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
			cx += columns[c]; c++;

			// rate in kB/S
			sprintf_s(str, sizeof(str), "%.3f", rate);
			debugRenderPtr->Draw2dText(Maths::HVec2(cx, y), str, white, h);
			cx += columns[c]; c++;
			y += h;
		}
		y += h;			
		max.Y(y);

		m_chartArea.setBounds(min, max);
	}

	void TrafficLog::renderPie(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float width, float height)
	{
		m_pieArea.setBounds(Maths::HVec2(x, y), Maths::HVec2(x + width, y + height));

		float totalRate = (float)(m_average.m_bytes[TRAFFIC_TYPE_SOCKET]) / 1024.0f;
		
		Maths::HVec2 cen(x + width * 0.5f, y + height * 0.5f);
		float rad = (height < width)? height : width;
		rad *= 0.35f;
		
		float twopi = Maths::Pi * 2.0f;
		float a = 0;
		for(int i = TRAFFIC_TYPE_FIRST; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			const char* name = categoryName(i);
			unsigned int col = categoryColour(i);

			if(name == '\0' || col == 0)
				continue;

			float rate = (float)(m_average.m_bytes[i]) / 1024.0f;
			float ratio = (totalRate > 0.00001f)? rate / totalRate : 0.0f;

			float da = twopi * ratio;
			renderPortion(debugRenderPtr, Maths::RGBA(col), cen, rad, a, da);
			a += da;
		}

		if(a < twopi && a > 0.001f)
		{
			unsigned int col = 0xff000000;
			float da = twopi - a;
			renderPortion(debugRenderPtr, Maths::RGBA(col), cen, rad, a, da);
		}
	}

	void TrafficLog::renderFlow(DbUtil::I_DebugRenderPtr debugRenderPtr, float x, float y, float width, float height)
	{
		m_area.setBounds(Maths::HVec2(x, y), Maths::HVec2(x + width, y + height));

		netTime end = netClock::instance()->now();
		
		float packet_peak = (float)((float)m_peak.m_bytes[TRAFFIC_TYPE_SOCKET] / 1024.0f);
		float cap_peak = (float)((float)m_peak.limit() / 1024.0f);
		float peak = (cap_peak > packet_peak)? cap_peak : packet_peak;
		if(peak < 1.0f) peak = 1.0f;

		// display peak
		float packet_x_ratio = (end - m_peak.m_timeEnd) / (float) FLOW_TIME_SPAN;
		float packet_x = x + width * (1.0f - packet_x_ratio);
		char str[128];
		sprintf_s(str, sizeof(str), "%.3f kB/s", packet_peak);
		debugRenderPtr->Draw2dText(Maths::HVec2(packet_x, y - 20), str, Maths::RGBA(0xFFFFFFFF), 24);
		
		// the cap limit line
		Maths::HVec2 cappos(0, 0);
		int count = 0;
		float bw = width / (float)(MAX_REPORTS);
		
		// render bars one by one
		for(int i = 1; i < MAX_REPORTS; i ++)
		{
			const TrafficSample* r = report(m_reportsPtr - i);
			if(r == NULL) continue;
		
			// current sample values
			float rate = (float)((float)r->m_bytes[TRAFFIC_TYPE_SOCKET] / 1024.0f);
			float cap = (float)((float)r->limit() / 1024.0f);

			// position of the sample in the render area
			float bx = x + width - (bw * i);
		
			// render single stacked bar
			renderBar(debugRenderPtr, r, peak, bx, y, bw, height);

			// render cap line
			Maths::HVec2 nextcappos((bx + bw * 0.5f), y + (1.0f - (cap / peak)) * height);

			if(count > 0)
			{
				debugRenderPtr->Draw2dLine(cappos, nextcappos, Maths::RGBA(0xFFFFFFFF));
			}
			cappos = nextcappos;
			count++;
		}
	}

	void TrafficLog::renderPortion(DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::RGBA colour, const Maths::HVec2& cen, float rad, float a, float da)
	{
		Maths::HVec2 firstp(cen.X() + cos(a) * rad, cen.Y() + sin(a) * rad);
		Maths::HVec2 p = firstp;

		const float twopi = 2 * Maths::Pi;
		const float step = twopi / 32.0f;
		const int count = (int)(da / step) + 1;
		const float s = da / count;
		for(int i = 0; i < count; i ++)
		{
			a += s;
			Maths::HVec2 nextp(cen.X() + cos(a) * rad, cen.Y() + sin(a) * rad);
			debugRenderPtr->Draw2dTriangle(cen, p, nextp, colour);
			debugRenderPtr->Draw2dLine(p, nextp, Maths::RGBA(0xFFFFFFFF));
			p = nextp;
		}
		debugRenderPtr->Draw2dLine(cen, firstp, Maths::RGBA(0xFFFFFFFF));
	}

	void TrafficLog::renderBar(DbUtil::I_DebugRenderPtr debugRenderPtr, const TrafficSample* report, float maxRate, float x, float y, float barwidth, float barheight)
	{
		Maths::HVec2 pos(x, y + barheight);
		Maths::HVec2 size(barwidth, 0);
		
		for(int i = TRAFFIC_TYPE_FIRST; i < TRAFFIC_TYPE_COUNT; i ++)
		{
			const char* name = categoryName(i);
			unsigned int col = categoryColour(i);
			if(name == '\0' || col == 0)
				continue;

			float rate = (float) ((float)report->m_bytes[i] / 1024.0f);
			
			float height = (rate / (float)maxRate) * barheight;
			size.Y(height);
			
			debugRenderPtr->Draw2dBar(Maths::HVec2(pos.X(), pos.Y() - height), size, Maths::RGBA(col));
			pos.Y(pos.Y() - height);
		}

		float rate = (float) ((float)report->m_bytes[TRAFFIC_TYPE_SOCKET] / 1024.0f);
		float socket = (rate / (float)maxRate) * barheight;
		if(pos.Y() > (1 + y + barheight - socket))
		{
			unsigned int col = 0xffffffff;
			float height = (pos.Y() - (y + barheight - socket));
			size.Y(height);
			debugRenderPtr->Draw2dBar(Maths::HVec2(pos.X(), pos.Y() - height), size, Maths::RGBA(col));
		}	
	}
};

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
