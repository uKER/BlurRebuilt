#ifndef FUNCTIONQUEUE_H
#define FUNCTIONQUEUE_H

//-----------------------------------------------------------------------------------

/* -- FUNCTION QUEUE SYSTEM --

You can use this to queue up functions in order to run them later.

You must define a function 'item' to encapsulate each function, by deriving a class from C_FunctionQueueItem and
overriding the Do method. Your item class may contain member data which to act as function arguments.

Call C_FunctionQueue::Add to add a function to the queue, and C_FunctionQueue::Flush to run all the functions
in the queue.

*/


class C_FunctionQueueItem : public Ut::ListElementSingle< C_FunctionQueueItem >
{
public:
	virtual ~C_FunctionQueueItem() {}

	void Destroy() { delete this; }

	virtual void Do() = 0;
};


class C_FunctionQueue
{
public:

	// Adds a copy of item to the end of the queue.
	template < class T >
	void Add( const T & item )
	{
		m_theList.PushBack( *( new T( item ) ) );
	}

	// Runs all the functions in the queue, in order, then clears the queue.
	void Flush()
	{
		for ( Iterator it = m_theList.Begin(); it != m_theList.End(); ++it )
			it->Do();

		m_theList.Clear();
	}


private:
	typedef Ut::UtList< C_FunctionQueueItem > List;
	typedef List::Iterator Iterator;
	
	List m_theList;
};


//-----------------------------------------------------------------------------------

#endif
