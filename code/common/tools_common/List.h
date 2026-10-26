// Linked list class


//List items can have flags for stuff which can't be stored on the 
//object itself. 


/** \brief Enum flag bit locations in the listItem m_flags bitfield.

Some data needs to be stored in the list itself, as it isn't shared between 
references. For example, although several polygons can link to the same edge, 
they need to know which way round the vertices are with respect to the winding 
order of the polygon. The LIST_ITEM_EDGE_REVERSED flag uses bit 2 to register this.

*/

template <typename T> class ListItem;
template <typename T> class List;

/**\brief Templated List item class, with reference to data

This listitem class has back and forward pointers to other list items, and a reference 
to the templated data type. This must be set on construction. A flags bitfield allows 
flags to be stored within the list, as the data referenced may appear in many lists. 

The Get accessors should be used as the pointers themselves are private, and are manipulated
by the list class only

*/

//TODO: add pointer to list to which item belongs. For safety when adding/removing.


template <typename T>
class ListItem 
{
public:

	/** Constructor must be called with a pointer to some data to reference
	*/

	ListItem(const T& data) : m_pNext(0), m_pPrev(0), m_data(data)
	{
	}

	/** Destructor does NOT delete referenced data
	*/
	~ListItem()
	{
	}

	/**Read only accessors, everything else should be done via List class
	*/
	inline ListItem<T>* GetNext() const
	{
		return m_pNext;
	}

	/**Read only accessors, everything else should be done via List class
	*/
	inline ListItem<T>* GetPrev() const
	{
		return m_pPrev;
	}

	inline T& GetData()
	{
		return m_data;
	}

	inline const T& GetData() const
	{
		return m_data;
	}

private:
	friend class List<T>;

	ListItem* m_pNext;
	ListItem* m_pPrev;

	T m_data;
};

/**\brief Doubly Linked List class

Only needs to be lightweight and simple, may need allocator at some point. Items can be added
before/after another object, or just to the end of the list. Also, can be added either as a listItem 
or just a bare object, in which case a listItem is created and returned. 

The head of the list is reached via the Begin member, and the size of the list is maintained. 

*/

template <typename T>
class List
{
	ListItem<T>* m_pHead;
	ListItem<T>* m_pTail;
	
	U32 m_numItems;

public:

	List()
	{
		m_pTail = 0;
		m_pHead = 0;
		m_numItems = 0;
	}

	List(const List& rhs)
	{
		m_pTail = 0;
		m_pHead = 0;
		m_numItems = 0;
		*this = rhs;
	}

	void operator = (const List& rhs)
	{
		Clear();
		ListItem<T>* pItem = rhs.Begin();
		while (pItem)
		{
			Add(pItem->GetData());
			pItem = pItem->GetNext();
		}
	}

	/**Destructor deletes all listItems in list, but NOT their referenced data
	*/

	~List()
	{
		Clear();
	}

	void Clear()
	{
		ListItem<T>* ptr = m_pHead;
		while (ptr)
		{
			ListItem<T>* tmp = ptr->m_pNext;
			delete ptr;			
			ptr = tmp;
		}
	}

	/**Add bare data, returning a pointer to the created listItem
	*/

	inline ListItem<T>* Add(const T& data)
	{
		ListItem<T>* newItem = new ListItem<T>(data);
		newItem->m_pPrev = m_pTail;
		if (m_pTail)
		{
			m_pTail->m_pNext = newItem;
		}
		else
		{
			m_pHead = newItem;
		}
		m_pTail = newItem;
		m_numItems++;
		return newItem;
	}

	inline void AddAfter(const T& data, ListItem<T>* before)
	{
		ListItem<T>* newListItem = new ListItem<T>(data);

		newListItem->m_pNext = before->m_pNext;
		newListItem->m_pPrev = before;
		before->m_pNext = newListItem;
		if (newListItem->m_pNext)
		{
			newListItem->m_pNext->m_pPrev = newListItem;	
		}
		else
		{
			m_pTail = newListItem;
		}
		m_numItems++;
	}

	inline void AddBefore(const T& data, ListItem<T>* after)
	{
		ListItem<T>* newListItem = new ListItem<T>(data);

		newListItem->m_pNext = after;
		newListItem->m_pPrev = after->m_pPrev;
		if (newListItem->m_pPrev)
		{
			newListItem->m_pPrev->m_pNext = newListItem;
		}
		else
		{
			m_pHead = newListItem;
		}
		after->m_pPrev = newListItem;		
		m_numItems++;	
	}

	/**Add listItem after another listItem
	*/

	inline void AddAfter(ListItem<T>* item, ListItem<T>* before)
	{
		item->m_pNext = before->m_pNext;
		item->m_pPrev = before;
		before->m_pNext = item;
		if (item->m_pNext)
		{
			item->m_pNext->m_pPrev = item;	
		}
		else
		{
			m_pTail = item;
		}
		m_numItems++;
	}

	
	/**Add listItem before another listItem
	*/

	inline void AddBefore(ListItem<T>* item, ListItem<T>* after)
	{
		item->m_pNext = after;
		item->m_pPrev = after->m_pPrev;
		if (item->m_pPrev)
		{
			item->m_pPrev->m_pNext = item;
		}
		else
		{
			m_pHead = item;
		}
		after->m_pPrev = item;		
		m_numItems++;	
	}


	/**Remove the listItem
	*/

	inline void Remove(ListItem<T>* item)
	{
		if ((item != m_pHead) && (item != m_pTail))
		{
			item->m_pPrev->m_pNext = item->m_pNext;
			item->m_pNext->m_pPrev = item->m_pPrev;
		}
		if (item == m_pHead)
		{
			m_pHead = item->m_pNext;
			if (m_pHead)
			{
				m_pHead->m_pPrev = 0;
			}
		}
		if (item == m_pTail)
		{
			m_pTail = item->m_pPrev;
			if (m_pTail)
			{
				m_pTail->m_pNext = 0;
			}
		}

		m_numItems--;
		delete item;
	}

	/**Get a pointer to the head of the list
	*/

	inline ListItem<T>* Begin() const
	{
		return m_pHead;
	}

	inline ListItem<T>* End() const
	{
		return m_pTail;
	}

	/**Return the size of the list
	*/

	inline const U32 Size() const
	{
		return m_numItems;
	}

};
