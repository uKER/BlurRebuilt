#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "ScarabImpl.h"

/*

	[ColourSwatch(
			red:fieldName,
			green:fieldName,
			blue:fieldName,
			hue:fieldName,
			sat:fieldName,
			val:fieldName,
			alpha:fieldName,
			intensity:fieldName,		// Colour intensity multiplier (for hdr >1.0f colours)
			maxIntensity:8.0,			// Max value that intensity can be set to (defaults to 8)
			gamma:2.2,					// Gamma value to apply to colour display, set to 1.0 for linear colour, defaults to 2.2
			scale:1.0,					// Scale to apply to colour values before reading/writing to fields, (e.g. set to 255 for byte colour values)
			rgb:hex,					// Hex rgb value
			argb:hex,					// Hex argb value
		)]

*/

SnColourSwatch::SnColourSwatch(LfControl* field) :
	SnControl(field),
	_dom(DomCallback, this),
	_ignoreNotifications(0),
	_idSwatch(this),
	_wndSwatch(this),
	_swatchType(kNone),
	_showAlpha(false)
{
	bool red	= FindParam("ColourSwatch", "red") != 0;
	bool green	= FindParam("ColourSwatch", "green") != 0;
	bool blue	= FindParam("ColourSwatch", "blue") != 0;
	bool hue	= FindParam("ColourSwatch", "hue") != 0;
	bool sat	= FindParam("ColourSwatch", "sat") != 0;
	bool val	= FindParam("ColourSwatch", "val") != 0;
	bool alpha	= FindParam("ColourSwatch", "alpha") != 0;
	bool rgb	= FindParam("ColourSwatch", "rgb") != 0;
	bool argb	= FindParam("ColourSwatch", "argb") != 0;

	if (red && !green && !blue)
		_swatchType = kRed;
	else if (!red && green && !blue)
		_swatchType = kGreen;
	if (!red && !green && blue)
		_swatchType = kBlue;
	else if (red || green || blue)
		_swatchType = kRgb;
	else if (hue && !sat && !val)
		_swatchType = kHue;
	else if (!hue && !sat && val)
		_swatchType = kVal;
	else if (hue || sat || val)
		_swatchType = kHsv;
	else if (rgb)
		_swatchType = kHexRgb;
	else if (argb)
		_swatchType = kHexRgbA;

	_showAlpha = alpha || argb;
}

SnColourSwatch::~SnColourSwatch()
{
}

void SnColourSwatch::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(parentDom);
}

void SnColourSwatch::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(AcquireBlock(parentDom, index));
}

bool SnColourSwatch::OnBuild()
{
	if (!_idSwatch.Allocate())
		return false;

	if ((HWND)_wndSwatch == 0)
	{
		if (!_wndSwatch.Create(*Panel(), _idSwatch.IdAsHMenu()))
			return false;
	}

	return true;
}

void SnColourSwatch::OnMeasure(HDC ic)
{
	_size.cx = ((_swatchType != kNone) && _showAlpha) ? 40 : 20;
	_size.cy = 20;
}

void SnColourSwatch::OnLayout(ScarabLayoutContext* layoutContext)
{
	layoutContext->SetWindowPos(_wndSwatch, _client.left, _client.top, _client.Width(), _client.Height(), SWP_SHOWWINDOW);
}

void SnColourSwatch::SetDom(Xt::DomBlock* block)
{
	Monitor(&_dom, block);

	SyncControls();
}

void SnColourSwatch::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnColourSwatch* swatch)
{
	if ((event->code == Xt::kDomRemoved) && (event->from == monitor->element))
		swatch->SetDom(0);
	else
		swatch->SyncControls();
}

namespace
{
	double GetValue(Xt::DomBlock* block, LarvaAttrList* al, const char* attrName, const char* paramName, double defValue = 0.0f)
	{
		if (LarvaParam* param = al->FindParam(attrName, paramName))
			return atof(AsText(At(block, param->AsString())));

		return defValue;
	}

	void SetValue(Xt::DomBlock* block, LarvaAttrList* al, const char* attrName, const char* paramName, XtString value)
	{
		if (LarvaParam* param = al->FindParam(attrName, paramName))
		{
			if (XtsIsIdentifier(param->AsString()))
				Set(block, param->AsString(), value);
		}
	}
}

