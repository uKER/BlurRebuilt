#include "stdafx.h"
#include "attk/Attk.h"

namespace AttkUt
{

C_ImageAdjustmentCurve<Point2> CreateThresholdCurve(
	U32 threshold)
{
	C_ImageAdjustmentCurve<Point2> curve;
	curve.SetCurveDegree(1);
	curve.AddControlVertex(Point2(0.f, 0.f));
	curve.AddControlVertex(Point2(((F32)threshold)/255.f, 0.f));
	curve.AddControlVertex(Point2(((F32)threshold)/255.f, 1.f));
	curve.AddControlVertex(Point2(1.f, 1.f));

	return curve;


}

C_ImageAdjustmentCurve<Point2> CreateContrastEnhancementCurve(
	U32 pivot,
	U32 width)
{
	C_ImageAdjustmentCurve<Point2> curve;
	curve.SetCurveDegree(2);
	width = Maths::Min(pivot, 255 - pivot);

	curve.AddControlVertex(Point2(((F32)(pivot-width))/255.f, 0.f));
	curve.AddControlVertex(Point2((F32)(pivot)/255.f, 0.f));
	curve.AddControlVertex(Point2((F32)(pivot)/255.f, 1.f));
	curve.AddControlVertex(Point2(((F32)(pivot+width))/255.f, 1.f));

	curve.AddKnot(0.f);
	curve.AddKnot(0.f);
	curve.AddKnot(0.f);
	curve.AddKnot(1.f);
	curve.AddKnot(1.f);
	curve.AddKnot(1.f);

	return curve;

}

}

