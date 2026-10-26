#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Chart.h"
#include "XtWin/TimelineChart.h"
#include "XtWin/NgMaths.h"

extern void DebugLn(const char* text, ...);

TimelineChart::TimelineChart()
{
	_capture = false;

	_oldPos.x = _oldPos.y = -1;

	_minX = 0;
	_maxX = 0;

	_minY = 0;
	_maxY = 10;

	_scrollX = 0;
	_scrollY = 0;

	_scaleX = 1;
	_scaleY = 1;

	_gcb = NULL;

	_dragSize = 3;
	_tangentDragSize = 2;

	_lastDrawn = 0;

	_rectSelectionActive = false;

	strcpy_s(_minString, 16, "");
	strcpy_s(_maxString, 16, "");
}

void TimelineChart::SetXExtents(double min, double max)
{
	_minX = min + _scrollX;
	_maxX = max + _scrollX;
}

void TimelineChart::SetYExtents(double min, double max)
{
	_minY = min + _scrollY;
	_maxY = max + _scrollY;
}

void TimelineChart::SetMinMaxString(char *min, char *max)
{
	strcpy_s(_minString, 16, min);
	strcpy_s(_maxString, 16, max);
}

void TimelineChart::SetCallbacks(TimelineChartCallbacks *gcb)
{
	_gcb = gcb;
}

void TimelineChart::SetScroll(double x, double y)
{
	_scrollX = x;
	_scrollY = y;
}

void TimelineChart::SetZoom(double x, double y)
{
	_scaleX = x;
	_scaleY = y;
}

#define _BORDER1X 48
#define _BORDER1Y 24
#define _BORDER2 12

double TimelineChart::XScale(void)
{
	double s = (_maxX - _minX) * _scaleX;

	if (s <= 0.001)
		return 0.001;

	return (_clientSize.cx-(_BORDER1X+_BORDER2)) / Max(s, 0.001);
}

double TimelineChart::YScale(void)
{
	double s = (_maxY-_minY) * _scaleY;

	if (s <= 0.001)
		return 0.001;

	return (_clientSize.cy-(_BORDER1Y+_BORDER2)) / Max(s, 0.001);
}

int TimelineChart::XToScreen(double x)
{
	return (int) _BORDER1X + (int) ((x-_minX) * XScale());
}

int TimelineChart::YToScreen(double y)
{
	return (int) _clientSize.cy-1-_BORDER1Y - (int) ((y-_minY) * YScale());
}

WinPoint TimelineChart::PointToScreen(ChartPoint gp)
{
	WinPoint wp;

	wp.x = XToScreen(gp.x);
	wp.y = YToScreen(gp.y);

	return wp;
}

double TimelineChart::ScreenToX(int x)
{
	return (x - _BORDER1X) / ((double)_clientSize.cx-(_BORDER1X+_BORDER2)) * ((_maxX-_minX)*_scaleX) + _minX;
}

double TimelineChart::ScreenToY(int y)
{
	return (_clientSize.cy-1-_BORDER1Y-y) / ((double)_clientSize.cy-(_BORDER1Y+_BORDER2)) * ((_maxY-_minY)*_scaleY) + _minY;
}

extern double FindNiceValue(double v);

void TimelineChart::DrawSelectionRect(Gdiplus::Graphics* g)
{
	if (!_rectSelectionActive)
		return;

	Gdiplus::Pen selected(0xB0FFA500, 2.0f);
	Gdiplus::Pen selectedAlt(0xB0FF0000, 2.0f);
	Gdiplus::SolidBrush background(Gdiplus::Color(100, 100, 100, 200));

	float pattern[] = { 2.0f, 2.0f };

	selected.SetDashPattern(pattern, ArrayCount(pattern));
	selectedAlt.SetDashPattern(pattern, ArrayCount(pattern));

	selectedAlt.SetDashOffset(2.0f);

	NgRect rect = Normalise(NgRect((float)_rectTopLeft.x, (float)_rectTopLeft.y, (float)_rectBottomRight.x-_rectTopLeft.x, (float)_rectBottomRight.y-_rectTopLeft.y));

	g->FillRectangle(&background, rect);
	g->DrawRectangle(&selected, rect);
	g->DrawRectangle(&selectedAlt, rect);
}

