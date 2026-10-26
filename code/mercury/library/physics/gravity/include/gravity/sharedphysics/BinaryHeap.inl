//#include "gravity/sharedphysics/BinaryHeap.h"


#ifndef BINARYHEAP_INL
#define BINARYHEAP_INL

namespace SharedPhysics
{

template <class Elem>
NOALIAS inline U32 BinaryHeap<Elem>::left(const U32 root) const
{
  // assert(root < currentSize/2);
   return (root * 2) + 1;
}

template <class Elem>
NOALIAS inline U32 BinaryHeap<Elem>::right(const U32 root) const
{
  // assert(root < (currentSize-1)/2); //does a right child exist?
   return (root * 2) + 2;
}

template <class Elem>
NOALIAS inline int BinaryHeap<Elem>::parent(const int child) const
{
  // assert(child != 0); //main root has no parent
   return (child - 1) / 2;
}

template <class Elem>
NOALIAS inline bool BinaryHeap<Elem>::isFull() const
{
	return currentSize > maxSize;
}


template <class Elem>
NOALIAS inline bool BinaryHeap<Elem>::isEmpty() const
{
	return currentSize == 0;
}

template <class Elem>
NOALIAS inline U32 BinaryHeap<Elem>::size() const
{
   return currentSize;
}

template <class Elem>
NOALIAS inline Elem BinaryHeap<Elem>::value(const U32 pos) const
{
   BIZ_ASSERT(pos < currentSize); //is pos a valid index in the heap
   return heap[pos];
}

template <class Elem>
NOALIAS inline void BinaryHeap<Elem>::makeEmpty() 
{
	currentSize=0;
}

template <class Elem>
NOALIAS inline void BinaryHeap<Elem>::setSize(const U32 size)
{
	currentSize=size;
}



/*template <class Elem>
__forceinline void BinaryHeap<Elem>::insert(Elem item, int(*Comp)(Elem&, Elem&))
{
   assert(!isFull());
   heap[currentSize] = item; //elements represents the array position after the last, since indexing starts with 0
   U32 new_pos = currentSize++; //index of the new item
  
   while((new_pos !=0) && (Comp(heap[new_pos],  heap[parent(new_pos)]))>0) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   U32 parentIndex=parent(new_pos);
	   Elem temp=heap[new_pos];
	   heap[new_pos]=heap[parentIndex];
	   heap[parentIndex]=temp;			//swap the value of item with its lesser parent
	   new_pos = parentIndex; //update the item's positions
	  // parentIndex=parent(new_pos);
   }
   
}
*/

template <class Elem>
/*NOALIAS __forceinline*/ void BinaryHeap<Elem>::insert(const Elem PCREF item)
{
   BIZ_ASSERT(!isFull());
	//if(!isFull())
	//{
   heap[currentSize] = item;
   Elem new_elem = item;
   //*new_elem = item; //elements represents the array position after the last, since indexing starts with 0
   int new_pos = currentSize++; //index of the new item
   Elem par = heap[parent(new_pos)];
   
   
   while((new_pos !=0) && new_elem < par) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   U32 parentIndex=parent(new_pos);
	   heap[parentIndex] = new_elem;
	   heap[new_pos] = par;
	   //Elem temp=*new_elem;
	   //*new_elem=*pParent;
	   //*pParent=temp;			//swap the value of item with its lesser parent
	   new_pos = parentIndex; //update the item's positions
	   new_elem = par;
	   par = heap[Maths::Max(0, parent(new_pos))];
	  // parentIndex=parent(new_pos);
   }
	//}
   
}


template <class Elem>
NOALIAS inline void BinaryHeap<Elem>::deleteTop()
{
	BIZ_ASSERT(!isEmpty());
	//if(!isEmpty())
	//{
	currentSize--; //update the amount of elements in heap
	//res=heap[0];
	if(currentSize != 0) //if we didn't delete the root
	{
		//Elem temp=heap[0];
		heap[0]=heap[currentSize];
		//heap[currentSize]=temp;
      //swap(array[0],array[elements]);
		reheap();
	}
	//}
}

