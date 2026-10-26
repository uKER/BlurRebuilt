#include "AudioPch.h"
#include "coreheaders.h"

#include "Core/Maths/HVec/New/FFT.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
typedef unsigned long ULONG;
#define ULONG_PTR DWORD
#endif

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::SpectralEnvelope::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	SpectralEnvelope
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::SpectralEnvelope::SpectralEnvelope( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP

	m_eType = Core::DSP::Effect::eSpectralEnvelope;

	m_fBin1_Magnitude = 0;
	m_fBin2_Magnitude = 0;
	m_fBin3_Magnitude = 0;
	m_fBin4_Magnitude = 0;
	m_fBin5_Magnitude = 0;
	m_fBin6_Magnitude = 0;
	m_fBin7_Magnitude = 0;
	m_fBin8_Magnitude = 0;
	m_fBin9_Magnitude = 0;
	m_fBin10_Magnitude = 0;
	m_fBin11_Magnitude = 0;
	m_fBin12_Magnitude = 0;
	m_fBin13_Magnitude = 0;
	m_fBin14_Magnitude = 0;
	m_fBin15_Magnitude = 0;
	m_fBin16_Magnitude = 0;

	frameLength = uFrameLength;

	m_fftSize = frameLength;

	m_PVSize = uFrameLength/2;
	//m_magBuffer = new F32 [m_PVSize];
	////m_magBuffer = (F32*)System::Get()->Alloc(m_PVSize * sizeof(F32), 128);

//	m_phaseBuffer = new F32 [m_PVSize];
	////m_phaseBuffer = (F32*)System::Get()->Alloc(m_PVSize * sizeof(F32), 128);
	m_uChannels = uChannels;

	//m_windowBuffer = (F32*)aligned_memory_alloc(m_windowBuffer_Unaligned,m_fftSize*sizeof(F32),128);
////	m_windowBuffer = (F32*)System::Get()->Alloc(m_fftSize * sizeof(F32), 128);
	//m_windowBuffer = new F32 [m_fftSize];
////	memset(m_windowBuffer,0,m_fftSize*sizeof(F32));

	///*F32 **/m_phaseBuffer = (F32*)System::Get()->Alloc(m_PVSize * sizeof(F32), 128);
	//memset(m_phaseBuffer,0,m_PVSize*sizeof(F32));
	///*F32 **/m_magBuffer = (F32*)System::Get()->Alloc(m_PVSize * sizeof(F32), 128);
	//memset(m_magBuffer,0,m_PVSize*sizeof(F32));

	m_firstTimeProcessing = true;

	if(uSampleRate != 48000)
	{
	#ifndef CFG_GOLD
		printf("\nSpectralEnvelop is currently tuned to work at a samplerate of 48000 Hz, if not, the frequency bands won't be tuned");
	#endif
		BIZ_ASSERT(uSampleRate == 48000);
	}

#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

#ifndef PLATFORM_SPU
Audio::Core::DSP::SpectralEnvelope::~SpectralEnvelope()
{
	//free(m_windowBuffer_Unaligned);
	//	System::Free(m_phaseBuffer);
	//System::Free(m_magBuffer);
	
	////free(m_magBuffer);
	////free(m_phaseBuffer);
	//delete [] m_magBuffer;
	//delete [] m_phaseBuffer;
	//delete [] m_windowBuffer;

}
#endif
////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::SpectralEnvelope::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.SpectralEnvelope.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;


	//S32 m_fftSize;

	//init the buffers
	if(m_firstTimeProcessing)
	{

		m_windowBuffer = &info.pStaticBuffers[ m_uBuffer + m_uChannels * m_fftSize * Buffers_Window];
	    memset(m_windowBuffer,0,m_fftSize*sizeof(F32));

		m_spectralBuffer = &info.pStaticBuffers[ m_uBuffer + m_uChannels * m_fftSize * Buffers_Spectral];
	    memset(m_spectralBuffer,0,m_fftSize*sizeof(F32));

		//m_phaseBuffer = &info.pDynamicBuffers[ m_uDynamicBuffer + m_uChannels * m_fftSize/2 * DynamicBuffers_Phase ];
		//memset(m_phaseBuffer,0,m_fftSize/2*sizeof(F32));
		
		//m_magBuffer = &info.pDynamicBuffers[ m_uDynamicBuffer + m_uChannels * m_fftSize/2 * DynamicBuffers_Mag ];
		//memset(m_magBuffer,0,m_fftSize/2*sizeof(F32));

		m_firstTimeProcessing = false;
	}


	



	F32* aSample = (F32*)pFrameData;

	m_fBin1_Magnitude= 0;
	m_fBin2_Magnitude= 0;
	m_fBin3_Magnitude= 0;
	m_fBin4_Magnitude= 0;
	m_fBin5_Magnitude= 0;
	m_fBin6_Magnitude= 0;
	m_fBin7_Magnitude= 0;
	m_fBin8_Magnitude= 0;
	m_fBin9_Magnitude= 0;
	m_fBin10_Magnitude= 0;
	m_fBin11_Magnitude= 0;
	m_fBin12_Magnitude= 0;
	m_fBin13_Magnitude= 0;
	m_fBin14_Magnitude= 0;
	m_fBin15_Magnitude= 0;
	m_fBin16_Magnitude= 0;
	
	Frame::Copy(m_windowBuffer,aSample);

#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
	RealFFT_Split(m_windowBuffer,frameLength);
#else
	::Maths::RealFFT_Split(m_windowBuffer,frameLength);
#endif
	

	

	RectangularToPolarConversionNewFormat(m_windowBuffer,frameLength);


	// 48000 / 256 = 187.5
	m_fBin1_Magnitude= m_windowBuffer[0]; //0 - 187.5 -
	m_fBin2_Magnitude= m_windowBuffer[1]; // 375 -
	m_fBin3_Magnitude= m_windowBuffer[2]; // 562.5 - 

	m_fBin4_Magnitude= m_windowBuffer[3] + m_windowBuffer[4]; // 750 -937.5 -

	m_fBin4_Magnitude /= 2;


	m_fBin5_Magnitude= m_windowBuffer[5] + m_windowBuffer[6] + m_windowBuffer[7]; // 1500

	m_fBin5_Magnitude /= 3;

	for(int i = 8;i <= 16; i++) //1687.5 - 3000 Hz
		m_fBin6_Magnitude += m_windowBuffer[i]; 

	m_fBin6_Magnitude /= 9;


	for(int i = 17;i <= 32; i++) //3187.5 - 6000 Hz
		m_fBin7_Magnitude += m_windowBuffer[i];  

	m_fBin7_Magnitude /= 16;
	
	for(int i = 33;i <= (frameLength/2)-1; i++) //6187.5 - 22kHz
		m_fBin8_Magnitude +=  m_windowBuffer[i];

	m_fBin8_Magnitude /= (frameLength/2) - 33;


	//do second chan
	if(m_uChannels == 2)
	{
		memset(m_windowBuffer,0,m_fftSize*sizeof(F32));

		Frame::Copy(m_windowBuffer,aSample+frameLength);

#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
		RealFFT_Split(m_windowBuffer,frameLength);
#else
		::Maths::RealFFT_Split(m_windowBuffer,frameLength);
#endif

		RectangularToPolarConversionNewFormat(m_windowBuffer,frameLength);

			// 48000 / 256 = 187.5
		m_fBin9_Magnitude= m_windowBuffer[0]; //0 - 187.5 -
		m_fBin10_Magnitude= m_windowBuffer[1]; // 375 -
		m_fBin11_Magnitude= m_windowBuffer[2]; // 562.5 - 

		m_fBin12_Magnitude= m_windowBuffer[3] + m_windowBuffer[4]; // 750 -937.5 -

		m_fBin12_Magnitude /= 2;


		m_fBin13_Magnitude= m_windowBuffer[5] + m_windowBuffer[6] + m_windowBuffer[7]; // 1500

		m_fBin13_Magnitude /= 3;

		for(int i = 8;i <= 16; i++) //1687.5 - 3000 Hz
			m_fBin14_Magnitude += m_windowBuffer[i]; 

		m_fBin14_Magnitude /= 9;


		for(int i = 17;i <= 32; i++) //3187.5 - 6000 Hz
			m_fBin15_Magnitude += m_windowBuffer[i];  

		m_fBin15_Magnitude /= 16;
		
		for(int i = 33;i <= (frameLength/2)-1; i++) //6187.5 - 22kHz
			m_fBin16_Magnitude +=  m_windowBuffer[i];

		m_fBin16_Magnitude /= (frameLength/2) - 33;


	}
	
	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}

