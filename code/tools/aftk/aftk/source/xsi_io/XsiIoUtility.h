#ifndef XSI_UTILITY_H
#define XSU_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace XsiIo
{
	class XsiVectorWrapper : public CSIBCVector4D
	{
	public:
		// access to elements
		template <int I>
		float& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(&m_fX + I);}

		template <int I>
		const float& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(&m_fX + I);} 

		// type and size
		typedef float DataType; 
		const static int SIZE = 4;

		// types for constructors
		typedef Ut::NullType type1;
		typedef Ut::NullType type2;
	};

	typedef UtMaths::MVector<XsiVectorWrapper> XsiVector;

	class XsiMatrixWrapper : public CSIBCMatrix4x4
	{
	public:
		// type and size
		typedef XsiVector DataType; 
		const static int SIZE = 4;

		// access to elements
		template <int I>
		typename DataType& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *((DataType*) Raw() + I);}

		template <int I>
		const typename DataType& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *((const DataType*) (const_cast<XsiMatrixWrapper*>(this)->Raw()) + I);} 

		// types for constructors
		typedef Ut::NullType type1;
		typedef Ut::NullType type2;
	};

	typedef UtMaths::MMatrix<XsiMatrixWrapper> XsiMatrix;
}

#endif
