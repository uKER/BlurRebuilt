#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

SnGradient::SnGradient(LfControl* field) :
	SnControl(field),
	_dom(DomCallback, this)
{
}

SnGradient::~SnGradient()
{
}

void SnGradient::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnGradient::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnGradient::OnBuild()
{
	return true;
}

void SnGradient::OnMeasure(HDC ic)
{
	_size.cx = 400;
	_size.cy = 20;
}

void SnGradient::OnLayout(ScarabLayoutContext* layoutContext)
{
}

namespace
{
	void DrawRect(HDC hdc, int x0, int y0, int x1, int y1, float r0, float g0, float b0, float r1, float g1, float b1)
	{
		TRIVERTEX v[2];

		v[0].x		= x0;
		v[0].y		= y0;
		v[0].Red	= Clamp<int>((int)(r0 * 0xFFFF), 0, 0xFFFF);
		v[0].Green	= Clamp<int>((int)(g0 * 0xFFFF), 0, 0xFFFF);
		v[0].Blue	= Clamp<int>((int)(b0 * 0xFFFF), 0, 0xFFFF);
		v[0].Alpha	= 0xFFFF;

		v[1].x		= x1;
		v[1].y		= y1;
		v[1].Red	= Clamp<int>((int)(r1 * 0xFFFF), 0, 0xFFFF);
		v[1].Green	= Clamp<int>((int)(g1 * 0xFFFF), 0, 0xFFFF);
		v[1].Blue	= Clamp<int>((int)(b1 * 0xFFFF), 0, 0xFFFF);
		v[1].Alpha	= 0xFFFF;

		GRADIENT_RECT r;

		r.UpperLeft		= 0;
		r.LowerRight	= 1;

		GradientFill(hdc, v, 2, &r, 1, GRADIENT_FILL_RECT_H);
	}

	void GetValues(SnGradient* gr, Xt::DomElement* root, float& t, float& r, float& g, float& b)
	{
		if (Xt::DomComponent* com = AsComponent(root))
			root = &com->data;

		t = Clamp((float)atof(AsText(FindElementByPath(root, gr->GetString("Gradient", "time", "time")))), 0.0f, 1.0f);
		r = Clamp((float)atof(AsText(FindElementByPath(root, gr->GetString("Gradient", "r", "r")))), 0.0f, 1.0f);
		g = Clamp((float)atof(AsText(FindElementByPath(root, gr->GetString("Gradient", "g", "g")))), 0.0f, 1.0f);
		b = Clamp((float)atof(AsText(FindElementByPath(root, gr->GetString("Gradient", "b", "b")))), 0.0f, 1.0f);
	}
}

void SnGradient::OnPaint(HDC hdc, const WinRect& rcPaint)
{
	ScarabNode::OnPaint(hdc, rcPaint);

	DrawRect(hdc, _client.left, _client.top, _client.right, _client.bottom - 5, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

	if (Xt::DomBlock* block = AsBlock(_dom))
	{
		if (Xt::DomArray* array = AsArray(FindElementByPath(block, GetString("Gradient", "path"))))
		{
			if (Count(array) > 0)
			{
				float oldT, oldR, oldG, oldB;

				GetValues(this, At(array, 0), oldT, oldR, oldG, oldB);

				oldT = 0.0f;

				int width = _client.Width() - 2;

				for(size_t i = 0; i < Count(array); i++)
				{
					float t, r, g, b;

					GetValues(this, At(array, i), t, r, g, b);

					int x0 = _client.left + 1 + (int)(width * oldT);
					int x1 = _client.left + 1 + (int)(width * t);

					DrawRect(hdc, x0, _client.top + 1, x1, _client.bottom - 6, oldR, oldG, oldB, r, g, b);

					oldT = t;
					oldR = r;
					oldG = g;
					oldB = b;
				}

				int x0 = _client.left + 1 + (int)(width * oldT);

				DrawRect(hdc, x0, _client.top + 1, _client.right - 1, _client.bottom - 6, oldR, oldG, oldB, oldR, oldG, oldB);

				// Dots

				for(size_t i = 0; i < Count(array); i++)
				{
					float t, r, g, b;

					GetValues(this, At(array, i), t, r, g, b);

					int x = _client.left + 1 + (int)(width * t);

					DrawRect(hdc, x - 2, _client.bottom - 4, x + 2, _client.bottom, 0, 0, 0, 0, 0, 0);
					DrawRect(hdc, x - 1, _client.bottom - 3, x + 1, _client.bottom - 1, r, g, b, r, g, b);
				}
			}
		}
	}
}

void SnGradient::SetDom(Xt::DomBlock* block)
{
	Monitor(&_dom, block);

	SyncControls();
}

void SnGradient::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGradient* swatch)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == monitor->element))
		swatch->SetDom(0);
	else
		swatch->SyncControls();
}

void SnGradient::SyncControls()
{
	if (ScarabPanel* panel = Panel())
	{
		if (*panel)
			InvalidateRect(*panel, &_client, FALSE);
	}
}