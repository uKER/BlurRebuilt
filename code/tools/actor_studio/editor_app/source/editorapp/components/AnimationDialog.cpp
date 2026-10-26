#include "stdafx.h"
#include <wx/treectrl.h>
#include <wx/grid.h>

#include <algorithm>

#include "system/misc/StringUtil.h"

#include <xtwin/WndCore.h>
#include <xtwin/TimelineChart.h>
#include <xtwin/Scarab.h>
#include <XtWin/WndClasses.h>
#include <xtwin/TreeCtrl.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>

#include <EditorLib/GlobalSignals.h>

#include <EditorLib/Aftk/AftkRefHelpers.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneXtActorNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Xt/XtTypeDatabaseHelpers.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/IActorPropertyNode.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/ComplexRenderer.h>
#include <EditorApp/Editor/CameraConnection.h>

#include <EditorApp/Components/SplitterControl.h>
#include <EditorApp/Components/scene_graph/SceneGraphPanel.h>
#include <EditorApp/Components/scene_graph/SelectionSceneGraphPanel.h>

#include <EditorApp/Tasks/UITasks.h>

#include <UserNode/Props/Animation/DgAnimationPropNode.h>
#include <UserNode/Props/Animation/RenderAnimationPropNode.h>

#include <EditorApp/Components/AnimationDialog.h>

#include <Aftk/AftkEnum.h>
#include <Aftk/AftkTypedefs.h>
#include <Aftk/AftkBounds.h>
#include <Aftk/AftkMaths.h>
#include <Aftk/AftkEuler.h>

using namespace Editor;


class ScarabFormWrapper : public wxPanel
{
public:
	ScarabFormWrapper(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
	: wxPanel(parent, winid, pos, size, style, name)
	{
	}

	bool MSWShouldPreProcessMessage(WXMSG* msg)
	{
		if (msg->hwnd!=GetHWND() && msg->message == WM_KEYDOWN)	//keydown events from a non-wx child, ie. the scarabform
		{
			char className[256];
			if (::GetClassName(::GetFocus(), className, sizeof(className)))
			{
				//only allow msg pre-processing if event is from one of the scarabform container windows
				if (strnicmp(className,"WindowClassImpl",15)==0 ||
					strnicmp(className,"FrameWindowClassImpl",20)==0 ||
					strnicmp(className,"MdiWindowClassImpl",18)==0)
				{
					return true;
				}
			}
			return false;
		}
		return wxWindow::MSWShouldPreProcessMessage(msg);
	}

	bool MSWTranslateMessage(WXMSG* msg)
	{
		if (wxWindowMSW::MSWTranslateMessage(msg))
			return true;
		else
		{
			//send a new event to parent panel.
			//This is because ::IsDialog() will dispatch messages to the underlying ScarabForm
			//so we'll never see it again in wxWidgets land
			::SendMessage((HWND)GetHWND(), msg->message, msg->wParam, msg->lParam);
			return false;
		}
	}
};

class BitmapCheckButton : public wxBitmapButton
{
public:
	BitmapCheckButton(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER)
	: wxBitmapButton(parent, winid, wxNullBitmap, pos, size, style)
	, m_checked(true)
	{
	}

	~BitmapCheckButton()
	{
	}

	void SetBitmapChecked(wxBitmap& bitmap)
	{
		m_checkedImage = bitmap;
		Check(m_checked);
	}

	void SetBitmapUnchecked(wxBitmap& bitmap)
	{
		m_uncheckedImage = bitmap;
		Check(m_checked);
	}

	void Check(bool check)
	{
		m_checked = check;
		if (m_checked)
			SetBitmapLabel(m_checkedImage);
		else
			SetBitmapLabel(m_uncheckedImage);
	}

	bool IsChecked()
	{
		return m_checked;
	}

	void OnMouseEvent(wxMouseEvent& e)
	{
		if (e.LeftDown())
			Check(!m_checked);
	}

private:
	wxBitmap		m_checkedImage;
	wxBitmap		m_uncheckedImage;
	bool			m_checked;

	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(BitmapCheckButton, wxBitmapButton)
	 EVT_MOUSE_EVENTS(BitmapCheckButton::OnMouseEvent)
END_EVENT_TABLE()


C_AnimationPanel::C_Slots::C_Slots(C_AnimationPanel *pDialog)
: m_pDialog(pDialog)
{
}

void C_AnimationPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.connect(this, &Editor::C_AnimationPanel::C_Slots::OnCoreEnableViewsSlot);
	s.coreSelectionChanged.connect(this, &Editor::C_AnimationPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.dgNodeUpdated.connect(this, &Editor::C_AnimationPanel::C_Slots::OnDgNodeUpdatedSlot);
	s.dgNodeDestroy.connect(this, &Editor::C_AnimationPanel::C_Slots::OnDgNodeDestroyedSlot);
	s.levelOpening.connect(this, &Editor::C_AnimationPanel::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &Editor::C_AnimationPanel::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &Editor::C_AnimationPanel::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &Editor::C_AnimationPanel::C_Slots::OnLevelClosedSlot);
}

void C_AnimationPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.disconnect(this);
	s.coreSelectionChanged.disconnect(this);
	s.dgNodeUpdated.disconnect(this);
	s.dgNodeDestroy.disconnect(this);
	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);
}

void C_AnimationPanel::C_Slots::OnCoreEnableViewsSlot(bool enable)
{
	m_pDialog->OnCoreEnableViews(enable);
}

void C_AnimationPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pDialog->OnCoreSelectionChanged();
}

void C_AnimationPanel::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pDialog->OnDgNodeUpdated(dgNode);
}

void C_AnimationPanel::C_Slots::OnDgNodeDestroyedSlot(const EditorLib::I_DgNode& dgNode)
{
	m_pDialog->OnDgNodeDestroyed(dgNode);
}

void C_AnimationPanel::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelOpening(level);
}
			
void C_AnimationPanel::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelOpened(level);
}

void C_AnimationPanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelClosing(level);
}

void C_AnimationPanel::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelClosed(level);
}

IMPLEMENT_CLASS(C_AnimationPanel, wxPanel)

