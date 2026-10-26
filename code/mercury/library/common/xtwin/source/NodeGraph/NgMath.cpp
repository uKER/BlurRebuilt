#include "Core/XtWinPch.h"
#include "NgImpl.h"

NgColour HsvToRgb(const NgColour& hsv)
{
	double fi;
	double ff = modf(fmod(fmod((double)hsv.r, 1.0) + 1.0, 1.0) * 6.0, &fi);
	int i = (int)fi;

	float v = hsv.b;
	float p = (float)(v * (1.0 - (hsv.g)));
	float q = (float)(v * (1.0 - (hsv.g * ff)));
	float r = (float)(v * (1.0 - (hsv.g * (1.0 - ff))));

	switch(i)
	{
		default:	return NgColour(v, r, p, hsv.a);
		case 1:		return NgColour(q, v, p, hsv.a);
		case 2:		return NgColour(p, v, r, hsv.a);
		case 3:		return NgColour(p, q, v, hsv.a);
		case 4:		return NgColour(r, p, v, hsv.a);
		case 5:		return NgColour(v, p, q, hsv.a);
	}
}

NgColour RgbToHsv(const NgColour& rgb)
{
	float min = Min(rgb.r, Min(rgb.g, rgb.b));
	float max = Max(rgb.r, Max(rgb.g, rgb.b));

	float delta	= max - min;
	float s		= delta / (max + 1e-6f);

	float h;

	if (rgb.r == max)
	{
		h = (rgb.g - rgb.b) / (delta + 1e-6f);
	}
	else if (rgb.g == max)
	{
		h = 2 + (rgb.b - rgb.r) / (delta + 1e-6f);
	}
	else
	{
		h = 4 + (rgb.r - rgb.g) / (delta + 1e-6f);
	}

	h *= (1.0f / 6.0f);

	return NgColour(h, s, max, rgb.a);
}

NgColour Lighten(const NgColour& colour, float v)
{
	return NgColour(v) + (colour * (1.0f - v));
}

NgColour Lerp(const NgColour& colour0, const NgColour& colour1, float a)
{
	return NgColour(
			Lerp(colour0.r, colour1.r, a),
			Lerp(colour0.g, colour1.g, a),
			Lerp(colour0.b, colour1.b, a),
			Lerp(colour0.a, colour1.a, a)
		);
}