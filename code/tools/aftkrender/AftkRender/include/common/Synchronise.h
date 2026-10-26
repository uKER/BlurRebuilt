#ifndef AFTKRENDER_SYNCHRONISE_H
#define AFTKRENDER_SYNCHRONISE_H

#ifdef _MSC_VER
#pragma once
#endif

#define DECLARE_SYNCHRONISED									\
	public:														\
		bool MutexWait(const U32 timeMs = WAIT_TIME_INFINITE);	\
		void MutexRelease();									\
	protected:													\
		MT::Mutex	m_Mutex;


#define IMPLEMENT_SYNCHRONISED(className)			\
	bool className::MutexWait(const U32 timeMs)		\
	{												\
		return m_Mutex.Wait(timeMs);				\
	}												\
	void className::MutexRelease()					\
	{												\
		m_Mutex.Release();							\
	}


namespace AftkRender
{
	template <typename T>
	class Synchronise
	{
	public:
		Synchronise(T* pObj)
		: m_pObj(pObj)
		{
			assert(m_pObj && "Null synchonised access object");
			m_pObj->MutexWait();
		}

		~Synchronise()
		{
			m_pObj->MutexRelease();
		}

	private:
		T* m_pObj;
	};
};

#endif