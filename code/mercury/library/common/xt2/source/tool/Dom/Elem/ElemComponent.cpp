#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

namespace Xt
{

	// Xt::DomComponent

	DomComponent::DomComponent() :
		DomElement(kDomComponent),
		id(0)
	{
		data.parent = this;
	}

	DomComponent::~DomComponent()
	{
		Notify(this, &DomRemoved(this));
	}

	// Xt::DomComponent::ComponentOf

	DomComponent* ComponentOf(DomElement* element)
	{
		for(;;)
		{
			if (!element)
				return 0;

			if (element->kind == kDomComponent)
				return (DomComponent*)element;

			element = element->parent;
		}
	}

	// Xt::DomComponent::ComponentDataOf

	DomBlock* AsComponentData(DomElement* element)
	{
		return (element && (element->kind == kDomComponent)) ? &((DomComponent*)element)->data : 0;
	}

	// Xt::DomComponent::GenerateUniqueComponentId

	XtId GenerateUniqueComponentId()
	{
		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		// TODO: Eventually these should really be unique, rather than just hopefully

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id)
				continue;

			return id;
		}

		return 0;
	}

	// Xt::DomComponent::SetId

	bool SetId(DomComponent* component, XtId id)
	{
		if (!component)
			return false;

		component->id = id;

		Notify(component, &DomComponentChanged(component));

		return true;
	}

	// Xt::DomComponent::SetType

	bool SetType(DomComponent* component, const XtString& type)
	{
		if (!component)
			return false;

		component->type = type;

		Notify(component, &DomComponentChanged(component));

		return true;
	}

}