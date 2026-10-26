#include "stdafx.h"

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>

#include <EditorLib/Misc/Selection.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

#include <EditorApp/Components/Layer/LayerPropertyPanel.h>
#include <EditorApp/Components/Layer/LayerPropertyMenuBar.h>

#include <EditorApp/Tasks/LayerTasks.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/Misc/EventProxy.h>

#include <wx/tooltip.h>
#include <wx/colordlg.h>
#include <wx/statline.h>

#include <minmax.h>
#include <gdiplus.h>
#include <math.h>
#include <xtwin/Constants.h>
#include <xtwin/Algorithms.h>

using namespace Core::Algorithms;
#include <XtWin\ColourPalette.h>

namespace Editor
{

C_LayerPropertyCtrl::C_Slots::C_Slots(C_LayerPropertyCtrl *pLayerCtrl)
: m_pLayerPropCtrl(pLayerCtrl)
{
}

void C_LayerPropertyCtrl::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelOpening.connect(this, &C_LayerPropertyCtrl::C_Slots::OnLevelOpeningSlot);
	s.levelOpened.connect(this, &C_LayerPropertyCtrl::C_Slots::OnLevelOpenedSlot);
	s.levelClosing.connect(this, &C_LayerPropertyCtrl::C_Slots::OnLevelClosingSlot);
	s.levelClosed.connect(this, &C_LayerPropertyCtrl::C_Slots::OnLevelClosedSlot);

	s.layerEvent.connect(this, &C_LayerPropertyCtrl::C_Slots::OnLayerEventSlot);
}

void C_LayerPropertyCtrl::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);

	s.layerEvent.disconnect(this);
}

void C_LayerPropertyCtrl::C_Slots::OnLevelOpeningSlot(const EditorLib::I_Level& level)
{
	m_pLayerPropCtrl->OnLevelOpening(level);
}

void C_LayerPropertyCtrl::C_Slots::OnLevelOpenedSlot(const EditorLib::I_Level& level)
{
	m_pLayerPropCtrl->OnLevelOpened(level);
}

void C_LayerPropertyCtrl::C_Slots::OnLevelClosingSlot(const EditorLib::I_Level& level)
{
	m_pLayerPropCtrl->OnLevelClosing(level);
}

void C_LayerPropertyCtrl::C_Slots::OnLevelClosedSlot(const EditorLib::I_Level& level)
{
	m_pLayerPropCtrl->OnLevelClosed(level);
}

void C_LayerPropertyCtrl::C_Slots::OnLayerEventSlot(const EditorLib::I_Layer& layer, EditorLib::E_LayerEvent e)
{
	if (e == EditorLib::E_LayerEvent_Added)
		m_pLayerPropCtrl->OnLevelLayerAdded(layer);
	else if (e == EditorLib::E_LayerEvent_Removed)
		m_pLayerPropCtrl->OnLevelLayerRemoved(layer);
	else if (e == EditorLib::E_LayerEvent_Deserialised)
		m_pLayerPropCtrl->OnLayerDeserialised(layer);
}

const char* C_LayerPropertyCtrl::m_pIconStringTable[] =
{
#define DEF(e, str) str,
	#include <EditorApp/Components/Layer/LayerPropertyIconMapping.h>
#undef DEF
};

#define LAYER_CTRL_SIZE		20
#define LAYER_LABEL_SPACE	4
#define LAYER_CTRL_SPACE	2

IMPLEMENT_CLASS(C_LayerPropertyCtrl, wxControl)
BEGIN_EVENT_TABLE(C_LayerPropertyCtrl, wxControl)
  EVT_MOUSE_EVENTS(C_LayerPropertyCtrl::OnMouseEvent)
  EVT_PAINT(C_LayerPropertyCtrl::OnPaint)
  EVT_IDLE(C_LayerPropertyCtrl::OnIdle)
END_EVENT_TABLE()

