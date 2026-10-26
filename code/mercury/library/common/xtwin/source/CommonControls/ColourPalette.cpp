#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/ColourPalette.h"

namespace
{
	const NgVector	kWheelPt		= NgVector(220.0f, 220.0f);
	const float		kWheelHueR0		= 160.0f;
	const float		kWheelHueR1		= 190.0f;
	const float		kWheelMiniR0	= 197.5f;
	const float		kWheelMiniR1	= 210.0f;
	const float		kWheelOutside	= 222.5f;
	const float		kWheelTriR		= 148.0f;
	const float		kWheelSwatchR0	= 120.0f;
	const float		kWheelSwatchR1	= 150.0f;
	const float		kWheelMarkerPad	= 1.0f;
	const float		kWheelSelectPad	= 4.0f;
	const int		kWheelSegments	= 80;
	const int		kWheelBars		= 5;
	const bool		kWheelRotate	= false;
}

ColourPalette::ColourPalette() :
	_dragging(kDragNone),
	_clickTime(0),
	_displayAlpha(true),
	_displayIntensity(true),
	_maxIntensity(8.0f),
	_alpha(1.0f),
	_intensity(1.0f),
	_oldAlpha(1.0f),
	_oldIntensity(1.0f)
{
}

ColourPalette::~ColourPalette()
{
}

void ColourPalette::OnChanged()
{
}

void ColourPalette::SetData(const ColourSwatchData* csd)
{
	if (csd)
	{
		_rgb.r = Clamp(csd->red, 0.0f, 1.0f);
		_rgb.g = Clamp(csd->green, 0.0f, 1.0f);
		_rgb.b = Clamp(csd->blue, 0.0f, 1.0f);
		_rgb.a = csd->alpha;

		_hsv = RgbToHsv(_rgb);

		_alpha		= csd->alpha;
		_intensity	= csd->intensity;

		_oldRgb			= _rgb;
		_oldHsv			= _hsv;
		_oldAlpha		= _alpha;
		_oldIntensity	= _intensity;

		InvalidateWindow(*this);
	}
}

void ColourPalette::GetData(ColourSwatchData* csd)
{
	csd->red		= _rgb.r;
	csd->green		= _rgb.g;
	csd->blue		= _rgb.b;
	csd->alpha		= _alpha;
	csd->intensity	= _intensity;
}

void ColourPalette::SetDisplayAlpha(bool displayAlpha)
{
	if (displayAlpha != _displayAlpha)
	{
		_displayAlpha = displayAlpha;

		InvalidateWindow(*this);
	}
}

void ColourPalette::SetDisplayIntensity(bool displayIntensity)
{
	if (displayIntensity != _displayIntensity)
	{
		_displayIntensity = displayIntensity;

		InvalidateWindow(*this);
	}
}

void ColourPalette::SetMaxIntensity(float maxIntensity)
{
	if (maxIntensity != _maxIntensity)
	{
		_maxIntensity = maxIntensity;

		InvalidateWindow(*this);
	}
}

void ColourPalette::DoModal(const WinPoint& pt, HWND parent)
{
	WinSize sz(440, 440);

	PopupWindow::DoModal(pt - WinPoint((int)kWheelPt.vx, (int)kWheelPt.vy), sz, parent);
}

bool ColourPalette::PreCreateWindow(CREATESTRUCT* cs)
{
	if (!PopupWindow::PreCreateWindow(cs))
		return false;

	cs->style		&= ~WS_BORDER;
	cs->dwExStyle	&= ~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE);
	cs->dwExStyle	|= WS_EX_LAYERED;

	_wndClassRedraw = true;

	return true;
}

