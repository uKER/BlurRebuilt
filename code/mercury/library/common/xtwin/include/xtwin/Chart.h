#ifndef XTWIN_CHART_H
#define XTWIN_CHART_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"

struct ChartPoint
{
	ChartPoint() : x(0), y(0), colour(RGB(255, 255, 255)) {}

	double	x;
	double	y;

	COLORREF	colour;
};

struct ChartLine
{
	inline ChartLine() :
		count(0),
		colour(RGB(0xFF, 0xDD, 0xAA)),
		isHermite(false),
		isTemplate(false),
		xMin(0.0),
		xMax(0.0),
		yMin(0.0),
		yMax(0.0),
		generatedX(false),
		generatedY(false)
	{
	}

	int			count;
	COLORREF	colour;
	bool		isHermite;
	bool		isTemplate;

	double		xMin, xMax;
	double		yMin, yMax;

	bool		generatedX;
	bool		generatedY;

};


class ChartCallbacks
{
public:
	virtual int GetLineCount(void) = 0;
	virtual void GetLineInfo(int line, ChartLine *out) = 0;
	virtual void GetLineValue(int line, int index, ChartPoint *out) = 0;
	virtual void SetLineValue(int line, int index, const ChartPoint *in) = 0;
	virtual void GetInTangentValue(int line, int index, double *in) = 0;
	virtual void GetOutTangentValue(int line, int index, double *out) = 0;
	virtual void SetInTangentValue(int line, int index, double in) = 0;
	virtual void SetOutTangentValue(int line, int index, double out) = 0;
};


class Chart : public ChildWindow
{
public:
	Chart();

	LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

	void SetXExtents(double min, double max);
	void SetYExtents(double min, double max);

	void SetMinMaxString(char *min, char *max);

	void SetCallbacks(ChartCallbacks *cb);

	void SetDragSize(int size);

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

	void DrawGrid(HDC hdc);
	void DrawChart(HDC hdc);
	void DrawHermiteSpline(HDC hdc, int lineIndex, HPEN pen, HPEN blackPen);

	void DoCursor(void);

	void PaintWindow(void);

	void MouseMove(WinPoint &pt, WPARAM wParam);
	void LMouseDown(WinPoint &pt, WPARAM wParam);
	void LMouseUp(WinPoint &pt, WPARAM wParam);

	ChartCallbacks		*_gcb;

public:
	int			_id;

	WinPoint	_oldPos;
	bool		_capture;
	int			_dragIndex;		// Index of point to drag
	int			_dragLine;		// Index of line containing point to be dragged
	bool		_dragTangent;	// Are we dragging a tangent control?
	bool		_dragIn;		// Are we dragging the 'in' tangent?

	double		_minX, _maxX;
	double		_minY, _maxY;

	WinSize		_clientSize;
	int			_dragSize;
	int			_tangentDragSize;

	char		_minString[16];
	char		_maxString[16];

	DWORD		_lastDrawn;
};

#endif // XTWIN_CHART_H