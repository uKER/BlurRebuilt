#include "AudioPch.h"
#include "coreheaders.h"

#include <system/scheduler/overlays.h>

#ifndef PLATFORM_SPU
// shouldn't really be including system headers here, but we need it for the adpcm buffer memory allocs..
#include "../systemheaders.h"
#else
#include "SystemHeadersSPU.h"
#endif


#define AUDIO_CORE_DATA_TYPE        F32
#define AUDIO_CORE_SAMPLE_RATE      48000
#define AUDIO_CORE_FRAME_LENGTH     256
#define AUDIO_CORE_STRIDE           AUDIO_CORE_FRAME_LENGTH
#define AUDIO_CORE_MIXER_INPUTS     16
#define AUDIO_CORE_NUM_DSP          16

bool Audio::g_bUseAltConnectMode = true;
bool Audio::g_bQuitAllTasks = false;

#ifdef PLATFORM_SPU
U32 g_u32CurrentCodeOverlayId = (U32)-1;
//void* g_pOverlayTable = NULL;
#endif

#ifndef PLATFORM_SPU
Audio::Core::Renderer* g_pRenderer = NULL;
#endif



namespace Audio
{
	namespace Core
	{
        const F32 fSampleRate = (F32)uSampleRate;
        const F32 fNyquist = (F32)( uSampleRate >> 1 );
        const F32 fFrameLength = (F32)uFrameLength;
		const F32 fRcpFrameLength = 1.f / F32( uFrameLength );
		const F32 fRcpSampleRate = 1.f / F32( uSampleRate );

#if defined(PLATFORM_PC)
		// [Blur2PC] T144: retail 0x01028c3c and its setters 0x0053f660 / 0x0053f6d0 (see audio_core.h)
		F32 fOutputRateRatio = 1.f;

		void ResetOutputSampleRate()
		{
			fOutputRateRatio = 1.f;
		}

		void SetOutputSampleRate(U32 uOutputSampleRate)
		{
			fOutputRateRatio = fSampleRate / (F32)uOutputSampleRate;
		}
#endif
	} // namespace Core
} // namespace Audio



U32 Audio::Core::PCMS16Decoder::m_uTotalPCM = 0;
U32 Audio::Core::PCMS16Decoder::m_uActivePCM = 0;
U32 Audio::Core::ADPCM4bitDecoder::m_uTotalADPCM = 0;
U32 Audio::Core::ADPCM4bitDecoder::m_uActiveADPCM = 0;


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


#if defined( __INTEL_COMPILER )

template < U32 uEffects, U32 uOutputs >
void* Audio::Core::Generator< uEffects, uOutputs >::operator new( size_t sz )
{
	//return new U8[sz];
	return malloc( sz );
};

// TODO: FIND WORKAROUND FOR INTERNAL COMPILER ERROR WITH INTEL COMPILER!
template < U32 uEffects, U32 uOutputs >
void Audio::Core::Generator< uEffects, uOutputs >::operator delete( void* ptr )
{
	//delete ptr;
	free( ptr );
};

template < U32 uEffects, U32 uOutputs >
void* Audio::Core::Generator< uEffects, uOutputs >::operator new( size_t sz, void* pHere )
{
	return pHere;
};

template < U32 uEffects, U32 uOutputs >
void Audio::Core::Generator< uEffects, uOutputs >::operator delete( void* ptr, void* pHere )
{
};

#endif

////////////////////////////////////////////////////////////////////////////////
//	CreateRenderer
////////////////////////////////////////////////////////////////////////////////
#ifndef PLATFORM_SPU
Audio::Core::Renderer* Audio::CreateRenderer()
{
    if (g_pRenderer == NULL)
        g_pRenderer = new Core::Renderer(8);

	return g_pRenderer;
}
#endif


#ifdef PLATFORM_SPU

bool bDebugOverlayInfo = false;
bool bOverlayBreak = false;

