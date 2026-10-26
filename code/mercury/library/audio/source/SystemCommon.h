#ifndef _SYSTEM_COMMON_H_
#define _SYSTEM_COMMON_H_


namespace Audio
{
	struct SystemState
	{
        U32 m_uFrameNum;

		//	speakers
		U32 m_uNumSpeakers;
		F32 m_fSpeakerDistance;

		bool m_bHasCentre;
		bool m_SpeakerAngleHasChanged;

		MixerComponent::PanningMode m_ePanningMode;

		F32 m_afAngles[ MAX_NUM_SURROUND_SPEAKERS + 1 ];
	};

#if defined(PLATFORM_PS3) || defined(PLATFORM_SPU)
	struct SystemSpuInfo
	{
		SystemState state;
		ISystem::Info debugInfo;
	};
#endif

	void GetInfoCommon(void* p, bool bAverage);
};

#endif	//	_SYSTEM_COMMON_H_
