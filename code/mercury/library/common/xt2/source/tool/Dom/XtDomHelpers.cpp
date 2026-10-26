#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	//
	// Xt::Indexer
	//

	Indexer* Indexer::Clone()
	{
		return new Indexer(*this);
	}

	DomElement* Indexer::At(DomElement* parent)
	{
		return 0;
	}

	DomField* Indexer::AcquireField(DomElement* parent)
	{
		return 0;
	}

	DomRef* Indexer::AcquireRef(DomElement* parent)
	{
		return 0;
	}

	DomBlock* Indexer::AcquireBlock(DomElement* parent)
	{
		return 0;
	}

	DomArray* Indexer::AcquireArray(DomElement* parent)
	{
		return 0;
	}

	DomComponent* Indexer::AcquireComponent(DomElement* parent)
	{
		return 0;
	}

	DomRecord* Indexer::AcquireRecord(DomElement* parent)
	{
		return 0;
	}

	DomPack* Indexer::AcquirePack(DomElement* parent)
	{
		return 0;
	}

	bool Indexer::Set(DomElement* parent, Meta* meta)
	{
		return false;
	}

	bool Indexer::Remove(DomElement* parent)
	{
		return false;
	}

	DomElement* Xt::Indexer::Acquire(DomElement* parent, DomKind kind)
	{
		switch(kind)
		{
			case kDomField:		return AcquireField(parent);
			case kDomRef:		return AcquireRef(parent);
			case kDomBlock:		return AcquireBlock(parent);
			case kDomArray:		return AcquireArray(parent);
			case kDomComponent:	return AcquireComponent(parent);
			case kDomRecord:	return AcquireRecord(parent);
			case kDomPack:		return AcquirePack(parent);
		}

		return 0;
	}

	//
	// Xt::NullIndexer
	//

	NullIndexer* NullIndexer::Clone()
	{
		return new NullIndexer(*this);
	}

	DomElement* NullIndexer::At(DomElement* parent)
	{
		return parent;
	}

	DomField* NullIndexer::AcquireField(DomElement* parent)
	{
		return AsField(parent);
	}

	DomRef* NullIndexer::AcquireRef(DomElement* parent)
	{
		return AsRef(parent);
	}

	DomBlock* NullIndexer::AcquireBlock(DomElement* parent)
	{
		return AsBlock(parent);
	}

	DomArray* NullIndexer::AcquireArray(DomElement* parent)
	{
		return AsArray(parent);
	}

	DomComponent* NullIndexer::AcquireComponent(DomElement* parent)
	{
		return AsComponent(parent);
	}

	DomRecord* NullIndexer::AcquireRecord(DomElement* parent)
	{
		return AsRecord(parent);
	}

	DomPack* NullIndexer::AcquirePack(DomElement* parent)
	{
		return AsPack(parent);
	}

	//
	// Xt::BlockIndexer
	//

	BlockIndexer* BlockIndexer::Clone()
	{
		return new BlockIndexer(*this);
	}

	DomElement* BlockIndexer::At(DomElement* parent)
	{
		return Xt::At(AsBlock(parent), name);
	}

	DomField* BlockIndexer::AcquireField(DomElement* parent)
	{
		return Xt::AcquireField(AsBlock(parent), name);
	}

	DomRef* BlockIndexer::AcquireRef(DomElement* parent)
	{
		return Xt::AcquireRef(AsBlock(parent), name);
	}

	DomBlock* BlockIndexer::AcquireBlock(DomElement* parent)
	{
		return Xt::AcquireBlock(AsBlock(parent), name);
	}

	DomArray* BlockIndexer::AcquireArray(DomElement* parent)
	{
		return Xt::AcquireArray(AsBlock(parent), name);
	}

	DomComponent* BlockIndexer::AcquireComponent(DomElement* parent)
	{
		return Xt::AcquireComponent(AsBlock(parent), name);
	}

	bool BlockIndexer::Set(DomElement* parent, Meta* meta)
	{
		if (DomBlock* block = AsBlock(parent))
		{
			if (MetaElement* element = meta->AsElement())
			{
				return Xt::Set(block, name, XtUt::Pop(&element->element)) != 0;
			}
		}

		return false;
	}

	bool BlockIndexer::Remove(DomElement* parent)
	{
		if (DomBlock* block = AsBlock(parent))
			return Xt::Remove(block, name);

		return false;
	}

	//
	// Xt::ArrayIndexer
	//

	ArrayIndexer* ArrayIndexer::Clone()
	{
		return new ArrayIndexer(*this);
	}

	DomElement* ArrayIndexer::At(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return FindById(array, id);
		}

		return 0;
	}

	DomField* ArrayIndexer::AcquireField(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return AcquireFieldById(array, id);
		}

		return 0;
	}

	DomRef* ArrayIndexer::AcquireRef(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return AcquireRefById(array, id);
		}

		return 0;
	}

	DomBlock* ArrayIndexer::AcquireBlock(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return AcquireBlockById(array, id);
		}

		return 0;
	}

	DomArray* ArrayIndexer::AcquireArray(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return AcquireArrayById(array, id);
		}

		return 0;
	}

	DomComponent* ArrayIndexer::AcquireComponent(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return AcquireComponentById(array, id);
		}

		return 0;
	}
	
	bool FindIdInOrder(XtId id, const XtUt::Array<XtId>* order)
	{
		for(XtUt::Array<XtId>::ConstIterator it(order); it; ++it)
		{
			if (id == *it)
				return true;
		}

		return false;
	}

	bool ArrayIndexer::Set(DomElement* parent, Meta* meta)
	{
		if (DomArray* array = AsArray(parent))
		{
			if (method == kIndex)
			{
				if (MetaIndex* index = meta->AsIndex())
				{
					XtUt::Array<XtId> newOrder;

					if (!XtUt::Reserve(&newOrder, array->index.count))
						return false;

					for(XtUt::Array<XtId>::ConstIterator it(index->order); it; ++it)
					{
						if (XtUt::Exists(&array->elements, *it))
						{
							if (!XtUt::Append(&newOrder, *it))
								return false;
						}
					}

					for(XtUt::Array<XtId>::ConstIterator it(&array->index); it; ++it)
					{
						if (!FindIdInOrder(*it, index->order)) // TODO: Optimisation - Create reverse mapping
						{
							if (!XtUt::Append(&newOrder, *it))
								return false;
						}
					}

					if (newOrder.count != array->index.count)
						return false;

					XtUt::Replace(&array->index, &newOrder);

					Notify(array, &Xt::DomArrayChanged(array, 0));

					return true;
				}
			}
			else if (method == kElement)
			{
				if (MetaElement* element = meta->AsElement())
					return Xt::SetById(array, id, XtUt::Pop(&element->element)) != 0;
			}
		}

		return false;
	}

	bool ArrayIndexer::Remove(DomElement* parent)
	{
		if (method == kElement)
		{
			if (DomArray* array = AsArray(parent))
				return Xt::RemoveById(array, id);
		}

		return false;
	}

	//
	// Xt::ComponentIndexer
	//

	ComponentIndexer* ComponentIndexer::Clone()
	{
		return new ComponentIndexer(*this);
	}

	DomBlock* ComponentIndexer::AcquireBlock(DomElement* parent)
	{
		if (DomComponent* component = AsComponent(parent))
		{
			if (method == kData)
				return &component->data;
		}

		return 0;
	}

	bool ComponentIndexer::Set(DomElement* parent, Meta* meta)
	{
		if (DomComponent* component = AsComponent(parent))
		{
			if (MetaId* id = meta->AsId())
			{
				switch(method)
				{
					case kId: return SetId(component, id->value);
				}
			}
			else if (MetaString* string = meta->AsString())
			{
				switch(method)
				{
					case kType:	return SetType(component, string->value);
				}
			}
			else if (MetaElement* element = meta->AsElement())
			{
				if (method == kData)
				{
					if (DomBlock* block = AsBlock(element->element))
					{
						// TODO: This is incorrect, the data should be set to the block passed in,
						// not cloned from it.  However until the component has a pointer to its data
						// block, I can't do this, so cloning is how we're going to have to do it for
						// now.

						return Xt::Clone(&component->data, block);
					}
				}
			}
		}

		return false;
	}

	bool ComponentIndexer::Remove(DomElement* parent)
	{
		if (DomComponent* component = AsComponent(parent))
		{
			switch(method)
			{
				case kId:	return SetId(component, 0);
				case kType:	return SetType(component, kXtNullString);
			}
		}

		return false;
	}

	//
	// Xt::RecordIndexer
	//

	RecordIndexer* RecordIndexer::Clone()
	{
		return new RecordIndexer(*this);
	}

	DomComponent* RecordIndexer::AcquireComponent(DomElement* parent)
	{
		if (DomRecord* record = AsRecord(parent))
		{
			if (method == kRoot)
				return &record->root;
		}

		return 0;
	}

	bool RecordIndexer::Set(DomElement* parent, Meta* meta)
	{
		if (DomRecord* record = AsRecord(parent))
		{
			if (MetaString* string = meta->AsString())
			{
				switch(method)
				{
					case kName: return SetName(record, string->value);
				}
			}
			else if (MetaElement* element = meta->AsElement())
			{
				if (method == kRoot)
				{
					if (DomComponent* component = AsComponent(element->element))
					{
						// TODO: This is incorrect, the root should be set to the component passed in,
						// not cloned from it.  However until the record has a pointer to its root
						// component, I can't do this, so cloning is how we're going to have to do it for
						// now.

						return Xt::Clone(&record->root, component);
					}
				}
			}
		}

		return false;
	}

	bool RecordIndexer::Remove(DomElement* parent)
	{
		if (DomRecord* record = AsRecord(parent))
		{
			switch(method)
			{
				case kName: return SetName(record, kXtNullString);
			}
		}

		return false;
	}

	//
	// Xt::PackIndexer
	//

	PackIndexer* PackIndexer::Clone()
	{
		return new PackIndexer(*this);
	}

	DomElement* PackIndexer::At(DomElement* parent)
	{
		if (method == kRecord)
		{
			if (DomPack* pack = AsPack(parent))
				return pack->records[id];
		}

		return 0;
	}

	DomBlock* PackIndexer::AcquireBlock(DomElement* parent)
	{
		if (DomPack* pack = AsPack(parent))
		{
			if (method == kProperties)
				return &pack->properties;
		}

		return 0;
	}

	DomRecord* PackIndexer::AcquireRecord(DomElement* parent)
	{
		if (method == kRecord)
		{
			if (DomPack* pack = AsPack(parent))
			{
				if (DomRecord* record = pack->records[id])
					return record;

				//return NewRecord(pack, kXtNullString, kXtNullString); // TODO...
			}
		}

		return 0;
	}

	bool PackIndexer::Set(DomElement* parent, Meta* meta)
	{
		if (DomPack* pack = AsPack(parent))
		{
			if (method == kName)
			{
				if (MetaString* string = meta->AsString())
				{
					return SetName(pack, string->value);
				}
			}
			else if (method == kProperties)
			{
				if (MetaElement* element = meta->AsElement())
				{
					if (DomBlock* block = AsBlock(element->element))
					{
						// TODO: This is incorrect, the properties should be set to the block passed in,
						// not cloned from it.  However until the pack has a pointer to its properties
						// block, I can't do this, so cloning is how we're going to have to do it for
						// now.

						return Xt::Clone(&pack->properties, block);
					}
				}
			}
			else if (method == kRecord)
			{
				if (MetaElement* element = meta->AsElement())
				{
					XtUt::Ptr<DomElement> el = element->element;

					if (DomRecord* record = AsRecord(el))
					{
						if ((id != 0) && (record->id == id))
						{
							RemoveRecord(pack, id);

							XtUt::Pop(&el);

							return AddRecord(pack, record) != 0;
						}
					}
				}
			}
		}

		return false;
	}

	bool PackIndexer::Remove(DomElement* parent)
	{
		if (DomPack* pack = AsPack(parent))
		{
			switch(method)
			{
				case kRecord:	return RemoveRecord(pack, id);
				case kName:		return SetName(pack, kXtNullString);
			}
		}

		return false;
	}

	//
	// Xt::DomElement::Compare
	//

	int Compare(DomElement* lhs, DomElement* rhs)
	{
		if (lhs->kind != rhs->kind)
			return lhs->kind - rhs->kind;

		switch(lhs->kind)
		{
			case kDomField:
			return strcmp(((DomField*)lhs)->value, ((DomField*)rhs)->value);

			case kDomRef:
			return ((DomRef*)lhs)->value - ((DomRef*)rhs)->value;

			case kDomBlock:
			{
				DomBlock::Iterator itL((DomBlock*)lhs);
				DomBlock::Iterator itR((DomBlock*)rhs);

				while(itL && itR)
				{
					if (itL.Key() < itR.Key())
						return -1;

					if (itL.Key() > itR.Key())
						return 1;

					if (int result = Compare(itL, itR))
						return result;

					++itL;
					++itR;
				}

				if (itL)
					return -1;

				if (itR)
					return 1;
			}
			break;

			case kDomArray:
			{
				DomArray::IndexIterator itL((DomArray*)lhs);
				DomArray::IndexIterator itR((DomArray*)rhs);

				while(itL && itR)
				{
					if (itL.Key() < itR.Key())
						return -1;

					if (itL.Key() > itR.Key())
						return 1;

					if (int result = Compare(itL, itR))
						return result;

					++itL;
					++itR;
				}

				if (itL)
					return -1;

				if (itR)
					return 1;
			}
			break;

			case kDomComponent:
			{
				DomComponent* cl = (DomComponent*)lhs;
				DomComponent* cr = (DomComponent*)rhs;

				if (cl->id != cr->id)
					return cl->id - cr->id;

				if (int result = strcmp(cl->type, cr->type))
					return result;

				if (int result = Compare(&cl->data, &cr->data))
					return result;
			}
			break;

			case kDomRecord:
			{
				DomRecord* rl = (DomRecord*)lhs;
				DomRecord* rr = (DomRecord*)rhs;

				if (rl->id != rr->id)
					return rl->id - rr->id;

				if (int result = strcmp(rl->name, rr->name))
					return result;

				if (int result = Compare(&rl->root, &rr->root))
					return result;
			}
			break;

			default:
				assert(0);
			return (XtIntPtr)lhs - (XtIntPtr)rhs;
		}

		return 0;
	}

	//
	// Xt Dotted Path
	//

	// Xt::SetByDottedPath

	bool SetByDottedPath(DomBlock* block, const char* path, Xt::DomElement* element)
	{
		if (!block || !path || !element)
			return false;

		const char* start	= path;
		const char* end		= path;

		while((*end != '\0') && (*end != '.'))
			end++;

		XtString token(start, end - start);

		if (!XtsIsName(token))
			return false;

		if (*end == '.')
			return SetByDottedPath(AcquireBlock(block, token), end + 1, element);

		return Set(block, token, element) != 0;
	}

	// Xt::SetByDottedPath

	bool SetByDottedPath(DomBlock* block, const char* path, const XtString& value)
	{
		if (!block || !path)
			return false;

		const char* start	= path;
		const char* end		= path;

		while((*end != '\0') && (*end != '.'))
			end++;

		XtString token(start, end - start);

		if (!XtsIsName(token))
			return false;

		if (*end == '.')
			return SetByDottedPath(AcquireBlock(block, token), end + 1, value);

		return Set(block, token, value) != 0;
	}

	// Xt::GetByDottedPath

	DomElement* GetByDottedPath(DomBlock* block, const char* path)
	{
		if (!path)
			return 0;

		for(;;)
		{
			if (!block || !path)
				return 0;

			const char* start	= path;
			const char* end		= path;

			while((*end != '\0') && (*end != '.'))
				end++;

			XtString token(start, end - start);

			if (!XtsIsName(token))
				return false;

			if (*end == '\0')
				return At(block, token);

			block	= AsBlock(At(block, token));
			path	= end + 1;
		}
	}

	//
	// XtLocTable helpers
	//

	// Xt::FindLocEntryById

	DomBlock* FindLocEntryById(DomRecord* locTable, XtId id)
	{
		if (!locTable || !id)
			return 0;

		DomArray* strings = AsArray(At(&locTable->root.data, "strings"));

		if (!strings)
			return 0;

		return AsBlock(At(&strings->elements, id));
	}

	// Xt::FindLocIdByName

	XtId FindLocIdByName(DomRecord* locTable, const XtString& name)
	{
		if (!locTable || !*name)
			return 0;

		DomArray* strings = AsArray(At(&locTable->root.data, "strings"));

		if (!strings)
			return 0;

		for(DomArray::ElementIterator it(strings); it; ++it)
		{
			if (AsText(At(AsBlock(it), "name")) == name)
				return it.Key();
		}

		return 0;
	}

	// Xt::FindLocEntryByName

	DomBlock* FindLocEntryByName(DomRecord* locTable, const XtString& name)
	{
		if (!locTable || !*name)
			return 0;

		DomArray* strings = AsArray(At(&locTable->root.data, "strings"));

		if (!strings)
			return 0;

		for(DomArray::ElementIterator it(strings); it; ++it)
		{
			Xt::DomBlock* block = AsBlock(it);

			if (AsText(At(block, "name")) == name)
				return block;
		}

		return 0;
	}

	// Xt::CreateNewLocEntry

	DomBlock* CreateNewLocEntry(DomRecord* locTable, XtId withExplicitId)
	{
		if (!locTable)
			return 0;

		DomArray* strings = AcquireArray(&locTable->root.data, "strings");

		if (!strings)
			return 0;

		if (withExplicitId == 0)
			withExplicitId = GenerateUniqueArrayIdForLoc(strings);

		return AppendWithId(strings, withExplicitId, new DomBlock);
	}

	// Xt::AcquireLocEntryByName

	DomBlock* AcquireLocEntryByName(DomRecord* locTable, const XtString& name)
	{
		if (!locTable || !*name)
			return 0;

		DomArray* strings = AcquireArray(&locTable->root.data, "strings");

		if (!strings)
			return 0;

		for(DomArray::ElementIterator it(strings); it; ++it)
		{
			Xt::DomBlock* block = AsBlock(it);

			if (AsText(At(block, "name")) == name)
				return block;
		}

		Xt::DomBlock* newBlock = CreateNewLocEntry(locTable);

		if (!newBlock)
			return 0;

		if (!Set(newBlock, "name", name))
			return 0;

		return newBlock;
	}

	// Xt::RemoveLocEntryById

	bool RemoveLocEntryById(DomRecord* locTable, XtId id)
	{
		if (!locTable || !id)
			return false;

		DomArray* strings = AsArray(At(&locTable->root.data, "strings"));

		if (!strings)
			return false;

		for(DomArray::IndexIterator it(strings); it; ++it)
		{
			if (it.Key() == id)
			{
				return Remove(strings, it.index);
			}
		}

		return false;
	}

	//
	// Schema<->Dom helpers
	//

	// TypeOf

	LtStruct* TypeOf(LarvaSchema* schema, DomRecord* record)
	{
		return record ? AsStruct(FindTypeByName(schema, record->root.type)) : 0;
	}

	LtStruct* TypeOf(LarvaSchema* schema, LtStruct* baseType, DomComponent* component)
	{
		if (!schema || !baseType || !component)
			return 0;

		LtStruct* derivedType = AsStruct(FindTypeByName(schema, component->type));

		return IsBaseOf(baseType, derivedType) ? derivedType : 0;
	}

	LarvaType* TypeOf(LarvaSchema* schema, LarvaField* field, DomElement* element)
	{
		if (!schema || !field)
			return 0;

		if (field->kind == kLfPtr)
			return TypeOf(schema, ((LfPtr*)field)->baseType, AsComponent(element));

		return field->Type();
	}

	// At

	DomElement* At(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? At(AsBlock(parent), field->name) : parent;
	}

	// Acquire

	DomField* AcquireField(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? AcquireField(AsBlock(parent), field->name) : AsField(parent);
	}

	DomRef* AcquireRef(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? AcquireRef(AsBlock(parent), field->name) : AsRef(parent);
	}

	DomBlock* AcquireBlock(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? AcquireBlock(AsBlock(parent), field->name) : AsBlock(parent);
	}

	DomArray* AcquireArray(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? AcquireArray(AsBlock(parent), field->name) : AsArray(parent);
	}

	DomComponent* AcquireComponent(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		return *field->name ? AcquireComponent(AsBlock(parent), field->name) : AsComponent(parent);
	}

	// AcquireT

	template<typename DomType, DomType* AsFunc(DomElement*), DomType* AcquireFunc(DomElement*, LarvaField*)>
	DomType* AcquireT(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		switch(method)
		{
			case kIndexSelf:	return AsFunc(parent);
			case kIndexAt:		return AsFunc(At(parent, field));
			case kIndexAcquire:	return AcquireFunc(parent, field);
		}

		return 0;
	}

	DomField* AcquireField(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		return AcquireT<DomField, AsField, AcquireField>(parent, field, method);
	}

	DomRef* AcquireRef(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		return AcquireT<DomRef, AsRef, AcquireRef>(parent, field, method);
	}

	DomBlock* AcquireBlock(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		return AcquireT<DomBlock, AsBlock, AcquireBlock>(parent, field, method);
	}

	DomArray* AcquireArray(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		return AcquireT<DomArray, AsArray, AcquireArray>(parent, field, method);
	}

	DomComponent* AcquireComponent(DomElement* parent, LarvaField* field, IndexMethod method)
	{
		return AcquireT<DomComponent, AsComponent, AcquireComponent>(parent, field, method);
	}

	// Acquire

	namespace
	{
		struct AcquireVisitor : public LvFieldDom
		{
			Xt::DomElement* element;

		public:
			AcquireVisitor() :
				element(0)
			{
			}

		public:
			virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)			{ element = dom; return true; }
			virtual bool Visit(LfInt* field, Xt::DomField* dom)				{ element = dom; return true; }
			virtual bool Visit(LfFloat* field, Xt::DomField* dom)			{ element = dom; return true; }
			virtual bool Visit(LfString* field, Xt::DomField* dom)			{ element = dom; return true; }
			virtual bool Visit(LfWideString* field, Xt::DomField* dom)		{ element = dom; return true; }
			virtual bool Visit(LfBool* field, Xt::DomField* dom)			{ element = dom; return true; }
			virtual bool Visit(LfEnum* field, Xt::DomField* dom)			{ element = dom; return true; }
			virtual bool Visit(LfFlags* field, Xt::DomBlock* dom)			{ element = dom; return true; }
			virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)	{ element = dom; return true; }
			virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)			{ element = dom; return true; }
			virtual bool Visit(LfWeakPtr* field, Xt::DomRef* dom)			{ element = dom; return true; }
			virtual bool Visit(LfHandle* field, Xt::DomField* dom)			{ element = dom; return true; }
			virtual bool Visit(LfArray* field, Xt::DomArray* dom)			{ element = dom; return true; }
			virtual bool Visit(LfLocString* field, Xt::DomBlock* dom)		{ element = dom; return true; }
			virtual bool Visit(LfControl* field, Xt::DomElement* dom)		{ element = dom; return true; }
		};
	}

	DomElement* Acquire(DomElement* parent, LarvaField* field)
	{
		if (!parent || !field)
			return 0;

		AcquireVisitor v;

		field->Accept(&v, parent, Xt::kIndexAcquire);

		return v.element;
	}

}