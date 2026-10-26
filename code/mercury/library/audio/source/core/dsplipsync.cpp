#include "AudioPch.h"
#include "coreheaders.h"

#include "Core/Maths/HVec/New/FFT.h"


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

////////////////////////////////////////////////////////////////////////////////
//	static data
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::LipSync::bDisableAll = false;

////////////////////////////////////////////////////////////////////////////////
//	LipSync
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::DSP::LipSync::LipSync( const char* pchInfo, U16 uChannels ) : Effect( pchInfo, uChannels )
{
#ifndef AUDIO_DISABLE_DSP

	m_eType = Core::DSP::Effect::eLipSync;

	m_fprevRMS = 0.f;
	m_fprevOrigRMS = 0.f;
	m_fprevPitch = 0.5f;
	m_fblendRMS = 0.75f;
	m_fblendPitch = 0.90f;
	m_fsilenceThreshold = 0.023f;
	m_fclosedMouthWidth = 0.355f;

	m_fprevSib = 0.5f;
	m_fprevSibOutput = 0.5f;
	m_fblendSib = 0.98f;
	m_fsibThreshold = .06;
	m_fsibScale = .5;
	m_fpitchThreshold = .24;
	m_fSibPostScale = 40;

	m_ftotalSilence = .006;

	m_ffreqdiv = 6800 * 2;//

	m_fcurve = 0.5f;
	m_fscale = 2.2f;

	m_fsibSmoothScale = .1;
	m_fsilSmoothScale = .1;
	m_fsilRMSSmoothScale = .4;

	m_fSibDiff = 0;

	m_fSubFrameRunningTotalRMS =0;	
	m_fSubFrameRunningTotalPitch =0;
	m_fSubFrameRunningTotalSibDiff =0;


	m_bhasSib = false;

	m_firstTimeProcessing = true;

#endif	//	#ifndef AUDIO_DISABLE_DSP
}
#endif

////////////////////////////////////////////////////////////////////////////////
//	Process
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DSP::LipSync::Process(void* pFrameData, const ProcessInfo& info, bool bProcess)
{
	AUDIO_PERF_MICRO_EVENT("Audio.Generate.DSP.LipSync.Process");

#ifndef AUDIO_DISABLE_DSP

	if( !m_bActive || bDisableAll )
		return true;

		//init the buffers
	if(m_firstTimeProcessing)
	{

		hambuffer =				&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_ham ];
		deribuffer =			&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_deri ];
		deri_x =				&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_deri_x ];
		deri_f =				&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_deri_f ];
		deri_Imag_buffer =		&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_deri_Imag ];
		deri_Real_buffer =		&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_deri_Real ];
		Imag_buffer =			&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_Imag ];
		Real_buffer =			&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_Real ];
		fftBuffer =				&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_fft ];
		ampbuffer =				&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_amp ];
		testBufferSorensen =	&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_testBufferSorensen ];
		signalBuffer =			&info.pStaticBuffers[ m_uBuffer + m_uChannels * uFrameLength * Buffers_signal ];


		m_firstTimeProcessing = false;
	}


	F32* aSample = (F32*)pFrameData;

	//  each channel
	for (U32 uChannel = 0; uChannel < m_uChannels; uChannel++)
	{
	
		//LIPSYNC MAIN ALGORITHM
		//*********************************

		F32 IFDFreq = 0;
		F32 RMSValue = 0;

		GetPitchRMS(aSample,fSampleRate,&IFDFreq, &RMSValue);

		F32 scaledPitch = powf(IFDFreq/m_ffreqdiv, m_fcurve); //1926 is avg pitch of maledialog1
		F32 scaledRMS = RMSValue * m_fscale;

		F32 smoothedRMS = m_fprevRMS * m_fblendRMS + scaledRMS * (1-m_fblendRMS);
		F32 smoothedPitch = m_fprevPitch * m_fblendPitch + scaledPitch * (1-m_fblendPitch);

		F32 scaledSib = smoothedPitch * m_fsibScale;

		F32 smoothedSib = m_fprevRMS * m_fblendSib + scaledSib * (1-m_fblendSib);

		F32 SibOutput = smoothedRMS;

		if(smoothedRMS < m_fsibThreshold && smoothedPitch > m_fpitchThreshold) //if sib
		{
			//if sibilent, set rms value to sib value
			SibOutput = smoothedSib;
			smoothedPitch += (m_fclosedMouthWidth - smoothedPitch) * m_fsibSmoothScale; //fairly simple smoothing

		}
		else
		if(smoothedRMS < m_fsilenceThreshold) //if silence
		{
			//if silent and not sibilent, rms remains normal
			
			//new bit
			smoothedPitch += (m_fclosedMouthWidth - smoothedPitch) * m_fsilSmoothScale; //fairly simple smoothing
			smoothedRMS = m_fsilenceThreshold;
			
			if(IFDFreq == 0 && RMSValue < m_ftotalSilence)
				smoothedPitch = m_fclosedMouthWidth;
			
		}

	
		m_fprevRMS = smoothedRMS; //store for next pass
		m_fprevOrigRMS = RMSValue;
		m_fprevPitch = smoothedPitch;
		m_fprevSib = smoothedSib;
		m_fprevSibOutput = SibOutput;

		if(RMSValue < m_ftotalSilence)//IFDFreq == 0 && 
			m_fSibDiff = 0; //prevent jitter
		else
		m_fSibDiff = (SibOutput-smoothedRMS) * m_fSibPostScale;


		if(!m_fSibDiff && (smoothedRMS <= m_fsilenceThreshold) )//RMSValue< totalSilence && prevOrigRMS < totalSilence
		{
			smoothedRMS = m_fsilenceThreshold; 
			smoothedPitch = m_fclosedMouthWidth; 
		}

		//get avg over frames
		m_fSubFrameRunningTotalRMS		+= smoothedRMS;
		m_fSubFrameRunningTotalPitch	+= smoothedPitch;
		m_fSubFrameRunningTotalSibDiff	+= m_fSibDiff;

		if (info.uFrameMod == info.uFrameInt - 1)
		{
					//set output variables
			m_fRMSOutput = m_fSubFrameRunningTotalRMS * info.fWidth;
			m_fPitchOutput = m_fSubFrameRunningTotalPitch * info.fWidth;
			m_fSibDifference = m_fSubFrameRunningTotalSibDiff * info.fWidth;

			m_fSubFrameRunningTotalRMS		= 0;	
			m_fSubFrameRunningTotalPitch	= 0;	
			m_fSubFrameRunningTotalSibDiff	= 0;

		}
		//else
		//{//set to zero between frame calls
		//		m_fRMSOutput = 0;
		//		m_fPitchOutput = 0;
		//		m_fSibDifference =0;
		//}





	}
	return true;

