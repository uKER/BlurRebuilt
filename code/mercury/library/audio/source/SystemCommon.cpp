#include "AudioPch.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//	GetInfoCommon
////////////////////////////////////////////////////////////////////////////////
void Audio::GetInfoCommon(void* p, bool bAverage)
{
	Audio::ISystem::Info* pInfo = (Audio::ISystem::Info*)p;

#ifndef CFG_GOLD
	pInfo->uTotalWaves = Core::Generator::TotalWave();
	pInfo->uActiveWaves = Core::Generator::ActiveWave();
	pInfo->uTotalMixers = Core::Generator::TotalMix();
	pInfo->uActiveMixers = Core::Generator::ActiveMix();
	pInfo->uTotalGenerators = pInfo->uTotalWaves + pInfo->uTotalMixers;
	pInfo->uActiveGenerators = pInfo->uActiveWaves + pInfo->uActiveMixers;

	pInfo->uTotalPCMDecoders = Core::PCMS16Decoder::Total();
	pInfo->uActivePCMDecoders = Core::PCMS16Decoder::Active();
	pInfo->uTotalXMADecoders = 0;
	pInfo->uActiveXMADecoders = 0;
#else // CFG_GOLD
	pInfo->uTotalWaves = 0;
	pInfo->uActiveWaves = 0;
	pInfo->uTotalMixers = 0;
	pInfo->uActiveMixers = 0;
	pInfo->uTotalGenerators = 0;
	pInfo->uActiveGenerators = 0;

	pInfo->uTotalPCMDecoders = 0;
	pInfo->uActivePCMDecoders = 0;
	pInfo->uTotalXMADecoders = 0;
	pInfo->uActiveXMADecoders = 0;
	pInfo->uTotalXMADecoders = 0;
	pInfo->uActiveXMADecoders = 0;
#endif // CFG_GOLD

#ifndef PLATFORM_SPU
	pInfo->uCurrentMessageQueueBytes = Audio::System::Get()->GetQueue()->QueuedBytes();
	pInfo->uPeakMessageQueueBytes = Audio::System::Get()->GetQueue()->PeakQueuedBytes();

	pInfo->uNumObjectDefinitions = Audio::System::Get()->GetDefinitionManager()->Count();
#endif

	pInfo->uTotalADPCMDecoders = Core::ADPCM4bitDecoder::Total();
	pInfo->uActiveADPCMDecoders = Core::ADPCM4bitDecoder::Active();
}


#if defined (PLATFORM_PS3) || defined (PLATFORM_SPU)
void Audio::CopyInfoFromSpuStruct(ISystem::Info* pInfo)
{
#ifndef CFG_GOLD
	Core::Generator::m_uActiveWave = pInfo->uActiveWaves;
	Core::Generator::m_uTotalWave = pInfo->uTotalWaves;
	Core::Generator::m_uActiveMix = pInfo->uActiveMixers;
	Core::Generator::m_uTotalMix = pInfo->uTotalMixers;
#endif

	Core::ADPCM4bitDecoder::SetTotal(pInfo->uTotalADPCMDecoders);
	Core::ADPCM4bitDecoder::SetActive(pInfo->uActiveADPCMDecoders);
}
#endif
