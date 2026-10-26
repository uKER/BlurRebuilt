#include "Common/XtPch.h"
#include <Xt/XtTool.h>
#include <Xt/Impl/CryptRandom.h>
#include "../../Export/XtFlaskHelpers.h"

namespace Xt
{

	// Xt::DomDb

	DomDb::DomDb() :
		DomElement(kDomDb),
		open(false),
		schema(0),
		exportSchema(0),
		dirty(false)
	{
		properties.parent = this;
	}

	DomDb::~DomDb()
	{
		Free(this);
	}

	// Xt::DomDb::Free

	void Free(DomDb* db)
	{
		if (!db)
			return;

		Notify(db, &DomDbClosing(db));

		Free(&db->packs);
		Free(&db->pristine);

		delete db->schema;
		delete db->exportSchema;

		Free(&db->languages);

		Free(&db->build.platforms);
		Free(&db->build.staging);
		Free(&db->build.deployment);

		Clear(&db->properties);

		Free(&db->removedPacks);

		db->lock.Close();

		DeleteFile(db->path + ".db-lock");

		db->open				= false;
		db->path				= "";
		db->name				= "";
		db->exportSchema		= 0;
		db->schema				= 0;
		db->dirty				= false;

		db->build.structure.packs			= "";
		db->build.structure.localisation	= "";
	}

	// Xt::DomDb::Close

	void Close(DomDb* db, Storage* storage, StatusCallback* status)
	{
		if (!db)
			return;

		if (db->open)
		{
			Flush(db, storage, status);
			Free(db);
		}
	}

	// Xt::DomDb::DbOf

	DomDb* DbOf(DomElement* element)
	{
		for(;;)
		{
			if (!element)
				return 0;

			if (element->kind == kDomDb)
				return (DomDb*)element;

			element = element->parent;
		}
	}

	// Xt::DomDb::DbPropertiesOf

	DomBlock* DbPropertiesOf(DomElement* element)
	{
		return (element && (element->kind == kDomDb)) ? &((DomDb*)element)->properties : 0;
	}

	// Xt::DomDb::SetDirty

	void SetDirty(DomDb* db, bool forceNotify)
	{
		assert(db != 0);

		if (!db->dirty || forceNotify)
		{
			db->dirty = true;

			Notify(db, &DomDbChanged(db));
		}
	}

	// Xt::DomDb::NewPack

	DomPack* NewPack(DomDb* db, const XtString& name)
	{
		assert(db != 0);

		CryptRandom cr;

		if (!cr.Initialise())
			return 0;

		for(int i = 0; i < 32; i++)
		{
			XtId id = (XtId)cr();

			if (!id || FindPackById(db, id))
				continue;

			DomPack* pack = new DomPack;

			if (!pack)
				return 0;

			pack->id	= id;
			pack->name	= name;

			if (!AddPack(db, pack))
				return 0;

			SetDirty(pack, false);

			return pack;
		}

		return 0;
	}

	// Xt::DomDb::AddPack

	bool AddPack(DomDb* db, DomPack* pack)
	{
		if (!db || !pack || !pack->id)
		{
			delete pack;
			return false;
		}

		if (XtUt::Exists(&db->packs, pack->id))
		{
			delete pack;
			return false;
		}

		if (!Insert(&db->packs, pack->id, pack))
			return 0;

		pack->parent = db;

		Notify(pack, &DomEvent(kDomPackParentChanged, pack));

		return true;
	}

	// Xt::DomDb::RemovePack

	bool RemovePack(DomDb* db, XtId id)
	{
		DomPack* pack = db->packs[id];

		if (!pack)
			return false;

		// TODO: Delete records individually for notifications?

		XtString filename = pack->filename;

		if (!Remove(&db->packs, FirstOf(&db->packs, id), LastOf(&db->packs, id)))
			return false;

		if (*filename)
			XtUt::AppendEx(&db->removedPacks, filename);

		SetDirty(db, false);

		return true;
	}

	// Xt::DomDb::RemoveRecord

	bool RemoveRecord(DomDb* db, XtId id)
	{
		if (DomRecord* record = FindRecordById(db, id))
		{
			if (DomPack* pack = PackOf(record))
				return RemoveRecord(pack, record->id);
		}

		return false;
	}

	// Xt::Dom::FindPackById

	DomPack* FindPackById(DomDb* db, XtId packId)
	{
		return (db && packId) ? db->packs[packId] : 0;
	}

	// Xt::Dom::FindRecordById

	DomRecord* FindRecordById(DomDb* db, XtId recordId)
	{
		if (db && recordId)
		{
			for(DomDb::Iterator it(db); it; ++it)
			{
				if (DomRecord* record = it->records[recordId])
					return record;
			}
		}

		return 0;
	}

}