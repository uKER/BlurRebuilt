#ifndef MERCURY_MACRO_EXPANDER_H
#define MERCURY_MACRO_EXPANDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Sys
{
	class C_XmlDomBaseNode;
}

#include <core/utility/singleton.h>
#include <btl/utility/string.h>
#include <system/serialise/filepath.h>

namespace Tools
{
class C_MacroExpander : public Ut::Singleton< C_MacroExpander >
{
public:
	C_MacroExpander();

	bool ParseMacroGroupsXML(
		const Sys::C_XmlDomBaseNode* pNode, 
		const char* macroGroupName);

	void Expand(Util::C_String& str) const;

	void Expand(Btl::String& str) const;

	void Expand(Sys::C_FilePath& str) const;

	void Insert(const Util::C_String& name, const Util::C_String& value);

	void Insert(const Btl::String& name, const Btl::String& value);

	void Clear();

private:
	struct C_Macro
	{
		Btl::String name;
		Btl::String value;
	};

	Ut::Vector<C_Macro> m_macros;
};

}

#endif