LRESULT ColourPalette::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			if (PopupWindow::WndProc(msg, wParam, lParam) < 0)
				return -1;

			_dragging = kDragNone;
		return 0;

		case WM_SIZE:
			Redraw();
		break;

		case WM_PAINT:
		case WM_ERASEBKGND:
		return 0;

		// Mouse move

		case WM_MOUSEMOVE:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			DoDrag(pt);
			DoSetCursor(pt);
		}
		return 0;

		// Left mouse button

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if ((msg == WM_LBUTTONDBLCLK) && ((GetTickCount() - _clickTime) < GetDoubleClickTime()))
			{
				EndModal(0);

				return 0;
			}

			_dragging	= kDragNone;
			_clickTime	= GetTickCount();

			switch(HitTest(pt))
			{
				case kHitHue:		_dragging = kDragHue;		break;
				case kHitSatVal:	_dragging = kDragSatVal;	break;
				case kHitRed:		_dragging = kDragRed;		break;
				case kHitGreen:		_dragging = kDragGreen;		break;
				case kHitBlue:		_dragging = kDragBlue;		break;
				case kHitAlpha:		_dragging = kDragAlpha;		break;
				case kHitIntensity:	_dragging = kDragIntensity;	break;
			}

			DoDrag(pt);
			DoSetCursor(pt);
		}
		return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			DoDrag(pt);

			_dragging = kDragNone;

			DoSetCursor(pt);
		}
		return 0;

		// Middle mouse button

		case WM_MBUTTONDOWN:
		case WM_MBUTTONDBLCLK:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (_dragging == kDragNone)
			{
				_dragging = kDragHue;

				DoDrag(pt);
				DoSetCursor(pt);
			}
		}
		return 0;

		// Right mouse button

		case WM_RBUTTONDOWN:
		case WM_RBUTTONDBLCLK:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			if (HitTest(pt) == kHitOutside)
			{
				if (_dragging == kDragNone)
					EndModal(0);
			}
		}
		return 0;

		case WM_RBUTTONUP:
			if (_dragging == kDragNone)
				EndModal(0);
		return 0;

		// Mouse wheel

		case WM_MOUSEWHEEL:
		{
			WinPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

			ScreenToClient(*this, &pt);

			HitResult	hit		= HitTest(pt);
			float		delta	= (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
			float		side	= (pt.x < kWheelPt.vx) ? 1.0f : -1.0f;

			delta = SquarePersistSign(delta);

			switch(hit)
			{
				case kHitHue:
					_hsv.r	= fmodf(fmodf(_hsv.r + (side * delta * 0.005f), 1.0f) + 1.0f, 1.0f);
					_rgb	= HsvToRgb(_hsv);
				break;

				case kHitSatVal:
				{
					NgVector sv = SatValFromPt(pt);

					if ((sv.vx >= 0.0f) && (sv.vx <= 1.0f) && (sv.vy >= 0.0f) && (sv.vy <= 1.0f))
					{
						_hsv.g = Clamp(Lerp(_hsv.g, sv.vx, -delta * 0.01f), 0.0f, 1.0f);
						_hsv.b = Clamp(Lerp(_hsv.b, sv.vy, -delta * 0.01f), 0.0f, 1.0f);

						_rgb = HsvToRgb(_hsv);
					}
				}
				break;

				case kHitRed:
					_rgb.r	= Clamp(_rgb.r + (delta * 0.005f), 0.0f, 1.0f);
					_hsv	= RgbToHsv(_rgb);
				break;

				case kHitGreen:
					_rgb.g	= Clamp(_rgb.g + (delta * 0.005f), 0.0f, 1.0f);
					_hsv	= RgbToHsv(_rgb);
				break;

				case kHitBlue:
					_rgb.b	= Clamp(_rgb.b + (delta * 0.005f), 0.0f, 1.0f);
					_hsv	= RgbToHsv(_rgb);
				break;

				case kHitAlpha:
					_alpha = Clamp(_alpha + (delta * 0.005f), 0.0f, 1.0f);
					_rgb.a = _alpha;
					_hsv.a = _alpha;
				break;

				case kHitIntensity:
					_intensity = Clamp(_intensity + (_maxIntensity * delta * 0.005f), 0.0f, _maxIntensity);
				break;
			}

			if ((hit != kHitNone) && (hit != kHitOutside))
			{
				OnChanged();
				Redraw();
			}
		}
		break;
	}

	return PopupWindow::WndProc(msg, wParam, lParam);
}

