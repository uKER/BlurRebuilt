#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/ColourSwatch.h"
#include "XtWin/ColourPalette.h"

// TODO: Context Menu -> Preset swatch management - Save Current To Preset List, Select Preset, Manage Presets, etc...

class ColourSwatchPalette : public ColourPalette
{
public:
	ColourSwatchPalette(ColourSwatch* swatch) :
		_swatch(swatch)
	{
	}

public:
	virtual void OnChanged()
	{
		ColourSwatchData csd;

		GetData(&csd);

		_swatch->SetData(&csd);

		_swatch->OnChanged();
	}

private:
	ColourSwatch* _swatch;
};

ColourSwatch::ColourSwatch() :
	_displayColour(true),
	_displayAlpha(true),
	_displayIntensity(true),
	_maxIntensity(8.0f)
{
}

ColourSwatch::~ColourSwatch()
{
}

void ColourSwatch::OnChanged()
{
}

void ColourSwatch::SetData(const ColourSwatchData* csd)
{
	if (csd)
	{
		_csd = *csd;

		InvalidateWindow(*this);
	}
}

void ColourSwatch::GetData(ColourSwatchData* csd)
{
	*csd = _csd;
}

void ColourSwatch::SetDisplayColour(bool displayColour)
{
	if (displayColour != _displayColour)
	{
		_displayColour = displayColour;

		InvalidateWindow(*this);
	}
}

void ColourSwatch::SetDisplayAlpha(bool displayAlpha)
{
	if (displayAlpha != _displayAlpha)
	{
		_displayAlpha = displayAlpha;

		InvalidateWindow(*this);
	}
}

void ColourSwatch::SetDisplayIntensity(bool displayIntensity)
{
	if (displayIntensity != _displayIntensity)
	{
		_displayIntensity = displayIntensity;

		InvalidateWindow(*this);
	}
}

void ColourSwatch::SetMaxIntensity(float maxIntensity)
{
	if (maxIntensity != _maxIntensity)
	{
		_maxIntensity = maxIntensity;

		InvalidateWindow(*this);
	}
}

bool ColourSwatch::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!ChildWindow::PreCreateWindow(cs))
		return false;

	cs->style		|= WS_CLIPCHILDREN | WS_BORDER;
	cs->dwExStyle	|= WS_EX_CONTROLPARENT;

	_wndClassRedraw = true;

	return true;
}

LRESULT ColourSwatch::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(_hwnd, &ps);

			OnPaint(&Gdiplus::Graphics(ps.hdc));

			if (GetFocus() == *this)
			{
				WinRect rc(*this);

				InflateRect(&rc, -1, -1);

				DrawFocusRect(ps.hdc, &rc);
			}

			EndPaint(_hwnd, &ps);
		}
		return 0;

		case WM_LBUTTONDOWN:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			ClientToScreen(*this, &pt);

			SetFocus(*this);

			ColourSwatchPalette cp(this);

			cp.SetData(&_csd);
			cp.SetDisplayAlpha(_displayAlpha);
			cp.SetDisplayIntensity(_displayIntensity);
			cp.SetMaxIntensity(_maxIntensity);

			cp.DoModal(pt, *this);

			cp.GetData(&_csd);

			InvalidateWindow(*this);
		}
		return 0;

		case WM_SETFOCUS:
		case WM_KILLFOCUS:
			InvalidateWindow(*this);
		return 0;
	}

	return ChildWindow::WndProc(msg, wParam, lParam);
}

namespace
{
	void DrawHatchedRect(Gdiplus::Graphics* g, const NgRect& rc, const NgColour& colour, float alpha, int cx, int cy)
	{
		Gdiplus::SolidBrush light(Lerp(NgColour(), colour, Clamp(alpha, 0.0f, 1.0f)));
		Gdiplus::SolidBrush dark(Lerp(NgColour(0.75f, 1.0f), colour, Clamp(alpha, 0.0f, 1.0f)));

		NgRect newRc;

		newRc.sz = NgVector(4.0f, 4.0f);

		for(int j = 0; j < cy; j++)
		{
			for(int i = 0; i < cx; i++)
			{
				Gdiplus::SolidBrush* brush = ((i + j) & 1) ? &light : &dark;

				newRc.pt = rc.pt + (newRc.sz * NgVector((float)i, (float)j));

				g->FillRectangle(brush, newRc);
			}
		}
	}
}

void ColourSwatch::OnPaint(Gdiplus::Graphics* g)
{
	NgRect rc(WinRect(*this));

	NgColour colour(_csd.red, _csd.green, _csd.blue);

	Gdiplus::SolidBrush solid(colour);

	NgVector sz = rc.sz * NgVector(0.5f, 1.0f);

	if (_displayAlpha && _displayColour)
	{
		g->FillRectangle(&solid, NgRect(rc.pt, sz));
		DrawHatchedRect(g, NgRect(rc.pt + NgVector(sz.vx, 0.0f), sz), colour, _csd.alpha, 5, 5);
	}
	else if (_displayAlpha)
		DrawHatchedRect(g, rc, colour, _csd.alpha, 5, 5);
	else
		g->FillRectangle(&solid, rc);
}