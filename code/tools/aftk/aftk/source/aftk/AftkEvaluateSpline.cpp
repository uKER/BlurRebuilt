#include <stdafx.h>
#include "AftkEvaluateSpline.h"
#include "AftkMoveToMercury.h"

using namespace Aftk;

C_EvaluateSpline::C_EvaluateSpline()
{
	m_evaluationClass = E_EvaluationClass_Unknown;
	m_numSubDivisions = 0;
	m_pSplineEvaluater = 0;
	m_pSplineLengthConverter = 0;
}

C_EvaluateSpline::~C_EvaluateSpline()
{
}

bool C_EvaluateSpline::Create(const Aftk::C_SceneSplineNode& spline, U32 numSubDivisions)
{
	m_numSubDivisions = numSubDivisions;
	U32 roundedUpSubDiv = (S32)floorf(m_numSubDivisions + 0.5f);

	if (spline.GetDegree() == 0)
	{
		return false;
	}
	//Linear Spline
	else if (spline.GetDegree() == 1)
	{
		m_evaluationClass = E_EvaluationClass_Linear;

		m_pSplineEvaluater = 
			new LinearSpline<Position3>(
				&spline.GetCVArray()[0], 
				spline.GetCVArray().Size());
		m_pSplineLengthConverter = 
			new SplineLengthConverter<LinearSpline<Aftk::Position3> >(
				(LinearSpline<Position3>*)m_pSplineEvaluater,
				roundedUpSubDiv);
	}
	//BSpline
	else
	{
		m_evaluationClass = E_EvaluationClass_BSpline;

		BSpline<Position3>* pSpline = 
			new BSpline<Position3>(
				&spline.GetCVArray()[0], 
				&spline.GetKnotArray()[0], 
				spline.GetCVArray().Size());
		m_pSplineEvaluater = pSpline;

		SplineLengthConverter<BSpline<Aftk::Position3> >* pSplineLengthConverter = 
			new SplineLengthConverter<BSpline<Aftk::Position3> >(
				(BSpline<Position3>*)m_pSplineEvaluater,
				roundedUpSubDiv);
		m_pSplineLengthConverter = pSplineLengthConverter;
	}

	return true;
}

void C_EvaluateSpline::Destroy()
{
	switch (m_evaluationClass)
	{
	case E_EvaluationClass_Linear:
		delete (LinearSpline<Position3>*)m_pSplineEvaluater;
		delete (SplineLengthConverter<LinearSpline<Aftk::Position3> >*)m_pSplineLengthConverter;
		break;
	case E_EvaluationClass_BSpline:
		delete (BSpline<Position3>*)m_pSplineEvaluater;
		delete (SplineLengthConverter<BSpline<Aftk::Position3> >*)m_pSplineLengthConverter;
		break;

	default:
		break;
	}
}

C_EvaluateSpline::E_EvaluationClass C_EvaluateSpline::GetEvaluationClass() const
{
	return m_evaluationClass;
}

Position3 C_EvaluateSpline::GetPosition(F32 u) const
{
	switch (m_evaluationClass)
	{
	case E_EvaluationClass_Linear:
		return ((LinearSpline<Aftk::Position3>*)m_pSplineEvaluater)->Position(u);
		break;
	case E_EvaluationClass_BSpline:
		return ((BSpline<Aftk::Position3>*)m_pSplineEvaluater)->Position(u);

	default:
		break;
	}

	return Position3(0,0,0);
}

F32 C_EvaluateSpline::GetLength(F32 u) const
{
	switch (m_evaluationClass)
	{
	case E_EvaluationClass_Linear:
		return ((SplineLengthConverter<LinearSpline<Aftk::Position3> >*)m_pSplineLengthConverter)->ConvertLength(u);
		break;
	case E_EvaluationClass_BSpline:
		return  ((SplineLengthConverter<BSpline<Aftk::Position3> >*)m_pSplineLengthConverter)->ConvertLength(u);

	default:
		break;
	}

	return 0.f;
}

F32 C_EvaluateSpline::GetTotalLength() const
{
	switch (m_evaluationClass)
	{
	case E_EvaluationClass_Linear:
		{
			SplineLengthConverter<LinearSpline<Aftk::Position3> >* pSplineLengthConv = 
				(SplineLengthConverter<LinearSpline<Aftk::Position3> >*)m_pSplineLengthConverter;
		
			return pSplineLengthConv->TotalLength();
		}
	case E_EvaluationClass_BSpline:
		{
			SplineLengthConverter<BSpline<Aftk::Position3> >* pSplineLengthConv = 
				(SplineLengthConverter<BSpline<Aftk::Position3> >*)m_pSplineLengthConverter;

			return pSplineLengthConv->TotalLength();
		}

	default:
		break;
	}

	return 0.f;
}

F32 C_EvaluateSpline::GetEvalationStepSize() const
{
	switch (m_evaluationClass)
	{
	case E_EvaluationClass_Linear:
	case E_EvaluationClass_BSpline:
		{
			F32 length = GetTotalLength();
			return length / m_numSubDivisions;
		}

	default:
		break;
	}

	return 0.f;
}