bool ColourPalette::DismissTest(const WinPoint& pt)
{
	if (_dragging != kDragNone)
		return false;

	return HitTest(pt) == kHitOutside;
}

ColourPalette::HitResult ColourPalette::HitTest(const WinPoint& pt)
{
	float r = Length(pt - kWheelPt);
	float a = Rotation(pt - kWheelPt);

	if (r > kWheelOutside)
		return kHitOutside;

	if ((r > (kWheelMiniR0 - kWheelSelectPad)) && (r < (kWheelMiniR1 + kWheelSelectPad)))
	{
		for(int i = 0; i < kWheelBars; i++)
		{
			NgVector se = StartEndFromBar(i, 0.0f);
			float f = NormaliseAngle(a - se.vx) / (se.vy - se.vx);

			if ((f >= 0.0f) && (f <= 1.0f))
			{
				switch(i)
				{
					case 0: return kHitRed;
					case 1: return kHitGreen;
					case 2: return kHitBlue;
					case 3: return _displayAlpha ? kHitAlpha : kHitNone;
					case 4: return _displayIntensity ? kHitIntensity : kHitNone;
				}
			}
		}
	}

	if ((r > kWheelHueR0) && (r < kWheelHueR1))
		return kHitHue;

	if (r < kWheelTriR)
	{
		NgVector sv = SatValFromPt(pt);

		if ((sv.vx >= 0.0f) && (sv.vx <= 1.0f) && (sv.vy >= 0.0f) && (sv.vy <= 1.0f))
			return kHitSatVal;
	}

	return kHitNone;
}

NgVector ColourPalette::SatValFromPt(const WinPoint& pt)
{
	float tf0 = WheelRotation();
	float tf1 = tf0 - (Constants::k2Pi / 3.0f);
	float tf2 = tf1 - (Constants::k2Pi / 3.0f);

	NgVector sv0 = kWheelPt + Rotation(tf0, kWheelTriR - kWheelMarkerPad);
	NgVector sv1 = kWheelPt + Rotation(tf1, kWheelTriR - kWheelMarkerPad);
	NgVector sv2 = kWheelPt + Rotation(tf2, kWheelTriR - kWheelMarkerPad);

	float area = AreaOfPara(sv1 - sv0, sv2 - sv0);

	float a = AreaOfPara(sv1 - pt, sv2 - pt) / area;
	float b = 1.0f - (AreaOfPara(sv2 - pt, sv0 - pt) / area);

	return NgVector(a / b, b);
}

NgVector ColourPalette::StartEndFromBar(int barNo, float inset)
{
	float start	= ((barNo / (float)kWheelBars) * Constants::k2Pi) + 0.025f + DegToRad(90.0f - 18.0f) + inset;
	float end	= (((barNo + 1) / (float)kWheelBars) * Constants::k2Pi) - 0.025f + DegToRad(90.0f - 18.0f) - inset;

	if (barNo >= 3)
		Swap(start, end);

	return NgVector(start, end);
}

float ColourPalette::WheelRotation()
{
	return -DegToRad(90.0f);
	//return _hsv.r * Constants::k2Pi;
}

