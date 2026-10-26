#ifndef JOBQUEUE_ATOMIC_QUEUE_H
#define JOBQUEUE_ATOMIC_QUEUE_H

#include "system/scheduler/jobqueue.h"

namespace Scheduler
{

#define QUEUE_REFILL_SIZE 1024

//! jqAtomicQueuePtr
/** Represents a pointer to a node in a jqAtomicQueue.

	jqAtomicQueuePtr is a pointer to a queue node, paired with a count value.  They are stored in a union with a 64bit value.

	The pointer is simply a pointer to some place in the queue.  The count value is a *change count* for the pointer value.
	Every time the pointer value is modified, the count value is incremented.  The reason for this is due to complicated 
	interaction patterns with multiple threads accessing a lock free queue at the same time.  

	For example, due to the fact that nodes are recycled via a free list, a thread might pop and then push a node with 
	the same pointer value, while a second thread is in the middle of operating on the queue.  The second thread might 
	mistakenly assume that the before and after nodes were the same node, causing it to corrupt the queue.  The change 
	count variable protects against this case.

	The union with a 64bit value allows the class to use 64bit atomic compare and swap operations on both the pointer value
	and the change count simultaneously.
*/
#if defined(_WIN64)
// [Blur2PC][x64] With a 64-bit pointer, Ptr and Count no longer fit one 64-bit word. They form a
// 16-byte pair operated on with cmpxchg16b, so the full 32-bit change count still guards against ABA.
template<typename T>
struct __declspec(align(16)) jqAtomicQueuePtr
{
	union
	{
		struct
		{
			T * volatile Ptr;            //!< Pointer to node data.
			volatile unsigned int Count; //!< Change count for Ptr.  Incremented whenever Ptr changes.
		};
		volatile __int64 Val128[2];      //!< 128bit value for operating on Ptr and Count atomically.
	};

	jqAtomicQueuePtr() { Val128[0] = 0; Val128[1] = 0; }
	jqAtomicQueuePtr(T* volatile Ptr, volatile unsigned int Count) { Val128[1] = 0; this->Ptr = Ptr; this->Count = Count; }
	jqAtomicQueuePtr(const jqAtomicQueuePtr& o) { Val128[0] = o.Val128[0]; Val128[1] = o.Val128[1]; }
	jqAtomicQueuePtr& operator=(const jqAtomicQueuePtr& o) { Val128[0] = o.Val128[0]; Val128[1] = o.Val128[1]; return *this; }
};

//! Atomic read of a pointer/count pair (a compare-exchange that stores back what it finds).
template<typename T>
inline jqAtomicQueuePtr<T> jqPtrLoad(const volatile jqAtomicQueuePtr<T>* p)
{
	jqAtomicQueuePtr<T> r;
	_InterlockedCompareExchange128((volatile __int64*)p->Val128, 0, 0, (__int64*)r.Val128);
	return r;
}

template<typename T>
inline bool jqPtrCompareAndSwap(volatile jqAtomicQueuePtr<T>* p, const jqAtomicQueuePtr<T>& newvalue, const jqAtomicQueuePtr<T>& compare)
{
	__int64 cmp[2] = { compare.Val128[0], compare.Val128[1] };
	return _InterlockedCompareExchange128(p->Val128, newvalue.Val128[1], newvalue.Val128[0], cmp) != 0;
}

template<typename T>
inline bool jqPtrEqual(const jqAtomicQueuePtr<T>& a, const jqAtomicQueuePtr<T>& b)
{
	return a.Val128[0] == b.Val128[0] && a.Val128[1] == b.Val128[1];
}
#else
template<typename T>
struct jqAtomicQueuePtr
{
	union
	{
		struct
		{
			T * volatile Ptr;            //!< Pointer to node data.
			volatile unsigned int Count; //!< Change count for Ptr.  Incremented whenever Ptr changes.
		};
		volatile U64 Val64;              //!< 64bit value for operating on Ptr and Count atomically.
	};

