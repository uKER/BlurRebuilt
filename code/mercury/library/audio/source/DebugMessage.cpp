#include "AudioPch.h"
#include "SystemHeaders.h"

//using namespace Audio;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


bool Audio::DebugMessage::SetMessageData(void* pData, U32 u32Size)
{
	return false;
}


bool Audio::DebugMessage::Despatch()
{
	return false;
}


void Audio::DebugMessage::ReceiveData(void* pData, U32 uSize)
{
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
