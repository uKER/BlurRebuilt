#ifndef ARCHIVE_KEY_H
#define ARCHIVE_KEY_H
#ifdef _MSC_VER
#pragma once
#endif

#include <string.h>
#include "btl/containers/map.h"
#include "btl/containers/vector.h"

namespace Arch
{

class SerialisationIdManager;

// The serialisation key is a unique identifier for the object.
// (The real identifier is a string but we don't want to store that on the object.)
class SerialisationKey
{
public:
	SerialisationKey() {}
	
	bool operator < (const SerialisationKey& rhs) const {return m_key < rhs.m_key;}

	bool IsNull() {return m_key == -1;}

	static SerialisationKey NullKey() {return SerialisationKey(-1);}

private:
	FRIEND_CLASS SerialisationIdManager;

	typedef S16 DataType;
	explicit SerialisationKey(DataType key) : m_key(key) {}
	DataType GetValue() const {return m_key;}
	DataType m_key;
};

template <typename T>
const Arch::SerialisationKey GetSerialisationKey(T* dummy) 
{
	return SerialisationKey::NullKey();
}

#define GET_SERIALISATION_KEY(class_) Arch::FindKey<class_>()

/////////////////////////////////////////////////////////////////////
// Objects use a string as an identifier, but need to store something smaller.
// This class maps strings to keys.
class SerialisationIdManager
{
public:

	// Given a GUID string, returns the unique key.
	// A null key is returned for objects that have not been registered.
	inline static SerialisationKey FindKey(const char* guid)
	{
		if (guid)
		{
			Create();
			NameMap::Iterator r = s_pInstance->nameMap.Find(StringRef(guid));
			if (r != s_pInstance->nameMap.End())
				return r->GetValue();
			
			assert(!"Type not registered");
		}

		return SerialisationKey(-1);
	}

	// Given a GUID string, returns a unique key.
	inline static SerialisationKey RegisterKey(const char* guid)
	{
		assert(guid); // Polymorphic types must have a guid.
		Create();
		NameMap::InsertionReturnType r = s_pInstance->nameMap.Insert(StringRef(guid));
		if (r.inserted)
		{
			r.iterator->GetValue() = SerialisationKey((Arch::SerialisationKey::DataType)s_pInstance->nameVector.Size());
			s_pInstance->nameVector.PushBack(guid);
		}
		else
			assert(0 && "GUID not unique.");

		return r.iterator->GetValue();
	}

	static const char* GetGUID(const SerialisationKey& key)
	{
		assert(s_pInstance && "No names added to the manager.");
		return s_pInstance->nameVector[key.GetValue()];
	}

	// Need to delete this after the last handler.
	static void AddReference() {Create(); s_pInstance->count++;}
	static void DeleteReference() {s_pInstance->count--; if (s_pInstance->count == 0) Destroy();}

private:
	SerialisationIdManager() : count(0) {}

	static void Create()
	{
		if (!s_pInstance)
			s_pInstance = new SerialisationIdManager;
	}

	static void Destroy()
	{
		delete s_pInstance;
		s_pInstance = 0;
	};

	struct StringRef
	{
		StringRef(const char* s) : m_s(s) {}
		bool operator < (const StringRef& rhs) const {return strcmp(m_s, rhs.m_s) < 0;}
		const char* m_s;
	};

	typedef Btl::Map<StringRef, SerialisationKey> NameMap;
	typedef Btl::Vector<const char*> NameVector;

	NameMap nameMap;
	NameVector nameVector;
	int count;

	static SerialisationIdManager* s_pInstance;
};

}

#endif
