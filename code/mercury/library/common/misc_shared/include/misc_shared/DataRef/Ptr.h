#ifndef PTR_CD19CE69_5B05_457B_9957_2E5546BAF971_H
#define PTR_CD19CE69_5B05_457B_9957_2E5546BAF971_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/DataRef/DataRefDeclare.h>


namespace DataRef
{

/**
@brief Smart pointer for data from arbitrary sources.

Provides pointer semantics for data accessed through the I_Storage interface.
Users should never handle I_Storage objects directly, instead they should use
services that return C_Ptr values.
*/
template <typename T_DataType>
class C_Ptr
{
public:
	inline explicit C_Ptr(const I_Storage<T_DataType>* pStorage = NULL);
	inline C_Ptr(const C_Ptr& copy);
	inline ~C_Ptr() {SetNull();}
	inline C_Ptr& operator = (const C_Ptr& copy);

	inline bool IsNull() const {return (GetData() == NULL);}
	inline void SetNull();

	inline const T_DataType* GetData() const;

	inline const T_DataType* operator -> () const {return GetData();} 
	inline const T_DataType& operator * () const {return *GetData();}

	template <class T_Stream> void Serialise(T_Stream& stream) const;
	template <class T_Stream> void Deserialise(T_Stream& stream);

	inline C_StorageTypeId GetStorageType() const;

private:
	const I_Storage<T_DataType>*	m_pStorage;	///< interface for stored data

};

} // namespace DataRef


#include <misc_shared/DataRef/Ptr.inl>

#endif // PTR_CD19CE69_5B05_457B_9957_2E5546BAF971_H
