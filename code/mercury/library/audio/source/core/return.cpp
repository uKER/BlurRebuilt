#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#endif

////////////////////////////////////////////////////////////////////////////////
//	ReturnSize (static)
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
U32 Audio::Core::Return::ReturnSize( U16 uChannelsOut, U16 uEffects )
{
	U32 uSize = sizeof( Return );

	//	effects
	uSize += sizeof( DSP::Element ) * uEffects;

	return AUDIO_CORE_ALIGN_16( uSize );
}

////////////////////////////////////////////////////////////////////////////////
//	Return
////////////////////////////////////////////////////////////////////////////////
Audio::Core::Return::Return( const char* pchInfo, U32 u32NumChannels, U16 uEffects ) : Generator( pchInfo, u32NumChannels, false )
{
	m_uClassSize = sizeof( Return );

	m_uEffects = uEffects;

	U8* pEnd = (U8*)this + m_uClassSize;
	DSP::Element* aEffects = (DSP::Element*)pEnd;

	Generator::ResetEffects( aEffects, m_uEffects );

	Generator::m_eType = eReturn;

	m_bMuted = false;
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::Return::SetProcessFlag( const ProcessInfo& info, bool bProcess )
{
	return m_bProcessing;
}

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Return::Process( const ProcessInfo& info )
{
      AUDIO_PERF_MICRO_EVENT("Audio.Generate.Return.Process");

      if (Generator::m_uBuffer != (U32)-1 )
      {
            F32* pFrameBuffer = (F32*)&info.pStaticBuffers[ Generator::m_uBuffer ];

            Generator::m_bProcessing = false;
      }
      else
            AUDIO_CORE_WARN( "Return '%s' not connected to audio graph", m_pchInfo );

      m_uFrameCount = info.uFrameNum;
}



////////////////////////////////////////////////////////////////////////////////
//	DumpStats
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::Return::DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals)
{
    if (pInternals)
    {
        pInternals->uTotalNumGenerators++;

        if (Generator::m_bProcessing)
            pInternals->uNumGeneratorsActive++;
    }

	F32* pFrameData = (F32*)&info.pDynamicBuffers[Generator::m_uBuffer];

    if (Generator::m_pchInfo)
        AUDIO_SYS_DEBUG("%s[Return %dch] \"%s\" <%s> F:0x%.8x (%.2f)", pchPrefix, m_u32NumChannels, Generator::m_pchInfo, Generator::m_bProcessing ? "active" : "inactive", pFrameData, *pFrameData);
    else
        AUDIO_SYS_DEBUG("%s[Return %dch] <%s> F:0x%.8x (%.2f)", pchPrefix, m_u32NumChannels, Generator::m_bProcessing ? "active" : "inactive", pFrameData, *pFrameData);
}
