#ifndef EDITOR_ANIMATION_DIALOG_H
#define EDITOR_ANIMATION_DIALOG_H

#ifdef _MSC_VER
#pragma once
#endif

#include "sigslot/SigSlot.h"

#include <UserNode/Props/Animation/DgAnimationPropNode.h>

class wxListBox;
class wxBoxSizer;
class wxTreeCtrl;
class wxTreeEvent;
class wxTreeItemData;
class wxGrid;
class wxGridEvent;
class BitmapCheckButton;

class ScarabForm;
class TimelineChartCallbacks;

namespace EditorLib
{
	class C_Actor;
}

namespace Editor
{
	class C_ReferenceData;
	class C_AnimationPanel;

	class C_DopeSheetPanel : public wxPanel
	{
	public:
		C_DopeSheetPanel(wxWindow *parent,  wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(500, 300),
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);
		
		void Shutdown();

	private:
		class C_Slots : public sigslot::has_slots<>
		{
		public: 
			C_Slots(C_DopeSheetPanel *pDialog);
			void Connect();
			void Disconnect();

			void OnCoreEnableViewsSlot(bool enable);
			void OnCoreSelectionChangedSlot();
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			
		private:
			C_DopeSheetPanel* m_pDialog;
		};

		void SelectItem(const EditorLib::C_NodeId& id);
		void OnCoreEnableViews(bool enable);
		void OnCoreSelectionChanged();
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnSize(wxSizeEvent& e);

		C_Slots				m_Slots;

		wxPanel*			m_pScarabFormPanel;
		ScarabForm*			m_pScarabForm;
		bool				m_connected;
	};

	class C_AnimationPanel : public wxPanel
	{
	public:
		C_AnimationPanel(wxWindow *parent,  wxWindowID id = wxID_ANY,
							const wxPoint& pos = wxDefaultPosition,
							const wxSize& size = wxSize(500, 300),
							long style = wxTAB_TRAVERSAL|wxNO_BORDER);

		~C_AnimationPanel();
		void Shutdown();

		void OnNodeSelect(wxTreeEvent& e);

	private:
		class C_AnimationTreeItemData : public wxTreeItemData
		{
		public:
			enum E_TreeItemDataType
			{
				E_TreeItemDataType_ObjectsRoot,
				E_TreeItemDataType_Curve,
				E_TreeItemDataType_Channel,
				E_TreeItemDataType_Object,
				E_TreeItemDataType_Unknown
			};

			C_AnimationTreeItemData()
			{
				m_dataType = E_TreeItemDataType_Unknown;
			}

			C_AnimationTreeItemData(E_TreeItemDataType type)
			{
				m_dataType = type;
			}

			C_AnimationTreeItemData(EditorLib::C_DgAnimationPropNode::AnimationCurve* curve)
			{
				m_pCurve = curve;
				m_dataType = E_TreeItemDataType_Curve;
			}

			C_AnimationTreeItemData(EditorLib::C_DgAnimationPropNode::Channel* channel)
			{
				m_pChannel = channel;
				m_dataType = E_TreeItemDataType_Channel;
			}

			C_AnimationTreeItemData(EditorLib::C_DgAnimationPropNode::Object* object)
			{
				m_pObject = object;
				m_dataType = E_TreeItemDataType_Object;
			}

			E_TreeItemDataType GetType()
			{
				return m_dataType;
			}

			EditorLib::C_DgAnimationPropNode::AnimationCurve* GetCurve()
			{
				return m_pCurve;
			}

			EditorLib::C_DgAnimationPropNode::Channel* GetChannel()
			{
				return m_pChannel;
			}

			EditorLib::C_DgAnimationPropNode::Object* GetObject()
			{
				return m_pObject;
			}

		private:
			EditorLib::C_DgAnimationPropNode::AnimationCurve* m_pCurve;
			EditorLib::C_DgAnimationPropNode::Channel* m_pChannel;
			EditorLib::C_DgAnimationPropNode::Object* m_pObject;
			E_TreeItemDataType m_dataType;
		};

		class AnimationTimer : public wxTimer
		{
		public:
			AnimationTimer(C_AnimationPanel* animationPanel);

			virtual void Notify();
		private:
			C_AnimationPanel* m_pDialog;
		};

		class AnimationChartCallbacks : public ::TimelineChartCallbacks
		{
		public:
			AnimationChartCallbacks(C_AnimationPanel* animationPanel);

			virtual int GetLineCount(void);
			virtual double GetTimelineValue(void);
			virtual void GetLineInfo(int line, TimelineChartLine *out);
			virtual void GetLineValue(int line, int index, ChartPoint *out);
			virtual void SetLineValue(int line, int index, const ChartPoint *in);
			virtual void GetInTangentValue(int line, int index, double *in);
			virtual void DeleteKey(int line, int index);
			virtual void SmoothTangents(int line, int index);
			virtual void GetOutTangentValue(int line, int index, double *out);
			virtual void SetInTangentValue(int line, int index, double in);
			virtual void SetOutTangentValue(int line, int index, double out);
			
			virtual int GetNumSelectedComponents();
			virtual void GetSelectedComponent(int index, int *outLine, int *outIndex);
			virtual void SetSelectedComponent(int line, int index, bool selected=true);
			virtual void ClearSelection();
		private:
			C_AnimationPanel* m_pDialog;
		};

		class C_Slots : public sigslot::has_slots<>
		{
		public: 
			C_Slots(C_AnimationPanel *pDialog);
			void Connect();
			void Disconnect();

