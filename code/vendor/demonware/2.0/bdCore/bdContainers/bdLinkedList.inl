// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS



template <typename T> inline 
bdLinkedList<T>::bdLinkedList()
: m_head(BD_NULL),
  m_tail(BD_NULL),
  m_size(0)
{
}

template <typename T> inline 
bdLinkedList<T>::~bdLinkedList()
{
	clear();
}

template <typename T> inline 
bdBool bdLinkedList<T>::isEmpty() const
{
	return m_size == 0;
}

template <typename T> inline 
bdUInt bdLinkedList<T>::getSize() const
{
	return m_size;
}

template <typename T> inline 
void bdLinkedList<T>::clear()
{
	while (m_head != BD_NULL)
	{		
		Node *const next = m_head->m_next;
		delete m_head;
		m_head = next;
	}

	m_tail = BD_NULL;
	m_size = 0;
}

template <typename T> inline 
T& bdLinkedList<T>::getHead()
{
	BD_ASSERT(m_head != BD_NULL, "bdLinkedList::GetHead, "
								 "list is empty so has no head.");
	return m_head->m_data;
}

template <typename T> inline 
const T& bdLinkedList<T>::getHead() const
{
	BD_ASSERT(m_head != BD_NULL, "bdLinkedList::GetHead, "
								 "list is empty so has no head.");
	return m_head->m_data;
}

template <typename T> inline 
T& bdLinkedList<T>::getTail()
{
	BD_ASSERT(m_tail != BD_NULL, "bdLinkedList::GetHead, "
								 "list is empty so has no tail.");
	return m_tail->m_data;
}

template <typename T> inline 
const T& bdLinkedList<T>::getTail() const
{
	BD_ASSERT(m_tail != BD_NULL, "bdLinkedList::GetHead, "
								 "list is empty so has no tail.");
	return m_tail->m_data;
}

template <typename T> inline 
void bdLinkedList<T>::addHead(const T &value)
{
	const Position position = getHeadPosition();
	insertBefore(position, value);
}

template <typename T> inline 
void bdLinkedList<T>::addTail(const T &value)
{
	const Position position = getTailPosition();
	insertAfter(position, value);
}

template <typename T> inline 
void bdLinkedList<T>::removeHead()
{
	Position headPosition = getHeadPosition();
	removeAt(headPosition);
}

template <typename T> inline 
void bdLinkedList<T>::removeTail()
{
	Position tailPosition = getTailPosition();
	removeAt(tailPosition);
}

template <typename T> inline
typename bdLinkedList<T>::Position bdLinkedList<T>::getHeadPosition() const
{
	return reinterpret_cast<Position>(m_head);
}

template <typename T> inline
typename bdLinkedList<T>::Position bdLinkedList<T>::getTailPosition() const
{
	return reinterpret_cast<Position>(m_tail);
}

template <typename T> inline
const T& bdLinkedList<T>::forward(Position &position) const
{
	const Node *const node = reinterpret_cast<Node*>(position);
	position = reinterpret_cast<Position>(node->m_next);
	return node->m_data;	
}

template <typename T> inline
T& bdLinkedList<T>::forward(Position &position)
{
	Node *const node = reinterpret_cast<Node*>(position);
	position = reinterpret_cast<Position>(node->m_next);
	return node->m_data;		
}

template <typename T> inline
const T& bdLinkedList<T>::backward(Position &position) const
{
	const Node *const node = reinterpret_cast<Node*>(position);
	position = reinterpret_cast<Position>(node->m_prev);
	return node->m_data;	
}

template <typename T> inline
T& bdLinkedList<T>::backward(Position &position)
{
	Node *const node = reinterpret_cast<Node*>(position);
	position = reinterpret_cast<Position>(node->m_prev);
	return node->m_data;		
}


