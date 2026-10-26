#ifndef SCARABIMPL_H
#define SCARABIMPL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "XtWin/WndCore.h"
#include "XtWin/DopeSheet.h"
#include "XtWin/Timeline.h"
#include "XtWin/Scarab.h"
#include "NodeGraph/ScarabDomNodeGraph.h"
#include "DopeSheet/ScarabDopeSheet.h"
#include "Timeline/ScarabTimeline.h"
#include "XtWin/GridView.h"

//
// ScarabNode
//

enum ScarabLayout
{
	kSlVertical,
	kSlHorizontal,
	kSlInline
};

struct ScarabClipTarget
{
	Xt::DomElement*	element;
	LarvaField*		field;

public:
	ScarabClipTarget(Xt::DomElement* element_, LarvaField* field_);
};

struct ScarabLayoutContext
{
	HDWP _dwp;
	HWND _lastLayoutWnd;

public:
	ScarabLayoutContext();

public:
	void Begin();
	void End();

public:
	void SetWindowPos(HWND wnd, int x, int y, int cx, int cy, UINT flags);
};

class ScarabNode
{
public:
	ScarabNode(ScarabPanel* panel = 0);
	virtual ~ScarabNode();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomField* parentDom);
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual void OnBind(Xt::DomComponent* parentDom);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual LRESULT OnNotify(NMHDR* hdr);
	virtual void OnCommand(int id, int code);
	virtual void OnScroll(int id, int msg, int method, HWND scrollBar);
	virtual void OnLeftButton(const WinPoint& pt);
	virtual void OnContextMenu(const WinPoint& pt);
	virtual void OnPaint(HDC hdc, const WinRect& rcPaint);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	bool Populate();
	bool Build();
	void Bind(Xt::DomField* parentDom);
	void Bind(Xt::DomBlock* parentDom);
	void Bind(Xt::DomArray* parentDom, int index);
	void Bind(Xt::DomComponent* parentDom);
	void Measure(HDC ic);
	void Layout(ScarabLayoutContext* layoutContext, const WinPoint& parentOrigin);

public:
	ScarabNode* Add(ScarabNode* node);
	ScarabNode* Insert(int index, ScarabNode* node);
	void Clear();
	void SetEnabled(bool enabled);

public:
	virtual ScarabNode* HitTestLabel(const WinPoint& pt);

public:
	template<typename Type> inline Type* Add(Type* node)
	{
		return (Type*)Add((ScarabNode*)node);
	}

	template<typename Type> inline Type* Insert(int index, Type* node)
	{
		return (Type*)Insert(index, (ScarabNode*)node);
	}

public:
	inline ScarabPanel* Panel()
	{
		return _panel;
	}

	inline ScarabNode* Parent()
	{
		return _parent;
	}

	Xt::DomDb* Db();
	LarvaSchema* Schema();

	bool IsEnabled() const;

private:
	struct MeasurePassContext
	{
		int margin;

	public:
		inline MeasurePassContext() :
			margin(0)
		{
		}
	};

private:
	void MeasurePass_Margin(MeasurePassContext* ctx, ScarabNode* node);
	void MeasurePass_Position(MeasurePassContext* ctx, ScarabNode* node);

private:
	ScarabPanel* _panel;
	ScarabNode* _parent;

public:
	XtUt::List<ScarabNode> _nodes;

private:
	bool _enabled;
	bool _built;

public:
	bool		_labelVisible;
	XtString	_labelText;
	WinSize		_labelSize;
	int			_labelOffset;

public:
	bool		_borderVisible;
	u32			_borderColour;

public:
	WinPoint	_position;
	WinSize		_size;
	WinRect		_location;
	WinRect		_client;
	int			_labelAlignMargin;

public:
	WinPoint		_anchor;
	int				_spacing;
	ScarabLayout	_layoutMethod;
	WinRect			_padding;

public:
	bool _whiteBackground;
};

//
// ScarabId
//

