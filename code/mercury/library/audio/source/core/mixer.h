#ifndef _MIXER_H_
#define _MIXER_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Mixer
////////////////////////////////////////////////////////////////////////////////
		class Mixer : public Generator
		{
		public:
#ifndef PLATFORM_SPU
			static U32 MixerSize( U16 uInputs, U16 uChannelsIn, U16 uChannelsOut, U16 uEffects );

		public:
			Mixer( const char* pchInfo, U16 uInputs, U16 uChannelsIn, U16 uChannelsOut, U16 uEffects, bool bAlwaysOn );
			audio_virtual ~Mixer();

            void DynamicBufferDepths( const ProcessInfo& info, BufferStack& buffers );
            void DynamicBufferOffsets( const ProcessInfo& info, BufferStack& buffers );
            U32 StaticBufferOffsets( const ProcessInfo& info, U32 uSize );

            bool SetInput( U32 uInput, U32 uType, U32 uOffset );

#endif	//	#ifndef PLATFORM_SPU

			bool SetProcessFlag( const ProcessInfo& info, bool bProcess );
			void Process( const ProcessInfo& info );

			F32* OutputBuffer( const ProcessInfo& info ) { return &info.pDynamicBuffers[ m_uBuffer ]; }

#ifdef ENABLE_DEBUG_EXTENSIONS
			void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL);
#endif

			inline void SetGain(U32 uInput, U32 uChIn, U32 uChOut, F32 fGain)
			{
				#ifdef CFG_DEBUG
				if (!Maths::IsFinite(fGain))
				{
					AUDIO_CORE_ERROR("Mixer::SetGain() Gain value is NAN! '%s'. chanIn = %d, chanOut = %d", m_pchInfo, uChIn, uChOut);
					return;
				}
				#endif

				U8* pThis = (U8*)this + m_uClassSize;
				F32* afGain = (F32*)&pThis[ m_uGainOffset ];

				afGain[uInput * m_uChannelsIn * m_uChannelsOut + uChIn * m_uChannelsOut + uChOut] = fGain;
			}

			inline void SetGains(U32 uInput, F32* afGains)
			{
				#ifndef CFG_SPEED
				for (S32 i=0; i<m_uChannelsIn * m_uChannelsOut; i++)
				{
					if (!Maths::IsFinite(afGains[i]))
					{
						AUDIO_CORE_ERROR("Mixer::SetGains() Gain value(s) is NAN! '%s'.", m_pchInfo);
						return;
					}
				}
				#endif

				U8* pThis = (U8*)this + m_uClassSize;
				F32* afGain = (F32*)&pThis[ m_uGainOffset ];

				memcpy(&afGain[uInput * m_uChannelsIn * m_uChannelsOut], afGains, m_uChannelsIn * m_uChannelsOut * sizeof(F32));
			}

#ifdef DUMP_PROCESSING_STATS
			void DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth );
#endif

		protected:
			U16 m_uInputs;
			U16 m_uInputOffset;

			U16 m_uChannelsIn;
			U16 m_uChannelsOut;

			U16 m_uGainOffset;
			U16 m_uPrevGainOffset;
		};
	};
};

#endif // _MIXER_H_


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