C_AnimationPanel::C_AnimationPanel(wxWindow *parent,  wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
, m_connected(false)
, m_pChart(0)
{
	SetBackgroundColour(*wxWHITE);
	m_pNoAnimationSizer = new wxBoxSizer( wxVERTICAL );
	
	wxStaticText* noAnimationLabel = new wxStaticText(this, wxID_ANY, "No animation selected, or no data assigned. \n\n Please select an animation actor, or assign a clip via the node properties panel.", wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE|wxST_NO_AUTORESIZE);

	m_pNoAnimationSizer->Add(noAnimationLabel, 1, wxEXPAND|wxALL, 5 );

	m_pEditSizer = new wxBoxSizer( wxVERTICAL );
	wxBoxSizer* pSizer1 = new wxBoxSizer( wxHORIZONTAL );
	wxBoxSizer* pSizer2 = new wxBoxSizer( wxHORIZONTAL );

	m_pChartPanel = new ScarabFormWrapper(this, wxID_ANY);
	m_pChartPanel->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(C_AnimationPanel::OnFrameChart), NULL, this);
	m_pChartPanel->Connect(wxEVT_SIZE, wxSizeEventHandler(C_AnimationPanel::OnSize), NULL, this);

	m_pTimeline = new wxSlider(this, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL);
	m_pTimelineValue = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_pTimelineValue->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnTimelineValueChanged ), NULL, this );

	m_pClipLength = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_pClipLength->SetToolTip( wxT("Clip Length (in seconds)") );
	m_pClipLength->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnClipLengthChange ), NULL, this );

	m_pQuantisation = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_pQuantisation->SetToolTip( wxT("Quantisation in 1/t") );
	m_pQuantisation->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnQuantisationChange ), NULL, this );

	m_pPlayButton = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pPlayButton->SetToolTip( wxT("Play the animation") );
	m_pPlayButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnRunAnimation ), NULL, this );
	m_pPlayButton->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnPlayButtonUpdate ), NULL, this );
	m_pPlayButton->SetBitmapLabel(wxBitmap(wxT("./icons/animation_play.png"), wxBITMAP_TYPE_ANY));
	m_pPlayButton->SetBitmapSelected(wxBitmap(wxT("./icons/animation_playdisabled.png"), wxBITMAP_TYPE_ANY));
	m_pPlayButton->SetBitmapDisabled(wxBitmap(wxT("./icons/animation_playdisabled.png"), wxBITMAP_TYPE_ANY));
	m_pPlayButton->SetBackgroundColour(*wxWHITE);

	m_pPauseButton = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pPauseButton->SetToolTip( wxT("Pause the animation") );
	m_pPauseButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnRunAnimation ), NULL, this );
	m_pPauseButton->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnPauseButtonUpdate ), NULL, this );
	m_pPauseButton->SetBitmapLabel(wxBitmap(wxT("./icons/animation_pause.png"), wxBITMAP_TYPE_ANY));
	m_pPauseButton->SetBitmapSelected(wxBitmap(wxT("./icons/animation_pausedisabled.png"), wxBITMAP_TYPE_ANY));
	m_pPauseButton->SetBitmapDisabled(wxBitmap(wxT("./icons/animation_pausedisabled.png"), wxBITMAP_TYPE_ANY));
	m_pPauseButton->SetBackgroundColour(*wxWHITE);

	m_pLoopButton = new BitmapCheckButton(this, wxID_ANY);
	m_pLoopButton->SetToolTip( wxT("Loop the animation") );
	m_pLoopButton->SetBitmapChecked(wxBitmap(wxT("./icons/animation_loop.png"), wxBITMAP_TYPE_ANY));
	m_pLoopButton->SetBitmapUnchecked(wxBitmap(wxT("./icons/animation_straight.png"), wxBITMAP_TYPE_ANY));
	m_pLoopButton->SetBackgroundColour(*wxWHITE);

	m_pShowDopeSheetButton = new wxBitmapButton( this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER );
	m_pShowDopeSheetButton->SetToolTip( wxT("Show the animation dope sheet") );
	m_pShowDopeSheetButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnShowDopeSheetButton ), NULL, this );
	m_pShowDopeSheetButton->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnShowDopeSheetButtonUpdate), NULL, this );
	m_pShowDopeSheetButton->SetBitmapLabel(wxBitmap(wxT("./icons/animation_dope.png"), wxBITMAP_TYPE_ANY));
	m_pShowDopeSheetButton->SetBitmapSelected(wxBitmap(wxT("./icons/animation_dopedisabled.png"), wxBITMAP_TYPE_ANY));
	m_pShowDopeSheetButton->SetBitmapDisabled(wxBitmap(wxT("./icons/animation_dopedisabled.png"), wxBITMAP_TYPE_ANY));
	m_pShowDopeSheetButton->SetBackgroundColour(*wxWHITE);

	m_pKeyGameCamButton = new wxBitmapButton(this, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxDefaultSize, wxNO_BORDER);
	m_pKeyGameCamButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnKeyFromGameCam ), NULL, this );
	m_pKeyGameCamButton->Connect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnKeyFromGameCamUpdate ), NULL, this );
	m_pKeyGameCamButton->SetBitmapLabel(wxBitmap(wxT("./icons/camera_setkeyselected.png"), wxBITMAP_TYPE_ANY));
	m_pKeyGameCamButton->SetBitmapSelected(wxBitmap(wxT("./icons/camera_setkey.png"), wxBITMAP_TYPE_ANY));
	m_pKeyGameCamButton->SetBitmapDisabled(wxBitmap(wxT("./icons/camera_setkey.png"), wxBITMAP_TYPE_ANY));
	m_pKeyGameCamButton->SetToolTip( wxT("Set keyframe from game camera") );
	m_pKeyGameCamButton->SetBackgroundColour(*wxWHITE);

	m_pAnimatedObjectPanel = new wxPanel(this, wxID_ANY);
	m_pAnimatedObjectPanel->SetMinSize( wxSize( -1,30 ) );

	Tasks::ShowDopeSheetDialog();

	m_pAnimatedObject = 0;

	pSizer1->Add( m_pClipLength, 0, wxALL, 2 );
	pSizer1->Add( m_pQuantisation, 0, wxALL, 2 );
	pSizer1->Add( m_pPlayButton, 0, wxALL, 2 );
	pSizer1->Add( m_pPauseButton, 0, wxALL, 2 );
	pSizer1->Add( m_pLoopButton, 0, wxALL, 2 );
	pSizer1->Add( m_pKeyGameCamButton, 0, wxALL, 2 );
	pSizer1->Add( m_pShowDopeSheetButton, 0, wxALL, 2 );
	pSizer1->Add( m_pAnimatedObjectPanel, 1, wxEXPAND|wxALL, 5 );

	m_pChannelTree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxNO_BORDER|wxTR_DEFAULT_STYLE|wxTR_HAS_BUTTONS|wxTR_EDIT_LABELS|wxTR_EXTENDED|wxTR_FULL_ROW_HIGHLIGHT|
		wxTR_HIDE_ROOT|wxTR_MULTIPLE|wxTR_TWIST_BUTTONS|wxHSCROLL|wxTAB_TRAVERSAL|wxVSCROLL);

	m_pChannelTree->Connect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( C_AnimationPanel::OnChannelSelectionChanged ), NULL, this );
	m_pChannelTree->Connect( wxEVT_COMMAND_TREE_ITEM_MENU, wxTreeEventHandler( C_AnimationPanel::OnContextMenu ), NULL, this );
	m_pChannelTree->Connect( wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler( C_AnimationPanel::OnNodeRename ), NULL, this );
	m_pChannelTree->Connect( wxEVT_COMMAND_TREE_END_LABEL_EDIT, wxTreeEventHandler( C_AnimationPanel::OnNodeRename ), NULL, this );

	wxTreeItemId rootId = m_pChannelTree->AddRoot(wxT("Root"), -1, -1, 0);

	m_pChannelGrid = new wxGrid(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0);

	m_pChannelGrid->CreateGrid(0,1);
	m_pChannelGrid->SetColLabelValue(0, "Value");
	m_pChannelGrid->EnableEditing(true);
	m_pChannelGrid->EnableGridLines(true);
	m_pChannelGrid->EnableDragGridSize(true);
	m_pChannelGrid->SetMargins(0, 0);
	m_pChannelGrid->EnableDragColMove(false);
	m_pChannelGrid->EnableDragColSize(true);
	m_pChannelGrid->SetColLabelSize(16);
	m_pChannelGrid->SetColLabelAlignment(wxALIGN_CENTRE, wxALIGN_CENTRE);
	m_pChannelGrid->EnableDragRowSize(false);
	m_pChannelGrid->SetRowLabelSize(300);
	m_pChannelGrid->SetRowLabelAlignment(wxALIGN_RIGHT, wxALIGN_CENTRE);
	m_pChannelGrid->SetDefaultCellAlignment(wxALIGN_LEFT, wxALIGN_TOP);
	m_pChannelGrid->SetLabelBackgroundColour(wxColour(236,236,236));

	m_pChannelGrid->Connect(wxEVT_GRID_CELL_CHANGE, wxGridEventHandler(C_AnimationPanel::OnCellChanged), NULL, this);
	m_pChannelGrid->Connect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( C_AnimationPanel::OnCellRightClick ), NULL, this );

	wxSplitterCtrl* pSplitter = new wxSplitterCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER|wxSP_SASH_BORDERS);
	pSplitter->Split(E_SplitterLayout_3Bottom, m_pChartPanel, m_pChannelTree, m_pChannelGrid);
	pSplitter->SetSplitterProportion(3000);
	m_pEditSizer->Add(pSplitter, 1, wxEXPAND, 0);

	pSizer2->Add(m_pTimeline, 1, wxEXPAND, 5);
	pSizer2->Add(m_pTimelineValue, 0, wxALL, 2);
	m_pEditSizer->Add(pSizer2, 0, wxEXPAND, 5);
	m_pEditSizer->Add(pSizer1, 0, wxEXPAND, 5);

	ShowEditControls(false);

	m_Flags.pendingSync = false;
	m_Flags.dirty = false;
	m_Flags.viewEnabled = true;

	m_Slots.Connect();
	m_connected = true;

	m_pChartCallbacks = new AnimationChartCallbacks(this);

	Connect(wxID_ANY, wxEVT_SIZE, wxSizeEventHandler(C_AnimationPanel::OnSize));
	Connect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_AnimationPanel::OnIdle));
	
	m_pTimeline->Connect( wxEVT_SCROLL_TOP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	m_pTimeline->Connect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
	
	m_pAnimationTimer = new AnimationTimer(this);
}

C_AnimationPanel::~C_AnimationPanel()
{
	Shutdown();
}

void C_AnimationPanel::Shutdown()
{
	if (m_connected)
	{
		SetSizer(0, false);

		Disconnect(wxID_ANY, wxEVT_SIZE, wxSizeEventHandler(C_AnimationPanel::OnSize));
		Disconnect(wxID_ANY, wxEVT_IDLE, wxIdleEventHandler(C_AnimationPanel::OnIdle));

		this->Disconnect(wxEVT_CHAR, wxCharEventHandler( C_AnimationPanel::OnFrameChart ), NULL, this );

		m_pChartPanel->Disconnect(wxEVT_SIZE, wxSizeEventHandler(C_AnimationPanel::OnSize), NULL, this);

		m_pTimeline->Disconnect( wxEVT_SCROLL_TOP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_BOTTOM, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_LINEUP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_PAGEUP, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );
		m_pTimeline->Disconnect( wxEVT_SCROLL_CHANGED, wxScrollEventHandler( C_AnimationPanel::OnTimelineScrub ), NULL, this );

		m_pTimelineValue->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnTimelineValueChanged ), NULL, this );

		m_pChannelGrid->Disconnect( wxEVT_GRID_CELL_RIGHT_CLICK, wxGridEventHandler( C_AnimationPanel::OnCellRightClick ), NULL, this );
		m_pChannelGrid->Disconnect( wxEVT_GRID_CELL_CHANGE, wxGridEventHandler(C_AnimationPanel::OnCellChanged), NULL, this );

		m_pChannelTree->Disconnect( wxEVT_COMMAND_TREE_SEL_CHANGED, wxTreeEventHandler( C_AnimationPanel::OnChannelSelectionChanged ), NULL, this );
		m_pChannelTree->Disconnect( wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT, wxTreeEventHandler( C_AnimationPanel::OnNodeRename ), NULL, this );
		m_pChannelTree->Disconnect( wxEVT_COMMAND_TREE_END_LABEL_EDIT, wxTreeEventHandler( C_AnimationPanel::OnNodeRename ), NULL, this );

		m_pClipLength->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnClipLengthChange ), NULL, this );
		m_pQuantisation->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler( C_AnimationPanel::OnQuantisationChange ), NULL, this );	
		m_pPlayButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnRunAnimation ), NULL, this );
		m_pPlayButton->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnPlayButtonUpdate ), NULL, this );
		m_pPauseButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnRunAnimation ), NULL, this );
		m_pPauseButton->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnPauseButtonUpdate ), NULL, this );

		m_pShowDopeSheetButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnShowDopeSheetButton ), NULL, this );
		m_pShowDopeSheetButton->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnShowDopeSheetButtonUpdate ), NULL, this );

		m_pKeyGameCamButton->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler( C_AnimationPanel::OnKeyFromGameCam ), NULL, this );
		m_pKeyGameCamButton->Disconnect(wxEVT_UPDATE_UI, wxUpdateUIEventHandler( C_AnimationPanel::OnKeyFromGameCamUpdate ), NULL, this );

		m_Slots.Disconnect();
		m_connected = false;

		SAFE_DELETE_PTR(m_pEditSizer);
		SAFE_DELETE_PTR(m_pNoAnimationSizer);
		SAFE_DELETE_PTR(m_pChart);
		SAFE_DELETE_PTR(m_pChartCallbacks);
		SAFE_DELETE_PTR(m_pAnimatedObject);
		SAFE_DELETE_PTR(m_pAnimatedObjectPanel);
		SAFE_DELETE_PTR(m_pAnimationTimer);
	}
}

