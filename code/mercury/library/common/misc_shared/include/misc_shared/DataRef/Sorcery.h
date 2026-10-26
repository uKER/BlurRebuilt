#ifndef SORCERY_0E86D3E1_3BC0_42B9_BCEC_1EFC51AEC88B_H
#define SORCERY_0E86D3E1_3BC0_42B9_BCEC_1EFC51AEC88B_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/DataRef/DataRefDeclare.h>
#include <misc_shared/UID/FourCC.h>
#include <Xt/XtGame.h>


namespace DataRef
{

const C_StorageTypeId kSorceryStorageType = FOURCC_U32('SORC');

typedef U32 C_SorceryUid; ///< persistent unique identifier

/// Creates a C_Ptr for sorcery data.
template <class T_DataType>
C_Ptr<T_DataType> SorceryLookup(const T_DataType* pData);

/// Callback function for Xt annotations.
template <class T_DataType, class T_RecordType>
void SorceryCallback(XtEventType eventType, XtRecord* pRecord, T_RecordType* pData, XtCallbackContext* pContext);

/// Serialisation for sorcery data.
template <typename T_DataType, class T_Stream> 
void SorcerySerialise(const I_Storage<T_DataType>& storage, T_Stream& stream);

/// Deserialisation for sorcery data.
template <typename T_DataType, class T_Stream> 
void SorceryDeserialise(C_Ptr<T_DataType>& ptr, T_Stream& stream);

} // namespace DataRef


#include <misc_shared/DataRef/Sorcery.inl>

#endif // SORCERY_0E86D3E1_3BC0_42B9_BCEC_1EFC51AEC88B_H
