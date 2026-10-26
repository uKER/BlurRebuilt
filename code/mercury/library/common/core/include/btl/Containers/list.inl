//////////////////////////////////////////////////////////////
// List inline functions
//////////////////////////////////////////////////////////////
	
// add item to the front of the list
template <typename T, typename Parameters>
inline void List<T, Parameters>::PushFront(const T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.First() != 0)
		structure.First()->LE_PushFront(i);
	else // empty list
	{ 
		i.LE_Clear();
		structure.SetLast(&i);
	}

	structure.SetFirst(&i);

	counter.AddElement();
}

template <typename T, typename Parameters>
inline void List<T, Parameters>::PushFront(T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.First() != 0)
		structure.First()->LE_PushFront(i);
	else // empty list
	{ 
		i.LE_Clear();
		structure.SetLast(&i);
	}

	structure.SetFirst(&i);

	counter.AddElement();
}

// add default item to the back of the list (needs double-ended, but not double-linked, external linked only)
template <typename T, typename Parameters>
void List<T, Parameters>::PushFront()
{
	StoredType& i = initialiser.Initialise();

	if (structure.First() != 0)
		structure.First()->LE_PushFront(i);
	else // empty list
	{ 
		i.LE_Clear();
		structure.SetLast(&i);
	}

	structure.SetFirst(&i);

	counter.AddElement();
}

// add item to the back of the list (needs double-ended, but not double-linked)
template <typename T, typename Parameters>
void List<T, Parameters>::PushBack(const T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(i);
	else // empty list
	{
		i.LE_Clear();
		structure.SetFirst(&i);
	}

	structure.SetLast(&i);

	counter.AddElement();
}

// add item to the back of the list (needs double-ended, but not double-linked)
template <typename T, typename Parameters>
void List<T, Parameters>::PushBack(T& item)
{
	StoredType& i = initialiser.Initialise(item);

	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(i);
	else // empty list
	{
		i.LE_Clear();
		structure.SetFirst(&i);
	}

	structure.SetLast(&i);

	counter.AddElement();
}

// add item to the back of the list (needs double-ended, but not double-linked, external linked only)
template <typename T, typename Parameters>
void List<T, Parameters>::PushBack()
{
	StoredType& i = initialiser.Initialise();

	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(i);
	else // empty list
	{
		i.LE_Clear();
		structure.SetFirst(&i);
	}

	structure.SetLast(&i);

	counter.AddElement();
}

// insert item after iterator pointing to list item
template <typename T, typename Parameters>
void List<T, Parameters>::InsertAfter(T& item, StoredType* where)
{
	BTL_ASSERT(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);
	
	where->LE_InsertAfter(i);
	structure.SetFirstIfEqual(&i, 0);
	structure.SetLastIfEqual(&i, where);

	counter.AddElement();
}

// insert item after iterator pointing to list item
template <typename T, typename Parameters>
void List<T, Parameters>::InsertAfter(const T& item, StoredType* where)
{
	BTL_ASSERT(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);

	where->LE_InsertAfter(i);
	structure.SetFirstIfEqual(&i, 0);
	structure.SetLastIfEqual(&i, where);

	counter.AddElement();
}

// insert item before iterator pointing to list item (double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::InsertBefore(T& item, StoredType* where)
{
	BTL_ASSERT(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);

	where->LE_InsertBefore(i);
	structure.SetFirstIfEqual(&i, where);
	structure.SetLastIfEqual(&i, 0);

	counter.AddElement();
}

// insert item before iterator pointing to list item (double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::InsertBefore(const T& item, StoredType* where)
{
	BTL_ASSERT(where != 0); // iterator must point to valid element

	StoredType& i = initialiser.Initialise(item);

	where->LE_InsertBefore(i);
	structure.SetFirstIfEqual(&i, where);
	structure.SetLastIfEqual(&i, 0);

	counter.AddElement();
}