void SnColourSwatch::SyncControls()
{
	if (_ignoreNotifications <= 0)
	{
		_ignoreNotifications++;

		ColourSwatchData csd;

		if (Xt::DomBlock* block = AsBlock(_dom))
		{
			csd.intensity = (float)GetValue(block, &_field->attrs, "ColourSwatch", "intensity");

			switch(_swatchType)
			{
				case kNone:
					csd.alpha	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "alpha");
				break;

				case kRed:
				case kGreen:
				case kBlue:
				case kRgb:
					csd.red		= (float)GetValue(block, &_field->attrs, "ColourSwatch", "red");
					csd.green	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "green");
					csd.blue	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "blue");
					csd.alpha	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "alpha");
				break;

				case kHue:
				case kVal:
				case kHsv:
					csd.red		= (float)GetValue(block, &_field->attrs, "ColourSwatch", "hue");
					csd.green	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "sat");
					csd.blue	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "val");
					csd.alpha	= (float)GetValue(block, &_field->attrs, "ColourSwatch", "alpha");
				break;

				case kHexRgb:
				{
					XtU32 hex = (XtU32)_strtoi64(AsText(At(block, GetString("ColourSwatch", "rgb", "colour"))), 0, 16);

					csd.red		= ((hex & 0xFF0000) >> 16) / 255.0f;
					csd.green	= ((hex & 0xFF00) >> 8) / 255.0f;
					csd.blue	= (hex & 0xFF) / 255.0f;
					csd.alpha	= 1.0f;
				}
				break;

				case kHexRgbA:
				{
					XtU32 hex = (XtU32)_strtoi64(AsText(At(block, GetString("ColourSwatch", "argb", "colour"))), 0, 16);

					csd.red		= ((hex & 0xFF0000) >> 16) / 255.0f;
					csd.green	= ((hex & 0xFF00) >> 8) / 255.0f;
					csd.blue	= (hex & 0xFF) / 255.0f;
					csd.alpha	= ((hex & 0xFF000000) >> 24) / 255.0f;
				}
				break;
			}

			float invScale	= 1.0f / (float)GetFloat("ColourSwatch", "scale", 1.0f);
			float gamma		= (float)GetFloat("ColourSwatch", "gamma", 2.2f);

			csd.red		*= invScale;
			csd.green	*= invScale;
			csd.blue	*= invScale;
			csd.alpha	*= invScale;

			switch(_swatchType)
			{
				case kHue:
				case kVal:
				case kHsv:
				{
					NgColour rgb = HsvToRgb(NgColour(csd.red, csd.green, csd.blue));

					csd.red		= rgb.r;
					csd.green	= rgb.g;
					csd.blue	= rgb.b;
				}
				break;
			}

			csd.red		= powf(csd.red, 2.2f / gamma);
			csd.green	= powf(csd.green, 2.2f / gamma);
			csd.blue	= powf(csd.blue, 2.2f / gamma);
		}

		_wndSwatch.SetData(&csd);
		_wndSwatch.SetDisplayColour(_swatchType != kNone);
		_wndSwatch.SetDisplayAlpha(_showAlpha);
		_wndSwatch.SetDisplayIntensity(FindParam("ColourSwatch", "intensity") != 0);
		_wndSwatch.SetMaxIntensity((float)GetFloat("ColourSwatch", "maxIntensity", 8.0));

		_ignoreNotifications--;
	}
}

SnColourSwatch::Swatch::Swatch(SnColourSwatch* swatch) :
	_swatch(swatch)
{
}

void SnColourSwatch::Swatch::OnChanged()
{
	if (_swatch->_ignoreNotifications <= 0)
	{
		_swatch->_ignoreNotifications++;

		if (Xt::DomBlock* block = AsBlock(_swatch->_dom))
		{
			ColourSwatchData csd;

			GetData(&csd);

			float scale = (float)_swatch->GetFloat("ColourSwatch", "scale", 1.0f);
			float gamma = (float)_swatch->GetFloat("ColourSwatch", "gamma", 2.2f);

			csd.red		= powf(csd.red, gamma / 2.2f);
			csd.green	= powf(csd.green, gamma / 2.2f);
			csd.blue	= powf(csd.blue, gamma / 2.2f);

			switch(_swatch->_swatchType)
			{
				case kHue:
				case kVal:
				case kHsv:
				{
					NgColour rgb = RgbToHsv(NgColour(csd.red, csd.green, csd.blue));

					csd.red		= rgb.r;
					csd.green	= rgb.g;
					csd.blue	= rgb.b;
				}
				break;
			}

			csd.red		*= scale;
			csd.green	*= scale;
			csd.blue	*= scale;
			csd.alpha	*= scale;

			SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "intensity", Format("%g", csd.intensity));

			switch(_swatch->_swatchType)
			{
				case kNone:
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "alpha",	Format("%g", csd.alpha));
				break;

				case kRed:
				case kGreen:
				case kBlue:
				case kRgb:
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "red",		Format("%g", csd.red));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "green",	Format("%g", csd.green));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "blue",	Format("%g", csd.blue));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "alpha",	Format("%g", csd.alpha));
				break;

				case kHue:
				case kVal:
				case kHsv:
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "hue",		Format("%g", csd.red));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "sat",		Format("%g", csd.green));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "val",		Format("%g", csd.blue));
					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "alpha",	Format("%g", csd.alpha));
				break;

				case kHexRgb:
				{
					XtU32 rgb = 0;

					rgb |= Clamp((XtU32)(csd.red * 255.0f), 0u, 255u);
					rgb |= Clamp((XtU32)(csd.green * 255.0f), 0u, 255u) << 8;
					rgb |= Clamp((XtU32)(csd.blue * 255.0f), 0u, 255u) << 16;

					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "rgb", Format("%06X", rgb));
				}
				break;

				case kHexRgbA:
				{
					XtU32 argb = 0;

					argb |= Clamp((XtU32)(csd.red * 255.0f), 0u, 255u);
					argb |= Clamp((XtU32)(csd.green * 255.0f), 0u, 255u) << 8;
					argb |= Clamp((XtU32)(csd.blue * 255.0f), 0u, 255u) << 16;
					argb |= Clamp((XtU32)(csd.alpha * 255.0f), 0u, 255u) << 24;

					SetValue(block, &_swatch->_field->attrs, "ColourSwatch", "argb", Format("%08X", argb));
				}
				break;
			}
		}

		_swatch->_ignoreNotifications--;
	}
}