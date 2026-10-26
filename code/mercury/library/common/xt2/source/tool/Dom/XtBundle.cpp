#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	BundleItem* Append(Bundle* bundle, BundleItem* item)
	{
		if (!bundle || !item)
		{
			delete item;
			return 0;
		}

		return AppendEx(&bundle->elements, item);
	}

	BundleItem* Append(Bundle* bundle, LarvaField* field, DomElement* element)
	{
		if (!bundle || !field || !element)
			return 0;

		XtUt::Ptr<BundleItem> item = new BundleItem;

		if (!item)
			return 0;

		item->type	= field->container->name;
		item->field	= field->name;

		if ((item->element = Clone(element)) == 0)
			return 0;

		return Append(bundle, Pop(&item));
	}

	void Clear(Bundle* bundle)
	{
		if (bundle)
			Free(&bundle->elements);
	}

}