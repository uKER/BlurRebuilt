#ifndef EDITOR_WORKER_H
#define EDITOR_WORKER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/threads/Thread.h"
#include "Delegate.h"

namespace Editor
{

	//Helper classes to call functions (via our Delegate class) from a separate thread.

	template <typename R>
	class C_Worker0 : public MT::Thread
	{
	public:
		C_Worker0(Delegate<R()> callback)
		: m_Callback(callback)
		{ }

		~C_Worker0()
		{ }

	protected:
		void ThreadMain() { m_Callback(); }

		Delegate<R()>	m_Callback;
	};

	template <typename R, typename P1>
	class C_Worker1 : public MT::Thread
	{
	public:
		C_Worker1(Delegate<R(P1)> callback, P1 param1)
		: m_Callback(callback)
		, m_Param1(param1)
		{
		}

		~C_Worker1()
		{
		}

	protected:
		void ThreadMain() { m_Callback(m_Param1); }

		P1				m_Param1;
		Delegate<R(P1)>	m_Callback;
	};

	template <typename R, typename P1, typename P2>
	class C_Worker2 : public MT::Thread
	{
	public:
		C_Worker2(Delegate<R(P1,P2)> callback, P1 param1, P2 param2)
		: m_Callback(callback)
		, m_Param1(param1)
		, m_Param2(param2)
		{ }

		~C_Worker2()
		{ }

	protected:
		void ThreadMain() { m_Callback(m_Param1, m_Param2); }

		P1				m_Param1;
		P2				m_Param2;
		Delegate<R(P1,P2)>	m_Callback;
	};
}

#endif