template <class Elem>
NOALIAS inline void BinaryHeap<Elem>::deleteTop(Elem& res)
{
	BIZ_ASSERT(!isEmpty());
	const U32 size(--currentSize); //update the amount of elements in heap
	res=heap[0];

	if(size > 0)//if we didn't delete the root
	{
		heap[0]=heap[size];
		reheap(size);
	}
}

template<class Elem>
NOALIAS inline Elem BinaryHeap<Elem>::getTop() const 
{
	BIZ_ASSERT(currentSize > 0);
	return heap[0];
}


template <class Elem>
NOALIAS inline void BinaryHeap<Elem>::reheap(const U32 currentSize)
{
	U32 root=0;
	//U32 size(currentSize);

	while(root < (currentSize>>1))//insure a left child exist
	{
		U32 child = left(root);
		child += U32((child < (currentSize-1)) & (heap[child] > heap[child+1])); //if a right child exists, and it's smaller than the left child, it will be used
		if(heap[root] <= heap[child]) //if root is smaller than its smallest child, stop.
			break;

		Elem temp=heap[root];
		heap[root]=heap[child];
		heap[child]=temp;
		root=child;
	}
}


template <class Elem>
NOALIAS inline U32 BinaryHeapStorage<Elem>::left(const U32 root) const
{
  // assert(root < currentSize/2);
   return (root * 2) + 1;
}

template <class Elem>
NOALIAS inline U32 BinaryHeapStorage<Elem>::right(const U32 root) const
{
  // assert(root < (currentSize-1)/2); //does a right child exist?
   return (root * 2) + 2;
}

template <class Elem>
NOALIAS inline int BinaryHeapStorage<Elem>::parent(const int child) const
{
  // assert(child != 0); //main root has no parent
	return Maths::Max( 0, (child - 1) / 2 );
}

template <class Elem>
NOALIAS inline bool BinaryHeapStorage<Elem>::isFull() const
{
	return m_currentSize > m_maxSize;
}


template <class Elem>
NOALIAS inline bool BinaryHeapStorage<Elem>::isEmpty() const
{
	return m_currentSize == 0;
}

template <class Elem>
NOALIAS inline U32 BinaryHeapStorage<Elem>::size() const
{
   return m_currentSize;
}

template <class Elem>
NOALIAS inline Elem* BinaryHeapStorage<Elem>::value(const U32 pos) const
{
   BIZ_ASSERT(pos < m_currentSize); //is pos a valid index in the heap
   return m_heap[pos].pElem;
}

template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::makeEmpty() 
{
	m_currentSize=0;
}

template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::setSize(const U32 size)
{
	m_currentSize=size;
}



/*template <class Elem>
__forceinline void BinaryHeap<Elem>::insert(Elem item, int(*Comp)(Elem&, Elem&))
{
   assert(!isFull());
   heap[currentSize] = item; //elements represents the array position after the last, since indexing starts with 0
   U32 new_pos = currentSize++; //index of the new item
  
   while((new_pos !=0) && (Comp(heap[new_pos],  heap[parent(new_pos)]))>0) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   U32 parentIndex=parent(new_pos);
	   Elem temp=heap[new_pos];
	   heap[new_pos]=heap[parentIndex];
	   heap[parentIndex]=temp;			//swap the value of item with its lesser parent
	   new_pos = parentIndex; //update the item's positions
	  // parentIndex=parent(new_pos);
   }
   
}
*/

template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::insert(Elem* item, U32 currentSize)
{
   BIZ_ASSERT(!isFull());
   int childIndex = currentSize++;
   int parentIndex = parent(childIndex);
   m_heap[childIndex].pElem = item;   
   Elem* pChildElement  = item;
   Elem*  pParentElement = m_heap[parentIndex].pElem;

   while((childIndex !=0) & (*pChildElement < *pParentElement)) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   int newParentIndex = parent(parentIndex);
	   Elem* pNewParentElement = m_heap[newParentIndex].pElem;

	   //swap the value of item with its lesser parent
	   m_heap[parentIndex].pElem = pChildElement;
	   m_heap[childIndex].pElem  = pParentElement;

	   childIndex   = parentIndex;			//update the item's positions
	
	   pChildElement = pParentElement;
	   parentIndex   = newParentIndex;
	   pParentElement = pNewParentElement;
   }

   /*while((new_pos !=0) && heap[new_pos] < heap[parent(new_pos)]) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   U32 parentIndex=parent(new_pos);

	   Elem* temp=heap[parentIndex].pElem;
	   heap[parentIndex].pElem = heap[new_pos].pElem;
	   heap[new_pos].pElem = temp; //swap the value of item with its lesser parent
	   
	   new_pos = parentIndex;     //update the item's positions
   }
   */
}


