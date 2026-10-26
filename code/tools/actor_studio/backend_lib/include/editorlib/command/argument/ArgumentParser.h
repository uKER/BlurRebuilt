#ifndef EDITOR_LIB_ARGUMENT_PARSER_H
#define EDITOR_LIB_ARGUMENT_PARSER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_ArgumentParser
	{
	public:
		~C_ArgumentParser();

		C_Status Parse(C_ArgumentDatabase& db, const Btl::String& arguments) const;

	private:
		C_ArgumentParser(const C_Syntax* pSyntax);

		C_ArgumentParser(const C_ArgumentParser& rhs);
		const C_ArgumentParser& operator=(const C_ArgumentParser& rhs);

		void PreprocessArguments(	Btl::String& preprocessed, 
									const Btl::String& arguments) const;

		U32 GetNumManditoryArgs() const;

		const C_Syntax* m_pSyntax;

		FRIEND_CLASS C_CommandProcessor;
	};
}

#endif
