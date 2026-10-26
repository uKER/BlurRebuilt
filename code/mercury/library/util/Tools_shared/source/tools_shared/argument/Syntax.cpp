#include "stdafx.h"
#include <Tools_Shared/Argument/Argument.h>
#include <Tools_Shared/Argument/Syntax.h>
#include <cctype>

namespace Tools
{

C_Syntax::C_Syntax()
{
}

C_Syntax::~C_Syntax()
{
}

void C_Syntax::AddArg(const C_Argument& arg)
{
	m_arguments.PushBack(arg);
}

const ArgList& C_Syntax::GetArgs() const
{
	return m_arguments;
}

const C_Argument* C_Syntax::GetArgument(const Btl::String& argName) const
{
	//Reserve * as indicating no short arg.
	if (argName.Length() == 0 || argName == "*")
		return 0;

	bool isShortName;
	if (argName[0] == '#')
	{
		isShortName = true;
	}
	else
	{
		if (argName[0] != '-')
			return 0;

		if (argName[1] != '-')
		{
			isShortName = true;
		}
		else
			isShortName = false;
	}

	if (isShortName)
	{
		Btl::String strippedArg;
		if (argName.Length() > 1)
			strippedArg = argName(1, argName.Length());
		else
			strippedArg = argName;

		if (strippedArg.Length() > 1)
			return 0;

		for (ArgList::ConstIterator it = m_arguments.Begin(); it != m_arguments.End(); ++it)
		{
			const C_Argument& arg = *it;
			if (std::tolower(arg.GetShortName()) == std::tolower(strippedArg[0]))
				return &arg;
		}
	}
	else
	{
		Btl::String strippedArg = argName(2, argName.Length());
		for (ArgList::ConstIterator it = m_arguments.Begin(); it != m_arguments.End(); ++it)
		{
			const C_Argument& arg = *it;
			if (!_stricmp(arg.GetLongName().Chars(), strippedArg.Chars()))
				return &arg;
		}
	}
	return 0;
}

}
