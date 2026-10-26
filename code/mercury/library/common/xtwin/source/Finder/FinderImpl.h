#ifndef XTWIN_FINDER_FINDERIMPL_H
#define XTWIN_FINDER_FINDERIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/Core.h"
#include "XtWin/WndCore.h"
#include "XtWin/NgMaths.h"
#include "XtWin/Finder.h"

class FinderControl;

enum FinderCmp
{
	kFinderCmp_None,
	kFinderCmp_Equal,
	kFinderCmp_NotEqual,
	kFinderCmp_Less,
	kFinderCmp_LessEqual,
	kFinderCmp_Greater,
	kFinderCmp_GreaterEqual,
	kFinderCmp_Between,
	kFinderCmp_IsTrue,
	kFinderCmp_IsFalse,
	kFinderCmp_Contains,
	kFinderCmp_DoesNotContain,
	kFinderCmp_StartsWith,
	kFinderCmp_EndsWith,
	kFinderCmp_IsNull,
	kFinderCmp_IsNotNull,
	kFinderCmp_Display
};

//
// FinderControlId
//

class FinderControlId
{
public:
	FinderControlId(FinderControl* control);
	~FinderControlId();

public:
	bool Allocate();
	void Free();

public:
	inline int Id() const			{ return _id; }
	inline HMENU IdAsHMenu() const	{ return (HMENU)(intptr_t)_id; }
	inline operator int() const		{ return _id; }

private:
	int				_id;
	FinderControl*	_control;
};

inline bool operator==(const FinderControlId& lhs, int rhs)
{
	return lhs.Id() == rhs;
}

inline bool operator==(int lhs, const FinderControlId& rhs)
{
	return lhs == rhs.Id();
}

inline bool operator==(const FinderControlId& lhs, UINT_PTR rhs)
{
	return lhs.Id() == rhs;
}

inline bool operator==(UINT_PTR lhs, const FinderControlId& rhs)
{
	return lhs == rhs.Id();
}

//
// FinderRow
//

class FinderRow : public ChildWindow
{
public:
	FinderRow(FinderDlg* finder);
	virtual ~FinderRow();

public:
	virtual void BuildQuery(Xt::Query* query);
	virtual void BuildColumns(XtUt::List<FinderColumn>* columns);

public:
	void SetLabel(const XtString& label);
	void SetEnabled(bool enabled);
	FinderControl* AddControl(FinderControl* control);

public:
	template<typename Type> Type* AddControl(Type* control)
	{
		return (Type*)AddControl(static_cast<FinderControl*>(control));
	}

public:
	virtual int Height();
	virtual void OnEnableChanged();
	virtual void OnFocus(FinderControl* control);
	virtual void OnChanged(FinderControl* control);

public:
	FinderDlg* Finder()
	{
		return _finder;
	}

	bool IsEnabled()
	{
		return _enabled;
	}

public:
	virtual bool PreCreateWindow(CREATESTRUCT* cs);
	virtual LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	void DoLayout();

private:
	int AllocateControlId(FinderControl* control);
	void FreeControlId(int id);

private:
	static const int kIdLabel = 1;

private:
	FinderDlg* _finder;

private:
	HWND		_labelWnd;
	XtString	_labelText;
	int			_labelWidth;
	int			_labelMargin;

private:
	bool _enabled;

private:
	struct ControlId
	{
		int				id;
		FinderControl*	control;

	public:
		inline ControlId(int id_ = 0, FinderControl* control_ = 0) :
			id(id_),
			control(control_)
		{
		}

	public:
		inline bool operator<(const ControlId& ci) const
		{
			return id < ci.id;
		}
	};

private:
	XtUt::List<FinderControl>	_controls;
	XtUt::Set<ControlId>		_ids;

friend class FinderDlg;
friend class FinderControl;
friend class FinderControlId;
};

//
// FinderControl
//

class FinderControl
{
public:
	FinderControl(FinderRow* row);
	virtual ~FinderControl();

public:
	void SetWidth(int min, int max, float weight);
	void SetEnabled(bool enabled);

public:
	virtual bool Create();
	virtual void Layout(WinRect& rc);

public:
	virtual void OnCommand(int id, int code);
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnChanged();
	virtual void OnEnableChanged();

public:
	FinderRow* Row()
	{
		return _row;
	}

	bool IsEnabled()
	{
		return _enabled && (!_row || _row->IsEnabled());
	}

private:
	FinderRow* _row;

private:
	int		_widthMin;
	int		_widthMax;
	float	_widthWeight;
	int		_widthActual;

private:
	bool	_enabled;

friend class FinderRow;
};

//
// FinderControls
//

class FcCombo : public FinderControl
{
public:
	FcCombo(FinderRow* row);
	virtual ~FcCombo();

public:
	void Clear();
	void AddString(const char* text);
	void SetDroppedWidth(int width);
	void SetSelection(int index);
	int GetSelection();

public:
	virtual bool Create();
	virtual void Layout(WinRect& rc);

public:
	virtual void OnCommand(int id, int code);

protected:
	HWND			_hwnd;
	FinderControlId	_id;
};

class FcEdit : public FinderControl
{
public:
	FcEdit(FinderRow* row);
	virtual ~FcEdit();

public:
	void SetText(const XtString& text);
	XtString GetText();

public:
	virtual bool Create();
	virtual void Layout(WinRect& rc);

public:
	virtual void OnCommand(int id, int code);

protected:
	HWND			_hwnd;
	FinderControlId	_id;
};

