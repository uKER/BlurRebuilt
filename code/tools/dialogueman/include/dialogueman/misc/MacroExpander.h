#ifndef DMAN_MACRO_EXPANDER_H
#define DMAN_MACRO_EXPANDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{
class C_MacroExpander 
{
public:
	C_MacroExpander();

	void Expand(Util::C_String& str) const;

	void Insert(const Util::C_String& name, const Util::C_String& value);

	Util::C_String GetValue(const Util::C_String& name);

	void Clear();

private:
	struct C_Macro
	{
		Util::C_String name;
		Util::C_String value;
	};

	Ut::Vector<C_Macro> m_macros;
};

}

#endif
