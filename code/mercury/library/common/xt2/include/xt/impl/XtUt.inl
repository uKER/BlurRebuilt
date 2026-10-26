#ifndef BIZARRE_XT_UT_INL
#define BIZARRE_XT_UT_INL

#ifdef _MSC_VER
#pragma once
#endif

// XtUt::PlacementNew

namespace XtUt { struct PlacementNew { }; }

inline void* operator new(size_t, const XtUt::PlacementNew&, void* where)
{
	return where;
}

inline void operator delete(void*, const XtUt::PlacementNew&, void*)
{
}

namespace XtUt
{
	
	// XtUt::Construct

	template<typename Type>
	inline Type* Construct(void* where)
	{
		return new(PlacementNew(), where) Type();
	}

	// XtUt::Construct

	template<typename Type>
	inline Type* Construct(void* where, const Type& toCopy)
	{
		return new(PlacementNew(), where) Type(toCopy);
	}

	// XtUt::ConstructN

	template<typename Type>
	inline Type* ConstructN(void* where, size_t count)
	{
		for(size_t i = 0; i < count; i++)
			new(PlacementNew(), static_cast<Type*>(where) + i) Type();

		return static_cast<Type*>(where);
	}

	// XtUt::Destruct

	template<typename Type>
	inline void Destruct(void* where)
	{
		static_cast<Type*>(where)->~Type();
	}

	// XtUt::DestructN

	template<typename Type>
	inline void DestructN(void* where, size_t count)
	{
		for(size_t i = 0; i < count; i++)
			(static_cast<Type*>(where) + i)->~Type();
	}

	// XtUt::AllocateAsBytes

	template<typename Type>
	inline Type* AllocateAsBytes(size_t count)
	{
		return reinterpret_cast<Type*>(new XtU8[count * sizeof(Type)]);
	}

	// XtUt::FreeAsBytes

	template<typename Type>
	inline void FreeAsBytes(Type* array)
	{
		delete reinterpret_cast<XtU8*>(array);
	}

	// XtUt::Min

	template<typename Type>
	inline Type Min(Type lhs, Type rhs)
	{
		return (lhs < rhs) ? lhs : rhs;
	}

	template<typename Type>
	inline Type Min(Type v0, Type v1, Type v2)
	{
		if (v0 < v1)
			return Min(v0, v2);
		else
			return Min(v1, v2);
	}

	// XtUt::Max

	template<typename Type>
	inline Type Max(Type lhs, Type rhs)
	{
		return (lhs < rhs) ? rhs : lhs;
	}

	template<typename Type>
	inline Type Max(Type v0, Type v1, Type v2)
	{
		if (v0 < v1)
			return Max(v1, v2);
		else
			return Max(v0, v2);
	}

	// XtUt::Lerp

	template<typename ValueType, typename FactorType>
	inline ValueType Lerp(const ValueType& lhs, const ValueType& rhs, const FactorType& factor)
	{
		return (ValueType)(lhs + (rhs - lhs) * factor);
	}

	// XtUt::AlignLower

	template<typename Type>
	inline Type AlignLower(Type value, Type alignment)
	{
		return value & ~(alignment - 1);
	}

	// XtUt::AlignUpper

	template<typename Type>
	inline Type AlignUpper(Type value, Type alignment)
	{
		return ((value) + (alignment - 1)) & ~(alignment - 1);
	}

	// XtUt::MemCpy

	template<typename Type>
	inline void MemCpy(Type* dst, Type* src, size_t count)
	{
		memcpy(dst, src, sizeof(Type) * count);
	}

	// XtUt::MemMove

	template<typename Type>
	inline void MemMove(Type* dst, Type* src, size_t count)
	{
		memmove(dst, src, sizeof(Type) * count);
	}

	//
	// XtUt::Predicate
	//

	// XtUt::LessThan

	template<typename Type>
	struct LessThan
	{
		bool operator()(const Type& lhs, const Type& rhs) const
		{
			return lhs < rhs;
		}
	};

	// XtUt::GreaterThan

	template<typename Type>
	struct GreaterThan
	{
		bool operator()(const Type& lhs, const Type& rhs) const
		{
			return lhs > rhs;
		}
	};

	//
	// XtUt::MergeSort
	//

	// XtUt::MergeSort_Merge

	template<typename Type, typename Behaviour, typename Predicate>
	void MergeSort_Merge(Container<Type, Behaviour>* container, Predicate predicate, Array<Type>* scratch, size_t from, size_t middle, size_t to)
	{
		for(size_t i = from; i < to; i++)
			(*scratch)[i] = (*container)[i];

		size_t lhs = from;
		size_t rhs = middle;
		size_t out = from;

		while((lhs < middle) && (rhs < to))
		{
			if (predicate((*scratch)[lhs], (*scratch)[rhs]))
				(*container)[out++] = (*scratch)[lhs++];
			else
				(*container)[out++] = (*scratch)[rhs++];
		}

		while(lhs < middle)
			(*container)[out++] = (*scratch)[lhs++];

		while(rhs < to)
			(*container)[out++] = (*scratch)[rhs++];
	}