template <typename T> inline
typename bdLinkedList<T>::Position bdLinkedList<T>::find(const T &value,
												const Position beginAfter) const
{
	Node *node;
	if(beginAfter == BD_NULL)
	{
		node = m_head;
	}
	else
	{
		node = (reinterpret_cast<Node*>(beginAfter))->m_next;
	}

	Position result = BD_NULL;

	while (node != BD_NULL)
	{
		if (node->m_data == value)
		{
			result = reinterpret_cast<Position>(node);
			break;
		}
		node = node->m_next;
	}

	return result;
}

template <typename T> inline
T& bdLinkedList<T>::getAt(const Position position)
{
	Node *const node = reinterpret_cast<Node*>(position);
	return node->m_data;
}

template <typename T> inline
const T& bdLinkedList<T>::getAt(const Position position) const
{
	const Node *const node = reinterpret_cast<Node*>(position);
	return node->m_data;
}

template <typename T> inline
void bdLinkedList<T>::setAt(const Position position,
							const T &value)
{
	Node *const node = reinterpret_cast<Node*>(position);
	node->m_data = value;
}

template <typename T> inline
void bdLinkedList<T>::removeAt(Position &position)
{
	Node *const node = reinterpret_cast<Node*>(position);
	
	if(node != BD_NULL)
	{
		forward(position);
		if(node == m_head)
		{
			m_head = m_head->m_next;
		}
		else
		{
			node->m_prev->m_next = node->m_next;
		}

		if(node == m_tail)
		{
			m_tail = node->m_prev;
		}
		else
		{
			node->m_next->m_prev = node->m_prev;
		}

		delete node;
		m_size--;
	}
}

template <typename T> inline
void bdLinkedList<T>::insertBefore(const Position position,
								   const T& value)
{
	Node *const node = reinterpret_cast<Node*>(position);

	Node *const newNode = new Node(value);
	
	if(node)
	{
		newNode->m_next = node;
		newNode->m_prev = node->m_prev;
		
		if(node->m_prev != BD_NULL)
		{
			node->m_prev->m_next = newNode;
		}
		else
		{
			BD_ASSERT (node == m_head, "bdLinkedList::insertBefore, node has no previous "
									   "entry, but is not the head.");
			m_head = newNode;
		}

		node->m_prev = newNode;

	}
	else
	{
		// insert at the head
		newNode->m_next = m_head;
		newNode->m_prev = BD_NULL;

		if (m_head != BD_NULL)
		{
			m_head->m_prev = newNode;
		}
		else
		{
			m_tail = newNode;
		}

		m_head = newNode;
	}

	m_size++;
}

template <typename T> inline
void bdLinkedList<T>::insertBefore(const Position position,
								   bdLinkedList<T>& list)
{

	const bdUInt listSize = list.getSize();
	if(listSize == 0)
	{
		return;
	}

	Node *const node = reinterpret_cast<Node*>(position);

	if(node)
	{
		list.m_tail->m_next = node;
		list.m_head->m_prev = node->m_prev;
		
		if(node->m_prev != BD_NULL)
		{
			node->m_prev->m_next = list.m_head;
		}
		else
		{
			BD_ASSERT (node == m_head, "bdLinkedList::insertBefore, node has no previous "
									   "entry, but is not the head.");
			m_head = list.m_head;
		}

		node->m_prev = list.m_tail;

	}
	else
	{
		// insert at the head
		list.m_tail->m_next = m_head;
		
		if (m_head != BD_NULL)
		{
			m_head->m_prev = list.m_tail;
		}
		else
		{
			m_tail = list.m_tail;
		}

		m_head = list.m_head;
	}

	m_size += listSize;

	list.m_head = BD_NULL;
	list.m_tail = BD_NULL;
	list.m_size = 0;
}


