#ifndef _COMPONENT_HELPER_H_
#define _COMPONENT_HELPER_H_

#ifdef AUDIO_STRING_HASH_COMPONENTS
#include "StringHash.h"
//#include "..\Object.h"
#endif

#if !defined (PLATFORM_SPU)
namespace Audio
{
	template <typename T>
	T* Component::PreInit(void* pData)
	{
		m_uInitOffset = (U8*)pData - (U8*)m_pObject->GetInitData();
		T* pInitData = (T*)m_pObject->GetInitData(m_uInitOffset);

		m_pchName = m_pObject->GetString(pInitData->strName);

		#ifdef AUDIO_STRING_HASH_COMPONENTS 
		m_nameHash = StringHash(m_pchName);

		if (gbAudioLogComponentHashValues)
			AUDIO_SYS_DEBUG("Hashing %s->%s as 0x%x\n", m_pObject->Name(), m_pchName, m_nameHash.GetValue());
		#endif	// #ifdef AUDIO_STRING_HASH_COMPONENTS

		return (T*)(pInitData);
	}
}	// namespace Audio
#endif // #if !defined (PLATFORM_SPU)
#endif // _COMPONENT_H_