class ScarabId
{
public:
	inline ScarabId(ScarabNode* node) :
		_id(0),
		_node(node)
	{
	}

	inline ~ScarabId()
	{
		Free();
	}

public:
	bool Allocate();
	void Free();

public:
	inline int Id() const
	{
		return _id;
	}

	inline HMENU IdAsHMenu() const
	{
		return (HMENU)(intptr_t)_id;
	}

	inline operator int() const
	{
		return _id;
	}
	
private:
	int			_id;
	ScarabNode*	_node;
};

inline bool operator==(const ScarabId& lhs, int rhs)
{
	return lhs.Id() == rhs;
}

inline bool operator==(int lhs, const ScarabId& rhs)
{
	return lhs == rhs.Id();
}

inline bool operator==(const ScarabId& lhs, UINT_PTR rhs)
{
	return lhs.Id() == rhs;
}

inline bool operator==(UINT_PTR lhs, const ScarabId& rhs)
{
	return lhs == rhs.Id();
}

//
// ScarabWnd
//

class ScarabWnd
{
public:
	inline ScarabWnd() :
		_hwnd(0)
	{
	}

	inline ~ScarabWnd()
	{
		Close();
	}

public:
	bool Create(DWORD styleEx, LPCTSTR className, LPCTSTR windowName, DWORD style, HWND parent, int id)
	{
		Close();

		 return (_hwnd = CreateWindowEx(styleEx, className, windowName, style, 0, 0, 0, 0, parent, (HMENU)(intptr_t)id, 0, 0)) != 0;
	}

	bool Create(DWORD styleEx, LPCSTR className, LPCSTR windowName, DWORD style, int width, int height, HWND parent, int id)
	{
		Close();

		return (_hwnd = CreateWindowEx(styleEx, className, windowName, style, 0, 0, width, height, parent, (HMENU)(intptr_t)id, 0, 0)) != 0;
	}

	bool Create(DWORD styleEx, LPCWSTR className, LPCWSTR windowName, DWORD style, HWND parent, int id)
	{
		Close();

		return (_hwnd = CreateWindowExW(styleEx, className, windowName, style, 0, 0, 0, 0, parent, (HMENU)(intptr_t)id, 0, 0)) != 0;
	}

	void Close()
	{
		if (_hwnd != 0)
		{
			DestroyWindow(_hwnd);
			_hwnd = 0;
		}
	}

public:
	inline operator HWND()
	{
		return _hwnd;
	}

private:
	HWND _hwnd;
};

//
// SnRecord
//

class SnRecord : public ScarabNode
{
public:
	SnRecord(LtStruct* type, const XtString& filter);
	virtual ~SnRecord();

public:
	virtual bool OnPopulate();

private:
	LtStruct*	_type;
	XtString	_filter;
};

//
// SnStruct
//

class SnStruct : public ScarabNode
{
public:
	SnStruct(LfStruct* field);
	virtual ~SnStruct();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual void OnBind(Xt::DomComponent* parentDom);
	virtual void OnMeasure(HDC ic);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	LfStruct*		_field;
	Xt::DomMonitor	_dom;
};

//
// SnGroup
//

class SnGroup : public ScarabNode
{
public:
	SnGroup(ScarabForm* form, const XtString& groupName, bool isCollapsed, XtU32 colour);
	virtual ~SnGroup();

public:
	ScarabNode* Body();

public:
	virtual bool OnPopulate();
	virtual bool OnBuild();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnLeftButton(const WinPoint& pt);
	virtual void OnPaint(HDC hdc, const WinRect& rcPaint);

public:
	virtual ScarabNode* HitTestLabel(const WinPoint& pt);

private:
	class SpcGroup : public ScarabPanelContext
	{
		SnGroup* _group;

	public:
		SpcGroup(SnGroup* group) :
			_group(group)
		{
		}

	public:
		virtual void OnSizeChanged()
		{
			_group->Panel()->DirtyLayout();
		}
	};

private:
	XtString	_groupName;
	XtU32		_rgb;
	bool		_collapsed;
	ScarabId	_idGroup;
	ScarabPanel	_wndGroup;
	SpcGroup	_spc;
};