			void OnCoreEnableViewsSlot(bool enable);
			void OnCoreSelectionChangedSlot();
			void OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc);
			void OnDgNodeDestroyedSlot(const EditorLib::I_DgNode& dgNode);
			void OnLevelOpeningSlot(const EditorLib::I_Level& level);
			void OnLevelOpenedSlot(const EditorLib::I_Level& level);
			void OnLevelClosingSlot(const EditorLib::I_Level& level);
			void OnLevelClosedSlot(const EditorLib::I_Level& level);

		private:
			C_AnimationPanel* m_pDialog;
		};

		struct C_Flags
		{
			U32 dirty : 1;
			U32 pendingSync : 1;
			U32 viewEnabled : 1;
			U32 timelineDirty : 1;
			U32 timelineScrubDirty : 1;
			U32 dataDirty : 1;
			U32 selectedItemDirty : 1;
			U32 pendingSelectionSync : 1;
		} m_Flags;

		void OnIdle(wxIdleEvent& e);
		void OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode);
		void OnDgNodeDestroyed(const EditorLib::I_DgNode& dgNode);
		void OnCoreEnableViews(bool enable);
		void OnCoreSelectionChanged();
		void OnLevelOpening(const EditorLib::I_Level& level);
		void OnLevelOpened(const EditorLib::I_Level& level);
		void OnLevelClosing(const EditorLib::I_Level& level);
		void OnLevelClosed(const EditorLib::I_Level& level);
		void SelectItem(const EditorLib::C_NodeId& nodeId);

		void OnSize(wxSizeEvent& e);
		void OnTimelineScrub(wxScrollEvent& e);
		void OnTimelineValueChanged(wxCommandEvent& e);

		void OnCellChanged(wxGridEvent& e);
		void OnCellRightClick(wxGridEvent& e);
		void OnSetKeyframe(wxCommandEvent& e);

		void OnClipLengthChange(wxCommandEvent& e);
		void OnQuantisationChange(wxCommandEvent& e);

		void OnRunAnimation(wxCommandEvent& e);
		void OnPlayButtonUpdate(wxUpdateUIEvent& e);
		void OnPauseButtonUpdate(wxUpdateUIEvent& e);

		void SyncSelectionToTree();
		void OnChannelSelectionChanged(wxTreeEvent& e);
		void OnContextMenu(wxTreeEvent& e);
		void OnNodeRename(wxTreeEvent& e);

		void OnObjectAdd(wxCommandEvent& e);
		void OnObjectRemove(wxCommandEvent& e);
		void OnObjectChangeActor(wxCommandEvent& e);
		void OnChannelAdd(wxCommandEvent& e);
		void OnChannelRemove(wxCommandEvent& e);

		void OnKeyFromGameCam(wxCommandEvent& e);
		void OnKeyFromGameCamUpdate(wxUpdateUIEvent& e);

		void OnShowDopeSheetButton(wxCommandEvent& e);
		void OnShowDopeSheetButtonUpdate(wxUpdateUIEvent& e);

		void OnAnimationTimer();

		void OnFrameChart(wxKeyEvent& e);

		void Refresh();

		EditorLib::C_DgAnimationPropNode* GetNode();

		double GetTimelineValue();
		void EvaluateGridValues();
		void UpdateProperty();

		void UpdateGridPositionValues(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3& position);
		void UpdateGridRotationValues(EditorLib::C_DgAnimationPropNode::Object*, const Maths::HVec3& rotation);

		void UpdateItem();

		void ShowEditControls(bool show);

		void CalculateChartExtents();

		EditorLib::C_Status XtLoad();

		EditorLib::C_NodeId		m_SelectedItem;
		EditorLib::C_DgAnimationPropNode* m_SelectedNode;

		C_Slots					m_Slots;

		wxSizer*				m_pNoAnimationSizer;
		wxSizer*				m_pEditSizer;

		TimelineChart*			m_pChart;
		TimelineChartCallbacks*	m_pChartCallbacks;
		wxPanel*				m_pChartPanel;
		Maths::HVec4			m_chartBounds;
		
		AnimationTimer*			m_pAnimationTimer;

		ScarabForm*				m_pAnimatedObject;
		wxPanel*				m_pAnimatedObjectPanel;

		wxTreeCtrl*				m_pChannelTree;
		Btl::Vector<EditorLib::C_DgAnimationPropNode::AnimationCurve*> m_SelectedCurves;
		wxGrid*					m_pChannelGrid;
		wxTextCtrl*				m_pQuantisation;
		wxTextCtrl*				m_pClipLength;

		wxBitmapButton*			m_pPlayButton;
		wxBitmapButton*			m_pPauseButton;
		BitmapCheckButton*		m_pLoopButton;
		wxBitmapButton*			m_pShowDopeSheetButton;
		wxBitmapButton*			m_pKeyGameCamButton;

		Btl::Map<U32, XtId>		 m_menuIdXtIdMap;
		Btl::Map<U32, LtStruct*> m_menuIdTypeMap;
		int						 m_cellRowSelected;

		wxTextCtrl*				m_pTimelineValue;
		wxSlider*				m_pTimeline;

		bool					m_connected;

		EditorLib::C_DgAnimationPropNode::Object* m_pMenuSelectedObject;
		EditorLib::C_DgAnimationPropNode::Channel* m_pMenuSelectedChannel;

		Btl::Vector<Btl::Pair<U32, U32> > m_selectedItems;

		DECLARE_CLASS(C_AnimationPanel)
	};

}
#endif
