#include "AudioPch.h"
#include "coreheaders.h"
#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "../SystemHeaders.h"
#endif

#ifndef PLATFORM_PC
#include <core/Maths.h>
#endif 

//using namespace Audio;
using namespace Audio::Core;

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#ifndef CFG_SPEED
//#define CHECK_OUTPUT_QUALITY
#endif

#ifndef PLATFORM_SPU
Debug::Bool g_switchResampler("Audio.System.NewResampler", true);
//Debug::Bool g_switchFillBufferFromS16("Audio.PerfTweaks.NewFillBufferS16", true);
#endif


#ifdef PLATFORM_SPU
U32 u32CurrentOverlayId = 0;

void SwitchInGeneratorOverlay()
{
#ifndef AUDIO_GENERATORS_NOT_OVERLAID
	u32CurrentOverlayId = Audio::System::Get()->GetCurrentCodeOverlayId();

	if (u32CurrentOverlayId != AUDIO_GENERATOR_OVERLAY_ID)
		CoreLoadCodeOverlay(AUDIO_GENERATOR_OVERLAY_ID);
#endif
}


void SwitchBackToOriginalOverlay()
{
#ifndef AUDIO_GENERATORS_NOT_OVERLAID
	if (u32CurrentOverlayId != AUDIO_GENERATOR_OVERLAY_ID)
		CoreLoadCodeOverlay(u32CurrentOverlayId);
#endif
}
#endif

#ifdef AUDIO_NEW_DECODERS
F32* ISampleRateConverter::m_afResampleBuffer = NULL;
U32 ISampleRateConverter::m_uBufferLength = 0;
////////////////////////////////////////////////////////////////////////////////
//	Init
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::ISampleRateConverter::Init( U32 uBufferLength )
{
	m_uBufferLength = uBufferLength;

	m_afResampleBuffer = new F32[ uBufferLength ];
}

