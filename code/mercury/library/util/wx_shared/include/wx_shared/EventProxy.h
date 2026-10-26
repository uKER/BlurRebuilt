#ifndef WX_SHARED_UI_WXEVENT_PROXY
#define WX_SHARED_UI_WXEVENT_PROXY

#ifdef _MSC_VER
#pragma once
#endif

#include "core/utility/Singleton.h"


//Proxy classes so that wxEvents can be delegated to objects not derived from wxEventHandler

//abstract base class
//derived from wxObject so it can be assigned to wxEvent::m_callbackUserData
class EventDelegate : public wxObject
{
public:
	virtual ~EventDelegate() = 0
	{}

	virtual void ProcessEvent(wxEvent& e) = 0;
};



template <typename E>
struct ConsumeEvent
{
	static void ProcessEvent(E& e)
	{
		//do nothing
	}
};

template <typename E>
struct SkipEvent
{
	static void ProcessEvent(E& e)
	{
		//allow event to propagate
		e.Skip();
	}
};

//Event delegate implementation with behavioural policies for parameterless callbacks
template <class E, class F, template <class> class EventBehaviour = ConsumeEvent>
class EventDelegateImpl : public EventDelegate
{
public:
    EventDelegateImpl(F func)
	: m_Function(func)
	{}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_Function();
		EventBehaviour<E>::ProcessEvent(static_cast<E&>(e));
    }
private:
     F m_Function;
};

//event delegate implementation for callbacks expecting a wxEvent parameter
template <class E, class F>
class EventDelegateImplWx : public EventDelegate
{
public:
    EventDelegateImplWx(F func)
	: m_Function(func)
	{}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_Function(static_cast<E&>(e));
    }
private:
     F m_Function;
};


class EventProxy : public wxEvtHandler, public Ut::Singleton<EventProxy, Ut::CreateUsingNew>
{
public:
    void OnEvent(wxEvent& e)
	{
		static_cast<EventDelegate*>(e.m_callbackUserData)->ProcessEvent(e);
    }
};


class EventConnection
{
public:
	EventConnection(wxEvtHandler* src, wxEventType evtid, wxObject* user, int winid, int lastid)
	: m_pSrcHandler(src)
	, m_EventID(evtid)
	, m_pDelegate(user)
	, m_WinID(winid)
	, m_WinIDLast(lastid)
	{}

    void Disconnect() const
    {
		m_pSrcHandler->Disconnect(m_WinID, m_WinIDLast, wxObjectEventFunction(&EventProxy::OnEvent), m_pDelegate, &EventProxy::Instance());
    }

//private:
	wxEvtHandler*	m_pSrcHandler;
	wxEventType		m_EventID;
	wxObject*		m_pDelegate;
	S32				m_WinID;
	S32				m_WinIDLast;
};

//wire up event to parameterless callback
template<class E, class F, template<class> class EventBehaviour>
EventConnection ConnectEvent0(wxEvtHandler *src, wxEventType evtId, F& fn, S32 winId=wxID_ANY, S32 winIdLast=wxID_ANY)
{
	wxObject* pDelegate(new EventDelegateImpl<E, F, EventBehaviour>(fn));

	src->Connect(winId, winIdLast, evtId, wxObjectEventFunction(&EventProxy::OnEvent), pDelegate, &EventProxy::Instance());
	return EventConnection(src, evtId, pDelegate, winId, winIdLast);
}

//wire up event to wxEvent callback
template <typename E, typename F>
EventConnection ConnectEvent1(wxEvtHandler* src, wxEventType evtId, F fn, S32 winId=wxID_ANY, S32 winIdLast=wxID_ANY)
{
	wxObject* pDelegate(new EventDelegateImplWx<E, F>(fn));

	src->Connect(winId, winIdLast, evtId, wxObjectEventFunction(&EventProxy::OnEvent), pDelegate, &EventProxy::Instance());
	return EventConnection(src, evtId, pDelegate, winId, winIdLast);
}


#endif