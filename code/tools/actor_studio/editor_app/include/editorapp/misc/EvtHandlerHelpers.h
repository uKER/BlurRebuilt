#ifndef EDITOR_EVT_HANDLER_HELPERS_H
#define EDITOR_EVT_HANDLER_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Editor
{
	namespace EvtHandlerHelpers
	{
		void PushBack(wxWindow* pSrc, wxEvtHandler *pHandler);
		wxEvtHandler* PopBack(wxWindow* pSrc);

	}

}

#endif