C_LayerPropertyCtrl::C_LayerPropertyCtrl(wxWindow *parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
: wxControl(parent, id, pos, size, style)
, m_pImageList(0)
, m_pCurrentLayer(0)
, m_slots(this)
, m_pContextMenu(0)
{
	U32 labelOffset = LAYER_CTRL_SPACE+E_CtrlType_Count*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE);
	
	m_pTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER|wxNO_BORDER);
	//pSizer->Add(labelOffset, 0, 0, wxEXPAND);
	//pSizer->Add(m_pTextCtrl, 0, wxEXPAND|wxALL, 4);
	m_pTextCtrl->Show(false);
	//SetSizer(pSizer);
	//Layout();

	m_pImageList = new wxImageList(LAYER_CTRL_SIZE, LAYER_CTRL_SIZE);
	for (U32 i=0; i<E_LayerPropertyIcon_Count; ++i)
	{
		wxImage image;
		image.LoadFile(m_pIconStringTable[i]);
		m_pImageList->Add(image);
	}

	m_pTextCtrl->Connect(wxID_ANY, wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(C_LayerPropertyCtrl::OnTextLabelEraseBackground), 0, this);
	m_pTextCtrl->Connect(wxID_ANY, wxEVT_KILL_FOCUS, wxFocusEventHandler(C_LayerPropertyCtrl::OnTextLabelEditFocus), 0, this);
	m_pTextCtrl->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(C_LayerPropertyCtrl::OnTextLabelEditEnd), 0, this);

	m_slots.Connect();
	m_flags.connected = true;
	m_flags.pendingSync = false;
}

C_LayerPropertyCtrl::~C_LayerPropertyCtrl()
{
	delete m_pContextMenu;
	delete m_pImageList;
	Shutdown();
}

