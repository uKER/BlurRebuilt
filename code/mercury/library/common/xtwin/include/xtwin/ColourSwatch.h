#ifndef XTWIN_COLOURSWATCH_H
#define XTWIN_COLOURSWATCH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin\WndCore.h"
#include "XtWin\NgMaths.h"

struct ColourSwatchData
{
	float red;
	float green;
	float blue;
	float alpha;
	float intensity;

public:
	ColourSwatchData() :
		red(1.0),
		green(1.0),
		blue(1.0),
		alpha(1.0),
		intensity(1.0)
	{
	}
};

class ColourSwatch : public ChildWindow
{
public:
	ColourSwatch();
	virtual ~ColourSwatch();

public:
	virtual void OnChanged();

public:
	void SetData(const ColourSwatchData* csd);
	void GetData(ColourSwatchData* csd);
	void SetDisplayColour(bool displayColour);
	void SetDisplayAlpha(bool displayAlpha);
	void SetDisplayIntensity(bool displayIntensity);
	void SetMaxIntensity(float maxIntensity);

public:
	inline bool DisplayAlpha() const { return _displayAlpha; }
	inline bool DisplayIntensity() const { return _displayIntensity; }

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void OnPaint(Gdiplus::Graphics* g);

private:
	ColourSwatchData		_csd;
	bool					_displayColour;
	bool					_displayAlpha;
	bool					_displayIntensity;
	float					_maxIntensity;
};

#endif // XTWIN_COLOURSWATCH_H