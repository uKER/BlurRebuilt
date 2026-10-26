#ifndef _BATCHFRAMEADD_H_
#define _BATCHFRAMEADD_H_

#if defined (PLATFORM_PS3) 
#define _AMAX_CELL_AUDIO_BLOCK CELL_AUDIO_BLOCK_8
#endif

#if defined (PLATFORM_SPU)
#define _AMAX_CELL_AUDIO_BLOCK 8
#endif


namespace Audio
{
	namespace BatchFrame
	{
	
		class cFrameAdd
		{
			public:
				F32*		mpTo;			// Destination (store result here )
				const F32*	mpFromA;		// Add this
				const F32*	mpFromB;		// to this
				F32			mfLevel;		// 
				cFrameAdd()
				: mpTo( 0 ), mpFromA( 0 ), mpFromB( 0 ) , mfLevel( 0 )
				{
				};
				cFrameAdd( F32*pTo, const F32* pFromA = NULL, const F32* pFromB = NULL, F32 fLevel = NULL)
				: mpTo( pTo ), mpFromA( pFromA ), mpFromB( pFromB ) , mfLevel( fLevel )
				{
				};
		
		};

		class cBatch
		{
			public:
				cFrameAdd*					mpQueue;
				U32							mFrameAddIndex;
				U32							mpQueueSize;
				Scheduler::GroupHandle		mGroup;
#ifdef PLATFORM_PS3
				sys_addr_t					m_pPort;
#endif
			public:
				#ifndef PLATFORM_SPU
				void Clear();
				void Add( F32* pTo, const F32* pFromA = NULL, const F32* pFromB = NULL, const F32 fLevel = 0.0f );
				void Process();
				bool IsEnabled();
				cBatch* Init( U32 queueSize );
				void InitTask( Scheduler::PolicyHandle policy );
				void Shutdown();
				void Wait();
#ifdef PLATFORM_PS3
				void SetAudioPort( sys_addr_t port ) { m_pPort = port; }
#endif
				#endif
		};
	

		extern cBatch gMoveAudioBatch;

	}
}

#endif