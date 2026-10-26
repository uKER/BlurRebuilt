#ifndef RENDER_DYNAMICEFFECT_HANDLE_H
#define RENDER_DYNAMICEFFECT_HANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

template<class T>
class C_Handle
{
public:
	C_Handle() : m_pObject(0), m_iSalt(0) {}
	C_Handle(T* pObject) { m_pObject = pObject; m_iSalt = pObject->GetSalt(); }
	~C_Handle() {};

public:
	INLINE T* Object()
	{
		if (!m_pObject || (m_pObject->GetSalt() != m_iSalt))
			return 0;

		return m_pObject;
	}

	INLINE const T* Object() const
	{
		if (!m_pObject || (m_pObject->GetSalt() != m_iSalt))
			return 0;

		return m_pObject;
	}

	INLINE const T* ObjectPtr() const
	{
		return m_pObject;
	}
	inline operator T*()					{ return Object(); }
	inline operator const T*() const		{ return Object(); }
	inline operator T&()					{ T*		object = Object(); assert(object != 0); return *object;	}
	inline operator const T&() const		{ const T*	object = Object(); assert(object != 0); return *object;	}
	inline T* operator->()					{ T*		object = Object(); assert(object != 0); return object;	}
	inline const T* operator->() const		{ const T*	object = Object(); assert(object != 0); return object;	}

protected:
	T*		m_pObject;
	U32		m_iSalt;
};

}

#endif // RENDER_DYNAMICEFFECT_HANDLE_H
