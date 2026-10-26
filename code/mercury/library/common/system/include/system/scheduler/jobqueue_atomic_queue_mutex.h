#ifndef JOBQUEUE_ATOMIC_QUEUE_H
#define JOBQUEUE_ATOMIC_QUEUE_H

#include <core/threads/AtomicMutex.h>

namespace Scheduler
{

//! Unidirectional queue class based on atomic mutexes.
/** 
	The class maintains Head and Tail pointers for the singly linked list of nodes.  New nodes are pushed on after Tail,
	and are popped from Head.  A dummy Head node is maintained to avoid the need to check for NULL pointers.

	Nodes are allocated and deleted using a free list.  The free list can be shared with other jqAtomicQueue objects of 
	the same node type.  When the queue runs out of nodes, it is expanded via calls to BizAlignedMalloc.  It is critical that
	BizAlignedMalloc be a thread safe implementation, and preferable that it also be lock free.

	The paper describing the algorithm in detail can be found here: http://www.research.ibm.com/people/m/michael/podc-1996.pdf 

	\remark On Playstation3, this class interoperates between the PPU and SPU processors.  Either can push or pop nodes.
	However, some care must be taken - all synchronization is done via PPU memory, so the class object must not
	move after being uploaded to the SPUs.  The SPUs will attempt to locate the original object via the ThisPtr
	member variable stored in the Init method.
*/
struct jqAtomicQueue
{
	//! Combination of node data and singly linked forward pointer.  Nodes are either in the queue or in the free list.
	struct NodeType
	{
		NodeType* Next;
		Scheduler::Task Data;
	};

	struct NodeBlockEntry
	{
		void*			Addr;
		NodeBlockEntry* Next;
	};

	typedef NodeType* NodePointerType;

	NodePointerType* FreeListPtr;  //!< The active free list. Initially points to _FreeList, but can be changed to share with other queues.
	NodePointerType _FreeList;     //!< Storage for the default free list.

	NodeBlockEntry* NodeBlockListHead;

	NodePointerType Head;          //!< The current head pointer of the queue, where nodes are added.  
	NodePointerType Tail;          //!< The current tail pointer of the queue, where nodes are popped.  

	MT::AtomicLock HeadLock;
	MT::AtomicLock TailLock;
	MT::AtomicLock FreeLock;

	//! Pointer to this, used by this class on SPU to reference its system memory version.
	jqAtomicQueue* ThisPtr;

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
		_FreeList = 0;
//		if (SharedFreeList)
//			FreeListPtr = SharedFreeList->FreeListPtr;
//		else
			FreeListPtr = &_FreeList;
		NodeBlockListHead = 0;

		HeadLock.Create();
		TailLock.Create();
		FreeLock.Create();

		// Initialize the queue to empty.
		NodeType* Node = AllocateNode();
		Node->Next = NULL;
		Head = Tail = Node;
	}

	//! Releases memory used by the queue.  Queue cannot be used after this is called.
	void Shutdown()
	{
		NodeBlockEntry*	pEntry = NodeBlockListHead;
		while(pEntry)
		{
			void* addr = pEntry->Addr;
			pEntry = pEntry->Next;
			BizAlignedFree(addr);
		}
	}

	//! Pushes a new node onto the front of the queue.  Allocates additional nodes if needed.
	void Push(const Task& Data)
	{
		// Allocate and fill the node structure.
		NodeType* Node = AllocateNode();

		// Using two atomics to fill in the node structure, to avoid trashing the Count field in its Next pointer.
		Spu::Store(&Node->Data, &Data, sizeof(Task));
		Spu::Set((U32*)&Node->Next, NULL);

		TailLock.Wait();
		NodeType* CurTail = (NodeType*)Spu::Get((U32*)&ThisPtr->Tail);
		Spu::Set((U32*)&CurTail->Next, (U32)Node);
		Spu::Set((U32*)&ThisPtr->Tail, (U32)Node);
		TailLock.Release();
	}

	//! Pops one node from the back of the queue.  Returns false if the queue is empty at the time of the pop.
	bool Pop(Task *p)
	{
		HeadLock.Wait();
		NodeType* Node = (NodeType*)Spu::Get((U32*)&ThisPtr->Head);
		NodeType* NewHead = (NodeType*)Spu::Get((U32*)&Node->Next);
		if (NewHead == NULL)
		{
			HeadLock.Release();
			return false;
		}
		Spu::Fetch(p, &NewHead->Data, sizeof(Task));
		Spu::Set((U32*)&ThisPtr->Head, (U32)NewHead);
		HeadLock.Release();
		FreeNode(Node);
		return true;
	}

	//! Allocates a new queue node from the free list, possibly invoking the system memory allocator to allocate more nodes.
	/**
		Internal method.  Note that once allocated, queue nodes will never be freed.  This is to prevent the possibility
		of the lock free counting mechanisms getting out of sync.
	*/
	NodeType* AllocateNode()
	{
		FreeLock.Wait();
		NodePointerType OldFreeList = (NodePointerType)Spu::Get((U32*)FreeListPtr);
		if (OldFreeList == NULL)
		{
			AllocateNodeBlock(1024);
			OldFreeList = (NodePointerType)Spu::Get((U32*)FreeListPtr);
		}
		NodePointerType OldFreeListNext = (NodePointerType)Spu::Get((U32*)&OldFreeList->Next);
		Spu::Set((U32*)FreeListPtr, (U32)OldFreeListNext);
		FreeLock.Release();
		return OldFreeList;
	}

	//! Releases a node to the free list.
	void FreeNode(NodeType* Node)
	{
		FreeLock.Wait();
		NodePointerType OldFreeList = (NodePointerType)Spu::Get((U32*)FreeListPtr);
		Spu::Set((U32*)&Node->Next, (U32)OldFreeList);
		Spu::Set((U32*)FreeListPtr, (U32)Node);
		FreeLock.Release();
	}

	//! Releases the node containing the given data to the free list.
	void FreeNode(Task* data)
	{
		NodeType* node = (NodeType*)((char*)data - sizeof(NodeType*));
		FreeNode(node);
	}

	//! Allocates a new block of node objects via the system memory allocator, linking them onto the head of the free list.
	/** 
		\warning It is critical that the callback defined for BizAlignedMalloc be thread safe!  It is preferable that
		it also be lock free.
	*/
	void AllocateNodeBlock(int Count)
	{
#if TL_PLAT_PS3_SPU
		// SPU does not support allocating from the heap.
		assert(false); // jqAtomicQueue ran out of nodes on SPU, but is incapable of expanding the Node pool!
#else
		// Allocate a new block of nodes.
		NodeType* NodeBlock = (NodeType*)BizAlignedMalloc((sizeof(NodeType) * Count) + sizeof(NodeBlockEntry), tl_alignof(NodeType));

		// Link the new nodes together.
		for(int i = 0; i < Count - 1; i++)
			NodeBlock[i].Next = &NodeBlock[i+1];
		NodeBlock[Count-1].Next = NULL;

		// Link this nodeblock to the previous
		NodeBlockEntry* pEntry = (NodeBlockEntry*)(NodeBlock+Count);
		pEntry->Addr = NodeBlock;
		pEntry->Next = NodeBlockListHead;
		NodeBlockListHead = pEntry;

		Spu::Set((U32*)FreeListPtr, (U32)NodeBlock);
#endif
	}
};

}

#endif