void TimelineChart::DrawGrid(Gdiplus::Graphics* g)
{
	RECT rc;
	GetClientRect(_hwnd, &rc);

	if (!_gcb)
		return;

	TimelineChartLine li;
	_gcb->GetLineInfo(0, &li);

	double xMin, xMax, yMin, yMax;
	if (li.generatedX || li.generatedY)
	{
		GetGeneratedMin(&xMin, &yMin);
		GetGeneratedMax(&xMax, &yMax);
	}

	Gdiplus::Pen pen1(Gdiplus::Color(0xDD, 0xDD, 0xFF), 1);
	Gdiplus::Pen pen2(Gdiplus::Color(0x99, 0x99, 0xF0), 1);
	Gdiplus::Pen pen3(Gdiplus::Color(0xBB, 0xBB, 0xF8), 2);
	Gdiplus::Pen pen4(Gdiplus::Color(0xFF, 0x00, 0x00), 2);

	if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
	if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

	SetXExtents(li.xMin, li.xMax);
	SetYExtents(li.yMin, li.yMax);

	// Calculate skip

	double chartWidth	= Max(fabs(ScreenToX(_clientSize.cx-_BORDER2) - ScreenToX(_BORDER1X)), 0.000001);
	double chartHeight	= Max(fabs(ScreenToY(_clientSize.cy-_BORDER2) - ScreenToY(_BORDER1Y)), 0.000001);

	double visualWidth	= _clientSize.cx-_BORDER1X-_BORDER2;
	double visualHeight	= _clientSize.cy-_BORDER1Y-_BORDER2;

	int xLabels = Max((int)(visualWidth / 32.0), 1);
	int yLabels = Max((int)(visualHeight / 32.0), 1);

	double xSkip = chartWidth / xLabels;
	double ySkip = chartHeight / yLabels;

	if (li.generatedX)
		xSkip = 1.0;
	else
		xSkip = FindNiceValue(xSkip);

	if (li.generatedY)
		ySkip = 1.0;
	else
		ySkip = FindNiceValue(ySkip);

	// Coords

	int chartLeft	= rc.left + _BORDER1X;
	int chartRight	= rc.right - _BORDER2 + 1;

	int chartTop	= rc.top + _BORDER2 - 1;
	int chartBottom	= rc.bottom - _BORDER1Y;

	// Horizontal gridlines
	for(double y = ((int)(_minY/ySkip))*ySkip; y < _minY+chartHeight; y += ySkip)
	{
		g->DrawLine(&pen1, chartLeft, YToScreen(y), chartRight, YToScreen(y));
	}

	// Vertical gridlines
	for(double x = ((int)(_minX/xSkip))*xSkip; x < _minX+chartWidth; x += xSkip)
	{
		g->DrawLine(&pen1, XToScreen(x), chartTop, XToScreen(x), chartBottom);
	}

	g->DrawLine(&pen2, XToScreen(li.timelineMin), chartTop, XToScreen(li.timelineMin), chartBottom);
	g->DrawLine(&pen2, XToScreen(li.timelineMax), chartTop, XToScreen(li.timelineMax), chartBottom);
	g->DrawLine(&pen2, chartLeft, YToScreen(0), chartRight, YToScreen(0));

	g->DrawRectangle(&pen3, chartLeft, chartTop, chartRight-chartLeft, chartBottom-chartTop);

	// Timeline indicator
	double timelineValue = _gcb->GetTimelineValue();
	g->DrawLine(&pen4, (int)XToScreen(timelineValue), chartTop, (int)XToScreen(timelineValue), chartBottom-1);

	Gdiplus::FontFamily ff(L"Franklin gothic medium");
	Gdiplus::Font fn(&ff, 8);
	Gdiplus::SolidBrush fontBrush(Gdiplus::Color(0, 0, 0));
	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetLineAlignment(Gdiplus::StringAlignmentNear);

	for(double x = ((int)(_minX/xSkip))*xSkip; x < _minX+chartWidth; x += xSkip)
	{
		Sz16 label = Format(L"%.2f", x);
		g->DrawString(label, label.Length(), &fn, Gdiplus::PointF((Gdiplus::REAL)XToScreen(x), (Gdiplus::REAL)chartBottom), &sf, &fontBrush);
	}

	sf.SetAlignment(Gdiplus::StringAlignmentFar);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	for(double y = ((int)(_minY/ySkip))*ySkip; y < _minY+chartHeight; y += ySkip)
	{
		Sz16 label = Format(L"%.2f", y);
		g->DrawString(label, label.Length(), &fn, Gdiplus::PointF((Gdiplus::REAL)chartLeft, (Gdiplus::REAL)YToScreen(y)), &sf, &fontBrush);
	}
}