void C_AnimationPanel::OnIdle(wxIdleEvent& e)
{
	if (m_Flags.pendingSync)
		return;

	if (m_Flags.dirty && m_Flags.viewEnabled)
	{
		if (m_Flags.dataDirty)
		{
			m_Flags.dataDirty = false;
			UpdateItem();
		}
		Refresh();
		
		m_Flags.dirty = false;
	}
}

void C_AnimationPanel::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode)
{
	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode || pNode->GetId() == dgNode.GetId() || pNode->GetAnimationId() == dgNode.GetId())
	{
		m_Flags.dirty = true;
		m_Flags.dataDirty = true;
	}
}

void C_AnimationPanel::OnDgNodeDestroyed(const EditorLib::I_DgNode& dgNode)
{
	m_Flags.dirty = true;

	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (pNode)
		if (pNode->GetId() == dgNode.GetId())
			SelectItem(EditorLib::C_NodeId());
}

void C_AnimationPanel::OnCoreEnableViews(bool enable)
{
	m_Flags.viewEnabled = enable;
}

void C_AnimationPanel::OnCoreSelectionChanged()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	if (selection.GetItems().Size() != 1)
	{
		EditorLib::C_NodeId invalidId;
		SelectItem(invalidId);
		return;
	}

	EditorLib::C_NodeId selectedItem = selection.GetItems()[0].id;
	EditorLib::I_DgNode *pNode =  EditorLib::C_Core::Instance().GetDgManager().FindNodeById(selectedItem);

	EditorLib::C_XtObject* pXtObject;
	if (pNode)
		pXtObject = pNode->AsXtObject();
	else
		pXtObject = 0;
		
	if (!pXtObject || !pXtObject->GetXtData())
	{
		EditorLib::C_NodeId invalidId;
		SelectItem(invalidId);
		return;
	}
	else
	{
		SelectItem(selectedItem);
	}
}

void C_AnimationPanel::OnLevelOpening(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = true;
}

void C_AnimationPanel::OnLevelOpened(const EditorLib::I_Level& level)
{
	m_Flags.pendingSync = false;
}

void C_AnimationPanel::OnLevelClosing(const EditorLib::I_Level& level)
{
	EditorLib::C_NodeId invalidId;
	SelectItem(invalidId);
	m_Flags.pendingSync = true;
}

void C_AnimationPanel::OnLevelClosed(const EditorLib::I_Level& level)
{
	assert(m_Flags.pendingSync);
	m_Flags.pendingSync = false;
}

C_AnimationPanel::AnimationTimer::AnimationTimer(C_AnimationPanel* panel)
{
	m_pDialog = panel;
}

void C_AnimationPanel::AnimationTimer::Notify()
{
	m_pDialog->OnAnimationTimer();
}

C_AnimationPanel::AnimationChartCallbacks::AnimationChartCallbacks(C_AnimationPanel* panel)
{
	m_pDialog = panel;
}

int C_AnimationPanel::AnimationChartCallbacks::GetLineCount(void)
{
	return m_pDialog->m_SelectedCurves.Size();
}

double C_AnimationPanel::AnimationChartCallbacks::GetTimelineValue(void)
{
	return m_pDialog->GetTimelineValue();
}

void C_AnimationPanel::AnimationChartCallbacks::GetLineInfo(int line, TimelineChartLine *out)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode* pNode = m_pDialog->GetNode();

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	out->count = curve->pCurveData->GetNumKnots();
	out->isHermite = true;
	out->xMin = m_pDialog->m_chartBounds.X();
	out->xMax = m_pDialog->m_chartBounds.Y();
	out->yMin = m_pDialog->m_chartBounds.Z();
	out->yMax = m_pDialog->m_chartBounds.W();
	out->timelineMin = 0;
	out->timelineMax = pNode->GetClipLength();
	Maths::RGBA colour = curve->colour;
	out->colour = RGB((colour&0xFF0000)>>16, (colour&0xFF00)>>8, colour&0xFF);
}

void C_AnimationPanel::AnimationChartCallbacks::GetLineValue(int line, int index, ChartPoint *out)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	out->x = (F32)curve->pCurveData->GetKnots()[index].u;
	out->y = curve->pCurveData->GetKnots()[index].value;
}

void C_AnimationPanel::AnimationChartCallbacks::SetLineValue(int line, int index, const ChartPoint *in)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	curve->pCurveData->GetKnots()[index].u = in->x;
	curve->pCurveData->GetKnots()[index].value = in->y;
	curve->Process();
	curve->Save();

	m_pDialog->m_Flags.dataDirty = true;
	m_pDialog->m_Flags.dirty = true;
}

void C_AnimationPanel::AnimationChartCallbacks::DeleteKey(int line, int index)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode* pNode = m_pDialog->GetNode();

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	for (int i=0; i<m_pDialog->m_selectedItems.Size();)
	{
		if (m_pDialog->m_selectedItems[i].first == line)
		{
			if (m_pDialog->m_selectedItems[i].second == index)
			{
				Btl::Vector<Btl::Pair<U32, U32> >::Iterator it = m_pDialog->m_selectedItems.Begin();
				it = it + i;
				m_pDialog->m_selectedItems.Erase(it);
			}
			else if (m_pDialog->m_selectedItems[i].second > (U32)index)
			{
				m_pDialog->m_selectedItems[i].second--;
				i++;
			}
			else
				i++;
				
		} else
			i++;
	}

	curve->RemoveKeyFrame(index);
	curve->Process();
	curve->Save();
	
	m_pDialog->m_Flags.dataDirty = true;
	m_pDialog->m_Flags.dirty = true;
}

void C_AnimationPanel::AnimationChartCallbacks::SmoothTangents(int line, int index)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode* pNode = m_pDialog->GetNode();

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	curve->SmoothTangents(index);
	curve->Process();
	curve->Save();
	
	m_pDialog->m_Flags.dataDirty = true;
	m_pDialog->m_Flags.dirty = true;
}

void C_AnimationPanel::AnimationChartCallbacks::GetInTangentValue(int line, int index, double *in)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	*in = Maths::RadToDeg(Maths::ArcTan(curve->pCurveData->GetKnots()[index].tangentIn));
}

void C_AnimationPanel::AnimationChartCallbacks::GetOutTangentValue(int line, int index, double *out)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	*out = Maths::RadToDeg(Maths::ArcTan(curve->pCurveData->GetKnots()[index].tangentOut));
}

void C_AnimationPanel::AnimationChartCallbacks::SetInTangentValue(int line, int index, double in)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	curve->pCurveData->GetKnots()[index].tangentIn = Maths::Tan(Maths::DegToRad(in));
	curve->Process();
	curve->Save();

	m_pDialog->m_Flags.dataDirty = true;
	m_pDialog->m_Flags.dirty = true;
}

void C_AnimationPanel::AnimationChartCallbacks::SetOutTangentValue(int line, int index, double out)
{
	if (line >= m_pDialog->m_SelectedCurves.Size())
		return;

	EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_pDialog->m_SelectedCurves[line];

	curve->pCurveData->GetKnots()[index].tangentOut = Maths::Tan(Maths::DegToRad(out));
	curve->Process();
	curve->Save();

	m_pDialog->m_Flags.dataDirty = true;
	m_pDialog->m_Flags.dirty = true;
}

int C_AnimationPanel::AnimationChartCallbacks::GetNumSelectedComponents()
{
	return m_pDialog->m_selectedItems.Size();
}

void C_AnimationPanel::AnimationChartCallbacks::GetSelectedComponent(int index, int *outLine, int *outIndex)
{
	*outLine = m_pDialog->m_selectedItems[index].first;
	*outIndex = m_pDialog->m_selectedItems[index].second;
}

