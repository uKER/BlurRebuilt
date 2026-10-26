#ifndef XT_EDITOR_SOLUTION_EXPLORER_H
#define XT_EDITOR_SOLUTION_EXPLORER_H

#include "wx/treectrl.h"

#include "ExplorerItemData.h"

#define USE_GENERIC_TREECTRL 0

#if USE_GENERIC_TREECTRL
#pragma message("USE_GENERIC_TREECTRL")
#include "wx/generic/treectlg.h"
#ifndef wxTreeCtrl
#define wxTreeCtrl wxGenericTreeCtrl
#define sm_classwxTreeCtrl sm_classwxGenericTreeCtrl
#endif
#endif

class C_ExplorerItemData;
class C_Record;

class C_Explorer : public wxTreeCtrl
{
public:
    enum
    {
        TreeCtrlIcon_File,
        TreeCtrlIcon_FileSelected,
        TreeCtrlIcon_Folder,
        TreeCtrlIcon_FolderSelected,
        TreeCtrlIcon_FolderOpened
    };

    C_Explorer() { }
    C_Explorer(wxWindow *parent, const wxWindowID id,
               const wxPoint& pos, const wxSize& size,
               long style);
    virtual ~C_Explorer(){};

    void			OnBeginDrag(wxTreeEvent& event);
    void			OnBeginRDrag(wxTreeEvent& event);
    void			OnEndDrag(wxTreeEvent& event);
    void			OnBeginLabelEdit(wxTreeEvent& event);
    void			OnEndLabelEdit(wxTreeEvent& event);

	void			OnTreeSelectionChanged(wxTreeEvent& event);
	void			OnTreeItemActivated(wxTreeEvent& event);
    
	void			OnDeleteItem(wxTreeEvent& event);
    void			OnContextMenu(wxContextMenuEvent& event);
    
    void			OnTreeKeyDown(wxTreeEvent& event);

    void			CreateImageList(int size = 16);
    void			CreateButtonsImageList(int size = 11);

    void			Initialise();
	void			ExpandRoot();

    void			DoSortChildren(const wxTreeItemId& item, bool reverse = false)
					 { m_reverseSort = reverse; wxTreeCtrl::SortChildren(item); }
    void			DoEnsureVisible() { if (m_lastItem.IsOk()) EnsureVisible(m_lastItem); }

    void			DoToggleIcon(const wxTreeItemId& item);

    void			ShowMenu(wxTreeItemId id, const wxPoint& pt);

    int				ImageSize(void) const { return m_imageSize; }

    void			SetLastItem(wxTreeItemId id) { m_lastItem = id; }

	wxTreeItemId	GetItemFromIndex(wxTreeItemId parent, S32 i);
	wxTreeItemId	GetItemFromName(wxTreeItemId parent, const wxString& name);
	S32				GetItemIndex(wxTreeItemId id) const;

	wxTreeItemId	AddItem(C_Record* pRecord);	
	
protected:
	wxTreeItemId	AddItem(wxTreeItemId parent, const wxString& name, const E_ExplorerItemType type, C_Record* pRecord);
	virtual int		OnCompareItems(const wxTreeItemId& i1, const wxTreeItemId& i2);

  private:
	
    int					m_imageSize;               // current size of images
    bool				m_reverseSort;             // flag for OnCompareItems
    wxTreeItemId		m_lastItem,                // for OnEnsureVisible()
						m_draggedItem;             // item being dragged right now

	wxTreeItemId		m_currentId;

	wxTreeItemId		m_rootId;

	   // NB: due to an ugly wxMSW hack you _must_ use DECLARE_DYNAMIC_CLASS()
    //     if you want your overloaded OnCompareItems() to be called.
    //     OTOH, if you don't want it you may omit the next line - this will
    //     make default (alphabetical) sorting much faster under wxMSW.
    DECLARE_DYNAMIC_CLASS(C_Explorer)
    DECLARE_EVENT_TABLE()
};

#endif // XT_EDITOR_SOLUTION_EXPLORER_H