void * Audio::Core::DSP::SpectralEnvelope::aligned_memory_alloc(void * pa, size_t size, size_t alignment )  
{  
     void /**pa,*/ *ptr;  
   
     //pa=malloc(((size+alignment-1)&~(alignment-1))+sizeof(void *)+alignment-1);  
   
     // 1  
     pa=malloc((size+alignment-1)+sizeof(void *));  
     if(!pa)  
         return NULL;  
   
     // 2  
     ptr=(void*)( ((ULONG_PTR)pa+sizeof(void *)+alignment-1)&~(alignment-1) );  
   
     // 3  
     *((void **)ptr-1)=pa;  
   
//     printf("CAlignedAlloc::new(%d,%d)=%x\n", (ULONG)size, alignment, (ULONG)ptr);  
   
     return ptr;  
}  

//********************************
//RectangularToPolarConversion
//********************************
void Audio::Core::DSP::SpectralEnvelope::RectangularToPolarConversionNewFormat(F32 * signal, int fftSize)
{

#ifndef AUDIO_DISABLE_DSP



#if !defined (PLATFORM_PC)
#if !defined (PLATFORM_PS3)
#if !defined (PLATFORM_SPU)

	//RectangularToPolarConversion 
	// r = sqrt(x*x + y*y)
	// q = arctan(y/x)
	//input:
	//re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
	// output:
	//	{[MAG],[MAG]......[PHASE],[PHASE]}

	//int PVSize = fftSize/2;




	//F32 m_phaseBuffer[m_PVSize];
	//F32 m_magBuffer[m_PVSize];


	//m_magBuffer[0]   =  Maths::Sqrt (signal [0] * signal [0] + signal [m_PVSize] * signal [m_PVSize]); //magnitude
	//m_phaseBuffer[0] =  Maths::ArcTan2(signal [m_PVSize],signal [0]);	//phase

	m_spectralBuffer[0]   =  Maths::Sqrt (signal [0] * signal [0] + signal [m_PVSize] * signal [m_PVSize]); //magnitude
	m_spectralBuffer[m_PVSize] =  Maths::ArcTan2(signal [m_PVSize],signal [0]);	//phase


	for (int i = 1; i <m_PVSize; i ++)//nbr_points/2
	{
		//F32	img = 0;
	//	F32	real = signal [i];

		//if (i > 0)
		//{
		//	img = signal [fftSize-i];
		//}
		//else
		//{
		//	//because of horrible FFT output format, img[0] == re(size/2), at signal[size/2)
		//	img = signal [PVSize];
		//}

		//m_magBuffer[i]   =  Maths::Sqrt (signal [i] * signal [i] + signal[fftSize-i] * signal[fftSize-i]); //magnitude
		//m_phaseBuffer[i] =  Maths::ArcTan2(signal[fftSize-i],signal [i]);	//phase

		
		m_spectralBuffer[i]   =  Maths::Sqrt (signal [i] * signal [i] + signal[fftSize-i] * signal[fftSize-i]); //magnitude
		m_spectralBuffer[i+m_PVSize] =  Maths::ArcTan2(signal[fftSize-i],signal [i]);	//phase
				
	}

	//output is now
	///[MAG],[MAG],.......[PHASE],[PHASE]
	Frame::Copy(signal,m_spectralBuffer);
	//Frame::Copy(signal+256,m_magBuffer+256);
	//Frame::Copy(signal+512,m_phaseBuffer);
	//Frame::Copy(signal+768,m_phaseBuffer+256);
	//replaces
	////output of conversion, PV, is [MAG],[PHASE],[MAG,[PHASE]
	//for (int i = 0, k = 0; i < fftSize-2, k < PVSize; i +=2,k++ )//nbr_points/2
	//{
	//	signal[i] =		ampbuffer[k];
	//	signal[i+1] =	phasebuffer[k];
	//}




	//System::Free(m_phaseBuffer);
	//System::Free(m_magBuffer);

	#else // PLATFORM_SPU

	//	do function
	  return;

#endif // PLATFORM_SPU
#endif
#endif //end pc

#else // AUDIO_DISABLE_DSP
	return;
#endif // AUDIO_DISABLE_DSP

}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
