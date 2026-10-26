#ifndef XTWIN_TIMELINECHART_H
#define XTWIN_TIMELINECHART_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"
#include "XtWin\Chart.h"

struct ChartPoint;

struct TimelineChartLine
{
	inline TimelineChartLine() :
		count(0),
		colour(RGB(0xFF, 0xDD, 0xAA)),
		isHermite(false),
		isTemplate(false),
		xMin(0.0),
		xMax(0.0),
		yMin(0.0),
		yMax(0.0),
		generatedX(false),
		generatedY(false),
		timelineMin(0.0),
		timelineMax(1.0)
	{
	}

	int			count;
	COLORREF	colour;
	bool		isHermite;
	bool		isTemplate;

	double		xMin, xMax;
	double		yMin, yMax;

	double		timelineMin, timelineMax;

	bool		generatedX;
	bool		generatedY;

};

class TimelineChartCallbacks
{
public:
	virtual int GetLineCount(void) = 0;
	virtual void GetLineInfo(int line, TimelineChartLine *out) = 0;
	virtual void GetLineValue(int line, int index, ChartPoint *out) = 0;
	virtual void SetLineValue(int line, int index, const ChartPoint *in) = 0;
	virtual void GetInTangentValue(int line, int index, double *in) = 0;
	virtual void GetOutTangentValue(int line, int index, double *out) = 0;
	virtual void SetInTangentValue(int line, int index, double in) = 0;
	virtual void SetOutTangentValue(int line, int index, double out) = 0;
	
	virtual int GetNumSelectedComponents() = 0;
	virtual void GetSelectedComponent(int index, int *outLine, int *outIndex) = 0;
	virtual void SetSelectedComponent(int line, int index, bool selected=true) = 0;
	virtual void ClearSelection() = 0;

	virtual double GetTimelineValue(void) = 0;
	virtual void DeleteKey(int line, int index) {}
	virtual void SmoothTangents(int line, int index) {}
	virtual void AddKey(int line, double u, double value) {}
};


class TimelineChart : public ChildWindow
{
public:
	TimelineChart();

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void SetXExtents(double min, double max);
	void SetYExtents(double min, double max);

	void SetMinMaxString(char *min, char *max);

	void SetCallbacks(TimelineChartCallbacks *cb);

	void SetDragSize(int size);

	void SetScroll(double x, double y);
	void SetZoom(double x, double y);

protected:
	double XScale(void);
	double YScale(void);

	int XToScreen(double x);
	int YToScreen(double y);
	WinPoint PointToScreen(ChartPoint gp);

	double ScreenToX(int x);
	double ScreenToY(int y);

	WinPoint GetInTangentPos(WinPoint pt, double tangent);
	WinPoint GetOutTangentPos(WinPoint pt, double tangent);

	void GetGeneratedMin(double* pxMin, double* pyMin);
	void GetGeneratedMax(double* pxMax, double* pyMax);

	void DrawGrid(Gdiplus::Graphics* g);
	void DrawChart(Gdiplus::Graphics* g);
	void DrawHermiteSpline(Gdiplus::Graphics* g, int lineIndex, Gdiplus::Pen* pen, Gdiplus::Pen* blackPen);
	void DrawSelectionRect(Gdiplus::Graphics* g);

	void DoCursor(void);

	void PaintWindow(void);

	void KeyDown(LPARAM lParam, WPARAM wParam);

	void MouseWheel(WinPoint &pt, WPARAM wParam);
	void MouseMove(WinPoint &pt, WPARAM wParam);
	void LMouseDown(WinPoint &pt, WPARAM wParam);
	void LMouseUp(WinPoint &pt, WPARAM wParam);

	void RMouseDown(WinPoint &pt, WPARAM wParam);
	void RMouseUp(WinPoint &pt, WPARAM wParam);

	void MMouseDown(WinPoint &pt, WPARAM wParam);
	void MMouseUp(WinPoint &pt, WPARAM wParam);

	TimelineChartCallbacks		*_gcb;

public:
	int			_id;

	WinPoint	_oldPos;
	bool		_capture;
	
	bool		_dragTangent;	// Are we dragging a tangent control?
	bool		_dragIn;		// Are we dragging the 'in' tangent?

	double		_startDragX, _startDragY;

	double		_minX, _maxX;
	double		_minY, _maxY;
	double		_scrollX, _scrollY;
	double		_scaleX, _scaleY;

	bool		_rectSelectionActive;
	WinPoint	_rectTopLeft;
	WinPoint	_rectBottomRight;

	WinSize		_clientSize;
	int			_dragSize;
	int			_tangentDragSize;

	char		_minString[16];
	char		_maxString[16];

	DWORD		_lastDrawn;
};

#endif // XTWIN_CHART_H