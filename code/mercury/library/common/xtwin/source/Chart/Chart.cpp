#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Chart.h"

extern void DebugLn(const char* text, ...);

Chart::Chart()
{
	_capture = false;

	_oldPos.x = _oldPos.y = -1;

	_minX = 0;
	_maxX = 0;

	_minY = 0;
	_maxY = 10;

	_gcb = NULL;

	_dragSize = 3;
	_tangentDragSize = 2;

	_lastDrawn = 0;

	strcpy_s(_minString, 16, "");
	strcpy_s(_maxString, 16, "");
}

void Chart::SetXExtents(double min, double max)
{
	_minX = min;
	_maxX = max;
}

void Chart::SetYExtents(double min, double max)
{
	_minY = min;
	_maxY = max;
}

void Chart::SetMinMaxString(char *min, char *max)
{
	strcpy_s(_minString, 16, min);
	strcpy_s(_maxString, 16, max);
}

void Chart::SetCallbacks(ChartCallbacks *gcb)
{
	_gcb = gcb;
}

#define _BORDER1 24
#define _BORDER2 12

double Chart::XScale(void)
{
	double s = _maxX - _minX;

	if (s <= 0.001)
		return 0.001;

	return (_clientSize.cx-(_BORDER1+_BORDER2)) / Max(s, 0.001);
}

double Chart::YScale(void)
{
	double s = _maxY-_minY;

	if (s <= 0.001)
		return 0.001;

	return (_clientSize.cy-(_BORDER1+_BORDER2)) / Max(s, 0.001);
}

int Chart::XToScreen(double x)
{
	return (int) _BORDER1 + (int) ((x-_minX) * XScale());
}

int Chart::YToScreen(double y)
{
	return (int) _clientSize.cy-1-_BORDER1 - (int) ((y-_minY) * YScale());
}

WinPoint Chart::PointToScreen(ChartPoint gp)
{
	WinPoint wp;

	wp.x = XToScreen(gp.x);
	wp.y = YToScreen(gp.y);

	return wp;
}

double Chart::ScreenToX(int x)
{
	return (x - _BORDER1) / ((double)_clientSize.cx-(_BORDER1+_BORDER2)) * Max(_maxX-_minX, 1.0) + _minX;
}

double Chart::ScreenToY(int y)
{
	return (_clientSize.cy-1-_BORDER1 - y) / ((double)_clientSize.cy-(_BORDER1+_BORDER2)) * Max(_maxY-_minY, 0.01) + _minY;
}

double FindNiceValue(double v)
{
	if (v < 0.0)			return -FindNiceValue(-v);
	else if (v < 0.00001)	return 0.00001;
	else if (v <= 0.011)	return FindNiceValue(v * 10.0) * 0.1;
	else if (v <= 0.021)	return 0.02;
	else if (v <= 0.026)	return 0.025;
	else if (v <= 0.051)	return 0.05;
	else if (v <= 0.076)	return 0.075;
	else if (v <= 0.11)		return 0.1;
	else if (v <= 0.21)		return 0.2;
	else if (v <= 0.26)		return 0.25;
	else if (v <= 0.51)		return 0.5;
	else if (v <= 0.76)		return 0.75;
	else if (v <= 1.1)		return 1.0;
	else					return FindNiceValue(v * 0.1) * 10.0;
}

