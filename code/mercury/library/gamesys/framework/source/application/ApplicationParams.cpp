#include "FrameworkPch.h"
#include "Framework/Application/ApplicationParams.h"

namespace Framework
{

#if defined(PLATFORM_PS3)

// C_ApplicationParamsPS3

C_ApplicationParamsPS3::C_ApplicationParamsPS3()
{
}

C_ApplicationParamsPS3::~C_ApplicationParamsPS3()
{
}

#elif defined(PLATFORM_PC)

// C_ApplicationParamsPC

C_ApplicationParamsPC::C_ApplicationParamsPC()
{
	m_IconResourceLarge					= 0;
	m_IconResourceSmall					= 0;
	m_hInstance							= 0;
}

C_ApplicationParamsPC::~C_ApplicationParamsPC()
{
}

#endif

// C_ApplicationParams

C_ApplicationParams::C_ApplicationParams()
{
	m_NetworkStackAlreadyStarted		= false;
	m_InputButtonCount					= 0;
	m_InputAxisCount					= 0;
	m_InputUserCount					= 1;
	m_ViewCount							= 4;
}

C_ApplicationParams::~C_ApplicationParams()
{
}

}; // namespace Framework
