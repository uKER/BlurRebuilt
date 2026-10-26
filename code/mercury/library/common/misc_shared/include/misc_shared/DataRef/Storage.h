#ifndef STORAGE_B8016557_34DE_463C_9591_8BA5B80ADDB0_H
#define STORAGE_B8016557_34DE_463C_9591_8BA5B80ADDB0_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/DataRef/DataRefDeclare.h>


namespace DataRef
{

/**
@brief Interface for externally reference counted data storage.

This is an abstraction used by C_Ptr to refer to data from arbitraty storage
sources. C_Ptr is responsible for calling Claim() and Release() at the
correct times.
*/
template <typename T_DataType> 
class I_Storage
{
public:
	/// @param pData stored data.
	I_Storage(const T_DataType* pData) : m_pData(pData), m_RefCount(0) {}

	/// @returns stored data.
	inline const T_DataType* GetData() const {return m_pData;}

	/// Increase the reference count.
	inline void Claim() const {++m_RefCount;}

	/// Decrease the reference count.
	inline void Release() const
	{
		if ((--m_RefCount) == 0) 
			NoMoreRefs();
	}

	/// @returns UID for the storage type.
	virtual C_StorageTypeId GetStorageType() const = 0;

protected:
	const T_DataType*	m_pData;		///< stored data
	mutable U32			m_RefCount;		///< reference count

	virtual ~I_Storage() {assert(m_RefCount == 0);}

private:
	I_Storage(const I_Storage&);				///< can not be copied
	I_Storage& operator = (const I_Storage&);	///< can not be copied

	/// Called when the reference count decreases to zero.
	virtual void NoMoreRefs() const = 0;

};

} // namespace DataRef


#endif // STORAGE_B8016557_34DE_463C_9591_8BA5B80ADDB0_H