void Chart::DrawGrid(HDC hdc)
{
	HBRUSH br = CreateSolidBrush(RGB(255, 255, 255));
	RECT rc;
	GetClientRect(_hwnd, &rc);
	FillRect(hdc, &rc, br);
	DeleteObject(br);

	if (!_gcb)
		return;

	double xMin, xMax, yMin, yMax;
	GetGeneratedMin(&xMin, &yMin);
	GetGeneratedMax(&xMax, &yMax);

	ChartLine li;
	_gcb->GetLineInfo(0, &li);

	HPEN pen1 = CreatePen(PS_SOLID, 1, RGB(0xDD, 0xDD, 0xFF));
	HPEN pen2 = CreatePen(PS_SOLID, 1, RGB(0x99, 0x99, 0xF0));
	HPEN pen3 = CreatePen(PS_SOLID, 2, RGB(0xBB, 0xBB, 0xF8));

	HPEN oldPen = (HPEN) SelectObject(hdc, pen1);

	if (!li.generatedX)
	{
		xMin = _minX;
		xMax = _maxX;
	}

	if (!li.generatedY)
	{
		yMin = _minY;
		yMax = _maxY;
	}

	SetXExtents(xMin, xMax);
	SetYExtents(yMin, yMax);

	// Calculate skip

	double chartWidth	= fabs(xMax - xMin) + 0.000001;
	double chartHeight	= fabs(yMax - yMin) + 0.000001;

	double visualWidth	= fabs((double)XToScreen(xMax) - (double)XToScreen(xMin));
	double visualHeight	= fabs((double)YToScreen(yMin) - (double)YToScreen(yMax));

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

	double xDelta = xSkip * 0.5;
	double yDelta = ySkip * 0.5;

	// Coords

	int chartLeft	= rc.left + _BORDER1;
	int chartRight	= rc.right - _BORDER2 + 1;

	int chartTop	= rc.top + _BORDER2 - 1;
	int chartBottom	= rc.bottom - _BORDER1;

	// Horizontal gridlines

	SelectObject(hdc, pen1);

	for(double y = -ySkip; y > (yMin - yDelta); y -= ySkip)
	{
		MoveToEx(hdc, chartLeft, YToScreen(y), NULL);
		LineTo(hdc, chartRight, YToScreen(y));
	}

	for(double y = ySkip; y < (yMax + yDelta); y += ySkip)
	{
		MoveToEx(hdc, chartLeft, YToScreen(y), NULL);
		LineTo(hdc, chartRight, YToScreen(y));
	}

	// Vertical gridlines

	for(double x = -xSkip; x > (xMin - xDelta); x -= xSkip)
	{
		MoveToEx(hdc, XToScreen(x), chartTop, NULL);
		LineTo(hdc, XToScreen(x), chartBottom);
	}

	for(double x = xSkip; x < (xMax + xDelta); x += xSkip)
	{
		MoveToEx(hdc, XToScreen(x), chartTop, NULL);
		LineTo(hdc, XToScreen(x), chartBottom);
	}

	SelectObject(hdc, pen2);

	MoveToEx(hdc, XToScreen(0), chartTop, NULL);
	LineTo(hdc, XToScreen(0), chartBottom);

	MoveToEx(hdc, chartLeft, YToScreen(0), NULL);
	LineTo(hdc, chartRight, YToScreen(0));

	// Outline
	SelectObject(hdc, pen3);
	SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));

	Rectangle(hdc, rc.left + _BORDER1, rc.top + _BORDER2 - 1, rc.right - _BORDER2 + 1, rc.bottom - _BORDER1);

	SelectObject(hdc, GetStockObject(WHITE_BRUSH));

	SelectObject(hdc, oldPen);

	DeleteObject(pen1);
	DeleteObject(pen2);
	DeleteObject(pen3);

	SetBkMode(hdc, TRANSPARENT);

	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
	SetTextAlign(hdc, TA_CENTER | TA_TOP);

	for(double x = -xSkip; x > (xMin - xDelta); x -= xSkip)
	{
		XtString sz = Format("%g", x);
		TextOut(hdc, XToScreen(x), YToScreen(yMin) + 4, sz, sz.Length());
	}

	for(double x = 0.0f; x < (xMax + xDelta); x += xSkip)
	{
		XtString sz = Format("%g", x);
		TextOut(hdc, XToScreen(x), YToScreen(yMin) + 4, sz, sz.Length());
	}

	SetTextAlign(hdc, TA_RIGHT | TA_BASELINE);

	for(double y = -ySkip; y > (yMin - yDelta); y -= ySkip)
	{
		XtString sz = Format("%g", y);
		TextOut(hdc, XToScreen(xMin) - 4, YToScreen(y) + 4, sz, sz.Length());
	}

	for(double y = 0.0f; y < (yMax + yDelta); y += ySkip)
	{
		XtString sz = Format("%g", y);
		TextOut(hdc, XToScreen(xMin) - 4, YToScreen(y) + 4, sz, sz.Length());
	}

	SetTextAlign(hdc, TA_LEFT | TA_TOP);
}