void TimelineChart::GetGeneratedMin(double* pxMin, double* pyMin)
{
	*pxMin = 0.0;
	*pyMin = 0.0;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		TimelineChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (line == 0)
		{
			*pxMin = li.xMin;
			*pyMin = li.yMin;
		}

		if (li.generatedX)
			*pxMin = (li.xMin > *pxMin) ? li.xMin : *pxMin;

		if (li.generatedY)
			*pyMin = (li.yMin > *pyMin) ? li.yMin : *pyMin;
	}
}

void TimelineChart::GetGeneratedMax(double* pxMax, double* pyMax)
{
	*pxMax = 0.0;
	*pyMax = 0.0;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		TimelineChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (line == 0)
		{
			*pxMax = li.xMax;
			*pyMax = li.yMax;
		}

		if (li.generatedX)
			*pxMax = (li.xMax > *pxMax) ? li.xMax : *pxMax;

		if (li.generatedY)
			*pyMax = (li.yMax > *pyMax) ? li.yMax : *pyMax;
	}
}

extern int Brighter(int rgb, float v);

void TimelineChart::DrawChart(Gdiplus::Graphics* g)
{
	if (!_gcb)
		return;

	if (_gcb->GetLineCount() == 0)
		return;

	double xMin=0, yMin=0, xMax=0, yMax=0;
	GetGeneratedMin(&xMin, &yMin);
	GetGeneratedMax(&xMax, &yMax);

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		TimelineChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
		if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

		SetXExtents(li.xMin, li.xMax);
		SetYExtents(li.yMin, li.yMax);

		if (li.count != 0)
		{
			Gdiplus::Pen blackPen(Gdiplus::Color(0,0,0), 1);
			Gdiplus::Pen pen(Gdiplus::Color((BYTE)(li.colour & 0xFF), (BYTE)((li.colour & 0xFF00)>>8), (BYTE)((li.colour & 0xFF0000)>>16)), 2);
			
			int sz = li.count;

			ChartPoint gp;
			WinPoint wp;
			WinPoint owp;

			if (!li.isHermite)
			{
				_gcb->GetLineValue(line, 0, &gp);

				owp = wp = PointToScreen(gp);

				for (int x=1; x<sz; x++)
				{
					_gcb->GetLineValue(line, x, &gp);

					wp = PointToScreen(gp);

					g->DrawLine(&pen, owp.x, owp.y, wp.x, wp.y);
				}
			}
				DrawHermiteSpline(g, line, &pen, &blackPen);

			if (!li.isTemplate)
			{
				for (int x=0; x<sz; x++)
				{
					_gcb->GetLineValue(line, x, &gp);

					wp = PointToScreen(gp);

					g->DrawRectangle(&blackPen, wp.x-_dragSize, wp.y-_dragSize, _dragSize*2 + 1, _dragSize*2 + 1);
				}
			}
		}
	}
}

