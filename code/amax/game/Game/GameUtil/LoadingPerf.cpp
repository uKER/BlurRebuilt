#include "stdafx.h"

#include "LoadingPerf.h"

#ifdef ENABLE_LOAD_PERF

//#define ENABLE_LOAD_PERF_DVD_EMULATION_MARKERS

namespace LoadPerf
{
	Debug::Logger &g_Log = Debug::GetLogger("game.loadingperf");

	C_Event::C_Event(char* pName)
	{
		m_Name = pName;
		m_pParent = NULL;
	}

	C_Event::~C_Event()
	{
		for(Ut::Vector<C_Event*>::Iterator i=m_ChildEvents.Begin(); i!=m_ChildEvents.End(); ++i)
		{
			C_Event* pChildEvent = *i;
			delete pChildEvent;
		}
	}

	void C_Event::Begin()
	{
		m_TimeBegin = Time::GetPerformanceCounter();
	}

	void C_Event::End()
	{
		m_TimeEnd = Time::GetPerformanceCounter();
	}

	C_Event* C_Event::Push(char* name)
	{
		C_Event* pNewChild = new C_Event(name);
		pNewChild->Begin();
		pNewChild->m_pParent = this;
		m_ChildEvents.PushBack(pNewChild);

		return pNewChild;
	}

	C_Event* C_Event::Pop()
	{
		End();
		return m_pParent;
	}

	void C_Event::Dump(const Util::C_String& prefix)
	{
		Time::C_Time duration = m_TimeEnd - m_TimeBegin;
		LOGDEBUG(g_Log, "%s %s - %s", prefix.Chars(), m_Name.Chars(), Time::FormatProfileDurationString(duration).Chars());
		for(Ut::Vector<C_Event*>::Iterator i=m_ChildEvents.Begin(); i!=m_ChildEvents.End(); ++i)
		{
			(*i)->Dump(prefix + "...");
		}
	}

	C_Event* C_Event::GetParent() const
	{
		return m_pParent;
	}
	
	const Util::C_String& C_Event::GetName() const
	{
		return m_Name;
	}


	C_Event* g_CurrLoadEvent = NULL;
	
	bool ProfilingStarted()
	{
		return (g_CurrLoadEvent != NULL);
	}

	void Push(char* name)
	{
		LOGDEBUG( g_Log, "%s", name );

		#ifdef ENABLE_LOAD_PERF_DVD_EMULATION_MARKERS
		{
			Util::C_String filename;
			filename.Printf("dontship\\LOADING_PERF_%s.txt", name);
			Sys::I_File* pFile =  Sys::C_FileSystem::Instance().Open(filename);
			if(pFile)
			{
				char pBuf[10];
				pFile->Read(pBuf, 10);
				SAFE_DELETE_PTR(pFile);
			}
		}
		#endif

		if(g_CurrLoadEvent == NULL)
		{
			g_CurrLoadEvent = new C_Event(name);
			g_CurrLoadEvent->Begin();
		}
		else
		{
			g_CurrLoadEvent = g_CurrLoadEvent->Push(name);
		}
	}

	void Pop()
	{
		BIZ_ASSERT(g_CurrLoadEvent != NULL);

		if (!g_CurrLoadEvent)
			return;

		if(g_CurrLoadEvent->GetParent() == NULL)
		{
			g_CurrLoadEvent->End();
			g_CurrLoadEvent->Dump("");
			delete g_CurrLoadEvent;
			g_CurrLoadEvent = NULL;
		}
		else
		{
			g_CurrLoadEvent = g_CurrLoadEvent->Pop();
		}
	}
	
	void Pop(char* name)
	{
		assert(g_CurrLoadEvent != NULL);
		assert(strcmp(name, g_CurrLoadEvent->GetName().Chars()) == 0);
		Pop();
	}

	C_ScopedEvent::C_ScopedEvent(char* name)
	{
		Push(name);
	}

	C_ScopedEvent::~C_ScopedEvent()
	{
		Pop();
	}
};

#endif
