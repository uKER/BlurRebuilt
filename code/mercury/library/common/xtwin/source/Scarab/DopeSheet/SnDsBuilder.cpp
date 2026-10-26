#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/DopeSheet.h"
#include "../ScarabImpl.h"
#include "SnDSImpl.h"

Gdiplus::Color LerpNoGamma(Gdiplus::Color colour0, Gdiplus::Color colour1, float a);

int ParentCount(DsRow* row)
{
	if (!row->Parent())
		return 0;

	return 1 + ParentCount(row->Parent());
}

struct SnDsRowId
{
	XtId					id;
	XtU32					index;
	XtUt::WeakRef<DsRow>	lastRow;

public:
	SnDsRowId(XtId id_, XtU32 index_) :
		id(id_),
		index(index_)
	{
	}
};

class SnDsRowBuilder : public LvFieldDom
{
	ScarabDopeSheet*		_sn;
	LarvaSchema*		_schema;
	DsRow*				_parent;
	SnDsRowId*			_parentId;

public:
	SnDsRowBuilder(ScarabDopeSheet* sn, LarvaSchema* schema, DsRow* parent, SnDsRowId* parentId) :
		_sn(sn),
		_schema(schema),
		_parent(parent),
		_parentId(parentId)
	{
	}

public:
	SnDsRow* AddRow(LarvaField* lf, Xt::DomElement* dom, SnDsRowId* id)
	{
		if (!lf || !dom || !id || !id->id)
		{
			DebugLn("AddRow FAIL %s", lf->name.Text());
			return 0;
		}

		SnDsRow* row = _sn->FindRow(id->id, id->index);

		if (!row)
			row = new SnDsRow(_sn, lf, dom, id->id, id->index);

		if (row)
		{
			static const Gdiplus::Color low		= LerpNoGamma((Gdiplus::Color)0xFFA0B0E0, Gdiplus::Color::Black, 0.05f);
			static const Gdiplus::Color high	= LerpNoGamma(low, Gdiplus::Color::White, 0.5f);

			row->Detach();
			_parent->InsertRow(row, _parentId->lastRow);

			row->SetColour(LerpNoGamma(low, high, Clamp((ParentCount(row) - 1) / 2.0f, 0.0f, 1.0f)).GetValue());

			_parentId->lastRow = row->Anchor();
		}

		id->index++;

		return row;
	}

	SnDsKey* AddKey(LarvaField* lf, Xt::DomComponent* dom, SnDsRowId* id)
	{
		if (!lf || !dom || !id || !id->id)
		{
			DebugLn("AddKey FAIL %s", lf->name.Text());
			return 0;
		}

		SnDsKey* key = ((SnDsRow*)_parent)->FindKey(dom->id);

		if (!key)
			key = new SnDsKey(_sn, lf, dom);

		return (SnDsKey*)_parent->AddKey(key);
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		if (!dom)
			return true;

		if (field->attrs.GetBool("DopeSheet", "row"))
		{
			SnDsRow* row = AddRow(field, dom, _parentId);

			return !row || VisitMembers(field->type, dom, &SnDsRowBuilder(_sn, _schema, row, _parentId), Xt::kIndexAcquire);
		}

		return VisitMembers(field->type, dom, this, Xt::kIndexAcquire);
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		if (!dom)
			return true;

		DsRow* row = _parent;

		SnDsRowId id(dom->id, 0);

		if (field->attrs.GetBool("DopeSheet", "row"))
			row = AddRow(field, dom, &id);
		else if (field->attrs.GetBool("DopeSheet", "key"))
			AddKey(field, dom, &id);

		return !row || VisitMembers(field->type, &dom->data, &SnDsRowBuilder(_sn, _schema, row, &id), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		if (!dom)
			return true;

		DsRow* row = _parent;

		SnDsRowId id(dom->id, 0);

		if (field->attrs.GetBool("DopeSheet", "row"))
			row = AddRow(field, dom, &id);
		else if (field->attrs.GetBool("DopeSheet", "key"))
			AddKey(field, dom, &id);

		LtStruct* type = TypeOf(_schema, field->baseType, dom);

		return !row || !type || VisitMembers(type, &dom->data, &SnDsRowBuilder(_sn, _schema, row, &id), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		return !dom || VisitArrayItems(field, dom, this, Xt::kIndexAcquire);
	}
};

void DoSnDSBuild(ScarabDopeSheet* sn, LarvaSchema* schema, DsRow* parent, LtStruct* type, Xt::DomBlock* dom)
{
	SnDsRowId id(0, 0);

	VisitMembers(type, dom, &SnDsRowBuilder(sn, schema, parent, &id), Xt::kIndexAcquire); 
}