void Chart::GetGeneratedMin(double* pxMin, double* pyMin)
{
	*pxMin = 0.0;
	*pyMin = 0.0;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		ChartLine li;
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

void Chart::GetGeneratedMax(double* pxMax, double* pyMax)
{
	*pxMax = 0.0;
	*pyMax = 0.0;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		ChartLine li;
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

int Brighter(int rgb, float v)
{
	float r = ((rgb & 0xFF0000) >> 16) / 255.0f;
	float g = ((rgb & 0xFF00) >> 8) / 255.0f;
	float b = ((rgb & 0xFF)) / 255.0f;

	r = sqrtf(r);
	g = sqrtf(g);
	b = sqrtf(b);

	r *= v;
	g *= v;
	b *= v;

	r *= r;
	g *= g;
	b *= b;

	int ir = Clamp((int)(r * 255.0f), 0, 255);
	int ig = Clamp((int)(g * 255.0f), 0, 255);
	int ib = Clamp((int)(b * 255.0f), 0, 255);

	return (ir << 16) | (ig << 8) | ib;
}

void Chart::DrawChart(HDC hdc)
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
		ChartLine li;
		_gcb->GetLineInfo(line, &li);

		if (li.generatedX)	{li.xMin = xMin; li.xMax = xMax;}
		if (li.generatedY)	{li.yMin = yMin; li.yMax = yMax;}

		SetXExtents(li.xMin, li.xMax);
		SetYExtents(li.yMin, li.yMax);

		if (li.count != 0)
		{
			HPEN blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			HPEN pen = CreatePen(PS_SOLID, 2, li.colour);
			HBRUSH brush = CreateSolidBrush(Brighter(li.colour, 1.5f));
			HPEN op = (HPEN) SelectObject(hdc, pen);

			SelectObject(hdc, brush);

			int sz = li.count;

			ChartPoint gp;
			WinPoint wp;

			if (!li.isHermite)
			{
				_gcb->GetLineValue(line, 0, &gp);

				wp = PointToScreen(gp);

				MoveToEx(hdc, wp.x, wp.y, NULL);
				for (int x=1; x<sz; x++)
				{
					_gcb->GetLineValue(line, x, &gp);

					wp = PointToScreen(gp);

					LineTo(hdc, wp.x, wp.y);
				}
			}
			else
				DrawHermiteSpline(hdc, line, pen, blackPen);

			SelectObject(hdc, blackPen);

			if (!li.isTemplate)
			{
				for (int x=0; x<sz; x++)
				{
					_gcb->GetLineValue(line, x, &gp);

					wp = PointToScreen(gp);

					Rectangle(hdc, wp.x-_dragSize, wp.y-_dragSize, wp.x+_dragSize+1, wp.y+_dragSize+1);
				}
			}

			SelectObject(hdc, op);
			SelectObject(hdc, GetStockObject(WHITE_BRUSH));

			DeleteObject(pen);
			DeleteObject(blackPen);
			DeleteObject(brush);
		}
	}
}

WinPoint Chart::GetInTangentPos(WinPoint pt, double tangent)
{
	double angle = tangent;

	double x, y;

	x = -cos(angle);// * XScale();
	y =  sin(angle);// * YScale();

	//double len = sqrt(x*x+y*y);
	x *= 32;// / len;
	y *= 32;// / len;

	pt.x += (int) x;
	pt.y += (int) y;

	return pt;
}

WinPoint Chart::GetOutTangentPos(WinPoint pt, double tangent)
{
	double angle = tangent;

	double x, y;

	x =  cos(angle);// * XScale();
	y = -sin(angle);// * YScale();

	//double len = sqrt(x*x+y*y);
	x *= 32;// / len;
	y *= 32;// / len;

	pt.x += (int) x;
	pt.y += (int) y;

	return pt;
}

#define PI 3.14159265358979323846
#define RAD_2_DEG(A) ((A)*57.2957795)
#define DEG_2_RAD(A) ((A)/57.2957795)

/////////////////////////////////////////////////////
void Chart::DrawHermiteSpline(HDC hdc, int lineIndex, HPEN pen, HPEN blackPen)
{
	ChartPoint p0, p1;
	double t0in, t0out, t1in, t1out;

	ChartLine li;

	_gcb->GetLineInfo(lineIndex, &li);

	SelectObject(hdc, blackPen);

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

				MoveToEx(hdc, XToScreen(p0.x), YToScreen(p0.y), NULL);

				LineTo(hdc, wp.x, wp.y);
			}

			if (i < li.count-1)
			{
				wp = GetOutTangentPos(PointToScreen(p0), t0out);

				MoveToEx(hdc, XToScreen(p0.x), YToScreen(p0.y), NULL);
				LineTo(hdc, wp.x, wp.y);
			}
		}
	}

	SelectObject(hdc, pen);

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

		double timeSpan = Max(_maxX-_minX, 1.0);

		int nSteps = 100;
		double fStep = timeSpan/(double)nSteps;
		double fTime = _minX;

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

			if (i==0)
				MoveToEx(hdc, xp, yp, NULL);
			else
				LineTo(hdc, xp, yp);
		}
	}

	SelectObject(hdc, blackPen);

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

				Rectangle(hdc, wp.x-_tangentDragSize, wp.y-_tangentDragSize, wp.x+_tangentDragSize+1, wp.y+_tangentDragSize+1);
			}

			if (i < li.count-1)
			{
				wp = GetOutTangentPos(PointToScreen(p0), t0out);

				Rectangle(hdc, wp.x-_tangentDragSize, wp.y-_tangentDragSize, wp.x+_tangentDragSize+1, wp.y+_tangentDragSize+1);
			}
		}
	}
}
/////////////////////////////////////////////////////

