#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	//
	// Xt::Change
	//

	Change::Change(ChangeType type_) :
		type(type_)
	{
	}

	Change::~Change()
	{
	}

	Change* Change::Clone()
	{
		return 0;
	}

	//
	// Xt::ChangeRemove
	//

	ChangeRemove::ChangeRemove() :
		Change(kChangeRemove)
	{
	}

	ChangeRemove::~ChangeRemove()
	{
	}

	ChangeRemove* ChangeRemove::Clone()
	{
		return new ChangeRemove;
	}

	//
	// Xt::ChangeLeaf
	//

	ChangeLeaf::ChangeLeaf() :
		Change(kChangeLeaf),
		element(0)
	{
	}

	ChangeLeaf::~ChangeLeaf()
	{
		delete element;
	}

	ChangeLeaf* NewChangeLeaf(DomElement* element)
	{
		XtUt::Ptr<ChangeLeaf> newLeaf = new ChangeLeaf;

		if (!newLeaf)
			return 0;

		if ((newLeaf->element = Clone(element)) == 0)
			return 0;

		return Pop(&newLeaf);
	}

	ChangeLeaf* ChangeLeaf::Clone()
	{
		return NewChangeLeaf(element);
	}

	//
	// Xt::ChangeBlock
	//

	ChangeBlock::ChangeBlock() :
		Change(kChangeBlock)
	{
	}

	ChangeBlock::~ChangeBlock()
	{
	}

	ChangeBlock* ChangeBlock::Clone()
	{
		XtUt::Ptr<ChangeBlock> newBlock = new ChangeBlock;

		if (!newBlock)
			return 0;

		for(ChangeBlock::Iterator it(&changes); it; ++it)
		{
			if (!XtUt::Insert(&newBlock->changes, it.Key(), it->Clone()))
				return 0;
		}

		return Pop(&newBlock);
	}

	//
	// Xt::ChangeArray 
	//

	ChangeArray::ChangeArray() :
		Change(kChangeArray),
		index(0)
	{
	}

	ChangeArray::~ChangeArray()
	{
		delete index;
	}

	ChangeArray* ChangeArray::Clone()
	{
		XtUt::Ptr<ChangeArray> newArray = new ChangeArray;

		if (!newArray)
			return 0;

		if (index && ((newArray->index = index->Clone()) == 0))
			return 0;

		for(ChangeArray::Iterator it(&changes); it; ++it)
		{
			if (!XtUt::Insert(&newArray->changes, it.Key(), it->Clone()))
				return 0;
		}

		return Pop(&newArray);
	}

	//
	// Xt::ChangeComponent
	//

	ChangeComponent::ChangeComponent() :
		Change(kChangeComponent),
		id(0),
		type(0),
		data(0)
	{
	}

	ChangeComponent::~ChangeComponent()
	{
		delete id;
		delete type;
		delete data;
	}

	ChangeComponent* ChangeComponent::Clone()
	{
		XtUt::Ptr<ChangeComponent> newComponent = new ChangeComponent;

		if (!newComponent)
			return 0;

		if (id && ((newComponent->id = id->Clone()) == 0))
			return 0;

		if (type && ((newComponent->type = type->Clone()) == 0))
			return 0;

		if (data && ((newComponent->data = data->Clone()) == 0))
			return 0;

		return Pop(&newComponent);
	}

	//
	// Xt::ChangeRecord
	//

	ChangeRecord::ChangeRecord() :
		Change(kChangeRecord),
		name(0),
		root(0)
	{
	}

	ChangeRecord::~ChangeRecord()
	{
		delete name;
		delete root;
	}

	ChangeRecord* ChangeRecord::Clone()
	{
		XtUt::Ptr<ChangeRecord> newRecord = new ChangeRecord;

		if (!newRecord)
			return 0;

		if (name && ((newRecord->name = name->Clone()) == 0))
			return 0;

		if (root && ((newRecord->root = root->Clone()) == 0))
			return 0;

		return Pop(&newRecord);
	}

	//
	// Xt::ChangePack
	//

	ChangePack::ChangePack() :
		Change(kChangePack),
		name(0),
		properties(0)
	{
	}

	ChangePack::~ChangePack()
	{
		delete name;
		delete properties;
	}

	ChangePack* ChangePack::Clone()
	{
		XtUt::Ptr<ChangePack> newPack = new ChangePack;

		if (!newPack)
			return 0;

		if (name && ((newPack->name = name->Clone()) == 0))
			return 0;

		if (properties && ((newPack->properties = properties->Clone()) == 0))
			return 0;

		for(ChangePack::Iterator it(&changes); it; ++it)
		{
			if (!XtUt::Insert(&newPack->changes, it.Key(), it->Clone()))
				return 0;
		}

		return Pop(&newPack);
	}

	//
	// Xt::ChangeId
	//

	ChangeId::ChangeId() :
		Change(kChangeId)
	{
	}

	ChangeId::~ChangeId()
	{
	}

	ChangeId* NewChangeId(XtId value)
	{
		XtUt::Ptr<ChangeId> newId = new ChangeId;

		if (newId)
			newId->value = value;

		return Pop(&newId);
	}

	ChangeId* ChangeId::Clone()
	{
		return NewChangeId(value);
	}

	//
	// Xt::ChangeString
	//

	ChangeString::ChangeString() :
		Change(kChangeString)
	{
	}

	ChangeString::~ChangeString()
	{
	}

	ChangeString* NewChangeString(const XtString& value)
	{
		XtUt::Ptr<ChangeString> newString = new ChangeString;

		if (newString)
			newString->value = value;

		return Pop(&newString);
	}

	ChangeString* ChangeString::Clone()
	{
		return NewChangeString(value);
	}

	//
	// Xt::ChangeIndex
	//

	ChangeIndex::ChangeIndex() :
		Change(kChangeIndex)
	{
	}

	ChangeIndex::~ChangeIndex()
	{
	}

	ChangeIndex* NewChangeIndex(XtUt::Array<XtId>* order)
	{
		XtUt::Ptr<ChangeIndex> newIndex = new ChangeIndex;

		if (newIndex && order && (order->count > 0))
		{
			XtId* id = XtUt::AppendExN(&newIndex->order, order->count);

			if (!id)
				return 0;

			XtUt::MemCpy<XtId>(id, order->elements, newIndex->order.count);
		}

		return Pop(&newIndex);
	}

	ChangeIndex* ChangeIndex::Clone()
	{
		return NewChangeIndex(&order);
	}

	//
	// Xt::ChangeConflict
	//

	ChangeConflict::ChangeConflict() :
		Change(kChangeConflict),
		lhs(0),
		rhs(0)
	{
	}

	ChangeConflict::~ChangeConflict()
	{
		delete lhs;
		delete rhs;
	}

	ChangeConflict* NewChangeConflict(Change* lhs, Change* rhs)
	{
		XtUt::Ptr<ChangeConflict> newConflict = new ChangeConflict;

		if (!newConflict || ((newConflict->lhs = lhs->Clone()) == 0) || ((newConflict->rhs = rhs->Clone()) == 0))
			return 0;

		return Pop(&newConflict);
	}

	ChangeConflict* ChangeConflict::Clone()
	{
		return NewChangeConflict(lhs, rhs);
	}

}