template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::insert(Elem* item)
{
   BIZ_ASSERT(!isFull());
   int childIndex = m_currentSize++;
   int parentIndex = parent(childIndex);
   m_heap[childIndex].pElem = item;   
   if(parentIndex == childIndex)
	   return;
   /*Elem* __restrict pChildElement  = item;
   Elem* __restrict pParentElement = m_heap[parentIndex].pElem;

   while((childIndex !=0) & ((*pChildElement) < (*pParentElement))) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   int newParentIndex = parent(parentIndex);
	   Elem* __restrict pNewParentElement = m_heap[newParentIndex].pElem;

	   //swap the value of item with its lesser parent
	   m_heap[parentIndex].pElem = pChildElement;
	   m_heap[childIndex].pElem  = pParentElement;

	   childIndex   = parentIndex;			//update the item's positions
	
	   pChildElement = pParentElement;
	   parentIndex   = newParentIndex;
	   pParentElement = pNewParentElement;
   }*/

   U32 new_pos = childIndex;

   while((new_pos !=0) && m_heap[new_pos] < m_heap[parent(new_pos)]) //loop while the item has not become the main root, and while its value is less than its parent
   {
	   U32 parentIndex=parent(new_pos);

	   Elem* temp=m_heap[parentIndex].pElem;
	   m_heap[parentIndex].pElem = m_heap[new_pos].pElem;
	   m_heap[new_pos].pElem = temp; //swap the value of item with its lesser parent
	   
	   new_pos = parentIndex;     //update the item's positions
   }

#ifdef _DEBUG
	for(U32 a = 1; a < this->m_currentSize; ++a)
	{
		BIZ_ASSERT(m_heap[0] <= m_heap[a]);
	}
#endif   
}


template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::deleteTop()
{
	BIZ_ASSERT(!isEmpty());
	//if(!isEmpty())
	//{
	m_currentSize--; //update the amount of elements in heap
	//res=m_heap[0];
	if(m_currentSize != 0) //if we didn't delete the root
	{
		m_heap[0]=m_heap[m_currentSize];
		reheap();
	}
}

template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::deleteTop(Elem** res)
{
	BIZ_ASSERT(!isEmpty());
	const U32 size(--m_currentSize); //update the amount of elements in heap
	*res=m_heap[0].pElem;

	if(size > 0)//if we didn't delete the root
	{
		m_heap[0]=m_heap[size];
		reheap(size);
	}
#ifdef _DEBUG
	for(U32 a = 1; a < this->m_currentSize; ++a)
	{
		BIZ_ASSERT(m_heap[0] <= m_heap[a]);
	}
#endif
}

template<class Elem>
NOALIAS inline Elem* BinaryHeapStorage<Elem>::getTop() const 
{
	BIZ_ASSERT(m_currentSize > 0);
	return m_heap[0].pElem;
}


template <class Elem>
NOALIAS inline void BinaryHeapStorage<Elem>::reheap(const U32 currentSize)
{
	U32 root=0;
	while(root < (currentSize>>1))//insure a left child exist
	{
		U32 child = left(root);
		child += U32((child < (currentSize-1)) & (m_heap[child] > m_heap[child+1])); //if a right child exists, and it's smaller than the left child, it will be used
		if(m_heap[root] <= m_heap[child]) //if root is smaller than its smallest child, stop.
		{
			return;
		}
		BHStorage<Elem> temp=m_heap[root];
		m_heap[root]=m_heap[child];
		m_heap[child]=temp;
		root=child;
	}
}



};

#endif