WinPoint TimelineChart::GetInTangentPos(WinPoint pt, double tangent)
{
	double angle = tangent;

	double x, y;

	x = -cos(angle) * XScale();
	y =  sin(angle) * YScale();

	double len = sqrt(x*x+y*y);
	x *= 32 / len;
	y *= 32 / len;

	pt.x += (int) x;
	pt.y += (int) y;

	return pt;
}

WinPoint TimelineChart::GetOutTangentPos(WinPoint pt, double tangent)
{
	double angle = tangent;

	double x, y;

	x =  cos(angle) * XScale();
	y = -sin(angle) * YScale();

	double len = sqrt(x*x+y*y);
	x *= 32 / len;
	y *= 32 / len;

	pt.x += (int) x;
	pt.y += (int) y;

	return pt;
}

#define PI 3.14159265358979323846
#define RAD_2_DEG(A) ((A)*57.2957795)
#define DEG_2_RAD(A) ((A)/57.2957795)

/////////////////////////////////////////////////////
void TimelineChart::DrawHermiteSpline(Gdiplus::Graphics* g, int lineIndex, Gdiplus::Pen* pen, Gdiplus::Pen* blackPen)
{
	ChartPoint p0, p1;
	double t0in, t0out, t1in, t1out;

	TimelineChartLine li;

	_gcb->GetLineInfo(lineIndex, &li);

	int prevxp;
	int prevyp;

	if (li.count > 1)
	{
		int pointIndex = 0;

		_gcb->GetLineValue(lineIndex, pointIndex, &p0);
		_gcb->GetLineValue(lineIndex, pointIndex+1, &p1);

		_gcb->GetInTangentValue(lineIndex, pointIndex, &t0in);
		_gcb->GetOutTangentValue(lineIndex, pointIndex, &t0out);
		_gcb->GetInTangentValue(lineIndex, pointIndex+1, &t1in);
		_gcb->GetOutTangentValue(lineIndex, pointIndex+1, &t1out);

		t0in  = DEG_2_RAD(t0in);
		t0out = DEG_2_RAD(t0out);
		t1in  = DEG_2_RAD(t1in);
		t1out = DEG_2_RAD(t1out);

		// keyFrames[i].x = value			- y
		// keyFrames[i].y = in tangent		- t?in
		// keyFrames[i].z = out tangent		- t?out
		// keyFrames[i].w = time			- x
		// minimum 2 keyframes needed

		double timeSpan = ScreenToX(_clientSize.cx-_BORDER2)-ScreenToX(_BORDER1X);
			
		int nSteps = _clientSize.cx/2;
		double fStep = timeSpan/(double)nSteps;
		double fTime = ScreenToX(_BORDER1X);

		for(int i = 0; i <= nSteps; ++i, fTime += fStep)
		{
			if(pointIndex+1 < li.count-1)
			{
				if(fTime>p1.x)
				{
					pointIndex++;
					_gcb->GetLineValue(lineIndex, pointIndex, &p0);
					_gcb->GetLineValue(lineIndex, pointIndex+1, &p1);

					_gcb->GetInTangentValue(lineIndex, pointIndex, &t0in);
					_gcb->GetOutTangentValue(lineIndex, pointIndex, &t0out);
					_gcb->GetInTangentValue(lineIndex, pointIndex+1, &t1in);
					_gcb->GetOutTangentValue(lineIndex, pointIndex+1, &t1out);

					t0in  = DEG_2_RAD(t0in);
					t0out = DEG_2_RAD(t0out);
					t1in  = DEG_2_RAD(t1in);
					t1out = DEG_2_RAD(t1out);
				}
			}

			double fValue;

			if (fTime<p0.x)
				fValue = p0.y;
			else if (fTime>p1.x)
				fValue = p1.y;
			else
			{
				double fPeriod = (p1.x - p0.x);
				double u = (fTime - p0.x)/fPeriod;

				double y1 = p0.y;
				double y4 = p1.y;

				double y2 = y1 + tan(t0out)/(3.f/fPeriod);
				double y3 = y4 - tan(t1in)/(3.f/fPeriod);

				double u2 = u * u;
				double u3 = u2 * u;
				double h1 = -u3 + 3.f * u2 - 3.f * u + 1.f;
				double h2 = 3.f * u3 - 6.f * u2 + 3.f * u;
				double h3 = -3.f * u3 + 3.f * u2;
				double h4 = u3;
				fValue = y1 * h1 + y2 * h2 + y3 * h3 + y4 * h4;
			}

			int xp = XToScreen(fTime);
			int yp = YToScreen(fValue);

			if (i>0)
				g->DrawLine(pen, prevxp, prevyp, xp, yp);

			prevxp = xp;
			prevyp = yp;
		}
	}

	if (!li.isTemplate)
	{
		for (int i=0; i<li.count; i++)
		{
			WinPoint wp;

			_gcb->GetLineValue(lineIndex, i, &p0);
			_gcb->GetInTangentValue(lineIndex, i, &t0in);
			_gcb->GetOutTangentValue(lineIndex, i, &t0out);

			t0in  = DEG_2_RAD(t0in);
			t0out = DEG_2_RAD(t0out);

			if (i > 0)
			{
				wp = GetInTangentPos(PointToScreen(p0), t0in);

				g->DrawLine(blackPen, XToScreen(p0.x), YToScreen(p0.y), wp.x, wp.y);
			}

			if (i < li.count-1)
			{
				wp = GetOutTangentPos(PointToScreen(p0), t0out);

				g->DrawLine(blackPen, XToScreen(p0.x), YToScreen(p0.y), wp.x, wp.y);
			}
		}
	}

	if (!li.isTemplate)
	{
		for (int i=0; i<li.count; i++)
		{
			WinPoint wp;

			_gcb->GetLineValue(lineIndex, i, &p0);
			_gcb->GetInTangentValue(lineIndex, i, &t0in);
			_gcb->GetOutTangentValue(lineIndex, i, &t0out);

			t0in  = DEG_2_RAD(t0in);
			t0out = DEG_2_RAD(t0out);

			if (i > 0)
			{
				wp = GetInTangentPos(PointToScreen(p0), t0in);

				g->DrawRectangle(blackPen, wp.x-_tangentDragSize, wp.y-_tangentDragSize, _tangentDragSize*2, _tangentDragSize*2);
			}

			if (i < li.count-1)
			{
				wp = GetOutTangentPos(PointToScreen(p0), t0out);

				g->DrawRectangle(blackPen, wp.x-_tangentDragSize, wp.y-_tangentDragSize, _tangentDragSize*2, _tangentDragSize*2);
			}
		}
	}
}
/////////////////////////////////////////////////////

