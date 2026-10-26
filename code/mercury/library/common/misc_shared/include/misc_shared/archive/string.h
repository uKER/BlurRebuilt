#ifndef ARCHIVE_STRING_H
#define ARCHIVE_STRING_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/string.h"

namespace Arch
{

template <typename DataType, typename Buffer>
struct SerialisationInfo<Btl::StringBase<DataType, Buffer> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "Btl::String";}
};

template <typename DataType, typename Buffer, typename Archive>
void Deserialise(Archive &ar, Btl::StringBase<DataType, Buffer>& s)
{
	U32 n;
	ar >> n;
	s.Resize(n);
	DataType* pData = s.NonConstChars();
	for (U32 i = 0; i < n; i++)
		ar >> pData[i];
}

template <typename DataType, typename Buffer, typename Archive>
void Serialise(Archive &ar, Btl::StringBase<DataType, Buffer>& s)
{
	if (Btl::SameType<DataType, char>::result)
	{
		const char* pChars = s.Chars();
		ar << pChars;
	}
	else
	{
		U32 n = s.Length();
		const DataType *pChars = s.Chars();
		ar << MakeNvp("Size", n);
		for (U32 i = 0; i < n; i++)
		{
			ar << pChars[i];
		}
	}
}

}

#endif