void C_AnimationPanel::AnimationChartCallbacks::SetSelectedComponent(int line, int index, bool selected)
{
	for (Btl::Vector<Btl::Pair<U32, U32> >::Iterator it = m_pDialog->m_selectedItems.Begin(); it != m_pDialog->m_selectedItems.End(); ++it)
	{
		if (it->first == line && it->second == index)
		{
			if (!selected)
				m_pDialog->m_selectedItems.Erase(it);

			return;
		}
	}

	if (selected)
		m_pDialog->m_selectedItems.PushBack(Btl::MakePair((U32)line, (U32)index));
}

void C_AnimationPanel::AnimationChartCallbacks::ClearSelection()
{
	m_pDialog->m_selectedItems.Clear();
}

void C_AnimationPanel::UpdateItem()
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();
	C_RenderAnimationPropNode* pRenderNode = (C_RenderAnimationPropNode*)
		C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(node);

	if (!pRenderNode || node->GetXtData() == 0)
	{
		m_pChannelTree->DeleteChildren(m_pChannelTree->GetRootItem());
		m_SelectedCurves.Clear();
		if (m_pChannelGrid->GetNumberRows())
			m_pChannelGrid->DeleteRows(0, m_pChannelGrid->GetNumberRows());
		SAFE_DELETE_PTR(m_pChart);
		SAFE_DELETE_PTR(m_pAnimatedObject);

		ShowEditControls(false);
		return;
	} else {
		ShowEditControls(true);

		if (!m_pChart)
		{
			m_pChart = new TimelineChart();
			m_pChart->Create((HWND)m_pChartPanel->GetHandle());
		}
	}

	m_Flags.pendingSelectionSync = false;

	m_pChannelTree->Freeze();

	Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>& objects = node->GetObjects();

	wxTreeItemId rootNode = m_pChannelTree->GetRootItem();
	wxTreeItemIdValue cookie;
	wxTreeItemId objectsId = m_pChannelTree->GetFirstChild(rootNode, cookie);
	wxTreeItemId temp;

	if (!objectsId.IsOk())
	{
		m_Flags.pendingSelectionSync = true;
		objectsId = m_pChannelTree->AppendItem(m_pChannelTree->GetRootItem(), "Objects", -1, -1, new C_AnimationTreeItemData(C_AnimationTreeItemData::E_TreeItemDataType_ObjectsRoot));
		m_pChannelTree->SelectItem(objectsId, true);
	}

	// Remove deleted objects from the tree
	temp = m_pChannelTree->GetFirstChild(objectsId, cookie);
	while (temp.IsOk())
	{
		bool found = false;
		EditorLib::C_DgAnimationPropNode::Object* treeObject = ((C_AnimationTreeItemData*)m_pChannelTree->GetItemData(temp))->GetObject();
		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
		{
			if (treeObject == *object)
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			m_Flags.pendingSelectionSync = true;
			m_pChannelTree->Delete(temp);
			temp = m_pChannelTree->GetFirstChild(objectsId, cookie);
		} else {
			temp = m_pChannelTree->GetNextChild(objectsId, cookie);
		}
	}
	
	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = objects.Begin(); object != objects.End(); ++object)
	{
		wxTreeItemId objectId;
		
		// Find the object in the tree, and add it if it doesn't exist
		temp = m_pChannelTree->GetFirstChild(objectsId, cookie);
		while (temp.IsOk())
		{
			if (((C_AnimationTreeItemData*)m_pChannelTree->GetItemData(temp))->GetObject() == *object)
			{
				objectId = temp;
				break;
			}

			temp = m_pChannelTree->GetNextChild(objectsId, cookie);
		}

		if (!objectId.IsOk())
		{
			m_Flags.pendingSelectionSync = true;
			objectId = m_pChannelTree->AppendItem(objectsId, (*object)->GetName().Chars(), -1, -1, new C_AnimationTreeItemData(*object));
		}

		if ((*object)->GetName() != m_pChannelTree->GetItemText(objectId))
		{
			m_Flags.pendingSelectionSync = true;
			m_pChannelTree->SetItemText(objectId, (*object)->GetName().Chars());
		}

		Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>& channels = (*object)->GetChannels();

		// Remove deleted channels from the tree
		temp = m_pChannelTree->GetFirstChild(objectId, cookie);
		while (temp.IsOk())
		{
			bool found = false;
			EditorLib::C_DgAnimationPropNode::Channel* treeChannel = ((C_AnimationTreeItemData*)m_pChannelTree->GetItemData(temp))->GetChannel();
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = channels.Begin(); channel != channels.End(); ++channel)
			{
				if (treeChannel == *channel)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				m_Flags.pendingSelectionSync = true;
				m_pChannelTree->Delete(temp);
				temp = m_pChannelTree->GetFirstChild(objectId, cookie);
			} else {
				temp = m_pChannelTree->GetNextChild(objectId, cookie);
			}
		}

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = channels.Begin(); channel != channels.End(); ++channel)
		{
			wxTreeItemId channelId;

			// Find the channel in the tree, and add it if it doesn't exist
			temp = m_pChannelTree->GetFirstChild(objectId, cookie);
			while (temp.IsOk())
			{
				if (((C_AnimationTreeItemData*)m_pChannelTree->GetItemData(temp))->GetChannel() == *channel)
				{
					channelId = temp;
					break;
				}

				temp = m_pChannelTree->GetNextChild(objectId, cookie);
			}

			if (!channelId.IsOk())
			{
				m_Flags.pendingSelectionSync = true;

				channelId = m_pChannelTree->AppendItem(objectId, "<Channel>", -1, -1, new C_AnimationTreeItemData(*channel));

				Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>& curves = (*channel)->GetCurves();

				for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = curves.Begin(); curve != curves.End(); ++curve)
				{
					m_pChannelTree->AppendItem(channelId, wxT((*curve)->name.Chars()), -1, -1, new C_AnimationTreeItemData(*curve));
				}
			}

			Btl::String name;
			if ((*channel)->hasNameField)
				name = Btl::String("'") + (*channel)->name + Btl::String("' (") + (*channel)->xtType->label.Text() + Btl::String(")");
			else
				name = (*channel)->xtType->label.Text();

			if (name != m_pChannelTree->GetItemText(channelId))
			{
				m_Flags.pendingSelectionSync = true;
				m_pChannelTree->SetItemText(channelId, name.Chars());
			}
		}

		m_pChannelTree->Expand(objectId);
	}

	m_pChannelTree->Expand(objectsId);

	m_pChannelTree->Thaw();

	if (m_Flags.pendingSelectionSync)
	{
		m_Flags.pendingSelectionSync = false;
		SyncSelectionToTree();
	}
		
	pRenderNode->RegisterPositionCallback(Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>(this, &C_AnimationPanel::UpdateGridPositionValues));
	pRenderNode->RegisterRotationCallback(Ut::Delegate<void(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3&)>(this, &C_AnimationPanel::UpdateGridRotationValues));
	
	XtLoad();

	m_pTimeline->SetRange(0, node->GetClipLength()*node->GetQuantisation());
	m_pTimeline->SetValue(((C_RenderAnimationPropNode*)pRenderNode)->GetTimelineValue()*node->GetQuantisation());
	m_pTimelineValue->SetValue(wxString::Format("%lf", ((C_RenderAnimationPropNode*)pRenderNode)->GetTimelineValue()));

	if (m_pChart)
	{
		WinRect rcClient((HWND)m_pChartPanel->GetHandle());
		SetWindowPos(*m_pChart, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		m_pChart->SetCallbacks(m_pChartCallbacks);
		RedrawWindow(*m_pChart, 0, 0, RDW_INVALIDATE);
	}
}

void C_AnimationPanel::SelectItem(const EditorLib::C_NodeId& nodeId)
{
	m_Flags.selectedItemDirty = true;

	if (m_SelectedItem != nodeId)
	{
		m_SelectedItem = nodeId;

		m_pPlayButton->SetLabel("Play");
		m_pAnimationTimer->Stop();

		m_pChannelTree->DeleteChildren(m_pChannelTree->GetRootItem());

		m_SelectedCurves.Clear();
		if (m_pChannelGrid->GetNumberRows())
			m_pChannelGrid->DeleteRows(0, m_pChannelGrid->GetNumberRows());
		
		SAFE_DELETE_PTR(m_pAnimatedObject);

		if (!nodeId.IsValid())
		{
			ShowEditControls(false);
			SAFE_DELETE_PTR(m_pChart);
			return;
		}

		if (!m_pChart)
		{
			m_pChart = new TimelineChart();
			m_pChart->Create((HWND)m_pChartPanel->GetHandle());
		}

		UpdateItem();

		CalculateChartExtents();
	} else {
		UpdateItem();
	}
}

void C_AnimationPanel::ShowEditControls(bool show)
{
	if (show)
		SetSizer(m_pEditSizer, false);
	else
		SetSizer(m_pNoAnimationSizer, false);

	for (size_t i=0; i<m_pNoAnimationSizer->GetChildren().GetCount(); i++)
		m_pNoAnimationSizer->Show(i, !show);
	for (size_t i=0; i<m_pEditSizer->GetChildren().GetCount(); i++)
		m_pEditSizer->Show(i, show);

	m_pNoAnimationSizer->Layout();
	m_pEditSizer->Layout();
	Layout();
}

