#ifndef SCENE_GRAPH_HANDLE_H
#define SCENE_GRAPH_HANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

template <typename T>
class C_Handle
{
public:

	C_Handle() : m_pObject(0), m_Salt(0) {}
	C_Handle(T* p) { m_pObject = p; m_Salt = p->m_Salt; }
	~C_Handle() {};

public:

	INLINE T* Object()
	{
		if (!m_pObject || (m_pObject->m_Salt != m_Salt))
			return 0;

		return m_pObject;
	}

	INLINE const T* Object() const
	{
		if (!m_pObject || (m_pObject->m_Salt != m_Salt))
			return 0;

		return m_pObject;
	}

	inline operator T*()					{ return Object(); }
	inline operator const T*() const		{ return Object(); }
	inline operator T&()					{ T*		object = Object(); assert(object != 0); return *object;	}
	inline operator const T&() const		{ const T*	object = Object(); assert(object != 0); return *object;	}
	inline T* operator->()					{ T*		object = Object(); assert(object != 0); return object;	}
	inline const T* operator->() const		{ const T*	object = Object(); assert(object != 0); return object;	}

protected:

	T*	m_pObject;
	U32	m_Salt;

};

} // namespace SceneGraph

#endif // SCENE_GRAPH_HANDLE_H
