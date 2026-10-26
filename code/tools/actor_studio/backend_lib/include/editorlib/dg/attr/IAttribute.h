#ifndef EDITOR_LIB_I_ATTRIBUTE_H
#define EDITOR_LIB_I_ATTRIBUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	#include <Misc_Shared/Variant/Variant.h>

	class I_Attribute
	{
	public:
		virtual ~I_Attribute() {}

		///\brief Set the value of this attribute.
		///
		///@param value Attribute value.
		virtual void GetValue(U32& value, bool update = true) const = 0;
		virtual void GetValue(U64& value, bool update = true) const = 0;
		virtual void GetValue(S32& value, bool update = true) const = 0;
		virtual void GetValue(S64& value, bool update = true) const = 0;
		virtual void GetValue(F32& value, bool update = true) const = 0;
		virtual void GetValue(F64& value, bool update = true) const = 0;
		virtual void GetValue(bool& value, bool update = true) const = 0;
		virtual void GetValue(Btl::String& value, bool update = true) const = 0;
		virtual void GetValue(const Btl::String*& value, bool update = true) const = 0;
		virtual void GetValue(MathsStorage::Matrix44& value, bool update = true) const = 0;
		virtual void GetValue(const MathsStorage::Matrix44*& value, bool update = true) const = 0;

		///\brief Get the name of this attribute.
		///
		///@return Attribute name;
		virtual const char* GetName() const = 0;

		///\brief Get the value type of this attribute.
		///
		///This might be, U32, bool etc.
		virtual Util::E_VarType GetDataType() const = 0;
	};
}

#endif
