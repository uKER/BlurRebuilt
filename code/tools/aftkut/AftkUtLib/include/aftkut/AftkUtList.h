#ifndef __AFTKUT_LIST_H__
#define __AFTKUT_LIST_H__

// Linked list class


//List items can have flags for stuff which can't be stored on the 
//object itself. 

namespace AftkUt
{

	/** \brief Enum flag bit locations in the listItem m_flags bitfield.

	Some data needs to be stored in the list itself, as it isn't shared between 
	references. For example, although several polygons can link to the same edge, 
	they need to know which way round the vertices are with respect to the winding 
	order of the polygon. The LIST_ITEM_EDGE_REVERSED flag uses bit 2 to register this.

	*/

	enum ListItemFlags
	{
		LIST_ITEM_SELECTED = 1,
		LIST_ITEM_EDGE_REVERSED = 2
	};

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
		friend class List<T>;

		ListItem* m_pNext;
		ListItem* m_pPrev;
		
	public:

		/** Constructor must be called with a pointer to some data to reference
		*/

		ListItem(T* data) : m_pNext(0), m_pPrev(0), m_data(*data), m_flags(0)
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

		T& m_data;
		U32 m_flags;
	};

	/**\brief Doubly Linked List class for AftkUtLinkedMesh components

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

		/**Destructor deletes all listItems in list, but NOT their referenced data
		*/

		~List()
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

		inline ListItem<T>* Add(T* data)
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

		/**Add bare object after the listItem which contains the "before" object as its data
		*/

		inline ListItem<T>* AddAfter(T* item, T* before)
		{
			for (ListItem<T>* listItem = Begin(); listItem != 0; listItem = listItem->GetNext())
			{
				if (&listItem->m_data == before)
				{
					ListItem<T>* newItem = new ListItem<T>(item);
						
					AddAfter(newItem, listItem);
					return newItem;
				}
				
			}
			return 0;
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

		/**Add bare object before the listItem which contains the "after" object as its data
		*/

		inline ListItem<T>* AddBefore(T* item, T* after)
		{
			for (ListItem<T>* listItem = Begin(); listItem != 0; listItem = listItem->GetNext())
			{
				if (&listItem->m_data == after)
				{
					ListItem<T>* newItem = new ListItem<T>(item);
						
					AddBefore(newItem, listItem);
					return newItem;
				}
				
			}
			return 0;
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

		/**Remove the listItem containing the "item" as its data reference
		*/

		inline void Remove(T* item)
		{
			ListItem<T>* listItem = Begin();
			while (listItem != 0)
			{	
				ListItem<T>* nextListItem = listItem->GetNext();
				if (&listItem->m_data == item)
				{
					//if we have found a match, remove this list item
					Remove(listItem);
					return;
				}
				listItem = nextListItem;
				
			}

		}

		inline void RemoveAll(T* item)
		{
			ListItem<T>* listItem = Begin();
			while (listItem != 0)
			{	
				ListItem<T>* nextListItem = listItem->GetNext();
				if (&listItem->m_data == item)
				{
					//if we have found a match, remove this list item
					Remove(listItem);
				}
				listItem = nextListItem;
				
			}

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
}

#endif
