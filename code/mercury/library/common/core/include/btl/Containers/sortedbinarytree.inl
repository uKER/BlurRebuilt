/////////////////////////////////////////////////////////
// Sorted Binary Tree
/////////////////////////////////////////////////////////

//#define TREE_BUILDER_DEBUG

/// @cond 

template <typename Key, typename Data, typename Allocator>
inline SortedBinaryTree<Key, Data, Allocator>::SortedBinaryTree()
{
	Initialise();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::Initialise()
{
	// set up the sentinel
	m_sentinel.pFirst = SentinelPointer();
	m_sentinel.pSecond = SentinelPointer();
	m_sentinel.pParent = SentinelPointer();
	m_sentinel.level = 0;

	// root points to the sentinel
	m_pRoot = SentinelPointer();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::Free(Node* pNode)
{
	if (pNode != SentinelPointer())
	{
		Free(pNode->pFirst);
		Free(pNode->pSecond);
		m_initialiser.Release(pNode);
	}
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::BalanceInsertedItem(Node** pp)
{
	// walk back up the tree and rebalance
	int count = 0;

	while (*pp != m_pRoot)
	{
		if ((*pp)->pParent == m_pRoot)
			pp = &m_pRoot;
		else if ((*pp)->pParent->pParent->pFirst == (*pp)->pParent)
			pp = &((*pp)->pParent->pParent->pFirst);
		else
			pp = &((*pp)->pParent->pParent->pSecond);

		int change = Skew(*pp);
		change |= Split(*pp);

		if (change == 0) count++;
		else count = 0;

		if (count == 2) 
			return;
	}
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::EraseNode(Node** pp)
{
	Node* p = (*pp);
	Node* pParent = (*pp)->pParent;

	if ((*pp)->pFirst == SentinelPointer())
	{
		(*pp) = (*pp)->pSecond;
		(*pp)->pParent = pParent;
	}
	else if ((*pp)->pSecond == SentinelPointer())
	{
		(*pp) = (*pp)->pFirst;
		(*pp)->pParent = pParent;
	}
	else
		pp = SwapWithLastItem((*pp)->pFirst, (*pp));

	BalanceDeletedItem(pp);
	m_initialiser.Release(p);
	m_counter.EraseElement();
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::BalanceDeletedItem(Node** pp)
{
	// walk back up the tree and rebalance
	int count = 0;

	while (*pp != m_pRoot)
	{
		if ((*pp)->pParent == m_pRoot)
			pp = &m_pRoot;
		else if ((*pp)->pParent->pParent->pFirst == (*pp)->pParent)
			pp = &((*pp)->pParent->pParent->pFirst);
		else
			pp = &((*pp)->pParent->pParent->pSecond);

		int change = 0;
			
		if ((*pp)->pFirst->level < (*pp)->level - 1
			|| (*pp)->pSecond->level < (*pp)->level - 1)
		{
			(*pp)->level--;

			if ((*pp)->pSecond->level > (*pp)->level)
				(*pp)->pSecond->level = (*pp)->level;

			Skew((*pp));
			Skew((*pp)->pSecond);
			Skew((*pp)->pSecond->pSecond);
			Split((*pp));
			if ((*pp)->pSecond != SentinelPointer()) Split((*pp)->pSecond);

			change = 1;
		}

		if (change == 0) count++;
		else count = 0;

		if (count == 2) 
			return;
	}
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::Node** 
	SortedBinaryTree<Key, Data, Allocator>::SwapWithLastItem(Node*& pNode, Node*& pNodeToSwap)
{
	Node** pp = &pNode;

	while ((*pp)->pSecond != SentinelPointer())
		pp = &((*pp)->pSecond);

	Node* pc = *pp;

	BTL_ASSERT(pc != SentinelPointer());

	pc->pFirst->pParent = pc->pParent;
	*pp = pc->pFirst;
	pc->pFirst = pNodeToSwap->pFirst;
	pc->pSecond = pNodeToSwap->pSecond;
	pc->pParent = pNodeToSwap->pParent;
	pc->level = pNodeToSwap->level;
	pNodeToSwap->pFirst->pParent = pc;
	pNodeToSwap->pSecond->pParent = pc;
	pNodeToSwap = pc;

	return pp;
}

template <typename Key, typename Data, typename Allocator>
FORCE_INLINE int SortedBinaryTree<Key, Data, Allocator>::Skew(Node*& pNode)
{
	if (pNode->level == pNode->pFirst->level)
	{
		RotateFirst(pNode);
		return 1;
	}

	return 0;
}

template <typename Key, typename Data, typename Allocator>
FORCE_INLINE int SortedBinaryTree<Key, Data, Allocator>::Split(Node*& pNode)
{
	if (pNode->level == pNode->pSecond->pSecond->level)
	{
		BTL_ASSERT(pNode->pSecond != SentinelPointer());
		pNode->pSecond->level++;
		RotateSecond(pNode);
		return 1;
	}

	return 0;
}

template <typename Key, typename Data, typename Allocator>
FORCE_INLINE void SortedBinaryTree<Key, Data, Allocator>::RotateFirst(Node*& pNode)
{
	Node* p = pNode->pFirst;
	pNode->pFirst = p->pSecond;
	p->pSecond->pParent = pNode;
	p->pSecond = pNode;
	p->pParent = pNode->pParent;
	pNode->pParent = p;
	pNode = p;
}

template <typename Key, typename Data, typename Allocator>
FORCE_INLINE void SortedBinaryTree<Key, Data, Allocator>::RotateSecond(Node*& pNode)
{	
	Node* p = pNode->pSecond;
	pNode->pSecond = p->pFirst;
	p->pFirst->pParent = pNode;
	p->pFirst = pNode;
	p->pParent = pNode->pParent;
	pNode->pParent = p;
	pNode = p;
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::Iterator SortedBinaryTree<Key, Data, Allocator>::Begin()
{
	Node* pNode = m_pRoot;

	while (pNode->pFirst != SentinelPointer())
		pNode = pNode->pFirst;

	return Iterator(pNode, this);
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::ReverseIterator SortedBinaryTree<Key, Data, Allocator>::ReverseBegin()
{
	Node* pNode = m_pRoot;

	while (pNode->pSecond != SentinelPointer())
		pNode = pNode->pSecond;

	return ReverseIterator(pNode, this);
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::FixSentinelPointers(Node** pp, Node* pOldSentinel)
{
	if ((*pp) == pOldSentinel)
	{
		*pp = SentinelPointer();
	}
	else
	{
		// would be tricky to do this non-recursively as the sentinel pointers are still wrong
		FixSentinelPointers(&(*pp)->pFirst, pOldSentinel);
		FixSentinelPointers(&(*pp)->pSecond, pOldSentinel);
	}
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::Swap(SortedBinaryTree& other)
{
	Swap(m_pRoot, other.m_pRoot);
	Swap(m_counter, other.m_counter);
	// no need to swap the sentinel
	Swap(m_initialiser, other.m_initialiser);

	m_pRoot->pParent = SentinelPointer();
	other.m_pRoot->pParent = other.SentinelPointer();

	// traverse both trees and fix up the sentinel pointers
	FixSentinelPointers(&m_pRoot, other.SentinelPointer());
	other.FixSentinelPointers(&other.m_pRoot, SentinelPointer());
}

template <typename Key, typename Data, typename Allocator>
void SortedBinaryTree<Key, Data, Allocator>::Destroy()
{
	Clear();
	Initialiser empty;
	Swap(m_initialiser, empty);
}

///////////////////////////////////////////////////////////////////////
// debug
template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::PrintDebug() const
{
	PrintDebug(m_pRoot, 0);
}

template <typename Key, typename Data, typename Allocator>
inline void SortedBinaryTree<Key, Data, Allocator>::PrintDebug(const Node* pNode, int level) const
{
	//for (int i = 0; i < level; i++) 
	//	DBPrint("\t");

	if (pNode != SentinelPointer())
	{
		//DBPrint("%i \n", pNode->level);
		PrintDebug(pNode->pFirst, level + 1);
		PrintDebug(pNode->pSecond, level + 1);
		BTL_ASSERT(pNode->pFirst == SentinelPointer() || pNode->pFirst->pParent == pNode);
		BTL_ASSERT(pNode->pSecond == SentinelPointer() || pNode->pSecond->pParent == pNode);
		BTL_ASSERT(pNode->pFirst->level >= pNode->level - 1);
		BTL_ASSERT(pNode->pSecond->level >= pNode->level - 1);
	}
	//else
	//	DBPrint("(%i) \n", pNode->level);
}

//////////////////////////////////////////////////////////////////
// tree builder

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::Validate() const
{
#ifdef TREE_BUILDER_DEBUG
	Validate(m_pRoot, SentinelPointer());
#endif
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::Validate(const Node* pNode, const Node* pParent) const
{
	assert(pNode);

	if (pNode == SentinelPointer())
	{
		assert(pNode->pFirst == pNode);
		assert(pNode->pSecond == pNode);
		assert(pNode->level == 0);
		return;
	}
	else
	{
		assert(pNode->pParent == pParent);
		assert(pNode->pParent == SentinelPointer() || pNode->level <= pNode->pParent->level);
	}

	Validate(pNode->pFirst, pNode);
	Validate(pNode->pSecond, pNode);
}

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(const KeyType& item)
{
	return Insert(&m_pRoot, SentinelPointer(), item);
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(typename SortedBinaryTree<Key, Data, Allocator>::Iterator hint, 
										const KeyType& item)
{
	Validate();

	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	Node* pNode = GetNode(hint);

	// if the item is before the hint, do a normal insertion
	if (pNode == SentinelPointer() || item < *hint)
		return Insert(&m_pRoot, SentinelPointer(), item);
	
	// if the item immediately follows the hint, it can be inserted without starting from the root
	if (++hint == End() || !(*hint < item))
	{
		if (item < pNode->m_element)
			return Insert(&pNode->pFirst, pNode, item);
		else if (ElementPolicy<Iterator>::MULTIPLE || pNode->m_element < item)
			return Insert(&pNode->pSecond, pNode, item);
		else
		{
			Validate();
			return ElementPolicy<Iterator>::MakeInsertionReturnType(Iterator(pNode, this), false);
		}
	}
	else
		return Insert(&m_pRoot, SentinelPointer(), item);
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType
	TreeBuilder<Key, Data, Allocator, ElementPolicy>::Insert(typename SortedBinaryTree<Key, Data, Allocator>::Node** pp,
										typename SortedBinaryTree<Key, Data, Allocator>::Node* pParent,
										const KeyType& item)
{
	Validate();

	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	while (true)
	{
		if ((*pp) == SentinelPointer())
		{
			*pp = &m_initialiser.Initialise(item);
			(*pp)->pFirst = SentinelPointer();
			(*pp)->pSecond = SentinelPointer();
			(*pp)->pParent = pParent;
			(*pp)->level = 1;

			Node* pInserted = *pp;

			BalanceInsertedItem(pp);
			
			m_counter.AddElement();

			Validate();
			return ElementPolicy<Iterator>::MakeInsertionReturnType(Iterator(pInserted, this), true);
		}
		else
		{
			pParent = *pp;

			if (item < (*pp)->m_element)
				pp = &(*pp)->pFirst;
			else if (ElementPolicy<Iterator>::MULTIPLE || (*pp)->m_element < item)
				pp = &(*pp)->pSecond;
			else
			{
				Validate();
				return ElementPolicy<Iterator>::MakeInsertionReturnType(Iterator(*pp, this), false); // no insertion
			}
		}
	}
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::Erase(typename SortedBinaryTree<Key, Data, Allocator>::Iterator i)
{
	Node** pp;

	// We have an iterator but need the link to the item.
	if (GetNode(i) == m_pRoot)
		pp = &m_pRoot;
	else if (GetNode(i) == SentinelPointer())
		return;
	else if (GetNode(i)->pParent->pFirst == GetNode(i))
		pp = &(GetNode(i)->pParent->pFirst);
	else
		pp = &(GetNode(i)->pParent->pSecond);

	EraseNode(pp);
	Validate();
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
void TreeBuilder<Key, Data, Allocator, ElementPolicy>::ReverseErase(typename SortedBinaryTree<Key, Data, Allocator>::ReverseIterator i)
{
	Erase(i.Iterator());
}

template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
inline int TreeBuilder<Key, Data, Allocator, ElementPolicy>::Erase(const Key& key)
{
	typedef SortedBinaryTree<Key, Data, Allocator> Base;
	if (!ElementPolicy<Iterator>::MULTIPLE)
	{
		// There can be only one...
		// Look down the tree and delete if found.
		Node** pp = &m_pRoot;

		while (true)
		{
			if ((*pp) != SentinelPointer())
			{
				if (key < (*pp)->m_element)
					pp = &(*pp)->pFirst;
				else if ((*pp)->m_element < key)
					pp = &(*pp)->pSecond;
				else
				{
					EraseNode(pp);
					Validate();
					return 1;
				}
			}
			else
				return 0;
		}
	}
	else
	{
		// Find the first match, then delete all subsequent items that also match.
		// May not be the quickest way, but at least it's simple.
		Iterator i = Find(key);
		int count = 0;

		Key copy(key); // copy in case we delete the original

		if (i != End()) 
		{
			do
			{
				Iterator previous = i;
				++i;
				count++;
				Erase(previous);

			} while (i != End() && !(copy < *i));
		} 

		return count;
	}
}

/// Returns an iterator pointing to the first element matching the key, or to the end of the tree if no element matches the key.
template <typename Key, typename Data, typename Allocator, template <typename Tree> class ElementPolicy>
typename TreeBuilder<Key, Data, Allocator, ElementPolicy>::Iterator TreeBuilder<Key, Data, Allocator, ElementPolicy>::Find(const Key& key) 
{
	Node* pNode = m_pRoot;

	while (pNode != SentinelPointer())
	{
		if (key < pNode->m_element)
			pNode = pNode->pFirst;
		else if (pNode->m_element < key)
			pNode = pNode->pSecond;
		else
		{
			if (!ElementPolicy<Iterator>::MULTIPLE)
			{
				if (pNode->pFirst != SentinelPointer() && !(pNode->pFirst->m_element < key))
					pNode = pNode->pFirst;
				else
				{
					// to do: there's a quicker way to get the first match than iterating backwards
					Iterator i = Iterator(pNode, this);

					while (true)
					{
						Iterator p = i;
						--i;
						if (i == End() || *i < key)
							return p;
					}
				}
			}
			else
				return Iterator(pNode, this);
		}
	}

	return End();
}

/////////////////////////////////////////////////////////////////
// iterators

template <typename Key, typename Data, typename Allocator>
inline bool SortedBinaryTree<Key, Data, Allocator>::ConstIterator::operator != (const ConstIterator& i) const 
{
	return m_pNode != i.m_pNode;
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::ConstIterator& SortedBinaryTree<Key, Data, Allocator>::ConstIterator::operator ++()
{
	typename SortedBinaryTree::Node* pNode = m_pNode;

	if (pNode->pSecond != m_pTree->SentinelPointer())
	{
		pNode = pNode->pSecond;

		while (pNode->pFirst != m_pTree->SentinelPointer())
		{
			pNode = pNode->pFirst;
		};
	}
	else
	{
		while (pNode != m_pTree->SentinelPointer())
		{
			if (pNode == pNode->pParent->pFirst)
			{
				m_pNode = pNode->pParent;
				return *this;
			} 
			else
			{
				pNode = pNode->pParent;
			}
		};
	}

	m_pNode = pNode;

	return *this;
}

template <typename Key, typename Data, typename Allocator>
inline typename SortedBinaryTree<Key, Data, Allocator>::ConstIterator& SortedBinaryTree<Key, Data, Allocator>::ConstIterator::operator --()
{
	typename SortedBinaryTree::Node* pNode = m_pNode;

	if (pNode->pFirst != m_pTree->SentinelPointer())
	{
		pNode = pNode->pFirst;

		while (pNode->pSecond != m_pTree->SentinelPointer())
		{
			pNode = pNode->pSecond;
		};
	}
	else
	{
		while (pNode != m_pTree->SentinelPointer())
		{
			if (pNode == pNode->pParent->pSecond)
			{
				m_pNode = pNode->pParent;
				return *this;
			} 
			else
			{
				pNode = pNode->pParent;
			}
		};
	}

	m_pNode = pNode;

	return *this;
}

/// @endcond 
