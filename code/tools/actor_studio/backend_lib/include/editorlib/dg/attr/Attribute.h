#ifndef EDITOR_LIB_ATTRIBUTE_H
#define EDITOR_LIB_ATTRIBUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <core/maths/storage.h>
#include <misc_shared/Variant/Variant.h>
#include <EditorLib/Dg/Attr/IAttribute.h>

namespace EditorLib
{
	class C_Attribute : public I_Attribute
	{
	public:
		///\brief Constructor.
		///
		C_Attribute();

		///\brief Destructor.
		///
		~C_Attribute();

		///\brief Copy constructor.
		///
		///@param rhs Class to copy.
		C_Attribute(const C_Attribute &rhs); 

		///\brief assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		const C_Attribute& operator=(const C_Attribute &rhs);

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator==(const C_Attribute &rhs) const;

		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		bool operator!= (const C_Attribute &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store attributes within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		bool operator<(const C_Attribute &rhs) const;

		///\brief Sets the name of this attribute.
		///
		///@param name Attribute name.
		void SetName(const Btl::String& name);

		void SetValue(U32 value);
		void SetValue(U64 value);
		void SetValue(S32 value);
		void SetValue(S64 value);
		void SetValue(F32 value);
		void SetValue(F64 value);
		void SetValue(bool value);
		void SetValue(const Btl::String& value);
		void SetValue(const MathsStorage::Matrix44& value);

		///\brief Clear the contents of this attribute.
		///
		void Clear();

	//From I_Attribute
	public:
		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		const char* GetName() const;

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		INLINE virtual void GetValue(U32& value, bool update = true) const;
		INLINE virtual void GetValue(U64& value, bool update = true) const;
		INLINE virtual void GetValue(S32& value, bool update = true) const;
		INLINE virtual void GetValue(S64& value, bool update = true) const;
		INLINE virtual void GetValue(F32& value, bool update = true) const;
		INLINE virtual void GetValue(F64& value, bool update = true) const;
		INLINE virtual void GetValue(bool& value, bool update = true) const;
		INLINE virtual void GetValue(Btl::String& value, bool update = true) const;
		INLINE virtual void GetValue(const Btl::String*& value, bool update = true) const;
		INLINE virtual void GetValue(MathsStorage::Matrix44*& value, bool update = true) const;
		INLINE virtual void GetValue(const MathsStorage::Matrix44*& value, bool update = true) const;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		Util::E_VarType GetDataType() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		Btl::String					m_name;
		Util::C_Variant				m_variant;
	#endif
	};

	typedef Btl::Vector<C_Attribute> AttributeArray;

	#include "Attribute.inl"
}

#endif
