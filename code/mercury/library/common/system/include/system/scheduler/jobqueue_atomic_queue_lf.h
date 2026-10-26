#ifndef JOBQUEUE_ATOMIC_QUEUE_H
#define JOBQUEUE_ATOMIC_QUEUE_H

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
template<class T>
struct jqAtomicQueuePtr
{
	union
	{
		struct
		{
			T * volatile Ptr;            //!< Pointer to node data.
			volatile unsigned int Count; //!< Change count for Ptr.  Incremented whenever Ptr changes.
		};
		volatile u64 Val64;              //!< 64bit value for operating on Ptr and Count atomically.
	};

	jqAtomicQueuePtr() { Val64 = 0LL; }
	jqAtomicQueuePtr(u64 Val64) : Val64(Val64) {}
	jqAtomicQueuePtr(T* volatile Ptr, volatile unsigned int Count) : Ptr(Ptr), Count(Count) {}
};

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
template<class T, int RefillSize>
struct jqAtomicQueue
{
	//! Combination of node data and singly linked forward pointer.  Nodes are either in the queue or in the free list.
	struct NodeType
	{
		jqAtomicQueuePtr<NodeType> Next;
		T Data;
	};

	// A safe pointer to a node which keeps track of its modification count.
	typedef jqAtomicQueuePtr<NodeType> NodePointerType;

	NodePointerType* FreeListPtr;  //!< The active free list. Initially points to _FreeList, but can be changed to share with other queues.
	NodePointerType _FreeList;     //!< Storage for the default free list.

	NodePointerType Head;          //!< The current head pointer of the queue, where nodes are added.  Always up to date.
	NodePointerType Tail;          //!< The current tail pointer of the queue, where nodes are popped.  Can be behind the actual tail of the list.

	//! Pointer to this, used by this class on SPU to reference its system memory version.
	jqAtomicQueue<T, RefillSize>* ThisPtr;

	//! Initializes the queue, either allocating a new free list or sharing with an existing queue object.  
	/** 
		This function should only ever be called once, after which the object cannot be moved from its original 
		memory location, except to copy it to the SPU on Playstation3 (from where it will reference its original
		address via the ThisPtr member).
	*/
	void Init(jqAtomicQueue* SharedFreeList = NULL)
	{
		ThisPtr = this;

		// Initialize the free list.
		_FreeList.Val64 = 0;
		if (SharedFreeList)
			FreeListPtr = SharedFreeList->FreeListPtr;
		else
			FreeListPtr = &_FreeList;

		// Initialize the queue to empty.
		NodeType* Node = AllocateNode();
		Node->Next.Ptr = NULL;
		Head.Ptr = Tail.Ptr = Node;
	}

	//! Pushes a new node onto the front of the queue.  Allocates additional nodes if needed.
	void Push(const T& Data)
	{
		// Allocate and fill the node structure.
		NodeType* Node = AllocateNode();

		// Using two atomics to fill in the node structure, to avoid trashing the Count field in its Next pointer.
		jqStore(&Node->Data, &Data, sizeof(T));
		jqSet((u32*)&Node->Next.Ptr, NULL);

		for (;;)
		{
			// Get the current pointer to the tail node.
			NodePointerType CurTail(jqGet((u64*)&ThisPtr->Tail));

			// Get the tail node's next pointer.  If NULL, the tail node pointer correctly points to the end of the queue.
			// Otherwise, we have to scan the tail forward in order to find the true end.
			NodePointerType Next(jqGet((u64*)&CurTail.Ptr->Next));

			if (CurTail.Val64 == jqGet((u64*)&ThisPtr->Tail))
			{
				if (Next.Ptr == NULL)
				{
					// Next was NULL, meaning CurTailPtr was the actual tail and we can attempt to append after CurTailPtr.  
					// If the compare and swap succeeds we have appended, otherwise we'll try again.
					MT::MemoryFence();
					if (tlAtomicCompareAndSwap(&CurTail.Ptr->Next.Val64, NodePointerType(Node, Next.Count+1).Val64, Next.Val64))
					{
						// Now that we have added a node successfully, make a single attempt to advance the tail node pointer
						// to point at the new node.
						tlAtomicCompareAndSwap(&ThisPtr->Tail.Val64, NodePointerType(Node, CurTail.Count+1).Val64, CurTail.Val64);
						return;
					}
				}
				else
				{
					// Next was not NULL, meaning CurTail has fallen behind the true tail.  Attempt to advance it.
					tlAtomicCompareAndSwap(&ThisPtr->Tail.Val64, NodePointerType(Next.Ptr, CurTail.Count+1).Val64, CurTail.Val64);
				}
			}
		}
	}

