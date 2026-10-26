#ifndef BTL_TEST_ARCHIVE_H
#define BTL_TEST_ARCHIVE_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/types.h"

// So the tests don't depend on System.
// Just for testing compilation, does not actually function as an archive.
struct TestArchive
{
	template <typename T> void operator << (T& t) {Serialise(*this, const_cast<typename Btl::StripConst<T>::Result&>(t));}
	template <typename T> void operator >> (T& t) {Deserialise(*this, t);}

	void operator << (U32 t) {}
	void operator >> (U32& t) {t = 0;}
	void operator << (S32 t) {}
	void operator >> (S32& t) {t = 0;}
};

template <typename Archive, typename T>
void Serialise(Archive& ar, T& t) {t.Serialise(ar);}

template <typename Archive, typename T>
void Deserialise(Archive& ar, T& t) {t.Deserialise(ar);}

#endif
