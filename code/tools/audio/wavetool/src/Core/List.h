#ifndef LIST_H
#define LIST_H

#ifdef _MSC_VER
#pragma once
#endif

template<typename Type>
class List
{
public:
	inline List() :
		_list(0),
		_length(0),
		_capacity(0)
	{
	}

	inline ~List()
	{
		Clear();
	}

private:
	List(const List<Type>&);
	void operator=(const List<Type>&);

public:
	Type* Append(Type* item)
	{
		if (item == 0)
			return 0;

		if (!ImplExpand(1))
		{
			delete item;
			return 0;
		}

		_list[_length++] = item;

		return item;
	}

	Type* Insert(int before, Type* item)
	{
		if (item == 0)
			return 0;

		if (!ImplExpand(1))
		{
			delete item;
			return 0;
		}

		if (before < 0)
			before = 0;
		else if (before > _length)
			before = _length;

		for(int i = _length; i > before; i--)
			_list[i] = _list[i - 1];

		_list[before] = item;

		_length++;

		return item;
	}

	Type* Replace(int index, Type* item)
	{
		if (item == 0)
			return 0;

		if ((index < 0) || (index >= _length))
		{
			delete item;
			return 0;
		}

		delete _list[index];

		_list[index] = item;

		return item;
	}

public:
	Type* Detach(int index)
	{
		if ((index < 0) || (index >= _length))
			return 0;

		Type* item = _list[index];

		for(int i = index + 1; i < _length; i++)
			_list[i - 1] = _list[i];

		_length--;

		ImplShrink();

		return item;
	}

	bool Remove(int index)
	{
		if ((index < 0) || (index >= _length))
			return false;

		delete _list[index];

		for(int i = index + 1; i < _length; i++)
			_list[i - 1] = _list[i];

		_length--;

		ImplShrink();

		return true;
	}

	void Clear()
	{
		for(int i = 0; i < _length; i++)
			delete _list[i];

		delete [] _list;

		_list		= 0;
		_length		= 0;
		_capacity	= 0;
	}

public:
	inline int Length() const
	{
		return _length;
	}

	inline Type* Get(int index)
	{
		if ((index >= 0) && (index < _length))
			return _list[index];

		return 0;
	}

	inline const Type* Get(int index) const
	{
		if ((index >= 0) && (index < _length))
			return _list[index];

		return 0;
	}

public:
	inline Type& operator[](int index)
	{
		Type* item = Get(index);

		assert(item != 0);

		return *item;
	}

private:
	bool ImplExpand(int extra)
	{
		if ((_length + extra) <= _capacity)
			return true;

		int newCapacity = (_length + extra + 16) & (~0xF);

		Type** list = new Type*[newCapacity];

		if (!list)
			return false;

		for(int i = 0; i < _length; i++)
			list[i] = _list[i];

		delete [] _list;

		_list		= list;
		_capacity	= newCapacity;

		return true;
	}

	void ImplShrink()
	{
		if ((_length + 15) >= (_capacity >> 2))
			return;

		int newCapacity = (_length + 31) & (~0xF);

		Type** list = new Type*[newCapacity];

		if (!list)
			return;

		for(int i = 0; i < _length; i++)
			list[i] = _list[i];

		delete [] _list;

		_list		= list;
		_capacity	= newCapacity;
	}

private:
	Type**	_list;
	int		_length;
	int		_capacity;
};

#endif // LIST_H