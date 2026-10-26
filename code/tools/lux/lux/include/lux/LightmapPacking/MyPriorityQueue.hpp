// my own priority queue template
// need a Priority callback function 'int Priority()' which gets compared

#pragma once

template< class T >
class CMyPriorityQueue
{
public:
	struct element
	{
		T data;
		int iPriority;
	};

	typedef Ut::UtVector< Ut::DynamicVector<element> > T_List;

	T_List m_c;

	void clear()			{ m_c.Clear(); }
	void reserve(int n)		{ m_c.Reserve(n); }
	bool empty()			{ return m_c.Empty(); }

	// swap two elements of the queue
	void Swap(int i1, int i2)
	{
		element tmp;
		tmp = m_c[i1];
		m_c[i1] = m_c[i2];
		m_c[i2] = tmp;
	}

	// move a newly added node to the right place in the heap
	void push(T pNewNode, int iPriority)
	{
		int iTestNode;

		element newe;
		newe.data = pNewNode;
		newe.iPriority = iPriority;

		// add thew new node pointer
		m_c.PushBack(newe);

		// sort the node pointer into the right place
		iTestNode = m_c.Size();
		while ( iTestNode > 1 )
		{
			int iParentNode = iTestNode/2;
			if ( m_c[iParentNode-1].iPriority < m_c[iTestNode-1].iPriority )
			{
				Swap(iParentNode-1,iTestNode-1);
				iTestNode = iParentNode;
			}
			else
				break;
		}
	}

	// pop the top node from the heap
	T PopTopNode()
	{
		element temp;
		int m_n = m_c.Size();

		if (m_n==0) return 0;

		temp = m_c[1-1];
		m_c[1-1] = m_c[m_n-1];
		int test_node = 1;
		while (1)
		{
			int child;
			if ( ( test_node * 2 ) >=m_n )
				break;
			if ( ( test_node * 2 + 1) >= m_n )
				child = test_node * 2;
			else if ( m_c[ test_node * 2 - 1].iPriority > m_c[ test_node * 2 + 1 - 1].iPriority )
				child = test_node * 2;
			else
				child = test_node * 2 + 1;
			if ( m_c[test_node-1].iPriority < m_c[child-1].iPriority )
			{
				Swap( test_node-1, child-1 );
				test_node = child;
			}
			else
				break;
		}

//		Log("get item with priority: %d\n",temp.iPriority);
//		m_c[ m_n-1 ] = temp;
//		temp = m_c.Back();
		m_c.PopBack();
		return temp.data;
	}

	// TODO: iterate through the list
};

