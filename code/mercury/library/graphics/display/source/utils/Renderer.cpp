#include "stdafx.h"
#include "Display/Utils/Renderer.h"
#include "Display/Device/Device.h"

namespace Display
{

I_Renderer::I_Renderer(const Util::C_String& name) :
	m_Name(name),
	m_Type(E_RendererType_Dynamic),
	m_pResourceBundle(0)
{
	memset(m_Created, 0, sizeof(m_Created));
	m_Initialised = false;

	Device()->InsertRenderer(this);
}

I_Renderer::~I_Renderer()
{
	assert(m_Created[E_RendererState_Involatile] == false);
	assert(m_Created[E_RendererState_Volatile] == false);
	assert(m_Initialised == false);
	
	Device()->RemoveRenderer(this);
}

void I_Renderer::SetRendererType(const E_RendererType type)
{
	m_Type = type;
}

bool I_Renderer::Create()
{
	if(!m_Created[E_RendererState_Involatile])
	{
		m_Created[E_RendererState_Involatile] = OnCreate();

		// if creation failed call destroy incase of partial creation
		if(!m_Created[E_RendererState_Involatile])
			OnDestroy();

		return m_Created[E_RendererState_Involatile];
	}
	else
	{
		return true;
	}
}

bool I_Renderer::CreateVolatile()
{
	// [Blur2PC] a renderer whose involatile Create failed (Bink-less movies) has nothing to create;
	// letting it fail here made C_ApplicationPC skip the global colour/depth buffers.
	if(!m_Created[E_RendererState_Involatile])
		return true;
	if(!m_Created[E_RendererState_Volatile])
	{
		m_Created[E_RendererState_Volatile] = OnCreateVolatile();

		if(!m_Created[E_RendererState_Volatile])
			OnDestroyVolatile();

		return m_Created[E_RendererState_Volatile];
	}
	else
	{
		return true;
	}
}

void I_Renderer::DestroyVolatile()
{
	if(m_Created[E_RendererState_Volatile])
	{
		OnDestroyVolatile();
		m_Created[E_RendererState_Volatile] = false;
	}
}

void I_Renderer::Destroy()
{
	if(m_Created[E_RendererState_Involatile])
	{
		OnDestroy();
		m_Created[E_RendererState_Involatile] = false;
	}
}

bool I_Renderer::Initialise()
{
	// return true if system is already initialised
	if(m_Initialised)
		return true;

	m_Initialised = OnInitialise();
	
	// if system failed to initialise call shutdown incase of a partial initialisation
	if(!m_Initialised)
		OnShutdown();

	return m_Initialised;
}

bool I_Renderer::Reinitialise()
{
	// if system is already initialised call the shutdown method
	if(m_Initialised)
		Shutdown();

	return Initialise();
}

void I_Renderer::Shutdown()
{
	OnShutdown();
	m_Initialised = false;
}

void I_Renderer::Reset()
{
	OnReset();
}

bool I_Renderer::PreResize()
{
	return OnPreResize();
}

bool I_Renderer::PostResize()
{
	return OnPostResize();
}

bool I_Renderer::OnCreate()
{
	return true;
}

bool I_Renderer::OnCreateVolatile()
{
	return true;
}

void I_Renderer::OnDestroyVolatile()
{
}

void I_Renderer::OnDestroy()
{
}

bool I_Renderer::OnInitialise()
{
	return true;
}

void I_Renderer::OnShutdown()
{
}

void I_Renderer::OnReset()
{
}

bool I_Renderer::OnPreResize()
{
	return true;
}

bool I_Renderer::OnPostResize()
{
	return true;
}

void I_Renderer::DestroyRenderer()
{
	DestroyVolatile();
	Destroy();
}

}; // namespace Display
