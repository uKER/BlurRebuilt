#ifndef XT_IMPL_MERGE_H
#define XT_IMPL_MERGE_H

#ifdef _MSC_VER
#pragma once
#endif

//
// Xt Diff/Merge/Patch
//

namespace Xt
{

	enum ChangeType
	{
		kChangeNone,
		kChangeRemove,
		kChangeLeaf,
		kChangeBlock,
		kChangeArray,
		kChangeComponent,
		kChangeRecord,
		kChangePack,
		kChangeId,
		kChangeString,
		kChangeIndex,
		kChangeConflict
	};

	//
	// Xt::Change
	//

	struct Change
	{
		ChangeType type;

	protected:
		Change(ChangeType type_);

	public:
		virtual ~Change();

	public:
		virtual Change* Clone();
	};

	//
	// Xt::ChangeRemove
	//

	struct ChangeRemove : public Change
	{
	public:
		ChangeRemove();
		virtual ~ChangeRemove();

	public:
		virtual ChangeRemove* Clone();
	};

	//
	// Xt::ChangeLeaf
	//

	struct ChangeLeaf : public Change
	{
		DomElement* element;

	public:
		ChangeLeaf();
		virtual ~ChangeLeaf();

	public:
		virtual ChangeLeaf* Clone();
	};

	ChangeLeaf* NewChangeLeaf(DomElement* element);

	//
	// Xt::ChangeBlock
	//

	struct ChangeBlock : public Change
	{
		XtUt::Dictionary<XtString, Change> changes;

	public:
		typedef XtUt::Dictionary<XtString, Change>::Iterator Iterator;

	public:
		ChangeBlock();
		virtual ~ChangeBlock();

	public:
		virtual ChangeBlock* Clone();
	};

	//
	// Xt::ChangeArray
	//

	struct ChangeArray : public Change
	{
		Change*							index;
		XtUt::Dictionary<XtId, Change>	changes;

	public:
		typedef XtUt::Dictionary<XtId, Change>::Iterator Iterator;

	public:
		ChangeArray();
		virtual ~ChangeArray();

	public:
		virtual ChangeArray* Clone();
	};

	//
	// Xt::ChangeComponent
	//

	struct ChangeComponent : public Change
	{
		Change* id;
		Change* type;
		Change* data;

	public:
		ChangeComponent();
		virtual ~ChangeComponent();

	public:
		virtual ChangeComponent* Clone();
	};

	//
	// Xt::ChangeRecord
	//

	struct ChangeRecord : public Change
	{
		Change* name;
		Change* root;

	public:
		ChangeRecord();
		virtual ~ChangeRecord();

	public:
		virtual ChangeRecord* Clone();
	};

	//
	// Xt::ChangePack
	//

	struct ChangePack : public Change
	{
		Change*							name;
		Change*							properties;
		XtUt::Dictionary<XtId, Change>	changes;

	public:
		typedef XtUt::Dictionary<XtId, Change>::Iterator Iterator;

	public:
		ChangePack();
		virtual ~ChangePack();

	public:
		virtual ChangePack* Clone();
	};

	//
	// Xt::ChangeId
	//

	struct ChangeId : public Change
	{
		XtId value;

	public:
		ChangeId();
		virtual ~ChangeId();

	public:
		virtual ChangeId* Clone();
	};

	ChangeId* NewChangeId(XtId value);

	//
	// Xt::ChangeString
	//

	struct ChangeString : public Change
	{
		XtString value;

	public:
		ChangeString();
		virtual ~ChangeString();

	public:
		virtual ChangeString* Clone();
	};

	ChangeString* NewChangeString(const XtString& value);

	//
	// Xt::ChangeIndex
	//

	struct ChangeIndex : public Change
	{
		XtUt::Array<XtId> order;

	public:
		ChangeIndex();
		virtual ~ChangeIndex();

	public:
		virtual ChangeIndex* Clone();
	};

	ChangeIndex* NewChangeIndex(XtUt::Array<XtId>* order);

	//
	// Xt::ChangeConflict
	//

	struct ChangeConflict : public Change
	{
		Change* lhs;
		Change* rhs;

	public:
		ChangeConflict();
		virtual ~ChangeConflict();

	public:
		virtual ChangeConflict* Clone();
	};

	ChangeConflict* NewChangeConflict(Change* lhs, Change* rhs);

	//
	// Xt::Diff/Merge/Patch
	//

	Change* Diff(DomElement* lhs, DomElement* rhs, bool* result = 0);
	Change* Merge(Change* lhs, Change* rhs);
	bool Patch(DomElement* element, Change* change);

}

#endif // XT_IMPL_MERGE_H