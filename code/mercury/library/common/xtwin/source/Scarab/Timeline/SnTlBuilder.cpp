#include "Core/XtWinPch.h"
#include "XtWin/Core.h"
#include "XtWin/Timeline.h"
#include "../ScarabImpl.h"
#include "SnTlImpl.h"

Gdiplus::Color LerpNoGamma(Gdiplus::Color colour0, Gdiplus::Color colour1, float a);

struct SnTlRowId
{
	XtId					id;
	XtU32					index;
	XtUt::WeakRef<TlRow>	lastRow;

public:
	SnTlRowId(XtId id_, XtU32 index_) :
		id(id_),
		index(index_)
	{
	}
};

class SnTlRowBuilder : public LvFieldDom
{
	ScarabTimeline*		_sn;
	LarvaSchema*		_schema;
	TlDiagram*			_diagram;
	TlRow*				_currentRow;
	SnTlRowId*			_parentId;

public:
	SnTlRowBuilder(ScarabTimeline* sn, LarvaSchema* schema, TlDiagram* diagram, TlRow* currentRow, SnTlRowId* parentId) :
		_sn(sn),
		_schema(schema),
		_diagram(diagram),
		_currentRow(currentRow),
		_parentId(parentId)
	{
	}

public:
	SnTlRow* AddRow(LarvaField* lf, Xt::DomElement* dom, SnTlRowId* id)
	{
		if (!lf || !dom || !id || !id->id)
		{
			DebugLn("AddRow FAIL %s", lf->name.Text());
			return 0;
		}

		SnTlRow* row = _sn->FindRow(id->id, id->index);

		if (!row)
			row = new SnTlRow(_sn, lf, dom, id->id, id->index);

		if (row)
		{
			static const Gdiplus::Color low		= LerpNoGamma((Gdiplus::Color)0xFFA0B0E0, Gdiplus::Color::Black, 0.05f);
			static const Gdiplus::Color high	= LerpNoGamma(low, Gdiplus::Color::White, 0.5f);

			row->Detach();
			_diagram->InsertRow(row, _parentId->lastRow);

			_parentId->lastRow = row->Anchor();
		}

		id->index++;

		return row;
	}

	SnTlKey* AddKey(LarvaField* lf, Xt::DomComponent* dom, SnTlRowId* id)
	{
		if (!lf || !dom || !id || !id->id || !_currentRow)
		{
			DebugLn("AddKey FAIL %s", lf->name.Text());
			return 0;
		}

		SnTlKey* key = ((SnTlRow*)_currentRow)->FindKey(dom->id);

		if (!key)
			key = new SnTlKey(_sn, lf, dom);

		return (SnTlKey*)_currentRow->AddKey(key);
	}

public:
	virtual bool Visit(LfStruct* field, Xt::DomBlock* dom)
	{
		if (!dom)
			return true;

		if (field->attrs.GetBool("Timeline", "row"))
		{
			SnTlRow* row = AddRow(field, dom, _parentId);

			return !row || VisitMembers(field->type, dom, &SnTlRowBuilder(_sn, _schema, _diagram, row, _parentId), Xt::kIndexAcquire);
		}

		return VisitMembers(field->type, dom, this, Xt::kIndexAcquire);
	}

	virtual bool Visit(LfStrictPtr* field, Xt::DomComponent* dom)
	{
		if (!dom)
			return true;

		TlRow* row = _currentRow;

		SnTlRowId id(dom->id, 0);

		if (field->attrs.GetBool("Timeline", "row"))
			row = AddRow(field, dom, &id);
		else if (field->attrs.GetBool("Timeline", "key"))
			AddKey(field, dom, &id);

		return !row || VisitMembers(field->type, &dom->data, &SnTlRowBuilder(_sn, _schema, _diagram, row, &id), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfPtr* field, Xt::DomComponent* dom)
	{
		if (!dom)
			return true;

		TlRow* row = _currentRow;

		SnTlRowId id(dom->id, 0);

		if (field->attrs.GetBool("Timeline", "row"))
			row = AddRow(field, dom, &id);
		else if (field->attrs.GetBool("Timeline", "key"))
			AddKey(field, dom, &id);

		LtStruct* type = TypeOf(_schema, field->baseType, dom);

		return !row || !type || VisitMembers(type, &dom->data, &SnTlRowBuilder(_sn, _schema, _diagram, row, &id), Xt::kIndexAcquire);
	}

	virtual bool Visit(LfArray* field, Xt::DomArray* dom)
	{
		return !dom || VisitArrayItems(field, dom, this, Xt::kIndexAcquire);
	}
};

void DoSnTlBuild(ScarabTimeline* sn, LarvaSchema* schema, TlDiagram* diagram, LtStruct* type, Xt::DomBlock* dom)
{
	SnTlRowId id(0, 0);

	VisitMembers(type, dom, &SnTlRowBuilder(sn, schema, diagram, 0, &id), Xt::kIndexAcquire);
}