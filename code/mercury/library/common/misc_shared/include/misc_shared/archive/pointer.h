#ifndef ARCHIVE_POINTER_H
#define ARCHIVE_POINTER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "handler.h"

namespace Arch
{

// Archives through a pointer.
template <typename Archive, typename T>
void Serialise(Archive& ar, T*& p)
{
	if (p)
	{
		SerialisationType t = SerialisationInfo<T>::GetSerialisationType();
		ar << MakeNvp("Serialisation_Type", t);

		SerialisationKey key = GetSerialisationKey<T>(p);

		if (key.IsNull())
		{
			ar << *p;
		}
		else
		{
			// Since the keys can change between compilations we write the full identifier.
			// In principle a key could be assigned to each class by the archive, and stored in a table.
			const char* id = SerialisationIdManager::GetGUID(key);
			ar << MakeNvp("Type_Name", id);
			Streamer<Archive>* streamer = SerialisationHandlerManager<Streamer<Archive> >::GetHandler(id);
			streamer->Stream(ar, p);
		}
	}
	else
	{
		ar << SERIALISATION_TYPE_UNKNOWN;
	}
}

// Creates the object using operator new and loads from the archive.
template <typename Archive, typename T>
void Deserialise(Archive& ar, T*& p)
{
	SerialisationType serialisationType;
	ar >> serialisationType;

	if (serialisationType == SERIALISATION_TYPE_UNKNOWN)
	{
		p = 0;
	}
	else if (serialisationType == SERIALISATION_TYPE_POLYMORPHIC)
	{
		// A maximum length for the string would avoid the allocation for id.
		char* id;
		ar >> id;
		Factory<>* factory = SerialisationHandlerManager<Factory<> >::GetHandler(id);
		Streamer<Archive>* streamer = SerialisationHandlerManager<Streamer<Archive> >::GetHandler(id);
		delete [] id;

		p = (T*) factory->CreateObject();
		streamer->Stream(ar, p);
	}
	else // Simple or versioned. Note that T may be polymorphic now, but not when it was saved.
	{
		p = new T;
		ar >> *p;
	}
}

}

#endif
