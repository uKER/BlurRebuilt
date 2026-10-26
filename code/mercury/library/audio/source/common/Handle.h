#ifndef _HANDLE_H_
#define _HANDLE_H_

#include <btl/containers/vector.h>
#include <System/memory/MemoryTracker.h>

namespace Audio
{
const int HandleCollectionSize = 8192;

// [Blur2PC][x64] Collections map handles to object/instance/stream pointers, so the
// stored resource is pointer-sized (int on x86, as the original S32).
typedef intptr_t HandleResource;

#define flag_hsReservedForUse				1
#define flag_hsReleasePending				2
#define flag_hsInUse						4
#define flag_hsResourceAllocationFailed		8


	template<class T, int N>
	class HandleCollection
	{

	public:
		HandleCollection();
		~HandleCollection();
		bool				InitHandleCollection();
		T					RequestUnusedHandle();
		void				MapResourceToHandle(T h, HandleResource s32Resource);
		bool				FlagHandleForRelease(T h);
		bool				ReleaseHandle(T h, bool bReserve = false);
		inline bool			IsHandleValid(T h) { return (h.IsValid()); }
		inline bool			IsResourceRetrievable(T h) { if (IsHandleValid(h)) { return (IsInUse(h)); } return false; }
		inline HandleResource	GetResource(T h) { if (! (IsInUse(h) && IsInRange(h))) return N; return m_aResource.At(h.Value()); }
		inline HandleResource	GetDoomedResource(T h) { if (! (IsReleasePending(h) && IsInRange(h))) return N; return m_aResource.At(h.Value()); }
		inline bool			SetResourceAllocationFailed(T h) { if (IsReserved(h) && IsInRange(h)) { m_aState.At(h.Value()) = flag_hsResourceAllocationFailed; m_aResource.At(h.Value()) = 0; return true; } return false; }
		inline bool			IsReserved(T h) { return ((GetHandleState(h.Value()) & flag_hsReservedForUse) > 0); }
		inline bool			IsReleasePending(T h) { return ((GetHandleState(h.Value()) & flag_hsReleasePending) > 0); }
		inline bool			DidResourceAllocationFail(T h) { return (GetHandleState(h.Value()) & flag_hsResourceAllocationFailed > 0); }
		inline bool			IsInUse(T h) { return ((GetHandleState(h.Value()) & flag_hsInUse) > 0); }
		inline bool			Reset(T h) { if (DidResourceAllocationFail(h) && IsInRange(h)) { m_aState.At(h.Value()) = 0; return true; } return false; }
		inline bool			IsInRange(T h) { return (h.Value() < (U32)m_aResource.Size()); }
		bool				ReserveHandle(T h);

		inline bool			IsReservedOrInUse(T h) { return ((GetHandleState(h) & (flag_hsReservedForUse | flag_hsInUse)) > 0); }
		
		inline bool			IsNotReserved_And_IsNotInUse_OR_IsReleasePending(T h)
		{
			U32 handleState = GetHandleState(h);
			
			return (((handleState & (flag_hsReservedForUse | flag_hsInUse)) == 0) || ((handleState & flag_hsReleasePending) != 0));
		}

	protected:
		U16					GetHandleState(U16 u16) { return m_aState.At(u16); }
		bool				ReserveHandle(U16 u16);

		U16					m_u16UsedHandles;
		Btl::Vector<HandleResource, Btl::VectorParameters_Static<HandleCollectionSize> >	m_aResource;
		Btl::Vector<U8, Btl::VectorParameters_Static<HandleCollectionSize> >	m_aState;
		MT::Lock			m_Lock;
	};
}


////////////////////////////////////////////////////////////////////////////////
//	HandleCollection (constructor)
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
Audio::HandleCollection<T, N>::HandleCollection()
{
	m_u16UsedHandles = 0;
}


