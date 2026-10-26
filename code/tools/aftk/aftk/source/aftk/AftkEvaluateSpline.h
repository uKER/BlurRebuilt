#ifndef AFTK_EVALUATE_SPLINE_H
#define AFTK_EVALUATE_SPLINE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Aftk
{
	class C_EvaluateSpline
	{
	public:
		enum E_EvaluationClass
		{
			E_EvaluationClass_Linear,
			E_EvaluationClass_BSpline,

			E_EvaluationClass_Unknown
		};

		C_EvaluateSpline();

		~C_EvaluateSpline();

		bool Create(const Aftk::C_SceneSplineNode& spline, U32 numSubDivisions);

		void Destroy();

		E_EvaluationClass GetEvaluationClass() const;

		U32 GetNumSubDivisions() const;

		Position3 GetPosition(F32 u) const;

		F32 GetLength(F32 u) const;

		F32 GetTotalLength() const;

		F32 GetEvalationStepSize() const;

	private:
		E_EvaluationClass	m_evaluationClass;
		U32					m_numSubDivisions;
		void*				m_pSplineEvaluater;
		void*				m_pSplineLengthConverter;
	};
}

#endif
