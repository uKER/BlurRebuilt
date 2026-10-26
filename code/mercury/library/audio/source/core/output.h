#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#if !defined (CFG_GOLD) && !defined (PLATFORM_SPU) && !defined (PLATFORM_PS3)
//#define AUDIO_OUTPUT_RECORD
#endif

#define AUDIO_MAX_INPUTS_PER_OUTPUT 4

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Output
////////////////////////////////////////////////////////////////////////////////
		class Output : public Generator
		{
		public:
			static U32 OutputSize( U16 uChannelsIn, U16 uEffects );

		public:
#ifndef PLATFORM_SPU
			Output(const char* pchInfo, U32 u32NumChannels, U16 uEffects);
			//audio_virtual ~Output() {}

			void DynamicBufferDepths(const ProcessInfo& info, BufferStack& buffers);	//	TODO: just let object/output have access to m_uBuffer
			void DynamicBufferOffsets(const ProcessInfo& info, BufferStack& buffers);
            U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize);
#endif	//	#ifndef PLATFORM_SPU

			void SetProcessFlag( const ProcessInfo& info );
			void Process( const ProcessInfo& info );
			void MoveAudio( const ProcessInfo& info );
			void MoveAudioBatch( const ProcessInfo& info );
			
			void SetInput(U32 uInput, U32 uOffset)
			{
				Assert(uOffset!=(U32)-1);
				m_Item.eType = (Type)uInput;
				m_Item.uOffset = uOffset;
			}

			inline Item& GetItem() { return m_Item; }

			void AddInput( Input* pInput );
			void RemoveInput( Input* pInput );

            const U32 Channels() const { return m_u32NumChannels; }

			void SetMuted( bool bMuted ) { m_bMuted = bMuted; }

#ifdef DUMP_PROCESSING_STATS
			void DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth );
#endif

#ifdef ENABLE_DEBUG_EXTENSIONS
			void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL);
#endif

#if defined (AUDIO_OUTPUT_RECORD)
			void WriteWave( const ProcessInfo& info );
#endif
		protected:
			Item m_Item;

			Input* m_apInputs[ AUDIO_MAX_INPUTS_PER_OUTPUT ];

			U32 m_uOutBuffer;
			bool m_bMuted;
			bool m_bMakeData;

#if defined (AUDIO_OUTPUT_RECORD)
			WaveWriter* m_pWriter;
#endif
		};
	};
};

#endif // _OUTPUT_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