//
// SnInt
//

class SnInt : public ScarabNode
{
public:
	SnInt(LfInt* field);
	virtual ~SnInt();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);
	virtual void OnScroll(int id, int msg, int method, HWND scrollBar);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnInt* node);

private:
	void SyncControls();

private:
	LfInt*			_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_edit;
	ScarabWnd		_slider;
	ScarabId		_idEdit;
	ScarabId		_idSlider;
	int				_manualSet;
	XtI64			_rangeLow;
	XtI64			_rangeHigh;
};

//
// SnFloat
//

class SnFloat : public ScarabNode
{
public:
	SnFloat(LfFloat* field);
	virtual ~SnFloat();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);
	virtual void OnScroll(int id, int msg, int method, HWND scrollBar);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFloat* node);

private:
	void SyncControls();

private:
	LfFloat*		_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_edit;
	ScarabWnd		_slider;
	ScarabWnd		_icon;
	ScarabId		_idEdit;
	ScarabId		_idSlider;
	ScarabId		_idIcon;
	int				_manualSet;
	double			_rangeLow;
	double			_rangeHigh;
	int				_controlWidth;
};

//
// SnString
//

class SnString : public ScarabNode
{
public:
	SnString(LfString* field);
	virtual ~SnString();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnString* node);

private:
	void SyncControls();
	void LazyCheck();

private:
	LfString*		_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_edit;
	ScarabId		_id;
	int				_manualSet;
};

//
// SnWideString
//

class SnWideString : public ScarabNode
{
public:
	SnWideString(LfWideString* field);
	virtual ~SnWideString();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnWideString* node);

private:
	void SyncControls();

private:
	LfWideString*	_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_edit;
	ScarabId		_id;
	int				_manualSet;
};

//
// SnBool
//

class SnBool : public ScarabNode
{
public:
	SnBool(LfBool* field);
	virtual ~SnBool();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnBool* node);

private:
	void SyncControls();

private:
	LfBool*			_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_button;
	ScarabId		_id;
	int				_manualSet;
};

//
// SnEnum
//

class SnEnum : public ScarabNode
{
public:
	SnEnum(LfEnum* field);
	virtual ~SnEnum();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnEnum* node);
	static bool ComboCompare(LarvaOption* lhs, LarvaOption* rhs);

private:
	void SyncControls();
	void PopulateCombo();

private:
	LfEnum*						_field;
	Xt::DomMonitor				_dom;
	ScarabWnd					_combo;
	ScarabId					_id;
	int							_manualSet;
	XtUt::Array<LarvaOption*>	_comboItems;
	int							_default;
};

//
// SnFlags
//

class SnFlags : public ScarabNode
{
public:
	SnFlags(LfFlags* field);
	virtual ~SnFlags();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	LfFlags*		_field;
	Xt::DomMonitor	_dom;
};

//
// SnFlagBool
//

class SnFlagBool : public ScarabNode
{
public:
	SnFlagBool(LarvaOption* field);
	virtual ~SnFlagBool();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlagBool* node);

private:
	void SyncControls();

private:
	LarvaOption*	_field;
	Xt::DomMonitor	_dom;
	ScarabWnd		_button;
	ScarabId		_id;
	int				_manualSet;
};

//
// SnArray
//

class SnArrayHeader;

class SnArray : public ScarabNode
{
public:
	SnArray(LfArray* field);
	virtual ~SnArray();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	void OnIndexChanged();

private:
	void SetDom(Xt::DomArray* array);
	void SyncBody();

public:
	inline SnArrayHeader* Header()
	{
		return _header;
	}

	inline ScarabNode* Body()
	{
		return _body;
	}

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnArray* node);

private:
	LfArray*		_field;
	Xt::DomMonitor	_dom;
	SnArrayHeader*	_header;
	bool			_builtBody;
	ScarabNode*		_body;

friend class SnArrayHeader;
};