	// XtUt::MergeSort_Divide

	template<typename Type, typename Behaviour, typename Predicate>
	void MergeSort_Divide(Container<Type, Behaviour>* container, Predicate predicate, Array<Type>* scratch, size_t from, size_t to)
	{
		if ((from + 1) >= to)
			return;

		size_t middle = (from + to) / 2;

		MergeSort_Divide(container, predicate, scratch, from, middle);
		MergeSort_Divide(container, predicate, scratch, middle, to);

		MergeSort_Merge(container, predicate, scratch, from, middle, to);
	}

	// XtUt::MergeSort

	template<typename Type, typename Behaviour, typename Predicate>
	bool MergeSort(Container<Type, Behaviour>* container, Predicate predicate)
	{
		if (container->count > 1)
		{
			Array<Type> scratch;

			if (!Allocate(&scratch, container->count, container->count))
				return false;

			MergeSort_Divide(container, predicate, &scratch, 0, container->count);
		}

		return true;
	}

	// XtUt::MergeSort

	template<typename Type, typename Behaviour, typename Predicate>
	bool MergeSort(Container<Type, Behaviour>* container, Predicate predicate, size_t from, size_t to)
	{
		if ((to - from) > 1)
		{
			Array<Type> scratch;

			// TODO: Shouldn't allocate entire container

			if (!Allocate(&scratch, container->count, container->count))
				return false;

			MergeSort_Divide(container, predicate, &scratch, from, to);
		}

		return true;
	}

	//
	// XtUt::HeapSort
	//

	// XtUt::HeapSortSift

	template<typename Type, typename Behaviour, typename Predicate>
	void HeapSortSift(Container<Type, Behaviour>* container, Predicate predicate, int from, int to)
	{
		int i = from;

		for(;;)
		{
			i = (i * 2) + 1;

			if (i >= to)
				return;

			if ((i < (to - 1)) && predicate((*container)[i], (*container)[i + 1]))
				i++;

			if (!predicate((*container)[from], (*container)[i]))
				return;

			Swap(container, from, i);
			from = i;
		}
	}

	// XtUt::HeapSort

	template<typename Type, typename Behaviour, typename Predicate>
	void HeapSort(Container<Type, Behaviour>* container, Predicate predicate)
	{
		for(int i = (int)(container->count / 2) - 1; i >= 0; i--)
			HeapSortSift(container, predicate, i, (int)container->count);

		for(int i = (int)container->count - 1; i > 0; i--)
		{
			Swap(container, i, 0);
			HeapSortSift(container, predicate, 0, i);
		}
	}

	//
	// XtUt::Block
	//

	// XtUt::Block::Capacity

	inline size_t Capacity(const Block* mb)
	{
		return mb->end - mb->start;
	}

	// XtUt::Block::Used

	inline size_t Used(const Block* mb)
	{
		return mb->cursor - mb->start;
	}

	// XtUt::Block::Available

	inline size_t Available(const Block* mb)
	{
		return mb->end - mb->cursor;
	}

	// XtUt::Block::Allocate

	inline XtU8* Allocate(Block* mb, size_t capacity)
	{
		if (Capacity(mb) != capacity)
		{
			delete [] mb->start;

			if (capacity > 0)
			{
				if ((mb->start = new XtU8[capacity]) != 0)
					mb->end = mb->start + capacity;
				else
					mb->end = 0;
			}
			else
				mb->start = mb->end = 0;
		}

		mb->cursor = mb->start;

		return mb->start;
	}

	// XtUt::Block::Abandon

	inline void Abandon(Block* mb)
	{
		mb->start = mb->cursor = mb->end = 0;
	}

	// XtUt::Block::Free

	inline void Free(Block* mb)
	{
		delete [] mb->start;

		Abandon(mb);
	}

	// XtUt::Block::ReplaceData

	inline void ReplaceData(Block* destination, Block* source)
	{
		delete [] destination->start;

		destination->cursor	= source->start + Used(destination);

		destination->start	= source->start;
		destination->end	= source->end;

		Abandon(source);
	}

	// XtUt::Block::Read

	inline size_t Read(Block* mb, void* data, size_t length)
	{
		size_t toCopy = XtUt::Min(Available(mb), length);

		if (toCopy > 0)
		{
			memcpy(data, mb->cursor, toCopy);

			mb->cursor += toCopy;
		}

		return toCopy;
	}

	// XtUt::Block::Read

