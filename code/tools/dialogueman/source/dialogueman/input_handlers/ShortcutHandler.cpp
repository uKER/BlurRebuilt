#include "stdafx.h"

#include <DialogueMan/Input_Handlers/ShortcutHandler.h>

#include <DialogueMan/Actions/DeleteRows.h>

#include <DialogueMan/App.h>
#include <DialogueMan/Frame/Frame.h>

namespace DMan
{

C_ShortcutHandler::C_ShortcutHandler()
: wxEvtHandler(), m_acceleratorDisable(false)
{
	C_App *pApp = &wxGetApp();
	//pApp->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(C_ShortcutHandler::OnKeyDown), NULL, this);
	//pApp->Connect(wxEVT_KEY_UP, wxKeyEventHandler(C_ShortcutHandler::OnKeyUp), NULL, this);

}

C_ShortcutHandler::~C_ShortcutHandler()
{
	C_App *pApp = &wxGetApp();
	//pApp->Disconnect(wxEVT_KEY_DOWN, wxKeyEventHandler(C_ShortcutHandler::OnKeyDown), NULL, this);
	//pApp->Disconnect(wxEVT_KEY_UP, wxKeyEventHandler(C_ShortcutHandler::OnKeyUp), NULL, this);

}

void C_ShortcutHandler::OnKeyUp(wxKeyEvent& e)
{
}

void C_ShortcutHandler::OnKeyDown(wxKeyEvent& e)
{
}

void C_ShortcutHandler::OnFocus(wxFocusEvent& e)
{
}

}