void TimelineChart::DoCursor(void)
{
	WinPoint pt, wp;
	GetCursorPos(&pt);
	ScreenToClient(_hwnd, &pt);

	bool canDrag = false;

	if (_gcb)
	{
		for (int line=0; line<_gcb->GetLineCount() && !canDrag; line++)
		{
			TimelineChartLine li;
			_gcb->GetLineInfo(line, &li);

			if (li.isTemplate)
				continue;

			double xMin=0, yMin=0, xMax=0, yMax=0;
			GetGeneratedMin(&xMin, &yMin);
			GetGeneratedMax(&xMax, &yMax);

			if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
			if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

			SetXExtents(li.xMin, li.xMax);
			SetYExtents(li.yMin, li.yMax);

			for (int x=0; x<li.count && !canDrag; x++)
			{
				ChartPoint gp;
				_gcb->GetLineValue(line, x, &gp);

				wp = PointToScreen(gp);

				if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && 
					pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
					canDrag = true;

				if (li.isHermite)
				{
					double in, out;
					_gcb->GetInTangentValue(line, x, &in);
					_gcb->GetOutTangentValue(line, x, &out);

					in  = DEG_2_RAD(in);
					out = DEG_2_RAD(out);

					if (x > 0)
					{
						// In tangent
						wp = GetInTangentPos(PointToScreen(gp), in);

						if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && 
							pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
							canDrag = true;
					}

					if (x < li.count-1)
					{
						// Out tangent
						wp = GetOutTangentPos(PointToScreen(gp), out);

						if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && 
							pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
							canDrag = true;
					}
				}
			}
		}
	}

	if (canDrag)
		SetCursor(LoadCursor(0, IDC_CROSS));
	else
		SetCursor(LoadCursor(0, IDC_ARROW));
}

