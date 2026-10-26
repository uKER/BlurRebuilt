#include "stdafx.h"
#include <core/core.h>
#include <core/utility/singleton.h>
#include "wrap32/interface/wrap32_instance.h"
#include <psapi.h>
#include "wrap32/common/wrap32_utility.h"

C_Wrap32Instance::C_Wrap32Instance()
{
	//Cleanup existing COM process instance(s).
	C_Wrap32Utility::TerminateProcessesByName("wrap32_com.exe");

	::CoInitialize(NULL);

	HRESULT hr = m_serverInstance.CreateInstance(__uuidof(Wrap32));
	if (!SUCCEEDED(hr))
	{
		printf("FAIL: Couldn't create instance of Wrap32 COM object.\n");
		OutputDebugString("FAIL: Couldn't create instance of Wrap32 COM object.\n");
		//Terminate
		exit(1);
	}
}

C_Wrap32Instance::~C_Wrap32Instance()
{
	m_serverInstance.Release();
	::CoUninitialize();
};

IServerWrapperPtr C_Wrap32Instance::GetServerInstance()
{
	return m_serverInstance;
}
