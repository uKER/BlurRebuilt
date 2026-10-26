#pragma once

#include <Xt/XtGame.h>

namespace RacingPhysics
{

//////////////////////////////////////////////////////////////////

struct LinearFunctionXt
{
	float Evaluate(float x) const
	{		
		float xs = (x - x0) / (x1 - x0);
		xs = Maths::Clamped(xs, 0.f, 1.f);
		int N = (dataPoints.count - 1);
		int i = (int) (xs * N);
		float f = (xs * N - i);
		float y = f * dataPoints[Maths::Min(i + 1, dataPoints.count - 1)] + (1.f - f) * dataPoints[i];
		return y * yScale;
	}

	float			x0;
	float			x1;
	float			yScale;
	XtArray<float>	dataPoints;
};

//////////////////////////////////////////////////////////////////


struct LinearGraphXt
{
	float Evaluate(float x) const
	{
		if (x < dataPoints[0].x)
			 return dataPoints[0].y * yScale;

		for (int i = 1; i < dataPoints.count; i++)
		{
			if (dataPoints[i].x > x)
			{
				float f = (x - dataPoints[i - 1].x) / (dataPoints[i].x - dataPoints[i - 1].x);
				float y = f * dataPoints[i].y + (1.f - f) * dataPoints[i - 1].y;
				return y * yScale;
			}
		}

		return dataPoints[dataPoints.count - 1].y * yScale;
	}

	struct Point {float x; float y;};

	float			yScale;
	XtArray<Point>	dataPoints;
};

//////////////////////////////////////////////////////////////////

struct LinearParameterXt
{
	LinearParameterXt() {}

	float Evaluate(float x, bool clamped) const
	{
		float f = (x - x0) / (x1 - x0);
		if (clamped) f = Maths::Clamped(f, 0.f, 1.f);
		//float f2 = f * f;
		return f * y1 + (1.f - f) * y0;
	}

	float	x0;
	float	y0;
	float	x1;
	float	y1;
};

}
