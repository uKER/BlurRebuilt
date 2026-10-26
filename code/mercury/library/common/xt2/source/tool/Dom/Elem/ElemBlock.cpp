#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

namespace Xt
{

	// Xt::DomBlock

	DomBlock::DomBlock() :
		DomElement(kDomBlock)
	{
	}

	DomBlock::~DomBlock()
	{
		Notify(this, &DomRemoved(this));

		Free(&elements);
	}

	// Xt::DomBlock::Insert

	DomElement* Insert(DomBlock* block, const XtString& key, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!block || !CanCompose(element) || !*key)
		{
			delete element;
			return 0;
		}

		if (Exists(&block->elements, key))
		{
			delete element;
			return 0;
		}

		if (!Insert(&block->elements, key, static_cast<DomElement*>(element)))
			return 0;

		element->parent		= block;
		element->parentKey	= key;

		Notify(block, &DomBlockChanged(block, element, key));

		return element;
	}

	// Xt::DomBlock::Replace

	DomElement* Replace(DomBlock* block, const XtString& key, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!block || !CanCompose(element) || !*key)
		{
			delete element;
			return 0;
		}

		size_t index = FirstOf(&block->elements, key);

		if (index == LastOf(&block->elements, key))
		{
			delete element;
			return 0;
		}

		if (!Replace(&block->elements, index, element))
			return 0;

		element->parent		= block;
		element->parentKey	= key;

		Notify(block, &DomBlockChanged(block, element, key));

		return element;
	}

	// Xt::DomBlock::Set

	DomElement* Set(DomBlock* block, const XtString& key, DomElement* element)
	{
		if (!element || (element->parent != 0))
			return 0;

		if (!block || !CanCompose(element) || !*key)
		{
			delete element;
			return 0;
		}

		size_t index = FirstOf(&block->elements, key);

		if (index == LastOf(&block->elements, key))
		{
			if (!Insert(&block->elements, key, static_cast<DomElement*>(element)))
				return 0;
		}
		else if (!Replace(&block->elements, index, element))
			return 0;

		element->parent		= block;
		element->parentKey	= key;

		Notify(block, &DomBlockChanged(block, element, key));

		return element;
	}

	// Xt::DomBlock::Remove

	bool Remove(DomBlock* block, const XtString& key)
	{
		if (!block)
			return false;

		size_t first	= FirstOf(&block->elements, key);
		size_t last		= LastOf(&block->elements, key);

		if (first != last)
		{
			if (!Remove(&block->elements, first, last))
				return false;

			Notify(block, &DomBlockChanged(block, 0, key));
		}

		return true;
	}

	// Xt::DomBlock::Detach

	/*DomElement* Detach(DomBlock* block, const XtString& key)
	{
		if (!block)
			return 0;

		size_t first	= FirstOf(&block->elements, key);
		size_t last		= LastOf(&block->elements, key);

		if (first == last)
			return 0;

		DomElement* element = XtUt::Detach(&block->elements, first);

		element->parent = 0;

		Notify(block, &DomBlockChanged(block, 0, key));

		return element;
	}*/

	// Xt::DomBlock::Clear

	void Clear(DomBlock* block)
	{
		if (!block)
			return;

		Free(&block->elements);

		Notify(block, &DomBlockChanged(block, 0, kXtNullString));
	}

	// Xt::DomBlock::AcquireT

	template<typename DomType, DomType* AsFunc(DomElement*)>
	DomType* AcquireT(DomBlock* block, const XtString& key)
	{
		if (!block)
			return 0;

		if (DomType* element = AsFunc(At(block, key)))
			return element;

		return Set(block, key, new DomType);
	}

	// Xt::DomBlock::Acquire*

	DomField*		AcquireField(DomBlock* block, const XtString& key)		{ return AcquireT<DomField, AsField>(block, key);			}
	DomRef*			AcquireRef(DomBlock* block, const XtString& key)		{ return AcquireT<DomRef, AsRef>(block, key);				}
	DomBlock*		AcquireBlock(DomBlock* block, const XtString& key)		{ return AcquireT<DomBlock, AsBlock>(block, key);			}
	DomArray*		AcquireArray(DomBlock* block, const XtString& key)		{ return AcquireT<DomArray, AsArray>(block, key);			}
	DomComponent*	AcquireComponent(DomBlock* block, const XtString& key)	{ return AcquireT<DomComponent, AsComponent>(block, key);	}

	// Xt::DomBlock::Set

	DomField* Set(DomBlock* block, const XtString& key, const XtString& value)
	{
		if (DomField* field = AcquireField(block, key))
			return Set(field, value) ? field : 0;

		return 0;
	}

	DomField* Set(DomBlock* block, const XtString& key, const char* value)
	{
		if (DomField* field = AcquireField(block, key))
			return Set(field, value) ? field : 0;

		return 0;
	}

	DomRef* Set(DomBlock* block, const XtString& key, XtId value)
	{
		if (DomRef* ref = AcquireRef(block, key))
			return Set(ref, value) ? ref : 0;

		return 0;
	}

}