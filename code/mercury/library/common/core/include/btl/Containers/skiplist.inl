template <typename Key, typename Data, typename Allocator>
inline SkipList<Key, Data, Allocator>::SkipList()
{
	m_head.Extend(&m_headLinks);
	m_maxLevel = -1;
}

template <typename Key, typename Data, typename Allocator>
inline SkipList<Key, Data, Allocator>::~SkipList()
{
	Clear();
}

template <typename Key, typename Data, typename Allocator>
inline void SkipList<Key, Data, Allocator>::Clear()
{
	while (m_head.GetLink(0) != 0)
		PopFront();
}

template <typename Key, typename Data, typename Allocator>
inline void SkipList<Key, Data, Allocator>::PopFront()
{
	BTL_ASSERT(m_head.GetLink(0) != 0); // must not be empty list

	Node* pNode = m_head.GetLink(0);
	m_head.SetLink(0, pNode->GetLink(0));
	if (pNode->GetLink(0)) pNode->GetLink(0)->SetPrevious(&m_head);

	m_counter.EraseElement();
	m_linkAllocator.Free(pNode->Extend());
	m_initialiser.Release(pNode);	
}

template <typename Key, typename Data, typename Allocator>
inline typename SkipList<Key, Data, Allocator>::ConstIterator& SkipList<Key, Data, Allocator>::ConstIterator::operator ++()
{
	m_pNode = m_pNode->GetLink(m_level);
	return *this;
}

template <typename Key, typename Data, typename Allocator>
inline typename SkipList<Key, Data, Allocator>::ConstIterator& SkipList<Key, Data, Allocator>::ConstIterator::operator --()
{
	m_pNode = m_pNode->GetPrevious();
	return *this;
}

template <typename Key, typename Data, typename Allocator>
bool SkipList<Key, Data, Allocator>::Empty() const
{
	return m_head.GetLink(0) == 0;
}

template <typename Key, typename Data, typename Allocator>
int SkipList<Key, Data, Allocator>::Size() const
{
	return m_counter.Count(*this);
}

template <typename Key, typename Data, typename Allocator>
inline void SkipList<Key, Data, Allocator>::Destroy()
{
	SkipList<Key, Data, Allocator> other;
	Swap(other);
}

template <typename Key, typename Data, typename Allocator>
void SkipList<Key, Data, Allocator>::Swap(SkipList& other)
{
	Btl::Swap(m_initialiser, other.m_initialiser);
	Btl::Swap(m_counter, other.m_counter);
	Btl::Swap(m_head, other.m_head);
	Btl::Swap(m_headLinks, other.m_headLinks);
	Btl::Swap(m_linkAllocator, other.m_linkAllocator);
	Btl::Swap(m_maxLevel, other.m_maxLevel);

	m_head.Extend(&m_headLinks);
	other.m_head.Extend(&other.m_headLinks);

	{
		Node* pNode = m_head.GetLink(0);
		if (pNode) pNode->SetPrevious(&m_head);
	}

	{
		Node* pNode = other.m_head.GetLink(0);
		if (pNode) pNode->SetPrevious(&other.m_head);
	}
}