////////////////////////////////////////////////////////////////////////////////
//	HandleCollection (destructor)
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
Audio::HandleCollection<T, N>::~HandleCollection()
{
}


////////////////////////////////////////////////////////////////////////////////
//	InitHandleCollection
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
bool Audio::HandleCollection<T, N>::InitHandleCollection()
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	RequestUnusedHandle
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
T Audio::HandleCollection<T, N>::RequestUnusedHandle()
{
	T retvalue;

	if (m_aResource.Size() >= m_aResource.Capacity())
		return retvalue;

	m_Lock.Enter();
	U32 u32NumHandles = m_aResource.Size();
	if (m_u16UsedHandles == u32NumHandles)
	{
		retvalue = m_aResource.Size();
		HandleResource s32AddValue = 0;
		U8 u8AddValue = 0;

		MEMORY_TRACKER_PUSH("Audio.HandleCollections");
		m_aResource.PushBack(s32AddValue);
		m_aState.PushBack(u8AddValue);
		MEMORY_TRACKER_POP("Audio.HandleCollections");

		BIZ_ASSERT_MSG(m_u16UsedHandles < HandleCollectionSize, "I'll only fire once and I'm SKIPPABLE - Let Audio know!");
		ReserveHandle(retvalue);
		m_Lock.Exit();

		return retvalue;
	}

	for (U32 i = 0; i < u32NumHandles; i++)
	{
		if (GetHandleState(i) == 0)
		{
			retvalue = i;
			BIZ_ASSERT_MSG(m_u16UsedHandles < HandleCollectionSize, "I'll only fire once and I'm SKIPPABLE - Let Audio know!");
			ReserveHandle(retvalue);
			m_Lock.Exit();
			return retvalue;
		}
	}

	m_Lock.Exit();

	// return handle in invalid state
	// TODO: report error
	return retvalue;
}


////////////////////////////////////////////////////////////////////////////////
//	MapResourceToHandle
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
void Audio::HandleCollection<T, N>::MapResourceToHandle(T h, HandleResource s32Resource)
{
	m_Lock.Enter();

	// extra debugging for bug #5568
	#ifndef CFG_GOLD
	if (!h.IsValid())
		AUDIO_SYS_ERROR(DBSTR " Handle is not valid. Handle value is 0x%x", h.Value());
	#endif

	Assert(h.IsValid());

	// extra debugging for bug #5568
	#ifndef CFG_GOLD
	if (!(GetHandleState(h.Value()) & flag_hsReservedForUse))
		AUDIO_SYS_ERROR(DBSTR " Handle state is not reserved for use. Handle state is 0x%x", GetHandleState(h.Value()));
	#endif

	Assert(GetHandleState(h.Value()) & flag_hsReservedForUse);

	m_aResource.At(h.Value()) = s32Resource;

	m_aState.At(h.Value()) &= ~flag_hsReservedForUse;
	m_aState.At(h.Value()) |= flag_hsInUse;

	m_Lock.Exit();
}


////////////////////////////////////////////////////////////////////////////////
//	ReserveHandle
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
bool Audio::HandleCollection<T, N>::ReserveHandle(T h)
{
	if (! h.IsValid())
		return false;

	return ReserveHandle(h.Value());
}


