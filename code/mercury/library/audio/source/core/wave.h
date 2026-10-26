#ifndef _WAVE_H_
#define _WAVE_H_

//#include "../common/file.h"

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Wave
////////////////////////////////////////////////////////////////////////////////
		class Wave : public Generator
		{
		public:
			static U32 WaveSize( U16 uChannelsOut, U16 uEffects );

		public:
			static const U32 u32DecoderNone;

			Wave( const char* pchInfo, U32 u32NumChannels, U16 uEffects, bool bUseFastRamp, bool bAlwaysOn );
			audio_virtual ~Wave();

            void SetDecoder( U32 uDecoderType, U32 uDecoder );

#ifdef AUDIO_NEW_DECODERS
			void SetDecoder( DecoderBase* pDecoder ) { m_pDecoder = pDecoder; }
#endif // AUDIO_NEW_DECODERS
			
			Decoder* GetDecoder(const ProcessInfo& info) { return (m_uDecoder == u32DecoderNone || m_uDecoderType == u32DecoderNone) ? NULL : (Decoder*)&info.pCoreData[m_uDecoder]; }

			F32* OutputBuffer( const ProcessInfo& info ) { return &info.pDynamicBuffers[ m_uBuffer ]; }

			bool SetProcessFlag( const ProcessInfo& info, bool bProcess );
			void Process( const ProcessInfo& info );

            //  wave operations
            void Start(const ProcessInfo& info);
            void Stop(const ProcessInfo& info);
            void Reset(const ProcessInfo& info, U32 uOffset);

			inline bool IsPlaying() { return m_bPlaying || m_bProcessing; }

            inline void SetVolume(F32 fVolume) { m_fVolume = fVolume; }

			void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL);

//			bool HasCallbackFired(const ProcessInfo& info) { Decoder* pDecoder = GetDecoder(info); if (! pDecoder) return false; return pDecoder->HasCallbackFired(); }

        private:
            F32 m_fPrevSubVolume;
            F32 m_fPrevVolume;
            F32 m_fVolume;

			U32 m_uDecoderType;
			U32 m_uDecoder;
#ifdef AUDIO_NEW_DECODERS
			DecoderBase* m_pDecoder;
#endif // AUDIO_NEW_DECODERS

			bool m_bProcess;

			bool m_bPlaying;
			bool m_bDone;
			bool m_bUseFastRamp;
		};
	};
};

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif // _WAVE_H_
