#ifndef SCENE_GRAPH_RING_BUFFER_POOL_H
#define SCENE_GRAPH_RING_BUFFER_POOL_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

template <class T>
class DynamicRingBuffer
{
public:
	DynamicRingBuffer();
	~DynamicRingBuffer() {}

	void	Grow(); ///< Grow the buffer if necessary
	void	Reserve(const S32 size); ///< Reserve an initial size of the ring buffer
	S32		Size() const;
	T*		Allocate(); ///< Allocate an object.
	T*		AllocatePrefetch(); ///< Allocate an object and prefetch next.
	void	Prefetch(); ///< Prefetch next object.
	void	Free(T* pFree) {}; ///< Free an object (does nothing)
	S32		SizeInBytes() const;

private:

	Btl::Vector<T>			m_Buffer;
	S32						m_Previous;
	S32						m_Current;
	bool					m_Grow;
};

template <class T>
DynamicRingBuffer<T>::DynamicRingBuffer()
{
	m_Previous = 0;
	m_Current = 0;
	m_Grow = true;
}

template <class T>
INLINE void DynamicRingBuffer<T>::Reserve(const S32 size)
{
	if(m_Buffer.Size() < size)
	{
		m_Buffer.Resize(size);
		m_Grow = false;
	}
}

template <class T>
INLINE void DynamicRingBuffer<T>::Grow()
{
	if(m_Grow)
	{
		m_Buffer.Resize(m_Buffer.Size() * 2 + 1);
		m_Grow = false;
	}
	m_Previous = m_Current;
}

template <class T>
INLINE S32 DynamicRingBuffer<T>::Size() const
{
	return m_Buffer.Size();
}

template <class T>
FORCE_INLINE T* DynamicRingBuffer<T>::Allocate()
{
	if(!m_Grow)
	{
		++m_Current;
		m_Current = m_Current == m_Buffer.Size() ? 0 : m_Current;

		if(m_Current != m_Previous)
			return &m_Buffer[m_Current];
		else
		{
			m_Grow = true;
			return NULL;
		}
	}
	else
		return NULL;

}

template <class T>
FORCE_INLINE T* DynamicRingBuffer<T>::AllocatePrefetch()
{
	if(!m_Grow)
	{
		++m_Current;
		m_Current = m_Current == m_Buffer.Size() ? 0 : m_Current;

		if(m_Current != m_Previous)
		{
			T* pAllocate = &m_Buffer[m_Current];
			DCBT(sizeof(T), pAllocate);
			return pAllocate;
		}
		else
		{
			m_Grow = true;
			return NULL;
		}
	}
	else
		return NULL;

}

template <class T>
FORCE_INLINE void DynamicRingBuffer<T>::Prefetch()
{
	if(!m_Grow)
	{
		S32 current = m_Current + 1;
		current = current == m_Buffer.Size() ? 0 : current;

		if(current != m_Previous)
			DCBT(0, &m_Buffer[current]);
	}
}

template <class T>
FORCE_INLINE S32 DynamicRingBuffer<T>::SizeInBytes() const
{
	return sizeof(DynamicRingBuffer<T>) + sizeof(T) * m_Buffer.Size(); 
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_H