	inline size_t Read(Block* mb, size_t offset, void* data, size_t length)
	{
		if (offset >= Capacity(mb))
			return 0;

		size_t toCopy = XtUt::Min(Capacity(mb) - offset, length);

		if (toCopy > 0)
			memcpy(data, mb->start + offset, toCopy);

		return toCopy;
	}

	// XtUt::Block::Write

	inline size_t Write(Block* mb, const void* data, size_t length)
	{
		size_t toCopy = XtUt::Min(Available(mb), length);

		if (toCopy > 0)
		{
			memcpy(mb->cursor, data, toCopy);

			mb->cursor += toCopy;
		}

		return toCopy;
	}

	// XtUt::Block::Write

	inline size_t Write(Block* mb, size_t offset, const void* data, size_t length)
	{
		if (offset >= Capacity(mb))
			return 0;

		size_t toCopy = XtUt::Min(Capacity(mb) - offset, length);

		if (toCopy > 0)
			memcpy(mb->start + offset, data, toCopy);

		return toCopy;
	}

	// XtUt::Block::Clear

	inline void Clear(Block* mb)
	{
		memset(mb->start, 0, Capacity(mb));
	}

	//
	// XtUt::Container
	//

	template<typename Type, typename Behaviour>
	Type* Elements(Container<Type, Behaviour>* container)
	{
		assert(container != 0);
		return container->elements;
	}

	// XtUt::Container::Count

	template<typename Type, typename Behaviour>
	size_t Count(Container<Type, Behaviour>* container)
	{
		assert(container != 0);
		return container->count;
	}

	// XtUt::Container::Capacity

	template<typename Type, typename Behaviour>
	size_t Capacity(Container<Type, Behaviour>* container)
	{
		assert(container != 0);
		return container->capacity;
	}

	// XtUt::Container::Available

	template<typename Type, typename Behaviour>
	size_t Available(Container<Type, Behaviour>* container)
	{
		assert(container != 0);
		return container->capacity - container->count;
	}

	// XtUt::Container::Allocate

	template<typename Type, typename Behaviour>
	bool Allocate(Container<Type, Behaviour>* container, size_t count, size_t capacity)
	{
		assert(container != 0);
		assert(count <= capacity);

		Behaviour::Dispose(container->elements, container->count);
		Behaviour::Destruct(container->elements, container->count);

		container->count = 0;

		if (container->capacity != capacity)
		{
			FreeElements(container);

			if (capacity > 0)
			{
				if ((container->elements = Behaviour::Allocate(capacity)) == 0)
				{
					container->capacity = 0;

					return false;
				}
			}
			else
				container->elements = 0;

			container->capacity = capacity;
		}

		return (count <= 0) || (AppendN(container, count) != 0);
	}

	// XtUt::Container::Abandon

	template<typename Type, typename Behaviour>
	inline void Abandon(Container<Type, Behaviour>* container)
	{
		assert(container != 0);

		Behaviour::Abandon(container->elements, 0, container->count);

		container->elements	= 0;
		container->count	= 0;
		container->capacity	= 0;
	}

	// XtUt::Container::FreeElements

	template<typename Type, typename Behaviour>
	void FreeElements(Container<Type, Behaviour>* container)
	{
		assert(container != 0);

		Behaviour::Dispose(container->elements, container->count);
		Behaviour::Destruct(container->elements, container->count);

		Behaviour::Free(container->elements);
	}

	// XtUt::Container::Free

	template<typename Type, typename Behaviour>
	inline void Free(Container<Type, Behaviour>* container)
	{
		assert(container != 0);

		FreeElements(container);

		container->elements	= 0;
		container->count	= 0;
		container->capacity	= 0;
	}

	// XtUt::Container::Reserve

	template<typename Type, typename Behaviour>
	bool Reserve(Container<Type, Behaviour>* container, size_t capacity)
	{
		assert(container != 0);

		if (capacity <= container->capacity)
			return true;

		Type* elements;

		if ((elements = Behaviour::Allocate(capacity)) == 0)
			return false;

		MemCpy(elements, container->elements, container->count);

		Behaviour::Free(container->elements);

		container->elements = elements;
		container->capacity = capacity;

		return true;
	}

	// XtUt::Container::Clear

	template<typename Type, typename Behaviour>
	void Clear(Container<Type, Behaviour>* container, size_t index, size_t count)
	{
		assert(container != 0);

		Type* where = container->elements + index;

		Behaviour::Dispose(where, count);
		Behaviour::Destruct(where, count);
		Behaviour::Construct(where, count);
	}

	// XtUt::Container::Clear

	template<typename Type, typename Behaviour>
	inline void Clear(Container<Type, Behaviour>* container)
	{
		assert(container != 0);

		Clear(container, 0, container->count);
	}

	// XtUt::Container::At

	template<typename Type, typename Behaviour>
	inline Type* At(Container<Type, Behaviour>* container, size_t index)
	{
		assert(container != 0);
		return (index < container->count) ? &container->elements[index] : 0;
	}

