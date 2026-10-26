#pragma once

#ifdef _DEBUG
#import "wrap32_com.tlb" no_namespace
#else
#import "wrap32_com.tlb" no_namespace
#endif

#if 0
#include "wrap32_com.tlb"	// this is needed to make JAM pick up the dependency on wrap32_com.tlb
#endif

class C_Wrap32Instance : public Ut::Singleton<C_Wrap32Instance, Ut::CreateStatic>
{
public:
	C_Wrap32Instance();

	virtual ~C_Wrap32Instance();

	IServerWrapperPtr GetServerInstance();

private:
	IServerWrapperPtr m_serverInstance;
};
