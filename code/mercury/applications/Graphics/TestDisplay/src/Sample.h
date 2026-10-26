#ifndef SAMPLE_H
#define SAMPLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Application.h"

class C_Sample
{
public:

	C_Sample(C_Application* pApplication);
	virtual ~C_Sample();

	bool Create();
	bool CreateVolatile();
	void DestroyVolatile();
	void Destroy();

	void Update();
	void Render();

protected:

	virtual bool OnCreate();
	virtual bool OnCreateVolatile();
	virtual void OnDestroyVolatile();
	virtual void OnDestroy();

	virtual void OnUpdate();
	virtual void OnRender();

protected:

	C_Application* m_pApplication;
};

#endif // SAMPLE_H
