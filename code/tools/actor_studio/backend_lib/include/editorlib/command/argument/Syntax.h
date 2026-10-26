#pragma once

#ifndef EDITOR_LIB_SYNTAX_H
#define EDITOR_LIB_SYNTAX_H

#include <EditorLib/Command/Argument/Argument.h>

namespace EditorLib
{
	typedef Ut::UtList<const C_Argument> ArgumentList;

	class C_Syntax
	{
	public:
		C_Syntax();
		~C_Syntax();

		void AddArg(const C_Argument& arg);

		const ArgumentList& GetArgs() const;

		const C_Argument* GetArgument(const Btl::String& argName) const;

	private:
		C_Syntax(const C_Syntax& rhs);
		const C_Syntax& operator=(const C_Syntax& rhs);

		ArgumentList m_arguments;
	};
}

#endif