void C_AnimationPanel::OnSize(wxSizeEvent& e)
{
	__super::OnSize(e);

	if (m_pChart)
	{
		WinRect rcClient((HWND)m_pChartPanel->GetHandle());
		SetWindowPos(*m_pChart, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		RedrawWindow(*m_pChart, 0, 0, RDW_INVALIDATE);
	}

	if (m_pAnimatedObject)
	{
		WinRect rcClient((HWND)m_pAnimatedObjectPanel->GetHandle());
		SetWindowPos(*m_pAnimatedObject, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		RedrawWindow(*m_pAnimatedObject, 0, 0, RDW_INVALIDATE);
	}
}

void C_AnimationPanel::OnTimelineScrub(wxScrollEvent& e)
{
	m_Flags.dirty = true;
	m_Flags.timelineDirty = true;
	m_Flags.timelineScrubDirty = true;
}

void C_AnimationPanel::OnTimelineValueChanged(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode)
		return;

	double doubleValue;

	if (m_pTimelineValue->GetValue().ToDouble(&doubleValue))
	{
		m_pTimeline->SetValue(doubleValue*pNode->GetQuantisation());
		m_Flags.dirty = true;
		m_Flags.timelineDirty = true;
	}
}

void C_AnimationPanel::OnCellChanged(wxGridEvent& e)
{
	e.GetRow();

	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode)
		return;

	C_RenderAnimationPropNode* pRenderNode = (C_RenderAnimationPropNode*)
		C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pNode);

	if (!pRenderNode)
		return;

	

	wxString cellValueString = m_pChannelGrid->GetCellValue(e.GetRow(), 0);
	cellValueString.Trim();
	double cellValueDouble;

	if (cellValueString.ToDouble(&cellValueDouble))
	{
		pRenderNode->SetAbsoluteTransform(m_SelectedCurves[e.GetRow()], cellValueDouble);
		m_Flags.dirty = true;
	}
}

void C_AnimationPanel::OnCellRightClick(wxGridEvent& e)
{
	wxPoint pt = e.GetPosition();
    wxMenu menu;

	m_cellRowSelected = e.GetRow();

	int setKeyframeId = menu.Append(wxID_ANY, wxString::Format(wxT("Set Keyframe")))->GetId();
	
	Connect(setKeyframeId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnSetKeyframe));

    m_pChannelGrid->PopupMenu(&menu, pt);

	Disconnect(setKeyframeId, wxEVT_COMMAND_MENU_SELECTED);
}

void C_AnimationPanel::OnSetKeyframe(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode)
		return;

	AftkRender::C_SceneGraphNode* pRenderNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pNode);
	if (!pRenderNode)
		return;

	Btl::Vector<U32> selectedRows;

	wxGridCellCoordsArray cells = m_pChannelGrid->GetSelectedCells();

	for (size_t i=0; i < cells.Count(); i++)
	{
		selectedRows.PushBack(cells[i].GetRow());
	}

	wxArrayInt rows = m_pChannelGrid->GetSelectedRows();

	for (size_t i=0; i < rows.Count(); i++)
	{
		selectedRows.PushBack(rows[i]);
	}

	wxGridCellCoordsArray topLeft = m_pChannelGrid->GetSelectionBlockTopLeft();
	wxGridCellCoordsArray bottomRight = m_pChannelGrid->GetSelectionBlockBottomRight();

	for (size_t i=0; i<topLeft.Count(); i++)
	{
		for (int j=topLeft[i].GetRow(); j<=bottomRight[i].GetRow(); j++)
		{
			selectedRows.PushBack(j);
		}
	}

	if (!selectedRows.Size())
		selectedRows.PushBack(m_cellRowSelected);

	for (int i=0; i<selectedRows.Size(); i++)
	{
		wxString value = m_pChannelGrid->GetCellValue(selectedRows[i], 0);
		
		double doubleValue;
		if (!value.ToDouble(&doubleValue))
			return;

		EditorLib::C_DgAnimationPropNode::AnimationCurve* curve = m_SelectedCurves[selectedRows[i]];
		curve->SetOffset(0);
		U32 newKeyIndex;
		curve->AddKeyFrame(GetTimelineValue(), doubleValue, 0, 0, &newKeyIndex);
		curve->SmoothTangents(newKeyIndex);

		m_Flags.dirty = true;
	}
}

void C_AnimationPanel::OnAnimationTimer()
{
	if (m_pTimeline->GetValue() >= m_pTimeline->GetMax() && !m_pLoopButton->IsChecked())
		m_pAnimationTimer->Stop();
	else
	{
		m_Flags.timelineDirty = true;
		m_Flags.timelineScrubDirty = true;
		m_pTimeline->SetValue((m_pTimeline->GetValue()+1)%(m_pTimeline->GetMax()+1));
		::wxWakeUpIdle();
	}
	Refresh();
}

void C_AnimationPanel::Refresh()
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();
	if (!node)
		return;

	EvaluateGridValues();

	UpdateProperty();

	if (m_pChart)
	{
		RedrawWindow(*m_pChart, 0, 0, RDW_INVALIDATE);
	}

	m_pChannelGrid->ForceRefresh();

	if (m_pAnimationTimer->IsRunning())
		C_RenderCore::Instance().RefreshAllViewports();
	else
		C_RenderCore::Instance().DirtyAllViewports();

	m_Flags.dirty = false;
}

void C_AnimationPanel::OnRunAnimation(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	if (m_pAnimationTimer->IsRunning())
		m_pAnimationTimer->Stop();
	else
	{
		if (m_pTimeline->GetValue() >= m_pTimeline->GetMax() && !m_pLoopButton->IsChecked())
			m_pTimeline->SetValue(m_pTimeline->GetMin());

		m_pAnimationTimer->Start(1000/30);
	}
}

void C_AnimationPanel::OnPlayButtonUpdate(wxUpdateUIEvent& e)
{
	e.Enable(GetNode() && !m_pAnimationTimer->IsRunning());
}

void C_AnimationPanel::OnPauseButtonUpdate(wxUpdateUIEvent& e)
{
	e.Enable(GetNode() && m_pAnimationTimer->IsRunning());
}

void C_AnimationPanel::OnShowDopeSheetButton(wxCommandEvent& e)
{
	Tasks::ShowDopeSheetDialog();
}

void C_AnimationPanel::OnShowDopeSheetButtonUpdate(wxUpdateUIEvent& e)
{
	e.Enable(GetNode() != 0);
}

void C_AnimationPanel::SyncSelectionToTree()
{
	m_SelectedCurves.Clear();

	if (m_pChannelGrid->GetNumberRows())
		m_pChannelGrid->DeleteRows(0, m_pChannelGrid->GetNumberRows());
	
	wxArrayTreeItemIds selection;
	m_pChannelTree->GetSelections(selection);

	for (size_t i=0; i<selection.Count(); i++)
	{
		wxTreeEvent e;
		e.SetItem(selection[i]);
		OnChannelSelectionChanged(e);
	}
}

void C_AnimationPanel::OnChannelSelectionChanged(wxTreeEvent& e)
{
	if (m_Flags.pendingSelectionSync)
		return;

	wxArrayInt selection;

	wxTreeItemId node = e.GetItem();

	if (!node.IsOk())
		return;

	Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*> curves;

	C_AnimationTreeItemData* itemData = (C_AnimationTreeItemData*)m_pChannelTree->GetItemData(node);
	switch (itemData->GetType())
	{
	case (C_AnimationTreeItemData::E_TreeItemDataType_ObjectsRoot):
		{
			EditorLib::C_DgAnimationPropNode* pNode = GetNode();

			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Object*>::Iterator object = pNode->GetObjects().Begin(); object != pNode->GetObjects().End(); ++object)
			{
				for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = (*object)->GetChannels().Begin(); channel != (*object)->GetChannels().End(); ++channel)
				{
					for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
					{
						curves.PushBack(*curve);
					}
				}
			}
			break;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Object):
		{
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = itemData->GetObject()->GetChannels().Begin(); channel != itemData->GetObject()->GetChannels().End(); ++channel)
			{
				for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = (*channel)->GetCurves().Begin(); curve != (*channel)->GetCurves().End(); ++curve)
				{
					curves.PushBack(*curve);
				}
			}
			break;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Channel):
		{
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = itemData->GetChannel()->GetCurves().Begin(); curve != itemData->GetChannel()->GetCurves().End(); ++curve)
			{
				curves.PushBack(*curve);
			}
			break;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Curve):
		{
			curves.PushBack(itemData->GetCurve());
			break;
		}
	default:
		return;
	}

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve = curves.Begin(); curve != curves.End(); ++curve)
	{
		if (m_pChannelTree->IsSelected(node))
		{
			bool curveAlreadySelected = false;
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator selectedCurve = m_SelectedCurves.Begin(); selectedCurve != m_SelectedCurves.End(); ++selectedCurve)
			{
				if (*selectedCurve == *curve)
				{
					curveAlreadySelected = true;
					break;
				}
			}
			if (!curveAlreadySelected)
				m_SelectedCurves.PushBack(*curve);
		}
		else 
		{
			for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator selectedCurve = m_SelectedCurves.Begin(); selectedCurve != m_SelectedCurves.End(); ++selectedCurve)
			{
				if (*selectedCurve == *curve)
				{
					m_SelectedCurves.Erase(selectedCurve);
					break;
				}
			}
		}
	}

	if (m_pChannelGrid->GetNumberRows())
		m_pChannelGrid->DeleteRows(0, m_pChannelGrid->GetNumberRows());

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator selectedCurve = m_SelectedCurves.Begin(); selectedCurve != m_SelectedCurves.End(); ++selectedCurve)
	{
		m_pChannelGrid->AppendRows();
		Btl::String label = (*selectedCurve)->pChannel->GetObject()->GetName() + "." + Btl::String((*selectedCurve)->pChannel->name.Chars()) + "." + Btl::String((*selectedCurve)->name.Chars());
		m_pChannelGrid->SetRowLabelValue(m_pChannelGrid->GetNumberRows()-1, label.Chars());
	}

	m_Flags.dirty = true;
}

