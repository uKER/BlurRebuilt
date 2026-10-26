#ifndef _RETURN_H_
#define _RETURN_H_

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Return
////////////////////////////////////////////////////////////////////////////////
        class Return : public Generator
		{
		public:
#ifndef PLATFORM_SPU
			static U32 ReturnSize( U16 uChannelsOut, U16 uEffects );

		public:
            U32 StaticBufferOffsets( const ProcessInfo& info, U32 uSize )
			{
				m_uBuffer = uSize;

                uSize += m_u32NumChannels * uFrameLength;

                return uSize;
			}

			Return(const char* pchInfo, U32 u32NumChannels, U16 uEffects);
#endif // PLATFORM_SPU

			bool SetProcessFlag( const ProcessInfo& info, bool bProcess );
			void Process( const ProcessInfo& info );

			F32* OutputBuffer( const ProcessInfo& info ) { return &info.pStaticBuffers[ m_uBuffer ]; }

			void DumpStats(const ProcessInfo& info, const char* pchPrefix, Core::Internals* pInternals = NULL);

			void SetMuted( bool bMuted ) { m_bMuted = bMuted; }

		private:
			bool m_bMuted;
		};
	};
};

#endif // _RETURN_H_