//
// SnArrayHeader
//

class SnArrayHeader : public ScarabNode
{
public:
	SnArrayHeader(SnArray* array);
	virtual ~SnArrayHeader();

public:
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);
	virtual void OnPaint(HDC hdc, const WinRect& rcPaint);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

public:
	int Index();

public:
	void SyncControls(bool populateCombo);
	void PopulateCombo();
	void SetMiniMode(bool miniMode);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnArrayHeader* hdr);

private:
	Xt::DomMonitor _notifyDom;

private:
	SnArray*				_array;
	int						_index;
	XtUt::Array<XtString>	_items;

private:
	ScarabWnd		_combo;
	ScarabWnd		_add;
	ScarabWnd		_remove;
	ScarabWnd		_moveUp;
	ScarabWnd		_moveDown;
	ScarabWnd		_duplicate;
	ScarabId		_idCombo;
	ScarabId		_idAdd;
	ScarabId		_idRemove;
	ScarabId		_idMoveUp;
	ScarabId		_idMoveDown;
	ScarabId		_idDuplicate;

private:
	bool			_miniMode;
};

//
// SnFlatArray
//

class SnFlatArray : public ScarabNode
{
public:
	SnFlatArray(LfArray* field);
	virtual ~SnFlatArray();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlatArray* node);

private:
	void SetDom(Xt::DomArray* arr);
	void PopulateBody();
	void UpdateControls();

private:
	LfArray*		_field;
	Xt::DomMonitor	_dom;
	ScarabId		_msgId;

friend class SnFlatArrayItem;
};

//
// SnFlatArrayItem
//

class SnFlatArrayItem : public ScarabNode
{
public:
	SnFlatArrayItem(LfArray* field, SnFlatArray* array, int index);
	virtual ~SnFlatArrayItem();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnFlatArrayItem* node);

private:
	void SetDom(Xt::DomArray* arr);

private:
	LfArray*		_field;
	Xt::DomMonitor	_dom;
	int				_index;

private:
	SnFlatArray*	_array;
	int				_originalWidth;

private:
	ScarabWnd	_add;
	ScarabWnd	_remove;
	ScarabId	_idAdd;
	ScarabId	_idRemove;

friend class SnFlatArray;
};

//
// SnHandle
//

class SnHandle : public ScarabNode
{
public:
	SnHandle(LfHandle* field);
	virtual ~SnHandle();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

private:
	struct Item
	{
		XtId		id;
		XtString	label;
	};

private:
	void SetDom(Xt::DomField* field);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnHandle* node);
	static bool ComboCompare(const Item& lhs, const Item& rhs);

private:
	void SyncControls();
	void PopulateCombo(bool fast);
	void AddItem(Xt::DomRecord* record, bool bypassFilter);

private:
	LfHandle*			_field;
	Xt::DomMonitor		_dom;
	ScarabWnd			_combo;
	ScarabWnd			_open;
	ScarabId			_idCombo;
	ScarabId			_idOpen;
	XtUt::Array<Item>	_items;
};

//
// SnStrictPtr
//

class SnStrictPtr : public ScarabNode
{
public:
	SnStrictPtr(LfStrictPtr* field);
	virtual ~SnStrictPtr();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual void OnBind(Xt::DomComponent* parentDom);
	virtual void OnMeasure(HDC ic);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	LfStrictPtr*	_field;
	Xt::DomMonitor	_dom;
};

//
// SnPtr
//

class SnPtrHeader;

class SnPtr : public ScarabNode
{
public:
	SnPtr(LfPtr* field);
	virtual ~SnPtr();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	void OnMeasure(HDC ic);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomComponent* block);

public:
	void OnTypeChanged();

private:
	bool PopulateBody();
	bool IsCompact();
	bool IsCompactHyperLink();

private:
	LfPtr*			_field;
	Xt::DomMonitor	_dom;
	ScarabNode*		_header;
	ScarabNode*		_body;

friend class SnPtrHeader;
friend class SnPtrCompactHeader;
};

