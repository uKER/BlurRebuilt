#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

namespace Xt
{

	// Xt::DomArray

	DomArray::DomArray() :
		DomElement(kDomArray)
	{
	}

	DomArray::~DomArray()
	{
		Notify(this, &DomRemoved(this));

		Free(&elements);
		Free(&index);
	}

	// Xt::DomArray::GenerateUniqueArrayId

	XtId GenerateUniqueArrayId(DomArray* array)
	{
		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id)
				continue;

			if (array && array->elements[id])
				continue;

			return id;
		}

		return 0;
	}

	// Xt::DomArray::GenerateUniqueArrayIdForLoc

	XtId GenerateUniqueArrayIdForLoc(DomArray* array)
	{
		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id || (id < 32768))
				continue;

			if (array && array->elements[id])
				continue;

			return id;
		}

		return 0;
	}

	// Xt::DomArray::AppendWithId

	DomElement* AppendWithId(DomArray* array, XtId id, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || !CanCompose(element))
		{
			delete element;
			return 0;
		}

		if (!id || Exists(&array->elements, id))
		{
			delete element;
			return 0;
		}

		if (!XtUt::Insert(&array->elements, id, static_cast<DomElement*>(element)))
			return 0;

		if (!AppendEx(&array->index, id))
		{
			XtUt::Remove(&array->elements, FirstOf(&array->elements, id), LastOf(&array->elements, id));
			return 0;
		}

		element->parent		= array;
		element->parentId	= id;

		Notify(array, &DomArrayChanged(array, id));

		return element;
	}

	// Xt::DomArray::Append

	DomElement* Append(DomArray* array, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || !CanCompose(element))
		{
			delete element;
			return 0;
		}

		return AppendWithId(array, GenerateUniqueArrayId(array), element);
	}

	// Xt::DomArray::Insert

	DomElement* Insert(DomArray* array, size_t before, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || !CanCompose(element))
		{
			delete element;
			return 0;
		}

		XtId id = GenerateUniqueArrayId(array);

		if (!id)
		{
			delete element;
			return 0;
		}

		if (!XtUt::Insert(&array->elements, id, static_cast<DomElement*>(element)))
			return 0;

		if (!InsertEx(&array->index, before, id))
		{
			XtUt::Remove(&array->elements, FirstOf(&array->elements, id), LastOf(&array->elements, id));
			return 0;
		}

		element->parent		= array;
		element->parentId	= id;

		Notify(array, &DomArrayChanged(array, id));

		return element;
	}

	// Xt::DomArray::ReplaceById

	DomElement* ReplaceById(DomArray* array, XtId id, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || !id || !CanCompose(element))
		{
			delete element;
			return 0;
		}

		size_t first	= FirstOf(&array->elements, id);
		size_t last		= LastOf(&array->elements, id);

		if (first == last)
		{
			delete element;
			return 0;
		}

		if (!Replace(&array->elements, first, static_cast<DomElement*>(element)))
			return 0;

		XtUt::Remove(&array->elements, first + 1, last);

		element->parent		= array;
		element->parentId	= id;

		Notify(array, &DomArrayChanged(array, id));

		return element;
	}

	// Xt::DomArray::Replace

	DomElement* Replace(DomArray* array, size_t index, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || (index >= array->index.count))
		{
			delete element;
			return 0;
		}

		return ReplaceById(array, array->index[index], element);
	}

	// Xt::DomArray::SetById

	DomElement* SetById(DomArray* array, XtId id, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!array || !id || !CanCompose(element))
		{
			delete element;
			return 0;
		}

		size_t first	= FirstOf(&array->elements, id);
		size_t last		= LastOf(&array->elements, id);

		if (first == last)
		{
			if (!XtUt::Insert(&array->elements, id, static_cast<DomElement*>(element)))
				return 0;

			if (!AppendEx(&array->index, id))
			{
				XtUt::Remove(&array->elements, FirstOf(&array->elements, id), LastOf(&array->elements, id));
				return 0;
			}
		}
		else
		{
			if (!Replace(&array->elements, first, static_cast<DomElement*>(element)))
				return 0;

			XtUt::Remove(&array->elements, first + 1, last);
		}

		element->parent		= array;
		element->parentId	= id;

		Notify(array, &DomArrayChanged(array, id));

		return element;
	}

	// Xt::DomArray::RemoveById

	bool RemoveById(DomArray* array, XtId id)
	{
		int index = IndexOf(array, id);

		if (index < 0)
			return false;

		return Remove(array, index);
	}

	// Xt::DomArray::Remove

	bool Remove(DomArray* array, size_t index)
	{
		if (!array)
			return false;

		if (index >= array->index.count)
			return false;

		XtId id = array->index[index];

		Remove(&array->elements, FirstOf(&array->elements, id), LastOf(&array->elements, id));
		Remove(&array->index, index);

		Notify(array, &DomArrayChanged(array, id));

		return true;
	}

	// Xt::DomArray::Clear

	void Clear(DomArray* array)
	{
		if (!array)
			return;

		Free(&array->elements);
		Free(&array->index);

		Notify(array, &DomArrayChanged(array, 0));
	}

	// Xt::DomArray::Swap

	bool Swap(DomArray* array, size_t i, size_t j)
	{
		if (!array || (i >= array->index.count) || (j >= array->index.count))
			return false;

		XtId idI = array->index[i];
		XtId idJ = array->index[j];

		array->index[i] = idJ;
		array->index[j] = idI;

		Notify(array, &DomArrayChanged(array, idI));
		Notify(array, &DomArrayChanged(array, idJ));

		return true;
	}

	// Xt::DomArray::AcquireT

	template<typename DomType, DomType* AsFunc(DomElement*)>
	DomType* AcquireT(DomArray* array, int index)
	{
		if (!array)
			return 0;

		if (DomType* element = AsFunc(At(array, index)))
			return element;

		return Replace(array, index, new DomType);
	}

	// Xt::DomArray::Acquire*

	DomField*		AcquireField(DomArray* array, int index)		{ return AcquireT<DomField, AsField>(array, index);			}
	DomRef*			AcquireRef(DomArray* array, int index)			{ return AcquireT<DomRef, AsRef>(array, index);				}
	DomBlock*		AcquireBlock(DomArray* array, int index)		{ return AcquireT<DomBlock, AsBlock>(array, index);			}
	DomArray*		AcquireArray(DomArray* array, int index)		{ return AcquireT<DomArray, AsArray>(array, index);			}
	DomComponent*	AcquireComponent(DomArray* array, int index)	{ return AcquireT<DomComponent, AsComponent>(array, index);	}

	// Xt::DomArray::AcquireByIdT

	template<typename DomType, DomType* AsFunc(DomElement*)>
	DomType* AcquireByIdT(DomArray* array, XtId id)
	{
		if (!array || !id)
			return 0;

		DomElement* element = FindById(array, id);

		if (!element)
			return AppendWithId(array, id, new DomType);

		if (DomType* dt = AsFunc(element))
			return dt;

		return ReplaceById(array, id, new DomType);
	}

	// Xt::DomArray::AcquireById*

	DomField*		AcquireFieldById(DomArray* array, XtId id)		{ return AcquireByIdT<DomField, AsField>(array, id);			}
	DomRef*			AcquireRefById(DomArray* array, XtId id)		{ return AcquireByIdT<DomRef, AsRef>(array, id);				}
	DomBlock*		AcquireBlockById(DomArray* array, XtId id)		{ return AcquireByIdT<DomBlock, AsBlock>(array, id);			}
	DomArray*		AcquireArrayById(DomArray* array, XtId id)		{ return AcquireByIdT<DomArray, AsArray>(array, id);			}
	DomComponent*	AcquireComponentById(DomArray* array, XtId id)	{ return AcquireByIdT<DomComponent, AsComponent>(array, id);	}

	// Xt::DomArray::IndexOf

	int IndexOf(DomArray* array, XtId id)
	{
		if (!array || !id)
			return -1;

		for(XtUt::Array<XtId>::Iterator it(&array->index); it; ++it)
		{
			if (*it == id)
				return it.index;
		}

		return -1;
	}

}