	// XtUt::Container::AppendN

	template<typename Type, typename Behaviour>
	Type* AppendN(Container<Type, Behaviour>* container, size_t count)
	{
		assert(container != 0);

		if (count > Available(container))
			return 0;

		Type* where = container->elements + container->count;

		Behaviour::Construct(where, count);

		container->count += count;

		return where;
	}

	// XtUt::Container::Append

	template<typename Type, typename Behaviour>
	Type* Append(Container<Type, Behaviour>* container, const Type& object)
	{
		assert(container != 0);

		if (container->count >= container->capacity)
		{
			Behaviour::Dispose(&object, 1);
			return 0;
		}

		return Behaviour::CopyConstruct(&container->elements[container->count++], object);
	}

	// XtUt::Container::InsertN

	template<typename Type, typename Behaviour>
	Type* InsertN(Container<Type, Behaviour>* container, size_t before, size_t count)
	{
		assert(container != 0);

		if ((before > container->count) || (count > Available(container)))
			return 0;

		Type* where = container->elements + before;

		MemMove(where + count, where, container->count - before);

		Behaviour::Construct(where, count);

		container->count += count;

		return where;
	}

	// XtUt::Container::Insert

	template<typename Type, typename Behaviour>
	Type* Insert(Container<Type, Behaviour>* container, size_t before, const Type& object)
	{
		assert(container != 0);

		if ((before > container->count) || (Available(container) < 1))
		{
			Behaviour::Dispose(&object, 1);
			return 0;
		}

		Type* where = container->elements + before;

		MemMove(where + 1, where, container->count - before);

		container->count++;

		return Behaviour::CopyConstruct(&container->elements[before], object);
	}

	// XtUt::Container::RemoveN

	template<typename Type, typename Behaviour>
	bool RemoveN(Container<Type, Behaviour>* container, size_t index, size_t count)
	{
		assert(container != 0);

		if ((index > container->count) || (count > (container->count - index)))
			return false;

		Type* where = container->elements + index;

		Behaviour::Dispose(where, count);
		Behaviour::Destruct(where, count);

		MemMove(where, where + count, container->count - (index + count));

		container->count -= count;

		return true;
	}

	// XtUt::Container::Remove

	template<typename Type, typename Behaviour>
	bool Remove(Container<Type, Behaviour>* container, size_t index)
	{
		assert(container != 0);

		return RemoveN(container, index, 1);
	}

	// XtUt::Container::ReplaceN

	template<typename Type, typename Behaviour>
	Type* ReplaceN(Container<Type, Behaviour>* container, size_t index, size_t count)
	{
		assert(container != 0);

		if ((index > container->count) || (count > (container->count - index)))
			return 0;

		Clear(container, index, count);

		return container->elements + index;
	}

	// XtUt::Container::Replace

	template<typename Type, typename Behaviour>
	Type* Replace(Container<Type, Behaviour>* container, size_t index, const Type& object)
	{
		assert(container != 0);

		if (index >= container->count)
		{
			Behaviour::Dispose(&object, 1);
			return 0;
		}

		Type* where = container->elements + index;

		Behaviour::Dispose(where, 1);
		Behaviour::Destruct(where, 1);

		return Behaviour::CopyConstruct(where, object);
	}

	// XtUt::Container::Replace

	template<typename Type, typename Behaviour>
	void Replace(Container<Type, Behaviour>* dst, Container<Type, Behaviour>* src)
	{
		assert(dst != 0);
		assert(src != 0);

		FreeElements(dst);

		dst->elements	= src->elements;
		dst->count		= src->count;
		dst->capacity	= src->capacity;

		src->elements	= 0;
		src->count		= 0;
		src->capacity	= 0;
	}

	// XtUt::Container::Swap

	template<typename Type, typename Behaviour>
	bool Swap(Container<Type, Behaviour>* container, size_t i0, size_t i1)
	{
		assert(container != 0);

		if ((i0 >= container->count) || (i1 >= container->count))
			return false;

		if (i0 == i1)
			return true;

		XtU8 tmp[sizeof(Type)];

		Type* where0 = container->elements + i0;
		Type* where1 = container->elements + i1;

		MemCpy((Type*)tmp, where0, 1);
		MemCpy(where0, where1, 1);
		MemCpy(where1, (Type*)tmp, 1);

		return true;
	}

	// XtUt::Container::ComputeCapacityEx

	template<typename Type, typename Behaviour>
	inline size_t ComputeExtendedCapacity(Container<Type, Behaviour>* container, size_t count)
	{
		assert(container != 0);

		return XtUt::AlignUpper(XtUt::Max(
				container->count + count,
				container->capacity + XtUt::Max(container->capacity / 2, 128 / sizeof(Type), (size_t)4)
			), (size_t)4);
	}

	// XtUt::Container::EnsureSpace

