#ifndef EDITOR_LIB_ARGUMENT_DATABASE_H
#define EDITOR_LIB_ARGUMENT_DATABASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Misc_Shared/Variant/KeyValue.h>
#include <EditorLib/Misc/Status.h>

namespace EditorLib
{
	typedef Btl::Vector<Util::C_KeyValue> ArgumentDataArray;

	class C_ArgumentDatabase
	{
	public:
		C_ArgumentDatabase();
		C_ArgumentDatabase(const C_ArgumentDatabase& rhs);
		~C_ArgumentDatabase();

		const C_ArgumentDatabase& operator=(const C_ArgumentDatabase& rhs);

		bool IsSet(const char shortName) const;

		const ArgumentDataArray* GetArgData(const char shortName) const;

	private:
		struct C_ArgumentGroup
		{
			char shortName;
			ArgumentDataArray data;
		};

		C_Status Parse(const Btl::String& arguments);

		bool AddArgGroup(
			const C_Argument& flagArg,  
			const Btl::Vector<Btl::String>& data);

		template<class T>
		bool _GetCommandArg(T& value, const char shortName, U32 index) const;

		const C_ArgumentGroup* GetArgGroup(const char shortName) const;

		Btl::Vector<C_ArgumentGroup>		m_argumentGroups;

		FRIEND_CLASS C_ArgumentParser;
	};
}

#endif
