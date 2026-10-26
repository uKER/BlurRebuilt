///=================================================================
/// RecentEventsStore.cpp
/// 10.08.09
///=================================================================
#include "stdafx.h"

#include "RecentEventsStore.h"
///=================================================================
/// Defines
///=================================================================
namespace Game
{

#define RECENT_EVENT_STORE__PERFORM_UNIT_TESTS	(0 && !defined(CFG_GOLD))

#if RECENT_EVENT_STORE__PERFORM_UNIT_TESTS
	RecentEventsStore  g_unitTestDummy;
#endif
///=================================================================
/// RecentEventsStore
///=================================================================
///-----------------------------------------------------------------
/// Constructor/Destructor
///-----------------------------------------------------------------
RecentEventsStore::RecentEventsStore()
{
	Clear();
	mBuffer.Resize(s_maxRecentEvents);
	for(int i=0;i<s_maxRecentEvents;i++ )
	{
		mBuffer[i]=-i;
	}
#if RECENT_EVENT_STORE__PERFORM_UNIT_TESTS
	UnitTest();
#endif
	
}

///-----------------------------------------------------------------
/// Serialise
///-----------------------------------------------------------------
void 
RecentEventsStore::Serialise( 
	Sys::BinarySerialiserAutoEndian& stream 
)
{
	Sys::BinarySerialiserAutoEndian out( stream );

	out << mSize;
	out << mHead;
	for(int i=0;i<s_maxRecentEvents;i++ )
	{
		out << mBuffer[i];
	}
}

///-----------------------------------------------------------------
/// Deserialise
///-----------------------------------------------------------------
void 
RecentEventsStore::Deserialise( 
	Sys::BinaryDeserialiserAutoEndian& stream 
)
{
	Sys::BinaryDeserialiserAutoEndian in( stream );
	
	in >> mSize;
	in >> mHead;
	for(int i=0;i<s_maxRecentEvents;i++ )
	{
		in >> mBuffer[i];
	}
}

///-----------------------------------------------------------------
/// Push
///-----------------------------------------------------------------
void 
RecentEventsStore::Push(
	U64 val
)
{
	// No duplicates should be stored so first
	// remove any entries which match this new
	// value. 
	for(int i=0;i<Size();i++)
	{
		if(mBuffer[i]==val)
		{
			// If the value we are adding is already
			// at the tail we do not need to remove 
			// the element
			if(i!=mHead)
			{

				// There will only ever be at most, one
				// entry the same as the new value we are 
				// pushing.
				for(int j=i;j!=mHead;)
				{
					// work out our next valid index
					// moving towards mHead
					int nextIndex=j==s_maxRecentEvents-1?0:(j+1);

					// Copy over
					mBuffer[j] = mBuffer[nextIndex];
					
					// Move forwards
					j=nextIndex;	
				}

				// As we have successfully removed an element
				// step the head index back by one. 
				mHead=mHead==0?s_maxRecentEvents-1:(mHead-1);
			}
			else
			{
				int breakhere = 1;
			}
	
			// Update our size
			mSize-=1;
			// No point continuing 
			break;
		}
	}

	// Now the buffer is cleared of any instances
	// of the new value being pushed. 
	// Add to the head. 
	mBuffer[mHead]=val;
	mHead=mHead==s_maxRecentEvents-1?0:(mHead+1);
	mSize=mSize>=s_maxRecentEvents?s_maxRecentEvents:(mSize+1);
}

///-----------------------------------------------------------------
/// Clear
///-----------------------------------------------------------------
void
RecentEventsStore::Clear() 
{

	mHead = 0;
	mSize = 0;
}

///-----------------------------------------------------------------
/// Size
///-----------------------------------------------------------------
int
RecentEventsStore::Size() const
{
	// mTail is -1 until we wrap around 
	// for the first time, so our size
	// is less than Capacity()
	return mSize;
}

///-----------------------------------------------------------------
/// Capacity
///-----------------------------------------------------------------
int
RecentEventsStore::Capacity() const
{
	return s_maxRecentEvents;
}

///-----------------------------------------------------------------
/// operator[]
///-----------------------------------------------------------------
U64			
RecentEventsStore::operator[] (S32 index) const
{
	int size = Size();
	BIZ_ASSERT_MSG(index<size,"RecentEventsStore::operator[] - index out of bounds");
	int i=mHead-(index+1);
	i=i<0?(i+s_maxRecentEvents):i;
	return mBuffer[i];
}
///-----------------------------------------------------------------
/// UnitTest
///-----------------------------------------------------------------
#ifndef CFG_GOLD
void				
RecentEventsStore::Dump()
{
	printf("\n");
	int size = Size();
	for(int i=0;i<size;i++)
	{
		// How do we dump U64's?
		printf( "%d,", (U32)((*this)[i]));
	}
}
#endif
///-----------------------------------------------------------------
/// UnitTest
///-----------------------------------------------------------------
#ifndef CFG_GOLD
void				
RecentEventsStore::UnitTest()
{
#if RECENT_EVENT_STORE__PERFORM_UNIT_TESTS

	RecentEventsStore& store =  g_unitTestDummy;

	// Check Defaults;
	BIZ_ASSERT(store.Size()==0);
	BIZ_ASSERT(store.Capacity()==s_maxRecentEvents);

	// Pushing on a single value
	store.Push((U64)5);
	BIZ_ASSERT(store.Size()==1);
	BIZ_ASSERT(store[0]==(U64)5);
	BIZ_ASSERT(store.Capacity()==s_maxRecentEvents);

	// Resetting after a single value is pushed;
	store.Clear();
	BIZ_ASSERT(store.Size()==0);
	BIZ_ASSERT(store.Capacity()==s_maxRecentEvents);

	// Pushing on several different values but not wrapping
	store.Clear();
	store.Push((U64)0);
	store.Push((U64)1);
	store.Push((U64)2);
	store.Push((U64)3);
	store.Push((U64)4);
	BIZ_ASSERT(store.Size()==5);
	BIZ_ASSERT(store[0]==(U64)4);
	BIZ_ASSERT(store[1]==(U64)3);
	BIZ_ASSERT(store[2]==(U64)2);
	BIZ_ASSERT(store[3]==(U64)1);
	BIZ_ASSERT(store[4]==(U64)0);

	// Resetting after several values are pushed;
	store.Clear();
	BIZ_ASSERT(store.Size()==0);
	BIZ_ASSERT(store.Capacity()==s_maxRecentEvents);

	// Pushing the same value on several times
	store.Clear();
	store.Push((U64)4);
	store.Push((U64)4);
	store.Push((U64)4);
	store.Push((U64)4);
	store.Push((U64)4);
	store.Push((U64)4);
	store.Push((U64)4);
	BIZ_ASSERT(store.Size()==1);
	BIZ_ASSERT(store[0]==(U64)4);

	// Pushing on several different values but not wrapping
	// and then adding the first value pushed again
	store.Clear();
	store.Push((U64)0);
	store.Push((U64)1);
	store.Push((U64)2);
	store.Push((U64)3);
	store.Push((U64)4);
	store.Push((U64)0);
	BIZ_ASSERT(store.Size()==5);
	BIZ_ASSERT(store[0]==(U64)0);
	BIZ_ASSERT(store[1]==(U64)4);
	BIZ_ASSERT(store[2]==(U64)3);
	BIZ_ASSERT(store[3]==(U64)2);
	BIZ_ASSERT(store[4]==(U64)1);

	// Pushing on several different values but not wrapping
	// and then adding the last value pushed again
	store.Clear();
	store.Push((U64)0);
	store.Push((U64)1);
	store.Push((U64)2);
	store.Push((U64)3);
	store.Push((U64)4);
	store.Push((U64)4);
	BIZ_ASSERT(store.Size()==5);
	BIZ_ASSERT(store[0]==(U64)4);
	BIZ_ASSERT(store[1]==(U64)3);
	BIZ_ASSERT(store[2]==(U64)2);
	BIZ_ASSERT(store[3]==(U64)1);
	BIZ_ASSERT(store[4]==(U64)0);

	// Pushing on several different values but not wrapping
	// and then adding a previously add value (not the first
	// or last) again
	store.Clear();
	store.Push((U64)0);
	store.Push((U64)1);
	store.Push((U64)2);
	store.Push((U64)3);
	store.Push((U64)4);
	store.Push((U64)1);
	BIZ_ASSERT(store.Size()==5);
	BIZ_ASSERT(store[0]==(U64)1);
	BIZ_ASSERT(store[1]==(U64)4);
	BIZ_ASSERT(store[2]==(U64)3);
	BIZ_ASSERT(store[3]==(U64)2);
	BIZ_ASSERT(store[4]==(U64)0);

	// Pushing on enough values to wrap (capacity + 1 values)
	// in this order 0,1,2,3,4,5,6,7,8,9
	store.Clear();
	for(int i=0;i<store.Capacity();i++)
	{
		store.Push((U64)i);
	}
	// push the value 10
	store.Push((U64)store.Capacity());
	// State should be 10,9,8,7,6,5,4,3,2,1
	for(int i=0;i<store.Capacity();i++)
	{
		BIZ_ASSERT(store[i]==(U64)(store.Capacity()-i));
	}

	// Using the store in its previously wrapped state
	// add another value (the one at index 0)
	store.Push((U64)store[0]);

	// The state of the store should remain unchanged
	// 10,9,8,7,6,5,4,3,2,1
	store.Dump();
	for(int i=0;i<store.Capacity();i++)
	{
		BIZ_ASSERT(store[i]==(U64)(store.Capacity()-i));
	}
	
	// Using the store in its previously state
	// add another value (the one at the tail)
	// which should be 1
	U64 tailValue = store[store.Capacity()-1];
	store.Push((U64)tailValue);
	store.Dump();
	// The state of the store should have changed
	// so that all values will have been pushed 
	// back one place
	// 1,10,9,8,7,6,5,4,3,2
	BIZ_ASSERT(store[0]==tailValue);
	for(int i=1;i<store.Capacity();i++)
	{
		BIZ_ASSERT(store[i]==(U64)(store.Capacity()+1-i));
	}

	// Using the state as it was previously add
	// an existing item (not the head or tail value)
	// again several time
	store.Push((U64)7);
	store.Push((U64)7);
	store.Push((U64)7);
	BIZ_ASSERT(store[0]==(U64)7);
	BIZ_ASSERT(store[1]==(U64)1);
	BIZ_ASSERT(store[2]==(U64)10);
	BIZ_ASSERT(store[3]==(U64)9);
	BIZ_ASSERT(store[4]==(U64)8);
#endif
}
#endif

}