#else // AUDIO_DISABLE_DSP
	return false;
#endif // AUDIO_DISABLE_DSP
}
////////////////////////////////////////////////////////////////////////////////
//	LipSyncer
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::DSP::LipSync::GetPitchRMS(F32 *signal, F32 sampleRate, F32 *PitchOutput, F32 *RMSOutput)
{
#ifndef AUDIO_DISABLE_DSP

		// fft OBJECT
	//FFTReal <F32> fft_object (FRAMESIZE);   // 256-point FFT object constructed.
		const F32	PI		= 3.1415926535897932384626433832795;


		//F32 hambuffer[uFrameLength]; 
		//F32 deribuffer[uFrameLength];
		//F32 deri_x[uFrameLength];
		//F32 deri_f[uFrameLength];
		//F32 deri_Imag_buffer[uFrameLength];
		//F32 deri_Real_buffer[uFrameLength];
		//F32 Imag_buffer[uFrameLength];
		//F32 Real_buffer[uFrameLength];

		//F32 fftBuffer[uFrameLength];
		//F32 ampbuffer[uFrameLength];
		//F32 testBufferSorensen[uFrameLength];

		//F32 signalBuffer[uFrameLength];

		for(U32 i = 0; i < uFrameLength;i++)
		{   signalBuffer[i] = signal[i];
			//memcpy(signalBuffer,signal,uFrameLength);//make a copy, don't want to affect signal
		}

		

			F32 RMS = 0;
				
			// RMS Window calculation
//		F32  m_fRMSWindow   = m_fAverage * sampleRate;
//		F32  m_fRMSWindow1  = m_fRMSWindow - 1.0f;


		int i =0;

		//Apply Windows to signal& get RMS level
		 U32 w = 0;
								 
		for( w = 0;w< uFrameLength;w++)
		{
			//Gather RMS data
			RMS += signalBuffer[w] * signalBuffer[w]; //square amps

			//WINDOWING
			F32 coef1=(F32)((2*PI*w)/uFrameLength); //hamming window coefficients
			hambuffer[w] = (F32)(1-(0.54 + .46*cos(coef1))) * 1; //create hamming window
						
			if(w>0)
				deribuffer[w-1] = (hambuffer[w-1] - hambuffer[w]); //negative derivative
						
			if(w==uFrameLength-1)
				deribuffer[w] = 0; //the last index needs to be set to zero since above loop doesn't cover it
		}

		for( w = 0;w< uFrameLength;w++)//extra loop since deribuffer is lookahead
		{
			deri_x[w] = deribuffer[w] * signalBuffer[w]; //the negative derivative scaled by signal
			signalBuffer[w] = hambuffer[w] * signalBuffer[w]; //windowed signal
		} 

					
		//	// Compute FFT and IFFT DE Solas FFT
		//	//FFTReal
		////	fft_object.do_fft (testBufferFFTReal, signal);//fft of windowed signal
		//	fft_object.do_fft (fftBuffer, signal);//fft of windowed signal
		//	
		//	//FFTReal
		//	fft_object.do_fft (deri_f, deri_x); //fft of negative derivative signal


			//Sorensen FFT + Buffer Formatting
		//**********************************************
			//memcpy(testBufferSorensen,signal,256);
			for(int i = 0;i<256;i++)
			testBufferSorensen[i] = signalBuffer[i];

#if defined (PLATFORM_SPU) || defined (PLATFORM_PS3)
			RealFFT_Split(testBufferSorensen,uFrameLength);
						//Sorensen FFT
			RealFFT_Split(deri_x,uFrameLength);
#else
			
			::Maths::RealFFT_Split(testBufferSorensen,uFrameLength);
			::Maths::RealFFT_Split(deri_x,uFrameLength);
#endif

			
			//[Cian] - now format buffers for FFTReal, easier to process and i've previously written the code for that one anyway..
			for(U32 i = 0;i <= uFrameLength/2;i++)
			{
				fftBuffer[i] = testBufferSorensen[i];
				deri_f[i] = deri_x[i];
			}

			for(U32 i = uFrameLength/2+1; i < uFrameLength;i++)
			{
				int writePtr =  (uFrameLength - i) + uFrameLength/2;
				fftBuffer[i] = testBufferSorensen[writePtr];
				deri_f[i] = deri_x[writePtr];
			}
		//**********************************************
		//	END Sorensen FFT + Buffer Formatting


			// Put DFT results in buffers
		
			for (U32 i = 0; i <=uFrameLength/2; i++)//only need real part
			{
				F32			img;
				F32			img2;

				const F32	real = fftBuffer [i];
				const F32	real2 = deri_f [i];

				if (i > 0 && i < uFrameLength/2)
				{
					img = fftBuffer [i + uFrameLength/2];
					img2 = deri_f [i + uFrameLength/2];
				}
				else
				{
					img = 0;
					img2 = 0;
				}

				const F32	f_abs = Maths::Sqrt (real * real + img * img); //magnitude
//				const F32	f_phase=Maths::ArcTan(img/real);	//phase
				
				Imag_buffer[i] = img;
				Real_buffer[i] = real;
				
				deri_Imag_buffer[i] = img2;
				deri_Real_buffer[i] = real2;

				ampbuffer[i] = f_abs;//put f_abs in buffer so we can calculate fundamental
				//phasebuffer[i] = f_phase; //not used in lipsync...may be needed in future
				//	printf ("%5d: %+12.6f %+12.6f (%12.6f)  (%12.6f)\n", i, real, img, f_abs, f_phase);
				
			}


			//Get IFD estimate
							//**********************************************
				int limit = ( uFrameLength/ 2 ); //only check real

				F32 maxAmp= 0;
//				F32 max=0;
				int currentIndex=0;

				for ( i = 0;i <limit; i++) //search amps
				{
					if (maxAmp< ampbuffer[i])
					{
						maxAmp= ampbuffer[i]; //make this amp the max
				currentIndex = i; //store maxamp index

					}
				}

				//getting the instantaneos freq. complex formula equivilent to dividing the 2 DFT's
				//  (b * c - a * d) / (a*a) + (b*b)   ---- a,b are coeffs of windowed DFT. c,d of negative deri DFT
				
				//get the cf of bin
				F32 fftFundamental = currentIndex*(sampleRate * fRcpFrameLength); //get cf of bin
				//get power spectrum
				F32 pwrSpec = Real_buffer[currentIndex] * Real_buffer[currentIndex] + Imag_buffer[currentIndex] * Imag_buffer[currentIndex];
				
				//get the other bit 
				F32 complexMultiplication = (deri_Imag_buffer[currentIndex] * Real_buffer[currentIndex])
							- (deri_Real_buffer[currentIndex] * Imag_buffer[currentIndex]);
				//divide
				F32 deviation = complexMultiplication / pwrSpec;  

				//Hz conversion
				F32 IFDFreq = deviation * (sampleRate/(2*PI ));

				//Instantaneous frequency
				IFDFreq = fftFundamental - IFDFreq;

				if(IFDFreq == IFDFreq) //check if NAN
				*PitchOutput = IFDFreq; //output pitch
				else
					*PitchOutput = 0;

				//Calculate frame RMS value

				RMS = RMS / uFrameLength; //get avg
				
				*RMSOutput = Maths::Sqrt(RMS); //get RMS and output

#endif	//	#ifndef AUDIO_DISABLE_DSP
}

