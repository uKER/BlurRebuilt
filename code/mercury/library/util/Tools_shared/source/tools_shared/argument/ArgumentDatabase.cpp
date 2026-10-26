#include "stdafx.h"

#include <cctype>

#include <core/maths/mathsutil.h>

#include <Tools_Shared/Argument/ArgumentDatabase.h>
#include <Tools_Shared/Argument/Argument.h>
#include <Tools_Shared/Argument/Syntax.h>


namespace Tools
{

C_ArgumentDatabase::C_ArgumentDatabase()
{
}

C_ArgumentDatabase::~C_ArgumentDatabase()
{
}

C_ArgumentDatabase::C_ArgumentDatabase(const C_ArgumentDatabase& rhs)
{
	*this = rhs;
}

const C_ArgumentDatabase& C_ArgumentDatabase::operator=(const C_ArgumentDatabase& rhs)
{
	m_argumentGroups = rhs.m_argumentGroups;
	return *this;
}

bool C_ArgumentDatabase::AddArgGroup(
	const C_Argument& flagArg, 
	const Btl::Vector<Btl::String>& commandArgs)
{
	//Validation for duplicate flags.
	for (Btl::Vector<C_ArgumentGroup>::ConstIterator it = m_argumentGroups.Begin(); 
		it != m_argumentGroups.End(); ++it)
	{
		const C_ArgumentGroup& arg = *it;
		if (flagArg.GetShortName() != '*')
		{
			if (arg.shortName == flagArg.GetShortName())
			{
				LOGERROR(g_log, "Arg group already processed! Short name %c does not match %c", arg.shortName, flagArg.GetShortName());
				return false;
			}
		}
		else if (flagArg.GetLongName() != "")
		{
			if (arg.longName == flagArg.GetLongName())
			{
				LOGERROR(g_log, "Arg group already processed! Long name %c does not match %c", arg.longName, flagArg.GetLongName());
				return false;
			}
		}
		else
		{
			LOGERROR(g_log, "Argument with no name encountered!");
			return false;
		}
	}

	//Store new group.
	m_argumentGroups.PushBack();
	C_ArgumentGroup& result = m_argumentGroups.Back();
	result.shortName = flagArg.GetShortName();
	result.longName = flagArg.GetLongName();

	S32 nData = commandArgs.Size();
	result.data.Resize(nData);

	const ArgVarTypeArray& dataTypes = flagArg.GetDataTypes();
	S32 count = nData < dataTypes.Size() ? nData : dataTypes.Size();
	for (S32 i = 0; i < count; ++i)
	{
		//Consume data for each data type entry.
		if (dataTypes[i].multiArg)
		{
			if (i != dataTypes.Size()-1)
			{
				LOGERROR(g_log, "MultiArg must be last type in an arguments data type list!");
				return false;
			}

			//Eat remaining data.
			for (S32 j = i; j < nData; ++j)
			{
				Util::C_KeyValue& attr = result.data[j];
				attr.FromString(commandArgs[j], dataTypes[i].type);
			}
			return true;
		}

		Util::C_KeyValue& attr = result.data[i];
		attr.FromString(commandArgs[i], dataTypes[i].type);
	}
	return true;
}

const C_ArgumentDatabase::C_ArgumentGroup* C_ArgumentDatabase::GetArgGroup(
	const char shortName) const
{
	//Examine short names.
	for (Btl::Vector<C_ArgumentGroup>::ConstIterator it = m_argumentGroups.Begin(); 
		it != m_argumentGroups.End(); ++it)
	{
		const C_ArgumentGroup& arg = *it;
		if (std::tolower(arg.shortName) == std::tolower(shortName))
			return &arg;
	}

	//Not found.
	return 0;
}

const C_ArgumentDatabase::C_ArgumentGroup* C_ArgumentDatabase::GetArgGroup(
	const Btl::String& longName) const
{
	//Examine long names.
	for (Btl::Vector<C_ArgumentGroup>::ConstIterator it = m_argumentGroups.Begin(); 
		it != m_argumentGroups.End(); ++it)
	{
		const C_ArgumentGroup& arg = *it;
		if (arg.longName.Lower() == longName.Lower())
			return &arg;
	}

	//Not found.
	return 0;
}

bool C_ArgumentDatabase::IsSet(const char shortName) const
{
	return GetArgGroup(shortName) ? true : false;
}

bool C_ArgumentDatabase::IsSet(const Btl::String& longName) const
{
	return GetArgGroup(longName) ? true : false;
}

const ArgDataArray* C_ArgumentDatabase::GetArgData(const char shortName) const
{
	const C_ArgumentGroup* pArg = GetArgGroup(shortName);
	if (!pArg)
		return 0;

	return (ArgDataArray*)&pArg->data;
}

const ArgDataArray* C_ArgumentDatabase::GetArgData(const Btl::String& longName) const
{
	const C_ArgumentGroup* pArg = GetArgGroup(longName);
	if (!pArg)
		return 0;

	return (ArgDataArray*)&pArg->data;
}


}
