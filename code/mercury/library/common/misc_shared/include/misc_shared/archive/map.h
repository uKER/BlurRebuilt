#ifndef ARCHIVE_MAP_H
#define ARCHIVE_MAP_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/map.h"
#include "btl/containers/multimap.h"
#include "btl/containers/chainedhashmap.h"
#include "btl/containers/openhashmap.h"

namespace Arch
{

template <typename Key, typename Value, typename Allocator>
struct SerialisationInfo<Btl::Map<Key, Value, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::Map";}
};

template<typename Archive, typename Key, typename Value, typename Allocator>
void Serialise(Archive &ar, Btl::Map<Key, Value, Allocator> &t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::Map<Key, Value, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
	{
		ar << i->GetKey();
		ar << i->GetValue();
	}
}

template <class Archive, typename Key, typename Value, typename Allocator>
void Deserialise(Archive &ar, Btl::Map<Key, Value, Allocator> &t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		Value value;
		ar >> key;
		ar >> value;
		t.Insert(key, value);
	}
}

///////////////////////////////////////////////////////////////////////

template <typename Key, typename Value, typename Allocator>
struct SerialisationInfo<Btl::MultiMap<Key, Value, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::MultiMap";}
};

template<typename Archive, typename Key, typename Value, typename Allocator>
void Serialise(Archive &ar, Btl::MultiMap<Key, Value, Allocator> &t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::MultiMap<Key, Value, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
	{
		ar << i->GetKey();
		ar << i->GetValue();
	}
}

template <class Archive, typename Key, typename Value, typename Allocator>
void Deserialise(Archive &ar, Btl::MultiMap<Key, Value, Allocator> &t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		Value value; 
		ar >> key;
		ar >> value;
		t.Insert(key, value);
	}
}

////////////////////////////////////////////////////////////////////////

template <typename Key, typename Value, typename HashKeyOp, typename Allocator>
struct SerialisationInfo<Btl::ChainedHashMap<Key, Value, HashKeyOp, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::ChainedHashMap";}
};

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Allocator>
void Serialise(Archive& ar, Btl::ChainedHashMap<Key, Value, HashKeyOp, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::ChainedHashMap<Key, Value, HashKeyOp, Allocator>::Iterator i = t.Begin(); i != t.End(); ++i)
	{
		ar << i->GetKey();
		ar << i->GetValue();
	}
}

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Allocator>
void Deserialise(Archive& ar, Btl::ChainedHashMap<Key, Value, HashKeyOp, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		Value value;
		ar >> key;
		ar >> value;
		t.Insert(key, value);
	}
}

////////////////////////////////////////////////////////////////////////

template <typename Key, typename Value, typename HashKeyOp, typename Allocator>
struct SerialisationInfo<Btl::ChainedHashMultiMap<Key, Value, HashKeyOp, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::ChainedHashMultiMap";}
};

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Allocator>
void Serialise(Archive& ar, Btl::ChainedHashMultiMap<Key, Value, HashKeyOp, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::ChainedHashMultiMap<Key, Value, HashKeyOp, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
	{
		ar << i->GetKey();
		ar << i->GetValue();
	}
}

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Allocator>
void Deserialise(Archive& ar, Btl::ChainedHashMultiMap<Key, Value, HashKeyOp, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		Value value;
		ar >> key;
		ar >> value;
		t.Insert(key, value);
	}
}

//////////////////////////////////////////////////////////////////////////////

template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
struct SerialisationInfo<Btl::OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator> >
{
	static U8 GetVersion() {return 0;}
	static SerialisationType GetSerialisationType() {return Arch::SERIALISATION_TYPE_VERSION;}
	static const char* GetGUID() {return "Btl::OpenHashMap";}
};

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
void Serialise(Archive& ar, Btl::OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>& t)
{
	S32 n = t.Size();
	ar << n;
	for (typename Btl::OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::ConstIterator i = t.Begin(); i != t.End(); ++i)
	{
		ar << i->GetKey();
		ar << i->GetValue();
	}
}

template <typename Archive, typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
void Deserialise(Archive& ar, Btl::OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>& t)
{
	t.Clear();
	S32 n; ar >> n;

	for (int i = 0; i < n; i++)
	{
		Key key;
		Value value;
		ar >> key;
		ar >> value;
		t.Insert(key, value);
	}
}
}

#endif
