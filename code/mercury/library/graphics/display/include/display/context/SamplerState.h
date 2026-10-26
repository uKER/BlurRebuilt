#ifndef __CONTEXT_SAMPLERSTATE_H__
#define __CONTEXT_SAMPLERSTATE_H__

#include <display2/context/context.h>

namespace Display
{

/// for use with Display::Context class
class C_SamplerState : public Display2::C_SamplerState
{
public:
	void SetMaxAnisotropy(S8 x);

	bool operator == (C_SamplerState &other)
	{
		return (m_MaxAnisotropy == other.m_MaxAnisotropy) && IsSame(other);
	}

	void SetDefaultStates()
	{
		m_MaxAnisotropy = 1;
		Display2::C_SamplerState::SetDefaultStates();
	}

	S8 m_MaxAnisotropy;
};

}

#endif