// erase first element in the list
template <typename T, typename Parameters>
void List<T, Parameters>::PopFront()
{
	BTL_ASSERT(structure.First() != 0); // must not be empty list

	StoredType* i = structure.First();
	structure.SetFirst(structure.First()->LE_PopFront());

	if (structure.First() == 0)
		structure.SetLast(0);

	counter.EraseElement();
	initialiser.Release(i);
}

// erase last element in the list (double link and double ended only)
template <typename T, typename Parameters>
void List<T, Parameters>::PopBack()
{
	BTL_ASSERT(structure.Last() != 0); // must not be empty list

	StoredType* i = structure.Last();
	structure.SetLast(structure.Last()->LE_PopBack());

	if (structure.Last() == 0)
		structure.SetFirst(0);

	counter.EraseElement();
	initialiser.Release(i);
}

// erase element after iterator
template <typename T, typename Parameters>
void List<T, Parameters>::EraseNext(StoredType* where)
{
	BTL_ASSERT(where != 0 && where->LE_Next() != 0); // next must be valid item

	StoredType* i = where->LE_Next();
	structure.SetLastIfEqual(where, where->LE_Next());

	where->LE_EraseNext();

	counter.EraseElement();
	initialiser.Release(i);
}

template <typename T, typename Parameters>
void List<T, Parameters>::EraseSingleLinked(Iterator where)
{
	BTL_ASSERT(where != End() && !Empty());

	Iterator i = Begin();

	if (where == i)
	{
		PopFront();
		return;
	}

	Iterator previous = i; ++i;

	while (i != End())
	{
		if (where == i)
		{
			EraseNext(previous);
			return;
		}

		previous = i; ++i;
	}

	BTL_ASSERT(!"Item not found.");
}

// erase element before iterator (double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::ErasePrevious(StoredType* where)
{
	BTL_ASSERT(where != 0 && where->LE_Previous() != 0); // previous must be valid item

	StoredType* i = where->LE_Previous();
	structure.SetFirstIfEqual(where, where->LE_Previous());

	where->LE_ErasePrevious();

	counter.EraseElement();
	initialiser.Release(i);
}

// erase the entire list
template <typename T, typename Parameters>
void List<T, Parameters>::Clear()
{
	while (structure.First() != 0)
		PopFront();
}

// erase element pointed to by iterator (double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::Erase(StoredType* where)
{
	BTL_ASSERT(where != 0); // must be valid element

	StoredType* i = where;
	structure.SetLastIfEqual(where->LE_Previous(), where);
	structure.SetFirstIfEqual(where->LE_Next(), where);

	where->LE_Erase();

	counter.EraseElement();
	initialiser.Release(i);
}

// swap two elements (double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::Swap(Iterator i1, Iterator i2)
{
	BTL_ASSERT(i1 != 0 && i2 != 0); // must be valid elements

	structure.Swap((StoredType*)i1.Pointer(), (StoredType*)i2.Pointer());

	// temporary list element
	ListElementDouble<StoredType> temp;

	((StoredType*)i1.Pointer())->LE_Swap(temp);
	((StoredType*)i2.Pointer())->LE_Swap(*((StoredType*)i1.Pointer()));
	temp.LE_Swap(*((StoredType*)i2.Pointer()));
}

// reverse (double-ended and double-link only)
template <typename T, typename Parameters>
void List<T, Parameters>::Reverse()
{
	structure.Reverse();

	for (Iterator i = Begin(); i != End(); ++i)
	{
		StoredType* p = (StoredType*)(i.Pointer());
		p->LE_Reverse();
	}
}

// Move item from another list into this list.
template <typename T, typename Parameters>
void List<T, Parameters>::MoveFrontToBack(List& other)
{
	BTL_ASSERT(!other.Empty());

	// remove from the other list (pop front)
	StoredType* item = other.structure.First();
	other.structure.SetFirst(item->LE_PopFront());

	if (other.structure.First() == 0)
		other.structure.SetLast(0);

	other.counter.EraseElement();

	// move to this list (push back)
	if (structure.Last() != 0)
		structure.Last()->LE_PushBack(*item);
	else // empty list
	{
		item->LE_Clear();
		structure.SetFirst(item);
	}

	structure.SetLast(item);
	counter.AddElement();
}

