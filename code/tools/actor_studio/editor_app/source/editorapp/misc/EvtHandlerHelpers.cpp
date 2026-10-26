#include "stdafx.h"
#include "EditorApp/misc/EvtHandlerHelpers.h"

namespace Editor
{
namespace EvtHandlerHelpers
{

void PushBack(wxWindow* pSrc, wxEvtHandler *pHandler)
{
	assert(pSrc && "Null event source");
	assert(pHandler && "Null event handler");
	assert(pHandler->GetPreviousHandler() == 0 && "Handler is already part of a chain");
	assert(pHandler->GetNextHandler() == 0 && "Handler is already part of a chain");

	wxEvtHandler *pHandler0 = pSrc->GetEventHandler();

	if (pHandler0)
	{
		while (pHandler0->GetNextHandler())
			pHandler0 = pHandler0->GetNextHandler();

		pHandler0->SetNextHandler(pHandler);
		pHandler->SetPreviousHandler(pHandler0);
	}
	else
	{
		pSrc->SetEventHandler(pHandler);
	}
}



wxEvtHandler* PopBack(wxWindow* pSrc)
{
	assert(pSrc && "Null event source");

	wxEvtHandler *pHandler0 = pSrc->GetEventHandler();
	if (pHandler0)
	{
		if (pHandler0->GetNextHandler())
		{
			while (pHandler0->GetNextHandler())
				pHandler0 = pHandler0->GetNextHandler();

			wxEvtHandler *pPrevious = pHandler0->GetPreviousHandler();
			pPrevious->SetNextHandler(0);

			return pHandler0;
		}
		else
		{
			pSrc->SetEventHandler(0);
			return pHandler0;
		}
	}
	else
	{
		return 0;
	}
}

}
}