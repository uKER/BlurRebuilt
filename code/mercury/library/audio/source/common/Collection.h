#ifndef _COLLECTION_H_
#define _COLLECTION_H_

#define MAX_NUM_COLLECTION_PAGES	255
#define ELEMENTS_PER_PAGE			255


//namespace Audio
//{
//	template <class T>
//	class Collection
//	{
//
//	public:
//		Collection();
//		~Collection();
//
//		U16			Allocate();
//		bool		Release(U16 u16Id);
//		bool		DoesItemExist(U16 u16Id);
//		T*			GetItem(U16 u16Id);
//		inline T*	GetItemFast(U16 u16Id)				{ return (T*)&((m_apCollectionPage[GetPageIndex(u16Id)])[(GetElementIndex(u16Id) * m_u32ElementSize + 15) & 0xfffffff0]); }
//
//	private:
//		bool		AllocatePage(U16* pu16PageToUse);
//		bool		ReleasePage(U8 u8PageIndex);
//		U16			GetFreeElementInPage(U16 u16Page);
//
//		inline U8	GetPageIndex(U16 u16Id)				{ return (U8)(u16Id >> 8); }
//		inline U8	GetElementIndex(U16 u16Id)			{ return (U8)(u16Id & 0xff); }
//		inline U16	GetHandle(U8 u8Page, U8 u8Element)	{ return ((U16)u8Page << 8) + u8Element; }
//		inline bool	DoesPageExist(U16 u16Page)			{ return (u16Page < ELEMENTS_PER_PAGE && m_apCollectionPage[u16Page]); }
//
//		U8*			m_apCollectionPage[MAX_NUM_COLLECTION_PAGES];
//		U8			m_au8ElementsFree[MAX_NUM_COLLECTION_PAGES];
//		U32			m_aElementUsageBitArray[MAX_NUM_COLLECTION_PAGES * 8];
//		U32			m_u32NumPagesAllocated;
//		U32			m_u32ElementSize;
//	};
//};
//
//template <class T>
//Audio::Collection<T>::Collection()
//{
//	m_u32NumPagesAllocated = 0;
//
//	for (U32 i = 0; i < MAX_NUM_COLLECTION_PAGES; i++)
//	{
//		m_apCollectionPage[i] = 0;
//		m_au8ElementsFree[i] = ELEMENTS_PER_PAGE;
//	}
//
//	// align elements to 16 bytes. TODO: have variable alignment size
//	m_u32ElementSize = (sizeof(T) + 15) & 0xfffffff0;
//
//	memset(m_aElementUsageBitArray, 0, sizeof(U32) * 8 * MAX_NUM_COLLECTION_PAGES);
//}
//
//
//template <class T>
//Audio::Collection<T>::~Collection()
//{
//	for (U32 i = 0; i < MAX_NUM_COLLECTION_PAGES; i++)
//	{
//		if (m_apCollectionPage[i])
//			delete [] m_apCollectionPage[i];
//	}
//}
//
//
//template <class T>
//U16 Audio::Collection<T>::Allocate()
//{
//	U16 u16PageToUse = 0xffff;
//
//	if (! m_u32NumPagesAllocated)
//	{
//		if (! AllocatePage(&u16PageToUse))
//			return 0xffff;
//	}
//	else
//	{
//		for (U32 i = 0; i < MAX_NUM_COLLECTION_PAGES && (u16PageToUse == 0xffff); i++)
//		{
//			if (m_au8ElementsFree[i])
//				u16PageToUse = i;
//		}
//	}
//
//	if (u16PageToUse == 0xffff)
//		return 0xffff;
//
//	U16 u16ElementIndex = GetFreeElementInPage(u16PageToUse);
//
//	if (u16ElementIndex == 0xffff)
//		return 0xffff;
//
//	return GetHandle((U8)u16PageToUse, (U8)u16ElementIndex);
//}
//
//
//template <class T>
//bool Audio::Collection<T>::Release(U16 u16Id)
//{
//	U8 u8Page = GetPageIndex(u16Id);
//	U8 u8Element = GetElementIndex(u16Id);
//
//	if (! m_apCollectionPage[u8Page])
//		return false;
//
//	if (! (m_aElementUsageBitArray[u8Page * 8 + (u8Element / 32)] & 1 << (u8Element % 32)))
//		return false;
//
//	m_aElementUsageBitArray[u8Page * 8 + (u8Element / 32)] &= ~(1 << (u8Element % 32));
//
//	m_au8ElementsFree[u8Page]++;
//
//	if (m_au8ElementsFree[u8Page] == ELEMENTS_PER_PAGE)
//		ReleasePage(u8Page);
//
//	return true;
//}
//
//
//template <class T>
//bool Audio::Collection<T>::DoesItemExist(U16 u16Id)
//{
//	U8 u8PageIndex = GetPageIndex(u16Id);
//	U8 u8ElementIndex = GetElementIndex(u16Id);
//
//	if (! DoesPageExist(u8PageIndex))
//		return false;
//
//	return ((m_aElementUsageBitArray[(u8PageIndex * 8) + (u8ElementIndex / 32)] & (1 << (u8ElementIndex % 32))) != 0);
//}
//
//
//template <class T>
//T* Audio::Collection<T>::GetItem(U16 u16Id)
//{
//	if (! DoesItemExist(u16Id))
//		return 0;
//
//	return GetItemFast(u16Id);
//}
//
//
//template <class T>
//bool Audio::Collection<T>::AllocatePage(U16* pu16PageToUse)
//{
//	if (m_u32NumPagesAllocated == MAX_NUM_COLLECTION_PAGES)
//		return false;
// 
//	for (U32 i = 0; i < MAX_NUM_COLLECTION_PAGES; i++)
//	{
//		// TODO: could use bit array for page lookup, as well as element lookup
//
//		if (! m_apCollectionPage[i])
//		{
//			m_apCollectionPage[i] = new U8[(m_u32ElementSize * ELEMENTS_PER_PAGE + 15) & 0xfffffff0];	// TODO: use standard audio system allocator
//			bool bSuccess = (m_apCollectionPage[i] != 0);
//
//			if (bSuccess)
//			{
//				m_u32NumPagesAllocated++;
//				if (pu16PageToUse)
//					*(pu16PageToUse) = (U16)i;
//			}
//
//			return bSuccess;
//		}
//	}
//
//	return false;
//}
//
//
//template <class T>
//bool Audio::Collection<T>::ReleasePage(U8 u8PageIndex)
//{
//	if (! m_apCollectionPage[u8PageIndex])
//		return false;
//
//	delete [] m_apCollectionPage[u8PageIndex];	// TODO: use standard audio system allocator
//	m_apCollectionPage[u8PageIndex] = 0;
//	m_u32NumPagesAllocated--;
//
//	return true;
//}
//
//
//template <class T>
//U16 Audio::Collection<T>::GetFreeElementInPage(U16 u16Page)
//{
//	if ((! m_apCollectionPage[u16Page]) || (! m_au8ElementsFree[u16Page]))
//		return 0xffff;
//
//	U32* pu32PageBitArray = &(m_aElementUsageBitArray[u16Page * 8]);
//
//	for (U32 i = 0; i < 8; i++)
//	{
//		if (pu32PageBitArray[i] < 0xffffffff)
//		{
//			for (U32 i2 = 0; i2 < 32; i2++)
//			{
//				if (! (pu32PageBitArray[i] & (1 << i2)))
//				{
//					pu32PageBitArray[i] |= 1 << i2;
//					m_au8ElementsFree[u16Page]--;
//					return i * 8 + i2;
//				}	
//			}
//		}
//	}
//
//	return 0xffff;
//}

#endif	//	_COLLECTION_H_