////////////////////////////////////////////////////////////////////////////////
//	GetDecodedData
////////////////////////////////////////////////////////////////////////////////
bool Audio::Core::DecoderBase::GetDecodedFrame( F32* pfBuffer )
{
	if ( m_pSRC && m_pSRC->ResampleBuffer() )
	{
		if ( pfBuffer )
		{
			//	decode to resample buffer
			GetDecodedData( m_pSRC->ResampleBuffer(), m_pSRC->InputLength() );

			//	resample to output buffer, if buffer provided
			m_pSRC->Resample( pfBuffer, Audio::Core::uFrameLength );
		}
		else
		{
			//	reset history
			m_pSRC->ClearHistory();

			//	increment wave position but skip decoding
			GetDecodedData( NULL, m_pSRC->InputLength() );
		}
	}
	else
	{
		//	decode to output buffer
		GetDecodedData( pfBuffer, uFrameLength );
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
//	ConvertS16ToF32
////////////////////////////////////////////////////////////////////////////////
void Audio::Core::DecoderBase::ConvertS16ToF32( F32* pfBufferOut, const S16* pfBufferIn, const U32 uSamplesIn )
{
	for ( U32 uCount=0; uCount < uSamplesIn; uCount++ )
		*pfBufferOut++ = F32(*pfBufferIn++) * Audio::Maths::fInv2pow15;
}

#endif // AUDIO_NEW_DECODERS

void ResampleOld(const float fPitch, const unsigned int uSamplesRequired, const F32* __restrict in, F32* __restrict out)
{
	if (fPitch != 1.f) // JASOND: uSamplesRequired != uFrameLength ?
	{
#ifdef PLATFORM_SPUA
		F32 fSubSample = 0.0f; //
		F32 fSubSampleDelta = (F32)uSamplesRequired * fRcpFrameLength; 
		
		vector float vSubSample = { 0.0f, 1.0f, 2.0f, 3.0f };
		vector float vSubSampleDelta = spu_splats( fSubSampleDelta );
		vector unsigned int vione = { 1, 1, 1, 1 };
		vector float vfour = spu_splats( 4.0f );
		for (U32 uCount = 0; uCount < uFrameLength; uCount+=4)
		{
			vector float vfIndex = spu_mul( vSubSample, vSubSampleDelta );
			vector unsigned int viIndex = spu_convtu(vfIndex, 0);
			vector float vfraction = spu_convtf(viIndex, 0);
			vfraction = spu_sub( vfIndex, vfraction );

			int indexA = spu_extract( viIndex, 0 );
			int indexB = spu_extract( viIndex, 1 );
			int indexC = spu_extract( viIndex, 2 );
			int indexD = spu_extract( viIndex, 3 );
			vector float vSampleA = { in[ indexA ], in[ indexB ], in[ indexC ], in[ indexD ] };

			viIndex = spu_add( viIndex, vione );
			indexA = spu_extract( viIndex, 0 );
			indexB = spu_extract( viIndex, 1 );
			indexC = spu_extract( viIndex, 2 );
			indexD = spu_extract( viIndex, 3 );
			vector float vSampleB = { in[ indexA ], in[ indexB ], in[ indexC ], in[ indexD ] };
						
			vector float vdiff = spu_sub( vSampleB, vSampleA );
			vector float vdelta = spu_mul( vdiff, vfraction );
			vector float vout = spu_add( vSampleA, vdelta );
			// look at using qword si store instruction
			out[ uCount + 0] = spu_extract( vout, 0 );			
			out[ uCount + 1] = spu_extract( vout, 1 );			
			out[ uCount + 2] = spu_extract( vout, 2 );			
			out[ uCount + 3] = spu_extract( vout, 3 );			
			
			vSubSample = spu_add( vSubSample, vfour );
		}
#else
		for (U32 uCount = 0; uCount < uFrameLength; uCount++)
		{
			F32 fSubSample = (F32)uCount * (F32)uSamplesRequired * fRcpFrameLength;

			F32 fIndex = floor(fSubSample);

			F32 fFraction = fSubSample - (F32)fIndex;
			S32 gIndex = (S32)fIndex;

			F32 fSampleA = in[gIndex];
			F32 fSampleB = in[gIndex + 1];

			F32 fDiff = fSampleB - fSampleA;
			F32 fDelta = fDiff * fFraction;
			out[uCount] = fSampleA + fDelta;	//	could do /32768 here on shift down?
		}
#endif
	}
	else
	{
		//	copy
		for (U32 uCount= 0; uCount < uFrameLength; uCount++)
			out[uCount] = in[uCount];
	}
}


void ResampleOld(const float fPitch, unsigned int uSamplesRequired, const F32* __restrict inLeft, const F32* __restrict inRight, F32* __restrict outLeft, F32* __restrict outRight)
{
	ResampleOld(fPitch, uSamplesRequired, inLeft, outLeft);
	ResampleOld(fPitch, uSamplesRequired, inRight, outRight);
	//if (fPitch != 1.f) // JASOND: uSamplesRequired != uFrameLength ?
	//{
	//	for (U32 uCount = 0; uCount < uFrameLength; uCount++)
	//	{
	//		F32 fSubSample = (F32)uCount * (F32)uSamplesRequired * fRcpFrameLength;

	//		F32 fIndex = Audio::Maths::Floor(fSubSample);

	//		F32 fFraction = fSubSample - (F32)fIndex;
	//		S32 gIndex = (S32)fIndex;

	//		F32 fSampleAL = inLeft[gIndex];
	//		F32 fSampleBL = inLeft[gIndex + 1];
	//		F32 fSampleAR = inRight[gIndex];
	//		F32 fSampleBR = inRight[gIndex + 1];

	//		F32 fDiffL = fSampleBL - fSampleAL;
	//		F32 fDiffR = fSampleBR - fSampleAR;
	//		F32 fDeltaL = fDiffL * fFraction;
	//		F32 fDeltaR = fDiffR * fFraction;

	//		outLeft[uCount]		= fSampleAL + fDeltaL;	//	could do /32768 here on shift down?
	//		outRight[uCount]	= fSampleAR + fDeltaR;	//	and here
	//	}
	//}
	//else
	//{
	//	//	copy
	//	for (U32 uCount=0; uCount < uFrameLength; uCount++)
	//	{
	//		outLeft[uCount] = inLeft[uCount];
	//		outRight[uCount] = inRight[uCount];
	//	}
	//	}
	//}
}


void Decoder::Resample(const float fPitch, const unsigned int uSamplesRequired, const F32* __restrict in, F32* __restrict out)
{
	ResampleOld(fPitch, uSamplesRequired, in, out);
}

void Decoder::Resample(const float fPitch, const unsigned int uSamplesRequired, const F32* __restrict inLeft, const F32* __restrict inRight, F32* __restrict outLeft, F32* __restrict outRight)
{
	ResampleOld(fPitch, uSamplesRequired, inLeft, inRight, outLeft, outRight);
	{
	}
}

// Fill buffer - uses fixed point maths 8:24, max pitch +8 octaves interpolate:
void FillF32BufferFromS16Old(const unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dst)
{
	for (U32 uCount=0; uCount < samplesRequired; uCount++)
		*dst++ = F32(*src++) * Audio::Maths::fInv2pow15;
}


void Decoder::FillF32BufferFromS16(const unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dst)
{
	FillF32BufferFromS16Old(samplesRequired, src, dst);
}

void FillF32BufferFromS16Old(const unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dstL, F32* AUDIO_RESTRICT dstR)
{
	for (U32 uCount=0; uCount < samplesRequired; uCount++)
	{
		*dstL++ = F32(*src++) * Audio::Maths::fInv2pow15;
		*dstR++ = F32(*src++) * Audio::Maths::fInv2pow15;
	}
}


void Decoder::FillF32BufferFromS16(const unsigned int samplesRequired, const S16* AUDIO_RESTRICT src, F32* AUDIO_RESTRICT dstL, F32* AUDIO_RESTRICT dstR)
{
	FillF32BufferFromS16Old(samplesRequired, src, dstL, dstR);
}

void Decoder::LowPassFilter(unsigned int uSamplesRequired, MonoState& state, F32* AUDIO_RESTRICT buffer, float fA, float fB)
{
	//	1st order low pass filter
	F32 fPrevIn = state.fPrevIn;
	F32 fPrevOut = state.fPrevOut;
	
	for (U32 uCount=0; uCount < uSamplesRequired; uCount++)
	{
		F32 fIn  = buffer[uCount];
		F32 fOut = fA * (fIn + fPrevIn) + fB * fPrevOut;

		buffer[uCount] = fOut; 

		fPrevIn = fIn;
		fPrevOut = fOut;
	}
	state.fPrevIn = fPrevIn;
	state.fPrevOut = fPrevOut;

	{
		//	do extra sample
		F32 fIn  = buffer[uSamplesRequired];
		F32 fOut = fA * (fIn + fPrevIn) + fB * fPrevOut;

		buffer[uSamplesRequired] = fOut;

		//	store last sample for next iteration
		state.fPrevSample = buffer[uSamplesRequired];
	}
}

void Decoder::LowPassFilter(unsigned int uSamplesRequired, StereoState& state, F32* AUDIO_RESTRICT bufferL, F32* AUDIO_RESTRICT bufferR, float fA, float fB)
{
	//	1st order low pass filter
	F32 fPrevInL = state.fPrevInL;
	F32 fPrevInR = state.fPrevInR;
	F32 fPrevOutL = state.fPrevOutL;
	F32 fPrevOutR = state.fPrevOutR;

	for (U32 uCount = 0; uCount < uSamplesRequired; uCount++)
	{
		F32 fInL  = bufferL[uCount];
		F32 fOutL = fA * (fInL + fPrevInL) + fB * fPrevOutL;
		F32 fInR  = bufferR[uCount];
		F32 fOutR = fA * (fInR + fPrevInR) + fB * fPrevOutR;

		bufferL[uCount] = fOutL; 
		bufferR[uCount] = fOutR; 

		fPrevInL = fInL;
		fPrevInR = fInR;
		fPrevOutL = fOutL;
		fPrevOutR = fOutR;
	}
	state.fPrevInL = fPrevInL;
	state.fPrevInR = fPrevInR;
	state.fPrevOutL = fPrevOutL;
	state.fPrevOutR = fPrevOutR;

	//	do extra sample
	{
		F32 fInL  = bufferL[uSamplesRequired];
		F32 fOutL = fA * (fInL + fPrevInL) + fB * fPrevOutL;
		F32 fInR  = bufferR[uSamplesRequired];
		F32 fOutR = fA * (fInR + fPrevInR) + fB * fPrevOutR;

		bufferL[uSamplesRequired] = fOutL; 
		bufferR[uSamplesRequired] = fOutR; 

		//	store last sample for next iteration
		state.fPrevSampleL = fOutL;
		state.fPrevSampleR = fOutR;
	}
}

#ifndef VF_DECODERS

void Decoder::Init(const DecoderDataInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			if (m_u32NumChannels == 1)
				((MonoPCMS16Decoder*)this)->MonoPCMS16Decoder::Init(info);
			else if (m_u32NumChannels == 2)
				((StereoPCMS16Decoder*)this)->StereoPCMS16Decoder::Init(info);
		}
		break;

		case uADPCM4Bit:
		{
			if (m_u32NumChannels == 1)
				((MonoADPCM4bitDecoder*)this)->MonoADPCM4bitDecoder::Init(info);
			else if (m_u32NumChannels == 2)
				((StereoADPCM4bitDecoder*)this)->StereoADPCM4bitDecoder::Init(info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


void Decoder::Shutdown(const DecoderDataInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::Shutdown(info);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::Shutdown(info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


bool Decoder::IsActive(const DecoderDataInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif
	bool bRetvalue = false;

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			bRetvalue = ((PCMS16Decoder*)this)->PCMS16Decoder::IsActive(info);
		}
		break;

		case uADPCM4Bit:
		{
			bRetvalue = ((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::IsActive(info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif

	return bRetvalue;
}


//void Decoder::SetCallbackId(U32 u32StateOffset, U32 u32CallbackId)
//{
//#ifdef PLATFORM_SPU
//	SwitchInGeneratorOverlay();
//#endif
//
//	switch (m_eType)
//	{
//		case uPCM16BitSignedInt:
//		{
//			((PCMS16Decoder*)this)->PCMS16Decoder::SetCallbackId(u32StateOffset, u32CallbackId);
//		}
//		break;
//
//		case uADPCM4Bit:
//		{
//			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetCallbackId(u32StateOffset, u32CallbackId);
//		}
//		break;
//
//#ifdef PLATFORM_XENON
//		case uXMA16Bit:
//		case uXMA216Bit:
//		{
//			((XMADecoder*)this)->XMADecoder::SetCallbackId(u32StateOffset, u32CallbackId);
//		}
//		break;
//#endif
//
//		default:
//			Assert(0 && "Unknown audio decoder type.");
//		break;
//	}
//
//#ifdef PLATFORM_SPU
//	SwitchBackToOriginalOverlay();
//#endif
//}

void Decoder::SetHasCallback()
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::SetHasCallback();
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetHasCallback();
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


bool Decoder::FillBuffer(F32* afData, const ProcessInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	bool bRetValue = false;

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			if (m_u32NumChannels == 1)
				bRetValue = ((MonoPCMS16Decoder*)this)->MonoPCMS16Decoder::FillBuffer(afData, info);
			else if (m_u32NumChannels == 2)
				bRetValue = ((StereoPCMS16Decoder*)this)->StereoPCMS16Decoder::FillBuffer(afData, info);
		}
		break;

		case uADPCM4Bit:
		{
			if (m_u32NumChannels == 1)
				bRetValue = ((MonoADPCM4bitDecoder*)this)->MonoADPCM4bitDecoder::FillBuffer(afData, info);
			else if (m_u32NumChannels == 2)
				bRetValue = ((StereoADPCM4bitDecoder*)this)->StereoADPCM4bitDecoder::FillBuffer(afData, info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif

	return bRetValue;
}


void Decoder::SkipBuffer(const ProcessInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			if (m_u32NumChannels == 1)
				return ((MonoPCMS16Decoder*)this)->MonoPCMS16Decoder::SkipBuffer(info);
			else if (m_u32NumChannels == 2)
				return ((StereoPCMS16Decoder*)this)->StereoPCMS16Decoder::SkipBuffer(info);
		}
		break;

		case uADPCM4Bit:
		{
			if (m_u32NumChannels == 1)
				return ((MonoADPCM4bitDecoder*)this)->MonoADPCM4bitDecoder::SkipBuffer(info);
			else if (m_u32NumChannels == 2)
				return ((StereoADPCM4bitDecoder*)this)->StereoADPCM4bitDecoder::SkipBuffer(info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


F32 Decoder::Position(const DecoderDataInfo& info)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	F32 f32RetValue = 0.f;

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			f32RetValue = ((PCMS16Decoder*)this)->PCMS16Decoder::Position(info);
		}
		break;

		case uADPCM4Bit:
		{
			f32RetValue = ((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::Position(info);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif

	return f32RetValue;
}


void Decoder::Reset(const DecoderDataInfo& info, U32 uOffset)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::Reset(info, uOffset);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::Reset(info, uOffset);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


void Decoder::SetPitch(F32 fPitch)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::SetPitch(fPitch);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetPitch(fPitch);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


void Decoder::SetPosition(U32 uPosition)
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::SetPosition(uPosition);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetPosition(uPosition);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif
}


U32 Decoder::Length()
{
#ifdef PLATFORM_SPU
	SwitchInGeneratorOverlay();
#endif

	U32 u32RetValue = 0;

	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			if (m_u32NumChannels == 1)
				u32RetValue = ((MonoPCMS16Decoder*)this)->MonoPCMS16Decoder::Length();
			else if (m_u32NumChannels == 2)
				u32RetValue = ((StereoPCMS16Decoder*)this)->StereoPCMS16Decoder::Length();
		}
		break;

		case uADPCM4Bit:
		{
			if (m_u32NumChannels == 1)
				u32RetValue = ((MonoADPCM4bitDecoder*)this)->MonoADPCM4bitDecoder::Length();
			else if (m_u32NumChannels == 2)
				u32RetValue = ((StereoADPCM4bitDecoder*)this)->StereoADPCM4bitDecoder::Length();
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}

#ifdef PLATFORM_SPU
	SwitchBackToOriginalOverlay();
#endif

	return u32RetValue;
}


/*
#ifdef PLATFORM_SPU
void Decoder::PrepareDmaTransfer()
{
	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::PrepareDmaTransfer();
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::PrepareDmaTransfer();
		}
		break;

		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}
}


void Decoder::ReleaseDmaTransfer()
{
	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::ReleaseDmaTransfer();
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::ReleaseDmaTransfer();
		}
		break;

		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}
}
#endif
*/

#endif	//	VF_DECODERS

void Decoder::Prime(const DecoderDataInfo& info)
{
	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::SetPlaying(info, true);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetPlaying(info, true);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}
}


void Decoder::Flush(const DecoderDataInfo& info)
{
	switch (m_eType)
	{
		case uPCM16BitSignedInt:
		{
			((PCMS16Decoder*)this)->PCMS16Decoder::SetPlaying(info, false);
		}
		break;

		case uADPCM4Bit:
		{
			((ADPCM4bitDecoder*)this)->ADPCM4bitDecoder::SetPlaying(info, false);
		}
		break;


		default:
			Assert(0 && "Unknown audio decoder type.");
		break;
	}
}
