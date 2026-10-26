// a sorted linked list template
#pragma once

// FIXME: have to include the full path or it will load the wrong list.h
#ifdef KEN
#include "F:\Subversion\Common\trunk\Utility\list.h"
#else
#include "Utility\list.h"
#endif

using namespace Ut;

//
// typedef UtList<MyClass, SingleEndedList, MaintainedCounter> MyListType1;
// UtList<MyClass> myList; // default is DoubleEndedList, OnDemandCounter
// UtList<int> myList2; // a non-intrusive list
template< class T >
class CMySortedList
{
public:
	struct element
	{
	public:
		T data;
		int iPriority;
	};

	typedef UtList<element, SingleEndedList, MaintainedCounter> T_List;
	typedef typename UtList<element>::Iterator List_Iterator;

	UtList<element> m_c;

	void clear()			{ m_c.Clear(); }
	bool empty()			{ return m_c.Empty(); }

	// move a newly added node to the right place in the heap
	void push(T pNewNode, int iPriority)
	{
		element newe;
		newe.data = pNewNode;
		newe.iPriority = iPriority;

		List_Iterator i;
		for (i=m_c.Begin(); i!=m_c.End(); ++i)
		{
			if (i->iPriority < iPriority)
			{
				m_c.InsertBefore(newe, i);
				return;
			}
		}
		// add it at the end
		m_c.PushBack(newe);
	}
	void erase(int i)
	{
        m_c.Erase(i);
	}
	void push_back(T pNewNode)
	{
		element newe;
		newe.data = pNewNode;
		newe.iPriority = -1;
		// add it at the end
		m_c.PushBack(newe);
	}

	List_Iterator begin()	{ return m_c.Begin(); }
	List_Iterator end()		{ return m_c.End(); }
	List_Iterator &operator++() {this++;}
	List_Iterator erase(List_Iterator l) { return m_c.Erase(l); }
        
	const List_Iterator* operator->() const { return this->LE_Current(); }

	// pop the top node from the heap
	T PopTopNode()
	{
		element temp;
		temp = m_c.Back();
		m_c.PopBack();
		return temp.data;
	}

};

