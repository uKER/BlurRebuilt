#include "Core/StdAfx.h"
#include "SorceryView.h"

void NetUpdateHack(SorceryApp* app, Xt::DomRecord* record);

SorceryDocument::SorceryDocument(SorceryApp& app) :
	_link(this),
	_app(app),
	_kind(kSdkNone),
	_id(0),
	_dom(DomCallback, this)
{
}

SorceryDocument::~SorceryDocument()
{
	for(LinkIterator<SorceryView> it(_views); it; )
	{
		SorceryView* view = it;

		++it;

		view->Destroy(false);
	}
}

void SorceryDocument::SetModified()
{
	OnDetailsUpdated();
}

void SorceryDocument::OnDetailsUpdated()
{
	for(LinkIterator<SorceryView> it(_views); it; ++it)
		it->OnDetailsUpdated();
}

void SorceryDocument::DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SorceryDocument* doc)
{
	if (*monitor == event->from)
		doc->OnDetailsUpdated();
	else
	{
		if (doc->_kind == kSdkRecord)
		{
			bool doUpdate = true;

			if (event->from && event->from->parent && (event->from->parent->kind == Xt::kDomBlock))
			{
				if (*event->from->parentKey == '_')
					doUpdate = false;
			}

			if (doUpdate)
			{
				if (Xt::DomRecord* record = AsRecord(doc->_dom))
					NetUpdateHack(&doc->_app, record);
			}
		}
	}
}