	template<typename Type, typename Behaviour>
	bool EnsureSpace(Container<Type, Behaviour>* container, size_t count)
	{
		assert(container != 0);

		if (count <= Available(container))
			return true;

		return Reserve(container, ComputeExtendedCapacity(container, count));
	}

	// XtUt::Container::AppendExN

	template<typename Type, typename Behaviour>
	inline Type* AppendExN(Container<Type, Behaviour>* container, size_t count)
	{
		assert(container != 0);

		return EnsureSpace(container, count) ? AppendN(container, count) : 0;
	}

	// XtUt::Container::AppendEx

	template<typename Type, typename Behaviour>
	Type* AppendEx(Container<Type, Behaviour>* container, const Type& object)
	{
		assert(container != 0);

		if (!EnsureSpace(container, 1))
		{
			Behaviour::Dispose(&object, 1);
			return 0;
		}

		return Append(container, object);
	}

	// XtUt::Container::InsertExN

	template<typename Type, typename Behaviour>
	inline Type* InsertExN(Container<Type, Behaviour>* container, size_t before, size_t count)
	{
		assert(container != 0);

		if (before > container->count)
			return 0;

		return EnsureSpace(container, count) ? InsertN(container, before, count) : 0;
	}

	// XtUt::Container::InsertEx

	template<typename Type, typename Behaviour>
	inline Type* InsertEx(Container<Type, Behaviour>* container, size_t before, const Type& object)
	{
		assert(container != 0);

		if ((before > container->count) || !EnsureSpace(container, 1))
		{
			Behaviour::Dispose(&object, 1);
			return 0;
		}

		return Insert(container, before, object);
	}

	//
	// XtUt::List
	//

	// XtUt::List::At

	template<typename Type>
	inline Type* At(List<Type>* list, size_t index)
	{
		assert(list != 0);
		return (index < list->count) ? list->elements[index] : 0;
	}

	// XtUt::List::Append

	template<typename Type>
	inline Type* Append(List<Type>* list, Type* object)
	{
		assert(list != 0);

		if (object && Append(static_cast<typename List<Type>::ContainerType*>(list), object))
			return object;

		return 0;
	}

	// XtUt::List::AppendEx

	template<typename Type>
	inline Type* AppendEx(List<Type>* list, Type* object)
	{
		assert(list != 0);

		if (object && AppendEx(static_cast<typename List<Type>::ContainerType*>(list), object))
			return object;

		return 0;
	}

	// XtUt::List::Insert

	template<typename Type>
	inline Type* Insert(List<Type>* list, size_t before, Type* object)
	{
		assert(list != 0);

		if (object && Insert(static_cast<typename List<Type>::ContainerType*>(list), before, object))
			return object;

		return 0;
	}

	// XtUt::List::InsertEx

	template<typename Type>
	inline Type* InsertEx(List<Type>* list, size_t before, Type* object)
	{
		assert(list != 0);

		if (object && InsertEx(static_cast<typename List<Type>::ContainerType*>(list), before, object))
			return object;

		return 0;
	}

	//
	// XtUt::Set
	//

	// XtUt::Dictionary::Count

	template<typename Type, typename Pred>
	size_t Count(Set<Type, Pred>* set)
	{
		return set->elements.count;
	}

	// XtUt::Set::Free

	template<typename Type, typename Pred>
	void Free(Set<Type, Pred>* set)
	{
		assert(set != 0);

		Free(&set->elements);
	}

	// XtUt::Set::At

	template<typename Type, typename Pred>
	Type* At(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		size_t index = FirstOf(set, value);

		if ((index >= set->elements.count) || set->predicate(value, set->elements[index]))
			return 0;

		return &set->elements[index];
	}

	// XtUt::Set::Insert

	template<typename Type, typename Pred>
	Type* Insert(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		return InsertEx(&set->elements, LastOf(set, value), value);
	}

	// XtUt::Set::Remove

	template<typename Type, typename Pred>
	bool Remove(Set<Type, Pred>* set, size_t from, size_t to)
	{
		assert(set != 0);

		return (from <= to) && RemoveN(&set->elements, from, to - from);
	}

	// XtUt::Container::FirstOf

	template<typename Type, typename Pred>
	size_t FirstOf(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		size_t top		= 0;
		size_t bottom	= set->elements.count;

		while(top < bottom)
		{
			size_t middle = (top + bottom) / 2;

			if (set->predicate(set->elements[middle], value))
				top = middle + 1;
			else
				bottom = middle;
		}

		return top;
	}

	// XtUt::Set::LastOf

	template<typename Type, typename Pred>
	size_t LastOf(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		size_t top		= 0;
		size_t bottom	= set->elements.count;

		while(top < bottom)
		{
			size_t middle = (top + bottom) / 2;

			if (!set->predicate(value, set->elements[middle]))
				top = middle + 1;
			else
				bottom = middle;
		}

		return bottom;
	}

