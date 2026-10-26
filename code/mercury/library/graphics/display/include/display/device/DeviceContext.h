#ifndef DISPLAY_DEVICE_DEVICE_CONTEXT_H
#define DISPLAY_DEVICE_DEVICE_CONTEXT_H

namespace Display
{
class C_DeviceContext;

#define DISPLAY_MAX_CONTEXT_TYPES 8

class I_Context
{
	friend class C_DeviceContext;
public:
	/// constructor - shouldn't only be called by the DEFINE_CONTEXT macro
	I_Context();

	/// destructor
	virtual ~I_Context() {}

	/// allocate a new instance of this class
	virtual I_Context*	New() = 0;

	/// initialisation function
	virtual void		Initialise() = 0;

	/// clone the state of another context into this context
	virtual void		CloneState(const I_Context *pContext) = 0;
	/// invalidate all state stored on this context
	virtual void		InvalidateState() = 0;

	const U32			GetId() const { return m_Id; }
	const U32			GetFlags() const { return m_Flags; }

	/// pointer to device context, so this context can access other contexts
	C_DeviceContext*	m_pParent;

protected:
	/// id flag to uniquely identify this context
	U32					m_Id;
	/// flags to indicate dependent contexts
	U32					m_Flags;
};

/// use this macro in a .h file to declare a context.
/// The N argument determines the name of the Get'N'() function
/// The T argument determines the typename of the class that is to be registered
#define DECLARE_CONTEXT(N, T) \
extern int g_iContextHandle##T; \
class T; \
T *Get##N(const ::Display::C_DeviceContext *p); \
T *Get##N(const ::Display::I_Context *p);

/// use this macro in the .cpp file to define a context. see above for a description of the parameters
#if !PLATFORM_SPU
	#define DEFINE_CONTEXT(N, T) \
	int g_iContextHandle##T = ::Display::C_DeviceContext::RegisterContext(new T); \
	T *Get##N(const ::Display::C_DeviceContext *p) \
	{ \
		return (T*)p->GetContext(g_iContextHandle##T); \
	} \
	T *Get##N(const ::Display::I_Context *p) \
	{ \
		return (T*)p->m_pParent->GetContext(g_iContextHandle##T); \
	}
#else
	#define DEFINE_CONTEXT(N, T) \
	T g_##T; \
	int g_iContextHandle##T = ::Display::C_DeviceContext::RegisterContext(&g_##T); \
	T *Get##N(const ::Display::C_DeviceContext *p) \
	{ \
		return (T*)p->GetContext(g_iContextHandle##T); \
	} \
	T *Get##N(const ::Display::I_Context *p) \
	{ \
		return (T*)p->m_pParent->GetContext(g_iContextHandle##T); \
	}
#endif


/// this class represent the context in which rendering occurs on a particular thread
/// it's possible to use the REGISTER_CONTEXT macro to register sub-contexts with this class, which will then be created
/// inside every instance of this C_DeviceContext class.
/// You can then use the Get template to retrieve these contexts from the C_DeviceContext at any point
class C_DeviceContext
{
public:
	/// constructor
	C_DeviceContext();

	/// destructor
	~C_DeviceContext();

	/// call the one-time initialisation function 'Initialise' on all of the contexts
	void Initialise();

	/// return context pointer for given context handle
	I_Context *GetContext(int iHandle) const { return m_Contexts[iHandle]; }

	/// clone the state of the context that is passed in into this context
	void CloneState(const C_DeviceContext *pContext);

	/// invalidate state of all the contexts
	void InvalidateState();

	/// invalidate state of specified contexts
	void InvalidateState(const U32 contextFlags);

	/// set context flags for next rendering
	void SetContextFlags(const U32 contextFlags);

	/// get current context flags
	const U32 GetContextFlags() const { return m_ContextFlags; }

	/// register a new type of context, returns the handle for the new context
	static int RegisterContext(I_Context *pContext);

	/// free all registered context types
	static void FreeRegisteredContexts();

private:
	/// pointers to contexts
	Btl::Vector<I_Context*, Btl::VectorParameters_Static<DISPLAY_MAX_CONTEXT_TYPES> >
						m_Contexts;

	U32					m_ContextFlags;
	static I_Context*	m_RegisteredContexts[DISPLAY_MAX_CONTEXT_TYPES];
	static int			m_nRegisteredContexts;
};

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_CONTEXT_H
