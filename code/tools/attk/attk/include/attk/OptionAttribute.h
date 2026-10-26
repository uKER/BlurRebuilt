#ifndef ATTK_OPTION_ATTRIBUTE_H
#define ATTK_OPTION_ATTRIBUTE_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Attk
{
	class C_OptionAttribute
	{
	public:
		enum E_ValueType
		{
			E_ValueType_U32,
			E_ValueType_F32,
			E_ValueType_String
		};

		C_OptionAttribute();
		C_OptionAttribute(const C_OptionAttribute& rhs);
		~C_OptionAttribute();

		const C_OptionAttribute& operator=(const C_OptionAttribute& rhs);

		void SetAttrType(U32 attrId);
		void SetValue(S32 value);
		void SetValue(F32 value);
		void SetValue(const std::string& value);

		U32 GetAttrType() const;
		E_ValueType GetValueType() const;
		
		void GetValue(S32& value) const;
		void GetValue(F32& value) const;
		void GetValue(std::string& value) const;

		void Clear();

	private:
	};
}

#endif
