#include <btl/allocators/pool.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/DataRef/Storage.h>


namespace DataRef
{

/**
@brief Implements I_Storage for sorcery data.
This is an internal implementation detail of the SorceryLookup() functions.
*/
template <class T_DataType>
class C_StorageSorcery : public I_Storage<T_DataType>
{
public:
	typedef Btl::Map<C_SorceryUid, C_StorageSorcery> C_Map;
	
	/// @param pData sorcery data.
	C_StorageSorcery() : I_Storage<T_DataType>(NULL), m_SorceryUid(0), m_pHashTableNext(NULL) {}

	virtual ~C_StorageSorcery() {}

	inline C_SorceryUid GetSorceryUid() const {return m_SorceryUid;}

	// from I_Storage
	virtual C_StorageTypeId GetStorageType() const {return kSorceryStorageType;}

	/// @param pData sorcery data.
	inline void SetData(const T_DataType* pData) {I_Storage<T_DataType>::m_pData = pData;}

	/// @param pData sorcery data.
	inline static void OnConstruct(const T_DataType* pData)
	{
		BIZ_ASSERT(pData);
		const C_SorceryUid uid = pData->GetSorceryUid();

		if (uid == 0)
		{
			LOGERROR(Debug::GetRootLogger(), DBSTR "Zero UID in sorcery database, try updating Sorcery and ActorStudio to latest and re-exporting both");
		}
		else
		{
			C_StorageSorcery** ppLink = NULL;
			C_StorageSorcery* pStorage = Find(uid, &ppLink);

			if (pStorage == NULL)
			{
				BIZ_ASSERT(ppLink != NULL);
				BIZ_ASSERT(*ppLink == NULL);

				pStorage = s_Pool.Allocate();
				pStorage->SetData(pData);
				pStorage->m_SorceryUid = uid;
				pStorage->m_pHashTableNext = NULL;
				
				*ppLink = pStorage;
			}
			else if (pStorage->GetData() == NULL)
			{
				// replace the underlying data
				BIZ_ASSERT(pStorage->m_RefCount > 0);
				pStorage->SetData(pData);
			}
			else
			{
				LOGERROR(Debug::GetRootLogger(), DBSTR "Duplicate UID(0x%x) in sorcery database, try updating Sorcery and ActorStudio to latest and re-exporting both", uid);
			}
		}
	}

	/// @param pData sorcery data.
	inline static void OnDestruct(const T_DataType* pData)
	{
		const C_SorceryUid uid = pData->GetSorceryUid();

		if (uid == 0)
		{
			LOGERROR(Debug::GetRootLogger(), DBSTR "Zero UID in sorcery database, try updating Sorcery and ActorStudio to latest and re-exporting both");
		}
		else
		{
			C_StorageSorcery** ppLink = NULL;
			C_StorageSorcery* pStorage = Find(uid, &ppLink);

			if (pStorage == NULL)
			{
				// [Oscar]: 
				// are you *absolutely* sure that you're not getting any related output in DebugView? 
				// no other asserts before you hit this? 
				// you've tried updating and exporting Sorcery and ActorStudio? 
				// ok, in that case you can come and ask me if you want help debugging it :-)
				BIZ_ASSERT_FAIL("out-of-sync UID(0x%x) in sorcery database", uid);
			}
			else if (pStorage->GetData() != pData)
			{
				LOGERROR(Debug::GetRootLogger(), DBSTR "Duplicate UID(0x%x) in sorcery database, try updating Sorcery and ActorStudio to latest and re-exporting both", uid);
			}
			else
			{
				// clear the pointer
				pStorage->SetData(NULL);

				// if there are no more references then discard the uid
				if (pStorage->m_RefCount == 0)
				{
					BIZ_ASSERT(ppLink != NULL);
					BIZ_ASSERT(*ppLink == pStorage);
					*ppLink = pStorage->m_pHashTableNext;
					s_Pool.Free(pStorage);
				}
			}
		}
	}

	/// @param uid sorcery id.
	/// @returns storage object, or NULL if not found.
	inline static C_StorageSorcery* Lookup(const C_SorceryUid uid)
	{
		return Find(uid);
	}

private:
	C_SorceryUid		m_SorceryUid;		///< uid for the sorcery data
	C_StorageSorcery*	m_pHashTableNext;	///< next entry in hash table

	static const U32							kHashTableSize = (1 << 10); // must be power-of-two
	static const U32							kHashTableMask = kHashTableSize - 1;
	static C_StorageSorcery*					s_pHashTable [kHashTableSize];
	static Btl::DynamicPool<C_StorageSorcery>	s_Pool;