bool CoreLoadCodeOverlay(U32 u32OverlayId)
{
return true;	// temp, overlays possibly not required anymore
	if (g_u32CurrentCodeOverlayId == u32OverlayId)
		return true;

	U32 u32RetValue = 0;

	g_u32CurrentCodeOverlayId = u32OverlayId;

	//if (! g_pOverlayTable)
	//	DebugBreak();

	switch (u32OverlayId)
	{
#ifdef AUDIO_GENERATORS_NOT_OVERLAID
		case 1:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay1((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay1(20);
		}
		break;

		case 2:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay2((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay2(20);
		}
		break;

		case 3:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay3((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay3(20);
		}
		break;

		case 4:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay4((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay4(20);
		}
		break;

		case 5:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay5((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay5(20);
		}
		break;

		case 6:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay6((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay6(20);
		}
		break;

#ifndef AUDIO_SPU_MINIMAL
		case 7:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay7((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay7(20);
		}
		break;

		case 8:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay8((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay8(20);
		}
		break;

		case 9:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay9((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay9(20);
		}
		break;

		case 10:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay10((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay10(20);
		}
		break;

		case 11:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay11((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay11(20);
		}
		break;

		case 12:
		{
			u32RetValue = cellOvisStartMapping_CodeOverlay12((uint64_t)g_pOverlayTable, 20);
			cellOvisWaitMapping_CodeOverlay12(20);
		}
		break;
#endif
#else
		case 1:
		{
			//if (bDebugOverlayInfo)
			//	spu_printf("loading overlay 1\n");
			if (bOverlayBreak)
				DebugBreak();
			Scheduler::SelectOverlay(AUDIO_COMPONENT_OVERLAY_ID);
		}
		break;

		case 2:
		{
			//if (bDebugOverlayInfo)
			//	spu_printf("loading overlay 2\n");
			if (bOverlayBreak)
				DebugBreak();
			Scheduler::SelectOverlay(AUDIO_GENERATOR_OVERLAY_ID);
		}
		break;

#endif

		default:
		{
			u32RetValue = 1;
			DebugBreak();
		}
		break;
	}

	if (u32RetValue != 0)
	{
//		spu_printf("overlap error- error code is 0x%x, overlay table location is 0x%x\n", u32RetValue, g_pOverlayTable);
		DebugBreak();
		//Audio::Assert(0);
		return false;
	}
	else
		return true;
}
#endif // PLATFORM_SPU

////////////////////////////////////////////////////////////////////////////////
//	FFT
// Sorensen in-place split-radix FFT for real values
// data: array of F32s:
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

void Audio::Core::DSP::RealFFT_Split(F32 *data,long n){

#ifndef AUDIO_DISABLE_DSP

	const F32	PI		= 3.1415926535897932384626433832795;

  long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8;
  F32 t1,t2,t3,t4,t5,t6,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;

  const F32 fRoot2          = 1.41421356237f;
  
  sqrt2=fRoot2;
  n4=n-1;
  
  //data shuffling
	  for (i=0,j=0,n2=n/2; i<n4 ; i++){
	  if (i<j){
				t1=data[j];
				data[j]=data[i];
				data[i]=t1;
				}
	  k=n2;
	  while (k<=j){
				j-=k;
				k>>=1;	
				}
	  j+=k;
	  }
	
/*----------------------*/
	
	//length two butterflies	
	i0=0;
	id=4;
   do{
	   for (; i0<n4; i0+=id){ 
			i1=i0+1;
			t1=data[i0];
			data[i0]=t1+data[i1];
			data[i1]=t1-data[i1];
		}
	   id<<=1;
	   i0=id-2;
	   id<<=1;
	} while ( i0<n4 );

   /*----------------------*/
   //L shaped butterflies
n2=2;
for(k=n;k>2;k>>=1){  
	n2<<=1;
	n4=n2>>2;
	n8=n2>>3;
	e = 2*PI/(n2);
	i1=0;
	id=n2<<1;
	do{ 
		for (; i1<n; i1+=id){
			i2=i1+n4;
			i3=i2+n4;
			i4=i3+n4;
			t1=data[i4]+data[i3];
			data[i4]-=data[i3];
			data[i3]=data[i1]-t1;
			data[i1]+=t1;
			if (n4!=1){
				i0=i1+n8;
				i2+=n8;
				i3+=n8;
				i4+=n8;
				t1=(data[i3]+data[i4])/sqrt2;
				t2=(data[i3]-data[i4])/sqrt2;
				data[i4]=data[i2]-t1;
				data[i3]=-data[i2]-t1;
				data[i2]=data[i0]-t2;
				data[i0]+=t2;
			}
		 }
		 id<<=1;
		 i1=id-n2;
		 id<<=1;
	  } while ( i1<n );
	a=e;
	for (j=2; j<=n8; j++){  
		  a3=3*a;
		  cc1=cos(a);
		  ss1=sin(a);
		  cc3=cos(a3);
		  ss3=sin(a3);
		  a=j*e;
		  i=0;
		  id=n2<<1;
		  do{
		   for (; i<n; i+=id){  
			  i1=i+j-1;
			  i2=i1+n4;
			  i3=i2+n4;
			  i4=i3+n4;
			  i5=i+n4-j+1;
			  i6=i5+n4;
			  i7=i6+n4;
			  i8=i7+n4;
			  t1=data[i3]*cc1+data[i7]*ss1;
			  t2=data[i7]*cc1-data[i3]*ss1;
			  t3=data[i4]*cc3+data[i8]*ss3;
			  t4=data[i8]*cc3-data[i4]*ss3;
			  t5=t1+t3;
			  t6=t2+t4;
			  t3=t1-t3;
			  t4=t2-t4;
			  t2=data[i6]+t6;
			  data[i3]=t6-data[i6];
			  data[i8]=t2;
			  t2=data[i2]-t3;
			  data[i7]=-data[i2]-t3;
			  data[i4]=t2;
			  t1=data[i1]+t5;
			  data[i6]=data[i1]-t5;
			  data[i1]=t1;
			  t1=data[i5]+t4;
			  data[i5]-=t4;
			  data[i2]=t1;
			 }
		   id<<=1;
		   i=id-n2;
		   id<<=1;
		 } while(i<n);
	   }
	  }

	//division with array length
 //  for(i=0;i<n;i++) data[i]/=n;

#endif	//	#ifndef AUDIO_DISABLE_DSP
}

/////////////////////////////////////////////////////////
// Sorensen in-place inverse split-radix FFT for real values
// data: array of F32s:
// re(0),re(1),re(2),...,re(size/2),im(size/2-1),...,im(1)
//
// output:
// re(0),re(1),re(2),...,re(size-1)
// NOT normalized by array length
//
// Source:
// Sorensen et al: Real-Valued Fast Fourier Transform Algorithms,
// IEEE Trans. ASSP, ASSP-35, No. 6, June 1987
void Audio::Core::DSP::IRealFFT_Split(F32 *data,long n)
{

	#ifndef AUDIO_DISABLE_DSP


	const F32	pi		= 3.1415926535897932384626433832795;

	long i,j,k,i5,i6,i7,i8,i0,id,i1,i2,i3,i4,n2,n4,n8,n1;
	F32 t1,t2,t3,t4,t5,a3,ss1,ss3,cc1,cc3,a,e,sqrt2;
	sqrt2=sqrt(2.f);
	n1=n-1;
	n2=n<<1;
	for(k=n;k>2;k>>=1){
	id=n2;
	n2>>=1;
	n4=n2>>2;
	n8=n2>>3;
	e = 2*pi/(n2);
	i1=0;
	
	do
	{
		for (; i1<n; i1+=id)
		{
			i2=i1+n4;
			i3=i2+n4;
			i4=i3+n4;
			t1=data[i1]-data[i3];
			data[i1]+=data[i3];
			data[i2]*=2;
			data[i3]=t1-2*data[i4];
			data[i4]=t1+2*data[i4];
			
				if (n4!=1)
				{
					i0=i1+n8;
					i2+=n8;
					i3+=n8;
					i4+=n8;
					t1=(data[i2]-data[i0])/sqrt2;
					t2=(data[i4]+data[i3])/sqrt2;
					data[i0]+=data[i2];
					data[i2]=data[i4]-data[i3];
					data[i3]=2*(-t2-t1);
					data[i4]=2*(-t2+t1);
				}
		}

			id<<=1;
			i1=id-n2;
			id<<=1;
	} while ( i1<n1 );


			a=e;

			for (j=2; j<=n8; j++)
			{

				a3=3*a;
				cc1=cos(a);
				ss1=sin(a);
				cc3=cos(a3);
				ss3=sin(a3);
				a=j*e;
				i=0;
				id=n2<<1;
			
				do
				{
					for (; i<n; i+=id)
					{
							i1=i+j-1;
							i2=i1+n4;
							i3=i2+n4;
							i4=i3+n4;
							i5=i+n4-j+1;
							i6=i5+n4;
							i7=i6+n4;
							i8=i7+n4;
							t1=data[i1]-data[i6];
							data[i1]+=data[i6];
							t2=data[i5]-data[i2];
							data[i5]+=data[i2];
							t3=data[i8]+data[i3];
							data[i6]=data[i8]-data[i3];
							t4=data[i4]+data[i7];
							data[i2]=data[i4]-data[i7];
							t5=t1-t4;
							t1+=t4;
							t4=t2-t3;
							t2+=t3;
							data[i3]=t5*cc1+t4*ss1;
							data[i7]=-t4*cc1+t5*ss1;
							data[i4]=t1*cc3-t2*ss3;
							data[i8]=t2*cc3+t1*ss3;
					}

					id<<=1;
					i=id-n2;
					id<<=1;

				} while(i<n1);
			}
		}
			/*----------------------*/
			i0=0;
			id=4;
			do
			{
			for (; i0<n1; i0+=id)
			{
			i1=i0+1;
			t1=data[i0];
			data[i0]=t1+data[i1];
			data[i1]=t1-data[i1];
		}
		id<<=1;
		i0=id-2;
		id<<=1;
	} while ( i0<n1 );
	/*----------------------*/
	//data shuffling
	for (i=0,j=0,n2=n/2; i<n1 ; i++)
	{
		if (i<j){
		t1=data[j];
		data[j]=data[i];
		data[i]=t1;
	}

	k=n2;	

	while (k<=j)
	{
		j-=k;
		k>>=1;
		}
		j+=k;
	}

		//division with array length
   for(i=0;i<n;i++) data[i]/=n;

#endif //disable DSP
}

//	Bring in ATG FFT



Audio::Core::WaveStreamData* Audio::Core::GetWaveData(WaveServerLocalIndex hWaveData, const void* pWaveServerData)
{
	if (hWaveData.IsValid())
		return &(((WaveStreamData*)((U8*)pWaveServerData + 4))[hWaveData]);
	else
		return 0;
}


Audio::Core::AdpcmWaveInfo* Audio::Core::GetAdpcmBufferData(U32 uAdpcmWaveInfoIndex, const void* pAdpcmBufferData)
{
	AdpcmWaveInfo* pInfo = (AdpcmWaveInfo*)(((U8*)pAdpcmBufferData) + 4);
	return &(pInfo[uAdpcmWaveInfoIndex]);
}


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