//
// SnPtrContainer
//

class SnPtrContainer : public ScarabNode
{
public:
	virtual void OnBind(Xt::DomComponent* parentDom);
};

//
// SnPtrHeader
//

class SnPtrHeader : public ScarabNode
{
public:
	SnPtrHeader(SnPtr* ptr);
	virtual ~SnPtrHeader();

public:
	virtual void OnBind(Xt::DomComponent* parentDom);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);

private:
	void SetDom(Xt::DomComponent* com);

private:
	struct Category
	{
		XtString name;
		XtString label;
	};

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPtrHeader* node);
	static bool CompareCategory(const Category& lhs, const Category& rhs);
	static bool CompareType(LtStruct* lhs, LtStruct* rhs);

private:
	void SyncControls(bool syncCategory);
	void PopulateAllType();
	void PopulateCategory();
	void PopulateType();

private:
	Category* GetSelectedCategory();
	LtStruct* GetSelectedType();
	LtStruct* GetDomType();

private:
	Xt::DomMonitor			_dom;
	SnPtr*					_ptr;
	bool					_hasCategory;
	ScarabWnd				_wndCategory;
	ScarabWnd				_wndType;
	ScarabId				_idCategory;
	ScarabId				_idType;
	XtUt::Array<LtStruct*>	_itemsAllType;
	XtUt::Array<Category>	_itemsCategory;
	XtUt::Array<LtStruct*>	_itemsType;
};

//
// SnPtrCompactHeader
//

class SnPtrCompactHeader : public ScarabNode
{
public:
	SnPtrCompactHeader(SnPtr* ptr, bool hyperLink);
	virtual ~SnPtrCompactHeader();

public:
	virtual void OnBind(Xt::DomComponent* parentDom);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual LRESULT OnNotify(NMHDR* hdr);
	virtual void OnCommand(int id, int code);

private:
	void SetDom(Xt::DomComponent* component);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPtrCompactHeader* node);
	static bool TypeListCompare(LtStruct* lhs, LtStruct* rhs);

private:
	void SyncControls();
	void PopulateTypeList();
	XtString GetTypeName();
	void DoPopup();

private:
	Xt::DomMonitor			_dom;
	SnPtr*					_ptr;
	bool					_hyperLink;
	ScarabWnd				_button;
	ScarabId				_idButton;
	XtUt::Array<LtStruct*>	_typeList;
	WinSize					_hyperSize;
};

//
// SnWeakPtr
//

class SnWeakPtr : public ScarabNode
{
public:
	SnWeakPtr(LfWeakPtr* field);
	virtual ~SnWeakPtr();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

public:
	bool AddItem(LtStruct* type, Xt::DomComponent* component, XtId fastId);

private:
	struct Item
	{
		XtId			id;
		XtString		label;
		LtStruct*		type;
		Xt::DomMonitor	dom;

	public:
		inline Item() :
			id(0),
			type(0)
		{
		}
	};

private:
	void SetDom(Xt::DomRef* ref);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnWeakPtr* node);
	static bool ComboCompare(const Item* lhs, const Item* rhs);

private:
	void SyncControls();
	void PopulateCombo(bool fast);
	bool HasDropper();

private:
	LfWeakPtr*			_field;
	Xt::DomMonitor		_dom;
	Xt::DomMonitor		_watch;
	ScarabWnd			_wndCombo;
	ScarabWnd			_wndDropper;
	ScarabId			_idCombo;
	ScarabId			_idDropper;
	XtUt::List<Item>	_items;
};

//
// SnLocString
//

class SnLocString : public ScarabNode
{
public:
	SnLocString(LfLocString* field);
	virtual ~SnLocString();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);

private:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnLocString* node);

private:
	void SyncControls();
	void PopulateRecord();
	void PopulateRow(bool fast);

private:
	struct Item
	{
		XtId		id;
		XtString	label;
	};

