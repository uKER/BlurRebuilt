#ifndef EDITOR_SORCERY_HYPERGRAPH_PANEL_H
#define EDITOR_SORCERY_HYPERGRAPH_PANEL_H

class wxAuiManager;

class C_XtDomWriter;
class HyperViewForm;

namespace Editor
{

class C_SorceryHypergraphPanel : public wxWindow, public I_View
{
public:
    C_SorceryHypergraphPanel(
		wxWindow* pParent, 
		const wxWindowID id=wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER);

	~C_SorceryHypergraphPanel();

	void Shutdown();

	void OnCoreEnableViews(bool enable);
	void OnSelectionChanged();
	void OnNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);

	HyperViewForm*	GetHyperViewForm();

	void			OnShow(wxShowEvent& e);
	void			OnSize(wxSizeEvent& e);
	void			OnIdle(wxIdleEvent& e);

	void			Clear();

	virtual void	Refresh();

	void OnKeyDown(wxKeyEvent& e);
	bool MSWShouldPreProcessMessage(WXMSG* msg);
	bool MSWTranslateMessage(WXMSG* msg);

protected:
	class C_Slots : public sigslot::has_slots<>
	{
	public:
		C_Slots(C_SorceryHypergraphPanel* pForm);
		void Connect();
		void Disconnect();

		void OnCoreSelectionChangedSlot();
		void OnCoreEnableViewsSlot(bool enable);

		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);

		void OnNodeUpdated(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection*);
		void OnNodeDestroy(const EditorLib::I_DgNode& dgNode);

	private:
		C_SorceryHypergraphPanel* m_pWxScarabForm;
	};

	bool SetRecord(Xt::DomRecord* pRecord);
	bool SetDom(Xt::DomElement* pElement, LarvaType* pLarvaType, bool enabled);
	bool SetDom(Xt::DomBlock* pBlock, LarvaField* pLarvaType, bool enabled);
	Btl::String C_SorceryHypergraphPanel::GetRootXtNodeType(const EditorLib::I_DgNode* pDgNode) const;

	struct C_Flags
	{
		bool dirty		 : 1;
	};

	void			OnCoreDirtyViewsApplied(EditorLib::E_DirtyEventType e);

	C_Flags&		GetFlags();

	HyperViewForm*	m_pHyperViewForm;
	C_Slots			m_slots;
	C_Flags			m_flags;
	S32				m_viewEnabled;

	EditorLib::C_NodeId m_currentId;
	Btl::String			m_currentType;

	FRIEND_CLASS C_SorceryHypergraphPanel;
	FRIEND_CLASS C_SorceryHypergraphPanel::C_Slots;
};
}

#endif
