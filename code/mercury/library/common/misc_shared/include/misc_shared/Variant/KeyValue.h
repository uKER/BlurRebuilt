#ifndef MISC_SHARED_KEY_VALUE_H
#define MISC_SHARED_KEY_VALUE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "Variant.h"

namespace Util
{
	class C_KeyValue : public C_Variant
	{
	public:
		///\brief Constructor.
		///
		INLINE C_KeyValue();

		///\brief Destructor.
		///
		INLINE ~C_KeyValue();

		///\brief Copy constructor.
		///
		///@param rhs Class to copy.
		INLINE C_KeyValue(const C_KeyValue &rhs); 

		///\brief assignment operator.
		///
		///@param rhs Class to copy.
		///@return Reference to this class.
		INLINE const C_KeyValue& operator=(const C_KeyValue &rhs);

		///\brief Equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		INLINE bool operator==(const C_KeyValue &rhs) const;

		///\brief Negated equivilance test operator.
		///
		///@param rhs The class to test against.
		///@return bool Test result.
		INLINE bool operator!= (const C_KeyValue &rhs) const;

		///\brief Less than operator.
		///
		///This test is required in order to be able to store within a sorted container.
		///
		///@param rhs The class to test against
		///@return bool Test result.
		INLINE bool operator<(const C_KeyValue &rhs) const;

		///\brief Sets the name.
		///
		///@param name The name.
		INLINE void SetName(const Btl::String& name);

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		INLINE const Btl::String& GetName() const;

	#ifndef DOXYGEN_SHOULD_SKIP_THIS
	protected:
		Btl::String				m_name;
	#endif
	};

	#include "KeyValue.inl"

	typedef Btl::Vector<C_KeyValue> KeyValueArray;
	typedef const Btl::Vector<C_KeyValue> ConstKeyValueArray;
}

#endif