	//! Pops one node from the back of the queue.  Returns false if the queue is empty at the time of the pop.
	bool Pop(T *p)
	{
		for (;;)
		{
			// Get the Head, Tail and Head->Next pointers.
			NodePointerType CurHead(jqGet((u64*)&ThisPtr->Head));
			NodePointerType CurTail(jqGet((u64*)&ThisPtr->Tail));
			NodePointerType Next(jqGet((u64*)&CurHead.Ptr->Next));

			if (CurHead.Val64 == jqGet((u64*)&ThisPtr->Head))
			{
				// If Head is equal to Tail, the queue might be empty or else Tail might have fallen behind the real tail.
				if (CurHead.Ptr == CurTail.Ptr)
				{
					// If the queue really is empty, Head.Next should equal NULL.
					if (Next.Ptr == NULL)
						return false;

					// The Head node had a Next node, which means the Tail pointer must be falling behind.
					// Attempt to advance it by one node.
					tlAtomicCompareAndSwap(&ThisPtr->Tail.Val64, NodePointerType(Next.Ptr, CurTail.Count+1).Val64, CurTail.Val64);
				}
				else if (Next.Ptr != NULL)
				{
					// Success- Transfer the node's data into the output pointer and move the old Head node to the free list.
					// Must be read BEFORE the compare and swap, otherwise another Pop might free the node.
					jqFetch(p, &Next.Ptr->Data, sizeof(T));
					// Attempt to pop the Head node by advancing the Head pointer once.  The fence is here to make sure the
					// data copy happens before the CAS.
					MT::MemoryFence();
					if (tlAtomicCompareAndSwap(&ThisPtr->Head.Val64, NodePointerType(Next.Ptr, CurHead.Count+1).Val64, CurHead.Val64))
					{
						FreeNode(CurHead.Ptr);
						return true;
					}
				}
			}
		}
	}

	//! Allocates a new queue node from the free list, possibly invoking the system memory allocator to allocate more nodes.
	/**
		Internal method.  Note that once allocated, queue nodes will never be freed.  This is to prevent the possibility
		of the lock free counting mechanisms getting out of sync.
	*/
	NodeType* AllocateNode()
	{
		for (;;)
		{
			NodePointerType OldFreeList;
			OldFreeList.Val64 = jqGet((u64*)FreeListPtr);

			// If the free list pointer is NULL, attempt to allocate a new block of nodes and try again.
			if (OldFreeList.Ptr == NULL)
			{
				AllocateNodeBlock(RefillSize);
			}
			else
			{
				// Free list is not empty.  
				// Attempt to advance the free list pointer by one and return the old free list head as the new block.
				NodePointerType OldFreeListNext(jqGet((u64*)&OldFreeList.Ptr->Next));

				if (tlAtomicCompareAndSwap(&FreeListPtr->Val64, NodePointerType(OldFreeListNext.Ptr, OldFreeList.Count+1).Val64, OldFreeList.Val64))
					return OldFreeList.Ptr;
			}
		}
	}

	//! Releases a node to the free list.
	/**
		Internal method.  Note that once allocated, queue nodes will never be freed.  This is to prevent the possibility
		of the lock free counting mechanisms getting out of sync.
	*/
	void FreeNode(NodeType* Node)
	{
		for (;;)
		{
			// Link the node to the current free list head.
			NodePointerType OldFreeList(jqGet((u64*)FreeListPtr));
			jqSet((u32*)&Node->Next.Ptr, (u32)OldFreeList.Ptr);

			// Attempt to replace the free list head with this node.
			if (tlAtomicCompareAndSwap(&FreeListPtr->Val64, NodePointerType(Node, OldFreeList.Count+1).Val64, OldFreeList.Val64))
				break;
		} 
	}

	//! Allocates a new block of node objects via the system memory allocator, linking them onto the head of the free list.
	/** 
		\warning It is critical that the callback defined for tlMemAlloc be thread safe!  It is preferable that
		it also be lock free.
	*/
	void AllocateNodeBlock(int Count)
	{
#if TL_PLAT_PS3_SPU
		// SPU does not support allocating from the heap.
		TL_ASSERT(false, "jqAtomicQueue ran out of nodes on SPU, but is incapable of expanding the Node pool!");
#else
		// Allocate a new block of nodes.
		NodeType* NodeBlock = (NodeType*)tlMemAlloc(sizeof(NodeType) * Count, tl_alignof(NodeType));

		// Link the new nodes together.
		for(int i = 0; i < Count - 1; i++)
			NodeBlock[i].Next.Ptr = &NodeBlock[i+1];
		NodeBlock[Count-1].Next.Ptr = NULL;

		// Attempt to install the new block at the head of the free list.  If somebody else got here first and
		//  changed the free list, we have to release our block and start over again (although there will most
		//  likely now be nodes available, at least).
		NodePointerType OldFreeList(jqGet((u64*)FreeListPtr));

		MT::MemoryFence();
		if (!tlAtomicCompareAndSwap(&FreeListPtr->Val64, NodePointerType(NodeBlock, OldFreeList.Count+1).Val64, OldFreeList.Val64))
			tlMemFree(NodeBlock);
#endif
	}
};

#endif