	// XtUt::Set::Exists

	template<typename Type, typename Pred>
	bool Exists(Set<Type, Pred>* set, const Type& value)
	{
		assert(set != 0);

		size_t top = FirstOf(set, value);

		return (top < set->elements.count) && !set->predicate(value, set->elements[top]);
	}

	//
	// XtUt::Dictionary
	//

	// XtUt::Dictionary::Count

	template<typename Kt, typename Vt, typename Pred>
	size_t Count(Dictionary<Kt, Vt, Pred>* dict)
	{
		return dict->pairs.elements.count;
	}

	// XtUt::Dictionary::Free

	template<typename Kt, typename Vt, typename Pred>
	void Free(Dictionary<Kt, Vt, Pred>* dict)
	{
		assert(dict != 0);

		Free(&dict->pairs);
	}

	// XtUt::Dictionary::At

	template<typename Kt, typename Vt, typename Pred>
	typename Dictionary<Kt, Vt, Pred>::ValueType* At(Dictionary<Kt, Vt, Pred>* dict, const typename Dictionary<Kt, Vt, Pred>::KeyType& key)
	{
		assert(dict != 0);

		typename Dictionary<Kt, Vt, Pred>::Pair* pair = At(&dict->pairs, typename Dictionary<Kt, Vt, Pred>::Pair(key));

		return pair ? pair->value : 0;
	}

	// XtUt::Dictionary::Insert

	template<typename Kt, typename Vt, typename Pred>
	bool Insert(Dictionary<Kt, Vt, Pred>* dict, const typename Dictionary<Kt, Vt, Pred>::KeyType& key, typename Dictionary<Kt, Vt, Pred>::ValueType* value)
	{
		assert(dict != 0);

		return value && Insert(&dict->pairs, typename Dictionary<Kt, Vt, Pred>::Pair(key, value));
	}

	// XtUt::Dictionary::Replace

	template<typename Kt, typename Vt, typename Pred>
	bool Replace(Dictionary<Kt, Vt, Pred>* dict, size_t index, typename Dictionary<Kt, Vt, Pred>::ValueType* value)
	{
		assert(dict != 0);

		if (!value)
			return false;

		if (index >= dict->pairs.elements.count)
		{
			delete value;
			return false;
		}

		typename Dictionary<Kt, Vt, Pred>::Pair* pair = &dict->pairs.elements[index];

		delete pair->value;

		pair->value = value;

		return true;
	}

	// XtUt::Dictionary::Remove

	template<typename Kt, typename Vt, typename Pred>
	bool Remove(Dictionary<Kt, Vt, Pred>* dict, size_t from, size_t to)
	{
		assert(dict != 0);

		return Remove(&dict->pairs, from, to);
	}

	// XtUt::Dictionary::Detach

	template<typename Kt, typename Vt, typename Pred>
	Vt* Detach(Dictionary<Kt, Vt, Pred>* dict, size_t index)
	{
		assert(dict != 0);

		if (index >= dict->pairs.elements.count)
			return 0;

		typename Dictionary<Kt, Vt, Pred>::Pair* pair = &dict->pairs.elements[index];

		Vt* value = pair->value;

		pair->value = 0;

		Remove(&dict->pairs, index, index + 1);

		return value;
	}

	// XtUt::Container::FirstOf

	template<typename Kt, typename Vt, typename Pred>
	size_t FirstOf(Dictionary<Kt, Vt, Pred>* dict, const typename Dictionary<Kt, Vt, Pred>::KeyType& key)
	{
		assert(dict != 0);

		return FirstOf(&dict->pairs, typename Dictionary<Kt, Vt, Pred>::Pair(key));
	}

	// XtUt::Dictionary::LastOf

	template<typename Kt, typename Vt, typename Pred>
	size_t LastOf(Dictionary<Kt, Vt, Pred>* dict, const typename Dictionary<Kt, Vt, Pred>::KeyType& key)
	{
		assert(dict != 0);

		return LastOf(&dict->pairs, typename Dictionary<Kt, Vt, Pred>::Pair(key));
	}

	// XtUt::Dictionary::Exists

	template<typename Kt, typename Vt, typename Pred>
	bool Exists(Dictionary<Kt, Vt, Pred>* dict, const typename Dictionary<Kt, Vt, Pred>::KeyType& key)
	{
		assert(dict != 0);

		return Exists(&dict->pairs, typename Dictionary<Kt, Vt, Pred>::Pair(key));
	}

	//
	// XtUt::Iterators
	//

	// XtUt::ContainerIterator

	template<typename Type, typename Behaviour>
	struct ContainerIterator
	{
		Container<Type, Behaviour>*	container;
		size_t						index;

	public:
		inline ContainerIterator(Container<Type, Behaviour>* container_) :
			container(container_),
			index(0)
		{
		}