void C_LayerPropertyCtrl::OnMouseEvent(wxMouseEvent &e)
{
	C_HitTestResult result = HitTest(e.GetX(), e.GetY());
	if (!result.pLayer)
		return;

	if (e.ButtonDClick() || e.ButtonDown())
	{
		if (wxWindow::FindFocus() != this)
			SetFocus();

		switch (result.ctrlType)
		{
		case E_CtrlType_Visibility:
			{
				EditorLib::I_Layer::Flags flags = result.pLayer->GetLayerFlags();
				flags.visible = !flags.visible;
				result.pLayer->SetLayerFlags(flags);
			}
			break;
		case E_CtrlType_Selectable:
			{
				EditorLib::I_Layer::Flags flags = result.pLayer->GetLayerFlags();
				flags.selectable = !flags.selectable;
				result.pLayer->SetLayerFlags(flags);
			}
			break;
		case E_CtrlType_Colour:
			{
				if (result.pLayer->GetType() == EditorLib::E_LayerType_Actor)
				{
					if (e.LeftDown())
					{
						ColourPalette *pPalette = new ColourPalette();
						Maths::C_RGBA_U8 c = result.pLayer->GetDisplayColour();
						ColourSwatchData csd;
						{
							csd.red = c.R()/255.f;
							csd.green = c.G()/255.f;
							csd.blue = c.B()/255.f;
							csd.alpha = c.A()/255.f;
						}

						pPalette->SetData(&csd);
						pPalette->SetDisplayAlpha(false);
						pPalette->SetDisplayIntensity(false);

						wxPoint p(e.GetX(), e.GetY());
						p = ClientToScreen(p);

						pPalette->DoModal(WinPoint(p.x, p.y), (HWND)GetHandle());
						pPalette->GetData(&csd);
						result.pLayer->SetDisplayColour(Maths::RGBA(csd.red, csd.green, csd.blue, 1.f));
						

						delete pPalette;
					}
					else
					{
						//reset colour (ie. alpha=0)
						result.pLayer->SetDisplayColour(Maths::C_RGBA_U8(0, 0, 0, 0));
					}
				}
			}
			break;
		default:
			{
				if (e.RightDown())
				{
					m_pCurrentLayer = result.pLayer;
					OnShowContextMenu();
					return;
				}

				if (e.ControlDown())
				{
					//toggle selection
					m_layerDataArray[result.index].selected = !m_layerDataArray[result.index].selected;
					m_lastSelection = result.index;
				}
				else if (e.ShiftDown())
				{
					if (m_lastSelection == -1)
					{
						m_layerDataArray[result.index].selected = true;
						m_lastSelection = result.index;
					}
					else
					{
						//select all between last selection & current
						for (S32 i=0; i<m_lastSelection && i<result.index; ++i)
							m_layerDataArray[i].selected = false;

						for (S32 i=Maths::Min(result.index, m_lastSelection); i<=Maths::Max(result.index, m_lastSelection); ++i)
							m_layerDataArray[i].selected = true;

						for (S32 i=Maths::Max(result.index, m_lastSelection)+1; i<m_layerDataArray.Size(); ++i)
							m_layerDataArray[i].selected = false;			
					}
				}
				else
				{
					//deselect all
					for (S32 i=0; i<m_layerDataArray.Size(); ++i)
						m_layerDataArray[i].selected = false;

					//select current
					m_layerDataArray[result.index].selected = true;
					m_lastSelection = result.index;
				}
			}
			break;
		};

		Refresh();
	}
	else
	{
		wxString str;

		switch (result.ctrlType)
		{
		case E_CtrlType_Visibility:
			if (result.pLayer->GetLayerFlags().visible)
				str << "Visible";
			else
				str << "Invisible";
			break;
		case E_CtrlType_Selectable:
			if (result.pLayer->GetLayerFlags().selectable)
				str << "Selectable";
			else
				str << "Unselectable";
			break;
		case E_CtrlType_Colour:
			{
				Maths::C_RGBA_U8 c = result.pLayer->GetDisplayColour();
				if (c.A())
					str << _T("R:") << c.R() << _T(" G:") << c.G() << _T(" B:") << c.B();
				else
					str << _T("Default Colour");
			}
			break;
		};

		wxToolTip *pToolTip = GetToolTip();
		if (pToolTip)
			pToolTip->SetTip(str);
		else
			SetToolTip(str);
	}
}

void C_LayerPropertyCtrl::OnPaint(wxPaintEvent& e)
{
	wxPaintDC dc(this);
	wxControl::OnPaint(e);

	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
	{
		const C_LayerData& data = m_layerDataArray[i];
		if (data.selected)
		{
			wxSize size = GetSize();

			dc.SetBrush(wxColour(120,117,112,255));
			dc.SetPen(wxColour(120,117,112,255));

			dc.DrawRectangle(0, i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE), size.x, LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE);
		}

		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(wxColour(159,156,150,255));

		//visibility icon
		{
			if (m_pImageList)
			{
				E_LayerPropertyIcon icon = data.pLayer->GetLayerFlags().visible ? E_LayerPropertyIcon_Visible : E_LayerPropertyIcon_Invisible;											
				m_pImageList->Draw(icon, dc, LAYER_CTRL_SPACE, LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE));
			}
			dc.DrawRectangle (wxRect(LAYER_CTRL_SPACE, LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE), LAYER_CTRL_SIZE, LAYER_CTRL_SIZE));
		}
		
		//selection icon
		{
			if (m_pImageList)
			{
				E_LayerPropertyIcon icon = data.pLayer->GetLayerFlags().selectable ? E_LayerPropertyIcon_Selectable : E_LayerPropertyIcon_Unselectable;
				m_pImageList->Draw(icon, dc, LAYER_CTRL_SPACE+LAYER_CTRL_SIZE+LAYER_CTRL_SPACE, LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE));
			}
			dc.DrawRectangle (wxRect(LAYER_CTRL_SPACE+LAYER_CTRL_SIZE+LAYER_CTRL_SPACE, LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE), LAYER_CTRL_SIZE, LAYER_CTRL_SIZE));
		}

		//colour swatch
		{
			Maths::C_RGBA_U8 colour = data.pLayer->GetDisplayColour();
			if (colour.A() == 0)
				dc.DrawLine(LAYER_CTRL_SPACE+2*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE), 
					LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE), 
					LAYER_CTRL_SPACE+2*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE)+LAYER_CTRL_SIZE, 
					LAYER_CTRL_SPACE+LAYER_CTRL_SIZE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE));
			else
				dc.SetBrush(wxColour(colour.R(), colour.G(), colour.B(), colour.A()));

			dc.DrawRectangle (wxRect(LAYER_CTRL_SPACE+2*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE),LAYER_CTRL_SPACE+i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE), LAYER_CTRL_SIZE, LAYER_CTRL_SIZE));
		}

