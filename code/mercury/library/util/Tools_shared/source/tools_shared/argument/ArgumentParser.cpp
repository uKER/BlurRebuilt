#include "stdafx.h"
#include <Tools_Shared/Argument/Argument.h>
#include <Tools_Shared/Argument/Syntax.h>
#include <Tools_Shared/Argument/ArgumentDatabase.h>
#include <Tools_Shared/Argument/ArgumentParser.h>

namespace Tools
{

//We ignore tokens within a span.
void TokeniseSpan(
	Ut::Vector<Btl::String>& strings,
	const Btl::String& input,
	const char tok, 
	const char spanTok)
{
	assert(tok != spanTok);

	strings.Clear();
	S32 lastSpanTokPosition = -1;
	S32 lastTokPosition = -1;
	for (S32 i = 0; i < input.Length(); i++)
	{
		//If we've found our tok and we're not spanning, and we're not the first character in our input.
		if (input[i] == tok && lastSpanTokPosition == -1 && i > 0)
		{
			strings.PushBack();
			Btl::String& str = strings.Back();

			str = input(lastTokPosition+1, i);
			lastTokPosition = i;
		}
		else if (input[i] == spanTok)
		{
			//if not spanning
			if (lastSpanTokPosition == -1)
				lastSpanTokPosition = i;
			//flush span
			else
			{
				strings.PushBack();
				Btl::String& span = strings.Back();
				span = input(lastSpanTokPosition+1, i);
				//Nolonger spanning.
				lastSpanTokPosition = -1;
				lastTokPosition = i;
			}
		}
	}

	//Flush leftovers
	if (lastSpanTokPosition == -1)
	{
		if (lastTokPosition != -1 && lastTokPosition < input.Length()-1)
		{
			strings.PushBack();
			Btl::String& str = strings.Back();
			str = input(lastTokPosition+1, input.Length());
		}
	}
	else if (lastSpanTokPosition < input.Length()-2)
	{
		strings.PushBack();
		Btl::String& span = strings.Back();
		span = input(lastSpanTokPosition+1, input.Length());
	}
}

C_ArgumentParser::C_ArgumentParser(const C_Syntax* pSyntax)
{
	assert(pSyntax);
	m_pSyntax = pSyntax;
}

C_ArgumentParser::~C_ArgumentParser()
{
	delete m_pSyntax;
}

void DisplayHelp(const C_Syntax* pSyntax)
{
	LOGINFO(g_log, "\n-=Help=-\n\n");
	for (ArgList::ConstIterator it = pSyntax->GetArgs().Begin(); it != pSyntax->GetArgs().End(); ++it)
	{
		const C_Argument& arg = *it;
		LOGINFO(g_log, "  -%c  --%-15s  %s",
			arg.GetShortName(), arg.GetLongName().Chars(), arg.GetHelp().Chars());
	}

		LOGINFO(g_log, "  -%c  --%-15s  %s\n", 'h', "help", "display this page");
}

bool C_ArgumentParser::Parse(
	C_ArgumentDatabase& db, 
	const Btl::String& arguments) const
{
	Btl::String preprocessedArgs;
	PreprocessArguments(preprocessedArgs, arguments);

	if (!m_pSyntax)
		return true;

	if (preprocessedArgs.ISearch("-h") != -1 || preprocessedArgs.ISearch("--help") != -1)
	{
		DisplayHelp(m_pSyntax);
		return false;
	}

	Ut::Vector<Btl::String> strings;
	TokeniseSpan(strings, preprocessedArgs, ' ', '\"');
	//If no tokens then use raw argument string.
	if (strings.Size() == 0)
		strings.PushBack(preprocessedArgs);

	const C_Argument* pCurrentFlagArg = 0;
	Btl::Vector<Btl::String> currentComandArgs;

	bool done = false;
	for (S32 i = 0; i < strings.Size(); i++)
	{
		const Btl::String& str = strings[i];
		if (str.Length() < 1)
			continue;

		//If we're an argument.
		if (str[0] == '-' && str.Length() > 1 && !isdigit(str[1]))
		{
			if (str[1] != '-' && str.Length() > 2)
			{
				LOGERROR(g_log, "Incorrect syntax for short argument %s!", str.Chars());
				return false;
			}
			//Flush args appearing before first flag.
			if (currentComandArgs.Size() != 0)
			{
				//Set current to null argument.
				if (!pCurrentFlagArg)
				{
					pCurrentFlagArg = m_pSyntax->GetArgument('#');
					if (!pCurrentFlagArg)
					{
						//No more data to parse.
						done = true;
						break;
					}
				}

				//Attempt to flush arg
				if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
					return false;

				currentComandArgs.Clear();
			} 
			else if (pCurrentFlagArg)
			{
				//Attempt to flush arg
				if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
					return false;

				currentComandArgs.Clear();
			}

			pCurrentFlagArg = m_pSyntax->GetArgument(str);
			if (!pCurrentFlagArg)
			{
				LOGERROR(g_log, "Unexpected argument %s!", str.Chars());
				return false;
			}
		}
		else
		{
			//Parameters neet to be associated with a valid flag argument.
			currentComandArgs.PushBack(str);
		}
	}

	//Attempt to flush arg and param.
	if (!done)
	{
		if (!pCurrentFlagArg)
		{
			pCurrentFlagArg = m_pSyntax->GetArgument('#');
			if (!pCurrentFlagArg)
				return false;

			if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
				return false;
		} 
		else
		{
			if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
				return false;
		}
	}

	const ArgList& argList = m_pSyntax->GetArgs();

	//Did we find everything?
	for (ArgList::ConstIterator it = argList.Begin(); it != argList.End(); ++it)
	{
		const C_Argument& arg = *it;
		if (arg.IsRequired() && !(db.IsSet(arg.GetShortName()) || db.IsSet(arg.GetLongName())))
		{
			LOGERROR(g_log, "Required argument %c %s not found!", arg.GetShortName(), arg.GetLongName().Chars());
			return false;
		}
	}
	//Apply defaults.
	for (ArgList::ConstIterator it = argList.Begin(); it != argList.End(); ++it)
	{
		const C_Argument& arg = *it;
		if (arg.GetShortName() != '*')
		{
			if (db.IsSet(arg.GetShortName()))
				continue;
		}
		else if (db.IsSet(arg.GetLongName()))
			continue;

		//Set defaults.
		if (arg.GetDefaults().Size() > 0)
			if (!db.AddArgGroup(arg, arg.GetDefaults()))
				return false;
	}

	return true;
}

void C_ArgumentParser::PreprocessArguments(Btl::String& preprocessed, 
									  const Btl::String& arguments) const
{
	preprocessed = arguments;
	//TODO: long name contraction here.
}

U32 C_ArgumentParser::GetNumManditoryArgs() const
{
	U32 nManditory = 0;
	for (ArgList::ConstIterator it = m_pSyntax->GetArgs().Begin();
		it != m_pSyntax->GetArgs().End(); ++it)
	{
		const C_Argument& arg = *it;
		if (arg.IsRequired())
			nManditory++;
	}

	return nManditory;
}

}