bool ChannelSortPredicate(const LarvaType* a, const LarvaType* b)
{
	return a->label < b->label;
}

void C_AnimationPanel::OnNodeRename(wxTreeEvent& e)
{
	wxTreeItemId selectedItem = e.GetItem();

	if (!selectedItem.IsOk())
		return;

	C_AnimationTreeItemData* itemData = (C_AnimationTreeItemData*)m_pChannelTree->GetItemData(selectedItem);
	if (itemData->GetType() != C_AnimationTreeItemData::E_TreeItemDataType_Channel || !itemData->GetChannel()->hasNameField)
	{
		e.Veto();
		return;
	}

	if (e.GetEventType() == wxEVT_COMMAND_TREE_BEGIN_LABEL_EDIT)
	{
		m_pChannelTree->SetItemText(selectedItem, itemData->GetChannel()->GetName().Chars());
	} else {
		itemData->GetChannel()->SetName(e.GetLabel().ToAscii());
		GetNode()->XtSave();
	}
}

void C_AnimationPanel::OnContextMenu(wxTreeEvent& e)
{
	wxTreeItemId selectedItem = e.GetItem();

	if (!selectedItem.IsOk())
		return;

	wxPoint pt = e.GetPoint();

	C_AnimationTreeItemData* itemData = (C_AnimationTreeItemData*)m_pChannelTree->GetItemData(selectedItem);
	switch (itemData->GetType())
	{
	case (C_AnimationTreeItemData::E_TreeItemDataType_ObjectsRoot):
		{
			wxMenu menu;

			int newItemId = menu.Append(wxID_ANY, "Add New Object")->GetId();
			Connect(newItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnObjectAdd));

			PopupMenu(&menu, pt);

			Disconnect(newItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnObjectAdd));


			break;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Object):
		{
			EditorLib::C_DgAnimationPropNode::Object* object = itemData->GetObject();
			m_pMenuSelectedObject = object;

			wxMenu menu;
			
			wxMenu* addChannelMenu = new wxMenu();

			wxMenuItem* addChannelMenuItem = menu.AppendSubMenu(addChannelMenu, "Add Channel");
			
			wxMenuItem* changeAssignedObjectMenuItem = menu.Append(wxID_ANY, "Change Actor");
			Connect(changeAssignedObjectMenuItem->GetId(), wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnObjectChangeActor));

			int deleteItemId = menu.Append(wxID_ANY, "Delete Object")->GetId();
			Connect(deleteItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnObjectRemove));

			LtStruct* channelType = AsStruct(FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, "AnimationChannel"));
			Btl::Vector<LtStruct*> channelTypes;

			for(XtUt::Dictionary<XtString, LarvaType>::Iterator it(&EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema->types); it; ++it)
			{
				LtStruct* structType = AsStruct(it);
				
				if (structType && !structType->hidden && IsBaseOf(channelType, structType))
					channelTypes.PushBack(structType);
			}

			std::sort(channelTypes.Begin(), channelTypes.End(), ChannelSortPredicate);

			Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>& nodeChannels = object->GetChannels();

			for(Btl::Vector<LtStruct*>::Iterator it = channelTypes.Begin(); it != channelTypes.End(); ++it)
			{
				S32 maxInstances = 1;

				LarvaParam* maxInstancesParam = (*it)->FindParam("Animation", "maxinstances");
				if (maxInstancesParam)
				{
					if (maxInstancesParam->AsString() == "unlimited")
						maxInstances = -1;
					else
						maxInstances = maxInstancesParam->AsInt();
				}
				
				int instances = 0;
				for (Btl::Vector<EditorLib::C_DgAnimationPropNode::Channel*>::Iterator channel = nodeChannels.Begin(); channel != nodeChannels.End(); ++channel)
				{
					if ((*channel)->xtType == (*it))
						instances++;
				}

				if (maxInstances == -1 || instances < maxInstances)
				{
					int id = addChannelMenu->Append(wxID_ANY, wxString::Format(wxT((*it)->label)))->GetId();
					Connect(id, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnChannelAdd));
					m_menuIdTypeMap.Insert(id, *it);
				}
			}

			if (!addChannelMenu->GetMenuItemCount())
				menu.Enable(addChannelMenuItem->GetId(), false);

			PopupMenu(&menu, pt);

			for (Btl::Map<U32, LtStruct*>::Iterator it = m_menuIdTypeMap.Begin(); it != m_menuIdTypeMap.End(); ++it)
				Disconnect(it->GetKey(), wxEVT_COMMAND_MENU_SELECTED);

			for (Btl::Map<U32, XtId>::Iterator it = m_menuIdXtIdMap.Begin(); it != m_menuIdXtIdMap.End(); ++it)
				Disconnect(it->GetKey(), wxEVT_COMMAND_MENU_SELECTED);

			Disconnect(deleteItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnObjectRemove));

			m_menuIdTypeMap.Clear();
			m_menuIdXtIdMap.Clear();

			break;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Channel):
		{
			EditorLib::C_DgAnimationPropNode::Channel* channel = itemData->GetChannel();
			m_pMenuSelectedChannel = channel;

			wxMenu menu;

			int removeItemId = menu.Append(wxID_ANY, "Delete Channel")->GetId();
			Connect(removeItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnChannelRemove));

			PopupMenu(&menu, pt);

			Disconnect(removeItemId, wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(C_AnimationPanel::OnChannelRemove));

			return;
		}
	case (C_AnimationTreeItemData::E_TreeItemDataType_Curve):
		{
			return;
		}
	default:
		return;
	}
}

void C_AnimationPanel::OnObjectAdd(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	node->AddObject();
}

void C_AnimationPanel::OnObjectRemove(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	node->RemoveObject(m_pMenuSelectedObject);
}

void C_AnimationPanel::OnObjectChangeActor(wxCommandEvent& e)
{
	wxString windowTitle = "Choose new actor";
	wxDialog* dialog = new wxDialog((wxWindow*)this, (wxWindowID)wxID_ANY, windowTitle, wxDefaultPosition, wxSize(400,500), wxDEFAULT_DIALOG_STYLE);

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* panel = new wxPanel(dialog);

	sizer->Add(panel, 1, wxALL|wxEXPAND);
	sizer->Add(dialog->CreateSeparatedButtonSizer(wxOK|wxCANCEL), 0, wxALL|wxEXPAND);

	C_SelectionSceneGraphPanel* animationSceneGraph = new C_SelectionSceneGraphPanel(panel, wxID_ANY, wxDefaultPosition, wxSize(400, 450));

	dialog->SetSizer(sizer);
	dialog->Layout();

	if (dialog->ShowModal() == wxID_OK)
	{
		int itemCount = animationSceneGraph->GetSelection().GetItems().Size();
		if (itemCount > 0)
		{
			EditorLib::C_NodeId newId = animationSceneGraph->GetSelection().GetItems()[itemCount-1].id;

			EditorLib::I_DgNode *pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(newId);
			
			if (pNode && pNode->IsA(EditorLib::E_ObjType_SceneXtActorNode))
				m_pMenuSelectedObject->ChangeActor(newId.GetXtId());
		}
	}

	delete dialog;
}

void C_AnimationPanel::OnChannelAdd(wxCommandEvent& e)
{
	m_pMenuSelectedObject->AddChannel(m_menuIdTypeMap[e.GetId()]);
}

void C_AnimationPanel::OnChannelRemove(wxCommandEvent& e)
{
	(m_pMenuSelectedChannel->GetObject())->RemoveChannel(m_pMenuSelectedChannel);
}