class FcSelectType : public FcCombo
{
public:
	FcSelectType(FinderRow* row);
	virtual ~FcSelectType();

public:
	enum Filter
	{
		kNone,
		kRecords,
		kStructs
	};

public:
	void		SetTypeFilter(Filter filter);
	void		SetSelectedType(LarvaType* type);
	LarvaType*	GetSelectedType();

public:
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnEnableChanged();

private:
	void Populate();

private:
	Filter					_filter;
	XtUt::Array<LarvaType*>	_types;
};

class FcSelectField : public FcCombo
{
public:
	FcSelectField(FinderRow* row);
	virtual ~FcSelectField();

public:
	void			SetType(LtStruct* type);
	void			SetSelectedField(Xt::FieldPath* path);
	Xt::FieldPath*	GetSelectedField();

public:
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnEnableChanged();

private:
	void Populate();

public:
	struct Item
	{
		XtString		text;
		Xt::FieldPath	path;

		~Item()
		{
			Free(path.next);
		}
	};

private:
	LtStruct*			_type;
	XtUt::List<Item>	_fields;
};

class FcCompare : public FcCombo
{
public:
	FcCompare(FinderRow* row);
	virtual ~FcCompare();

public:
	void		SetField(LarvaField* field);
	void		SetSelectedCmp(FinderCmp cmp);
	FinderCmp	GetSelectedCmp();

public:
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnEnableChanged();

private:
	void Populate();

private:
	LarvaField*				_field;
	XtUt::Array<FinderCmp>	_cmps;
};

class FcSelectRecord : public FcCombo
{
public:
	FcSelectRecord(FinderRow* row);
	virtual ~FcSelectRecord();

public:
	void SetType(LtStruct* type);
	void SetSelectedRecord(XtId recordId);
	XtId GetSelectedRecord();

public:
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnEnableChanged();

private:
	void Populate();

private:
	struct Item
	{
		XtString	label;
		XtId		id;

	public:
		Item() : id(0) { }
	};

private:
	static bool SortCompare(Item& lhs, Item& rhs);

private:
	LtStruct*			_type;
	XtUt::Array<Item>	_records;
};

class FcSelectLocString : public FcCombo
{
public:
	FcSelectLocString(FinderRow* row);
	virtual ~FcSelectLocString();

public:
	void SetRecord(XtId recordId);
	void SetSelectedId(XtId locId);
	XtId GetSelectedId();

public:
	virtual void OnCreate();
	virtual void OnFocus();
	virtual void OnEnableChanged();

private:
	void Populate();

private:
	struct Item
	{
		XtString	label;
		XtId		id;

	public:
		Item() : id(0) { }
	};

private:
	static bool SortCompare(Item& lhs, Item& rhs);

private:
	XtId				_recordId;
	XtUt::Array<Item>	_strings;
};

//
// FinderRows
//

class FrRecordOfType : public FinderRow
{
public:
	FrRecordOfType(FinderDlg* finder);
	virtual ~FrRecordOfType();

public:
	virtual void BuildQuery(Xt::Query* query);

private:
	FcSelectType* _typeCtrl;
};

class FrObjectsOfType : public FinderRow
{
public:
	FrObjectsOfType(FinderDlg* finder);
	virtual ~FrObjectsOfType();

public:
	virtual void BuildQuery(Xt::Query* query);

private:
	FcSelectType* _typeCtrl;
};

class FrRecordsNamed : public FinderRow
{
public:
	FrRecordsNamed(FinderDlg* finder);
	virtual ~FrRecordsNamed();

public:
	virtual void BuildQuery(Xt::Query* query);

private:
	FcEdit* _editCtrl;
};

class FrDependsOn : public FinderRow
{
public:
	FrDependsOn(FinderDlg* finder);
	virtual ~FrDependsOn();

public:
	virtual void BuildQuery(Xt::Query* query);
	virtual void BuildColumns(XtUt::List<FinderColumn>* columns);

public:
	virtual void OnEnableChanged();
	virtual void OnFocus(FinderControl* control);
	virtual void OnChanged(FinderControl* control);

public:
	void SelectRecord(Xt::DomRecord* record);

private:
	void PopulateRecords();

private:
	FcSelectType*	_typeCtrl;
	FcCombo*		_recordsCtrl;

public:
	struct Item
	{
		XtString	text;
		XtId		id;

	public:
		Item(const XtString& text_, XtId id_) :
			text(text_),
			id(id_)
		{
		}
	};

private:
	XtUt::Array<Item> _records;
};

class FrExpression : public FinderRow
{
public:
	FrExpression(FinderDlg* finder);
	virtual ~FrExpression();

public:
	virtual void BuildQuery(Xt::Query* query);
	virtual void BuildColumns(XtUt::List<FinderColumn>* columns);

public:
	virtual void OnEnableChanged();
	virtual void OnFocus(FinderControl* control);
	virtual void OnChanged(FinderControl* control);

private:
	void Sync();

private:
	FcSelectType*	_typeCtrl;
	FcSelectField*	_fieldCtrl;
	FcCompare*		_compareCtrl;
	FcEdit*			_lowCtrl;
	FcEdit*			_highCtrl;
};

class FrUsingLocString : public FinderRow
{
public:
	FrUsingLocString(FinderDlg* finder);
	virtual ~FrUsingLocString();

public:
	virtual void BuildQuery(Xt::Query* query);

public:
	virtual void OnEnableChanged();
	virtual void OnFocus(FinderControl* control);
	virtual void OnChanged(FinderControl* control);

private:
	FcSelectRecord*		_recordCtrl;
	FcSelectLocString*	_stringCtrl;
};

#endif // XTWIN_FINDER_FINDERIMPL_H