	public:
		inline operator bool()
		{
			return index < container->count;
		}

		inline operator Type*()
		{
			assert(index < container->count);
			return &(*container)[index];
		}

		inline operator Type&()
		{
			assert(index < container->count);
			return (*container)[index];
		}

		inline Type& operator*()
		{
			assert(index < container->count);
			return (*container)[index];
		}

		inline Type* operator->()
		{
			assert(index < container->count);
			return &(*container)[index];
		}

	public:
		inline void operator++()
		{
			index++;
		}
	};

	// XtUt::ConstContainerIterator

	template<typename Type, typename Behaviour>
	struct ConstContainerIterator
	{
		const Container<Type, Behaviour>*	container;
		size_t								index;

	public:
		inline ConstContainerIterator(const Container<Type, Behaviour>* container_) :
			container(container_),
			index(0)
		{
		}

	public:
		inline operator bool()
		{
			return index < container->count;
		}

		inline operator const Type*()
		{
			assert(index < container->count);
			return &(*container)[index];
		}

		inline operator const Type&()
		{
			assert(index < container->count);
			return (*container)[index];
		}

		inline const Type& operator*()
		{
			assert(index < container->count);
			return (*container)[index];
		}

		inline const Type* operator->()
		{
			assert(index < container->count);
			return &(*container)[index];
		}

	public:
		inline void operator++()
		{
			index++;
		}
	};

	// XtUt::ListIterator

	template<typename Type>
	struct ListIterator
	{
		List<Type>*	list;
		size_t		index;

	public:
		inline ListIterator(List<Type>* list_) :
			list(list_),
			index(0)
		{
		}

	public:
		inline operator bool()
		{
			return index < list->count;
		}

		inline operator Type*()
		{
			assert(index < list->count);
			return (*list)[index];
		}

		inline operator Type&()
		{
			assert(index < list->count);
			return (*list)[index];
		}

		inline Type* operator->()
		{
			assert(index < list->count);
			return (*list)[index];
		}

	public:
		inline void operator++()
		{
			index++;
		}
	};

	// XtUt::SetIterator

	template<typename Type, typename Pred>
	struct SetIterator
	{
		Set<Type, Pred>*	set;
		size_t				index;

	public:
		inline SetIterator(Set<Type, Pred>* set_) :
			set(set_),
			index(0)
		{
		}

	public:
		inline operator bool()
		{
			return index < set->elements.count;
		}

		inline operator Type*()
		{
			assert(index < set->elements.count);
			return &set->elements[index];
		}

		inline operator Type&()
		{
			assert(index < set->elements.count);
			return set->elements[index];
		}

		inline Type* operator->()
		{
			assert(index < set->elements.count);
			return &set->elements[index];
		}

	public:
		inline void operator++()
		{
			index++;
		}
	};

	// XtUt::DictionaryIterator

	template<typename Kt, typename Vt, typename Pred>
	struct DictionaryIterator
	{
		Dictionary<Kt, Vt, Pred>*	dict;
		size_t						index;

	public:
		inline DictionaryIterator(Dictionary<Kt, Vt, Pred>* dict_) :
			dict(dict_),
			index(0)
		{
		}

	public:
		inline operator bool()
		{
			return index < dict->pairs.elements.count;
		}

		inline operator Vt*()
		{
			assert(index < dict->pairs.elements.count);
			return dict->pairs.elements[index].value;
		}

		inline operator Vt&()
		{
			assert(index < dict->pairs.elements.count);
			return *dict->pairs.elements[index].value;
		}

		inline Vt* operator->()
		{
			assert(index < dict->pairs.elements.count);
			return dict->pairs.elements[index].value;
		}

	public:
		inline void operator++()
		{
			index++;
		}

	public:
		inline const Kt& Key()
		{
			assert(index < dict->pairs.elements.count);
			return dict->pairs.elements[index].key;
		}

		inline const Vt& Value()
		{
			assert(index < dict->pairs.elements.count);
			return *dict->pairs.elements[index].value;
		}
	};

	//
	// XtUt::Ptr
	//

	// XtUt::Ptr::Push

	template<typename Type>
	Type* Push(Ptr<Type>* ptr, Type* newObject)
	{
		assert(ptr != 0);

		Type* object = ptr->object;

		ptr->object = newObject;

		return object;
	}

	// XtUt::Ptr::Pop

	template<typename Type>
	Type* Pop(Ptr<Type>* ptr)
	{
		assert(ptr != 0);

		return Push(ptr, (Type*)0);
	}

	//
	// XtUt::Link
	//

	// XtUt::Link::AttachBefore

	template<typename Type>
	void AttachBefore(Link<Type>* link, Link<Type>* before)
	{
		assert(link != 0);
		assert(link->object != 0);
		assert(before != 0);

		Detach(link);

		link->next			= before;
		link->prev			= before->prev;
		before->prev->next	= link;
		before->prev		= link;
	}