#pragma push_macro("DrawText")
#undef DrawText
		dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

		Btl::String name;
		if (data.pLayer->GetLevel())
			name += data.pLayer->GetLevel()->GetName() + ":";
		name += data.pLayer->GetName();

		wxCoord w=0, h=0, d=0;
		U32 labelOffset = LAYER_CTRL_SPACE+E_CtrlType_Count*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE)+LAYER_LABEL_SPACE;
		dc.GetTextExtent(name.Chars(), &w, &h);
		h = LAYER_CTRL_SPACE+((LAYER_CTRL_SIZE-(h-d))>>1)-1;
		h += i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE);

		if (data.pLayer->GetType() == EditorLib::E_LayerType_Reference)
		{
			wxFont font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
			font.SetStyle(wxFONTSTYLE_ITALIC);
			dc.SetFont(font);
		}
		
		dc.DrawText(name.Chars(), labelOffset, h);
#pragma pop_macro("DrawText")
	}
}

void C_LayerPropertyCtrl::OnIdle(wxIdleEvent& e)
{
}

void C_LayerPropertyCtrl::OnTextLabelEraseBackground(wxEraseEvent& e)
{
	//do nothing
}

void C_LayerPropertyCtrl::OnTextLabelEditFocus(wxFocusEvent& e)
{
	if (e.GetEventType() == wxEVT_KILL_FOCUS)
	{
		assert(m_pCurrentLayer);
		m_pCurrentLayer->SetName(m_pTextCtrl->GetValue().c_str());
		m_pTextCtrl->Show(false);
		Refresh();
	}
}

void C_LayerPropertyCtrl::OnTextLabelEditEnd(wxCommandEvent& e)
{
	assert(m_pCurrentLayer);
	m_pCurrentLayer->SetName(m_pTextCtrl->GetValue().c_str());
	m_pTextCtrl->Show(false);
}

wxSize C_LayerPropertyCtrl::DoGetBestSize() const
{
	wxCoord w=0, h=0;
	{
		wxCoord border;
		switch(GetBorder())
		{
		case wxBORDER_STATIC:
		case wxBORDER_SIMPLE:
			border = 1; break;
		case wxBORDER_SUNKEN:
			border = 2; break;
		case wxBORDER_DOUBLE:
			border = 3; break;
		case wxBORDER_NONE:
		default:
			border = 0;
		};

		w += E_CtrlType_Count*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE) + LAYER_LABEL_SPACE + 2*border;
		h += m_layerDataArray.Size()*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE) + 2*border;	
	}

	{
		wxCoord maxTextWidth = 0;
		for (S32 i=0; i<m_layerDataArray.Size(); ++i)
		{
			wxClientDC dc(wx_const_cast(C_LayerPropertyCtrl*, this));
			dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

			Btl::String name;
			if (m_layerDataArray[i].pLayer->GetLevel())
				name += m_layerDataArray[i].pLayer->GetLevel()->GetName() + ":";
			name += m_layerDataArray[i].pLayer->GetName();
			
			wxCoord w1=0, h1=0;
			dc.GetTextExtent(name.Chars(), &w1, &h1);	
			maxTextWidth = Maths::Max(maxTextWidth, w1 + 2*LAYER_CTRL_SPACE);
		}
		w += maxTextWidth;
	}

	wxSize bestSize(w, h);
	CacheBestSize(bestSize);

	return bestSize;
}

