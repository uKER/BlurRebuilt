#ifndef CURVE_CURVE_WRAPPER_H
#define CURVE_CURVE_WRAPPER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename CURVE>
	class C_CurveWrapper
	{
	public:
		C_CurveWrapper(typename CURVE::FlagsType flags) : m_curve(flags) {}

		~C_CurveWrapper() {}

		void Process(const typename CURVE::StorageType* pData) const
		{
			const_cast<C_CurveWrapper*>(this)->m_curve.SetData(const_cast<typename CURVE::StorageType*>(pData));
			m_curve.Process();
		}

		typename CURVE::VecType Evaluate(
			const typename CURVE::StorageType* pData, F32 u) const
		{
			const_cast<C_CurveWrapper*>(this)->m_curve.SetData(const_cast<typename CURVE::StorageType*>(pData));
			return m_curve.Evaluate(u);
		}

		typename CURVE::VecType EvaluateTangent(
			const typename CURVE::StorageType* pData, F32 u) const
		{
			const_cast<C_CurveWrapper*>(this)->m_curve.SetData(const_cast<typename CURVE::StorageType*>(pData));
			return m_curve.EvaluateTangent(u);
		}

		F32 GetMinU(
			const typename CURVE::StorageType* pData) const
		{
			const_cast<C_CurveWrapper*>(this)->m_curve.SetData(const_cast<typename CURVE::StorageType*>(pData));
			return m_curve.GetMinU();
		}

		F32 GetMaxU(
			const typename CURVE::StorageType* pData) const
		{
			const_cast<C_CurveWrapper*>(this)->m_curve.SetData(const_cast<typename CURVE::StorageType*>(pData));
			return m_curve.GetMaxU();
		}

	private:
		CURVE	m_curve;
	};
}

#endif
