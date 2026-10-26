#ifndef XT_EDITOR_EXPLORER_ITEM_DATA_H
#define XT_EDITOR_EXPLORER_ITEM_DATA_H

#include "wx/treebase.h"

class C_Record;

enum E_ExplorerItemType
{
	E_ExplorerItemType_Unknown,

	E_ExplorerItemType_XtDb,
	E_ExplorerItemType_Record,
	E_ExplorerItemType_Filter
};

class C_ExplorerItemData : public wxTreeItemData
{
public:
	
	C_ExplorerItemData() :  m_Name("") {}
	C_ExplorerItemData(const wxString& name, const E_ExplorerItemType type, C_Record* pRecord) : m_Name(name), m_Type(type), m_pRecord(pRecord) {}
	virtual ~C_ExplorerItemData();

	void					SetName(const wxString& name);

	wxString				GetName() const { return m_Name; }
	E_ExplorerItemType		GetType() const { return m_Type; }
	C_Record*				GetRecord() { return m_pRecord; }
	
protected:
    wxString				m_Name;
	E_ExplorerItemType		m_Type;
	C_Record*				m_pRecord;
};

#endif // XT_EDITOR_EXPLORER_ITEM_DATA_H