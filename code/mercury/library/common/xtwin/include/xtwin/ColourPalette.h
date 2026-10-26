#ifndef XTWIN_COLOURPALETTE_H
#define XTWIN_COLOURPALETTE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"
#include "XtWin\NgMaths.h"
#include "XtWin\ColourSwatch.h"

class ColourPalette : public PopupWindow
{
public:
	ColourPalette();
	virtual ~ColourPalette();

public:
	virtual void OnChanged();

public:
	void SetData(const ColourSwatchData* csd);
	void GetData(ColourSwatchData* csd);
	void SetDisplayAlpha(bool displayAlpha);
	void SetDisplayIntensity(bool displayAlpha);
	void SetMaxIntensity(float maxIntensity);

public:
	inline bool DisplayAlpha() const { return _displayAlpha; }
	inline bool DisplayIntensity() const { return _displayIntensity; }

public:
	void DoModal(const WinPoint& pt, HWND parent);

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

public:
	virtual bool DismissTest(const WinPoint& pt);

private:
	enum HitResult
	{
		kHitNone,
		kHitOutside,
		kHitHue,
		kHitSatVal,
		kHitRed,
		kHitGreen,
		kHitBlue,
		kHitAlpha,
		kHitIntensity
	};

	enum DragState
	{
		kDragNone,
		kDragHue,
		kDragSatVal,
		kDragRed,
		kDragGreen,
		kDragBlue,
		kDragAlpha,
		kDragIntensity
	};

private:
	HitResult HitTest(const WinPoint& pt);
	NgVector SatValFromPt(const WinPoint& pt);
	NgVector StartEndFromBar(int barNo, float inset);
	float WheelRotation();

private:
	void DoDrag(const WinPoint& pt);
	void DoSetCursor(const WinPoint& pt);
	void Redraw();
	void OnPaint(Gdiplus::Graphics* g);

private:
	DragState	_dragging;
	DWORD		_clickTime;

private:
	bool	_displayAlpha;
	bool	_displayIntensity;
	float	_maxIntensity;

private:
	NgColour	_rgb;
	NgColour	_hsv;
	float		_alpha;
	float		_intensity;

private:
	NgColour	_oldRgb;
	NgColour	_oldHsv;
	float		_oldAlpha;
	float		_oldIntensity;
};

#endif // XTWIN_COLOURPALETTE_H