void Chart::DoCursor(void)
{
	WinPoint pt, wp;
	GetCursorPos(&pt);
	ScreenToClient(_hwnd, &pt);

	bool canDrag = false;

	if (_gcb)
	{
		for (int line=0; line<_gcb->GetLineCount() && !canDrag; line++)
		{
			ChartLine li;
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

void Chart::PaintWindow(void)
{
	HDC hdc = GetDC(_hwnd);
	HDC hdc0 = CreateCompatibleDC(hdc);

	HBITMAP hbmp = CreateCompatibleBitmap(hdc, _clientSize.cx, _clientSize.cy);
	HBITMAP obmp = (HBITMAP) SelectObject(hdc0, hbmp);

	DrawGrid(hdc0);
	DrawChart(hdc0);

	ReleaseDC(_hwnd, hdc);

	PAINTSTRUCT ps;
	hdc = BeginPaint(_hwnd, &ps);
	::BitBlt(hdc, 0, 0, _clientSize.cx, _clientSize.cy, hdc0, 0, 0, SRCCOPY);
	EndPaint(_hwnd, &ps);

	SelectObject(hdc0, obmp);
	DeleteBitmap(hbmp);
	DeleteDC(hdc0);
}


void Chart::MouseMove(WinPoint &pt, WPARAM wParam)
{
	if (_capture)
	{
		ChartPoint gp;

		ChartLine li;
		_gcb->GetLineInfo(_dragLine, &li);

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
			_gcb->GetLineValue(_dragLine, _dragIndex, &gp);

			if (_dragIn)
			{
				double in;
				_gcb->GetInTangentValue(_dragLine, _dragIndex, &in);

				gp.x = gp.x - ScreenToX(pt.x);
				gp.y = gp.y - ScreenToY(pt.y);

				if (gp.x < 0)
					gp.x = 0.0000001;

				in = RAD_2_DEG(atan2(gp.y * YScale(), gp.x * XScale()));
				_gcb->SetInTangentValue(_dragLine, _dragIndex, in);
			}
			else
			{
				double out;
				_gcb->GetOutTangentValue(_dragLine, _dragIndex, &out);

				gp.x = ScreenToX(pt.x) - gp.x;
				gp.y = ScreenToY(pt.y) - gp.y;

				if (gp.x < 0)
					gp.x = 0.0000001;

				out = RAD_2_DEG(atan2(gp.y * YScale(), gp.x * XScale()));
				_gcb->SetOutTangentValue(_dragLine, _dragIndex, out);
			}
		}
		else
		{
			gp.x = Clamp(ScreenToX(pt.x), _minX, _maxX);
			gp.y = Clamp(ScreenToY(pt.y), _minY, _maxY);

			_gcb->SetLineValue(_dragLine, _dragIndex, &gp);
		}

		InvalidateWindow(_hwnd);
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

void Chart::LMouseDown(WinPoint &pt, WPARAM wParam)
{
	_dragIndex = -1;
	_dragLine = -1;
	_dragTangent = false;
	_dragIn = false;

	for (int line=0; line<_gcb->GetLineCount(); line++)
	{
		ChartLine li;
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
				_dragIndex = x;
				_dragLine = line;
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
						_dragIndex = x;
						_dragLine = line;
						_dragTangent = true;
						_dragIn = true;
					}
				}

				if (x < li.count-1)
				{
					wp = GetOutTangentPos(PointToScreen(gp), out);
					if (pt.x >= wp.x-_dragSize-1 && pt.x <= wp.x+_dragSize+1 && pt.y >= wp.y-_dragSize-1 && pt.y <= wp.y+_dragSize+1)
					{
						_dragIndex = x;
						_dragLine = line;
						_dragTangent = true;
						_dragIn = false;
					}
				}
			}
		}
	}

	if (_dragIndex != -1)
	{
		_capture = true;
		SetCapture(_hwnd);

		InvalidateWindow(_hwnd);
	}
}

void Chart::LMouseUp(WinPoint &pt, WPARAM wParam)
{
	if (_capture)
	{
		_capture = false;
		ReleaseCapture();
	}
}

LRESULT Chart::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_CREATE:
		break;

	case WM_SIZE:
		_clientSize.cx = LOWORD(lParam);
		_clientSize.cy = HIWORD(lParam);
		break;

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

	case WM_RBUTTONDOWN:
		SetFocus(*this);
		return 1;

	case WM_RBUTTONUP:
		SetFocus(*this);
		return 1;
	};

	return ChildWindow::WndProc(msg, wParam, lParam);
}
