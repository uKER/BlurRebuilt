#ifndef FBX_IO_UTILITY_H
#define FBX_IO_UTILITY_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <fbxsdk.h>

namespace FbxIo
{
	/*
	class FbxVectorWrapper : public KFbxVector4
	{
	public:
		// access to elements
		template <int I>
		float& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return (float&)GetAt(I);}

		template <int I>
		const float& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return (const float&)( const_cast<FbxVectorWrapper*>(this)->GetAt(I) );} 

		// type and size
		typedef float DataType; 
		const static int SIZE = 4;

		// types for constructors
		typedef Ut::NullType type1;
		typedef Ut::NullType type2;
	};

	typedef UtMaths::MVector<FbxVectorWrapper> FbxVector;

	
	class FbxMatrixWrapper : public KFbxXMatrix
	{
	public:
		// type and size
		typedef FbxVector DataType; 
		const static int SIZE = 4;

		// access to elements
		template <int I>
		typename DataType& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return GetColumn(I);}

		template <int I>
		const typename DataType& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return (const DataType&)(const_cast<FbxMatrixWrapper*>(this)->GetColumn(I));} 

		// types for constructors
		typedef Ut::NullType type1;
		typedef Ut::NullType type2;
	};

	typedef UtMaths::MMatrix<FbxMatrixWrapper> FbxMatrix;*/

	bool GetOptionValueString(const char* pStr, std::string &value);
	bool GetOptionValueBool(const char* pStr, bool &value);
	bool GetOptionValueF32(const char* pStr, F32 &value);
	bool GetOptionValueU32(const char* pStr, U32 &value);
}

#endif //FBX_IO_UTILITY_H
