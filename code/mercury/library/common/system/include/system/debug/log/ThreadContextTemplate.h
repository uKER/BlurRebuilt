#ifndef __SYSTEM_LOG_THREAD_CONTEXT_TEMPLATE_H__
#define __SYSTEM_LOG_THREAD_CONTEXT_TEMPLATE_H__

#pragma once

namespace MT
{
	class ThreadLocalStorage;
}

namespace Debug
{

template <typename T>
T InitialiseEmpty()
{
	return T();
}

/// A stack of debug context items which unique per-thread
// PERFORMANCE WARNING: pushing context and then destroying the thread will not free the context stack until the app exits
template<typename T>
class ThreadContextTemplate
{
public:
	typedef Ut::UtVector<Ut::StackVector<T,48> >			ItemStack;
	typedef Ut::Vector<ItemStack*>							StackPtrVector;

	static ThreadContextTemplate& Instance()
	{
		static ThreadContextTemplate s_instance;

		return s_instance;
	}


	/// constructor for the singleton
	ThreadContextTemplate()
	{
	}

	// destructor for the singleton
	~ThreadContextTemplate()
	{
		for (int i=0; i<m_Stacks.Size(); i++)
			delete m_Stacks[i];
	}

	static void CleanUp(void)
	{
		for (int i=0; i<Instance().m_Stacks.Size(); i++)
			delete Instance().m_Stacks[i];

		Instance().m_Stacks.Destroy();
	}

	/// Push context item onto the stack
	void Push(const T &item)
	{
		GetStack()->PushBack(item);
	}
	
	/// Pop context item from the stack
	void Pop()
	{
		assert(GetStack()->Size() > 1); // assert here if we try to pop the last context from the stack

		GetStack()->PopBack();
	}

	/// Get current thread context item
	const T &GetContext()
	{
		ItemStack *pStack = (ItemStack*)m_TLS.GetData();
		if (!pStack)
		{
			static const T emptyItem = InitialiseEmpty<T>();
			return emptyItem;
		}
		return GetStack()->Back();
	}

private:
	/// retrieve the stack for the current thread, create if needed
	ItemStack *GetStack()
	{
		ItemStack *pStack = (ItemStack*)m_TLS.GetData();
		if (pStack)
			return pStack;

		// create a new stack for this thread
		pStack = new ItemStack;

		m_Mutex.Wait();
		{
			m_Stacks.PushBack(pStack);
		}
		m_Mutex.Release();

		m_TLS.SetData(pStack);
		static const T emptyItem = InitialiseEmpty<T>();
		Push(emptyItem);
		return pStack;
	}

	// thread local storage
	MT::ThreadLocalStorage			m_TLS;

	MT::Mutex						m_Mutex;

	// keep pointers to the stacks for all the threads
	StackPtrVector					m_Stacks;
};

};

#endif



