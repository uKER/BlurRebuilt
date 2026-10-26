#ifndef JOBQUEUE_ATOMIC_QUEUE_H
#define JOBQUEUE_ATOMIC_QUEUE_H

#include <core/threads/AtomicMutex.h>

template<class T, int UNUSED>
struct jqAtomicQueue
{
	int Count;
	T* Data;

	volatile int InCount;
	volatile int OutCount;
	volatile U32 WritePos;
	volatile U32 SyncWritePos;
	volatile U32 ReadPos;

	jqAtomicQueue<T, UNUSED>* ThisPtr;

	void Init(int _Count)
	{
		ThisPtr = this;

		InCount = 0;
		OutCount = 0;
		WritePos = 0;
		SyncWritePos = 0;
		ReadPos = 0;

		Count = _Count;
		Data = (T*)BizAlignedMalloc(sizeof(T) * Count, tl_alignof(T));
	}

	void Shutdown()
	{
		BizAlignedFree(Data);
	}

	void Push(const T& Value)
	{
		for (;;)
		{
			if (MT::AtomicIncrement(&ThisPtr->InCount) < Count)
				break;
			MT::AtomicDecrement(&ThisPtr->InCount);
			MT::YieldThread();
		}

		int Index = MT::AtomicAdd(&ThisPtr->WritePos, 1);
		if (Index == Count-1)
			MT::AtomicAdd(&ThisPtr->WritePos, -Count);

		Spu::Store(&Data[Index % Count], &Value, sizeof(T));
		MT::MemoryFence();

		while (!MT::AtomicCompareAndSwap(&ThisPtr->SyncWritePos, (Index + 1) % Count, Index % Count))
			MT::YieldThread();

		MT::AtomicIncrement(&ThisPtr->OutCount);
	}

	bool Pop(T *Value)
	{
		if (MT::AtomicDecrement(&ThisPtr->OutCount) <= 0)
		{
			MT::AtomicIncrement(&ThisPtr->OutCount);
			return false;
		}

		int Index = MT::AtomicAdd(&ThisPtr->ReadPos, 1);
		if (Index == Count-1)
			MT::AtomicAdd(&ThisPtr->ReadPos, -Count);

		Spu::Fetch(Value, &Data[Index % Count], sizeof(T));
		MT::MemoryFence();

		MT::AtomicDecrement(&ThisPtr->InCount);
		return true;
	}
};

#endif
