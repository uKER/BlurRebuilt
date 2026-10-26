#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>

namespace Xt
{

	// Xt::DomPack

	DomPack::DomPack() :
		DomElement(kDomPack),
		id(0),
		dirty(false)
	{
		properties.parent = this;
	}

	DomPack::~DomPack()
	{
		Notify(this, &DomRemoved(this));

		Free(&records);
	}

	// Xt::DomPack::PackOf

	DomPack* PackOf(DomElement* element)
	{
		for(;;)
		{
			if (!element)
				return 0;

			if (element->kind == kDomPack)
				return (DomPack*)element;

			element = element->parent;
		}
	}

	// Xt::DomPack::AsPackProperties

	DomBlock* AsPackProperties(DomElement* element)
	{
		return (element && (element->kind == kDomPack)) ? &((DomPack*)element)->properties : 0;
	}

	// Xt::DomPack::SetName

	bool SetName(DomPack* pack, const XtString& name)
	{
		if (!pack)
			return false;

		pack->name = name;

		SetDirty(pack, false);

		Notify(pack, &DomPackChanged(pack));

		return true;
	}

	// Xt::DomPack::GenerateUniqueRecordId

	XtId GenerateUniqueRecordId(DomPack* pack)
	{
		if (!pack)
			return 0;

		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id || Exists(&pack->records, id))
				continue;

			return id;
		}

		return 0;
	}

	XtId GenerateUniqueRecordId(DomDb* db)
	{
		if (!db)
			return 0;

		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id || FindRecordById(db, id))
				continue;

			return id;
		}

		return 0;
	}

	// Xt::DomPack::NewRecord

	DomRecord* NewRecord(DomPack* pack, const XtString& name, const XtString& type)
	{
		if (!pack)
			return 0;

		DomRecord* record = new DomRecord;

		if (!record)
			return 0;

		if (DomDb* db = DbOf(pack))
			record->id = GenerateUniqueRecordId(db);
		else
			record->id = GenerateUniqueRecordId(pack);

		record->name		= name;
		record->root.type	= type;

		return AddRecord(pack, record);
	}

	// Xt::DomPack::AddRecord

	DomRecord* AddRecord(DomPack* pack, DomRecord* record)
	{
		if (!record || (record->parent != 0))
			return 0;

		if (!pack)
		{
			delete record;
			return 0;
		}

		if (
				!record->id ||
				Exists(&pack->records, record->id) ||
				FindRecordById(DbOf(pack), record->id)
			)
		{
			delete record;
			return 0;
		}

		record->dirty = false;

		if (!Insert(&pack->records, record->id, record))
			return 0;

		record->parent = pack;

		Notify(record, &DomEvent(kDomRecordParentChanged, record));

		SetDirty(record, false);

		return record;
	}

	// Xt::DomPack::RemoveRecord

	bool RemoveRecord(DomPack* pack, XtId id)
	{
		DomRecord* record = At(&pack->records, id);

		if (!record)
			return false;

		if (!Remove(&pack->records, FirstOf(&pack->records, id), LastOf(&pack->records, id)))
			return false;

		SetDirty(pack, false);

		return true;
	}

	// Xt::DomPack::DuplicateRecord

	DomRecord* DuplicateRecord(LarvaSchema* schema, DomRecord* record)
	{
		if (!record)
			return 0;

		DomPack* pack = PackOf(record);

		if (!pack)
			return 0;

		DomRecord* newRecord = NewRecord(pack, record->name, record->root.type);

		if (!newRecord)
			return 0;

		if (!Clone(&newRecord->root, &record->root))
		{
			RemoveRecord(pack, newRecord->id);
			return 0;
		}

		if (LtStruct* type = TypeOf(schema, newRecord))
		{
			Xt::IdMap map;

			if (!RegenerateComponentIds(schema, type, &newRecord->root, &map))
			{
				RemoveRecord(pack, newRecord->id);
				return 0;
			}

			MapWeakPtrs(schema, type, &newRecord->root, &map);

			RegenerateUniqueIds(schema, type, &newRecord->root.data);
		}

		return newRecord;
	}

	// Xt::DomPack::MoveRecord

	bool MoveRecord(DomRecord* record, DomPack* dstPack)
	{
		// TODO: This function should just leave the record in the old pack if it runs OOM, rather than freeing the record

		if (!record || !dstPack)
			return false;

		if (DomPack* srcPack = PackOf(record))
		{
			if (srcPack == dstPack)
				return true;

			size_t first	= XtUt::FirstOf(&srcPack->records, record->id);
			size_t last		= XtUt::LastOf(&srcPack->records, record->id);

			if (first == last)
				return false;

			XtUt::Detach(&srcPack->records, first);

			record->parent = 0;

			SetDirty(srcPack, false);
		}

		if (!Insert(&dstPack->records, record->id, record))
			return false;

		record->parent = dstPack;

		Notify(record, &DomEvent(kDomRecordParentChanged, record));

		SetDirty(record, false);

		return true;
	}

	// Xt::DomPack::SetDirty

	void SetDirty(DomPack* pack, bool forceNotify)
	{
		assert(pack != 0);

		if (!pack->dirty || forceNotify)
		{
			pack->dirty = true;

			Notify(pack, &DomPackChanged(pack));
		}

		if (DomDb* db = DbOf(pack))
			SetDirty(db, forceNotify);
	}

}