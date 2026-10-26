#ifndef FRAMEWORK_APPLICATION_PARAMS_H
#define FRAMEWORK_APPLICATION_PARAMS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Framework
{

#if defined(PLATFORM_PS3)

//! C_ApplicationParamsPS3. Params for creating the application on the PS3.

class C_ApplicationParamsPS3
{
public:

	C_ApplicationParamsPS3();
	~C_ApplicationParamsPS3();
};

#elif defined(PLATFORM_PC)

//! C_ApplicationParamsPC. Params for creating the application on the PC.

class C_ApplicationParamsPC
{
public:

	C_ApplicationParamsPC();
	~C_ApplicationParamsPC();

public:

	S32 							m_IconResourceLarge;
	S32 							m_IconResourceSmall;
	HINSTANCE						m_hInstance;
};

#endif

//! C_ApplicationParams. Params for creating the application.

class C_ApplicationParams
{
public:

	C_ApplicationParams();
	~C_ApplicationParams();

public:

	#if defined(PLATFORM_PS3)
	C_ApplicationParamsPS3				m_PS3;
	#elif defined(PLATFORM_PC)
	C_ApplicationParamsPC				m_PC;
	#endif

	bool								m_NetworkStackAlreadyStarted;
	U32 								m_InputButtonCount;
	U32 								m_InputAxisCount;
	U32 								m_InputUserCount;
	U32									m_ViewCount;
};

}; // namespace Framework

#endif // FRAMEWORK_APPLICATION_PARAMS_H
