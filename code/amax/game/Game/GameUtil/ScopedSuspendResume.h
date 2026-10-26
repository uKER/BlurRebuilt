#ifndef GAME_SCOPED_SUSPEND_RESUME_H
#define GAME_SCOPED_SUSPEND_RESUME_H

#ifdef _MSC_VER
#pragma once
#endif

class C_ScopedDeviceSuspend
{
public:
	C_ScopedDeviceSuspend()
		: m_Suspended(false)
	{
		Suspend();
	}
	~C_ScopedDeviceSuspend()
	{
		if(m_Suspended)
			Resume();
	}
	void Suspend()
	{
		m_Suspended = true;
	}

	void Resume()
	{
		m_Suspended = false;
	}

private:
	bool m_Suspended;
};

#endif // GAME_SCOPED_SUSPEND_RESUME_H
