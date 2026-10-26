#ifndef FACTORY_13EADECB_9C1D_4FEB_B3B6_52E8C31DA169_H
#define FACTORY_13EADECB_9C1D_4FEB_B3B6_52E8C31DA169_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/DataRef/DataRefDeclare.h>
#include <misc_shared/UID/FourCC.h>


namespace DataRef
{

const C_StorageTypeId kFactoryStorageType = FOURCC_U32('FACT');

/// Creates a C_Ptr from a polymorphic factory.
template <class T_Factory>
C_Ptr<typename T_Factory::C_DataType> FactoryCreate(const typename T_Factory::C_TypeId typeId, typename T_Factory::C_DataType** ppData = NULL);

/// Serialisation for polymorphic factory data.
template <class T_Factory, class T_Stream> 
void FactorySerialise(const I_Storage<typename T_Factory::C_DataType>& storage, T_Stream& stream);

/// Deserialisation for polymorphic factory data.
template <class T_Factory, class T_Stream> 
void FactoryDeserialise(C_Ptr<typename T_Factory::C_DataType>& ptr, T_Stream& stream);

} // namespace DataRef


#include <misc_shared/DataRef/Factory.inl>

#endif // FACTORY_13EADECB_9C1D_4FEB_B3B6_52E8C31DA169_H
