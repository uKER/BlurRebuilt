#include "stdafx.h"
#include <EditorLib/Command/Argument/Argument.h>

namespace EditorLib
{

C_Argument::C_Argument(
	const char shortName,
	const Btl::String& longName,
	const Btl::String& help,
	E_Required isRequired)
{
	m_shortName = shortName;
	m_longName = longName;
	m_help = help;
	m_isRequired = isRequired;
}

C_Argument::C_Argument(const C_Argument& rhs)
{
	*this = rhs;
}

C_Argument::~C_Argument()
{
}

const C_Argument& C_Argument::operator=(const C_Argument& rhs)
{
	m_shortName = rhs.m_shortName;
	m_longName = rhs.m_longName;
	m_help = rhs.m_help;
	m_isRequired = rhs.m_isRequired;
	m_dataTypes = rhs.m_dataTypes;

	return *this;
}

char C_Argument::GetShortName() const
{
	return m_shortName;
}

const Btl::String& C_Argument::GetLongName() const
{
	return m_longName;
}

const Btl::String& C_Argument::GetHelp() const
{
	return m_help;
}

bool C_Argument::IsRequired() const
{
	return m_isRequired == E_Required_True ? true : false;
}

const VarTypeArray& C_Argument::GetDataTypes() const
{
	return m_dataTypes;
}

VarTypeArray& C_Argument::GetDataTypes()
{
	return m_dataTypes;
}

}