////////////////////////////////////////////////////////////////////////////////////
//////	FFT
////// Sorensen in-place split-radix FFT for real values
////// data: array of doubles:
////// re(0),re(1),re(2),...,re(size-1)
////// 
////// output:
////// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
////// normalized by array length
//////
////// Source: 
////// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
////// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
////
////void Audio::Core::DSP::LipSync::realfft_split(F32 *data,long n){
////
////// temp.. disable this on spu for now, so we fit in memory
////#ifndef PLATFORM_SPU
////#ifndef AUDIO_DISABLE_DSP
////
////	const F32	PI		= 3.1415926535897932384626433832795;
////
////  long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8;
////  F32 t1,t2,t3,t4,t5,t6,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;
////  
////  sqrt2=Maths::fRoot2;
////  n4=n-1;
////  
////  //data shuffling
////      for (i=0,j=0,n2=n/2; i<n4 ; i++){
////	  if (i<j){
////				t1=data[j];
////				data[j]=data[i];
////				data[i]=t1;
////				}
////	  k=n2;
////	  while (k<=j){
////				j-=k;
////				k>>=1;	
////				}
////	  j+=k;
////      }
////	
/////*----------------------*/
////	
////	//length two butterflies	
////	i0=0;
////	id=4;
////   do{
////       for (; i0<n4; i0+=id){ 
////			i1=i0+1;
////			t1=data[i0];
////			data[i0]=t1+data[i1];
////			data[i1]=t1-data[i1];
////		}
////	   id<<=1;
////       i0=id-2;
////       id<<=1;
////    } while ( i0<n4 );
////
////   /*----------------------*/
////   //L shaped butterflies
////n2=2;
////for(k=n;k>2;k>>=1){  
////	n2<<=1;
////	n4=n2>>2;
////	n8=n2>>3;
////	e = 2*PI/(n2);
////	i1=0;
////	id=n2<<1;
////	do{ 
////	    for (; i1<n; i1+=id){
////			i2=i1+n4;
////			i3=i2+n4;
////			i4=i3+n4;
////			t1=data[i4]+data[i3];
////			data[i4]-=data[i3];
////			data[i3]=data[i1]-t1;
////			data[i1]+=t1;
////			if (n4!=1){
////				i0=i1+n8;
////				i2+=n8;
////				i3+=n8;
////				i4+=n8;
////				t1=(data[i3]+data[i4])/sqrt2;
////				t2=(data[i3]-data[i4])/sqrt2;
////				data[i4]=data[i2]-t1;
////				data[i3]=-data[i2]-t1;
////				data[i2]=data[i0]-t2;
////				data[i0]+=t2;
////			}
////	     }
////		 id<<=1;
////	     i1=id-n2;
////	     id<<=1;
////	  } while ( i1<n );
////	a=e;
////	for (j=2; j<=n8; j++){  
////	      a3=3*a;
////	      cc1=cos(a);
////	      ss1=sin(a);
////	      cc3=cos(a3);
////	      ss3=sin(a3);
////	      a=j*e;
////	      i=0;
////	      id=n2<<1;
////	      do{
////		   for (; i<n; i+=id){  
////			  i1=i+j-1;
////			  i2=i1+n4;
////			  i3=i2+n4;
////			  i4=i3+n4;
////			  i5=i+n4-j+1;
////			  i6=i5+n4;
////			  i7=i6+n4;
////			  i8=i7+n4;
////			  t1=data[i3]*cc1+data[i7]*ss1;
////			  t2=data[i7]*cc1-data[i3]*ss1;
////			  t3=data[i4]*cc3+data[i8]*ss3;
////			  t4=data[i8]*cc3-data[i4]*ss3;
////			  t5=t1+t3;
////			  t6=t2+t4;
////			  t3=t1-t3;
////			  t4=t2-t4;
////			  t2=data[i6]+t6;
////			  data[i3]=t6-data[i6];
////			  data[i8]=t2;
////			  t2=data[i2]-t3;
////			  data[i7]=-data[i2]-t3;
////			  data[i4]=t2;
////			  t1=data[i1]+t5;
////			  data[i6]=data[i1]-t5;
////			  data[i1]=t1;
////			  t1=data[i5]+t4;
////			  data[i5]-=t4;
////			  data[i2]=t1;
////		     }
////		   id<<=1;
////		   i=id-n2;
////		   id<<=1;
////		 } while(i<n);
////	   }
////      }
////
////	//division with array length
//// //  for(i=0;i<n;i++) data[i]/=n;
////
////#endif	//	#ifndef AUDIO_DISABLE_DSP
////#endif	//	PLATFORM_SPU
////}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
