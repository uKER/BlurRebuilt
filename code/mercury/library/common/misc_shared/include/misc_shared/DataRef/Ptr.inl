#include <misc_shared/DataRef/Storage.h>


namespace DataRef
{

template <typename T_DataType>
inline C_Ptr<T_DataType>::C_Ptr(const I_Storage<T_DataType>* pStorage) : m_pStorage(pStorage)
{
	if (pStorage != NULL)
		pStorage->Claim();
}

template <typename T_DataType>
inline C_Ptr<T_DataType>::C_Ptr(const C_Ptr& copy) : m_pStorage(copy.m_pStorage)
{
	if (copy.m_pStorage != NULL)
		copy.m_pStorage->Claim();
}

template <typename T_DataType>
inline C_Ptr<T_DataType>& C_Ptr<T_DataType>::operator = (const C_Ptr& copy)
{
	if (copy.m_pStorage != m_pStorage)
	{
		SetNull();

		if (copy.m_pStorage != NULL)
		{
			m_pStorage = copy.m_pStorage;
			copy.m_pStorage->Claim();
		}
	}

	return *this;
}

template <typename T_DataType>
inline const T_DataType* C_Ptr<T_DataType>::GetData() const
{
	return (m_pStorage == NULL) ? NULL : m_pStorage->GetData();
}

template <typename T_DataType>
inline void C_Ptr<T_DataType>::SetNull()
{
	if (m_pStorage)
	{
		m_pStorage->Release();
		m_pStorage = NULL;
	}
}

/**
Requires a SerialiseStorage function to be declared as follows:
@code
template <class T_Stream>
void SerialisePtr(const DataRef::I_Storage<C_MyDataType>& storage, T_Stream& stream)
{
	switch (storage.GetStorageType())
	{
	case DataRef::kFactoryStorageType:	FactorySerialise(storage, stream);	break;
	case DataRef::kSorceryStorageType:	SorcerySerialise(storage, stream);	break;

	default:
		assert(false); // don't know how to serialise this type
		LOGERROR(Debug::GetRootLogger(), "Failed to serialise DataRef... ask a programmer to investigate");
		break;
	}
}
@endcode

@param stream receives data.
*/
template <typename T_DataType>
template <class T_Stream>
void C_Ptr<T_DataType>::Serialise(T_Stream& stream) const
{
	if (m_pStorage == NULL)
		stream << kNullStorageType;
	else
	{
		stream << m_pStorage->GetStorageType();
		SerialiseStorage(*m_pStorage, stream);
	}
}

/**
Requires a DeserialisePtr function to be declared as follows:
@code
template <class T_Stream>
void DeserialisePtr(const C_StorageTypeId storageType, C_Ptr<C_MyDataType>& ptr, T_Stream& stream)
{
	switch (storageType)
	{
	case kFactoryStorageType:	FactoryDeserialise(ptr, stream);	break;
	case kSorceryStorageType:	SorceryDeserialise(ptr, stream);	break;

	default:
		assert(false); // don't know how to deserialise this type
		ptr.SetNull();
		LOGERROR(Debug::GetRootLogger(), "Failed to deserialise DataRef... ask a programmer to investigate");
		break;
	}
}
@endcode

@param stream provides data.
*/
template <typename T_DataType>
template <class T_Stream>
void C_Ptr<T_DataType>::Deserialise(T_Stream& stream)
{
	C_StorageTypeId storageType = 0;
	stream >> storageType;
	
	if (storageType == kNullStorageType)
		SetNull();
	else
		DeserialisePtr(storageType, *this, stream);
}

template <typename T_DataType>
C_StorageTypeId C_Ptr<T_DataType>::GetStorageType() const
{
	return (m_pStorage != NULL) ? m_pStorage->GetStorageType() : kNullStorageType;
}

} // namespace DataRef
