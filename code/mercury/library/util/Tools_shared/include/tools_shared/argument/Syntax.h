#ifndef TOOLS_SYNTAX_H
#define TOOLS_SYNTAX_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <tools_shared/Argument/Argument.h>

namespace Tools
{
	class C_Syntax
	{
	public:
		C_Syntax();
		~C_Syntax();

		void AddArg(const C_Argument& arg);

		const ArgList& GetArgs() const;

		const C_Argument* GetArgument(const Btl::String& argName) const;

	private:
		C_Syntax(const C_Syntax& rhs);
		const C_Syntax& operator=(const C_Syntax& rhs);

		ArgList m_arguments;
	};
}

#endif