void C_LayerPropertyCtrl::Shutdown()
{
	if (m_flags.connected)
	{
		m_layerDataArray.Clear();
		m_slots.Disconnect();
		m_flags.connected = false;
	}
}

void C_LayerPropertyCtrl::EditLayerName()
{
	if (m_pCurrentLayer)
		EditLayerName(m_pCurrentLayer);
	else
		LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::EditLayerName(const EditorLib::I_Layer *pLayer)
{
	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
	{
		if (m_layerDataArray[i].pLayer == pLayer)
		{
			m_pCurrentLayer = m_layerDataArray[i].pLayer;
			U32 x = LAYER_CTRL_SPACE+E_CtrlType_Count*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE)+4;
			U32 y = i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE)+4;

			//set appropriate background colour
			if (m_layerDataArray[i].selected)
				m_pTextCtrl->SetBackgroundColour(wxColour(120,117,112,255));
			else
				m_pTextCtrl->SetBackgroundColour(*wxWHITE);
				//::SendMessage(GetHwnd(), TVM_SETBKCOLOR, 0, -1);

			m_pTextCtrl->SetValue(pLayer->GetName().Chars());
			m_pTextCtrl->SetSize(x, y, GetSize().x-x, LAYER_CTRL_SIZE, wxSIZE_AUTO);
			m_pTextCtrl->SetSelection(-1,-1);
			//m_pTextCtrl->SetInsertionPointEnd();
			m_pTextCtrl->InvalidateBestSize();
			m_pTextCtrl->Show();
			m_pTextCtrl->SetFocus();

			return;
		}
	}

	LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::SetSelectedState(const EditorLib::I_Layer *pLayer, bool state)
{
	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
	{
		if (m_layerDataArray[i].pLayer == pLayer)
		{
			m_layerDataArray[i].selected = state;
			return;
		}
	}

	LOGERROR(g_Log, "Failed to find layer data");
}

bool C_LayerPropertyCtrl::GetSelectedState(const EditorLib::I_Layer* pLayer) const
{
	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
		if (m_layerDataArray[i].pLayer == pLayer)
			return m_layerDataArray[i].selected;

	LOGERROR(g_Log, "Failed to find layer data");
	return false;
}

const C_LayerPropertyCtrl::LayerDataArray& C_LayerPropertyCtrl::GetLayerData() const
{
	return m_layerDataArray;
}

C_LayerPropertyCtrl::C_HitTestResult C_LayerPropertyCtrl::HitTest(wxCoord x, wxCoord y) const
{
	C_HitTestResult result = {-1, NULL, E_CtrlType_Unknown};

	if (!m_layerDataArray.Size() ||
		y<=0 || y>=m_layerDataArray.Size()*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE))
		return result;

	S32 index = y/(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE+LAYER_CTRL_SPACE);
	assert(index < m_layerDataArray.Size());
	result.index = index;
	result.pLayer = m_layerDataArray[index].pLayer;

	for (S32 i=0; i<E_CtrlType_Count; ++i)
	{
		S32 xx = LAYER_CTRL_SPACE + i*(LAYER_CTRL_SIZE+LAYER_CTRL_SPACE);
		if (x > xx && x < xx+LAYER_CTRL_SIZE)
			result.ctrlType = (E_CtrlType) i;
	}
	
	return result;
}

void C_LayerPropertyCtrl::OnLevelOpening(const EditorLib::I_Level& level)
{
	m_flags.pendingSync = true;
}

void C_LayerPropertyCtrl::OnLevelOpened(const EditorLib::I_Level& level)
{
	assert(m_flags.pendingSync);
	m_flags.pendingSync = false;
}

