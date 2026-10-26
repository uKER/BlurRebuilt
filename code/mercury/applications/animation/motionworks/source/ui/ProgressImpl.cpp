#include "stdafx.h"
#include "../Progress.h"
#include "../Db.h"
#include <wx/progdlg.h>
#include <wx/thread.h>
#include <wx/event.h>

enum E_EventId
{
	E_EventId_WorkerSetMessage = 201,
	E_EventId_WorkerSetTitle,
	E_EventId_WorkerFinished
};


DECLARE_EVENT_TYPE(wxEVT_WORKER_EVENT, -1)

//DEFINE_EVENT_TYPE(wxEVT_WORKER_EVENT)

class C_EventBlocker: public wxEvtHandler
{
public:
	C_EventBlocker( wxWindow* window, wxEventType eventToBlock ):
		m_window(window)
	{
		m_eventsToBlock.PushBack( (S32) eventToBlock );
		
		if ( m_window )
		{
			m_window->PushEventHandler( this );
		}
	}
	
	~C_EventBlocker()
	{
		m_window->RemoveEventHandler( this );
	}
	
	virtual bool ProcessEvent( wxEvent& e )
	{
		for ( S32 i = 0; i < m_eventsToBlock.Size(); ++i )
		{
			wxEventType t = (wxEventType) m_eventsToBlock[i];
			if ( t == e.GetEventType() )
			{
				return true;
			}
		}
		return false;
	}

private:
	wxWindow* m_window;
	Btl::Vector<S32> m_eventsToBlock;
};

class C_ProgressWindow: public wxProgressDialog, public I_Progress
{
public:
	C_ProgressWindow( wxWindow* parent ):
		wxProgressDialog("Title", "Message", 101, parent, wxPD_SMOOTH | wxPD_AUTO_HIDE | wxPD_APP_MODAL)
	{
		if ( parent )
		{
			// Centre inside parent window.
			wxSize size( 350, 150 );
			wxRect parentRect = parent->GetRect();
			wxRect thisRect = wxRect(size).CentreIn( parentRect );
			SetSize( thisRect );
		}
	}
	
	virtual void SetTitle( const char* title )
	{
		SetLabel(title);
	}
	
	virtual void ErrorMessage( const char* msg )
	{
		::wxMessageBox( msg, _T("Error"), wxICON_ERROR | wxOK, GetParent() );
	}
	
	virtual bool ProgressNotify( int iHierarchyDepth, const F32 percent, const Util::C_String &description )
	{
		Update( static_cast<S32>(percent), description.Chars() );
		return true;
	}
	
	
private:
};

bool RunLongTaskRaw(void* parentWindow, LongTaskCallback callback, void* context)
{
	C_ProgressWindow p( (wxWindow*)parentWindow );
	
	return (*callback)(&p, context);
}