void TimelineChart::PaintWindow(void)
{
	PAINTSTRUCT ps;
	BeginPaint(_hwnd, &ps);

	{
		Gdiplus::Graphics g(ps.hdc);

		WinRect rcPaint = ps.rcPaint;

		Gdiplus::Bitmap bm(rcPaint.Width(), rcPaint.Height(), &g);

		{
			Gdiplus::Graphics g2(&bm);

			g2.Clear(Gdiplus::Color::White);

			g2.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
			g2.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			g2.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);

			g2.TranslateTransform(-(float)rcPaint.left, -(float)rcPaint.top);

			WinRect rcClient(*this);

			DrawGrid(&g2);
			DrawChart(&g2);
			DrawSelectionRect(&g2);
		}

		Gdiplus::CachedBitmap cbm(&bm, &g);

		g.DrawCachedBitmap(&cbm, rcPaint.left, rcPaint.top);
	}

	EndPaint(_hwnd, &ps);
}


void TimelineChart::MouseMove(WinPoint &pt, WPARAM wParam)
{
	double currentX = ScreenToX(pt.x);
	double currentY = ScreenToY(pt.y);

	double dragX = currentX - _startDragX;
	double dragY = currentY - _startDragY;

	_startDragX = currentX;
	_startDragY = currentY;

	_rectBottomRight = pt;

	if ((wParam & MK_LBUTTON || wParam & MK_MBUTTON) && _gcb->GetNumSelectedComponents() && _capture && !_rectSelectionActive)
	{
		ChartPoint gp;

		int line, index;
		_gcb->GetSelectedComponent(0, &line, &index);

		TimelineChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (li.isTemplate)
			return;

		double xMin=0, yMin=0, xMax=0, yMax=0;
		GetGeneratedMin(&xMin, &yMin);
		GetGeneratedMax(&xMax, &yMax);

		if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
		if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

		SetXExtents(li.xMin, li.xMax);
		SetYExtents(li.yMin, li.yMax);

		if (_dragTangent)
		{
			_gcb->GetLineValue(line, index, &gp);

			if (_dragIn)
			{
				double in;
				_gcb->GetInTangentValue(line, index, &in);

				gp.x = (XToScreen(gp.x) - pt.x) / XScale();
				gp.y = (YToScreen(gp.y) - pt.y) / YScale();

				if (gp.x < 0)
					gp.x = 0.0000001;

				in = RAD_2_DEG(atan2(-gp.y, gp.x));
				_gcb->SetInTangentValue(line, index, in);
				_gcb->SetOutTangentValue(line, index, in);
			}
			else
			{
				double out;
				_gcb->GetOutTangentValue(line, index, &out);

				gp.x = (pt.x - XToScreen(gp.x)) / XScale();
				gp.y = (pt.y - YToScreen(gp.y)) / YScale();

				if (gp.x < 0)
					gp.x = 0.0000001;

				out = RAD_2_DEG(atan2(-gp.y, gp.x));
				_gcb->SetOutTangentValue(line, index, out);
				_gcb->SetInTangentValue(line, index, out);
			}
		}
		else
		{
			int numSelected = _gcb->GetNumSelectedComponents();
			for (int i=0; i<numSelected; i++)
			{
				int line, index;
				_gcb->GetSelectedComponent(i, &line, &index);

				ChartPoint cp;
				_gcb->GetLineValue(line, index, &cp);

				cp.x += dragX;
				cp.y += dragY;

				_gcb->SetLineValue(line, index, &cp);
			}
		}

		InvalidateWindow(_hwnd);
	}
	else if (_capture)
	{
		InvalidateWindow(_hwnd);

		if (wParam & MK_RBUTTON)
		{
			WinPoint change = pt - _oldPos;
			double xDiff = _maxX-_minX;
			double yDiff = _maxY-_minY;
			_scrollX -= (double)change.x*xDiff*_scaleX/(double)_clientSize.cx;
			_scrollY += (double)change.y*yDiff*_scaleY/(double)_clientSize.cy;
			InvalidateWindow(_hwnd);
		}
	}
	else
	{
		DWORD timeSinceDrawn = GetTickCount() - _lastDrawn;

		if (timeSinceDrawn > 1000)
		{
			InvalidateWindow(_hwnd);

			_lastDrawn = GetTickCount();
		}
	}

	_oldPos = pt;
}

