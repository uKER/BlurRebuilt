#ifndef EDITOR_LIB_ARGUMENT_H
#define EDITOR_LIB_ARGUMENT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/utility/string.h>
#include <Misc_Shared/Variant/Variant.h>

namespace EditorLib
{

class C_Argument
{
public:
	enum E_Required
	{
		E_Required_False,
		E_Required_True
	};

	C_Argument(
		const char shortName, 
		const Btl::String& longName,
		const Btl::String& help,
		E_Required isRequired);

	C_Argument(const C_Argument& rhs);

	~C_Argument();

	const C_Argument& operator=(const C_Argument& rhs);

	char GetShortName() const;

	const Btl::String& GetLongName() const;

	const Btl::String& GetHelp() const;

	bool IsRequired() const;

	const VarTypeArray& GetDataTypes() const;

	VarTypeArray& GetDataTypes();

private:
	char			m_shortName;
	Btl::String		m_longName;
	Btl::String		m_help;
	E_Required		m_isRequired;
	VarTypeArray	m_dataTypes;
};

}

#endif