void C_AnimationPanel::OnKeyFromGameCam(wxCommandEvent& e)
{
	C_CameraConnection *pConnection = C_RenderCore::Instance().GetCameraConnection();
	CameraMessage msg;
	if (pConnection && pConnection->Recieve(msg))
	{
		EditorLib::C_DgAnimationPropNode* pNode = GetNode();
		if (!pNode)	return;

		const EditorLib::C_SceneNode* pTopLevelParent = 0;
		{
			const EditorLib::C_DgXtCompNode* pParentComponent = pNode->GetParent();
			assert(pParentComponent);
			const EditorLib::I_DgNode* pTopLevelDgParent = pParentComponent->GetParent();
			assert(pTopLevelDgParent);
			if (pTopLevelDgParent->IsA(EditorLib::E_ObjType_SceneNode))
				pTopLevelParent = (EditorLib::C_SceneNode*)pTopLevelDgParent;
		}
		if (!pTopLevelParent) return;

		Maths::C_Quaternion q0(msg.a, msg.b, msg.c, msg.d);
		q0.Normalise();
		Maths::HMat33 rot(q0);		
		Maths::HMat44 camWorldH(rot);
		camWorldH[3] = Maths::HVec4(msg.x, msg.y, msg.z, 1.f);

		EditorLib::Matrix44 camWorld;
		Aftk::NightmareToTemplateMatrix44(camWorld, camWorldH);

		Aftk::Matrix44 invNodeWorld;
		{
			EditorLib::Matrix44 nodeWorld = pTopLevelParent->GetWorldMatrix();
			//Check matrix is valid.
			F32 det = UtMaths::GetDeterminant4x4(nodeWorld );
			if (det == 0)
				return;
			UtMaths::Invert4x4(&invNodeWorld, nodeWorld);
		}

		EditorLib::Matrix44 camLocal = camWorld * invNodeWorld;
		EditorLib::Vector3 rotation;

		if ((F64)camLocal[1][0] > 0.998)
			rotation = EditorLib::Vector3(0, atan2(camLocal[0][2], camLocal[2][2]), Maths::PiBy2);
		else if ((F64)camLocal[1][0] < -0.998)
			rotation = EditorLib::Vector3(0, atan2(camLocal[0][2], camLocal[2][2]), -Maths::PiBy2);
		else
			rotation = EditorLib::Vector3(atan2(-camLocal[1][2], camLocal[1][1]), atan2(-camLocal[2][0], camLocal[0][0]), asin(camLocal[1][0]));

		for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve=m_SelectedCurves.Begin(); curve!=m_SelectedCurves.End(); curve++)
		{
			F32 value = 0.f;
			
			if ((*curve)->pChannel->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
			{
				if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
					value = camLocal[3][0];
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y) 
					value = camLocal[3][1];
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z) 
					value = camLocal[3][2];
			} else if ((*curve)->pChannel->metadata == EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
			{
				if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
					value = Maths::RadToDeg(rotation[0]);
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
					value = Maths::RadToDeg(rotation[1]);
				else if ((*curve)->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
					value = Maths::RadToDeg(rotation[2]);
			} else
			{
				continue;
			}

			U32 newKeyIndex;
			(*curve)->AddKeyFrame(GetTimelineValue(), value, 0, 0, &newKeyIndex);
			(*curve)->SmoothTangents(newKeyIndex);

			(*curve)->EulerFilter();

			m_Flags.dirty = true;
		}
	}
	else
		LOGERROR(g_Log, "Connection to game camera failed");
	
}

void C_AnimationPanel::OnKeyFromGameCamUpdate(wxUpdateUIEvent& e)
{
	C_CameraConnection *pConnection = C_RenderCore::Instance().GetCameraConnection();
	e.Enable( GetNode() && pConnection && 
			  pConnection->GetConnectionState() == C_CameraConnection::E_ConnectionState_Connected);
}

EditorLib::C_DgAnimationPropNode* C_AnimationPanel::GetNode()
{
	if (!m_SelectedItem.IsValid())
		return 0;

	if (m_Flags.selectedItemDirty)
	{
		EditorLib::I_DgNode* dgNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(m_SelectedItem);
		
		if (!dgNode)
		{
			m_SelectedNode = 0;
		}
		else if (dgNode && dgNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase))
		{
			EditorLib::C_SceneXtNodeBase* pSceneXtActorNode = (EditorLib::C_SceneXtNodeBase*)dgNode;

			for (EditorLib::DgActorComponentArray::ConstIterator compIt = pSceneXtActorNode->GetComponents().Begin(); 
				compIt != pSceneXtActorNode->GetComponents().End(); ++compIt)
			{
				Sys::WeakPtr<EditorLib::C_DgXtCompNode> pComponent = *compIt;
				if (!pComponent) continue;

				for (EditorLib::DgPropArray::ConstIterator propIt = pComponent->GetProperties().Begin(); 
					propIt != pComponent->GetProperties().End(); ++propIt)
				{
					Sys::WeakPtr<EditorLib::C_DgXtPropNodeBase> pProp = *propIt;
					if (pProp && pProp->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
					{
						m_SelectedNode = (EditorLib::C_DgAnimationPropNode*)pProp.RawPtr();
						return m_SelectedNode;
					}
				}
			}
			m_SelectedNode = 0;
			return m_SelectedNode;
		}
		else if (!dgNode->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
		{
			m_SelectedNode = 0;
		}
		else
		{
			m_SelectedNode = (EditorLib::C_DgAnimationPropNode*)dgNode;
		}
	}

	return m_SelectedNode;
}

void C_AnimationPanel::UpdateProperty()
{
	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode)
		return;

	AftkRender::C_SceneGraphNode* pRenderNode = C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(pNode);

	if (!pRenderNode)
		return;

	if (m_Flags.timelineDirty)
	{
		if (m_Flags.timelineScrubDirty)
		{
			float a = (m_pTimeline->GetValue()-m_pTimeline->GetMin()) / pNode->GetQuantisation();
			float b = (m_pTimeline->GetValue()-m_pTimeline->GetMin());
			float c = pNode->GetQuantisation();
			m_pTimelineValue->ChangeValue(wxString::Format("%f", (float)(m_pTimeline->GetValue()-m_pTimeline->GetMin()) / (float)pNode->GetQuantisation()));
			m_Flags.timelineScrubDirty = false;
		}
		((C_RenderAnimationPropNode*)pRenderNode)->SetTimelineValue(GetTimelineValue());
		m_Flags.timelineDirty = false;
	}
}

double C_AnimationPanel::GetTimelineValue()
{
	EditorLib::C_DgAnimationPropNode* pNode = GetNode();

	if (!pNode)
		return 0.0;

	double doubleValue;

	if (m_pTimelineValue->GetValue().Trim().ToDouble(&doubleValue))
		return doubleValue;
	else
		return (double)(m_pTimeline->GetValue()-m_pTimeline->GetMin()) / (double)pNode->GetQuantisation();
}

void C_AnimationPanel::EvaluateGridValues()
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	C_RenderAnimationPropNode* pRenderNode = (C_RenderAnimationPropNode*)
		C_RenderCore::Instance().GetRenderScene().GetSceneGraphNode(node);

	if (!pRenderNode)
		return;

	for (int line=0; line<m_pChannelGrid->GetNumberRows(); line++)
	{
		EditorLib::C_DgAnimationPropNode::AnimationCurve* animationCurve = m_SelectedCurves[line];

		F32 evaluatedValue = 0.0f;

		if (animationCurve->pCurveData->GetNumKnots())
			m_pChannelGrid->SetCellBackgroundColour(line, 0, wxColor(0x92C8FA));
		else
			m_pChannelGrid->SetCellBackgroundColour(line, 0, wxColor(0xFFFFFF));

		evaluatedValue = animationCurve->Evaluate(GetTimelineValue()) + animationCurve->GetOffset();;		

		wxString value = wxString::Format("%lf", evaluatedValue);
		m_pChannelGrid->SetCellValue(line, 0, value);
	}
}

void C_AnimationPanel::OnClipLengthChange(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	double clipLength;
	if (m_pClipLength->GetValue().ToDouble(&clipLength))
	{
		clipLength = Maths::Min((F32)clipLength, 1000.f);
		clipLength = Maths::Max((F32)clipLength, 0.f);

		node->SetClipLength(clipLength);
		m_pTimeline->SetRange(0, node->GetClipLength()*node->GetQuantisation());

		m_Flags.dirty = true;
	}
}

void C_AnimationPanel::OnQuantisationChange(wxCommandEvent& e)
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node)
		return;

	unsigned long quantisation;
	if (m_pQuantisation->GetValue().ToULong(&quantisation))
	{
		quantisation = Maths::Min(quantisation, 65535);
		quantisation = Maths::Max(quantisation, 1);
		
		node->SetQuantisation(quantisation);
		m_pTimeline->SetRange(0, node->GetClipLength()*quantisation);

		m_Flags.dirty = true;
	}
}

void C_AnimationPanel::UpdateGridPositionValues(EditorLib::C_DgAnimationPropNode::Object* object, const Maths::HVec3& position)
{
	for (int i=0; i<m_SelectedCurves.Size(); i++)
	{
		if (m_SelectedCurves[i]->pChannel->GetObject() != object)
			continue;

		if (m_SelectedCurves[i]->pChannel->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Translation)
			continue;

		wxString value;

		if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
			value = wxString::Format("%f", (F32)position.X());
		else if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
			value = wxString::Format("%f", (F32)position.Y());
		else if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
			value = wxString::Format("%f", (F32)position.Z());
		else 
			continue;
			
		m_pChannelGrid->SetCellValue(i, 0, value);
	}
}

void C_AnimationPanel::UpdateGridRotationValues(EditorLib::C_DgAnimationPropNode::Object* object, const Maths::HVec3& rotation)
{
	for (int i=0; i<m_SelectedCurves.Size(); i++)
	{
		if (m_SelectedCurves[i]->pChannel->GetObject() != object)
			continue;

		if (m_SelectedCurves[i]->pChannel->metadata != EditorLib::C_DgAnimationPropNode::Channel::E_AnimationChannel_Metadata_Rotation)
			continue;

		wxString value;

		if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_X)
			value = wxString::Format("%f", (F32)rotation.X());
		else if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Y)
			value = wxString::Format("%f", (F32)rotation.Y());
		else if (m_SelectedCurves[i]->metadata == EditorLib::C_DgAnimationPropNode::AnimationCurve::E_AnimationCurve_Metadata_Z)
			value = wxString::Format("%f", (F32)rotation.Z());
		else 
			continue;
			
		m_pChannelGrid->SetCellValue(i, 0, value);
	}
}

