#include "stdafx.h"

#include <DialogueMan/Misc/MacroExpander.h>
#include <Tools_Shared/Utility/MacroExpander.h>

namespace DMan
{

C_MacroExpander::C_MacroExpander()
{
}

void C_MacroExpander::Expand(Util::C_String& str) const
{
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		const C_Macro& macro = m_macros[i];
		str = str.IReplace(Util::C_String("%") + macro.name + Util::C_String("%"), macro.value);
	}
	Tools::C_MacroExpander::Instance().Expand(str);
}

void C_MacroExpander::Insert(const Util::C_String& name, const Util::C_String& value)
{
	Util::C_String tmp = name.Lower();
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

Util::C_String C_MacroExpander::GetValue(const Util::C_String& name)
{
	Util::C_String tmp = name.Lower();
	for (S32 i = 0; i < m_macros.Size(); i++)
	{
		C_Macro& macro = m_macros[i];
		if (macro.name == tmp)
			return macro.value;
	}
	return "";
}

void C_MacroExpander::Clear()
{
	m_macros.Clear();
}

}