private:
	static bool ComboCompare(const Item& lhs, const Item& rhs);
	virtual HBRUSH OnControlColour(int id, int msg, HDC hdc, HWND hwnd);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

private:
	LfLocString*		_field;
	Xt::DomMonitor		_domBlock;
	Xt::DomMonitor		_domRecord;
	Xt::DomMonitor		_domRow;
	ScarabId			_idRecord;
	ScarabId			_idRow;
	ScarabId			_idPreview;
	ScarabId			_idOpen;
	ScarabWnd			_record;
	ScarabWnd			_row;
	ScarabWnd			_preview;
	ScarabWnd			_open;
	XtUt::Array<Item>	_items;
	int					_manualSet;
};

//
// SnControl
//

class SnControl : public ScarabNode
{
public:
	SnControl(LfControl* field);
	virtual ~SnControl();

public:
	LarvaAttr* FindAttr(const char* name);
	LarvaParam* FindParam(LarvaAttr* attr, const char* key);
	LarvaParam* FindParam(const char* attr, const char* param);

public:
	XtString GetString(const char* attrName, const char* paramName, const XtString& defaultValue = kXtNullString);
	double GetFloat(const char* attrName, const char* paramName, double defaultValue = 0.0f);
	XtI64 GetInt(const char* attrName, const char* paramName, int defaultValue = 0);
	bool GetBool(const char* attrName, const char* paramName, bool defaultValue = false);
	XtU32 GetColour(const char* attrName, const char* paramName, XtU32 defaultValue = 0xFFFFFF);

protected:
	LfControl* _field;
};

//
// SnColourSwatch
//

#include "XtWin/ColourSwatch.h"

class SnColourSwatch : public SnControl
{
public:
	SnColourSwatch(LfControl* field);
	virtual ~SnColourSwatch();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

private:
	void SetDom(Xt::DomBlock* block);
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnColourSwatch* swatch);

private:
	void SyncControls();

private:
	enum SwatchType
	{
		kNone,
		kRed,
		kGreen,
		kBlue,
		kRgb,
		kHue,
		kVal,
		kHsv,
		kHexRgb,
		kHexRgbA
	};

private:
	class Swatch : public ColourSwatch
	{
	public:
		Swatch(SnColourSwatch* swatch);

	public:
		virtual void OnChanged();

	private:
		SnColourSwatch* _swatch;
	};

private:
	Xt::DomMonitor	_dom;
	int				_ignoreNotifications;

private:
	ScarabId	_idSwatch;
	Swatch		_wndSwatch;

private:
	SwatchType		_swatchType;
	bool			_showAlpha;
};

//
// SnChart
//

#include "XtWin\Chart.h"

class SnChart;

class SnChartData : public ChartCallbacks
{
public:
	SnChartData(SnChart* sn);

public:
	virtual int	 GetLineCount();
	virtual void GetLineInfo(int lineNo, ChartLine* line);
	virtual void GetLineValue(int lineNo, int pointNo, ChartPoint* point);
	virtual void SetLineValue(int lineNo, int pointNo, const ChartPoint* point);
	virtual void GetInTangentValue(int lineNo, int pointNo, double *in);
	virtual void GetOutTangentValue(int lineNo, int pointNo, double *out);
	virtual void SetInTangentValue(int lineNo, int pointNo, double in);
	virtual void SetOutTangentValue(int lineNo, int pointNo, double out);

private:
	SnChart* _sn;
};

class SnChart : public SnControl
{
public:
	SnChart(LfControl* field);
	virtual ~SnChart();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnChart* node);

private:
	int		_xSize;
	int		_ySize;

	struct Line
	{
		XtString path;
		XtString xField;
		XtString yField;
		XtString tanIn;
		XtString tanOut;

		Xt::DomMonitor dom;

		int colour;

		double xMin;
		double xMax;

		double yMin;
		double yMax;

		bool generatedX;
		bool generatedY;
		bool hermite;

		bool isTemplate;

	public:
		Line();
	};