template <typename T> inline
void bdLinkedList<T>::insertAfter(const Position position,
								  const T& value)
{
	Node *const node = reinterpret_cast<Node*>(position);

	Node *const newNode = new Node(value);

	if(node)
	{
		newNode->m_next = node->m_next;
		newNode->m_prev = node;
		
		if(node->m_next != BD_NULL)
		{
			node->m_next->m_prev = newNode;
		}
		else
		{
			BD_ASSERT (node == m_tail, "bdLinkedList::insertAfter, node has no next "
									   "entry, but is not the tail.");
			m_tail = newNode;
		}

		node->m_next = newNode;
	}
	else
	{
		// insert at the tail
		newNode->m_next = BD_NULL;
		newNode->m_prev = m_tail;

		if (m_tail != BD_NULL)
		{
			m_tail->m_next = newNode;
		}
		else
		{
			m_head = newNode;
		}

		m_tail = newNode;
	}

	m_size++;
}

template <typename T> inline
void bdLinkedList<T>::insertAfter(const Position position,
								  bdLinkedList<T>& list)
{
	const bdUInt listSize = list.getSize();
	if(listSize == 0)
	{
		return;
	}

	Node *const node = reinterpret_cast<Node*>(position);

	if(node)
	{
		list.m_tail->m_next = node->m_next;
		list.m_head->m_prev = node;
		
		if(node->m_next != BD_NULL)
		{
			node->m_next->m_prev = list.m_tail;
		}
		else
		{
			BD_ASSERT (node == m_tail, "bdLinkedList::insertAfter, node has no next "
									   "entry, but is not the tail.");
			m_tail = list.m_tail;
		}

		node->m_next = list.m_head;

	}
	else
	{
		// insert at the tail
		list.m_head->m_prev = m_tail;

		if (m_tail != BD_NULL)
		{
			m_tail->m_next = list.m_head;
		}
		else
		{
			m_head = list.m_head;
		}

		m_tail = list.m_tail;
	}

	m_size += listSize;

	list.m_head = BD_NULL;
	list.m_tail = BD_NULL;
	list.m_size = 0;
}

template <typename T> inline
void bdLinkedList<T>::moveToHead(const Position position)
{
	BD_ASSERT (position, "bdLinkedList<T>::moveToHead, can't move null to head");
	Node *node = reinterpret_cast<Node*>(position);

	if (node == m_head)
	{
		return;
	}
    Node *before = node->m_prev;
	Node *after = node->m_next;
	if (before)
	{
		before->m_next = after;
	}
	else
	{
		BD_ASSERT (false, "bdLinkedList<T>::moveToHead, only the head can have m_prev == BD_NULL");
	}
	if (after)
	{
		after->m_prev = before;
	}
	else
	{
		BD_ASSERT (m_tail == node, "bdLinkedList<T>::moveToHead, only the tail can have m_next == BD_NULL");
	}
	node->m_prev = BD_NULL;
	node->m_next = m_head;
	m_head->m_prev = node;
	m_head = node;
	
	// Find the new tail of the linked list
	if (m_tail == node)
	{
		m_tail = before;
	}
}

template <typename T> inline
void bdLinkedList<T>::moveToTail(const Position position)
{
	BD_ASSERT (position, "bdLinkedList<T>::moveToTail, can't move null to tail");
	Node *node = reinterpret_cast<Node*>(position);

	if (node == m_tail)
	{
		return;
	}
    Node *before = node->m_prev;
	Node *after = node->m_next;
	if (before)
	{
		before->m_next = after;
	}
	else
	{
		BD_ASSERT (node == m_head, "bdLinkedList<T>::moveToTail, only the head can have m_prev == BD_NULL");
	}
	if (after)
	{
		after->m_prev = before;
	}
	else
	{
		BD_ASSERT (false, "bdLinkedList<T>::moveToTail, only the tail can have m_next == BD_NULL");
	}
	node->m_prev = m_tail;
	node->m_next = BD_NULL;
	m_tail->m_next = node;
	m_tail = node;
	// Find the new head of the linked list
	if (m_head == node)
	{
		m_head = after;
	}
}


