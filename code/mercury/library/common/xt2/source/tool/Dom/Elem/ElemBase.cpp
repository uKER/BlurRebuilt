#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

namespace Xt
{
	
	// Xt::DomElement

	DomElement::DomElement(DomKind kind_) :
		kind(kind_),
		parent(0),
		parentId(0)
	{
	}

	DomElement::~DomElement()
	{
		while(DomMonitor* monitor = PopBack(&monitors))
			monitor->element = 0;
	}

	// Xt::DomElement::Notify

	void DoNotify(DomElement* element, DomEvent* event, XtUt::LinkList<DomMonitor>::Iterator& it)
	{
		if (!it)
			return;

		XtUt::WeakRef<DomMonitor> monitor = it->self;

		++it;

		DoNotify(element, event, it);

		if (monitor && monitor->callback)
			monitor->callback(monitor, event, monitor->context);
	}

	void Notify(DomElement* element, DomEvent* event)
	{
		for(; element != 0; element = element->parent)
		{
			DoNotify(element, event, XtUt::LinkList<DomMonitor>::Iterator(&element->monitors)); // TODO: Really need GetFirst/GetNext functions

			// Don't bubble up removed messages to prevent querying of dom while in inconsistent state

			if (event->code == kDomRemoved)
				break;

			// Manage dirty flags for record, pack and db

			if ((element->kind == kDomRecord) || (element->kind == kDomPack) || (element->kind == kDomDb))
			{
				switch(event->code)
				{
					case kDomFieldChanged:
					case kDomRefChanged:
					case kDomArrayChanged:
					case kDomComponentChanged:
						SetDirty(element, false);
					break;

					case kDomBlockChanged:
					{
						DomBlockChanged* bc = (DomBlockChanged*)event;

						// TODO: May need distinction between dirtyForUndo and dirtyForSave

						bool setDirty = !bc->child;

						if (!setDirty)
						{
							if (bc->child->kind == kDomField)
								setDirty = *((DomField*)bc->child)->value != '\0';
							else if (bc->child->kind == kDomRef)
								setDirty = ((DomRef*)bc->child)->value != 0;
						}

						if (setDirty)
							SetDirty(element, false);
					}
					break;
				}
			}
		}
	}

	// Xt::DomElement::SetDirty

	void SetDirty(DomElement* element, bool forceNotify)
	{
		for(; element; element = element->parent)
		{
			switch(element->kind)
			{
				case kDomRecord:	SetDirty((DomRecord*)element, forceNotify);	return;
				case kDomPack:		SetDirty((DomPack*)element, forceNotify);	return;
				case kDomDb:		SetDirty((DomDb*)element, forceNotify);		return;
			}
		}
	}

}