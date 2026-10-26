#include "Common/XtPch.h"
#include <Xt/XtTool.h>

namespace Xt
{

	// Xt::DomRecord

	DomRecord::DomRecord() :
		DomElement(kDomRecord),
		id(0),
		dirty(false)
	{
		root.parent = this;
	}

	DomRecord::~DomRecord()
	{
		Notify(this, &DomRemoved(this));
	}

	// Xt::DomRecord::RecordOf

	DomRecord* RecordOf(DomElement* element)
	{
		for(;;)
		{
			if (!element)
				return 0;

			if (element->kind == kDomRecord)
				return (DomRecord*)element;

			element = element->parent;
		}
	}

	// Xt::DomRecord::AsRecordRoot

	DomComponent* AsRecordRoot(DomElement* element)
	{
		return (element && (element->kind == kDomRecord)) ? &((DomRecord*)element)->root : 0;
	}

	// Xt::DomRecord::SetName

	bool SetName(DomRecord* record, const XtString& name)
	{
		if (!record)
			return false;

		record->name = name;

		Notify(record, &DomRecordChanged(record));

		return true;
	}

	// Xt::DomRecord::SetDirty

	void SetDirty(DomRecord* record, bool forceNotify)
	{
		assert(record != 0);

		if (!record->dirty || forceNotify)
		{
			record->dirty = true;

			Notify(record, &DomRecordChanged(record));
		}

		if (DomPack* pack = PackOf(record))
			SetDirty(pack, forceNotify);
	}

	// Xt::DomRecord::Revert

	bool Revert(DomRecord* record, DomPack* pristinePack)
	{
		if (!record || !pristinePack)
			return false;

		DomRecord* pristineRecord = pristinePack->records[record->id];

		if (!pristineRecord)
			return false;

		if (!Clone(&record->root, &pristineRecord->root))
			return false;

		record->dirty = false;

		Notify(record, &DomRecordChanged(record));

		return true;
	}

	// Xt::DomRecord::RevertToPristine

	bool RevertToPristine(DomRecord* record)
	{
		DomPack*	pack	= PackOf(record);
		DomDb*		db		= DbOf(pack);

		if (!pack || !db)
			return false;

		XtUt::Block* data = db->pristine[pack->id];

		if (!data)
			return false;

		XtUt::Ptr<DomElement> pristinePack = Parse2(data, 0, 0);

		if (!pristinePack)
			return false;

		return Revert(record, AsPack(pristinePack));
	}

}