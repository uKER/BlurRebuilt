#include "stdafx.h"
#include <core/utility/singleton.h>
#include <system/xml/Xml.h>
#include <tools_shared/utility/MacroExpander.h>

namespace Tools
{

C_MacroExpander::C_MacroExpander()
{
	char* envStr;
	envStr = GetEnvironmentStrings();

	LPTSTR str = envStr;

	while (true)
	{
		if (*str == 0)
			break;
		while (*str != 0) 
			str++;
		str++;

		Btl::String var(str);
		S32 pathToDiscardIdx = var.Search(":=");
		if (pathToDiscardIdx < 3 && pathToDiscardIdx != -1)
			//Ignore silly paths provided at the start of envStr.
			continue;

		Btl::Vector<Btl::String> strings;
		var.Tokenise(strings, '=');

		if (strings.Size() < 2)
			continue;

		//Concat strings (lazy I know... i just need to get the sodding thing working for the moment.)
		for (S32 i = 2; i < strings.Size(); i++)
			strings[1] += Btl::String("=") + strings[i];

		Insert(strings[0], strings[1]);
	}

	FreeEnvironmentStrings(envStr);
}

bool C_MacroExpander::ParseMacroGroupsXML(const Sys::C_XmlDomBaseNode* pNode, const char* macroGroupName)
{
	if (!pNode)
		return false;

	if (!_stricmp(pNode->GetName().Chars(), macroGroupName))
	{
		for (S32 i = 0; i < pNode->GetChildren().Size(); i++)
		{
			Sys::C_XmlDomBaseNode* pMacro = pNode->GetChildren()[i];
			if (!pMacro)
				return false;

			const Sys::C_XmlAttribute* pMacroName = ((Sys::C_XmlDomElement*)(pMacro))->GetAttributes().GetAttributeByName("name");
			if (!pMacroName)
				return false;
			const Sys::C_XmlAttribute* pMacroValue = ((Sys::C_XmlDomElement*)(pMacro))->GetAttributes().GetAttributeByName("value");
			if (!pMacroValue)
				return false;

			Insert(pMacroName->value, pMacroValue->value);
		}
	}

	for (S32 i = 0; i < pNode->GetChildren().Size(); i++)
	{
		if (!ParseMacroGroupsXML(pNode->GetChildren()[i], macroGroupName))
			return false;
	}

	return true;
}

void C_MacroExpander::Expand(Util::C_String& str) const
{
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		const C_Macro& macro = m_macros[i];
		str = str.IReplace(Util::C_String("%") + macro.name.Chars() + Util::C_String("%"), macro.value.Chars());
	}
}

void C_MacroExpander::Expand(Btl::String& str) const
{
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		const C_Macro& macro = m_macros[i];
		str = str.IReplace(Btl::String("%") + macro.name + Btl::String("%"), macro.value);
	}
}

void C_MacroExpander::Expand(Sys::C_FilePath& str) const
{
	Btl::String tmp(str.Chars());
	Expand(tmp);
	str = tmp;
}

void C_MacroExpander::Insert(const Util::C_String& name, const Util::C_String& value)
{
	Util::C_String tmp = name.Lower();
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		//Replace.
		if (m_macros[i].name == tmp)
			m_macros[i].value = value.Chars();
	}

	//Insert 
	m_macros.PushBack();
	C_Macro& macro = m_macros.Back();
	macro.name = tmp.Chars();
	macro.value = value.Chars();
}

void C_MacroExpander::Insert(const Btl::String& name, const Btl::String& value)
{
	Btl::String tmp = name.Lower();
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		//Replace.
		if (m_macros[i].name == tmp)
			m_macros[i].value = value;
	}

	//Insert 
	m_macros.PushBack();
	C_Macro& macro = m_macros.Back();
	macro.name = tmp;
	macro.value = value;
}

void C_MacroExpander::Clear()
{
	m_macros.Clear();
}

}
