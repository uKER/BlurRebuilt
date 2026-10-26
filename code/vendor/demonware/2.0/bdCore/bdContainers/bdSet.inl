// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


/// As a general rule this class does not do anything fancy, it simply
///  acts as an interface on top of the red black tree.

/// However if we wish to get it to do fancy iteration stuff
///  it could expand.

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdSet<T, LESS_THAN_FUNCTOR>::bdSet()
: 
#ifdef BD_DEBUG
m_numIterators(0),
#endif // BD_DEBUG
m_tree()
{
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdSet<T, LESS_THAN_FUNCTOR>::bdSet(const bdSet& other)
: 
#ifdef BD_DEBUG
m_numIterators(0),
#endif // BD_DEBUG
m_tree()
{
#ifdef BD_PLATFORM_WIN32
#pragma message(" ")
#pragma message("BitDemon performace warning: bdSet copy constructor is slow.")
#pragma message(" ")
#endif // BD_PLATFORM_WIN32

	Iterator iter = other.getIterator();

	while(iter)
	{
		const T& element = other.next(iter);
		insert(element);
	}	
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdSet<T, LESS_THAN_FUNCTOR>::~bdSet()
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdSet::~bdSet, another iterator is being held while deleting set");
#endif // BD_DEBUG
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdUInt bdSet<T, LESS_THAN_FUNCTOR>::getSize() const
{
	return m_tree.getSize();
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdBool bdSet<T, LESS_THAN_FUNCTOR>::insert(const T& object)
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdSet::insert, another iterator is being held while inserting "
		"to set");
#endif // BD_DEBUG
	return m_tree.insert(object);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
bdBool bdSet<T, LESS_THAN_FUNCTOR>::remove(const T& object)
{
#ifdef BD_DEBUG

	BD_ASSERT(m_numIterators == 0, "bdSet::remove, another iterator is being held while removing "
		"from set");
#endif //BD_DEBUG

	return m_tree.remove(object);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline
bdBool bdSet<T, LESS_THAN_FUNCTOR>::find(const T& object) const
{
	return m_tree.find(object);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
typename bdSet<T, LESS_THAN_FUNCTOR>::Iterator bdSet<T, LESS_THAN_FUNCTOR>::getIterator() const
{
	Iterator result ;
	m_tree.initializeIterator(result);
#ifdef BD_DEBUG
	if (result != BD_NULL)
	{
		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)++;
		//static_cast<bdUInt>(m_numIterators)++;
	}
#endif // BD_DEBUG
	return result;
}

template <typename T, typename LESS_THAN_FUNCTOR> inline  
void bdSet<T, LESS_THAN_FUNCTOR>::releaseIterator(Iterator iterator) const
{
#ifdef BD_DEBUG
	if (iterator != BD_NULL)
	{
		BD_ASSERT(m_numIterators != 0,"bdHashMap::realeaseIterator"
			"Iterator count reached 0, can't release iterator");

		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)--;
		//static_cast<bdUInt>(m_numIterators)--;
	}
#endif // BD_DEBUG
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
const T& bdSet<T, LESS_THAN_FUNCTOR>::getAt(Iterator &iterator) const
{
	return m_tree.getAt(iterator);
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
const T& bdSet<T, LESS_THAN_FUNCTOR>::next(Iterator &iterator) const
{
#ifdef BD_DEBUG
	const T& value= m_tree.next(iterator);
	if (iterator == BD_NULL)
	{
		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)--;
		//static_cast<bdUInt>(m_numIterators)--;
	}
	return value;
#else // BD_DEBUG
	return m_tree.next(iterator);
#endif // BD_DEBUG
}

template <typename T, typename LESS_THAN_FUNCTOR> inline 
void bdSet<T, LESS_THAN_FUNCTOR>::clear()
{
	m_tree.clear();
}

template <typename T, typename LESS_THAN_FUNCTOR> 
void bdSet<T, LESS_THAN_FUNCTOR>::compare(const bdSet<T,LESS_THAN_FUNCTOR> &a, 
										  const bdSet<T,LESS_THAN_FUNCTOR> &b, 
										  bdSet<T,LESS_THAN_FUNCTOR> &diffA, 
										  bdSet<T,LESS_THAN_FUNCTOR> &diffB)
{
	// Declare an iterator for set a and set b
	Iterator iteratorA;
	Iterator iteratorB;
	LESS_THAN_FUNCTOR lessThan;

	iteratorA = a.getIterator();
	iteratorB = b.getIterator();
	T elementA;
	T elementB;

	if(iteratorA)
	{
		elementA = a.getAt(iteratorA);
	}

	if(iteratorB)
	{
		elementB = b.getAt(iteratorB);
	}

	while (iteratorA && iteratorB)
	{
		// If elementA == elementB, there is no difference. Just increment both iterators.
		if(!lessThan(elementA, elementB) && !lessThan(elementB, elementA))
		{
			a.next(iteratorA);
			if(iteratorA)
			{
				elementA = a.getAt(iteratorA);
			}
			b.next(iteratorB);
			if(iteratorB)
			{
				elementB = b.getAt(iteratorB);
			}
		}
		// If elementA < elementB, there is an element in A not in B. Add to diffA
		// and increment iteratorA.
		else if(lessThan(elementA, elementB))
		{
			diffA.insert(elementA);
			a.next(iteratorA);
			if(iteratorA)
			{
				elementA = a.getAt(iteratorA);
			}
		}
		// if elementA > elementB, there is an element in B not in A. Add to diffB
		// and increment iteratorB
		else if(lessThan(elementB, elementA))
		{
			diffB.insert(elementB);
			b.next(iteratorB);
			if(iteratorB)
			{
				elementB = b.getAt(iteratorB);
			}
		}
	}
	// Sets may be of different sizes, so add any remainders of longer set
	// to the corresponding difference set.
	// Add remaining elements of b to diffB
	while(iteratorB)
	{
		diffB.insert(elementB);
		b.next(iteratorB);
		if(iteratorB)
		{
			elementB = b.getAt(iteratorB);
		}
	}

	// Add remaining elements of a to diffA
	while(iteratorA)
	{
		diffA.insert(elementA);
		a.next(iteratorA);
		if(iteratorA)
		{
			elementA = a.getAt(iteratorA);
		}		
	}

}
