#include "StdAfx.h"
#include "Sample.h"

C_Sample::C_Sample(C_Application* pApplication)
{
	BIZ_ASSERT(pApplication);
	m_pApplication = pApplication;
}

C_Sample::~C_Sample()
{
}

bool C_Sample::Create()
{
	return OnCreate();
}

bool C_Sample::CreateVolatile()
{
	return OnCreateVolatile();
}

void C_Sample::DestroyVolatile()
{
	OnDestroyVolatile();
}

void C_Sample::Destroy()
{
	OnDestroy();
}

void C_Sample::Update()
{
	OnUpdate();
}

void C_Sample::Render()
{
	OnRender();
}

bool C_Sample::OnCreate()
{
	return true;
}

bool C_Sample::OnCreateVolatile()
{
	return true;
}

void C_Sample::OnDestroyVolatile()
{
}

void C_Sample::OnDestroy()
{
}

void C_Sample::OnUpdate()
{
}

void C_Sample::OnRender()
{
}
