// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>

template<class BASE>
bdCreatorBase<BASE>::~bdCreatorBase()
{
}

template<class BASE>
BASE* bdCreatorBase<BASE>::create() const
{
	return BD_NULL;
}


template<class DERIVED, class BASE>
bdCreator<DERIVED, BASE>::~bdCreator()
{
}

template<class DERIVED, class BASE>
BASE* bdCreator<DERIVED, BASE>::create() const
{
	return new DERIVED;
}

template<class DERIVED, class BASE>
bdUInt bdCreator<DERIVED, BASE>::getSizeOf() const
{
	return sizeof(DERIVED);
}

template<class BASE>
bdFactory<BASE>::~bdFactory()
{
	typename bdHashMap<bdUInt, bdCreatorBase<BASE> *>::Iterator iter = m_creators.getIterator();

	while(iter)
	{
		bdCreatorBase<BASE> *const creator = m_creators.getValue(iter);
		if(creator)
		{
			delete creator;
		}
		m_creators.next(iter);
	}
}

template<class BASE>
BASE* bdFactory<BASE>::create(const bdUInt type) const
{
	BASE* derived = BD_NULL;
    bdCreatorBase<BASE> *creator = BD_NULL;
	
	if (m_creators.get(type, creator)) 
	{
		if(creator)
		{
			derived = creator->create();
		}
	}
	else
	{
		BD_WARN("bdCore/bdFactory","No creator registered for object type.");
	}

    return derived;
}

template<class BASE>
bdBool bdFactory<BASE>::registerClass(const bdUInt type, 
									bdCreatorBase<BASE> * creator)
{
	bdBool registered = containsClass(type);
	if(!registered)
	{
		registered = m_creators.put(type, creator);
		if(!registered)
		{
			BD_WARN("bdCore/bdFactory", "Unable to register creator already type %u.", type);
		}
	}
	else
	{
		BD_WARN("bdCore/bdFactory", "Creator already registered for object type %u.", type);
	}

	return registered;
}

template<class BASE>
bdBool bdFactory<BASE>::containsClass(const bdUInt type)
{
	const bdBool present = m_creators.containsKey(type);
	return present;
}


template<class BASE>
bdTypedCreatorBase<BASE>::~bdTypedCreatorBase()
{
}

template<class BASE>
BASE* bdTypedCreatorBase<BASE>::create(const bdUInt) const
{
	return BD_NULL;
}


template<class DERIVED, class BASE>
bdTypedCreator<DERIVED, BASE>::~bdTypedCreator()
{
}

template<class DERIVED, class BASE>
BASE* bdTypedCreator<DERIVED, BASE>::create(const bdUInt type) const
{
	return new DERIVED(type);
}


template<class DERIVED, class BASE>
bdUInt bdTypedCreator<DERIVED, BASE>::getSizeOf() const
{
	return sizeof(DERIVED);
}

template<class BASE>
bdTypedFactory<BASE>::~bdTypedFactory()
{
	typename bdHashMap<bdUInt, bdTypedCreatorBase<BASE> *>::Iterator iter = m_creators.getIterator();

	while(iter)
	{
		bdTypedCreatorBase<BASE> *const creator = m_creators.getValue(iter);
		if(creator)
		{
			delete creator;
		}
		m_creators.next(iter);
	}
}

template<class BASE>
BASE* bdTypedFactory<BASE>::create(const bdUInt type) const
{
	BASE* derived = BD_NULL;
	bdTypedCreatorBase<BASE> *creator = BD_NULL;

	if (m_creators.get(type, creator)) 
	{
		if(creator)
		{
			derived = creator->create(type);
		}
	}
	else
	{
		BD_WARN("bdCore/bdTypedFactory","No creator registered for object type.");
	}

	return derived;
}

template<class BASE>
bdBool bdTypedFactory<BASE>::registerClass(const bdUInt type, 
										   bdTypedCreatorBase<BASE> * creator)
{
	bdBool registered = containsClass(type);
	if(!registered)
	{
		registered = m_creators.put(type, creator);
		if(!registered)
		{
			BD_WARN("bdCore/bdFactory", "Unable to register creator already type %u.", type);
		}
	}
	else
	{
		BD_WARN("bdCore/bdFactory", "Creator already registered for object type %u.", type);
	}

	return registered;
}

template<class BASE>
bdBool bdTypedFactory<BASE>::containsClass(const bdUInt type)
{
	const bdBool present = m_creators.containsKey(type);
	return present;
}