	/// find this item in the hash table
	static C_StorageSorcery* Find(const C_SorceryUid uid, C_StorageSorcery*** pppLink = NULL)
	{
		const U32 hashKey = (uid & kHashTableMask);
		BIZ_ASSERT(hashKey < kHashTableSize);

		C_StorageSorcery** ppStorage = &s_pHashTable[hashKey];

		while (*ppStorage != NULL)
		{
			if ((*ppStorage)->m_SorceryUid == uid)
			{
				break;
			}

			ppStorage = &(*ppStorage)->m_pHashTableNext;
		}

		if (pppLink != NULL)
		{
			*pppLink = ppStorage;
		}

		return *ppStorage;
	}

	// from I_Storage
	virtual void NoMoreRefs() const
	{
		if (I_Storage<T_DataType>::m_pData == NULL)
		{
			C_StorageSorcery** ppLink = NULL;
			C_StorageSorcery* pStorage = Find(m_SorceryUid, &ppLink);

			if (pStorage == this)
			{
				BIZ_ASSERT(ppLink != NULL);
				BIZ_ASSERT(*ppLink == pStorage);
				*ppLink = pStorage->m_pHashTableNext;
				s_Pool.Free(pStorage);
			}
			else
			{
				BIZ_ASSERT((pStorage == NULL) || (pStorage->m_SorceryUid == m_SorceryUid));
				BIZ_ASSERT_FAIL("C_StorageSorcery wasn't found in hash table");
			}
		}
	}

};

template <class T_DataType>
C_StorageSorcery<T_DataType>* C_StorageSorcery<T_DataType>::s_pHashTable [C_StorageSorcery<T_DataType>::kHashTableSize] = {NULL};

template <class T_DataType>
Btl::DynamicPool< C_StorageSorcery<T_DataType> > C_StorageSorcery<T_DataType>::s_Pool;


/**
Looks in the data registered by SorceryCallback() for a matching uid.
@param pData sorcery data.
@returns C_Ptr to sorcery data.
*/
template <class T_DataType>
C_Ptr<T_DataType> SorceryLookup(const T_DataType* pData)
{
	C_Ptr<T_DataType> ptr;

	if (pData != NULL)
	{
		const C_SorceryUid uid = pData->GetSorceryUid();

		if (uid != 0)
			ptr = C_Ptr<T_DataType>(C_StorageSorcery<T_DataType>::Lookup(uid));
	}

	return ptr;
}

/**
The function must be called every time an XtEventType is issued for T_DataType
objects.

T_RecordType must supply the following:
@code
class T_RecordType
{
public:
	void DoSorceryCallback(void (*pCallbackFn)(const T_DataType*));
};
@endcode

If T_RecordType already registers it's own callback function then that should
be modified to forward the callback on to SorceryCallback() in addition to it's
current duties. Otherwise, the Xt annotation for T_RecordType should be modified
with XT_CALLBACK(DataRef::SorceryCallback<T_RecordType>).

@param eventType standard Xt callback parameter.
@param pRecord standard Xt callback parameter.
@param pData standard Xt callback parameter.
@param pContext standard Xt callback parameter.
*/
template <class T_DataType, class T_RecordType>
void SorceryCallback(XtEventType eventType, XtRecord* pRecord, T_RecordType* pData, XtCallbackContext* pContext)
{
	switch (eventType)
	{
	case kXtEtConstruct:	pData->DoSorceryCallback(&C_StorageSorcery<T_DataType>::OnConstruct);	break;
	case kXtEtDestruct:		pData->DoSorceryCallback(&C_StorageSorcery<T_DataType>::OnDestruct);	break;
	}
}

/**
@param storage data to serialise.
@param stream receives data.
*/
template <typename T_DataType, class T_Stream> 
void SorcerySerialise(const I_Storage<T_DataType>& storage, T_Stream& stream)
{
	BIZ_ASSERT(storage.GetStorageType() == kSorceryStorageType);
	stream << static_cast<const C_StorageSorcery<T_DataType>*>(&storage)->GetSorceryUid();
}

/**
The data type must support the following interface:
@code
class T_DataType
{
public:
	C_SorceryUid GetSorceryUid() const;
	void Serialise(T_Stream&) const;
	void Deserialise(T_Stream&);
};
@endcode

@param ptr receives deserialised ptr.
@param stream provides data.
@see DeserialisePtr.
*/
template <typename T_DataType, class T_Stream> 
void SorceryDeserialise(C_Ptr<T_DataType>& ptr, T_Stream& stream)
{
	C_SorceryUid uid = 0;
	stream >> uid;
	ptr = C_Ptr<T_DataType>(C_StorageSorcery<T_DataType>::Lookup(uid));
}

} // namespace DataRef
