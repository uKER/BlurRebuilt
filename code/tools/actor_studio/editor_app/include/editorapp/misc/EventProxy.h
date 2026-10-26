#ifndef EDITOR_UI_WXEVENT_PROXY
#define EDITOR_UI_WXEVENT_PROXY

#ifdef _MSC_VER
#pragma once
#endif

//Proxy classes so that wxEvents can be delegated to objects not derived from wxEventHandler
//loosely based on the discussion at http://wxforum.shadonet.com/viewtopic.php?t=16766
//- hooked in to use Ut::Delegate
//- added ability to hook up parameterless functions
//- wxConnect functions return a wxEventConnection which can subsequently be used to disconnect the handler.

// helper macro to define a structure mapping the event argument type and the appropriate wxEventType
#define DECLARE_WXEVENT_STRUCT( event, args ) \
        struct wxCONCAT( wxEVENT_, event ) \
        { static wxEventType EventType() { return wxCONCAT( wxEVT_, event ); } \
          typedef args EventArgs; \
        };

#include <EditorApp/Misc/EventProxyDecl.h>

// trait class that groups the behaviour of one event
template< typename Event >
struct wxEventTraits
{
        static wxEventType EventType() { return Event::EventType(); }
        typedef typename Event::EventArgs EventArgs;
};

// polymorphic type that can process wxWidgets events
struct wxEventDelegate : public wxObject
{
	virtual ~wxEventDelegate() {}
	virtual void ProcessEvent(wxEvent& e) = 0;
};

// derivation for wxEvent recieving member functions
template< typename Event, typename Sink >
struct wxEventDelegate_MemberRecieve : public wxEventDelegate
{
	typedef wxEventTraits< Event > Traits;
	typedef typename Traits::EventArgs Args;
	typedef void (Sink::*Handler)(Args&);

	wxEventDelegate_MemberRecieve(Sink* sink, Handler handler)
		: m_delegate(sink, handler)
	{
	}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_delegate(static_cast<Args&>(e));
	}

	Ut::Delegate<void(Args&)> m_delegate;
};

// derivation for wxEvent recieving static functions
template< typename Event, typename Handler >
struct wxEventDelegate_StaticRecieve : public wxEventDelegate
{
	typedef wxEventTraits< Event > Traits;
	typedef typename Traits::EventArgs Args;

	wxEventDelegate_StaticRecieve(Handler handler)
		: m_delegate(handler)
	{
	}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_delegate(static_cast<Args&>(e));
	}

	Ut::Delegate<void(Args&)> m_delegate;
};

// derivation for member functions ignoring wxEvent object parameter
template< typename Sink >
struct wxEventDelegate_Member : public wxEventDelegate
{
	typedef void (Sink::*Handler)();

	wxEventDelegate_Member(Sink* sink, Handler handler, bool skipEvent)
		: m_delegate(sink, handler)
		, m_skipEvent(skipEvent)
	{
	}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_delegate();
		if (m_skipEvent) e.Skip();
	}

	Ut::Delegate<void()>	m_delegate;
	bool					m_skipEvent;
};

// derivation for static functions ignoring wxEvent object parameter
template< typename Handler >
struct wxEventDelegate_Static : public wxEventDelegate
{
	wxEventDelegate_Static(Handler handler, bool skipEvent)
		: m_delegate(handler)
		, m_skipEvent(skipEvent)
	{
	}

	virtual void ProcessEvent(wxEvent& e)
	{
		m_delegate();
		if (m_skipEvent) e.Skip();
	}

	Ut::Delegate<void()>	m_delegate;
	bool					m_skipEvent;
};


struct wxEventProxy : public wxEvtHandler
{
	static wxEventProxy& Get()
	{
		static wxEventProxy instance;
		return instance;
	}

	void OnEvent(wxEvent& e)
	{
		wxEventDelegate& delegate = static_cast<wxEventDelegate&>(*e.m_callbackUserData);
		delegate.ProcessEvent(e);
	}
};


struct wxEventConnection
{
    void Disconnect() const
    {
		m_source->Disconnect(m_firstId, m_lastId, wxObjectEventFunction(&wxEventProxy::OnEvent), m_delegate, &wxEventProxy::Get());
    }

	wxEvtHandler*		m_source;
	wxEventType			m_event;
	wxEventDelegate*	m_delegate;
	int					m_firstId;
	int					m_lastId;
};


// form that connects a method of source (accepting parameter Event&) to an event of source
template< typename Event, typename Source >
wxEventConnection wxConnect(Source* source, void (Source::*handler)(typename wxEventTraits<Event>::EventArgs&), int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_MemberRecieve<Event,Source>(source, handler) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get());

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;
}

// form that connects a method of source (without parameters) to an event of source
template< typename Event, typename Source >
wxEventConnection wxConnect(Source* source, void (Source::*handler)(), bool skipEvent, int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_Member<Event,Source>(source, handler, skipEvent) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get());

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;
}

// form that connects a method (accepting parameter Event&) of sink to an event of source
template< typename Event, typename Sink >
wxEventConnection wxConnect(wxEvtHandler* source, void (Sink::*handler)(typename wxEventTraits<Event>::EventArgs&), Sink* sink, int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_MemberRecieve<Event,Sink>(sink, handler) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get());

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;

}

// form that connects a method (without parameters) of sink to an event of source
template< typename Event, typename Sink >
wxEventConnection wxConnect(wxEvtHandler* source, void (Sink::*handler)(), Sink* sink,  bool skipEvent, int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_Member<Sink>(sink, handler, skipEvent) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get());

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;

}

// form that connects a static function (accepting parameter Event&) to an event of source
template< typename Event, typename Handler >
wxEventConnection wxConnect(wxEvtHandler* source, Handler handler, int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_StaticRecieve<Event, Handler>(handler) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get()); 

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;

}

// form that connects a static function (without parameters) to an event of source
template< typename Event, typename Handler >
wxEventConnection wxConnect(wxEvtHandler* source, Handler handler, bool skipEvent, int firstId=wxID_ANY, int lastId=wxID_ANY)
{
	typedef wxEventTraits<Event> Traits;
	wxEventDelegate* delegate( new wxEventDelegate_Static<Handler>(handler, skipEvent) );
	source->Connect(firstId, lastId, Traits::EventType(), wxObjectEventFunction(&wxEventProxy::OnEvent), delegate, &wxEventProxy::Get()); 

	wxEventConnection connection = {source, Traits::EventType(), delegate, firstId, lastId};
	return connection;

}

#endif