	jqAtomicQueuePtr() { Val64 = 0LL; }
	jqAtomicQueuePtr(U64 Val64) : Val64(Val64) {}
	jqAtomicQueuePtr(T* volatile Ptr, volatile unsigned int Count) : Ptr(Ptr), Count(Count) {}
};

// [Blur2PC][x64] The 64-bit operations the queue always used, named so x64 can supply 128-bit ones.
template<typename T>
inline jqAtomicQueuePtr<T> jqPtrLoad(const volatile jqAtomicQueuePtr<T>* p)
{
	return jqAtomicQueuePtr<T>(Spu::Get((U64*)p));
}

template<typename T>
inline bool jqPtrCompareAndSwap(volatile jqAtomicQueuePtr<T>* p, const jqAtomicQueuePtr<T>& newvalue, const jqAtomicQueuePtr<T>& compare)
{
	return MT::AtomicCompareAndSwap(&p->Val64, newvalue.Val64, compare.Val64);
}

template<typename T>
inline bool jqPtrEqual(const jqAtomicQueuePtr<T>& a, const jqAtomicQueuePtr<T>& b)
{
	return a.Val64 == b.Val64;
}
#endif

//! Unidirectional lock free queue class based on atomic primitives.
/** 
	The class maintains Head and Tail pointers for the singly linked list of nodes.  New nodes are pushed on after Tail,
	and are popped from Head.  A dummy Head node is maintained to avoid the need to check for NULL pointers.

	Due to the wait-free nature of the class, it is possible for the Tail pointer to fall behind the real tail node.
	So, at various important points the class checks for this case and advances the Tail pointer.  The Next pointers 
	linking the nodes together and the Head pointer are guaranteed to always be valid, though.

	Nodes are allocated and deleted using a free list.  The free list can be shared with other jqAtomicQueue objects of 
	the same node type.  When the queue runs out of nodes, it is expanded via calls to tlMemAlloc.  It is critical that
	tlMemAlloc be a thread safe implementation, and preferable that it also be lock free.

	The paper describing the algorithm in detail can be found here: http://www.research.ibm.com/people/m/michael/podc-1996.pdf 

	\remark On Playstation3, this class interoperates between the PPU and SPU processors.  Either can push or pop nodes.
	However, some care must be taken - all synchronization is done via PPU memory, so the class object must not
	move after being uploaded to the SPUs.  The SPUs will attempt to locate the original object via the ThisPtr
	member variable stored in the Init method.
*/
struct jqAtomicQueue
{
	//! Combination of node data and singly linked forward pointer.  Nodes are either in the queue or in the free list.
	struct ALIGN128 NodeType
	{
		jqAtomicQueuePtr<NodeType> Next;
		Task Data;
	};

	// A safe pointer to a node which keeps track of its modification count.
	typedef jqAtomicQueuePtr<NodeType> NodePointerType;

	NodePointerType* FreeListPtr;  //!< The active free list. Initially points to _FreeList, but can be changed to share with other queues.
	NodePointerType _FreeList;     //!< Storage for the default free list.

	NodePointerType Head;          //!< The current head pointer of the queue, where nodes are added.  Always up to date.
	NodePointerType Tail;          //!< The current tail pointer of the queue, where nodes are popped.  Can be behind the actual tail of the list.

	//! Pointer to this, used by this class on SPU to reference its system memory version.
	jqAtomicQueue* ThisPtr;

	//! Initializes the queue, either allocating a new free list or sharing with an existing queue object.  
	/** 
		This function should only ever be called once, after which the object cannot be moved from its original 
		memory location, except to copy it to the SPU on Playstation3 (from where it will reference its original
		address via the ThisPtr member).
	*/
	void Init(jqAtomicQueue* SharedFreeList = NULL);

	//! Pushes a new node onto the front of the queue.  Allocates additional nodes if needed.
	void Push(const Task& Data);

	//! Pops one node from the back of the queue.  Returns false if the queue is empty at the time of the pop.
	bool Pop(Task *p);

	//! Allocates a new queue node from the free list, possibly invoking the system memory allocator to allocate more nodes.
	/**
		Internal method.  Note that once allocated, queue nodes will never be freed.  This is to prevent the possibility
		of the lock free counting mechanisms getting out of sync.
	*/
	NodeType* AllocateNode();

	//! Releases a node to the free list.
	/**
		Internal method.  Note that once allocated, queue nodes will never be freed.  This is to prevent the possibility
		of the lock free counting mechanisms getting out of sync.
	*/
	void FreeNode(NodeType* Node);

	//! Allocates a new block of node objects via the system memory allocator, linking them onto the head of the free list.
	/** 
		\warning It is critical that the callback defined for tlMemAlloc be thread safe!  It is preferable that
		it also be lock free.
	*/
	void AllocateNodeBlock(int Count);
	void Shutdown() {}
};

} // namespace Scheduler

#endif