// Move all items from another list onto the end of this list.
template <typename T, typename Parameters>
void List<T, Parameters>::Append(List& other)
{
	BTL_ASSERT(!other.Empty());

	counter.Merge(other.counter, other);

	if (structure.Last() != 0)
		structure.Last()->LE_Append(*other.structure.First());
	else
		structure.SetFirst(other.structure.First());

	structure.SetLast(other.structure.Last());

	other.structure.SetFirst(0);
	other.structure.SetLast(0);
}

// Move items from another list onto the end of this list.
template <typename T, typename Parameters>
void List<T, Parameters>::Append(List& other, StoredType* first, StoredType* last, int count)
{
	BTL_ASSERT(first && last && !other.Empty());

	// remove from other list
	if (first == other.structure.First())
		other.structure.SetFirst(last->LE_Next());
	
	if (last == other.structure.Last())
		other.structure.SetLast(first->LE_Previous());

	BTL_ASSERT((other.structure.First() && other.structure.Last()) || (!other.structure.First() && !other.structure.Last()));

	if (first->LE_Previous())
		first->LE_Previous()->LE_SetNext(last->LE_Next());

	if (last->LE_Next())
		last->LE_Next()->LE_SetPrevious(first->LE_Previous());

	// put into this list
	if (structure.Last() != 0)
		structure.Last()->LE_Append(*first);
	else
		structure.SetFirst(first);

	structure.SetLast(last);

	// update counters
	other.counter.Subtract(count);
	counter.Add(count);
}

// Merge items from another list into this list.
// If both lists are sorted, the merged list is also sorted.
template <typename T, typename Parameters>
void List<T, Parameters>::Merge(List& other)
{
	BTL_ASSERT(&other != this);

	List tempList;

	while (!Empty() && !other.Empty())
	{
		Iterator i0 = Begin();
		Iterator i1 = other.Begin();

		if  (*i0 <= *i1)
		{
			StoredType* first = (StoredType*) i0.Pointer();
			Iterator ip = i0;
			int count = 0;

			do
			{
				i0 = ip; 
				++ip;
				count++;
			} while (ip != End() && *ip <= *i1);
		
			StoredType* last = (StoredType*) i0.Pointer();

			tempList.Append(*this, first, last, count);
		}
		else
		{
			StoredType* first = (StoredType*) i1.Pointer();
			Iterator ip = i1;
			int count = 0;

			do
			{
				i1 = ip; 
				++ip;
				count++;
			} while (ip != End() && *ip <= *i0);
		
			StoredType* last = (StoredType*) i1.Pointer();

			tempList.Append(other, first, last, count);
		}
	}

	if (!Empty())
		tempList.Append(*this);

	if (!other.Empty())
		tempList.Append(other);

	Swap(tempList);
}

// using merge sort algorithm from stl
template <typename T, typename Parameters>
void List<T, Parameters>::Sort()
{
	int size = Size();
	if (size < 2) return;

	const int MAX_BINS = 25;
	List unsorted;
	List tempList;
	List bins[MAX_BINS];
	int maxBin = 0;
	int bin = 0;

	Btl::Swap(structure, unsorted.structure);
	unsorted.counter = counter;

	while (!unsorted.Empty())
	{
		tempList.MoveFrontToBack(unsorted);

		for (bin = 0; bin < maxBin && !bins[bin].Empty(); bin++)
		{
			bins[bin].Merge(tempList);
			bins[bin].Swap(tempList);
		}

		if (bin == MAX_BINS)
		{
			bins[bin - 1].Merge(tempList);
		}
		else
		{
			bins[bin].Swap(tempList);
			if (bin == maxBin)
				maxBin++;
		}
	}

	for (int bin = 1; bin < maxBin; bin++)
		bins[bin].Merge(bins[bin - 1]);

	Btl::Swap(structure, bins[maxBin - 1].structure);
}
