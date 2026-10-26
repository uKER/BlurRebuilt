#include "stdafx.h"

#include <DialogueMan/App.h>

#include <DialogueMan/Frame/Frame.h>
#include <DialogueMan/Misc/ViewDisabler.h>
#include <DialogueMan/Components/Scarab_form/WxScarabForm.h>

namespace DMan
{

C_ViewDisabler::C_ViewDisabler()
{
	wxGetApp().GetFrame()->GetScarabForm().SetDom((Xt::DomElement*)0,0);
	wxGetApp().EnableViews(false);
}

C_ViewDisabler::~C_ViewDisabler()
{
	wxGetApp().GetFrame()->GetScarabForm().OnSelectionChanged();
	wxGetApp().EnableViews(true);
}

}