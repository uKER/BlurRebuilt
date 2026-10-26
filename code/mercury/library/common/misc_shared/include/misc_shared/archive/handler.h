#ifndef ARCHIVE_HANDLER_H
#define ARCHIVE_HANDLER_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/map.h"
#include "btl/containers/vector.h"
#include "serialisationinfo.h"

namespace Arch
{

// Class to store handlers.
template <typename Handler>
class SerialisationHandlerManager
{
public:
	static void Register(Handler* handler, const SerialisationKey& id)
	{
		Create();
		s_pInstance->handlerMap.Insert(typename HandlerMap::DataType(id, handler));
	}

	static void DeRegister(Handler* handler, const SerialisationKey& id)
	{
		s_pInstance->handlerMap.Erase(id);

		// If there are no more objects, delete the instance.
		// This prevents a memory leak on exit.
		if (s_pInstance->handlerMap.Empty())
			Destroy();
	}

	static Handler* GetHandler(const SerialisationKey& id)
	{
		assert(s_pInstance && "No handlers added to the manager.");
		typename HandlerMap::Iterator find = s_pInstance->handlerMap.Find(id);

		assert((find != s_pInstance->handlerMap.End()) && "Handler not found.");
		return find->GetValue();
	}

	static Handler* GetHandler(const char* name)
	{
		return GetHandler(SerialisationIdManager::FindKey(name));
	}

private:
	SerialisationHandlerManager() {SerialisationIdManager::AddReference();}
	~SerialisationHandlerManager() {SerialisationIdManager::DeleteReference();}

	static void Create()
	{
		if (!s_pInstance)
			s_pInstance = new SerialisationHandlerManager;
	}

	static void Destroy()
	{
		delete s_pInstance;
		s_pInstance = 0;
	};

	typedef Btl::Map<SerialisationKey, Handler*> HandlerMap;
	HandlerMap handlerMap;

	static SerialisationHandlerManager* s_pInstance;
};

template <typename Handler>
SerialisationHandlerManager<Handler>* SerialisationHandlerManager<Handler>::s_pInstance;

// Handlers.
// There are two kinds of handler: Factory and Streamer.
// Factory makes the objects and takes an allocator as a template parameter.
// Streamer archives the objects and takes an archive as a template parameter.
template<typename Allocator = Btl::StandardAllocator<Btl::NullType> >
class Factory
{
public:
	virtual void* CreateObject() = 0;

	// The object can be freed directly using the allocator.
	// However, this function will call the correct destructor even if it is non-virtual.
	// It also returns the size of the object, in case the allocator needs it.
	virtual int DestroyObject(void* p) = 0;

protected:
	Factory() {}
	virtual ~Factory() {}
};

template<typename Archive>
class Streamer
{
public:
	virtual void Stream(Archive& ar, void* p) = 0;

protected:
	Streamer() {}
	virtual ~Streamer() {}
};

template <typename T, typename Allocator>
class Factory_T : public Factory<Allocator>
{
public:
	void* CreateObject() 
	{
		typedef typename Allocator::template Rebind<T>::Other AllocatorType;
		return AllocatorType::Allocate();
	}

	int DestroyObject(void* p) {((T*) p)->~T(); return sizeof(T);}

	Factory_T() {Arch::SerialisationHandlerManager<Factory<Allocator> >::Register(this, SerialisationIdManager::FindKey(SerialisationInfo<T>::GetGUID()));}
	~Factory_T() {Arch::SerialisationHandlerManager<Factory<Allocator> >::DeRegister(this, SerialisationIdManager::FindKey(SerialisationInfo<T>::GetGUID()));}

};

template <typename T, typename Archive>
class Streamer_T : public Streamer<Archive>
{
public:
	void Stream(Archive& ar, void* p) {ar & *(T*) p;}
	
	Streamer_T() {SerialisationHandlerManager<Streamer<Archive> >::Register(this, SerialisationIdManager::FindKey(SerialisationInfo<T>::GetGUID()));}
	~Streamer_T() {SerialisationHandlerManager<Streamer<Archive> >::DeRegister(this, SerialisationIdManager::FindKey(SerialisationInfo<T>::GetGUID()));}
};

template <typename T>
class RegisterGuid
{
public:
	RegisterGuid() 
	{
		SerialisationIdManager::RegisterKey(SerialisationInfo<T>::GetGUID());
	}
};


// An allocator can be used with polymorphic serialisation if
// i) It does not require the size of the object when freeing memory, and
// ii) The destructor is virtual.
// Otherwise, FactoryAllocator can wrap another allocator to free objects correctly.
template <typename T, typename Allocator = Btl::StandardAllocator<Btl::NullType> >
class FactoryAllocator
{
public:
	typedef T* PointerType;

	static T* Allocate() 
	{
		return Allocator::template Rebind<T>::Other::Allocate();
	}

	static void Free(T* p, int n = 0)
	{
		if (p)
		{
			typedef typename Allocator::template Rebind<Btl::NullType>::Other AllocatorType;
			const char* id = SerialisationIdManager::GetGUID(GetSerialisationKey<T>(p));
			Factory<AllocatorType>* factory = SerialisationHandlerManager<Factory<AllocatorType> >::GetHandler(id);
			int n = factory->DestroyObject(p);
			Allocator::template Rebind<Btl::SameSizeAndAlignment<T> >::Other::Free((Btl::SameSizeAndAlignment<T>*)p, n);
		}
	}

	template <typename U> 
	struct Rebind 
	{
		typedef FactoryAllocator<U, Allocator> Other;
	};
};

//////////////////////////////////////////////////////////////////////
// Macros to create handlers.

#define SERIALISATION_FACTORY(Type_, Allocator_)	{static Arch::Factory_T<Type_, Allocator_::Rebind<Btl::NullType>::Other > a;}

#define SERIALISATION_STREAMER(Type_, Archive_)		{static Arch::Streamer_T<Type_, Archive_> a;}

#define SERIALISATION_FACTORY_DEFAULT(Type_)		SERIALISATION_FACTORY(Type_, Btl::StandardAllocator<Btl::NullType>) \
													SERIALISATION_FACTORY(Type_, Arch::FactoryAllocator<Btl::NullType>)

#define SERIALISATION_STREAMER_DEFAULT(Type_)		SERIALISATION_STREAMER(Type_, Arch::BinaryOutputArchive_t<Ut::NoEndianSwap>) \
													SERIALISATION_STREAMER(Type_, Arch::BinaryOutputArchive_t<Ut::DoEndianSwap>) \
													SERIALISATION_STREAMER(Type_, Arch::BinaryInputArchive_t<Ut::NoEndianSwap>) \
													SERIALISATION_STREAMER(Type_, Arch::BinaryInputArchive_t<Ut::DoEndianSwap>) \
													SERIALISATION_STREAMER(Type_, Arch::XmlOutputArchive)

#define SERIALISATION_HANDLER(Type_)				{static Arch::RegisterGuid<Type_> a;} \
													SERIALISATION_STREAMER_DEFAULT(Type_) \
													SERIALISATION_FACTORY_DEFAULT(Type_)
}

#endif