void TimelineChart::MouseWheel(WinPoint &pt, WPARAM wParam)
{
	ScreenToClient(_hwnd, &pt);

	short mouseWheel = HIWORD(wParam);

	if (!(LOWORD(wParam) & MK_CONTROL))
	{
		double oldX = ScreenToX(pt.x);
		_scaleX -= (double)mouseWheel/((double)WHEEL_DELTA*20./_scaleX);
		_scrollX += oldX-ScreenToX(pt.x);
	}

	if (!(LOWORD(wParam) & MK_SHIFT))
	{
		double oldY = ScreenToY(pt.y);
		_scaleY -= (double)mouseWheel/((double)WHEEL_DELTA*20./_scaleY);
		_scrollY += oldY-ScreenToY(pt.y);
	}

	InvalidateWindow(_hwnd);
}

void TimelineChart::LMouseDown(WinPoint &pt, WPARAM wParam)
{
	int dragIndex = -1;
	int dragLine = -1;
	
	_dragTangent = false;
	_dragIn = false;

	_rectTopLeft = pt;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		TimelineChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (li.isTemplate)
			continue;

		double xMin=0, yMin=0, xMax=0, yMax=0;
		GetGeneratedMin(&xMin, &yMin);
		GetGeneratedMax(&xMax, &yMax);

		if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
		if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

		SetXExtents(li.xMin, li.xMax);
		SetYExtents(li.yMin, li.yMax);

		for (int x=0; x<li.count; x++)
		{
			ChartPoint gp;
			WinPoint wp;
			_gcb->GetLineValue(line, x, &gp);

			wp = PointToScreen(gp);

			if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
			{
				dragIndex = x;
				dragLine = line;
			}
			else if (li.isHermite)
			{
				double in, out;

				_gcb->GetInTangentValue(line, x, &in);
				_gcb->GetOutTangentValue(line, x, &out);

				in  = DEG_2_RAD(in);
				out = DEG_2_RAD(out);

				if (x > 0)
				{
					wp = GetInTangentPos(PointToScreen(gp), in);
					if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
					{
						dragIndex = x;
						dragLine = line;
						_dragTangent = true;
						_dragIn = true;
					}
				}

				if (x < li.count-1)
				{
					wp = GetOutTangentPos(PointToScreen(gp), out);
					if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
					{
						dragIndex = x;
						dragLine = line;
						_dragTangent = true;
						_dragIn = false;
					}
				}
			}
		}
	}

	{
		// if (!SHIFT_DOWN && !CTRL_DOWN)
		_gcb->ClearSelection();

		if (dragIndex >= 0 && dragLine >= 0)
			_gcb->SetSelectedComponent(dragLine, dragIndex);
		else
			_rectSelectionActive = true;

		_startDragX = ScreenToX(pt.x);
		_startDragY = ScreenToY(pt.y);

		_capture = true;
		SetCapture(_hwnd);

		InvalidateWindow(_hwnd);
	}
}

