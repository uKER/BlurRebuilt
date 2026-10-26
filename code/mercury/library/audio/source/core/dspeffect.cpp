#include "AudioPch.h"
#include "coreheaders.h"


#ifdef SN_TARGET_PS3_SPU

////////////////////////////////////////////////////////////////////////////////
//	FFT
// Sorensen in-place split-radix FFT for real values
// data: array of doubles:
// re(0),re(1),re(2),...,re(size-1)
// 
// output:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
// normalized by array length
//
// Source: 
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

//void realfft_split(F32 *data,long n){
//#ifndef AUDIO_DISABLE_DSP
//
//	const F32	PI		= 3.1415926535897932384626433832795;
//
//  long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8;
//  F32 t1,t2,t3,t4,t5,t6,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;
//  
//  sqrt2=Audio::Maths::fRoot2;
//  n4=n-1;
//  
//  //data shuffling
//      for (i=0,j=0,n2=n/2; i<n4 ; i++){
//	  if (i<j){
//				t1=data[j];
//				data[j]=data[i];
//				data[i]=t1;
//				}
//	  k=n2;
//	  while (k<=j){
//				j-=k;
//				k>>=1;	
//				}
//	  j+=k;
//      }
//	
///*----------------------*/
//	
//	//length two butterflies	
//	i0=0;
//	id=4;
//   do{
//       for (; i0<n4; i0+=id){ 
//			i1=i0+1;
//			t1=data[i0];
//			data[i0]=t1+data[i1];
//			data[i1]=t1-data[i1];
//		}
//	   id<<=1;
//       i0=id-2;
//       id<<=1;
//    } while ( i0<n4 );
//
//   /*----------------------*/
//   //L shaped butterflies
//n2=2;
//for(k=n;k>2;k>>=1){  
//	n2<<=1;
//	n4=n2>>2;
//	n8=n2>>3;
//	e = 2*PI/(n2);
//	i1=0;
//	id=n2<<1;
//	do{ 
//	    for (; i1<n; i1+=id){
//			i2=i1+n4;
//			i3=i2+n4;
//			i4=i3+n4;
//			t1=data[i4]+data[i3];
//			data[i4]-=data[i3];
//			data[i3]=data[i1]-t1;
//			data[i1]+=t1;
//			if (n4!=1){
//				i0=i1+n8;
//				i2+=n8;
//				i3+=n8;
//				i4+=n8;
//				t1=(data[i3]+data[i4])/sqrt2;
//				t2=(data[i3]-data[i4])/sqrt2;
//				data[i4]=data[i2]-t1;
//				data[i3]=-data[i2]-t1;
//				data[i2]=data[i0]-t2;
//				data[i0]+=t2;
//			}
//	     }
//		 id<<=1;
//	     i1=id-n2;
//	     id<<=1;
//	  } while ( i1<n );
//	a=e;
//	for (j=2; j<=n8; j++){  
//	      a3=3*a;
//	      cc1=cos(a);
//	      ss1=sin(a);
//	      cc3=cos(a3);
//	      ss3=sin(a3);
//	      a=j*e;
//	      i=0;
//	      id=n2<<1;
//	      do{
//		   for (; i<n; i+=id){  
//			  i1=i+j-1;
//			  i2=i1+n4;
//			  i3=i2+n4;
//			  i4=i3+n4;
//			  i5=i+n4-j+1;
//			  i6=i5+n4;
//			  i7=i6+n4;
//			  i8=i7+n4;
//			  t1=data[i3]*cc1+data[i7]*ss1;
//			  t2=data[i7]*cc1-data[i3]*ss1;
//			  t3=data[i4]*cc3+data[i8]*ss3;
//			  t4=data[i8]*cc3-data[i4]*ss3;
//			  t5=t1+t3;
//			  t6=t2+t4;
//			  t3=t1-t3;
//			  t4=t2-t4;
//			  t2=data[i6]+t6;
//			  data[i3]=t6-data[i6];
//			  data[i8]=t2;
//			  t2=data[i2]-t3;
//			  data[i7]=-data[i2]-t3;
//			  data[i4]=t2;
//			  t1=data[i1]+t5;
//			  data[i6]=data[i1]-t5;
//			  data[i1]=t1;
//			  t1=data[i5]+t4;
//			  data[i5]-=t4;
//			  data[i2]=t1;
//		     }
//		   id<<=1;
//		   i=id-n2;
//		   id<<=1;
//		 } while(i<n);
//	   }
//      }
//
//	//division with array length
// //  for(i=0;i<n;i++) data[i]/=n;
//
//#endif	//	#ifndef AUDIO_DISABLE_DSP
//}
//
//
//////////////////////////////////////////////////////////////////////////////////
////	LipSyncer
//////////////////////////////////////////////////////////////////////////////////
//void GetPitchRMS(F32 *signal, F32 sampleRate, F32 *PitchOutput, F32 *RMSOutput, Audio::Core::DSP::LipSync* pLipSync)
//{
//#ifndef AUDIO_DISABLE_DSP
//
//	// fft OBJECT
////FFTReal <F32> fft_object (FRAMESIZE);   // 256-point FFT object constructed.
//	const F32	PI		= 3.1415926535897932384626433832795;
//
//
//	F32 hambuffer[Audio::Core::uFrameLength]; 
//	F32 deribuffer[Audio::Core::uFrameLength];
//	F32 deri_x[Audio::Core::uFrameLength];
//	F32 deri_f[Audio::Core::uFrameLength];
//	F32 deri_Imag_buffer[Audio::Core::uFrameLength];
//	F32 deri_Real_buffer[Audio::Core::uFrameLength];
//	F32 Imag_buffer[Audio::Core::uFrameLength];
//	F32 Real_buffer[Audio::Core::uFrameLength];
//
//	F32 fftBuffer[Audio::Core::uFrameLength];
//	F32 ampbuffer[Audio::Core::uFrameLength];
//	F32 testBufferSorensen[Audio::Core::uFrameLength];
//
//	F32 signalBuffer[Audio::Core::uFrameLength];
//
//	for(U32 i = 0; i < Audio::Core::uFrameLength;i++)
//	{   signalBuffer[i] = signal[i];
//		//memcpy(signalBuffer,signal,uFrameLength);//make a copy, don't want to affect signal
//	}
//
//	
//
//	F32 RMS = 0;
//			
//		// RMS Window calculation
////		F32  m_fRMSWindow   = m_fAverage * sampleRate;
////		F32  m_fRMSWindow1  = m_fRMSWindow - 1.0f;
//
//
//	int i =0;
//
//	//Apply Windows to signal& get RMS level
//	 U32 w = 0;
//							 
//	for( w = 0;w< Audio::Core::uFrameLength;w++)
//	{
//		//Gather RMS data
//		RMS += signalBuffer[w] * signalBuffer[w]; //square amps
//
//		//WINDOWING
//		F32 coef1=(F32)((2*PI*w)/Audio::Core::uFrameLength); //hamming window coefficients
//		hambuffer[w] = (F32)(1-(0.54 + .46*cos(coef1))) * 1; //create hamming window
//					
//		if(w>0)
//			deribuffer[w-1] = (hambuffer[w-1] - hambuffer[w]); //negative derivative
//					
//		if(w==Audio::Core::uFrameLength-1)
//			deribuffer[w] = 0; //the last index needs to be set to zero since above loop doesn't cover it
//	}
//
//	for( w = 0;w< Audio::Core::uFrameLength;w++)//extra loop since deribuffer is lookahead
//	{
//		deri_x[w] = deribuffer[w] * signalBuffer[w]; //the negative derivative scaled by signal
//		signalBuffer[w] = hambuffer[w] * signalBuffer[w]; //windowed signal
//	} 
//
//				
//	//	// Compute FFT and IFFT DE Solas FFT
//	//	//FFTReal
//	////	fft_object.do_fft (testBufferFFTReal, signal);//fft of windowed signal
//	//	fft_object.do_fft (fftBuffer, signal);//fft of windowed signal
//	//	
//	//	//FFTReal
//	//	fft_object.do_fft (deri_f, deri_x); //fft of negative derivative signal
//
//
//	//Sorensen FFT + Buffer Formatting
////**********************************************
//	//memcpy(testBufferSorensen,signal,256);
//	for(int i = 0;i<256;i++)
//	testBufferSorensen[i] = signalBuffer[i];
//
//	realfft_split(testBufferSorensen,Audio::Core::uFrameLength);
//	//Sorensen FFT
//	realfft_split(deri_x,Audio::Core::uFrameLength);
//
//	
//	//[Cian] - now format buffers for FFTReal, easier to process and i've previously written the code for that one anyway..
//	for(U32 i = 0;i <= Audio::Core::uFrameLength/2;i++)
//	{
//		fftBuffer[i] = testBufferSorensen[i];
//		deri_f[i] = deri_x[i];
//	}
//
//	for(U32 i = Audio::Core::uFrameLength/2+1; i < Audio::Core::uFrameLength;i++)
//	{
//		int writePtr =  (Audio::Core::uFrameLength - i) + Audio::Core::uFrameLength/2;
//		fftBuffer[i] = testBufferSorensen[writePtr];
//		deri_f[i] = deri_x[writePtr];
//	}
////**********************************************
////	END Sorensen FFT + Buffer Formatting
//
//
//	// Put DFT results in buffers
//
//	for (U32 i = 0; i <=Audio::Core::uFrameLength/2; i++)//only need real part
//	{
//		F32			img;
//		F32			img2;
//
//		const F32	real = fftBuffer [i];
//		const F32	real2 = deri_f [i];
//
//		if (i > 0 && i < Audio::Core::uFrameLength/2)
//		{
//			img = fftBuffer [i + Audio::Core::uFrameLength/2];
//			img2 = deri_f [i + Audio::Core::uFrameLength/2];
//		}
//		else
//		{
//			img = 0;
//			img2 = 0;
//		}
//
//		const F32	f_abs = Audio::Maths::Sqrt (real * real + img * img); //magnitude
////				const F32	f_phase=Audio::Maths::ArcTan(img/real);	//phase
//		
//		Imag_buffer[i] = img;
//		Real_buffer[i] = real;
//		
//		deri_Imag_buffer[i] = img2;
//		deri_Real_buffer[i] = real2;
//
//		ampbuffer[i] = f_abs;//put f_abs in buffer so we can calculate fundamental
//		//phasebuffer[i] = f_phase; //not used in lipsync...may be needed in future
//		//	printf ("%5d: %+12.6f %+12.6f (%12.6f)  (%12.6f)\n", i, real, img, f_abs, f_phase);
//		
//	}
//
//
////Get IFD estimate
//				//**********************************************
//	int limit = ( Audio::Core::uFrameLength/ 2 ); //only check real
//
//	F32 maxAmp= 0;
////				F32 max=0;
//	int currentIndex=0;
//
//	for ( i = 0;i <limit; i++) //search amps
//	{
//		if (maxAmp< ampbuffer[i])
//		{
//			maxAmp= ampbuffer[i]; //make this amp the max
//	currentIndex = i; //store maxamp index
//
//		}
//	}
//
//	//getting the instantaneos freq. complex formula equivilent to dividing the 2 DFT's
//	//  (b * c - a * d) / (a*a) + (b*b)   ---- a,b are coeffs of windowed DFT. c,d of negative deri DFT
//	
//	//get the cf of bin
//	F32 fftFundamental = currentIndex*(sampleRate * Audio::Core::fRcpFrameLength); //get cf of bin
//	//get power spectrum
//	F32 pwrSpec = Real_buffer[currentIndex] * Real_buffer[currentIndex] + Imag_buffer[currentIndex] * Imag_buffer[currentIndex];
//	
//	//get the other bit 
//	F32 complexMultiplication = (deri_Imag_buffer[currentIndex] * Real_buffer[currentIndex])
//				- (deri_Real_buffer[currentIndex] * Imag_buffer[currentIndex]);
//	//divide
//	F32 deviation = complexMultiplication / pwrSpec;  
//
//	//Hz conversion
//	F32 IFDFreq = deviation * (sampleRate/(2*PI ));
//
//	//Instantaneous frequency
//	IFDFreq = fftFundamental - IFDFreq;
//
//	if(IFDFreq == IFDFreq) //check if NAN
//	*PitchOutput = IFDFreq; //output pitch
//	else
//		*PitchOutput = 0;
//
//	//Calculate frame RMS value
//
//	RMS = RMS / Audio::Core::uFrameLength; //get avg
//	
//	*RMSOutput = Audio::Maths::Sqrt(RMS); //get RMS and output
//
//#endif	//	#ifndef AUDIO_DISABLE_DSP
//}


