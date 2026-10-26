#include "stdafx.h"

#include "Display/Device/DeviceContext.h"

namespace Display
{

/// invalidate state of all the contexts
void C_DeviceContext::InvalidateState()
{
	for (int i=0; i<m_Contexts.Size(); i++)
		m_Contexts[i]->InvalidateState();
}

/// invalidate state of specified contexts
void C_DeviceContext::InvalidateState(const U32 contextFlags)
{
	if(contextFlags)
	{
		for (int i=0; i<m_Contexts.Size(); i++)
		{
			if(m_Contexts[i]->GetId() & contextFlags)
				m_Contexts[i]->InvalidateState();
		}
	}
}

/// set context flags for next rendering
void C_DeviceContext::SetContextFlags(const U32 contextFlags)
{
	if(contextFlags != m_ContextFlags)
	{
		U32 invalidateFlags = contextFlags & ~m_ContextFlags;
		InvalidateState(invalidateFlags);
		m_ContextFlags = contextFlags;
	}
}

/// constructor
I_Context::I_Context()
	: m_pParent(NULL)
{
}

I_Context*	C_DeviceContext::m_RegisteredContexts[DISPLAY_MAX_CONTEXT_TYPES] = {0};
int			C_DeviceContext::m_nRegisteredContexts = 0;

/// register a new type of context, returns the handle for the new context
int C_DeviceContext::RegisterContext(I_Context *p)
{
	assert(m_nRegisteredContexts < DISPLAY_MAX_CONTEXT_TYPES);
	m_RegisteredContexts[m_nRegisteredContexts++] = p;
	return m_nRegisteredContexts-1;
}

/// constructor
C_DeviceContext::C_DeviceContext()
{
	int n = m_nRegisteredContexts;
	m_Contexts.Resize(n);
	for (int i=0; i<n; i++)
	{
#if PLATFORM_SPU
		m_Contexts[i] = m_RegisteredContexts[i];
#else
		m_Contexts[i] = m_RegisteredContexts[i]->New();
#endif
		m_Contexts[i]->m_pParent = this;
		m_Contexts[i]->m_Id = 1 << i;
	}
	m_ContextFlags = 0xFFFFFFFF;
}

/// destructor
C_DeviceContext::~C_DeviceContext()
{
#if !PLATFORM_SPU
	for (int i=0; i<m_Contexts.Size(); i++)
		delete m_Contexts[i];
#endif
}

/// call the one-time initialisation function 'Initialise' on all of the contexts
void C_DeviceContext::Initialise()
{
#if !PLATFORM_SPU
	for (int i=0; i<m_Contexts.Size(); i++)
		m_Contexts[i]->Initialise();
#endif
}

#if !PLATFORM_SPU

/// clone the state of the context that is passed in into this context
void C_DeviceContext::CloneState(const C_DeviceContext *pContext)
{
	if (pContext)
	{
		const Btl::Vector<I_Context*, Btl::VectorParameters_Static<8> > &contexts = pContext->m_Contexts;
		assert(m_Contexts.Size() == contexts.Size());
		for (int i=0; i<contexts.Size(); i++)
			m_Contexts[i]->CloneState(contexts[i]);
	}
	else
	{
		for (int i=0; i<m_Contexts.Size(); i++)
			m_Contexts[i]->CloneState(NULL);
	}
	m_ContextFlags = 0xFFFFFFFF;
}

/// free all registered context types
void C_DeviceContext::FreeRegisteredContexts()
{
	for (int i=0; i<C_DeviceContext::m_nRegisteredContexts; i++)
		delete C_DeviceContext::m_RegisteredContexts[i];
}


// class to clean up registered contexts
class C_GlobalDeviceContext
{
public:
	~C_GlobalDeviceContext()
	{
		C_DeviceContext::FreeRegisteredContexts();
	}
};

static C_GlobalDeviceContext g_GlobalDeviceContext;
#endif // !PLATFORM_SPU

}; // namespace Display