///////////////////////////////////////////////////////////

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
typename SkipListBuilder<Key, Data, Allocator, ElementPolicy>::InsertionReturnType SkipListBuilder<Key, Data, Allocator, ElementPolicy>::Insert(const Key& item, S32 level)
{
	// Get Level of the node.
	S32 newLevel = Min(level, Base::MAX_LINKS - 1);
	S32 maxLevel = Min(Max(level, m_maxLevel + 1), Base::MAX_LINKS - 1);
	while(((m_random.GetRandom() % 3) == 1) && newLevel < maxLevel) newLevel++;
	m_maxLevel = Max(m_maxLevel, newLevel);

	static S32 count[Base::MAX_LINKS] = {0};
	count[newLevel]++;

	Node* previous = &m_head;
	Node* pNewNode = 0;
	Node* links[Base::MAX_LINKS];

	for (int level = m_maxLevel; level > -1; level--)
	{
		Node* next = previous->GetLink(level);

		while (next && next->m_element.GetKey() < item)
		{
			previous = previous->GetLink(level);
			next = previous->GetLink(level);
		}

		if (next == 0 || ElementPolicy<Iterator>::MULTIPLE || item < next->m_element.GetKey() )
		{
			links[level] = previous;
		}
		else // found match
		{
			return ElementPolicy<Iterator>::MakeInsertionReturnType(Iterator(next, this, 0), false);
		}
	}

	pNewNode = &m_initialiser.Initialise(item);

	if (newLevel > 2)
		pNewNode->Extend(m_linkAllocator.Allocate());

	m_counter.AddElement();

	for (int level = 0; level <= newLevel; level++)
	{
		Node* previous = links[level];
		Node* next = previous->GetLink(level);

		pNewNode->SetLink(level, next);
		previous->SetLink(level, pNewNode);

		if (level == 0)
		{
			if (next) next->SetPrevious(pNewNode);
			pNewNode->SetPrevious(previous);
		}
	}

	return ElementPolicy<Iterator>::MakeInsertionReturnType(Iterator(pNewNode, this, 0), true);
}

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
int SkipListBuilder<Key, Data, Allocator, ElementPolicy>::Erase(const Key& key)
{
	if (ElementPolicy<Iterator>::MULTIPLE)
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

	Node* previous = &m_head;
	Node* links[Base::MAX_LINKS] = {};
	Node* nodeToDelete = 0;

	for (int level = m_maxLevel; level > -1; level--)
	{
		Node* next = previous->GetLink(level);

		while (next && next->m_element.GetKey() < key)
		{
			previous = previous->GetLink(level);
			next = previous->GetLink(level);
		}

		if (next && !(key < next->m_element.GetKey()))
		{
			links[level] = previous;
			nodeToDelete = next;
		}
	}

	if (nodeToDelete)
	{
		// Fix up the links.
		for (int level = 0; level <= m_maxLevel; level++)
		{
			if (links[level])
			{
				Node* previous = links[level];
				Node* next = nodeToDelete->GetLink(level);

				assert(previous->GetLink(level) == nodeToDelete);
				assert(next != nodeToDelete);

				previous->SetLink(level, next);
				if (level == 0 && next) next->SetPrevious(previous);
			}
		}

		m_counter.EraseElement();
		m_linkAllocator.Free(nodeToDelete->Extend());
		m_initialiser.Release(nodeToDelete);

		return 1;
	}
	else
	{
		return 0;
	}
}

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
void SkipListBuilder<Key, Data, Allocator, ElementPolicy>::Erase(Iterator i)
{
	assert(i.m_pNode);

	// Can't erase directly as no backward links.
	Node* previous = &m_head;
	Node* links[Base::MAX_LINKS] = {};
	Node* nodeToDelete = 0;

	for (int level = m_maxLevel; level > -1; level--)
	{
		Node* next = previous->GetLink(level);

		while (next && next->m_element.GetKey() < i->GetKey())
		{
			previous = previous->GetLink(level);
			next = previous->GetLink(level);
		}

		if (next == i.m_pNode)
		{
			links[level] = previous;
			nodeToDelete = next;
		}
	}

	assert(nodeToDelete);

	// Fix up the links.
	for (int level = 0; level <= m_maxLevel; level++)
	{
		if (links[level])
		{
			Node* next = nodeToDelete->GetLink(level);

			assert(links[level]->GetLink(level) == nodeToDelete);
			assert(next != nodeToDelete);

			links[level]->SetLink(level, next);
		}
	}

	m_counter.EraseElement();
	m_linkAllocator.Free(nodeToDelete->Extend());
	m_initialiser.Release(nodeToDelete);
}

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
typename SkipList<Key, Data, Allocator>::Iterator SkipListBuilder<Key, Data, Allocator, ElementPolicy>::Find(const Key& key)
{
	Node* previous = &m_head;

	for (int level = m_maxLevel; level > -1; level--)
	{
		Node* next = previous->GetLink(level);

		while (next && next->m_element.GetKey() < key)
		{
			previous = previous->GetLink(level);
			next = previous->GetLink(level);
		}

		if (next && !(key < next->m_element.GetKey()))
		{
			return Iterator(next, this, 0);
		}
	}

	return Iterator(0, this, 0);
}

template <typename Key, typename Data, typename Allocator, template <typename Iterator> class ElementPolicy>
typename SkipList<Key, Data, Allocator>::ConstIterator SkipListBuilder<Key, Data, Allocator, ElementPolicy>::Find(const Key& key) const
{
	return const_cast<SkipList<Key, Data, Allocator> >(this)->Find(key);
}

template <typename Tree>
void CopySkipListElements(Tree& tree1, const Tree& tree2)
{
	typename Tree::ConstIterator end = tree2.End();
	for (typename Tree::ConstIterator it = tree2.Begin(); it != end; ++it)
		tree1.Insert(*it);
}