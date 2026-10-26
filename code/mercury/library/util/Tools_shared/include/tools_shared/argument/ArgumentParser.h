#ifndef TOOLS_ARGUMENT_PARSER_H
#define TOOLS_ARGUMENT_PARSER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace Tools
{
	class C_ArgumentDatabase;
	class C_Syntax;

	class C_ArgumentParser
	{
	public:
		C_ArgumentParser(const C_Syntax* pSyntax);
		~C_ArgumentParser();

		bool Parse(C_ArgumentDatabase& db, const Btl::String& arguments) const;

	private:
		C_ArgumentParser(const C_ArgumentParser& rhs);
		const C_ArgumentParser& operator=(const C_ArgumentParser& rhs);

		void PreprocessArguments(	Btl::String& preprocessed, 
									const Btl::String& arguments) const;

		U32 GetNumManditoryArgs() const;

		const C_Syntax* m_pSyntax;
	};
}

#endif
