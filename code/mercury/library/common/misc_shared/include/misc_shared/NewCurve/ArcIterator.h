#ifndef CURVE_ARC_ITERATOR_H
#define CURVE_ARC_ITERATOR_H

//Accepts a reparameterised curve

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	class C_ArcIterator
	{
	public:
		C_ArcIterator(
			const I_Curve<VECTYPE>& curve,
			const C_Reparameterise& table,
			F32 arcSegmentLength);

		///\brief Copy constructor.
		///
		///@param rhs The iterator to copy.
		C_ArcIterator(const C_ArcIterator<VECTYPE>& rhs);

		~C_ArcIterator();

		///\brief Asignment operator.
		///
		///@param rhs The iterator to copy.
		const C_ArcIterator<VECTYPE>& operator=(const C_ArcIterator<VECTYPE>& rhs);

		operator const VECTYPE*();

		//Pre-increment
		const VECTYPE* operator ++();

		//Post-increment iterator
		const VECTYPE* operator ++(int);

		///\brief Clears the iterator to its initial state.
		///
		void Reset();

		U32 GetNumPoints() const;

	private:
		const I_Curve<VECTYPE>*		m_pCurve;
		const C_Reparameterise*		m_pTable;
		F32							m_arcSegmentLength;
		F32							m_arcLength;
		
		U32							m_currentSegment;
		U32							m_numSegments;
		VECTYPE						m_currentValue;
	};

	#include "ArcIterator.inl"
}

#endif