EditorLib::C_Status C_AnimationPanel::XtLoad()
{	
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node || !m_pChart)
		return EditorLib::E_Status_Failure;

	m_pClipLength->ChangeValue(wxString::Format("%lf", node->GetClipLength()));
	m_pQuantisation->ChangeValue(wxString::Format("%lu", node->GetQuantisation()));

	return EditorLib::E_Status_Success;
}

void C_AnimationPanel::OnFrameChart(wxKeyEvent& e)
{
	if (e.GetKeyCode() == 'f' || e.GetKeyCode() == 'F')
	{
		CalculateChartExtents();

		if (!m_pChart)
			return;
	
		m_pChart->SetScroll(0, 0);
		m_pChart->SetZoom(1,1);
	}
}

void C_AnimationPanel::CalculateChartExtents()
{
	EditorLib::C_DgAnimationPropNode* node = GetNode();

	if (!node || !m_pChart)
		return;

	m_chartBounds.X(FLT_MAX);
	m_chartBounds.Y(FLT_MIN);
	m_chartBounds.Z(FLT_MAX);
	m_chartBounds.W(FLT_MIN);

	for (Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*>::Iterator curve=m_SelectedCurves.Begin(); curve != m_SelectedCurves.End(); ++curve)
	{
		for (Btl::Vector<Curve::C_MayaHermiteSplineKnotU<F32> >::Iterator knot = (*curve)->pCurveData->GetKnots().Begin(); knot != (*curve)->pCurveData->GetKnots().End(); ++knot)
		{
			if (knot->u < m_chartBounds.X())
				m_chartBounds.X(knot->u);
			if (knot->u > m_chartBounds.Y())
				m_chartBounds.Y(knot->u);
			if (knot->value < m_chartBounds.Z())
				m_chartBounds.Z(knot->value);
			if (knot->value > m_chartBounds.W())
				m_chartBounds.W(knot->value + 0.1f);
		}
	}

	if (m_chartBounds.X() == FLT_MAX)
		m_chartBounds.X(0);
	
	if (m_chartBounds.Y() == FLT_MIN)
		m_chartBounds.Y(5);

	if (m_chartBounds.Z() == FLT_MAX)
		m_chartBounds.Z(-50);

	if (m_chartBounds.W() == FLT_MIN)
		m_chartBounds.W(50);

	if (m_chartBounds.X() == m_chartBounds.Y())
	{
		m_chartBounds.X(m_chartBounds.X()-0.1f);
		m_chartBounds.Y(m_chartBounds.Y()+0.1f);
	}

	if (m_chartBounds.Z() == m_chartBounds.W())
	{
		m_chartBounds.Z(m_chartBounds.Z()-0.1f);
		m_chartBounds.W(m_chartBounds.W()+0.1f);
	}

	float widthBorder = (m_chartBounds.Y()-m_chartBounds.X())*0.1f;
	m_chartBounds.X(m_chartBounds.X() - widthBorder);
	m_chartBounds.Y(m_chartBounds.Y() + widthBorder);
	float heightBorder = (m_chartBounds.W()-m_chartBounds.Z())*0.1f;
	m_chartBounds.Z(m_chartBounds.Z() - heightBorder);
	m_chartBounds.W(m_chartBounds.W() + heightBorder);

	m_Flags.dirty = true;
}

/////
/////

C_DopeSheetPanel::C_Slots::C_Slots(C_DopeSheetPanel* pDialog)
: m_pDialog(pDialog)
{
}

void C_DopeSheetPanel::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.connect(this, &Editor::C_DopeSheetPanel::C_Slots::OnCoreEnableViewsSlot);
	s.coreSelectionChanged.connect(this, &Editor::C_DopeSheetPanel::C_Slots::OnCoreSelectionChangedSlot);
	s.levelClosing.connect(this, &Editor::C_DopeSheetPanel::C_Slots::OnLevelClosingSlot);
}

void C_DopeSheetPanel::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.coreEnableViews.disconnect(this);
	s.coreSelectionChanged.disconnect(this);
	s.levelClosing.disconnect(this);
}

void C_DopeSheetPanel::C_Slots::OnCoreEnableViewsSlot(bool enable)
{
	m_pDialog->OnCoreEnableViews(enable);
}

void C_DopeSheetPanel::C_Slots::OnCoreSelectionChangedSlot()
{
	m_pDialog->OnCoreSelectionChanged();
}

void C_DopeSheetPanel::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pDialog->OnLevelClosing(level);
}

C_DopeSheetPanel::C_DopeSheetPanel(wxWindow *parent, wxWindowID id,
							const wxPoint& pos,
							const wxSize& size,
							long style)
: wxPanel(parent, id, pos, size, style)
, m_Slots(this)
, m_connected(false)
, m_pScarabForm(0)
, m_pScarabFormPanel(0)
{
	Connect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_DopeSheetPanel::OnSize));

	wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);

	m_pScarabFormPanel = new wxPanel(this, wxID_ANY);
	m_pScarabFormPanel->SetMinSize( wxSize( -1,30 ) );

	pSizer->Add( m_pScarabFormPanel, 1, wxEXPAND|wxALL, 5 );	

	SetSizer(pSizer);
	Layout();

	Tasks::ShowDopeSheetDialog();

	m_pScarabForm = 0;

	m_Slots.Connect();

	m_connected = true;
}

void C_DopeSheetPanel::Shutdown()
{
	if (m_connected)
	{
		Disconnect(GetId(), wxEVT_SIZE, wxSizeEventHandler(C_DopeSheetPanel::OnSize));

		m_Slots.Disconnect();
		m_connected = false;

		SAFE_DELETE_PTR(m_pScarabForm);
		SAFE_DELETE_PTR(m_pScarabFormPanel);
	}
}

void C_DopeSheetPanel::OnCoreEnableViews(bool enable)
{
}

void C_DopeSheetPanel::OnCoreSelectionChanged()
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	if (selection.GetItems().Size() != 1)
	{
		EditorLib::C_NodeId invalidId;
		SelectItem(invalidId);
		return;
	}

	EditorLib::C_NodeId selectedItem = selection.GetItems()[0].id;
	EditorLib::I_DgNode *pNode =  EditorLib::C_Core::Instance().GetDgManager().FindNodeById(selectedItem);

	EditorLib::C_XtObject* pXtObject;
	if (pNode)
		pXtObject = pNode->AsXtObject();
	else
		pXtObject = 0;
		
	if (!pXtObject || !pXtObject->GetXtData())
	{
		EditorLib::C_NodeId invalidId;
		SelectItem(invalidId);
		return;
	}
	else
	{
		SelectItem(selectedItem);
	}
}

void C_DopeSheetPanel::OnLevelClosing(const EditorLib::I_Level& level)
{
	EditorLib::C_NodeId invalidId;
	SelectItem(invalidId);
}

void C_DopeSheetPanel::SelectItem(const EditorLib::C_NodeId& id)
{
	SAFE_DELETE_PTR(m_pScarabForm);

	if (!id.IsValid())
		return;

	EditorLib::C_DgAnimationPropNode* node = 0;

	EditorLib::I_DgNode* dgNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(id);
		
	if (!dgNode)
		return;

	if (dgNode->IsA(EditorLib::E_ObjType_SceneXtNodeBase))
	{
		EditorLib::C_SceneXtNodeBase* pSceneXtActorNode = (EditorLib::C_SceneXtNodeBase*)dgNode;

		if (!pSceneXtActorNode)
			return;
		else
		{
			for (EditorLib::DgActorComponentArray::ConstIterator component = pSceneXtActorNode->GetComponents().Begin(); component != pSceneXtActorNode->GetComponents().End(); ++component)
			{
				for (EditorLib::DgPropArray::ConstIterator prop=(*component)->GetProperties().Begin(); prop!=(*component)->GetProperties().End(); ++prop)
				{
					if ((*prop) && (*prop)->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
					{
						node = (EditorLib::C_DgAnimationPropNode*)(*prop).RawPtr();
					}
				}
			}
		}
	}
	else if (!dgNode->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
		return;
	else
		node = (EditorLib::C_DgAnimationPropNode*)dgNode;

	if (!node || !node->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
		return;

	Xt::DomComponent* pComponent = Xt::AsComponent(node->GetXtData());

	if (!pComponent)
		return;

	m_pScarabForm = new ScarabForm(new ScarabFormContext());
	m_pScarabForm->Create((HWND)m_pScarabFormPanel->GetHandle());
	
	LtStruct* animationClipType = AsStruct(FindTypeByName(EditorLib::C_Core::Instance().GetXtManager().GetDb()->schema, "AnimationClip"));
	m_pScarabForm->SetDom(&pComponent->data, animationClipType);

	WinRect rcClient((HWND)m_pScarabFormPanel->GetHandle());
	SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
	RedrawWindow(*m_pScarabForm, 0, 0, RDW_INVALIDATE);
}

void C_DopeSheetPanel::OnSize(wxSizeEvent& e)
{
	__super::OnSize(e);

	if (m_pScarabForm)
	{
		WinRect rcClient((HWND)m_pScarabFormPanel->GetHandle());
		SetWindowPos(*m_pScarabForm, 0, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
		RedrawWindow(*m_pScarabForm, 0, 0, RDW_INVALIDATE);
	}
}