//////////////////////////////////////////////////////////////////////////////////
////	ProcessDSP
//////////////////////////////////////////////////////////////////////////////////
//bool Audio::Core::DSP::ProcessDSP(int eType, void* pData, int iNumChannels, void* pFrameData, const ProcessInfo& info, bool bProcess)
//{
//#ifndef AUDIO_SPU_MINIMAL
//	switch (eType)
//	{
//	case Effect::eLipSync:
//		{
//			LipSync* pThis = (LipSync*)pData;
//			
//			F32* aSample = (F32*)pFrameData;
//		
//			//  each channel
//			for (U32 uChannel = 0; uChannel < pThis->m_uChannels; uChannel++)
//			{
//				//LIPSYNC MAIN ALGORITHM
//				//*********************************
//
//				F32 IFDFreq = 0;
//				F32 RMSValue = 0;
//
//				GetPitchRMS(aSample,fSampleRate,&IFDFreq, &RMSValue, pThis);
//
//				F32 scaledPitch = powf(IFDFreq/pThis->m_ffreqdiv, pThis->m_fcurve); //1926 is avg pitch of maledialog1
//				F32 scaledRMS = RMSValue * pThis->m_fscale;
//
//				F32 smoothedRMS = pThis->m_fprevRMS * pThis->m_fblendRMS + scaledRMS * (1-pThis->m_fblendRMS);
//				F32 smoothedPitch = pThis->m_fprevPitch * pThis->m_fblendPitch + scaledPitch * (1-pThis->m_fblendPitch);
//
//				F32 scaledSib = smoothedPitch * pThis->m_fsibScale;
//
//				F32 smoothedSib = pThis->m_fprevRMS * pThis->m_fblendSib + scaledSib * (1-pThis->m_fblendSib);
//
//				F32 SibOutput = smoothedRMS;
//
//				if(smoothedRMS < pThis->m_fsibThreshold && smoothedPitch > pThis->m_fpitchThreshold) //if sib
//				{
//					//if sibilent, set rms value to sib value
//					SibOutput = smoothedSib;
//					smoothedPitch += (pThis->m_fclosedMouthWidth - smoothedPitch) * pThis->m_fsibSmoothScale; //fairly simple smoothing
//
//				}
//				else
//				if(smoothedRMS < pThis->m_fsilenceThreshold) //if silence
//				{
//					//if silent and not sibilent, rms remains normal
//					
//					//new bit
//					smoothedPitch += (pThis->m_fclosedMouthWidth - smoothedPitch) * pThis->m_fsilSmoothScale; //fairly simple smoothing
//					smoothedRMS = pThis->m_fsilenceThreshold;
//					
//					if(IFDFreq == 0 && RMSValue < pThis->m_ftotalSilence)
//						smoothedPitch = pThis->m_fclosedMouthWidth;
//					
//				}
//
//			
//				pThis->m_fprevRMS = smoothedRMS; //store for next pass
//				pThis->m_fprevOrigRMS = RMSValue;
//				pThis->m_fprevPitch = smoothedPitch;
//				pThis->m_fprevSib = smoothedSib;
//				pThis->m_fprevSibOutput = SibOutput;
//
//				if(RMSValue < pThis->m_ftotalSilence)//IFDFreq == 0 && 
//					pThis->m_fSibDiff = 0; //prevent jitter
//				else
//				pThis->m_fSibDiff = (SibOutput-smoothedRMS) * pThis->m_fSibPostScale;
//
//
//				if(!pThis->m_fSibDiff && (smoothedRMS <= pThis->m_fsilenceThreshold) )//RMSValue< totalSilence && prevOrigRMS < totalSilence
//				{
//					smoothedRMS = pThis->m_fsilenceThreshold; 
//					smoothedPitch = pThis->m_fclosedMouthWidth; 
//				}
//
//				//get avg over frames
//				pThis->m_fSubFrameRunningTotalRMS		+= smoothedRMS;
//				pThis->m_fSubFrameRunningTotalPitch	+= smoothedPitch;
//				pThis->m_fSubFrameRunningTotalSibDiff	+= pThis->m_fSibDiff;
//
//				if (info.uFrameMod == info.uFrameInt - 1)
//				{
//							//set output variables
//					pThis->m_fRMSOutput = pThis->m_fSubFrameRunningTotalRMS * info.fWidth;
//					pThis->m_fPitchOutput = pThis->m_fSubFrameRunningTotalPitch * info.fWidth;
//					pThis->m_fSibDifference = pThis->m_fSubFrameRunningTotalSibDiff * info.fWidth;
//
//					pThis->m_fSubFrameRunningTotalRMS		= 0;	
//					pThis->m_fSubFrameRunningTotalPitch	= 0;	
//					pThis->m_fSubFrameRunningTotalSibDiff	= 0;
//
//				}
//				//else
//				//{//set to zero between frame calls
//				//		pThis->m_fRMSOutput = 0;
//				//		pThis->m_fPitchOutput = 0;
//				//		pThis->m_fSibDifference =0;
//				//}
//			}
//		}
//		break;
//	case Effect::eIIRLowPass1stOrder:
//		{
//			IIRLowPass1stOrder* pThis = (IIRLowPass1stOrder*)pData;
//
//			//	limit freq sweep rate
//			F32 fCutoffDelta = (pThis->m_fTargetCutoff - pThis->m_fCutoff);
//			F32 fStep = pThis->m_fCutoff * fFrameLength * 0.001953125f; // 1/512 - 1oct per 256 samp frame
//			fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
//			pThis->m_fCutoff += fCutoffDelta;
//
//			if (pThis->m_fCutoff != pThis->m_fPrevCutoff)
//			{
//				pThis->m_fB = Maths::ArcTan(Maths::fPi * pThis->m_fCutoff * fRcpSampleRate);
//				pThis->m_fB = -(pThis->m_fB - 1.f) * Maths::Rcp(1.f + pThis->m_fB);
//				pThis->m_fA = 0.5f * (1.f - pThis->m_fB);
//
//				pThis->m_fPrevCutoff = pThis->m_fCutoff;
//			}
//		    
//			F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//		    
//			#if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_LOW_PASS_1 ) // Version 0 : Original // JASOND
//		    
//			//  each channel
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				F32 fIn1  = pThis->m_fIn1[uCh];
//				F32 fOut1 = pThis->m_fOut1[uCh];
//		        
//				//  each sample
//				for ( U32 uSample = 0; uSample < uFrameLength; uSample++ )
//				{
//					const S32 i = uCh * uFrameLength + uSample;
//		            
//					const F32 fIn0  = aSample[i];
//		            
//					const F32 fOut0 = pThis->m_fA * ( fIn0 + fIn1 ) +
//									  pThis->m_fB * fOut1;
//					
//					aSample[i] = pThis->m_fGain * fOut0;
//		            
//					fIn1  = fIn0;
//					fOut1 = fOut0;
//				}
//		        
//				//  store previous samples
//				pThis->m_fIn1[uCh] = fIn1;
//				pThis->m_fOut1[uCh] = fOut1;
//			}
//		    
//			#elif 0 // Version 1 : Expanded & Unrolled 4 times
//		    
//			const F32 fA( pThis->m_fA );
//		    
//			const F32 fB1( pThis->m_fB );
//			const F32 fB2( fB1 * fB1 );
//			const F32 fB3( fB1 * fB2 );
//			const F32 fB4( fB2 * fB2 );
//		    
//			const F32 fGain( pThis->m_fGain );
//		    
//			//  each channel
//			S32 i = 0;
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				F32 fIn0  = pThis->m_fIn0[uCh];
//				F32 fIn1  = pThis->m_fIn1[uCh];
//				F32 fIn2  = pThis->m_fIn2[uCh];
//				F32 fIn3  = pThis->m_fIn3[uCh];
//				F32 fIn4;
//				F32 fIn5;
//				F32 fIn6;
//				F32 fIn7;
//		        
//				F32 fOut0 = pThis->m_fOut0[uCh];
//				F32 fOut1 = pThis->m_fOut1[uCh];
//				F32 fOut2 = pThis->m_fOut2[uCh];
//				F32 fOut3 = pThis->m_fOut3[uCh];
//				F32 fOut4;
//				F32 fOut5;
//				F32 fOut6;
//				F32 fOut7;
//		        
//				F32 fA_x_fIn0_plus_fIn1 = fA * ( fIn0 + fIn1 );
//				F32 fA_x_fIn1_plus_fIn2 = fA * ( fIn1 + fIn2 );
//				F32 fA_x_fIn2_plus_fIn3 = fA * ( fIn2 + fIn3 );
//				F32 fA_x_fIn3_plus_fIn4;
//				F32 fA_x_fIn4_plus_fIn5;
//				F32 fA_x_fIn5_plus_fIn6;
//				F32 fA_x_fIn6_plus_fIn7;
//		        
//				//  each sample
//				for ( U32 uSample = 0; uSample < uFrameLength; uSample+=4, i+=4 )
//				{
//					//const F32 fIn4 = aSample[i+0];
//					//const F32 fIn5 = aSample[i+1];
//					//const F32 fIn6 = aSample[i+2];
//					//const F32 fIn7 = aSample[i+3];
//					fIn4 = aSample[i+0];
//					fIn5 = aSample[i+1];
//					fIn6 = aSample[i+2];
//					fIn7 = aSample[i+3];
//		            
//					//fA_x_fIn0_plus_fIn1 = fA * ( fIn0 + fIn1 ); // Same as fA_x_fIn4_plus_fIn5 from previous loop!
//					//fA_x_fIn1_plus_fIn2 = fA * ( fIn1 + fIn2 ); // Same as fA_x_fIn5_plus_fIn6 from previous loop!
//					//fA_x_fIn2_plus_fIn3 = fA * ( fIn2 + fIn3 ); // Same as fA_x_fIn6_plus_fIn7 from previous loop!
//					fA_x_fIn3_plus_fIn4 = fA * ( fIn3 + fIn4 );
//					fA_x_fIn4_plus_fIn5 = fA * ( fIn4 + fIn5 );
//					fA_x_fIn5_plus_fIn6 = fA * ( fIn5 + fIn6 );
//					fA_x_fIn6_plus_fIn7 = fA * ( fIn6 + fIn7 );
//		            
//					// const F32 fOut4 = fA * ( fIn3 + fIn4 ) +
//					//                   fB * ( fA * ( fIn2 + fIn3 ) +
//					//                          fB * ( fA * ( fIn1 + fIn2 ) +
//					//                                 fB * ( fA * ( fIn0 + fIn1 ) +
//					//                                        fB * fOut0 ) ) );
//					
//					//const F32 fOut4 = fB4 * fOut0
//					//                + fB3 * fA_x_fIn0_plus_fIn1
//					//                + fB2 * fA_x_fIn1_plus_fIn2
//					//                + fB1 * fA_x_fIn2_plus_fIn3
//					//                +       fA_x_fIn3_plus_fIn4;
//					//
//					//const F32 fOut5 = fB4 * fOut1
//					//                + fB3 * fA_x_fIn1_plus_fIn2
//					//                + fB2 * fA_x_fIn2_plus_fIn3
//					//                + fB1 * fA_x_fIn3_plus_fIn4
//					//                +       fA_x_fIn4_plus_fIn5;
//					//
//					//const F32 fOut6 = fB4 * fOut2
//					//                + fB3 * fA_x_fIn2_plus_fIn3
//					//                + fB2 * fA_x_fIn3_plus_fIn4
//					//                + fB1 * fA_x_fIn4_plus_fIn5
//					//                +       fA_x_fIn5_plus_fIn6;
//					//
//					//const F32 fOut7 = fB4 * fOut3
//					//                + fB3 * fA_x_fIn3_plus_fIn4
//					//                + fB2 * fA_x_fIn4_plus_fIn5
//					//                + fB1 * fA_x_fIn5_plus_fIn6
//					//                +       fA_x_fIn6_plus_fIn7;
//		            
//					fOut4 = fA_x_fIn3_plus_fIn4;
//					fOut5 = fA_x_fIn4_plus_fIn5;
//					fOut6 = fA_x_fIn5_plus_fIn6;
//					fOut7 = fA_x_fIn6_plus_fIn7;
//		            
//					fOut4 += fB1 * fA_x_fIn2_plus_fIn3;
//					fOut5 += fB1 * fA_x_fIn3_plus_fIn4;
//					fOut6 += fB1 * fA_x_fIn4_plus_fIn5;
//					fOut7 += fB1 * fA_x_fIn5_plus_fIn6;
//		            
//					fOut4 += fB2 * fA_x_fIn1_plus_fIn2;
//					fOut5 += fB2 * fA_x_fIn2_plus_fIn3;
//					fOut6 += fB2 * fA_x_fIn3_plus_fIn4;
//					fOut7 += fB2 * fA_x_fIn4_plus_fIn5;
//		            
//					fOut4 += fB3 * fA_x_fIn0_plus_fIn1;
//					fOut5 += fB3 * fA_x_fIn1_plus_fIn2;
//					fOut6 += fB3 * fA_x_fIn2_plus_fIn3;
//					fOut7 += fB3 * fA_x_fIn3_plus_fIn4;
//		            
//					fOut4 += fB4 * fOut0;
//					fOut5 += fB4 * fOut1;
//					fOut6 += fB4 * fOut2;
//					fOut7 += fB4 * fOut3;
//					
//					aSample[i+0] = fGain * fOut4;
//					aSample[i+1] = fGain * fOut5;
//					aSample[i+2] = fGain * fOut6;
//					aSample[i+3] = fGain * fOut7;
//		            
//					fA_x_fIn0_plus_fIn1 = fA_x_fIn4_plus_fIn5;
//					fA_x_fIn1_plus_fIn2 = fA_x_fIn5_plus_fIn6;
//					fA_x_fIn2_plus_fIn3 = fA_x_fIn6_plus_fIn7;
//					//fIn0 = fIn4; // Not needed in loop but MUST be accounted for before storing previous samples after loop!
//					//fIn1 = fIn5;
//					//fIn2 = fIn6;
//					fIn3 = fIn7; // This one IS needed for calculating fA_x_fIn3_plus_fIn4 in next iteration of loop
//		            
//					fOut0 = fOut4;
//					fOut1 = fOut5;
//					fOut2 = fOut6;
//					fOut3 = fOut7;
//				}
//		        
//				fIn0 = fIn4; // Account for optimisation AFTER loop!
//				fIn1 = fIn5;
//				fIn2 = fIn6;
//		        
//				//  store previous samples
//		        
//				pThis->m_fIn0[uCh] = fIn0;
//				pThis->m_fIn1[uCh] = fIn1;
//				pThis->m_fIn2[uCh] = fIn2;
//				pThis->m_fIn3[uCh] = fIn3;
//		        
//				pThis->m_fOut0[uCh] = fOut0;
//				pThis->m_fOut1[uCh] = fOut1;
//				pThis->m_fOut2[uCh] = fOut2;
//				pThis->m_fOut3[uCh] = fOut3;
//			}
//		    
//			#elif 0 // Version 2 : Expanded, Unrolled 8 times & Vectorised
//		    
//			using namespace Audio::Maths;
//		    
//			const Vector4 fA( MakeVector4( pThis->m_fA ) );
//		    
//			const Vector4 fB1( MakeVector4( pThis->m_fB ) );
//			const Vector4 fB2( fB1 * fB1 );
//			const Vector4 fB3( fB1 * fB2 );
//			const Vector4 fB4( fB2 * fB2 );
//			const Vector4 fB5( fB2 * fB3 );
//			const Vector4 fB6( fB3 * fB3 );
//			const Vector4 fB7( fB3 * fB4 );
//			const Vector4 fB8( fB4 * fB4 );
//		    
//			const Vector4 fGain( MakeVector4( pThis->m_fGain ) );
//		    
//			//  each channel
//			S32 i = 0;
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				Vector4 fIn0123( pThis->m_fIn0123[uCh] );
//				Vector4 fIn4567( pThis->m_fIn4567[uCh] );
//		        
//				Vector4 fOut0123( pThis->m_fOut0123[uCh] );
//				Vector4 fOut4567( pThis->m_fOut4567[uCh] );
//		        
//				//  each sample
//				for ( U32 uSample = 0; uSample < uFrameLength; uSample+=8, i+=8 )
//				{
//					const Vector4 fIn89AB( LoadAligned( &aSample[i+0] ) );
//					const Vector4 fInCDEF( LoadAligned( &aSample[i+4] ) );
//		            
//					#if defined( PLATFORpThis->m_XENON )
//					__dcbt( 0x80, &aSample[i+0] );
//					#endif
//		            
//					const Vector4 fA_x_fIn0123_plus_fIn1234( fA * ( fIn0123 + RotateLeft<1>( fIn0123, fIn4567 ) ) );
//					const Vector4 fA_x_fIn4567_plus_fIn5678( fA * ( fIn4567 + RotateLeft<1>( fIn4567, fIn89AB ) ) );
//					const Vector4 fA_x_fIn89AB_plus_fIn9ABC( fA * ( fIn89AB + RotateLeft<1>( fIn89AB, fInCDEF ) ) );
//					const Vector4 fA_x_fInCDEF_plus_fInDEF0( fA * ( fInCDEF + RotateLeft<1>( fInCDEF, fIn0123 ) ) );
//		            
//					const Vector4 fA_x_fInBCDE_plus_fInCDEF( RotateLeft<3>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
//					const Vector4 fA_x_fInABCD_plus_fInBCDE( RotateLeft<2>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
//					const Vector4 fA_x_fIn9ABC_plus_fInABCD( RotateLeft<1>( fA_x_fIn89AB_plus_fIn9ABC, fA_x_fInCDEF_plus_fInDEF0 ) );
//		            
//					const Vector4 fA_x_fIn789A_plus_fIn89AB( RotateLeft<3>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
//					const Vector4 fA_x_fIn6789_plus_fIn789A( RotateLeft<2>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
//					const Vector4 fA_x_fIn5678_plus_fIn6789( RotateLeft<1>( fA_x_fIn4567_plus_fIn5678, fA_x_fIn89AB_plus_fIn9ABC ) );
//		            
//					const Vector4 fA_x_fIn3456_plus_fIn4567( RotateLeft<3>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
//					const Vector4 fA_x_fIn2345_plus_fIn3456( RotateLeft<2>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
//					const Vector4 fA_x_fIn1234_plus_fIn2345( RotateLeft<1>( fA_x_fIn0123_plus_fIn1234, fA_x_fIn4567_plus_fIn5678 ) );
//		            
//					Vector4 fOut89AB = fA_x_fIn789A_plus_fIn89AB +
//						   ( ( ( fB1 * fA_x_fIn6789_plus_fIn789A +
//								 fB2 * fA_x_fIn5678_plus_fIn6789 ) +
//							   ( fB3 * fA_x_fIn4567_plus_fIn5678 +
//								 fB4 * fA_x_fIn3456_plus_fIn4567 ) ) +
//							 ( ( fB5 * fA_x_fIn2345_plus_fIn3456 +
//								 fB6 * fA_x_fIn1234_plus_fIn2345 ) +
//							   ( fB7 * fA_x_fIn0123_plus_fIn1234 +
//								 fB8 * fOut0123 ) ) );
//		            
//					Vector4 fOutCDEF = fA_x_fInBCDE_plus_fInCDEF +
//						   ( ( ( fB1 * fA_x_fInABCD_plus_fInBCDE +
//								 fB2 * fA_x_fIn9ABC_plus_fInABCD ) +
//							   ( fB3 * fA_x_fIn89AB_plus_fIn9ABC +
//								 fB4 * fA_x_fIn789A_plus_fIn89AB ) ) +
//							 ( ( fB5 * fA_x_fIn6789_plus_fIn789A +
//								 fB6 * fA_x_fIn5678_plus_fIn6789 ) +
//							   ( fB7 * fA_x_fIn4567_plus_fIn5678 +
//								 fB8 * fOut4567 ) ) );
//		            
//					StoreAligned( fGain * fOut89AB, &aSample[i+0] );
//					StoreAligned( fGain * fOutCDEF, &aSample[i+4] );
//		            
//					fIn0123 = fIn89AB;
//					fIn4567 = fInCDEF;
//		            
//					fOut0123 = fOut89AB;
//					fOut4567 = fOutCDEF;
//				}
//		        
//				//  store previous samples
//		        
//				pThis->m_fIn0123[uCh] = fIn0123;
//				pThis->m_fIn4567[uCh] = fIn4567;
//		        
//				pThis->m_fOut0123[uCh] = fOut0123;
//				pThis->m_fOut4567[uCh] = fOut4567;
//			}
//		    
//			#elif 0 // Version 3 : Expanded, Unrolled 16 times & Vectorised
//		    
//			using namespace Audio::Maths;
//		    
//			const Vector4 fGain( MakeVector4( pThis->m_fGain ) );
//		    
//			const Vector4 fA( MakeVector4( pThis->m_fA ) );
//		    
//			const Vector4 fB01( MakeVector4( pThis->m_fB ) );
//			const Vector4 fB02( fB01 * fB01 );
//			const Vector4 fB03( fB01 * fB02 );
//			const Vector4 fB04( fB02 * fB02 );
//			const Vector4 fB05( fB02 * fB03 );
//			const Vector4 fB06( fB03 * fB03 );
//			const Vector4 fB07( fB03 * fB04 );
//			const Vector4 fB08( fB04 * fB04 );
//			const Vector4 fB09( fB04 * fB05 );
//			const Vector4 fB10( fB05 * fB05 );
//			const Vector4 fB11( fB05 * fB06 );
//			const Vector4 fB12( fB06 * fB06 );
//			const Vector4 fB13( fB06 * fB07 );
//			const Vector4 fB14( fB07 * fB07 );
//			const Vector4 fB15( fB07 * fB08 );
//			const Vector4 fB16( fB08 * fB08 );
//		    
//			Vector4 fIn_00_01_02_03;
//			Vector4 fIn_04_05_06_07;
//			Vector4 fIn_08_09_0A_0B;
//			Vector4 fIn_0C_0D_0E_0F;
//			Vector4 fIn_10_11_12_13;
//			Vector4 fIn_14_15_16_17;
//			Vector4 fIn_18_19_1A_1B;
//			Vector4 fIn_1C_1D_1E_1F;
//		    
//			Vector4 fB16_x_fOut_00_01_02_03;
//			Vector4 fB16_x_fOut_04_05_06_07;
//			Vector4 fB16_x_fOut_08_09_0A_0B;
//			Vector4 fB16_x_fOut_0C_0D_0E_0F;
//			Vector4 fOut_10_11_12_13;
//			Vector4 fOut_14_15_16_17;
//			Vector4 fOut_18_19_1A_1B;
//			Vector4 fOut_1C_1D_1E_1F;
//		    
//			Vector4 fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04;
//			Vector4 fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05;
//			Vector4 fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06;
//			Vector4 fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07;
//			Vector4 fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08;
//			Vector4 fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09;
//			Vector4 fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A;
//			Vector4 fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B;
//			Vector4 fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C;
//			Vector4 fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D;
//			Vector4 fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E;
//			Vector4 fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F;
//			Vector4 fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10;
//			Vector4 fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11;
//			Vector4 fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12;
//			Vector4 fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13;
//			Vector4 fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14;
//			Vector4 fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15;
//			Vector4 fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16;
//			Vector4 fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17;
//			Vector4 fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18;
//			Vector4 fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19;
//			Vector4 fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A;
//			Vector4 fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B;
//			Vector4 fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C;
//			Vector4 fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D;
//			Vector4 fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E;
//			Vector4 fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F;
//			Vector4 fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX;
//		    
//			//  each channel
//			S32 i = 0;
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				fIn_00_01_02_03 = pThis->m_fIn_00_01_02_03[uCh];
//				fIn_04_05_06_07 = pThis->m_fIn_04_05_06_07[uCh];
//				fIn_08_09_0A_0B = pThis->m_fIn_08_09_0A_0B[uCh];
//				fIn_0C_0D_0E_0F = pThis->m_fIn_0C_0D_0E_0F[uCh];
//		        
//				fB16_x_fOut_00_01_02_03 = pThis->m_fB16_x_fOut_00_01_02_03[uCh];
//				fB16_x_fOut_04_05_06_07 = pThis->m_fB16_x_fOut_04_05_06_07[uCh];
//				fB16_x_fOut_08_09_0A_0B = pThis->m_fB16_x_fOut_08_09_0A_0B[uCh];
//				fB16_x_fOut_0C_0D_0E_0F = pThis->m_fB16_x_fOut_0C_0D_0E_0F[uCh];
//		        
//				// -----------------------------------------------------------------------------------------------------------------------------------------------
//				// Loop optimisation: pre-calculate from initial 16 input values, can then be updated from next values after each iteration
//				fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA * ( fIn_00_01_02_03 + RotateLeft<1>( fIn_00_01_02_03, fIn_04_05_06_07 ) );
//				fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA * ( fIn_04_05_06_07 + RotateLeft<1>( fIn_04_05_06_07, fIn_08_09_0A_0B ) );
//				fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA * ( fIn_08_09_0A_0B + RotateLeft<1>( fIn_08_09_0A_0B, fIn_0C_0D_0E_0F ) );
//				fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 = fA * ( fIn_0C_0D_0E_0F + RotateLeft<1>( fIn_0C_0D_0E_0F, fIn_0C_0D_0E_0F ) ); // *** MUST BE RECOMPUTED IN LOOP BECAUSE RIGHTMOST ELEMENT IS INVALID! ***
//		        
//				fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = RotateLeft<1>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//				fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = RotateLeft<2>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//				fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = RotateLeft<3>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//		        
//				fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = RotateLeft<1>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//				fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = RotateLeft<2>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//				fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = RotateLeft<3>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//		        
//				fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = RotateLeft<1>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ); // These three don't actually use the invalid rightmost element!
//				fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = RotateLeft<2>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
//				fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = RotateLeft<3>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
//				// -----------------------------------------------------------------------------------------------------------------------------------------------
//		        
//				//  each sample
//				for ( U32 uSample = 0; uSample < uFrameLength; uSample+=16, i+=16 )
//				{
//					fIn_10_11_12_13 = LoadAligned( &aSample[i+0x0] );
//					fIn_14_15_16_17 = LoadAligned( &aSample[i+0x4] );
//					fIn_18_19_1A_1B = LoadAligned( &aSample[i+0x8] );
//					fIn_1C_1D_1E_1F = LoadAligned( &aSample[i+0xC] );
//		            
//					#if defined( PLATFORpThis->m_XENON )
//					__dcbt( 0x80, &aSample[i+0x0] );
//					#endif
//		            
//					// ----------------------------------------------------------------------------------------------------------------------
//					// Loop optimisation: Don't need to calc here, pre-compute before loop & update after each iteration
//					//fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA * ( fIn_00_01_02_03 + RotateLeft<1>( fIn_00_01_02_03, fIn_04_05_06_07 ) );
//					//fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA * ( fIn_04_05_06_07 + RotateLeft<1>( fIn_04_05_06_07, fIn_08_09_0A_0B ) );
//					//fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA * ( fIn_08_09_0A_0B + RotateLeft<1>( fIn_08_09_0A_0B, fIn_0C_0D_0E_0F ) );
//					// ----------------------------------------------------------------------------------------------------------------------
//					fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 = fA * ( fIn_0C_0D_0E_0F + RotateLeft<1>( fIn_0C_0D_0E_0F, fIn_10_11_12_13 ) ); // *** MUST BE RECOMPUTED IN LOOP BECAUSE RIGHTMOST ELEMENT IS INVALID! ***
//					fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 = fA * ( fIn_10_11_12_13 + RotateLeft<1>( fIn_10_11_12_13, fIn_14_15_16_17 ) );
//					fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 = fA * ( fIn_14_15_16_17 + RotateLeft<1>( fIn_14_15_16_17, fIn_18_19_1A_1B ) );
//					fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C = fA * ( fIn_18_19_1A_1B + RotateLeft<1>( fIn_18_19_1A_1B, fIn_1C_1D_1E_1F ) );
//					fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX = fA * ( fIn_1C_1D_1E_1F + RotateLeft<1>( fIn_1C_1D_1E_1F, fIn_1C_1D_1E_1F ) ); // NOTE: XX is an unused element so can safely be set to any garbage value!
//		            
//					// ---------------------------------
//					// Moved up for better scheduling
//					//fIn_00_01_02_03 = fIn_10_11_12_13; // Not needed anymore inside loop but MUST be accounted for after the loop!
//					//fIn_04_05_06_07 = fIn_14_15_16_17;
//					//fIn_08_09_0A_0B = fIn_18_19_1A_1B;
//					fIn_0C_0D_0E_0F = fIn_1C_1D_1E_1F;
//					// ---------------------------------
//		            
//					// ----------------------------------------------------------------------------------------------------------------------
//					// Loop optimisation: Don't need to calc here, pre-compute before loop & update after each iteration
//					//fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = RotateLeft<1>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//					//fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = RotateLeft<2>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//					//fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = RotateLeft<3>( fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04, fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 );
//					//
//					//fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = RotateLeft<1>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//					//fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = RotateLeft<2>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//					//fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = RotateLeft<3>( fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08, fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C );
//					//
//					//fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = RotateLeft<1>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
//					//fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = RotateLeft<2>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
//					//fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = RotateLeft<3>( fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C, fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 );
//					// ----------------------------------------------------------------------------------------------------------------------
//		            
//					fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11 = RotateLeft<1>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
//					fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 = RotateLeft<2>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
//					fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13 = RotateLeft<3>( fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10, fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 );
//		            
//					fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15 = RotateLeft<1>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
//					fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 = RotateLeft<2>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
//					fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17 = RotateLeft<3>( fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14, fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 );
//		            
//					fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19 = RotateLeft<1>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
//					fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A = RotateLeft<2>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
//					fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B = RotateLeft<3>( fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18, fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C );
//		            
//					fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D = RotateLeft<1>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
//					fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E = RotateLeft<2>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
//					fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F = RotateLeft<3>( fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C, fA_x_fIn_1C_1D_1E_1F_plus_fIn_1D_1E_1F_XX );
//		            
//					// const F32 fOut10 =        fA * ( fIn0F + fIn10 ) +
//					//                    fB1  * fA * ( fIn0E + fIn0F ) +
//					//                    fB2  * fA * ( fIn0D + fIn0E ) +
//					//                    fB3  * fA * ( fIn0C + fIn0D ) +
//					//                    fB4  * fA * ( fIn0B + fIn0C ) +
//					//                    fB5  * fA * ( fIn0A + fIn0B ) +
//					//                    fB6  * fA * ( fIn09 + fIn0A ) +
//					//                    fB7  * fA * ( fIn08 + fIn09 ) +
//					//                    fB8  * fA * ( fIn07 + fIn08 ) +
//					//                    fB9  * fA * ( fIn06 + fIn07 ) +
//					//                    fB10 * fA * ( fIn05 + fIn06 ) +
//					//                    fB11 * fA * ( fIn04 + fIn05 ) +
//					//                    fB12 * fA * ( fIn03 + fIn04 ) +
//					//                    fB13 * fA * ( fIn02 + fIn03 ) +
//					//                    fB14 * fA * ( fIn01 + fIn02 ) +
//					//                    fB15 * fA * ( fIn00 + fIn01 ) +
//					//                    fB16 *       fOut00;
//		            
//					#if 0
//		            
//					fOut_10_11_12_13 = ( ( ( (        fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
//											   fB01 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
//											 ( fB02 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
//											   fB03 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) )     +
//										   ( ( fB04 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
//											   fB05 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
//											 ( fB06 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
//											   fB07 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) ) )   +
//										 ( ( ( fB08 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B         +
//											   fB09 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A )       +
//											 ( fB10 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09         +
//											   fB11 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) )     +
//										   ( ( fB12 * fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07         +
//											   fB13 * fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 )       +
//											 ( fB14 * fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05         +
//											   fB15 * fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 ) ) ) ) +
//											   fB16_x_fOut_00_01_02_03;
//		            
//					fOut_14_15_16_17 = ( ( ( (        fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
//											   fB01 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
//											 ( fB02 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
//											   fB03 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) )     +
//										   ( ( fB04 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
//											   fB05 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
//											 ( fB06 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
//											   fB07 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) ) )   +
//										 ( ( ( fB08 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
//											   fB09 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
//											 ( fB10 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
//											   fB11 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) )     +
//										   ( ( fB12 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B         +
//											   fB13 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A )       +
//											 ( fB14 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09         +
//											   fB15 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) ) ) ) +
//											   fB16_x_fOut_04_05_06_07;
//		            
//					fOut_18_19_1A_1B = ( ( ( (        fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B         +
//											   fB01 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A )       +
//											 ( fB02 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19         +
//											   fB03 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) )     +
//										   ( ( fB04 * fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
//											   fB05 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
//											 ( fB06 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
//											   fB07 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) ) )   +
//										 ( ( ( fB08 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
//											   fB09 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
//											 ( fB10 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
//											   fB11 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) )     +
//										   ( ( fB12 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F         +
//											   fB13 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E )       +
//											 ( fB14 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D         +
//											   fB15 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) ) ) ) +
//											   fB16_x_fOut_08_09_0A_0B;
//		            
//					fOut_1C_1D_1E_1F = ( ( ( (        fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F         +
//											   fB01 * fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E )       +
//											 ( fB02 * fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D         +
//											   fB03 * fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C ) )     +
//										   ( ( fB04 * fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B         +
//											   fB05 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A )       +
//											 ( fB06 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19         +
//											   fB07 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) ) )   +
//										 ( ( ( fB08 * fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17         +
//											   fB09 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 )       +
//											 ( fB10 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15         +
//											   fB11 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) )     +
//										   ( ( fB12 * fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13         +
//											   fB13 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 )       +
//											 ( fB14 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11         +
//											   fB15 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) ) ) ) +
//											   fB16_x_fOut_0C_0D_0E_0F;
//		            
//					#else
//		            
//					// Eliminate common sub-expressions that are different only in powers of fB
//					// Now this is starting to get really messy...
//		            
//					const Vector4 tempA1 = ( (        fA_x_fIn_0F_10_11_12_plus_fIn_10_11_12_13   +
//											   fB01 * fA_x_fIn_0E_0F_10_11_plus_fIn_0F_10_11_12 ) +
//											 ( fB02 * fA_x_fIn_0D_0E_0F_10_plus_fIn_0E_0F_10_11   +
//											   fB03 * fA_x_fIn_0C_0D_0E_0F_plus_fIn_0D_0E_0F_10 ) );
//		            
//					const Vector4 tempA2 = ( ( fB04 * fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F   +
//											   fB05 * fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E ) +
//											 ( fB06 * fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D   +
//											   fB07 * fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C ) );
//		            
//					const Vector4 tempA3 = ( ( fB08 * fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B   +
//											   fB09 * fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A ) +
//											 ( fB10 * fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09   +
//											   fB11 * fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 ) );
//		            
//					const Vector4 tempA4 = ( ( fB12 * fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07   +
//											   fB13 * fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 ) +
//											 ( fB14 * fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05   +
//											   fB15 * fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 ) );
//		            
//					const Vector4 tempB1 = ( (        fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17   +
//											   fB01 * fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16 ) +
//											 ( fB02 * fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15   +
//											   fB03 * fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14 ) );
//		            
//					const Vector4 tempC1 = ( (        fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B   +
//											   fB01 * fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A ) +
//											 ( fB02 * fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19   +
//											   fB03 * fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18 ) );
//		            
//					const Vector4 tempD1 = ( (        fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F   +
//											   fB01 * fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E ) +
//											 ( fB02 * fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D   +
//											   fB03 * fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C ) );
//		            
//					fOut_10_11_12_13 = ( ( tempA1 + tempA2 ) + ( tempA3 + tempA4 ) ) + fB16_x_fOut_00_01_02_03;
//		            
//					fOut_14_15_16_17 = ( ( tempB1 + ( fB04 * tempA1 ) ) + ( ( fB04 * tempA2 ) + ( fB04 * tempA3 ) ) ) + fB16_x_fOut_04_05_06_07;
//		            
//					fOut_18_19_1A_1B = ( ( tempC1 + ( fB04 * tempB1 ) ) + ( ( fB08 * tempA1 ) + ( fB08 * tempA2 ) ) ) + fB16_x_fOut_08_09_0A_0B;
//		            
//					fOut_1C_1D_1E_1F = ( ( tempD1 + ( fB04 * tempC1 ) ) + ( ( fB08 * tempB1 ) + ( fB12 * tempA1 ) ) ) + fB16_x_fOut_0C_0D_0E_0F;
//		            
//					#endif
//		            
//					// -----------------------------------------------------------------------------------
//					// Loop optimisation:
//					fA_x_fIn_00_01_02_03_plus_fIn_01_02_03_04 = fA_x_fIn_10_11_12_13_plus_fIn_11_12_13_14;
//					fA_x_fIn_01_02_03_04_plus_fIn_02_03_04_05 = fA_x_fIn_11_12_13_14_plus_fIn_12_13_14_15;
//					fA_x_fIn_02_03_04_05_plus_fIn_03_04_05_06 = fA_x_fIn_12_13_14_15_plus_fIn_13_14_15_16;
//					fA_x_fIn_03_04_05_06_plus_fIn_04_05_06_07 = fA_x_fIn_13_14_15_16_plus_fIn_14_15_16_17;
//					fA_x_fIn_04_05_06_07_plus_fIn_05_06_07_08 = fA_x_fIn_14_15_16_17_plus_fIn_15_16_17_18;
//					fA_x_fIn_05_06_07_08_plus_fIn_06_07_08_09 = fA_x_fIn_15_16_17_18_plus_fIn_16_17_18_19;
//					fA_x_fIn_06_07_08_09_plus_fIn_07_08_09_0A = fA_x_fIn_16_17_18_19_plus_fIn_17_18_19_1A;
//					fA_x_fIn_07_08_09_0A_plus_fIn_08_09_0A_0B = fA_x_fIn_17_18_19_1A_plus_fIn_18_19_1A_1B;
//					fA_x_fIn_08_09_0A_0B_plus_fIn_09_0A_0B_0C = fA_x_fIn_18_19_1A_1B_plus_fIn_19_1A_1B_1C;
//					fA_x_fIn_09_0A_0B_0C_plus_fIn_0A_0B_0C_0D = fA_x_fIn_19_1A_1B_1C_plus_fIn_1A_1B_1C_1D;
//					fA_x_fIn_0A_0B_0C_0D_plus_fIn_0B_0C_0D_0E = fA_x_fIn_1A_1B_1C_1D_plus_fIn_1B_1C_1D_1E;
//					fA_x_fIn_0B_0C_0D_0E_plus_fIn_0C_0D_0E_0F = fA_x_fIn_1B_1C_1D_1E_plus_fIn_1C_1D_1E_1F;
//					// -----------------------------------------------------------------------------------
//		            
//					StoreAligned( fGain * fOut_10_11_12_13, &aSample[i+0x0] );
//					StoreAligned( fGain * fOut_14_15_16_17, &aSample[i+0x4] );
//					StoreAligned( fGain * fOut_18_19_1A_1B, &aSample[i+0x8] );
//					StoreAligned( fGain * fOut_1C_1D_1E_1F, &aSample[i+0xC] );
//		            
//					// ---------------------------------
//					// Moved up for better scheduling
//					////fIn_00_01_02_03 = fIn_10_11_12_13; // Not needed anymore inside loop but MUST be accounted for after the loop!
//					////fIn_04_05_06_07 = fIn_14_15_16_17;
//					////fIn_08_09_0A_0B = fIn_18_19_1A_1B;
//					//fIn_0C_0D_0E_0F = fIn_1C_1D_1E_1F;
//					// ---------------------------------
//		            
//					fB16_x_fOut_00_01_02_03 = fB16 * fOut_10_11_12_13;
//					fB16_x_fOut_04_05_06_07 = fB16 * fOut_14_15_16_17;
//					fB16_x_fOut_08_09_0A_0B = fB16 * fOut_18_19_1A_1B;
//					fB16_x_fOut_0C_0D_0E_0F = fB16 * fOut_1C_1D_1E_1F;
//				}
//		        
//				// ------------------------------------------------------------------------
//				// Not needed anymore inside loop but MUST be accounted for after the loop!
//				fIn_00_01_02_03 = fIn_10_11_12_13;
//				fIn_04_05_06_07 = fIn_14_15_16_17;
//				fIn_08_09_0A_0B = fIn_18_19_1A_1B;
//				// ------------------------------------------------------------------------
//		        
//				//  store previous samples
//		        
//				pThis->m_fIn_00_01_02_03[uCh] = fIn_00_01_02_03;
//				pThis->m_fIn_04_05_06_07[uCh] = fIn_04_05_06_07;
//				pThis->m_fIn_08_09_0A_0B[uCh] = fIn_08_09_0A_0B;
//				pThis->m_fIn_0C_0D_0E_0F[uCh] = fIn_0C_0D_0E_0F;
//		        
//				pThis->m_fB16_x_fOut_00_01_02_03[uCh] = fB16_x_fOut_00_01_02_03;
//				pThis->m_fB16_x_fOut_04_05_06_07[uCh] = fB16_x_fOut_04_05_06_07;
//				pThis->m_fB16_x_fOut_08_09_0A_0B[uCh] = fB16_x_fOut_08_09_0A_0B;
//				pThis->m_fB16_x_fOut_0C_0D_0E_0F[uCh] = fB16_x_fOut_0C_0D_0E_0F;
//			}
//		    
//			#else
//		    
//			using namespace Audio::Maths;
//		    
//			Vector4* pBuffer = (Vector4*)pFrameData;
//		    
//			const Scalar vGain = _Scalar( pThis->m_fGain );
//		    
//			const Scalar vB1 = _Scalar( pThis->m_fB );
//			const Scalar vB2 = vB1 * vB1;
//			const Scalar vB3 = vB1 * vB2;
//			const Scalar vB4 = vB2 * vB2;
//		    
//			const Scalar vA = _Scalar( pThis->m_fA );
//			const Scalar vA_x_vB1 = vA * vB1;
//			const Scalar vA_x_vB2 = vA * vB2;
//			const Scalar vA_x_vB3 = vA * vB3;
//		    
//			const Vector4 v4_zero = MakeVector4( g_VectorZero );
//		    
//			Vector4 vIn_00_01_02_03 = v4_zero;
//			Vector4 vIn_01_02_03_04 = v4_zero;
//			Vector4 vIn_02_03_04_05 = v4_zero;
//			Vector4 vIn_03_04_05_06 = v4_zero;
//			Vector4 vIn_04_05_06_07 = v4_zero;
//			Vector4 vIn_05_06_07_08 = v4_zero;
//			Vector4 vIn_06_07_08_09 = v4_zero;
//			Vector4 vIn_07_08_09_0A = v4_zero;
//			Vector4 vIn_08_09_0A_0B = v4_zero;
//			Vector4 vIn_09_0A_0B_0C = v4_zero;
//			Vector4 vIn_0A_0B_0C_0D = v4_zero;
//			Vector4 vIn_0B_0C_0D_0E = v4_zero;
//			Vector4 vIn_0C_0D_0E_0F = v4_zero;
//			Vector4 vIn_0D_0E_0F_10 = v4_zero;
//			Vector4 vIn_0E_0F_10_11 = v4_zero;
//			Vector4 vIn_0F_10_11_12 = v4_zero;
//			Vector4 vIn_10_11_12_13 = v4_zero;
//		    
//			Vector4 vOut_00_01_02_03 = v4_zero;
//			Vector4 vOut_04_05_06_07 = v4_zero;
//			Vector4 vOut_08_09_0A_0B = v4_zero;
//			Vector4 vOut_0C_0D_0E_0F = v4_zero;
//			Vector4 vOut_10_11_12_13 = v4_zero;
//		    
//			#if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
//			const Vector4 v_AB3_AB2_AB1_A = MakeVector4( vA_x_vB3, vA_x_vB2, vA_x_vB1, vA );
//			#endif
//		    
//			//  each channel
//			U32 uQuadSampleIndex = 0;
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				 vIn_10_11_12_13 =  pThis->m_vIn_00_01_02_03[uCh];
//				vOut_10_11_12_13 = pThis->m_vOut_00_01_02_03[uCh];
//		        
//				for ( U32 uSampleIndex=0; uSampleIndex < uFrameLength; uSampleIndex += 16 )
//				{
//					 vIn_00_01_02_03 =  vIn_10_11_12_13;
//					vOut_00_01_02_03 = vOut_10_11_12_13;
//		            
//					vIn_04_05_06_07 = pBuffer[ uQuadSampleIndex + 0 ];
//					vIn_08_09_0A_0B = pBuffer[ uQuadSampleIndex + 1 ];
//					vIn_0C_0D_0E_0F = pBuffer[ uQuadSampleIndex + 2 ];
//					vIn_10_11_12_13 = pBuffer[ uQuadSampleIndex + 3 ];
//		            
//					vIn_01_02_03_04 = RotateLeft<1>( vIn_00_01_02_03, vIn_04_05_06_07 );
//					vIn_02_03_04_05 = RotateLeft<2>( vIn_00_01_02_03, vIn_04_05_06_07 );
//					vIn_03_04_05_06 = RotateLeft<3>( vIn_00_01_02_03, vIn_04_05_06_07 );
//		            
//					vIn_05_06_07_08 = RotateLeft<1>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//					vIn_06_07_08_09 = RotateLeft<2>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//					vIn_07_08_09_0A = RotateLeft<3>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//		            
//					vIn_09_0A_0B_0C = RotateLeft<1>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//					vIn_0A_0B_0C_0D = RotateLeft<2>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//					vIn_0B_0C_0D_0E = RotateLeft<3>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//		            
//					vIn_0D_0E_0F_10 = RotateLeft<1>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//					vIn_0E_0F_10_11 = RotateLeft<2>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//					vIn_0F_10_11_12 = RotateLeft<3>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//		            
//					const Vector4 vSum_00_01_02_03 = vIn_00_01_02_03 + vIn_01_02_03_04;
//					const Vector4 vSum_01_02_03_04 = vIn_01_02_03_04 + vIn_02_03_04_05;
//					const Vector4 vSum_02_03_04_05 = vIn_02_03_04_05 + vIn_03_04_05_06;
//					const Vector4 vSum_03_04_05_06 = vIn_03_04_05_06 + vIn_04_05_06_07;
//					const Vector4 vSum_04_05_06_07 = vIn_04_05_06_07 + vIn_05_06_07_08;
//					const Vector4 vSum_05_06_07_08 = vIn_05_06_07_08 + vIn_06_07_08_09;
//					const Vector4 vSum_06_07_08_09 = vIn_06_07_08_09 + vIn_07_08_09_0A;
//					const Vector4 vSum_07_08_09_0A = vIn_07_08_09_0A + vIn_08_09_0A_0B;
//					const Vector4 vSum_08_09_0A_0B = vIn_08_09_0A_0B + vIn_09_0A_0B_0C;
//					const Vector4 vSum_09_0A_0B_0C = vIn_09_0A_0B_0C + vIn_0A_0B_0C_0D;
//					const Vector4 vSum_0A_0B_0C_0D = vIn_0A_0B_0C_0D + vIn_0B_0C_0D_0E;
//					const Vector4 vSum_0B_0C_0D_0E = vIn_0B_0C_0D_0E + vIn_0C_0D_0E_0F;
//					const Vector4 vSum_0C_0D_0E_0F = vIn_0C_0D_0E_0F + vIn_0D_0E_0F_10;
//					const Vector4 vSum_0D_0E_0F_10 = vIn_0D_0E_0F_10 + vIn_0E_0F_10_11;
//					const Vector4 vSum_0E_0F_10_11 = vIn_0E_0F_10_11 + vIn_0F_10_11_12;
//					const Vector4 vSum_0F_10_11_12 = vIn_0F_10_11_12 + vIn_10_11_12_13;
//#define _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_		            
//					#if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
//		            
//					vOut_04_05_06_07 = MakeVector4( v_AB3_AB2_AB1_A % vSum_00_01_02_03,
//													v_AB3_AB2_AB1_A % vSum_01_02_03_04,
//													v_AB3_AB2_AB1_A % vSum_02_03_04_05, 
//													v_AB3_AB2_AB1_A % vSum_03_04_05_06 ) +
//									   vB4 * vOut_00_01_02_03;
//		            
//					vOut_08_09_0A_0B = MakeVector4( v_AB3_AB2_AB1_A % vSum_04_05_06_07,
//													v_AB3_AB2_AB1_A % vSum_05_06_07_08,
//													v_AB3_AB2_AB1_A % vSum_06_07_08_09,
//													v_AB3_AB2_AB1_A % vSum_07_08_09_0A ) +
//									   vB4 * vOut_04_05_06_07;
//		            
//					vOut_0C_0D_0E_0F = MakeVector4( v_AB3_AB2_AB1_A % vSum_08_09_0A_0B,
//													v_AB3_AB2_AB1_A % vSum_09_0A_0B_0C,
//													v_AB3_AB2_AB1_A % vSum_0A_0B_0C_0D,
//													v_AB3_AB2_AB1_A % vSum_0B_0C_0D_0E ) +
//									   vB4 * vOut_08_09_0A_0B;
//		            
//					vOut_10_11_12_13 = MakeVector4( v_AB3_AB2_AB1_A % vSum_0C_0D_0E_0F,
//													v_AB3_AB2_AB1_A % vSum_0D_0E_0F_10,
//													v_AB3_AB2_AB1_A % vSum_0E_0F_10_11,
//													v_AB3_AB2_AB1_A % vSum_0F_10_11_12 ) +
//									   vB4 * vOut_0C_0D_0E_0F;
//		            
//					#else
//		            
//					vOut_04_05_06_07 = ( ( vA       * vSum_03_04_05_06 +
//										   vA_x_vB1 * vSum_02_03_04_05 ) +
//										 ( vA_x_vB2 * vSum_01_02_03_04 +
//										   vA_x_vB3 * vSum_00_01_02_03 ) ) +
//												vB4 * vOut_00_01_02_03;
//		            
//					vOut_08_09_0A_0B = ( ( vA       * vSum_07_08_09_0A +
//										   vA_x_vB1 * vSum_06_07_08_09 ) +
//										 ( vA_x_vB2 * vSum_05_06_07_08 +
//										   vA_x_vB3 * vSum_04_05_06_07 ) ) +
//												vB4 * vOut_04_05_06_07;
//		            
//					vOut_0C_0D_0E_0F = ( ( vA       * vSum_0B_0C_0D_0E +
//										   vA_x_vB1 * vSum_0A_0B_0C_0D ) +
//										 ( vA_x_vB2 * vSum_09_0A_0B_0C +
//										   vA_x_vB3 * vSum_08_09_0A_0B ) ) +
//												vB4 * vOut_08_09_0A_0B;
//		            
//					vOut_10_11_12_13 = ( ( vA       * vSum_0F_10_11_12 +
//										   vA_x_vB1 * vSum_0E_0F_10_11 ) +
//										 ( vA_x_vB2 * vSum_0D_0E_0F_10 +
//										   vA_x_vB3 * vSum_0C_0D_0E_0F ) ) +
//												vB4 * vOut_0C_0D_0E_0F;
//		            
//					#endif
//		            
//					pBuffer[ uQuadSampleIndex + 0 ] = vOut_04_05_06_07 * vGain; // TODO: Pre-multiply by gain for all but vOut_10_11_12_13
//					pBuffer[ uQuadSampleIndex + 1 ] = vOut_08_09_0A_0B * vGain;
//					pBuffer[ uQuadSampleIndex + 2 ] = vOut_0C_0D_0E_0F * vGain;
//					pBuffer[ uQuadSampleIndex + 3 ] = vOut_10_11_12_13 * vGain;
//					uQuadSampleIndex += 4;
//				}
//		        
//				 pThis->m_vIn_00_01_02_03[uCh] =  vIn_10_11_12_13;
//				pThis->m_vOut_00_01_02_03[uCh] = vOut_10_11_12_13;
//			}
//		            
//			#endif // Versions
//		    
//			return true;
//		}
//		break;
//
//	case Effect::eIIRLowPass2ndOrder:
//		{
//			IIRLowPass2ndOrder* pThis = (IIRLowPass2ndOrder*)pData;
//
//			//	limit freq sweep rate using Q
//			F32 fCutoffDelta = (pThis->m_fTargetCutoff - pThis->m_fCutoff);
//			F32 fThresh = Maths::RcpEst(Maths::Power(2.f, 3.f * pThis->m_fQ));	//	Q=0 -> 1oct, Q=1 -> 1/8oct per 256 samp frame
//			F32 fStep = fThresh * pThis->m_fCutoff * fFrameLength * 0.001953125f; // 1/512
//			fCutoffDelta = Maths::Clamp(fCutoffDelta, -fStep, fStep);
//			pThis->m_fCutoff += fCutoffDelta;
//
//			if ((pThis->m_fCutoff != pThis->m_fPrevCutoff) || (pThis->m_fQ != pThis->m_fPrevQ) || (pThis->m_fGain != pThis->m_fPrevGain))
//			{
//				F32 fRealQ = Maths::Power(2.f, 8.f * pThis->m_fQ);
//				F32 fOmega = Maths::fPi * pThis->m_fCutoff * Core::fRcpSampleRate;
//				F32 fSin = Maths::Sin(fOmega);
//				F32 fCos = Maths::Cos(fOmega);
//				F32 fAlpha = fSin / (2.f * fRealQ);
//				F32 fA0 = 1.f / (1.f + fAlpha);
//				pThis->m_fA1 = ((-2.f * fCos) * fA0);
//				pThis->m_fA2 = ((1.f - fAlpha) * fA0);
//				pThis->m_fB1 = ((1.f - fCos) * fA0) * pThis->m_fGain;
//				pThis->m_fB0 = pThis->m_fB1 * 0.5f;
//
//				pThis->m_fPrevCutoff = pThis->m_fCutoff;
//				pThis->m_fPrevQ = pThis->m_fQ;
//				pThis->m_fPrevGain = pThis->m_fGain;
//			}
//
//			F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//
//			F32 fPrevIn0;
//			F32 fPrevOut0;
//			F32 fPrevIn1;
//			F32 fPrevOut1;
//
//			//  each channel
//			for (U32 uCh = 0; uCh < pThis->m_uChannels; uCh++)
//			{
//				fPrevIn0 = pThis->m_fPrevIn0[uCh];
//				fPrevOut0 = pThis->m_fPrevOut0[uCh];
//				fPrevIn1 = pThis->m_fPrevIn1[uCh];
//				fPrevOut1 = pThis->m_fPrevOut1[uCh];
//
//				//  each sample
//				for (U32 i = 0; i < uFrameLength; i++)
//				{
//					U32 dwSample = uCh * uFrameLength + i;
//
//					F32 fIn = aSample[dwSample];
//					F32 fOut = pThis->m_fB0 * fIn + pThis->m_fB1 * fPrevIn0 + pThis->m_fB0 * fPrevIn1 - pThis->m_fA1 * fPrevOut0 - pThis->m_fA2 * fPrevOut1;
//
//					fOut = Maths::Clamp(fOut, -pThis->m_fClamp, pThis->m_fClamp);
//
//					fPrevIn1 = fPrevIn0;
//					fPrevOut1 = fPrevOut0;
//					fPrevIn0 = fIn;
//					fPrevOut0 = fOut;
//
//					aSample[dwSample] = fOut;
//				}
//
//				pThis->m_fPrevIn0[uCh] = fPrevIn0;
//				pThis->m_fPrevOut0[uCh] = fPrevOut0;
//				pThis->m_fPrevIn1[uCh] = fPrevIn1;
//				pThis->m_fPrevOut1[uCh] = fPrevOut1;
//			}
//		}
//		break;
//
//	case Effect::eIIRHighPass1stOrder:
//		{
//			IIRHighPass1stOrder* pThis = (IIRHighPass1stOrder*)pData;
//
//			if (pThis->m_fCutoff != pThis->m_fPrevCutoff)
//			{
//				pThis->m_fB = Maths::ArcTan(Maths::fPi * pThis->m_fCutoff * fRcpSampleRate);
//				pThis->m_fB = -(pThis->m_fB - 1.f) * Maths::Rcp(1.f + pThis->m_fB);
//				pThis->m_fA = 0.5f * (1.f + pThis->m_fB);
//
//				pThis->m_fPrevCutoff = pThis->m_fCutoff;
//			}
//
//			F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//
//		//    U32 uOffset = 0;
//#if 1
////			#if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_HIGH_PASS_1 ) // JASOND
//		    
//			//  each channel
//			for (U32 uCh = 0; uCh < pThis->m_uChannels; uCh++)
//			{
//				F32 fIn1 = pThis->m_fIn1[uCh];
//				F32 fOut1 = pThis->m_fOut1[uCh];
//
//				//  each sample
//				for (U32 uSample = 0; uSample < uFrameLength; uSample++)
//				{
//					F32 fIn  = aSample[uCh * uFrameLength + uSample];
//					F32 fOut = pThis->m_fA * (fIn - fIn1) + pThis->m_fB * fOut1;
//
//					fIn1  = fIn;
//					fOut1 = fOut;
//
//					aSample[uCh * uFrameLength + uSample] = pThis->m_fGain * fOut;
//				}
//
//				//  store previous samples
//				pThis->m_fIn1[uCh] = fIn1;
//				pThis->m_fOut1[uCh] = fOut1;
//			}
//		    
//			#else
//		    
//			using namespace Audio::Maths;
//		    
//			Vector4* pBuffer = (Vector4*)pFrameData;
//		    
//			const Scalar vGain = _Scalar( pThis->m_fGain );
//		    
//			const Scalar vB1 = _Scalar( pThis->m_fB );
//			const Scalar vB2 = vB1 * vB1;
//			const Scalar vB3 = vB1 * vB2;
//			const Scalar vB4 = vB2 * vB2;
//		    
//			const Scalar vA = _Scalar( pThis->m_fA );
//			const Scalar vA_x_vB1 = vA * vB1;
//			const Scalar vA_x_vB2 = vA * vB2;
//			const Scalar vA_x_vB3 = vA * vB3;
//		    
//			const Vector4 v4_zero = MakeVector4( g_VectorZero );
//		    
//			Vector4 vIn_00_01_02_03; // = v4_zero;
//			Vector4 vIn_01_02_03_04; // = v4_zero;
//			Vector4 vIn_02_03_04_05; // = v4_zero;
//			Vector4 vIn_03_04_05_06; // = v4_zero;
//			Vector4 vIn_04_05_06_07; // = v4_zero;
//			Vector4 vIn_05_06_07_08; // = v4_zero;
//			Vector4 vIn_06_07_08_09; // = v4_zero;
//			Vector4 vIn_07_08_09_0A; // = v4_zero;
//			Vector4 vIn_08_09_0A_0B; // = v4_zero;
//			Vector4 vIn_09_0A_0B_0C; // = v4_zero;
//			Vector4 vIn_0A_0B_0C_0D; // = v4_zero;
//			Vector4 vIn_0B_0C_0D_0E; // = v4_zero;
//			Vector4 vIn_0C_0D_0E_0F; // = v4_zero;
//			Vector4 vIn_0D_0E_0F_10; // = v4_zero;
//			Vector4 vIn_0E_0F_10_11; // = v4_zero;
//			Vector4 vIn_0F_10_11_12; // = v4_zero;
//			Vector4 vIn_10_11_12_13; // = v4_zero;
//		    
//			Vector4 vOut_00_01_02_03; // = v4_zero;
//			Vector4 vOut_04_05_06_07; // = v4_zero;
//			Vector4 vOut_08_09_0A_0B; // = v4_zero;
//			Vector4 vOut_0C_0D_0E_0F; // = v4_zero;
//			Vector4 vOut_10_11_12_13; // = v4_zero;
//		    
//			#if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
//			const Vector4 v_AB3_AB2_AB1_A = MakeVector4( vA_x_vB3, vA_x_vB2, vA_x_vB1, vA );
//			#endif
//		    
//			//  each channel
//			U32 uQuadSampleIndex = 0;
//			for ( U32 uCh = 0; uCh < pThis->m_uChannels; uCh++ )
//			{
//				 vIn_10_11_12_13 =  pThis->m_vIn_00_01_02_03[uCh];
//				vOut_10_11_12_13 = pThis->m_vOut_00_01_02_03[uCh];
//		        
//				for ( U32 uSampleIndex=0; uSampleIndex < uFrameLength; uSampleIndex += 16 )
//				{
//					 vIn_00_01_02_03 =  vIn_10_11_12_13;
//					vOut_00_01_02_03 = vOut_10_11_12_13;
//		            
//					vIn_04_05_06_07 = pBuffer[ uQuadSampleIndex + 0 ];
//					vIn_08_09_0A_0B = pBuffer[ uQuadSampleIndex + 1 ];
//					vIn_0C_0D_0E_0F = pBuffer[ uQuadSampleIndex + 2 ];
//					vIn_10_11_12_13 = pBuffer[ uQuadSampleIndex + 3 ];
//		            
//					vIn_01_02_03_04 = RotateLeft<1>( vIn_00_01_02_03, vIn_04_05_06_07 );
//					vIn_02_03_04_05 = RotateLeft<2>( vIn_00_01_02_03, vIn_04_05_06_07 );
//					vIn_03_04_05_06 = RotateLeft<3>( vIn_00_01_02_03, vIn_04_05_06_07 );
//		            
//					vIn_05_06_07_08 = RotateLeft<1>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//					vIn_06_07_08_09 = RotateLeft<2>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//					vIn_07_08_09_0A = RotateLeft<3>( vIn_04_05_06_07, vIn_08_09_0A_0B );
//		            
//					vIn_09_0A_0B_0C = RotateLeft<1>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//					vIn_0A_0B_0C_0D = RotateLeft<2>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//					vIn_0B_0C_0D_0E = RotateLeft<3>( vIn_08_09_0A_0B, vIn_0C_0D_0E_0F );
//		            
//					vIn_0D_0E_0F_10 = RotateLeft<1>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//					vIn_0E_0F_10_11 = RotateLeft<2>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//					vIn_0F_10_11_12 = RotateLeft<3>( vIn_0C_0D_0E_0F, vIn_10_11_12_13 );
//		            
//					const Vector4 vDif_00_01_02_03 = vIn_00_01_02_03 - vIn_01_02_03_04;
//					const Vector4 vDif_01_02_03_04 = vIn_01_02_03_04 - vIn_02_03_04_05;
//					const Vector4 vDif_02_03_04_05 = vIn_02_03_04_05 - vIn_03_04_05_06;
//					const Vector4 vDif_03_04_05_06 = vIn_03_04_05_06 - vIn_04_05_06_07;
//					const Vector4 vDif_04_05_06_07 = vIn_04_05_06_07 - vIn_05_06_07_08;
//					const Vector4 vDif_05_06_07_08 = vIn_05_06_07_08 - vIn_06_07_08_09;
//					const Vector4 vDif_06_07_08_09 = vIn_06_07_08_09 - vIn_07_08_09_0A;
//					const Vector4 vDif_07_08_09_0A = vIn_07_08_09_0A - vIn_08_09_0A_0B;
//					const Vector4 vDif_08_09_0A_0B = vIn_08_09_0A_0B - vIn_09_0A_0B_0C;
//					const Vector4 vDif_09_0A_0B_0C = vIn_09_0A_0B_0C - vIn_0A_0B_0C_0D;
//					const Vector4 vDif_0A_0B_0C_0D = vIn_0A_0B_0C_0D - vIn_0B_0C_0D_0E;
//					const Vector4 vDif_0B_0C_0D_0E = vIn_0B_0C_0D_0E - vIn_0C_0D_0E_0F;
//					const Vector4 vDif_0C_0D_0E_0F = vIn_0C_0D_0E_0F - vIn_0D_0E_0F_10;
//					const Vector4 vDif_0D_0E_0F_10 = vIn_0D_0E_0F_10 - vIn_0E_0F_10_11;
//					const Vector4 vDif_0E_0F_10_11 = vIn_0E_0F_10_11 - vIn_0F_10_11_12;
//					const Vector4 vDif_0F_10_11_12 = vIn_0F_10_11_12 - vIn_10_11_12_13;
//		            
//					#if defined( _AUDIO_MATHS_FAST_VECTOR4_DOT_PRODUCT_ )
//		            
//					vOut_04_05_06_07 = MakeVector4( v_AB3_AB2_AB1_A % vDif_00_01_02_03,
//													v_AB3_AB2_AB1_A % vDif_01_02_03_04,
//													v_AB3_AB2_AB1_A % vDif_02_03_04_05, 
//													v_AB3_AB2_AB1_A % vDif_03_04_05_06 ) +
//									   vB4 * vOut_00_01_02_03;
//		            
//					vOut_08_09_0A_0B = MakeVector4( v_AB3_AB2_AB1_A % vDif_04_05_06_07,
//													v_AB3_AB2_AB1_A % vDif_05_06_07_08,
//													v_AB3_AB2_AB1_A % vDif_06_07_08_09,
//													v_AB3_AB2_AB1_A % vDif_07_08_09_0A ) +
//									   vB4 * vOut_04_05_06_07;
//		            
//					vOut_0C_0D_0E_0F = MakeVector4( v_AB3_AB2_AB1_A % vDif_08_09_0A_0B,
//													v_AB3_AB2_AB1_A % vDif_09_0A_0B_0C,
//													v_AB3_AB2_AB1_A % vDif_0A_0B_0C_0D,
//													v_AB3_AB2_AB1_A % vDif_0B_0C_0D_0E ) +
//									   vB4 * vOut_08_09_0A_0B;
//		            
//					vOut_10_11_12_13 = MakeVector4( v_AB3_AB2_AB1_A % vDif_0C_0D_0E_0F,
//													v_AB3_AB2_AB1_A % vDif_0D_0E_0F_10,
//													v_AB3_AB2_AB1_A % vDif_0E_0F_10_11,
//													v_AB3_AB2_AB1_A % vDif_0F_10_11_12 ) +
//									   vB4 * vOut_0C_0D_0E_0F;
//		            
//					#else
//		            
//					vOut_04_05_06_07 = ( ( vA       * vDif_03_04_05_06 +
//										   vA_x_vB1 * vDif_02_03_04_05 ) +
//										 ( vA_x_vB2 * vDif_01_02_03_04 +
//										   vA_x_vB3 * vDif_00_01_02_03 ) ) +
//												vB4 * vOut_00_01_02_03;
//		            
//					vOut_08_09_0A_0B = ( ( vA       * vDif_07_08_09_0A +
//										   vA_x_vB1 * vDif_06_07_08_09 ) +
//										 ( vA_x_vB2 * vDif_05_06_07_08 +
//										   vA_x_vB3 * vDif_04_05_06_07 ) ) +
//												vB4 * vOut_04_05_06_07;
//		            
//					vOut_0C_0D_0E_0F = ( ( vA       * vDif_0B_0C_0D_0E +
//										   vA_x_vB1 * vDif_0A_0B_0C_0D ) +
//										 ( vA_x_vB2 * vDif_09_0A_0B_0C +
//										   vA_x_vB3 * vDif_08_09_0A_0B ) ) +
//												vB4 * vOut_08_09_0A_0B;
//		            
//					vOut_10_11_12_13 = ( ( vA       * vDif_0F_10_11_12 +
//										   vA_x_vB1 * vDif_0E_0F_10_11 ) +
//										 ( vA_x_vB2 * vDif_0D_0E_0F_10 +
//										   vA_x_vB3 * vDif_0C_0D_0E_0F ) ) +
//												vB4 * vOut_0C_0D_0E_0F;
//		            
//					#endif
//		            
//					pBuffer[ uQuadSampleIndex + 0 ] = vOut_04_05_06_07 * vGain; // TODO: Pre-multiply by gain for all but vOut_10_11_12_13
//					pBuffer[ uQuadSampleIndex + 1 ] = vOut_08_09_0A_0B * vGain;
//					pBuffer[ uQuadSampleIndex + 2 ] = vOut_0C_0D_0E_0F * vGain;
//					pBuffer[ uQuadSampleIndex + 3 ] = vOut_10_11_12_13 * vGain;
//					uQuadSampleIndex += 4;
//				}
//		        
//				 pThis->m_vIn_00_01_02_03[uCh] =  vIn_10_11_12_13;
//				pThis->m_vOut_00_01_02_03[uCh] = vOut_10_11_12_13;
//			}
//		            
//			#endif
//
//			return true;
//		}
//		break;
//
//	case Effect::eSaturator:
//		{
//			Saturator* pThis = (Saturator*)pData;
//
//			using namespace Audio::Maths;
//		    
//			#if !defined( AUDIO_MATHS_VECTOR_OPTIMISE_SATURATOR ) // JASOND
//		    
//			F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//		    
//			const F32 fPrevDrive = pThis->m_fPrevDrive;
//			const F32 fPrevClip  = pThis->m_fPrevClip;
//			const F32 fPrevLevel = pThis->m_fPrevLevel;
//		    
//			const F32 fDriveDelta = ( pThis->m_fDrive - pThis->m_fPrevDrive ) * Core::fRcpFrameLength;
//			const F32 fClipDelta  = ( pThis->m_fClip  - pThis->m_fPrevClip  ) * Core::fRcpFrameLength;
//			const F32 fLevelDelta = ( pThis->m_fLevel - pThis->m_fPrevLevel ) * Core::fRcpFrameLength;
//		    
//			const F32 fOne = 1.0f;
//			const F32 fMinusOne = -1.0f;
//		    
//			// each channel
//			U32 uSampleIndex = 0;
//			for ( U32 uChannelIndex = 0; uChannelIndex < pThis->m_uChannels; uChannelIndex++ )
//			{
//				F32 fFrameIndex = 0.0f;
//				for ( U32 uFrameIndex = 0; uFrameIndex < uFrameLength; uFrameIndex++ )
//				{
//					const F32 fSample = aSample[ uSampleIndex ];
//		            
//					const F32 fDrive = fPrevDrive + fFrameIndex * fDriveDelta;
//					const F32 fClip  = fPrevClip  + fFrameIndex * fClipDelta;
//					const F32 fLevel = fPrevLevel + fFrameIndex * fLevelDelta;
//					fFrameIndex += fOne;
//		            
//					const F32 fIn  = fDrive * Maths::Abs( fSample );
//					const F32 fSgn = Select( fSample, fOne, fMinusOne );
//					const F32 fOut = Select( fClip - fIn, fIn, fIn * RcpEst( fOne + fIn - fClip ) );
//		            
//					aSample[uSampleIndex++] = fLevel * fSgn * fOut;
//				}
//			}
//		    
//			#else
//		    
//			Vector4* __restrict aSample = (Vector4*)pFrameData;
//		    
//			const Vector4 vRecipFrameLength = MakeVector4( Core::fRcpFrameLength );
//		    
//			const Vector4 vPrevDrive = MakeVector4( pThis->m_fPrevDrive );
//			const Vector4 vPrevClip  = MakeVector4( pThis->m_fPrevClip  );
//			const Vector4 vPrevLevel = MakeVector4( pThis->m_fPrevLevel );
//		    
//			const Vector4 vDriveDelta = ( MakeVector4( pThis->m_fDrive ) - vPrevDrive ) * vRecipFrameLength;
//			const Vector4 vClipDelta  = ( MakeVector4( pThis->m_fClip  ) - vPrevClip  ) * vRecipFrameLength;
//			const Vector4 vLevelDelta = ( MakeVector4( pThis->m_fLevel ) - vPrevLevel ) * vRecipFrameLength;
//		    
//			const Vector4 vOne = MakeVector4( g_VectorOne );
//			
//			const Vector4 vSixteen = MakeVector4( g_VectorSixteen );
//		    
//			// each channel
//			U32 uQuadSampleIndex = 0;
//			for ( U32 uChannelIndex = 0; uChannelIndex < pThis->m_uChannels; uChannelIndex++ )
//			{
//				//F32 fFrameIndex = 0.0f;
//				Vector4 vFrameIndex_00_01_02_03 = MakeVector4( g_VectorFloat_00_01_02_03 );
//				Vector4 vFrameIndex_04_05_06_07 = MakeVector4( g_VectorFloat_04_05_06_07 );
//				Vector4 vFrameIndex_08_09_0A_0B = MakeVector4( g_VectorFloat_08_09_0A_0B );
//				Vector4 vFrameIndex_0C_0D_0E_0F = MakeVector4( g_VectorFloat_0C_0D_0E_0F );
//		        
//				for ( U32 uFrameIndex = 0; uFrameIndex < uFrameLength; uFrameIndex+=16 )
//				{
//					//const F32 fSample = aSample[ uSampleIndex ];
//					const Vector4 vSample_00_01_02_03 = aSample[ uQuadSampleIndex + 0 ];
//					const Vector4 vSample_04_05_06_07 = aSample[ uQuadSampleIndex + 1 ];
//					const Vector4 vSample_08_09_0A_0B = aSample[ uQuadSampleIndex + 2 ];
//					const Vector4 vSample_0C_0D_0E_0F = aSample[ uQuadSampleIndex + 3 ];
//		            
//					#if defined( PLATFORM_XENON )
//					__dcbt( 0x80, &aSample[ uQuadSampleIndex + 0 ] );
//					#endif
//		            
//					const Vector4 vAbsSample_00_01_02_03 = Abs( vSample_00_01_02_03 );
//					const Vector4 vAbsSample_04_05_06_07 = Abs( vSample_04_05_06_07 );
//					const Vector4 vAbsSample_08_09_0A_0B = Abs( vSample_08_09_0A_0B );
//					const Vector4 vAbsSample_0C_0D_0E_0F = Abs( vSample_0C_0D_0E_0F );
//					
//					//const F32 fDrive = fPrevDrive + fFrameIndex * fDriveDelta;
//					const Vector4 vDrive_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vDriveDelta, vPrevDrive );
//					const Vector4 vDrive_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vDriveDelta, vPrevDrive );
//					const Vector4 vDrive_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vDriveDelta, vPrevDrive );
//					const Vector4 vDrive_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vDriveDelta, vPrevDrive );
//					
//					//const F32 fClip = fPrevClip + fFrameIndex * fClipDelta;
//					const Vector4 vClip_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vClipDelta, vPrevClip );
//					const Vector4 vClip_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vClipDelta, vPrevClip );
//					const Vector4 vClip_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vClipDelta, vPrevClip );
//					const Vector4 vClip_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vClipDelta, vPrevClip );
//					
//					//const F32 fLevel = fPrevLevel + fFrameIndex * fLevelDelta;
//					const Vector4 vLevel_00_01_02_03 = MulAdd( vFrameIndex_00_01_02_03, vLevelDelta, vPrevLevel );
//					const Vector4 vLevel_04_05_06_07 = MulAdd( vFrameIndex_04_05_06_07, vLevelDelta, vPrevLevel );
//					const Vector4 vLevel_08_09_0A_0B = MulAdd( vFrameIndex_08_09_0A_0B, vLevelDelta, vPrevLevel );
//					const Vector4 vLevel_0C_0D_0E_0F = MulAdd( vFrameIndex_0C_0D_0E_0F, vLevelDelta, vPrevLevel );
//					
//					//fFrameIndex += fOne;
//					vFrameIndex_00_01_02_03 += vSixteen;
//					vFrameIndex_04_05_06_07 += vSixteen;
//					vFrameIndex_08_09_0A_0B += vSixteen;
//					vFrameIndex_0C_0D_0E_0F += vSixteen;
//		            
//					//const F32 fIn = fDrive * Abs( fSample );
//					const Vector4 vIn_00_01_02_03 = vDrive_00_01_02_03 * vAbsSample_00_01_02_03;
//					const Vector4 vIn_04_05_06_07 = vDrive_04_05_06_07 * vAbsSample_04_05_06_07;
//					const Vector4 vIn_08_09_0A_0B = vDrive_08_09_0A_0B * vAbsSample_08_09_0A_0B;
//					const Vector4 vIn_0C_0D_0E_0F = vDrive_0C_0D_0E_0F * vAbsSample_0C_0D_0E_0F;
//					
//					const Vector4 vOneMinusClip_00_01_02_03 = vOne - vClip_00_01_02_03;
//					const Vector4 vOneMinusClip_04_05_06_07 = vOne - vClip_04_05_06_07;
//					const Vector4 vOneMinusClip_08_09_0A_0B = vOne - vClip_08_09_0A_0B;
//					const Vector4 vOneMinusClip_0C_0D_0E_0F = vOne - vClip_0C_0D_0E_0F;
//					
//					//const F32 fSgn = Select( fSample, fOne, fMinusOne );
//					const Vector4 vSignSample_00_01_02_03 = Sign( vSample_00_01_02_03 );
//					const Vector4 vSignSample_04_05_06_07 = Sign( vSample_04_05_06_07 );
//					const Vector4 vSignSample_08_09_0A_0B = Sign( vSample_08_09_0A_0B );
//					const Vector4 vSignSample_0C_0D_0E_0F = Sign( vSample_0C_0D_0E_0F );
//					
//					// RcpEst( fIn + ( fOne - fClip ) )
//					//const Vector4 vSatRatio_00_01_02_03 = RcpEst( vIn_00_01_02_03 + vOneMinusClip_00_01_02_03 );
//					//const Vector4 vSatRatio_04_05_06_07 = RcpEst( vIn_04_05_06_07 + vOneMinusClip_04_05_06_07 );
//					//const Vector4 vSatRatio_08_09_0A_0B = RcpEst( vIn_08_09_0A_0B + vOneMinusClip_08_09_0A_0B );
//					//const Vector4 vSatRatio_0C_0D_0E_0F = RcpEst( vIn_0C_0D_0E_0F + vOneMinusClip_0C_0D_0E_0F );
//					const Vector4 vTemp_00_01_02_03 = MulAdd( vDrive_00_01_02_03, vAbsSample_00_01_02_03, vOneMinusClip_00_01_02_03 );
//					const Vector4 vTemp_04_05_06_07 = MulAdd( vDrive_04_05_06_07, vAbsSample_04_05_06_07, vOneMinusClip_04_05_06_07 );
//					const Vector4 vTemp_08_09_0A_0B = MulAdd( vDrive_08_09_0A_0B, vAbsSample_08_09_0A_0B, vOneMinusClip_08_09_0A_0B );
//					const Vector4 vTemp_0C_0D_0E_0F = MulAdd( vDrive_0C_0D_0E_0F, vAbsSample_0C_0D_0E_0F, vOneMinusClip_0C_0D_0E_0F );
//					const Vector4 vSatRatio_00_01_02_03 = RcpEst( vTemp_00_01_02_03 );
//					const Vector4 vSatRatio_04_05_06_07 = RcpEst( vTemp_04_05_06_07 );
//					const Vector4 vSatRatio_08_09_0A_0B = RcpEst( vTemp_08_09_0A_0B );
//					const Vector4 vSatRatio_0C_0D_0E_0F = RcpEst( vTemp_0C_0D_0E_0F );
//					
//					const Vector4 vSaturated_00_01_02_03 = vIn_00_01_02_03 * vSatRatio_00_01_02_03;
//					const Vector4 vSaturated_04_05_06_07 = vIn_04_05_06_07 * vSatRatio_04_05_06_07;
//					const Vector4 vSaturated_08_09_0A_0B = vIn_08_09_0A_0B * vSatRatio_08_09_0A_0B;
//					const Vector4 vSaturated_0C_0D_0E_0F = vIn_0C_0D_0E_0F * vSatRatio_0C_0D_0E_0F;
//					
//					const Vector4 vLevel_x_Sign_00_01_02_03 = vLevel_00_01_02_03 * vSignSample_00_01_02_03;
//					const Vector4 vLevel_x_Sign_04_05_06_07 = vLevel_04_05_06_07 * vSignSample_04_05_06_07;
//					const Vector4 vLevel_x_Sign_08_09_0A_0B = vLevel_08_09_0A_0B * vSignSample_08_09_0A_0B;
//					const Vector4 vLevel_x_Sign_0C_0D_0E_0F = vLevel_0C_0D_0E_0F * vSignSample_0C_0D_0E_0F;
//					
//					//const F32 fOut = Select( fClip - fIn, fIn, fIn * RcpEst( fOne + fIn - fClip ) );
//					const Vector4 vOut_00_01_02_03 = Select_GE( vClip_00_01_02_03, vIn_00_01_02_03, vIn_00_01_02_03, vSaturated_00_01_02_03 );
//					const Vector4 vOut_04_05_06_07 = Select_GE( vClip_04_05_06_07, vIn_04_05_06_07, vIn_04_05_06_07, vSaturated_04_05_06_07 );
//					const Vector4 vOut_08_09_0A_0B = Select_GE( vClip_08_09_0A_0B, vIn_08_09_0A_0B, vIn_08_09_0A_0B, vSaturated_08_09_0A_0B );
//					const Vector4 vOut_0C_0D_0E_0F = Select_GE( vClip_0C_0D_0E_0F, vIn_0C_0D_0E_0F, vIn_0C_0D_0E_0F, vSaturated_0C_0D_0E_0F );
//		            
//					//aSample[uSampleIndex++] = fLevel * fSgn * fOut;
//					aSample[ uQuadSampleIndex + 0 ] = vLevel_x_Sign_00_01_02_03 * vOut_00_01_02_03;
//					aSample[ uQuadSampleIndex + 1 ] = vLevel_x_Sign_04_05_06_07 * vOut_04_05_06_07;
//					aSample[ uQuadSampleIndex + 2 ] = vLevel_x_Sign_08_09_0A_0B * vOut_08_09_0A_0B;
//					aSample[ uQuadSampleIndex + 3 ] = vLevel_x_Sign_0C_0D_0E_0F * vOut_0C_0D_0E_0F;
//		            
//					uQuadSampleIndex += 4;
//				}
//			}
//		    
//			#endif
//		    
//			pThis->m_fPrevDrive = pThis->m_fDrive;
//			pThis->m_fPrevClip  = pThis->m_fClip;
//			pThis->m_fPrevLevel = pThis->m_fLevel;
//			
//			return true;
//		}
//		break;
//
//	case Effect::eCompressor:
//		{
//			Compressor* pThis = (Compressor*)pData;
//
//			//	TODO: fix the ps3 compressor
//			return true;
//		}
//		break;
//
//	case Effect::eIIRHighShelf:
//		{
//			IIRHighShelf* pThis = (IIRHighShelf*)pData;
//
//			if ((pThis->m_fCutoff != pThis->m_fPrevCutoff) || (pThis->m_fSlope != pThis->m_fPrevSlope) || (pThis->m_fGain != pThis->m_fPrevGain))
//			{
//				F64 fOmega = (F64)(Maths::f2Pi * pThis->m_fCutoff * Core::fRcpSampleRate);
//				F64 fSin = (F64)Maths::Sin(fOmega);
//				F64 fCos = (F64)Maths::Cos(fOmega);
//				//F64 fBeta = fSin * (F64)Maths::SlowSqrt(pThis->m_fGain) / (F64)pThis->m_fSlope;
//				F64 fBeta = fSin * (F64)(Maths::SlowSqrt(pThis->m_fGain) / pThis->m_fSlope);
//				F64 fTemp0 = pThis->m_fGain + 1.0;
//				F64 fTemp1 = pThis->m_fGain - 1.0;
//				F64 fTemp2 = fTemp0 * fCos;
//				F64 fTemp3 = fTemp1 * fCos;
//				//F64 fA0 = 1.0 / (fTemp0 - fTemp3 + fBeta);
//				F64 fA0 = (F64)(1.f / (F32)(fTemp0 - fTemp3 + fBeta));
//				pThis->m_fA1 = (F32)((2.0 * (fTemp1 - fTemp2)) * fA0);
//				pThis->m_fA2 = (F32)((fTemp0 - fTemp3 - fBeta) * fA0);
//				pThis->m_fB0 = (F32)((pThis->m_fGain * (fTemp0 + fTemp3 + fBeta)) * fA0);
//				pThis->m_fB1 = (F32)((-2.0 * pThis->m_fGain * (fTemp1 + fTemp2)) * fA0);
//				pThis->m_fB2 = (F32)((pThis->m_fGain * (fTemp0 + fTemp3 - fBeta)) * fA0);
//
//				pThis->m_fPrevCutoff = pThis->m_fCutoff;
//				pThis->m_fPrevSlope = pThis->m_fSlope;
//				pThis->m_fPrevGain = pThis->m_fGain;
//			}
//
//			if (bProcess)
//			{
//				F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//
//				//  each channel
//				for (U32 uCh = 0; uCh < pThis->m_uChannels; uCh++)
//				{
//					F32 fIn1 = pThis->m_fIn1[uCh];
//					F32 fOut1 = pThis->m_fOut1[uCh];
//					F32 fIn2 = pThis->m_fIn2[uCh];
//					F32 fOut2 = pThis->m_fOut2[uCh];
//
//					//  each sample
//					for (U32 uSample = 0; uSample < uFrameLength; uSample++)
//					{
//						F32 fIn  = aSample[uCh * uFrameLength + uSample];
//						F32 fOut = pThis->m_fB0 * fIn + pThis->m_fB1 * fIn1 + pThis->m_fB2 * fIn2 - pThis->m_fA1 * fOut1 - pThis->m_fA2 * fOut2;
//						fOut = Maths::Clamp(fOut, -pThis->m_fClamp, pThis->m_fClamp);
//
//						fIn2 = fIn1;
//						fOut2 = fOut1;
//						fIn1 = fIn;
//						fOut1 = fOut;
//
//						aSample[uCh * uFrameLength + uSample] = fOut;
//					}
//
//					//  store previous samples
//					pThis->m_fIn1[uCh] = fIn1;
//					pThis->m_fOut1[uCh] = fOut1;
//					pThis->m_fIn2[uCh] = fIn2;
//					pThis->m_fOut2[uCh] = fOut2;
//				}
//			}
//
//			return true;
//		}
//		break;
//
//	case Effect::eIIRLowShelf:
//		{
//			IIRLowShelf* pThis = (IIRLowShelf*)pData;
//
//			if ((pThis->m_fCutoff != pThis->m_fPrevCutoff) || (pThis->m_fSlope != pThis->m_fPrevSlope) || (pThis->m_fGain != pThis->m_fPrevGain))
//			{
//				F64 fOmega = ((F64)Maths::f2Pi * (F64)pThis->m_fCutoff * (F64)Core::fRcpSampleRate);
//				F64 fSin = (F64)Maths::Sin(fOmega);
//				F64 fCos = (F64)Maths::Cos(fOmega);
//				//F64 fBeta = fSin * (F64)Maths::SlowSqrt(m_fGain) / (F64)m_fSlope;
//				F64 fBeta = fSin * (F64)(Maths::SlowSqrt(pThis->m_fGain) / pThis->m_fSlope);
//				F64 fTemp0 = pThis->m_fGain + 1.0;
//				F64 fTemp1 = pThis->m_fGain - 1.0;
//				F64 fTemp2 = fTemp0 * fCos;
//				F64 fTemp3 = fTemp1 * fCos;
//				//F64 fA0 = 1.0 / (fTemp0 + fTemp3 + fBeta);
//				F64 fA0 = (F64)(1.f / (F32)(fTemp0 + fTemp3 + fBeta));
//				pThis->m_fA1 = (F32)((-2.0 * (fTemp1 + fTemp2)) * fA0);
//				pThis->m_fA2 = (F32)((fTemp0 + fTemp3 - fBeta) * fA0);
//				pThis->m_fB0 = (F32)((pThis->m_fGain * (fTemp0 - fTemp3 + fBeta)) * fA0);
//				pThis->m_fB1 = (F32)((2.0 * pThis->m_fGain * (fTemp1 - fTemp2)) * fA0);
//				pThis->m_fB2 = (F32)((pThis->m_fGain * (fTemp0 - fTemp3 - fBeta)) * fA0);
//
//				pThis->m_fPrevCutoff = pThis->m_fCutoff;
//				pThis->m_fPrevSlope = pThis->m_fSlope;
//				pThis->m_fPrevGain = pThis->m_fGain;
//			}
//
//			if (bProcess)
//			{
//				F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//
//				//  each channel
//				for (U32 uCh = 0; uCh < pThis->m_uChannels; uCh++)
//				{
//					F32 fIn1 = pThis->m_fIn1[uCh];
//					F32 fOut1 = pThis->m_fOut1[uCh];
//					F32 fIn2 = pThis->m_fIn2[uCh];
//					F32 fOut2 = pThis->m_fOut2[uCh];
//
//					//  each sample
//					for (U32 uSample = 0; uSample < uFrameLength; uSample++)
//					{
//						F32 fIn  = aSample[uCh * uFrameLength + uSample];
//						F32 fOut = pThis->m_fB0 * fIn + pThis->m_fB1 * fIn1 + pThis->m_fB2 * fIn2 - pThis->m_fA1 * fOut1 - pThis->m_fA2 * fOut2;
//						fOut = Maths::Clamp(fOut, -pThis->m_fClamp, pThis->m_fClamp);
//
//						fIn2 = fIn1;
//						fOut2 = fOut1;
//						fIn1 = fIn;
//						fOut1 = fOut;
//
//						aSample[uCh * uFrameLength + uSample] = fOut;
//					}
//
//					//  store previous samples
//					pThis->m_fIn1[uCh] = fIn1;
//					pThis->m_fOut1[uCh] = fOut1;
//					pThis->m_fIn2[uCh] = fIn2;
//					pThis->m_fOut2[uCh] = fOut2;
//				}
//			}
//
//			return true;
//		}
//		break;
//
//	case Effect::eIIRPeakEQ:
//		{
//			IIRPeakEQ* pThis = (IIRPeakEQ*)pData;
//
//			if ((pThis->m_fCentre != pThis->m_fPrevCentre) || (pThis->m_fQ != pThis->m_fPrevQ) || (pThis->m_fGain != pThis->m_fPrevGain))
//			{
//				F32 fRealQ = powf(2, 8.f * pThis->m_fQ);
//				F32 fOmega = Maths::f2Pi * pThis->m_fCentre * Core::fRcpSampleRate;
//				F32 fSin = Maths::Sin(fOmega);
//				F32 fCos = Maths::Cos(fOmega);
//				F32 fAlpha = fSin / (2.f * fRealQ);
//				F32 fTemp0 = fAlpha * pThis->m_fGain;
//				F32 fTemp1 = fAlpha / pThis->m_fGain;
//				F32 fA0 = 1.f / (1.f + fTemp1);
//				pThis->m_fA1 = (-2.f * fCos) * fA0;
//				pThis->m_fA2 = (1.f - fTemp1) * fA0;
//				pThis->m_fB0 = (1.f + fTemp0) * fA0;
//				pThis->m_fB1 = (1.f - fTemp0) * fA0;
//
//				pThis->m_fPrevCentre = pThis->m_fCentre;
//				pThis->m_fPrevQ = pThis->m_fQ;
//				pThis->m_fPrevGain = pThis->m_fGain;
//			}
//
//			F32* AUDIO_RESTRICT aSample = (F32*)pFrameData;
//
//			//  each channel
//			for (U32 uCh = 0; uCh < pThis->m_uChannels; uCh++)
//			{
//				F32 fIn1 = pThis->m_fIn1[uCh];
//				F32 fOut1 = pThis->m_fOut1[uCh];
//				F32 fIn2 = pThis->m_fIn2[uCh];
//				F32 fOut2 = pThis->m_fOut2[uCh];
//
//				//  each sample
//				for (U32 uSample = 0; uSample < uFrameLength; uSample++)
//				{
//					F32 fIn  = aSample[uCh * uFrameLength + uSample];
//					F32 fOut = pThis->m_fB0 * fIn + pThis->m_fA1 * fIn1 + pThis->m_fB1 * fIn2 - pThis->m_fA1 * fOut1 - pThis->m_fA2 * fOut2;
//
//					fOut = Maths::Clamp(fOut, -pThis->m_fClamp, pThis->m_fClamp);
//
//					fIn2 = fIn1;
//					fOut2 = fOut1;
//					fIn1 = fIn;
//					fOut1 = fOut;
//
//					aSample[uCh * uFrameLength + uSample] = fOut;
//				}
//
//				//  store previous samples
//				pThis->m_fIn1[uCh] = fIn1;
//				pThis->m_fOut1[uCh] = fOut1;
//				pThis->m_fIn2[uCh] = fIn2;
//				pThis->m_fOut2[uCh] = fOut2;
//			}
//
//			return true;
//		}
//		break;
//
//	default:
//		spu_printf("DSPEffect type %d not implemented\n", eType);
//		break;
//	}
//#endif // AUDIO_SPU_MINIMAL
//
//	return true;
//}

#endif // SN_TARGET_PS3_SPU


