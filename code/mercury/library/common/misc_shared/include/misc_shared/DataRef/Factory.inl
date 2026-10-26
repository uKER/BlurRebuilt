#include <btl/allocators/pool.h>
#include <misc_shared/DataRef/Ptr.h>
#include <misc_shared/DataRef/Storage.h>


namespace DataRef
{

/**
@brief Implements I_Storage for factory created data.
This is an internal implementation detail of the FactoryCreate() functions.
*/
template <class T_Factory>
class C_StorageFactory : public I_Storage<typename T_Factory::C_DataType>
{
public:
	C_StorageFactory() : I_Storage<typename T_Factory::C_DataType>(NULL) {}
	
	virtual ~C_StorageFactory() {}

	// from I_Storage
	virtual C_StorageTypeId GetStorageType() const {return kFactoryStorageType;}

	/// allocate from pool
	/// @param pData data to store.
	/// @returns storage for this data.
	static C_StorageFactory* Create(const typename T_Factory::C_DataType* pData)
	{
		C_StorageFactory* pStorage = s_Pool.Allocate();

		if (pStorage != NULL)
		{
			pStorage->m_pData = pData;
		}

		return pStorage;
	}

private:
	static Btl::DynamicPool<C_StorageFactory> s_Pool; ///< allocation pool

	// from I_Storage
	virtual void NoMoreRefs() const
	{
		T_Factory::Destroy(I_Storage<typename T_Factory::C_DataType>::m_pData);
		s_Pool.Free(const_cast<C_StorageFactory*>(this));
	}

};

template <class T_Factory>
Btl::DynamicPool< C_StorageFactory<T_Factory> > C_StorageFactory<T_Factory>::s_Pool;


/**
A polymorphic factory creates objects of a specified type through this interface:
@code
class T_Factory
{
public:
	typename C_DataType;
	typename C_TypeId;
	static C_DataType* Create(const C_TypeId);
	static void Destroy(const C_DataType*);
	static C_TypeId GetTypeOf(const C_DataType&);
};
@endcode

@param typeId type of object to create.
@param ppData if non-NULL receives the non-const pointer to the created object.
@returns C_Ptr for the created object.
*/
template <class T_Factory>
C_Ptr<typename T_Factory::C_DataType> FactoryCreate(const typename T_Factory::C_TypeId typeId, typename T_Factory::C_DataType** ppData)
{
	typename T_Factory::C_DataType* pData = T_Factory::Create(typeId);

	if (ppData != NULL)
		*ppData = pData;

	return C_Ptr<typename T_Factory::C_DataType>(C_StorageFactory<T_Factory>::Create(pData));
}

/**
@param storage data to serialise.
@param stream receives data.
*/
template <class T_Factory, class T_Stream> 
void FactorySerialise(const I_Storage<typename T_Factory::C_DataType>& storage, T_Stream& stream)
{
	BIZ_ASSERT(storage.GetStorageType() == kFactoryStorageType);
	BIZ_ASSERT(storage.GetData() != NULL);

	typedef typename T_Factory::C_StreamContext C_StreamContext;
	const C_StreamContext* pStreamContext = static_cast<const C_StreamContext*>(stream.GetContextItem(C_StreamContext::ClassUid()));

	if (pStreamContext != NULL)
	{
		stream << pStreamContext->GetIndex(storage.GetData());
	}
	else
	{
		stream << T_Factory::GetTypeOf(*storage.GetData()) << *storage.GetData();
	}
}

/**
@param ptr receives the deserialised value.
@param typeId type of object to deserialise.
@param stream provides the value.
@see FactoryCreate DeserialisePtr.
*/
template <class T_Factory, class T_Stream> 
void FactoryDeserialise(C_Ptr<typename T_Factory::C_DataType>& ptr, T_Stream& stream)
{
	typedef typename T_Factory::C_StreamContext C_StreamContext;
	const C_StreamContext* pStreamContext = static_cast<const C_StreamContext*>(stream.GetContextItem(C_StreamContext::ClassUid()));

	if (pStreamContext != NULL)
	{
		U32 index = 0;
		stream >> index;
		ptr = pStreamContext->Lookup(index);
	}
	else
	{
		typename T_Factory::C_TypeId typeId = 0;
		stream >> typeId;

		typename T_Factory::C_DataType* pData = T_Factory::Create(typeId);
		BIZ_ASSERT(pData != NULL);

		stream >> *pData;
		ptr = C_Ptr<typename T_Factory::C_DataType>(C_StorageFactory<T_Factory>::Create(pData));
	}
}

} // namespace DataRef