	// XtUt::Link::AttachAfter

	template<typename Type>
	void AttachAfter(Link<Type>* link, Link<Type>* after)
	{
		assert(link != 0);
		assert(link->object != 0);
		assert(after != 0);

		Detach(link);

		link->next			= after->next;
		link->prev			= after;
		after->next->prev	= link;
		after->next			= link;
	}

	// XtUt::Link::Detach

	template<typename Type>
	void Detach(Link<Type>* link)
	{
		assert(link != 0);

		link->next->prev	= link->prev;
		link->prev->next	= link->next;
		link->next			= link;
		link->prev			= link;
	}

	//
	// XtUt::LinkList
	//

	// XtUt::LinkList::Free

	template<typename Type>
	void Free(LinkList<Type>* list)
	{
		assert(list != 0);

		while(Type* object = PopBack(list))
			delete object;
	}

	// XtUt::LinkList::Abandon

	template<typename Type>
	void Abandon(LinkList<Type>* list)
	{
		assert(list != 0);

		while(PopBack(list))
			continue;
	}

	// XtUt::LinkList::PushFront

	template<typename Type>
	void PushFront(LinkList<Type>* list, Link<Type>* link)
	{
		assert(list != 0);
		assert(link != 0);

		AttachAfter(link, &list->elements);
	}

	// XtUt::LinkList::PushBack

	template<typename Type>
	void PushBack(LinkList<Type>* list, Link<Type>* link)
	{
		assert(list != 0);
		assert(link != 0);

		AttachBefore(link, &list->elements);
	}

	// XtUt::LinkList::PopFront

	template<typename Type>
	Type* PopFront(XtUt::LinkList<Type>* list)
	{
		assert(list != 0);

		Link<Type>* link = list->elements.next;

		if (!link->object)
			return 0;

		Detach(link);

		return link->object;
	}

	// XtUt::LinkList::PopBack

	template<typename Type>
	Type* PopBack(LinkList<Type>* list)
	{
		assert(list != 0);

		Link<Type>* link = list->elements.prev;

		if (!link->object)
			return 0;

		Detach(link);

		return link->object;
	}

	// XtUt::LinkList::IsEmpty

	template<typename Type>
	bool IsEmpty(const LinkList<Type>* list)
	{
		assert(list != 0);

		return list->elements.next == &list->elements;
	}

	// XtUt::LinkListIterator

	template<typename Type>
	struct LinkListIterator
	{
		Link<Type>* current;

	public:
		inline LinkListIterator(LinkList<Type>* list) :
			current(list->elements.next)
		{
		}

	public:
		inline operator Type*()
		{
			return current->object;
		}

		inline operator Type&()
		{
			assert(current->object != 0);
			return *current->object;
		}

		inline Type* operator->()
		{
			assert(current->object != 0);
			return current->object;
		}

	public:
		inline void operator++()
		{
			current = current->next;
		}
	};

	// XtUt::LinkListReverseIterator

	template<typename Type>
	struct LinkListReverseIterator
	{
		Link<Type>* current;

	public:
		inline LinkListReverseIterator(LinkList<Type>* list) :
			current(list->elements.prev)
		{
		}

	public:
		inline operator Type*()
		{
			return current->object;
		}

		inline operator Type&()
		{
			assert(current->object != 0);
			return *current->object;
		}

		inline Type* operator->()
		{
			assert(current->object != 0);
			return current->object;
		}

	public:
		inline void operator++()
		{
			current = current->prev;
		}
	};

	// XtUt::LinkListConstIterator

	template<typename Type>
	struct LinkListConstIterator
	{
		const Link<Type>* current;

	public:
		inline LinkListConstIterator(const LinkList<Type>* list) :
			current(list->elements.next)
		{
		}

	public:
		inline operator const Type*()
		{
			return current->object;
		}

		inline operator const Type&()
		{
			assert(current->object != 0);
			return *current->object;
		}

		inline const Type* operator->()
		{
			assert(current->object != 0);
			return current->object;
		}

	public:
		inline void operator++()
		{
			current = current->next;
		}
	};

	// XtUt::LinkListConstReverseIterator

	template<typename Type>
	struct LinkListConstReverseIterator
	{
		const Link<Type>* current;

	public:
		inline LinkListConstReverseIterator(const LinkList<Type>* list) :
			current(list->elements.prev)
		{
		}

	public:
		inline operator const Type*()
		{
			return current->object;
		}

		inline operator const Type&()
		{
			assert(current->object != 0);
			return *current->object;
		}

		inline const Type* operator->()
		{
			assert(current->object != 0);
			return current->object;
		}

	public:
		inline void operator++()
		{
			current = current->prev;
		}
	};

}  // namespace XtUt

#endif // BIZARRE_XT_UT_INL
