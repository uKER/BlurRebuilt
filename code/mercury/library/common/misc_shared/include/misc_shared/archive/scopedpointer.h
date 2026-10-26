#ifndef ARCHIVE_SCOPED_POINTER_H
#define ARCHIVE_SCOPED_POINTER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/scopedpointer.h"
#include "pointer.h"

namespace Arch
{

template <typename T, typename Allocator>
struct SerialisationInfo<Btl::ScopedPointer<T, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_PRIMITIVE;}
	static const char* GetGUID() {return "Btl::ScopedPointer";}
};

template <typename Archive, typename T, typename Allocator>
void Serialise(Archive& ar, Btl::ScopedPointer<T, Allocator>& p)
{
	ar << Pointer(p);
}

template <typename Archive, typename T, typename Allocator>
void Deserialise(Archive& ar, Btl::ScopedPointer<T, Allocator>& p)
{
	SerialisationType serialisationType;
	ar >> serialisationType;

	if (serialisationType == SERIALISATION_TYPE_UNKNOWN)
	{
		Free(p);
	}
	else if (serialisationType == SERIALISATION_TYPE_POLYMORPHIC)
	{
		// Like plain pointer, except that the allocator type comes from the ScopedPointer.
		char* id;
		ar >> id;
		typedef typename Allocator::template Rebind<Btl::NullType>::Other AllocatorType;
		Streamer<Archive>* streamer = SerialisationHandlerManager<Streamer<Archive> >::GetHandler(id);
		Factory<AllocatorType>* factory = SerialisationHandlerManager<Factory<AllocatorType> >::GetHandler(id);
		delete [] id;

		Initialise(p, (T*) factory->CreateObject());
		streamer->Stream(ar, Pointer(p));
	}
	else
	{
		Allocate(p);
		ar >> *p;
	}
}

}

#endif