void C_LayerPropertyCtrl::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_flags.pendingSync = true;
}

void C_LayerPropertyCtrl::OnLevelClosed(const EditorLib::I_Level& level)
{
	m_flags.pendingSync = false;
}

void C_LayerPropertyCtrl::OnLevelLayerAdded(const EditorLib::I_Layer& layer)
{
	C_LayerData data = {const_cast<EditorLib::I_Layer*>(&layer), false};
	m_layerDataArray.PushBack(data);

	m_pCurrentLayer = data.pLayer;

	InvalidateBestSize();
	GetParent()->SetVirtualSize(DoGetBestSize());
	Refresh();
}

void C_LayerPropertyCtrl::OnLevelLayerRemoved(const EditorLib::I_Layer& layer)
{
	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
		if (m_layerDataArray[i].pLayer == &layer)
			m_layerDataArray.EraseByIndex(i);

	if (m_lastSelection >= m_layerDataArray.Size())
		m_lastSelection = -1;

	if (m_pCurrentLayer == &layer)
	{
		m_pCurrentLayer = NULL;
		m_pTextCtrl->Show(false);
	}

	InvalidateBestSize();
	GetParent()->SetVirtualSize(DoGetBestSize());
	Refresh();
}

void C_LayerPropertyCtrl::OnLayerDeserialised(const EditorLib::I_Layer& layer)
{
	bool found = false;
	for (S32 i=0; i<m_layerDataArray.Size(); ++i)
		if (m_layerDataArray[i].pLayer == &layer)
			found = true;

	if (!found)
		OnLevelLayerAdded(layer);

	InvalidateBestSize();
	GetParent()->SetVirtualSize(DoGetBestSize());
	Refresh();
}

void C_LayerPropertyCtrl::OnShowContextMenu()
{
	if (!m_pContextMenu)
	{
		m_pContextMenu = new wxMenu();
		wxMenuItem *pItem = 0;

		pItem = m_pContextMenu->Append(wxID_ANY, "Edit Layer Name");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &C_LayerPropertyCtrl::EditLayerName, this, false, pItem->GetId());
		wxConnect<wxEVENT_UPDATE_UI>(m_pContextMenu, &C_LayerPropertyCtrl::OnEditLayerNameUpdate, this, pItem->GetId());
		pItem = m_pContextMenu->Append(wxID_ANY, "Select Objects in Layer");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &C_LayerPropertyCtrl::OnSelectLayerObjects, this, false, pItem->GetId());

		m_pContextMenu->AppendSeparator();

		pItem = m_pContextMenu->Append(wxID_ANY, "Add Selected Objects to Layer");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &C_LayerPropertyCtrl::OnAddToLayer, this, false, pItem->GetId());
		wxConnect<wxEVENT_UPDATE_UI>(m_pContextMenu, &C_LayerPropertyCtrl::OnAddToLayerUpdate, this, pItem->GetId());

		pItem = m_pContextMenu->Append(wxID_ANY, "Remove Selected Objects from Layer");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &C_LayerPropertyCtrl::OnRemoveFromLayer, this, false, pItem->GetId());
		wxConnect<wxEVENT_UPDATE_UI>(m_pContextMenu, &C_LayerPropertyCtrl::OnRemoveFromLayerUpdate, this, pItem->GetId());

		m_pContextMenu->AppendSeparator();

		pItem = m_pContextMenu->Append(wxID_ANY, "Delete Layer");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &C_LayerPropertyCtrl::OnDeleteLayer, this, false, pItem->GetId());
		pItem = m_pContextMenu->Append(wxID_ANY, "Delete All Unused Layers");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &Tasks::RemoveUnusedLayers, false, pItem->GetId());

		m_pContextMenu->AppendSeparator();

		pItem = m_pContextMenu->Append(wxID_ANY, "Membership...");
		wxConnect<wxEVENT_COMMAND_MENU_SELECTED>(m_pContextMenu, &Tasks::ShowLayerEditWindow, false, pItem->GetId());
	}
	m_pContextMenu->UpdateUI();
	PopupMenu(m_pContextMenu);
}

