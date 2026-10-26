#ifndef MISC_SHARED_FACTORY_H
#define MISC_SHARED_FACTORY_H

#ifdef _MSC_VER_
#pragma once
#endif

struct C_RegInfo
{
	typedef void*	(*CreateFunc)(void);

	C_RegInfo() { create = 0; }

	CreateFunc			create;
};

template<typename REGINFO>
class C_NullRegisterPolicy
{
public:
	template<typename DERIVED>
	static bool Register(REGINFO* pRegInfo) { return true; }
};

template<
	typename BASE, 
	typename REGINFO = C_RegInfo, 
	typename CUSTOMREGISTERPOLICY = C_NullRegisterPolicy<REGINFO> >
class C_Factory
{
public:
	C_Factory();
	~C_Factory();

	template<typename DERIVED>
	bool Register();

	BASE* Create(const Btl::String& typeName);

	BASE* Create(U32 type);

	REGINFO* Find(const Btl::String& typeName);

	REGINFO* Find(U32 type);

private:
	//Map from name to create func.
	typedef Btl::OpenHashMap<Btl::String, REGINFO*, Btl::HashKeyOpBtlString > ObjNameHashMap;
	ObjNameHashMap m_nameMapping;

	//Map from id to create func.
	typedef Btl::OpenHashMap<U32, REGINFO* > ObjTypeHashMap;
	ObjTypeHashMap m_typeMapping;
};

template<
	typename BASE, 
	typename REGINFO = C_RegInfo, 
	typename CUSTOMREGISTERPOLICY = C_NullRegisterPolicy<REGINFO> >
class C_IdFactory
{
public:
	C_IdFactory();
	~C_IdFactory();

	template<typename DERIVED>
	bool Register();

	BASE* Create(U32 type);

	REGINFO* Find(U32 type);

private:
	//Map from id to create func.
	typedef Btl::OpenHashMap<U32, REGINFO* > ObjTypeHashMap;
	ObjTypeHashMap m_typeMapping;
};

#include "Factory.inl"

#endif
