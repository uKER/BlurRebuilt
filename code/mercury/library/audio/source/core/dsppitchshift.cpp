#include "AudioPch.h"
#include "coreheaders.h"



#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::PitchShift::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	PitchShift
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::PitchShift::PitchShift(const char* pchInfo,U32 u32NumChannels) : Effect(pchInfo,u32NumChannels)
{
#ifndef AUDIO_DISABLE_DSP

	m_eType = Core::DSP::Effect::ePitchShift;

	//setup stuff

// temp, to get compiling.. jkc
//#ifdef PLATFORM_XENON
//	stSoundTouch.setSampleRate(XAUDIOSAMPLERATE_NATIVE);
//#else
	stSoundTouch.setSampleRate(48000);
//#endif
    stSoundTouch.setChannels(2);

    stSoundTouch.setTempoChange(0);//params->tempoDelta
    stSoundTouch.setPitchOctaves(0);//params->pitchDelta
    stSoundTouch.setRateChange(0);//params->rateDelta

    stSoundTouch.setSetting(SETTING_USE_QUICKSEEK, 0);//params->quick
    stSoundTouch.setSetting(SETTING_USE_AA_FILTER, 0);//!params->noAntiAliasy

	stSoundTouch.flush(); //reset
	
	stSoundTouch.setChannels(2);
	
	m_idelayCounter = 0;
	
	m_bufferUnderrunDelay = 30;//gGame.m_VariSpeed.iPitchShiftDelay; // //minimum frame delay required to prevent buffer underrun

	m_fPrevPitch = -1;
	

#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::PitchShift::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.PitchShift.Process");

#ifndef PLATFORM_SPU
#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

	static bool processBypass = true;

	if(!m_fPitch & processBypass)
		return true; //don't process.

	F32* FrameBufferData = (F32*)pFrameData;;

		//process
	int nChannels = 2;
	

	int singleBufferFrameSize = /*OUTBUFF_SIZE*/512 / nChannels;

	int nSamples = singleBufferFrameSize * nChannels;
	//int outBuffSize = 4096 * nChannels;
    int buffSizeSamples;


	if(m_fPitch != m_fPrevPitch)
	{
		stSoundTouch.setPitchOctaves(m_fPitch);
		m_fPrevPitch = m_fPitch;
	}


	SAMPLETYPE sampleBuffer[512];
	SAMPLETYPE tempBuffer[512];
	
    buffSizeSamples = nSamples / nChannels;
	
	//get samples
	for( int j = 0; j < singleBufferFrameSize*nChannels; j++ )
	{
		sampleBuffer[j] =  FrameBufferData[j]/*( FrameBufferData.pSampleBuffer[j] )*/;
		
	}

	if(nChannels == 2)
	{						
	//rearrange buffer for sample interleave
		int tempCounter = 0;
		for(int i = 0;i<singleBufferFrameSize*2;i++)
		{			

			if(i%2 == 0)//odd index
				tempBuffer[i] = sampleBuffer[tempCounter];
			if(i%2 == 1)//even index
			{
			tempBuffer[i] = sampleBuffer[tempCounter+singleBufferFrameSize];
			tempCounter++;
			}
		}

			for(int i = 0;i<singleBufferFrameSize*2;i++)
				sampleBuffer[i] = tempBuffer[i] * .25f;
	}


	stSoundTouch.putSamples(sampleBuffer, buffSizeSamples);

	if(m_idelayCounter >= m_bufferUnderrunDelay)//wait initially 25 frames until we process
	{
		nSamples = stSoundTouch.receiveSamples(sampleBuffer, buffSizeSamples);

	

		if(nSamples != 256 ) //if nothing has come out of the soundtouch process
		{
			int i = 0;
			i++;
		}

		if(nChannels == 2)
		{
			int tempCounter = 0;
			//rearrange buffer for Frame interleave
			for(int i = 0;i<singleBufferFrameSize*2;i++)
			{
				if(i%2 == 0)
					tempBuffer[tempCounter] = sampleBuffer[i];
				if(i%2 == 1)
				{
					tempBuffer[tempCounter+singleBufferFrameSize] = sampleBuffer[i];
					tempCounter++;
				}
			}
			for(int i = 0;i<singleBufferFrameSize*2;i++)
				FrameBufferData[i] = tempBuffer[i] / .25f;
		}

		nChannels++;
		nChannels	= 2;

	}
	else
	for( int j = 0; j < singleBufferFrameSize * nChannels; j++ )//initial buffer underrun delay
		FrameBufferData[j] = 0;

	m_idelayCounter++;

    return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
#else	// PLATFORM_SPU
sLOGERROR(0, "DSP pitch shift not implemented on SPU");
return false;
#endif
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
