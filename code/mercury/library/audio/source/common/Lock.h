#ifndef _LOCK_H_
#define _LOCK_H_

#if defined (PLATFORM_PS3)
#include "pthread.h" // for pthread_mutex_t
#endif // _PS3 

#include "core/threads/Mutex.h"

namespace Audio
{
	namespace Core
	{
////////////////////////////////////////////////////////////////////////////////
//	Lock
////////////////////////////////////////////////////////////////////////////////
		class Lock : public MT::Lock
        {
        public:
			bool Acquire()
			{
				Enter();
				return true;
			}
			bool Release()
			{
				Exit();
				return true;
			}

            void Increment(volatile S32* psThis, S32 sBy)
            {
				MT::AtomicAdd((U32*)psThis, sBy);
            };
        };

		class ThreadSafeIndex
		{
		public:
			ThreadSafeIndex() { m_u32Index = 0; }
			ThreadSafeIndex(U32 u32InitValue) { m_u32Index = u32InitValue; }

			void operator =(const U32 u32)
			{
#if defined(PLATFORM_PC)
				InterlockedExchange((LONG*)(&m_u32Index), u32);
#elif defined(PLATFORM_PS3)
				cellAtomicStore32((uint32_t*)&m_u32Index, u32);
#endif
			}

			void operator +=(const U32 u32)
			{
#if defined(PLATFORM_PC)
				InterlockedExchangeAdd((LONG*)&m_u32Index, u32);
#elif defined(PLATFORM_PS3)
				cellAtomicAdd32((uint32_t*)&m_u32Index, u32);
#endif
			}

//			const U32 operator +(const U32& u32) { return u32 + m_u32Index; }
			operator U32() { return m_u32Index; }

		private:
			volatile U32	m_u32Index;
		};
    }
}

#endif // _LOCK_H_

