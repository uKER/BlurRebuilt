#ifndef TOOLS_ARGUMENT_H
#define TOOLS_ARGUMENT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Btl/Utility/String.h>
#include <Btl/Containers/Vector.h>
#include <Misc_Shared/Variant/Variant.h>

namespace Tools
{

struct C_ArgType
{
	C_ArgType(Util::E_VarType type_, bool multiArg_ = false)
	{
		type = type_;
		multiArg = multiArg_;
	}
	C_ArgType(const C_ArgType& rhs)
	{
		*this = rhs;
	}
	const C_ArgType& operator=(const C_ArgType& rhs)
	{
		type = rhs.type;
		multiArg = rhs.multiArg;
		return *this;
	}

	Util::E_VarType type;
	bool multiArg;
};
typedef Btl::Vector<C_ArgType>		ArgVarTypeArray;
typedef Btl::Vector<Btl::String>	ArgDefaultArray;

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

	const ArgDefaultArray& GetDefaults() const;

	ArgDefaultArray& GetDefaults();

	bool IsRequired() const;

	const ArgVarTypeArray& GetDataTypes() const;

	ArgVarTypeArray& GetDataTypes();

private:
	char			m_shortName;
	Btl::String		m_longName;
	Btl::String		m_help;
	E_Required		m_isRequired;
	Btl::Vector<Btl::String>		
					m_defaults;
	ArgVarTypeArray	m_dataTypes;
};

typedef Ut::UtList<const C_Argument> ArgList;

}

#endif