private:
	void SyncControls();
	void SetupLine(Line* line);
	Xt::DomField* GetLineX(SnChart::Line* line, int pointNo);
	Xt::DomField* GetLineY(SnChart::Line* line, int pointNo);
	Xt::DomField* GetInTangent(SnChart::Line* line, int pointNo);
	Xt::DomField* GetOutTangent(SnChart::Line* line, int pointNo);

private:
	Xt::DomMonitor	_dom;
	ScarabId		_idChart;
	Chart			_wndChart;
	SnChartData		_gd;

private:
	XtUt::List<Line>	_line;

friend class SnChartData;
};

//
// SnGothamWeather
//

class SnGothamWeather : public SnControl
{
public:
	SnGothamWeather(LfControl* field);
	virtual ~SnGothamWeather();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual void OnCommand(int id, int code);

private:
	struct Item
	{
		XtString name;
	};

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGothamWeather* node);
	static bool ComboCompare(const Item& lhs, const Item& rhs);

private:
	void SetDom(Xt::DomBlock* block);
	void SyncControls();
	void PopulateCombo();

private:
	Xt::DomMonitor _domCity;
	Xt::DomMonitor _domWeather;

private:
	ScarabId	_idCombo;
	ScarabWnd	_wndCombo;

private:
	XtUt::Array<Item> _items;
};

//
// SnNodeGraph
//

class SnNodeGraph : public SnControl
{
public:
	SnNodeGraph(LfControl* field);
	virtual ~SnNodeGraph();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnNodeGraph* node);

private:
	Xt::DomMonitor		_dom;
	ScarabId			_idGraph;
	ScarabDomNodeGraph	_wndGraph;

private:
	WinSize _panelSize;
};

//
// SnInfo
//

class SnInfo : public SnControl
{
public:
	SnInfo(LfControl* field);
	virtual ~SnInfo();

public:
	virtual void OnMeasure(HDC ic);
	virtual void OnPaint(HDC hdc, const WinRect& rcPaint);

private:
	XtString GetMessage();
};

//
// SnSequencer
//

#include "XtWin/Sequencer.h"

class SnSequencerHeader;

class SnSequencer : public SnControl
{
public:
	SnSequencer(LfControl* field);
	virtual ~SnSequencer();

public:
	virtual bool OnPopulate();
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);

public:
	void SetBody(Xt::DomElement* element, int index, LarvaField* field);

private:
	void SetDom(Xt::DomBlock* block);
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnSequencer* seq);

private:
	Xt::DomMonitor	_domRoot;
	Xt::DomMonitor	_domBody;
	LarvaField*		_bodyField;

private:
	SnSequencerHeader*	_header;
	ScarabNode*			_body;

friend class SnSequencerHeader;
};

//
// SnSequencerHeader
//

class SnSiItem;

class SnSequencerHeader : public ScarabNode
{
public:
	SnSequencerHeader(SnSequencer* sequencer);
	virtual ~SnSequencerHeader();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

private:
	void SetDom(Xt::DomBlock* block);
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnSequencerHeader* seq);

private:
	void SyncControls();
	SnSiItem* FindItem(XtId id);

private:
	LarvaField*		_domField;
	Xt::DomMonitor	_dom;
	int				_ignoreNotifications;

private:
	SnSequencer*	_sequencer;

private:
	ScarabId	_idSequencer;
	Sequencer	_wndSequencer;

private:
	XtUt::LinkList<SnSiItem> _allItems;

friend class SnSiItem;
friend struct SnSiBuilderType;
friend struct SnSiBuilderField;
friend struct SnSiBuilderContext;
};

//
// SnUserRef
//

class SnUserRef : public SnControl
{
public:
	SnUserRef(LfControl* field);
	virtual ~SnUserRef();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

public:
	virtual void OnCommand(int id, int code);
	virtual bool MeasureItem(int id, MEASUREITEMSTRUCT* mis);
	virtual bool DrawItem(int id, DRAWITEMSTRUCT* dis);

public:
	bool AddItem(LtStruct* type, Xt::DomComponent* component);

private:
	struct Item
	{
		XtString		id;
		XtString		label;
		LtStruct*		type;
		Xt::DomMonitor	dom;