void TimelineChart::LMouseUp(WinPoint &pt, WPARAM wParam)
{
	if (_rectSelectionActive)
	{
		_rectSelectionActive = false;

		
		NgRect rect = Normalise(NgRect((float)ScreenToX(_rectTopLeft.x), (float)ScreenToY(_rectTopLeft.y), (float)ScreenToX(_rectBottomRight.x)-(float)ScreenToX(_rectTopLeft.x), (float)ScreenToY(_rectBottomRight.y)-(float)ScreenToY(_rectTopLeft.y)));

		int lineCount = _gcb->GetLineCount();

		for (int i=0; i<lineCount; i++)
		{
			TimelineChartLine line;
			_gcb->GetLineInfo(i, &line);

			for (int j=0; j<line.count; j++)
			{
				ChartPoint point;
				_gcb->GetLineValue(i, j, &point);

				if (point.x >= rect.BottomLeft().vx && point.x <= rect.BottomRight().vx && point.y <= rect.BottomLeft().vy && point.y >= rect.TopLeft().vy)
				{
					_gcb->SetSelectedComponent(i, j);
				}
			}
		}
	}
	
	InvalidateWindow(_hwnd);

	if (_capture)
	{
		_capture = false;
		ReleaseCapture();
	}
}

void TimelineChart::RMouseDown(WinPoint &pt, WPARAM wParam)
{
	_capture = true;
	SetCapture(_hwnd);
}

void TimelineChart::RMouseUp(WinPoint &pt, WPARAM wParam)
{
	if (_capture)
	{
		_capture = false;
		ReleaseCapture();
	}
}

void TimelineChart::MMouseDown(WinPoint &pt, WPARAM wParam)
{
	_startDragX = ScreenToX(pt.x);
	_startDragY = ScreenToY(pt.y);

	_capture = true;
	SetCapture(_hwnd);
}

void TimelineChart::MMouseUp(WinPoint &pt, WPARAM wParam)
{
	if (_capture)
	{
		_capture = false;
		ReleaseCapture();
	}
}

void TimelineChart::KeyDown(LPARAM lParam, WPARAM wParam)
{
	if (wParam == VK_DELETE)
	{
		while(_gcb->GetNumSelectedComponents())
		{
			int line, index;
			_gcb->GetSelectedComponent(0, &line, &index);

			_gcb->DeleteKey(line, index);
		}
	}
}

LRESULT TimelineChart::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		break;

	case WM_SIZE:
		_clientSize.cx = LOWORD(lParam);
		_clientSize.cy = HIWORD(lParam);
		break;

	case WM_KEYDOWN:
		KeyDown(lParam, wParam);
		return 1;

	case WM_SETCURSOR:
		DoCursor();
		return 1;

	case WM_ERASEBKGND:
		return 1;

	case WM_PAINT:
		PaintWindow();
		return 1;

	case WM_MOUSEMOVE:
		if (_gcb)
			MouseMove(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_LBUTTONDOWN:
		SetFocus(*this);

		if (_gcb)
			LMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_LBUTTONUP:
		SetFocus(*this);

		if (_gcb)
			LMouseUp(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_MBUTTONDOWN:
		SetFocus(*this);

		if (_gcb)
			MMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_MBUTTONUP:
		SetFocus(*this);

		if (_gcb)
			MMouseUp(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_RBUTTONDOWN:
		SetFocus(*this);

		if (_gcb)
			RMouseDown(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_RBUTTONUP:
		SetFocus(*this);

		if (_gcb)
			RMouseUp(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		return 1;

	case WM_MOUSEWHEEL:
		SetFocus(*this);

		if (_gcb)
			MouseWheel(WinPoint((short) LOWORD(lParam), (short) HIWORD(lParam)), wParam);
		
		return 1;

	};
	
	return ChildWindow::WndProc(msg, wParam, lParam);
}