void ColourPalette::DoDrag(const WinPoint& pt)
{
	switch(_dragging)
	{
		case kDragHue:
		{
			NgVector delta = pt - kWheelPt;

			_hsv.r	= fmodf((Rotation(delta) / Constants::k2Pi) + 1.0f, 1.0f);
			_rgb	= HsvToRgb(_hsv);
		}
		break;

		case kDragSatVal:
		{
			NgVector sv = SatValFromPt(pt);
		
			_hsv.g	= Clamp(sv.vx, 0.0f, 1.0f);
			_hsv.b	= Clamp(sv.vy, 0.0f, 1.0f);
			_rgb	= HsvToRgb(_hsv);
		}
		break;

		case kDragRed:
		{
			NgVector	delta	= pt - kWheelPt;
			NgVector	se		= StartEndFromBar(0, 0.025f);

			_rgb.r	= Clamp(NormaliseAngle(Rotation(delta) - se.vx) / (se.vy - se.vx), 0.0f, 1.0f);
			_hsv	= RgbToHsv(_rgb);
		}
		break;

		case kDragGreen:
		{
			NgVector	delta	= pt - kWheelPt;
			NgVector	se		= StartEndFromBar(1, 0.025f);

			_rgb.g	= Clamp(NormaliseAngle(Rotation(delta) - se.vx) / (se.vy - se.vx), 0.0f, 1.0f);
			_hsv	= RgbToHsv(_rgb);
		}
		break;

		case kDragBlue:
		{
			NgVector	delta	= pt - kWheelPt;
			NgVector	se		= StartEndFromBar(2, 0.025f);

			_rgb.b	= Clamp(NormaliseAngle(Rotation(delta) - se.vx) / (se.vy - se.vx), 0.0f, 1.0f);
			_hsv	= RgbToHsv(_rgb);
		}
		break;

		case kDragAlpha:
		{
			NgVector	delta	= pt - kWheelPt;
			NgVector	se		= StartEndFromBar(3, 0.025f);

			_alpha	= Clamp(NormaliseAngle(Rotation(delta) - se.vx) / (se.vy - se.vx), 0.0f, 1.0f);
			_rgb.a	= _alpha;
			_hsv.a	= _alpha;
		}
		break;

		case kDragIntensity:
		{
			NgVector	delta	= pt - kWheelPt;
			NgVector	se		= StartEndFromBar(4, 0.025f);

			_intensity = Clamp(NormaliseAngle(Rotation(delta) - se.vx) / (se.vy - se.vx), 0.0f, 1.0f) * _maxIntensity;
		}
		break;
	}

	if (_dragging != kDragNone)
	{
		OnChanged();
		Redraw();
	}
}

void ColourPalette::DoSetCursor(const WinPoint& pt)
{
	bool crosshair = _dragging != kDragNone;

	switch(HitTest(pt))
	{
		case kHitNone:
		break;

		case kHitOutside:
			crosshair = false;
		break;

		default:
			crosshair = true;
		break;
	}

	SetCursor(LoadCursor(0, MAKEINTRESOURCE(crosshair ? IDC_CROSS : IDC_ARROW)));
}

void ColourPalette::Redraw()
{
	WinRect rc;
	GetWindowRect(*this, &rc);

	BITMAPINFO bmi = { 0 };

	bmi.bmiHeader.biSize		= sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth		= rc.Width();
	bmi.bmiHeader.biHeight		= -rc.Height();
	bmi.bmiHeader.biPlanes		= 1;
	bmi.bmiHeader.biBitCount	= 32;
	bmi.bmiHeader.biCompression	= BI_RGB;

	HDC		hdc			= CreateCompatibleDC(0);
	HBITMAP	bitmap		= CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, 0, 0, 0);
	HANDLE	oldBitmap	= SelectObject(hdc, bitmap);

	{
		Gdiplus::Graphics g(hdc);

		g.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);
		//g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

		OnPaint(&g);
	}

	BLENDFUNCTION bf;

	bf.BlendOp				= AC_SRC_OVER;
	bf.BlendFlags			= 0;
	bf.SourceConstantAlpha	= 255;
	bf.AlphaFormat			= AC_SRC_ALPHA;

	UpdateLayeredWindow(*this, 0, &rc.Position(), &rc.Size(), hdc, &WinPoint(0, 0), RGB(255,1,255), &bf, ULW_ALPHA);

	SelectObject(hdc, oldBitmap);

	DeleteDC(hdc);
	DeleteBitmap(bitmap);
}

