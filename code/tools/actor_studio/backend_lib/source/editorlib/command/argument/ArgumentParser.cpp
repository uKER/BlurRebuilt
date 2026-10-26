#include "stdafx.h"
#include <EditorLib/Command/Argument/Argument.h>
#include <EditorLib/Command/Argument/Syntax.h>
#include <EditorLib/Command/Argument/ArgumentDatabase.h>
#include <EditorLib/Command/Argument/ArgumentParser.h>

namespace EditorLib
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

C_Status C_ArgumentParser::Parse(
	C_ArgumentDatabase& db, 
	const Btl::String& arguments) const
{
	Btl::String preprocessedArgs;
	PreprocessArguments(preprocessedArgs, arguments);

	if (!m_pSyntax)
		return E_Status_Success;

	if (preprocessedArgs.Length() == 0)
	{
		if (GetNumManditoryArgs() == 0)
			return E_Status_Success;
		else
			return E_Status_Failure;
	}

	Ut::Vector<Btl::String> strings;
	TokeniseSpan(strings, preprocessedArgs, ' ', '\"');
	//If no tokens then use raw argument string.
	if (strings.Size() == 0)
		strings.PushBack(preprocessedArgs);

	const ArgumentList& argList = m_pSyntax->GetArgs();

	const C_Argument* pCurrentFlagArg = 0;
	Btl::Vector<Btl::String> currentComandArgs;

	for (S32 i = 0; i < strings.Size(); i++)
	{
		const Btl::String& str = strings[i];
		if (str.Length() < 1)
			continue;

		//If we're an argument.
		if (str[0] == '-' && str.Length() > 1 && !isdigit(str[1]))
		{
			//Flush args appearing before first flag.
			if (currentComandArgs.Size() != 0)
			{
				//Set current to null argument.
				if (!pCurrentFlagArg)
				{
					pCurrentFlagArg = m_pSyntax->GetArgument('#');
					if (!pCurrentFlagArg)
						return E_Status_Failure;
				}

				//Attempt to flush arg
				if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
					return E_Status_Failure;

				currentComandArgs.Clear();
			} 
			else if (pCurrentFlagArg)
			{
				//Attempt to flush arg
				if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
					return E_Status_Failure;

				currentComandArgs.Clear();
			}

			pCurrentFlagArg = m_pSyntax->GetArgument(str);
		}
		else
		{
			//Parameters neet to be associated with a valid flag argument.
			currentComandArgs.PushBack(str);
		}
	}

	//Attempt to flush arg and param.
	if (!pCurrentFlagArg)
	{
		pCurrentFlagArg = m_pSyntax->GetArgument('#');
		if (!pCurrentFlagArg)
			return E_Status_Failure;

		if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
			return E_Status_Failure;
	} 
	else
	{
		if (!db.AddArgGroup(*pCurrentFlagArg, currentComandArgs))
			return E_Status_Failure;
	}


	return E_Status_Success;
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
	for (ArgumentList::ConstIterator it = m_pSyntax->GetArgs().Begin();
		it != m_pSyntax->GetArgs().End(); ++it)
	{
		const C_Argument& arg = *it;
		if (arg.IsRequired())
			nManditory++;
	}

	return nManditory;
}

}
