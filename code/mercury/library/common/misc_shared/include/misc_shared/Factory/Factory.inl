template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::C_Factory()
{
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::~C_Factory()
{
	for (ObjNameHashMap::Iterator it = m_nameMapping.Begin(); it != m_nameMapping.End(); ++it)
		delete it->GetValue();
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
BASE* C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Create(const Btl::String& typeName)
{
	REGINFO* pC = Find(typeName);
	if (!pC)
		return 0;

	BASE* pObj = (BASE*)pC->create();
	return pObj;
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
BASE* C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Create(U32 type)
{
	REGINFO* pC = Find(type);
	if (!pC)
		return 0;

	BASE* pObj = (BASE*)pC->create();
	return pObj;
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
REGINFO* C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Find(const Btl::String& typeName)
{
	ObjNameHashMap::Iterator it = m_nameMapping.Find(typeName.Lower());
	if (it == m_nameMapping.End())
		return 0;

	return it->GetValue();
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
REGINFO* C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Find(U32 type)
{
	ObjTypeHashMap::Iterator it = m_typeMapping.Find(type);
	if (it == m_typeMapping.End())
		return 0;

	return it->GetValue();
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
template<typename DERIVED>
bool C_Factory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Register()
{
	Btl::String name = DERIVED::GetStaticTypeName();
	name = name.Lower();

	ObjNameHashMap::Iterator it = m_nameMapping.Find(name);
	if (it != m_nameMapping.End())
	{
		//Duplicate command in registry!
		return false;
	}

	//Add to registry.
	REGINFO* pC = new REGINFO;
	pC->create = DERIVED::Create;
	m_nameMapping[name] = pC;

	//Add to objType->Create mapping
	{
		if (m_typeMapping.Find(DERIVED::GetStaticType()) != m_typeMapping.End())
		{
			assert(!"Duplicate type id!");
			return false;
		}

		//Insert.
		m_typeMapping[DERIVED::GetStaticType()] = pC;
	}

	if (!CUSTOMREGISTERPOLICY::Register<DERIVED>(pC))
		return false;

	return true;
}



template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
C_IdFactory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::C_IdFactory()
{
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
C_IdFactory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::~C_IdFactory()
{
	for (ObjTypeHashMap::Iterator it = m_typeMapping.Begin(); it != m_typeMapping.End(); ++it)
		delete it->GetValue();
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
BASE* C_IdFactory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Create(U32 type)
{
	REGINFO* pC = Find(type);
	if (!pC)
		return 0;

	BASE* pObj = (BASE*)pC->create();
	return pObj;
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
REGINFO* C_IdFactory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Find(U32 type)
{
	ObjTypeHashMap::Iterator it = m_typeMapping.Find(type);
	if (it == m_typeMapping.End())
		return 0;

	return it->GetValue();
}

template<
	typename BASE, 
	typename REGINFO,
	typename CUSTOMREGISTERPOLICY >
template<typename DERIVED>
bool C_IdFactory<BASE, REGINFO, CUSTOMREGISTERPOLICY>::Register()
{
	if (m_typeMapping.Find(DERIVED::GetStaticType()) != m_typeMapping.End())
	{
		assert(!"Duplicate type id!");
		return false;
	}

	//Add to registry.
	REGINFO* pC = new REGINFO;
	pC->create = DERIVED::Create;
	m_typeMapping[DERIVED::GetStaticType()] = pC;

	if (!CUSTOMREGISTERPOLICY::Register<DERIVED>(pC))
		return false;

	return true;
}
