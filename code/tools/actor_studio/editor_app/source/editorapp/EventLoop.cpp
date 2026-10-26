#include "stdafx.h"

#include "wx/evtloop.h"
#include "wx/ptr_scpd.h"
#include "wx/tooltip.h"
#include "wx/except.h"
#include "wx/ptr_scpd.h"

#if wxUSE_THREADS
    #include "wx/thread.h"

    // define the list of MSG strutures
    WX_DECLARE_LIST(MSG, wxMsgList);

    #include "wx/listimpl.cpp"

    WX_DEFINE_LIST(wxMsgList)
#endif // wxUSE_THREADS

#include <EditorApp/Input_Handlers/ShortcutHandler.h>
#include <EditorApp/App.h>
#include <EditorApp/EventLoop.h>

namespace Editor
{

C_EventLoop::C_EventLoop()
: wxEventLoop()
{
}

C_EventLoop::~C_EventLoop()
{
}


void C_EventLoop::ProcessMessage(WXMSG *msg)
{
    if ( !PreProcessMessage(msg) )
    {
        // if it wasn't done, dispatch it to the corresponding window
        ::TranslateMessage(msg);
		LRESULT r = ::DispatchMessage(msg);
		{
			// If the message wasn't handled elsewhere, and it's a key press message
			// then process it with our shortcut handler
			if (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP)
			{
				//according to msdn, WM_KEY* events should return 0 if handled, yet it seems to be the opposite - more wx problems??
				bool forwardToHandler = r == 0;

				//...but then it behaves as expected from drop down boxes
				if (forwardToHandler)
				{
					char className[256];
					if (::GetClassName(msg->hwnd, className, sizeof(className)))
					{
						char* pChar = className;
						for (S32 i=0; i<lengthof(className); ++i)
							if (className[i]<='Z' && className[i]>='A')
								className[i] += ('a'-'A');

						if (strstr(className, "combobox"))
							forwardToHandler = false;						
					}
				}

				if (forwardToHandler)
					// Call shortcut handler
					wxGetApp().GetShortcutHandler()->ProcessKey(msg);
			}
		}
    }
}

bool C_EventLoop::PreProcessMessage(WXMSG *msg)
{
    HWND hwnd = msg->hwnd;
    wxWindow *wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
    wxWindow *wnd;

    // this might happen if we're in a modeless dialog, or if a wx control has
    // children which themselves were not created by wx (i.e. wxActiveX control children)
    if ( !wndThis )
    {
        while ( hwnd && (::GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD ))
        {
            hwnd = ::GetParent(hwnd);

            // If the control has a wx parent, break and give the parent a chance
            // to process the window message
            wndThis = wxGetWindowFromHWND((WXHWND)hwnd);
            if (wndThis != NULL)
                break;
        }

        if ( !wndThis )
        {
            // this may happen if the event occurred in a standard modeless dialog (the
            // only example of which I know of is the find/replace dialog) - then call
            // IsDialogMessage() to make TAB navigation in it work

            // NOTE: IsDialogMessage() just eats all the messages (i.e. returns true for
            // them) if we call it for the control itself
            return hwnd && ::IsDialogMessage(hwnd, msg) != 0;
        }
    }

    if ( !AllowProcessing(wndThis) )
    {
        // not a child of critical window, so we eat the event but take care to
        // stop an endless stream of WM_PAINTs which would have resulted if we
        // didn't validate the invalidated part of the window
        if ( msg->message == WM_PAINT )
            ::ValidateRect(hwnd, NULL);

        return true;
    }

#if wxUSE_TOOLTIPS
    // we must relay WM_MOUSEMOVE events to the tooltip ctrl if we want it to
    // popup the tooltip bubbles
    if ( msg->message == WM_MOUSEMOVE )
    {
        // we should do it if one of window children has an associated tooltip
        // (and not just if the window has a tooltip itself)
        if ( wndThis->HasToolTips() )
            wxToolTip::RelayEvent((WXMSG *)msg);
    }
#endif // wxUSE_TOOLTIPS

    // allow the window to prevent certain messages from being translated/processed
    if ( !wndThis->MSWShouldPreProcessMessage((WXMSG *)msg) )
    {
        return false;
    }

    // try translations first: the accelerators override everything
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWTranslateMessage((WXMSG *)msg))
            return true;

        // stop at first top level window, i.e. don't try to process the key
        // strokes originating in a dialog using the accelerators of the parent
        // frame - this doesn't make much sense
        if ( wnd->IsTopLevel() )
			break;
    }

    // now try the other hooks (kbd navigation is handled here)
    for ( wnd = wndThis; wnd; wnd = wnd->GetParent() )
    {
        if ( wnd->MSWProcessMessage((WXMSG *)msg) )
            return true;

        // also stop at first top level window here, just as above because
        // if we don't do this, pressing ESC on a modal dialog shown as child
        // of a modal dialog with wxID_CANCEL will cause the parent dialog to
        // be closed, for example
        if ( wnd->IsTopLevel() )
            break;
    }

	// no special preprocessing for this message, dispatch it normally
    return false;
}

// ----------------------------------------------------------------------------
// C_EventLoop message processing dispatching
// ----------------------------------------------------------------------------

bool C_EventLoop::Pending() const
{
    MSG msg;
    return ::PeekMessage(&msg, 0, 0, 0, PM_NOREMOVE) != 0;
}

bool C_EventLoop::Dispatch()
{
    wxCHECK_MSG( IsRunning(), false, _T("can't call Dispatch() if not running") );

    MSG msg;
    BOOL rc = ::GetMessage(&msg, (HWND) NULL, 0, 0);

    if ( rc == 0 )
    {
        // got WM_QUIT
        return false;
    }

    if ( rc == -1 )
    {
        // should never happen, but let's test for it nevertheless
        wxLogLastError(wxT("GetMessage"));

        // still break from the loop
        return false;
    }

#if wxUSE_THREADS
    wxASSERT_MSG( wxThread::IsMain(),
                  wxT("only the main thread can process Windows messages") );

    static bool s_hadGuiLock = true;
    static wxMsgList s_aSavedMessages;

    // if a secondary thread owning the mutex is doing GUI calls, save all
    // messages for later processing - we can't process them right now because
    // it will lead to recursive library calls (and we're not reentrant)
    if ( !wxGuiOwnedByMainThread() )
    {
        s_hadGuiLock = false;

        // leave out WM_COMMAND messages: too dangerous, sometimes
        // the message will be processed twice
        if ( !wxIsWaitingForThread() || msg.message != WM_COMMAND )
        {
            MSG* pMsg = new MSG(msg);
            s_aSavedMessages.Append(pMsg);
        }

        return true;
    }
    else
    {
        // have we just regained the GUI lock? if so, post all of the saved
        // messages
        //
        // FIXME of course, it's not _exactly_ the same as processing the
        //       messages normally - expect some things to break...
        if ( !s_hadGuiLock )
        {
            s_hadGuiLock = true;

            wxMsgList::compatibility_iterator node = s_aSavedMessages.GetFirst();
            while (node)
            {
                MSG* pMsg = node->GetData();
                s_aSavedMessages.Erase(node);

                ProcessMessage(pMsg);
                delete pMsg;

                node = s_aSavedMessages.GetFirst();
            }
        }
    }
#endif // wxUSE_THREADS

    ProcessMessage(&msg);

    return true;
}

}