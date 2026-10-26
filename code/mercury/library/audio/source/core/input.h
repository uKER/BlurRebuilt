#ifndef _INPUT_H_
#define _INPUT_H_

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#if !defined (CFG_GOLD) && !defined (PLATFORM_SPU) && !defined (PLATFORM_PS3)
#define AUDIO_INPUT_RECORD
#endif

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Input
////////////////////////////////////////////////////////////////////////////////
        class Input : public Generator
		{
		public:
#ifndef PLATFORM_SPU
			static U32 InputSize( U16 uChannelsOut, U16 uEffects );

		public:
			Input(const char* pchInfo, U32 u32NumChannels, U16 uEffects);
#endif // PLATFORM_SPU

			bool SetProcessFlag( const ProcessInfo& info, bool bProcess );
			void Process( const ProcessInfo& info );
			bool WantsData() const { return (m_bWantsData & (!m_bMuted)); }

            void CopyFrame(ProcessInfo& info);

			void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL);

			void SetAltBuffer(F32* afAltBuffer) { m_afAltBuffer = afAltBuffer; }
			F32* GetAltBuffer() { return m_afAltBuffer; }

			F32* OutputBuffer( const ProcessInfo& info ) { return &info.pDynamicBuffers[ m_uBuffer ]; }

			void SetMuted( bool bMuted ) { m_bMuted = bMuted; }

#if defined (AUDIO_INPUT_RECORD)
			void WriteWave( const ProcessInfo& info );
#endif
		private:
			F32* m_afAltBuffer;

			bool m_bMuted;
			bool m_bWantsData;

#if defined (AUDIO_INPUT_RECORD)
			WaveWriter* m_pWriter;
#endif
		};
	};
};

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF

#endif // _INPUT_H_