	public:
		inline Item() :
			type(0)
		{
		}
	};

private:
	void SetDom(Xt::DomBlock* parentDom);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnUserRef* node);
	static bool ComboCompare(const Item* lhs, const Item* rhs);

private:
	void SyncControls();
	void PopulateCombo();
	void PopulateComboByRecord(Xt::DomRecord* record);
	void PopulateComboByType(Xt::DomRecord* baseRec,LtStruct* type);
	LtStruct* GetBaseType();

private:
	Xt::DomMonitor		_dom;
	Xt::DomMonitor		_watch;
	ScarabWnd			_wndCombo;
	ScarabId			_idCombo;
	XtUt::List<Item>	_items;
};

//
// SnDopeSheet
//

class SnDsKey;
class SnDsRow;

class SnDopeSheet : public SnControl
{
public:
	SnDopeSheet(LfControl* field);
	virtual ~SnDopeSheet();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnDopeSheet* node);

private:
	Xt::DomMonitor	_dom;
	ScarabId		_idDopeSheet;
	ScarabDopeSheet	_wndDopeSheet;

private:
	WinSize _panelSize;
};

//
// SnGradient
//

class SnGradient : public SnControl
{
public:
	SnGradient(LfControl* field);
	virtual ~SnGradient();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);
	virtual void OnPaint(HDC hdc, const WinRect& rcPaint);

private:
	void SetDom(Xt::DomBlock* block);
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGradient* swatch);

private:
	void SyncControls();

private:
	Xt::DomMonitor _dom;
};

//
// SnTimeline
//

class SnTlKey;
class SnTlRow;

class SnTimeline : public SnControl
{
public:
	SnTimeline(LfControl* field);
	virtual ~SnTimeline();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnTimeline* node);

private:
	Xt::DomMonitor	_dom;
	ScarabId		_idTimeline;
	ScarabTimeline	_wndTimeline;

private:
	WinSize _panelSize;
};

//
// SnColourSwatch
//

#include "XtWin/PropertySheet.h"

class SnPropertySheet : public SnControl
{
public:
	SnPropertySheet(LfControl* field);
	virtual ~SnPropertySheet();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

private:
	void SetDom(Xt::DomBlock* block);
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnPropertySheet* sheet);

private:
	void SyncControls();

private:
	Xt::DomMonitor	_dom;

private:
	ScarabId		_idSheet;
	PropertySheet	_wndSheet;
};

//
// SnGridView
//

class SnGridView : public SnControl
{
public:
	SnGridView(LfControl* field);
	virtual ~SnGridView();

public:
	virtual void OnBind(Xt::DomBlock* parentDom);
	virtual void OnBind(Xt::DomArray* parentDom, int index);
	virtual bool OnBuild();
	virtual void OnMeasure(HDC ic);
	virtual void OnLayout(ScarabLayoutContext* layoutContext);

public:
	virtual ScarabClipTarget ClipTarget();

private:
	void SetDom(Xt::DomBlock* block);

private:
	static void DomCallback(Xt::DomMonitor* monitor, Xt::DomEvent* event, SnGridView* node);

private:
	void AddColumnsForType(const XtString& path, const XtString& labelPrefix, LtStruct* type);

private:
	Xt::DomMonitor	_dom;
	ScarabId		_idGrid;
	GridView		_wndGrid;

private:
	WinSize _panelSize;

friend class SnGridContext;
friend class SnGridColumn;
friend class SnGridRow;
};

//
// ScarabBuild
//

bool ScarabBuild(ScarabNode* parentNode, LarvaField* field);
bool ScarabBuild(ScarabNode* parentNode, LtStruct* type, bool doFilter, const XtString& filter, const XtString& group);
bool ScarabBuild(ScarabNode* parentNode, LtStruct* type, bool doFilter, const XtString& filter);

#endif // SCARABIMPL_H