template<class T, int N>
bool Audio::HandleCollection<T, N>::ReserveHandle(U16 u16)
{
	if (m_aState[u16] != 0)
		return false;

	m_aState[u16] = flag_hsReservedForUse;
	m_u16UsedHandles++;

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	FlagHandleForRelease
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
bool Audio::HandleCollection<T, N>::FlagHandleForRelease(T h)
{
	m_Lock.Enter();

	if (! h.IsValid())
	{
		m_Lock.Exit();
		return false;
	}

	if ((! IsInUse(h)) && (! IsReserved(h)))
	{
		m_Lock.Exit();
		return false;
	}

	// if already flagged, return false.. should prevent multiple release messages getting put on queue
	if (IsReleasePending(h))
	{
		m_Lock.Exit();
		return false;
	}

	m_aState.At(h.Value()) |= flag_hsReleasePending;

	m_Lock.Exit();

	return true;
}


////////////////////////////////////////////////////////////////////////////////
//	ReleaseHandle
////////////////////////////////////////////////////////////////////////////////
template<class T, int N>
bool Audio::HandleCollection<T, N>::ReleaseHandle(T h, bool bReserve)
{
	m_Lock.Enter();

	if (! h.IsValid())
	{
		m_Lock.Exit();
		return false;
	}

	if (! IsReleasePending(h))
	{
		m_Lock.Exit();
		return false;
	}

	if (! bReserve)
	{
		m_aState.At(h.Value()) = 0;
		m_u16UsedHandles--;
	}
	else
		m_aState[h.Value()] = flag_hsReservedForUse;
	
	m_Lock.Exit();

	return true;
}

//	template<class T, int N>
//	class HandleCollection
//	{
//
//	public:
//		HandleCollection();
//		~HandleCollection();
//		bool				InitHandleCollection(S32 s32ArraySize = 65534);
//		T					RequestUnusedHandle();
//		void				MapResourceToHandle(T h, S32 s32Resource);
//		bool				FlagHandleForRelease(T h);
//		bool				ReleaseHandle(T h, bool bReserve = false);
//		inline bool			IsHandleValid(T h) { return (h.IsValid()); }
//		inline bool			IsResourceRetrievable(T h) { if (IsHandleValid(h)) { return (IsInUse(h)); } return false; }
//		inline S32			GetResource(T h) { if (! IsInUse(h)) return N; return m_aResource[h.Value()]; }
//		inline S32			GetDoomedResource(T h) { if (! IsReleasePending(h)) return N; return m_aResource[h.Value()]; }
//		inline bool			SetResourceAllocationFailed(T h) { if (IsReserved(h)) { m_aState[h.Value()] = flag_hsResourceAllocationFailed; m_aResource[h.Value()] = 0; return true; } return false; }
//		inline bool			IsReserved(T h) { return ((GetHandleState(h.Value()) & flag_hsReservedForUse) > 0); }
//		inline bool			IsReleasePending(T h) { return ((GetHandleState(h.Value()) & flag_hsReleasePending) > 0); }
//		inline bool			DidResourceAllocationFail(T h) { return (GetHandleState(h.Value()) & flag_hsResourceAllocationFailed > 0); }
//		inline bool			IsInUse(T h) { return ((GetHandleState(h.Value()) & flag_hsInUse) > 0); }
//		inline bool			Reset(T h) { if (DidResourceAllocationFail(h)) { m_aState[h.Value()] = 0; return true; } return false; }
////		HandleState			GetHandleState(T h) { return GetHandleState(h.Value()); }
//		bool				ReserveHandle(T h);
//
////		static const S16	InvalidHandle;
//
//	protected:
//		U16					GetHandleState(U16 u16) { return (m_aState[u16]); }
//		bool				ReserveHandle(U16 u16);
//
//		U16					m_u16MaxHandles;
//		volatile U16		m_u16CurrentHandle;
//		U16					m_u16UnusedHandles;
//		S32*				m_aResource;
//		U8*					m_aState;
//		Audio::Core::Lock	m_Lock;
//	};
//
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	HandleCollection (constructor)
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//Audio::HandleCollection<T, N>::HandleCollection()
//{
//	m_aState = 0;
//	m_aResource = 0;
//	m_u16CurrentHandle = 0;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	HandleCollection (destructor)
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//Audio::HandleCollection<T, N>::~HandleCollection()
//{
//	if (m_aResource)
//		delete [] m_aResource;
//
//	if (m_aState)
//		delete [] m_aState;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	InitHandleCollection
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//bool Audio::HandleCollection<T, N>::InitHandleCollection(S32 s32ArraySize)
//{
//	if (s32ArraySize > 0xfffe)
//		return false;
//
//// TODO: use system mem allocator
//	m_aResource = new S32[s32ArraySize];
//
//	if (! m_aResource)
//		return false;
//
//	m_aState = new U8[s32ArraySize];
//
//	if (! m_aState)
//	{
//		delete [] m_aResource;
//		m_aResource = 0;
//		return false;
//	}
//
//	for (S32 i = 0; i < s32ArraySize; i++)
//	{
//		m_aResource[i] = 0xffffffff;
//		m_aState[i] = 0;
//	}
//
//	m_u16MaxHandles = (U16)s32ArraySize;
//	m_u16UnusedHandles  = (U16)s32ArraySize;
//
//	return true;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	RequestUnusedHandle
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//T Audio::HandleCollection<T, N>::RequestUnusedHandle()
//{
//	S32 s32ReturnHandle = 0;
//	T invalid;
//	U16 u16HandleState;
//
//	do
//	{
//		m_Lock.Acquire();
//		s32ReturnHandle = (S32)m_u16CurrentHandle;
//		m_u16CurrentHandle = (m_u16CurrentHandle + 1) % m_u16MaxHandles;
//		m_Lock.Release();
//		u16HandleState = GetHandleState(s32ReturnHandle);
//
//	} while (u16HandleState != 0 && (m_u16UnusedHandles > 0));
//
//	if (u16HandleState != 0)	// there must be no handles free!
//	{
//		// TODO: report error
//		return invalid;
//	}
//
//	if (ReserveHandle(s32ReturnHandle))
//	{
//		T retvalue;
//		retvalue = (U16)s32ReturnHandle;
//		return retvalue;
//	}
//
//	// TODO: report error
//	return invalid;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	MapResourceToHandle
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//void Audio::HandleCollection<T, N>::MapResourceToHandle(T h, S32 s32Resource)
//{
//	Assert(h.IsValid());
//
//	Assert((m_aState[h.Value()] & flag_hsReservedForUse));
//
//	m_aResource[h.Value()] = s32Resource;
//
//	m_aState[h.Value()] &= ~flag_hsReservedForUse;
//	m_aState[h.Value()] |= flag_hsInUse;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	ReserveHandle
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//bool Audio::HandleCollection<T, N>::ReserveHandle(T h)
//{
//	if (! h.IsValid())
//		return false;
//
//	return ReserveHandle(h.Value());
//}
//
//
//template<class T, int N>
//bool Audio::HandleCollection<T, N>::ReserveHandle(U16 u16)
//{
//	if (m_aState[u16] != 0)
//		return false;
//
//	m_aState[u16] = flag_hsReservedForUse;
//
//	m_u16UnusedHandles--;
//
//	return true;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	FlagHandleForRelease
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//bool Audio::HandleCollection<T, N>::FlagHandleForRelease(T h)
//{
//	if (! h.IsValid())
//		return false;
//
//	if ((! IsInUse(h)) && (! IsReserved(h)))
//		return false;
//
//	// if already flagged, return false.. should prevent multiple release messages getting put on queue
//	if (IsReleasePending(h))
//		return false;
//
//	m_aState[h.Value()] |= flag_hsReleasePending;
//
//	return true;
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	ReleaseHandle
//////////////////////////////////////////////////////////////////////////////////
//template<class T, int N>
//bool Audio::HandleCollection<T, N>::ReleaseHandle(T h, bool bReserve)
//{
//	if (! h.IsValid())
//		return false;
//
//	if (! IsReleasePending(h))
//		return false;
//
//	if (! bReserve)
//	{
//		m_aState[h.Value()] = 0;
//		m_u16UnusedHandles++;
//	}
//	else
//		m_aState[h.Value()] = flag_hsReservedForUse;
//	
//	return true;
//}

#endif	//	#ifndef _HANDLE_H_