void ColourPalette::OnPaint(Gdiplus::Graphics* g)
{
	// Pens

	Gdiplus::Pen markerOutline0(Gdiplus::Color::Black, 3.5f);
	Gdiplus::Pen markerOutline1(Gdiplus::Color::White, 1.75f);

	Gdiplus::Pen outline(Gdiplus::Color::Black, 1.5f);

	// Brushes

	Gdiplus::SolidBrush blackBrush(Gdiplus::Color::Black);
	Gdiplus::SolidBrush whiteBrush(Gdiplus::Color::White);

	// Fill wheel

	for(int i = 0; i < kWheelSegments; i++)
	{
		float f0 = ((i - 0.1f) / (float)kWheelSegments) * Constants::k2Pi;
		float f1 = ((i + 1.1f) / (float)kWheelSegments) * Constants::k2Pi;

		NgVector in0 = kWheelPt + Rotation(f0, kWheelHueR0);
		NgVector in1 = kWheelPt + Rotation(f1, kWheelHueR0);

		NgVector out0 = kWheelPt + Rotation(f0, kWheelHueR1);
		NgVector out1 = kWheelPt + Rotation(f1, kWheelHueR1);

		NgColour c0 = HsvToRgb(NgColour((i - 0.1f) / (float)kWheelSegments, 1.0f, 1.0f, 1.0f));
		NgColour c1 = HsvToRgb(NgColour((i + 1.1f) / (float)kWheelSegments, 1.0f, 1.0f, 1.0f));

		Gdiplus::LinearGradientBrush brush(out0, out1, c0, c1);

		Gdiplus::PointF points[4];

		points[0] = in0;
		points[1] = in1;
		points[2] = out1;
		points[3] = out0;

		g->FillPolygon(&brush, points, 4);
	}

	// Fill inner wheel

	Gdiplus::SolidBrush fill(Gdiplus::Color(127, 0, 0, 0));

	g->FillEllipse(&fill, NgRect(kWheelPt - NgVector(kWheelHueR0), kWheelHueR0 * 2.0f));

	// Wheel outline

	g->DrawEllipse(&outline, NgRect(kWheelPt - NgVector(kWheelHueR0), kWheelHueR0 * 2.0f));
	g->DrawEllipse(&outline, NgRect(kWheelPt - NgVector(kWheelHueR1), kWheelHueR1 * 2.0f));

	// Triangle fill

	float tf0 = WheelRotation();
	float tf1 = tf0 - (Constants::k2Pi / 3.0f);
	float tf2 = tf1 - (Constants::k2Pi / 3.0f);

	Gdiplus::PointF triPts[3];

	triPts[0] = kWheelPt + Rotation(tf0, kWheelTriR);
	triPts[1] = kWheelPt + Rotation(tf1, kWheelTriR);
	triPts[2] = kWheelPt + Rotation(tf2, kWheelTriR);

	Gdiplus::PointF colourPts[3];

	colourPts[0] = kWheelPt + Rotation(tf0, kWheelTriR + 2.0f);
	colourPts[1] = kWheelPt + Rotation(tf1, kWheelTriR + 2.0f);
	colourPts[2] = kWheelPt + Rotation(tf2, kWheelTriR + 2.0f);

	Gdiplus::PointF markerPts[3];

	markerPts[0] = kWheelPt + Rotation(tf0, kWheelTriR - kWheelMarkerPad);
	markerPts[1] = kWheelPt + Rotation(tf1, kWheelTriR - kWheelMarkerPad);
	markerPts[2] = kWheelPt + Rotation(tf2, kWheelTriR - kWheelMarkerPad);

	Gdiplus::Color colourVals[3];

	colourVals[0] = HsvToRgb(NgColour(_hsv.r, 1.0f, 1.0f, 1.0f));
	colourVals[1] = Gdiplus::Color::Black;
	colourVals[2] = Gdiplus::Color::White;

	Gdiplus::PathGradientBrush fillTri(colourPts, 3);

	int count = 2;
	fillTri.SetSurroundColors(colourVals, &count);

	fillTri.SetCenterColor(colourVals[2]);
	fillTri.SetCenterPoint(triPts[2]);

	g->FillPolygon(&fillTri, triPts, 3);

	// Triangle outline

	g->DrawPolygon(&outline, triPts, 3);

	// Current colour swatch

	for(int i = 0; i < 3; i++)
	{
		Gdiplus::GraphicsPath currSwatch;

		float r0 = kWheelSwatchR0;
		float a0 = 13.625f;

		float r1 = kWheelSwatchR1;
		float a1 = 3.5f;

		currSwatch.AddArc(NgRect(kWheelPt - NgVector(r0), r0 * 2.0f), RadToDeg(tf0) + a0 + (i * 120.0f), (120.0f - (2.0f * a0)));

		currSwatch.AddArc(NgRect(kWheelPt - NgVector(r1), r1 * 2.0f), RadToDeg(tf0) + a1 + (i * 120.0f) + (120.0f - (2.0f * a1)), -(120.0f - (2.0f * a1)));

		currSwatch.CloseFigure();

		Gdiplus::SolidBrush currBrush(NgColour(_rgb, 1.0f));

		g->FillPath(&currBrush, &currSwatch);
		g->DrawPath(&markerOutline0, &currSwatch);
		g->DrawPath(&markerOutline1, &currSwatch);
	}

	// Old hue marker

	NgVector oldHuePt0 = kWheelPt + Rotation(_oldHsv.r * Constants::k2Pi, kWheelHueR0);
	NgVector oldHuePt1 = kWheelPt + Rotation(_oldHsv.r * Constants::k2Pi, kWheelHueR1);

	g->DrawEllipse(&markerOutline0, NgRect(oldHuePt0 - NgVector(1.0f), NgVector(2.0f)));
	g->DrawEllipse(&markerOutline0, NgRect(oldHuePt1 - NgVector(1.0f), NgVector(2.0f)));

	g->DrawEllipse(&markerOutline1, NgRect(oldHuePt0 - NgVector(1.0f), NgVector(2.0f)));
	g->DrawEllipse(&markerOutline1, NgRect(oldHuePt1 - NgVector(1.0f), NgVector(2.0f)));

	// Old sat/val marker

	NgVector oldSvPt = Lerp(NgVector(markerPts[1]), Lerp(NgVector(markerPts[2]), NgVector(markerPts[0]), _oldHsv.g), _oldHsv.b);

	g->DrawEllipse(&markerOutline0, NgRect(oldSvPt - NgVector(1.0f), NgVector(2.0f)));
	g->DrawEllipse(&markerOutline1, NgRect(oldSvPt - NgVector(1.0f), NgVector(2.0f)));

	// Hue marker

	NgVector huePt0 = kWheelPt + Rotation(_hsv.r * Constants::k2Pi, kWheelHueR0);
	NgVector huePt1 = kWheelPt + Rotation(_hsv.r * Constants::k2Pi, kWheelHueR1);

	NgVector huePt0a = kWheelPt + Rotation(_hsv.r * Constants::k2Pi, kWheelHueR0 + 3.0f);
	NgVector huePt1a = kWheelPt + Rotation(_hsv.r * Constants::k2Pi, kWheelHueR1 - 3.0f);

	g->DrawEllipse(&markerOutline0, NgRect(huePt0 - NgVector(3.0f), NgVector(6.0f)));
	g->DrawEllipse(&markerOutline0, NgRect(huePt1 - NgVector(3.0f), NgVector(6.0f)));
	g->DrawLine(&markerOutline0, huePt0a, huePt1a);

	g->DrawEllipse(&markerOutline1, NgRect(huePt0 - NgVector(3.0f), NgVector(6.0f)));
	g->DrawEllipse(&markerOutline1, NgRect(huePt1 - NgVector(3.0f), NgVector(6.0f)));
	g->DrawLine(&markerOutline1, huePt0a, huePt1a);

	// Sat/Val marker

	NgVector svPt = Lerp(NgVector(markerPts[1]), Lerp(NgVector(markerPts[2]), NgVector(markerPts[0]), _hsv.g), _hsv.b);

	g->DrawEllipse(&markerOutline0, NgRect(svPt - NgVector(5.0f), NgVector(10.0f)));
	g->DrawEllipse(&markerOutline1, NgRect(svPt - NgVector(5.0f), NgVector(10.0f)));

	// Minibars

	for(int barNo = 0; barNo < kWheelBars; barNo++)
	{
		NgVector startEnd	= StartEndFromBar(barNo, 0.0f);
		NgVector inStartEnd	= StartEndFromBar(barNo, 0.025f);

		float start		= startEnd.vx;
		float end		= startEnd.vy;
		float inStart	= inStartEnd.vx;
		float inEnd		= inStartEnd.vy;

		int segmentsPerBar = kWheelSegments / kWheelBars;

		bool isDisabled = false;

		if (barNo == 3)
			isDisabled = !_displayAlpha;
		else if (barNo == 4)
			isDisabled = !_displayIntensity;

		NgColour colour0;
		NgColour colour1;

		float value		= 0.0f;
		float oldValue	= 0.0f;

		if (isDisabled)
		{
			colour0		= NgColour(0.75f, 0.75f, 0.75f, 0.75f);
			colour1		= NgColour(0.75f, 0.75f, 0.75f, 0.75f);
		}
		else
		{
			switch(barNo)
			{
				case 0:
					colour0		= NgColour(0.3f, 0.1f, 0.1f, 1.0f);
					colour1		= NgColour(1.0f, 0.2f, 0.2f, 1.0f);
					value		= _rgb.r;
					oldValue	= _oldRgb.r;
				break;

				case 1:
					colour0		= NgColour(0.1f, 0.3f, 0.1f, 1.0f);
					colour1		= NgColour(0.2f, 1.0f, 0.2f, 1.0f);
					value		= _rgb.g;
					oldValue	= _oldRgb.g;
				break;

				case 2:
					colour0		= NgColour(0.1f, 0.1f, 0.3f, 1.0f);
					colour1		= NgColour(0.2f, 0.2f, 1.0f, 1.0f);
					value		= _rgb.b;
					oldValue	= _oldRgb.b;
				break;

				case 3:
					colour0		= NgColour(_rgb, 0.0f);
					colour1		= NgColour(_rgb, 1.0f);
					value		= _alpha;
					oldValue	= _oldAlpha;
				break;

				case 4:
					colour0		= NgColour(0.25f, 0.25f, 0.25f, 1.0f);
					colour1		= NgColour(1.0f, 1.0f, 1.0f, 1.0f);
					value		= _intensity / _maxIntensity;
					oldValue	= _oldIntensity / _maxIntensity;
				break;
			}
		}

		if ((barNo == 3) && !isDisabled)
		{
			Gdiplus::HatchBrush hatch(Gdiplus::HatchStyleLargeCheckerBoard, Gdiplus::Color(255, 255, 255, 255), Gdiplus::Color(255, 192, 192, 192));

			for(int i = 0; i < segmentsPerBar; i++)
			{
				float f0 = (i - 0.05f) / (float)segmentsPerBar;
				float f1 = (i + 1.05f) / (float)segmentsPerBar;

				float a0 = Lerp(start, end, f0);
				float a1 = Lerp(start, end, f1);

				NgVector in0 = kWheelPt + Rotation(a0, kWheelMiniR0);
				NgVector in1 = kWheelPt + Rotation(a1, kWheelMiniR0);

				NgVector out0 = kWheelPt + Rotation(a0, kWheelMiniR1);
				NgVector out1 = kWheelPt + Rotation(a1, kWheelMiniR1);

				Gdiplus::PointF points[4];

				points[0] = in0;
				points[1] = in1;
				points[2] = out1;
				points[3] = out0;

				g->FillPolygon(&hatch, points, 4);
			}
		}

		for(int i = 0; i < segmentsPerBar; i++)
		{
			float f0 = (i - 0.02f) / (float)segmentsPerBar;
			float f1 = (i + 1.02f) / (float)segmentsPerBar;

			float a0 = Lerp(start, end, f0);
			float a1 = Lerp(start, end, f1);

			NgVector in0 = kWheelPt + Rotation(a0, kWheelMiniR0);
			NgVector in1 = kWheelPt + Rotation(a1, kWheelMiniR0);

			NgVector out0 = kWheelPt + Rotation(a0, kWheelMiniR1);
			NgVector out1 = kWheelPt + Rotation(a1, kWheelMiniR1);

			NgColour c0 = Lerp(colour0, colour1, f0);
			NgColour c1 = Lerp(colour0, colour1, f1);

			Gdiplus::LinearGradientBrush brush(out0, out1, c0, c1);

			Gdiplus::PointF points[4];

			points[0] = in0;
			points[1] = in1;
			points[2] = out1;
			points[3] = out0;

			g->FillPolygon(&brush, points, 4);
		}

		for(int i = 0; i < segmentsPerBar; i++)
		{
			float f0 = Lerp(start, end, (i - 0.0f) / (float)segmentsPerBar);
			float f1 = Lerp(start, end, (i + 1.0f) / (float)segmentsPerBar);

			NgVector in0 = kWheelPt + Rotation(f0, kWheelMiniR0);
			NgVector in1 = kWheelPt + Rotation(f1, kWheelMiniR0);

			NgVector out0 = kWheelPt + Rotation(f0, kWheelMiniR1);
			NgVector out1 = kWheelPt + Rotation(f1, kWheelMiniR1);

			g->DrawLine(&outline, in0, in1);
			g->DrawLine(&outline, out0, out1);
		}

		// End caps

		NgVector in0 = kWheelPt + Rotation(start, kWheelMiniR0);
		NgVector in1 = kWheelPt + Rotation(end, kWheelMiniR0);

		NgVector out0 = kWheelPt + Rotation(start, kWheelMiniR1);
		NgVector out1 = kWheelPt + Rotation(end, kWheelMiniR1);

		g->DrawLine(&outline, in0, out0);
		g->DrawLine(&outline, in1, out1);

		if (!isDisabled)
		{
			// Old marker

			float		oldMarkerFactor	= Lerp(inStart, inEnd, Clamp(oldValue, 0.0f, 1.0f));
			NgVector	oldMarkerPt0	= kWheelPt + Rotation(oldMarkerFactor, kWheelMiniR0);
			NgVector	oldMarkerPt1	= kWheelPt + Rotation(oldMarkerFactor, kWheelMiniR1);

			g->DrawEllipse(&markerOutline0, NgRect(oldMarkerPt0 - NgVector(0.5f), NgVector(1.0f)));
			g->DrawEllipse(&markerOutline0, NgRect(oldMarkerPt1 - NgVector(0.5f), NgVector(1.0f)));

			g->DrawEllipse(&markerOutline1, NgRect(oldMarkerPt0 - NgVector(0.5f), NgVector(1.0f)));
			g->DrawEllipse(&markerOutline1, NgRect(oldMarkerPt1 - NgVector(0.5f), NgVector(1.0f)));

			// Marker

			float		markerFactor	= Lerp(inStart, inEnd, Clamp(value, 0.0f, 1.0f));
			NgVector	markerPt0		= kWheelPt + Rotation(markerFactor, kWheelMiniR0);
			NgVector	markerPt1		= kWheelPt + Rotation(markerFactor, kWheelMiniR1);

			NgVector	markerPt0a		= kWheelPt + Rotation(markerFactor, kWheelMiniR0 + 2.0f);
			NgVector	markerPt1a		= kWheelPt + Rotation(markerFactor, kWheelMiniR1 - 2.0f);

			g->DrawEllipse(&markerOutline0, NgRect(markerPt0 - NgVector(2.0f), NgVector(4.0f)));
			g->DrawEllipse(&markerOutline0, NgRect(markerPt1 - NgVector(2.0f), NgVector(4.0f)));
			g->DrawLine(&markerOutline0, markerPt0a, markerPt1a);

			g->DrawEllipse(&markerOutline1, NgRect(markerPt0 - NgVector(2.0f), NgVector(4.0f)));
			g->DrawEllipse(&markerOutline1, NgRect(markerPt1 - NgVector(2.0f), NgVector(4.0f)));
			g->DrawLine(&markerOutline1, markerPt0a, markerPt1a);
		}
	}
}