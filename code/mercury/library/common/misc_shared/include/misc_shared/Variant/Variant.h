#ifndef MISC_SHARED_VARIANT_H
#define MISC_SHARED_VARIANT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/utility/string.h>
#include <core/maths/storage.h>

namespace Util
{
	enum E_VarType
	{
		E_VarType_Null,
		E_VarType_U32,
		E_VarType_U64,
		E_VarType_S32,
		E_VarType_S64,
		E_VarType_F32,
		E_VarType_F64,
		E_VarType_Boolean,
		E_VarType_String,
		E_VarType_Matrix,

		E_VarType_Unknown
	};

	template<typename DATA_TYPE>
	INLINE E_VarType GetVarType();
	template<>
	INLINE E_VarType GetVarType<U32>();
	template<>
	INLINE E_VarType GetVarType<U64>();
	template<>
	INLINE E_VarType GetVarType<S32>();
	template<>
	INLINE E_VarType GetVarType<S64>();
	template<>
	INLINE E_VarType GetVarType<F32>();
	template<>
	INLINE E_VarType GetVarType<F64>();
	template<>
	INLINE E_VarType GetVarType<bool>();
	template<>
	INLINE E_VarType GetVarType<Btl::String>();
	template<>
	INLINE E_VarType GetVarType<MathsStorage::Matrix44>();

	class C_Variant
	{
	public:
		INLINE C_Variant();
		INLINE C_Variant(const C_Variant& rhs);
		INLINE ~C_Variant();

		///\brief assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		INLINE const C_Variant& operator=(const C_Variant &rhs);

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		INLINE bool operator==(const C_Variant &rhs) const;

		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		INLINE bool operator!= (const C_Variant &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		INLINE bool operator<(const C_Variant &rhs) const;

		INLINE void SetValue(const U32 value);
		INLINE void SetValue(const U64 value);
		INLINE void SetValue(const S32 value);
		INLINE void SetValue(const S64 value);
		INLINE void SetValue(const F32 value);
		INLINE void SetValue(const F64 value);
		INLINE void SetValue(const bool value);
		INLINE void SetValue(const Btl::String& value);
		INLINE void SetValue(const MathsStorage::Matrix44& value);
		
		INLINE bool GetValue(U32& value) const;
		INLINE bool GetValue(U64& value) const;
		INLINE bool GetValue(S32& value) const;
		INLINE bool GetValue(S64& value) const;
		INLINE bool GetValue(F32& value) const;
		INLINE bool GetValue(F64& value) const;
		INLINE bool GetValue(bool& value) const;
		INLINE bool GetValue(Btl::String& value) const;
		INLINE bool GetValue(MathsStorage::Matrix44& value) const;

		INLINE E_VarType GetDataType() const;

		INLINE void Clear();

		void FromString(const Btl::String& str, E_VarType dataType);
		void ToString(Btl::String& str);

	protected:
		union C_Data
		{
			U32						u32;
			U64						u64;
			S32						s32;
			S64						s64;
			F32						f32;
			F64						f64;
			bool					boolean;
			Btl::String*			pString;
			MathsStorage::Matrix44*	pMatrix;
		};

		INLINE void				ClearHeapData();
		INLINE C_Data*			GetData();
		INLINE const C_Data*	GetData() const;

		E_VarType		m_type;
		U8				m_data[sizeof(C_Data)];
	};

	#include "Variant.inl"
}

#endif
