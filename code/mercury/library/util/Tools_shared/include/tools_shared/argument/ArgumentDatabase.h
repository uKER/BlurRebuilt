#ifndef TOOLS_ARGUMENT_DATABASE_H
#define TOOLS_ARGUMENT_DATABASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <Misc_Shared/Variant/KeyValue.h>
#include "Argument.h"
#include "ArgumentParser.h"

namespace Tools
{
	typedef Btl::Vector<Util::C_KeyValue> ArgDataArray;

	class C_ArgumentDatabase
	{
	public:
		C_ArgumentDatabase();
		C_ArgumentDatabase(const C_ArgumentDatabase& rhs);
		~C_ArgumentDatabase();

		const C_ArgumentDatabase& operator=(const C_ArgumentDatabase& rhs);

		bool IsSet(const char shortName) const;

		bool IsSet(const Btl::String& longName) const;

		const ArgDataArray* GetArgData(const char shortName) const;

		const ArgDataArray* GetArgData(const Btl::String& longName) const;

	private:
		struct C_ArgumentGroup
		{
			C_ArgumentGroup() {}
			C_ArgumentGroup(const C_ArgumentGroup& rhs) { *this = rhs; }
			const C_ArgumentGroup& operator=(const C_ArgumentGroup& rhs)
			{
				shortName = rhs.shortName;
				longName = rhs.longName;
				data = rhs.data;

				return *this;
			}
			char shortName;
			Btl::String longName;
			ArgDataArray data;
		};

		bool Parse(const Btl::String& arguments);

		bool AddArgGroup(
			const C_Argument& flagArg,  
			const Btl::Vector<Btl::String>& data);

		template<class T>
		bool _GetCommandArg(T& value, const char shortName, U32 index) const;

		const C_ArgumentGroup* GetArgGroup(const char shortName) const;

		const C_ArgumentGroup* GetArgGroup(const Btl::String& longName) const;

		Btl::Vector<C_ArgumentGroup>		m_argumentGroups;

		FRIEND_CLASS C_ArgumentParser;
	};
}

#endif
