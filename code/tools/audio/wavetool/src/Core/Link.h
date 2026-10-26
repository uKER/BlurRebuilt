#ifndef LINK_H
#define LINK_H

#ifdef _MSC_VER
#pragma once
#endif

template<typename> class Link;
template<typename> class LinkList;
template<typename> class LinkIterator;

//
// Link
//

template<typename Type>
class Link
{
public:
	inline Link(Type* object) :
		_object(object),
		_next(this),
		_previous(this)
	{
		assert(object != 0);
	}

	inline Link(Type* object, LinkList<Type>& list) :
		_object(object),
		_next(this),
		_previous(this)
	{
		assert(object != 0);

		AttachBefore(list);
	}

	inline ~Link()
	{
		Detach();
	}

private:
	inline Link(LinkList<Type>& list) :
		_object(0),
		_next(this),
		_previous(this)
	{
	}

public:
	inline void AttachBefore(Link<Type>* link)
	{
		Detach();

		_next				= link;
		_previous			= link->_previous;
		_previous->_next	= this;
		_next->_previous	= this;
	}

	inline void AttachAfter(Link<Type>* link)
	{
		Detach();

		_previous			= link;
		_next				= link->_next;
		_next->_previous	= this;
		_previous->_next	= this;
	}
	
	inline void Detach()
	{
		_previous->_next	= _next;
		_next->_previous	= _previous;
		_next				= this;
		_previous			= this;
	}

public:
	inline operator Type*()
	{
		return object;
	}

	inline operator Type&()
	{
		return *object;
	}

	inline Type& operator->()
	{
		return *object;
	}

	inline operator const Type*() const
	{
		return object;
	}

	inline operator const Type&() const
	{
		return *object;
	}

	inline const Type& operator->() const
	{
		return *object;
	}

private:
	Type*		_object;
	Link<Type>*	_next;
	Link<Type>*	_previous;

template<typename> friend class LinkList;
template<typename> friend class LinkIterator;
};

//
// LinkList
//

template<typename Type>
class LinkList
{
public:
	LinkList() :
		_root(*this)
	{
	}

	~LinkList()
	{
	}

public:
	void Clear()
	{
		for(LinkIterator<Type> it(*this); it; )
		{
			Type* object = it;

			++it;

			delete object;
		}
	}

public:
	operator Link<Type>&()
	{
		return _root;
	}

	operator Link<Type>*()
	{
		return &_root;
	}

private:
	Link<Type> _root;

template<typename> friend class LinkIterator;
};

//
// LinkIterator
//

template<typename Type>
class LinkIterator
{
public:
	__forceinline LinkIterator(LinkList<Type>& list) :
		_current(list._root._next)
	{
	}

public:
	__forceinline operator Type*()
	{
		return _current->_object;
	}

	__forceinline operator Type&()
	{
		return *_current->_object;
	}

	__forceinline Type* operator->()
	{
		return _current->_object;
	}

	__forceinline const Type* operator->() const
	{
		return _current->_object;
	}

	__forceinline Type& operator*()
	{
		return *_current->_object;
	}

	__forceinline const Type& operator*() const
	{
		return *_current->_object;
	}

	__forceinline void operator++()
	{
		_current = _current->_next;
	}

private:
	Link<Type>* _current;
};

#endif // LINK_H