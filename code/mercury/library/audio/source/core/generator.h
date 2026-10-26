#ifndef _GENERATOR_H_
#define _GENERATOR_H_


namespace Sys
{
	class I_File;
}

namespace Audio
{
	namespace Core
	{
#if !defined (PLATFORM_SPU) && !defined (PLATFORM_PS3)
////////////////////////////////////////////////////////////////////////////////
//	WaveWriter
////////////////////////////////////////////////////////////////////////////////
		class WaveWriter
		{
		public:
			WaveWriter( const char* pchName, U32 uNumChannels, bool bFoldDown );
			~WaveWriter();

			bool Write( F32* pFrameData, F32 fLevel = 1.f );

			static WaveWriter* CreateWriter( const char* pchName, U32 uNumChannels, bool bFoldDown )
			{
				WaveWriter* pWriter = new WaveWriter( pchName, uNumChannels, bFoldDown );

				if( pWriter->m_bError )
				{
					delete pWriter;
					pWriter = NULL;
				}

				return pWriter;
			}

		private:
			void StartWave( Sys::I_File* pFile );
			void EndWave( Sys::I_File* pFile );

			void WriteEndianBytes( Sys::I_File* pFile, void* pData, U32 uSize );

		private:
			bool m_bFoldDown;
			bool m_bError;
			U32 m_uNumChannels;
			U32 m_uBytesWritten;
			Sys::I_File* m_apFile[ 6 ];
		};
#endif

////////////////////////////////////////////////////////////////////////////////
//	Generator
////////////////////////////////////////////////////////////////////////////////
		class Generator
		{
		public:
			static void* operator new( size_t sz, void* pHere ) { return pHere; };
			static void operator delete( void* ptr, void* pHere ) { };
			//	don't use
			static void* operator new( size_t sz ) { return new U8[sz]; }
			static void operator delete( void* ptr ) { delete[] (U8*)ptr; }

		public:
			Generator( const char* pchInfo, U32 u32NumChannels, bool bAlwaysOn );
			audio_virtual ~Generator();

			inline void ResetEffects( DSP::Element* aEffects, U16 uEffects )
			{
				for (U32 uEffect = 0; uEffect < uEffects; uEffect++)
				{
					aEffects[ uEffect ].eType = DSP::Effect::eUndefined;
					aEffects[ uEffect ].uOffset = 0;
				}
			}

#ifndef PLATFORM_SPU
			void DynamicBufferDepths( const ProcessInfo& info, BufferStack& buffers );
			void DynamicBufferOffsets( const ProcessInfo& info, BufferStack& buffers );
			U32 StaticBufferOffsets(const ProcessInfo& info, U32 uSize);
#endif	//	#ifndef PLATFORM_SPU

			bool Process( const ProcessInfo& info );

#ifndef PLATFORM_SPU
            bool SetEffect(U32 uIndex, U32 eType, U32 uOffset);
#endif
			void SetTrailFrames( const ProcessInfo& info );

            inline const U32 Channels() const { return m_u32NumChannels; }

			inline void Reset() { m_bProcess = false; };

			inline const bool IsActive() const { return m_bProcessing; };
			inline void SetActive( bool bActive ) { m_bProcessing = bActive; };

			inline const Type GetType() const { return (Type)m_eType; }

#ifdef DUMP_PROCESSING_STATS
			virtual void DumpProcessingStatus( const ProcessInfo& info, const char* pchInstance, U32 uDepth );
#endif

		protected:
            inline U32 FrameCount() { return m_uFrameCount; }

		public:
#ifndef CFG_GOLD
			static const U32 TotalMix() { return m_uTotalMix; }
			static const U32 ActiveMix() { return m_uActiveMix; }
			static const U32 TotalWave() { return m_uTotalWave; }
			static const U32 ActiveWave() { return m_uActiveWave; }
#endif

#ifndef CFG_GOLD
            static U32 m_uTotalGen;
            static U32 m_uActiveGen;
            static U32 m_uTotalMix;
            static U32 m_uActiveMix;
            static U32 m_uTotalWave;
            static U32 m_uActiveWave;
#endif

		protected:

#ifdef PLATFORM_SPU
			U32 m_u32SpuVFTablePad;
#endif
			U32 m_uBuffer;

			U32 m_uFrameCount;

			//U32 m_uTrailFrames;

			const char* m_pchInfo;

			U32 m_uTrailFrames;

			U16 m_eType;

			U16 m_u32NumChannels;

			U16 m_uEffects;

			U16 m_uClassSize;

            bool m_bDynamicBuffMem;
            bool m_bDynamicBuffSizes;
            bool m_bStaticBuffMem;

			bool m_bProcessing;

            bool m_bProcess;
            bool m_bAlwaysOn;
		};
	};
};

#endif // _GENERATOR_H_