void C_LayerPropertyCtrl::OnSelectLayerObjects()
{
	if (m_pCurrentLayer)
	{
		Ut::Vector<const EditorLib::I_Layer*> layerArray;
		layerArray.PushBack(m_pCurrentLayer);
		Tasks::SetSelectionFromLayers(layerArray, wxGetKeyState(WXK_SHIFT) ? "-a" : wxGetKeyState(WXK_CONTROL) ? "-r" : 0);
	}
	else
		LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::OnAddToLayer()
{
	if (m_pCurrentLayer)
		Tasks::AddSelectionToLayer(m_pCurrentLayer);
	else
		LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::OnRemoveFromLayer()
{
	if (m_pCurrentLayer)
	{
		Ut::Vector<EditorLib::I_Layer*> layerArray;
		layerArray.PushBack(m_pCurrentLayer);
		Tasks::RemoveSelectionFromLayers(layerArray);
	}
	else
		LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::OnDeleteLayer()
{
	if (m_pCurrentLayer)
		m_pCurrentLayer->GetLayerSet()->RemoveLayer(m_pCurrentLayer);
	else
		LOGERROR(g_Log, "Failed to find layer data");
}

void C_LayerPropertyCtrl::OnEditLayerNameUpdate(wxUpdateUIEvent& e)
{
	if (m_pCurrentLayer)
		e.Enable(m_pCurrentLayer->GetType() == EditorLib::E_LayerType_Actor);
	else
	{
		LOGERROR(g_Log, "Failed to find layer data");
		e.Enable(false);
	}
}

void C_LayerPropertyCtrl::OnAddToLayerUpdate(wxUpdateUIEvent& e)
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();
	e.Enable(items.Size() > 0);
}

void C_LayerPropertyCtrl::OnRemoveFromLayerUpdate(wxUpdateUIEvent& e)
{
	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();
	e.Enable(items.Size() > 0);
}

IMPLEMENT_CLASS(C_LayerPropertyPanel, wxPanel)
C_LayerPropertyPanel::C_LayerPropertyPanel(wxWindow *parent, wxWindowID winid, const wxPoint &pos, const wxSize &size, long style, const wxString &name)
: wxPanel(parent, winid, pos, size, style, name)
{
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	wxBoxSizer* pSizer = new wxBoxSizer(wxVERTICAL);

	SetSizer(pSizer);

	m_pMenuBar = new C_LayerPropertyMenuBar(this);
	m_pMenuBar->InitialiseUI();

	wxStaticLine *pLine = new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL);
	pSizer->Add(pLine, 0, wxEXPAND|wxALL, 1);

	wxScrolledWindow *pScrollWindow = new wxScrolledWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxScrolledWindowStyle | wxNO_BORDER);
	wxBoxSizer *pScrollWindowSizer = new wxBoxSizer(wxVERTICAL);
	pScrollWindow->SetSizer(pScrollWindowSizer);
	pScrollWindow->SetScrollRate(10,10);

	m_pLayerPropCtrl = new C_LayerPropertyCtrl(pScrollWindow, wxID_ANY);
	pScrollWindowSizer->Add(m_pLayerPropCtrl, 1, wxALL|wxEXPAND, 0);
	pSizer->Add(pScrollWindow, 1, wxALL|wxEXPAND, 0);
}

C_LayerPropertyPanel::~C_LayerPropertyPanel()
{
	delete m_pMenuBar;
}

C_LayerPropertyCtrl* C_LayerPropertyPanel::GetLayerPropertyCtrl()
{
	return m_pLayerPropCtrl;
}

void C_LayerPropertyPanel::Shutdown()
{
	m_pLayerPropCtrl->Shutdown();
}

}