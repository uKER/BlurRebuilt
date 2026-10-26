#include "stdafx.h"

#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/Editor/manipulators/contexts/IManipulatorContext.h>
#include <EditorApp/Editor/manipulators/Manipulator.h>
#include <EditorApp/Editor/manipulators/SelectManipulator.h>
#include <EditorApp/Components/ManipulatorPropertyPanel.h>

#include <EditorApp/Editor/RenderViewport.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Tasks/LevelTasks.h>
#include <EditorApp/Tasks/UITasks.h>

#include <EditorApp/App.h>

using namespace Editor;

E_SelectionMode C_SelectManipulator::m_SelectionMode = E_SelectionMode_Object;
SelectionMask	C_SelectManipulator::m_SelectionMask;

BEGIN_EVENT_TABLE(C_SelectManipulator, I_Manipulator)
  EVT_MOUSE_EVENTS(C_SelectManipulator::OnMouseEvent)
  EVT_MOUSE_CAPTURE_LOST(C_SelectManipulator::OnMouseCaptureLostEvent)
END_EVENT_TABLE()
C_SelectManipulator::C_SelectManipulator()
: m_Flags(0)
{
}

C_SelectManipulator::~C_SelectManipulator()
{
}

E_ManipulatorType C_SelectManipulator::GetType() const
{
	return E_ManipulatorType_Select;
}

bool C_SelectManipulator::IsA(E_ManipulatorType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == C_SelectManipulator::GetType()) 
		return true;

	return false;
}

bool C_SelectManipulator::IsActive() const
{
	return m_Flags & Active;
}

E_SelectionMode C_SelectManipulator::GetSelectionMode()
{
	return m_SelectionMode;
}

void C_SelectManipulator::SetSelectionMode(E_SelectionMode mode)
{
	m_SelectionMode = mode;
	C_ManipulatorPropertyPanel *pPropertyPanel = wxGetApp().GetManipulatorPropertyPanel();
	if (pPropertyPanel)
		pPropertyPanel->RefreshProperties();
}

SelectionMask C_SelectManipulator::GetSelectionMask()
{
	return m_SelectionMask;
}

void C_SelectManipulator::SetSelectionMask(SelectionMask mask)
{
	m_SelectionMask = mask;
	C_ManipulatorPropertyPanel *pPropertyPanel = wxGetApp().GetManipulatorPropertyPanel();
	if (pPropertyPanel)
		pPropertyPanel->RefreshProperties();
}

void C_SelectManipulator::AttachToViewport(C_RenderViewport *pViewport)
{
	if (m_pViewport && wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	I_Manipulator::AttachToViewport(pViewport);
	m_Flags = 0;
}

void C_SelectManipulator::OnMouseEvent(wxMouseEvent &e)
{
	if (e.LeftDown() && !wxWindow::GetCapture())
	{
		m_pViewport->CaptureMouse();
		m_StartPosition = e.GetPosition();
		m_EndPosition = e.GetPosition();
		m_Flags = Active;
	}

	else if (e.Dragging() && (m_Flags & Active))
	{
		if (m_Flags & Dirty)
			DrawSelectionRect();//removes previously drawn rectangle

		m_EndPosition.x = Maths::Clamp(e.GetPosition().x, 0, m_pViewport->GetSize().x-1);
		m_EndPosition.y = Maths::Clamp(e.GetPosition().y, 0, m_pViewport->GetSize().y-1);
		DrawSelectionRect();
	}
	else if (e.LeftUp() && (m_Flags & Active))
	{
		if (m_Flags & Dirty)
			DrawSelectionRect();//removes previously drawn rectangle

		m_Flags = 0;
		m_EndPosition.x = Maths::Clamp(e.GetPosition().x, 0, m_pViewport->GetSize().x-1);
		m_EndPosition.y = Maths::Clamp(e.GetPosition().y, 0, m_pViewport->GetSize().y-1);
		if (wxWindow::GetCapture() == m_pViewport)
			m_pViewport->ReleaseMouse();

		if (!Tasks::IsLevelLoaded())
			return e.Skip();

		wxPoint minPoint(Maths::Min(m_StartPosition.x, m_EndPosition.x), Maths::Min(m_StartPosition.y, m_EndPosition.y));
		wxPoint maxPoint(Maths::Max(m_StartPosition.x, m_EndPosition.x), Maths::Max(m_StartPosition.y, m_EndPosition.y));

		wxPoint size = maxPoint-minPoint;
		
		C_DgNodeSelection selectionCandidates;

		bool marqueeSelect = false;
		{
			RECT rect;
			//Single click.
			if (m_StartPosition == m_EndPosition)
			{
				RECT r={minPoint.x, minPoint.y, maxPoint.x+1, maxPoint.y+1};
				rect = r;
			}
			//Marquee selection
			else
			{
				marqueeSelect = true;
				RECT r={minPoint.x, minPoint.y, maxPoint.x, maxPoint.y};
				rect = r;
			}
			if (size.x < 2)
			{
				rect.left	-= 1;
				rect.right	+= 1;
			}

			if (size.y < 2)
			{
				rect.top	-= 1;
				rect.bottom	+= 1;
			}

			Display::Device()->SetView(NULL, false);
			if (Display::Device()->BeginScene())
			{
				m_pViewport->GetViewContext().BeginRender();			
				C_RenderCore::Instance().GetRenderPipeline().Select(rect, selectionCandidates);

				Display::Device()->SetView(NULL, false);
				Display::Device()->EndScene();
				m_pViewport->GetViewContext().EndRender(false);
			}
		}

		C_DgNodeSelection::ItemArray& candidateItems = const_cast<C_DgNodeSelection::ItemArray&>(selectionCandidates.GetItems());
		if (candidateItems.Size())
		{
			//In component selection mode we look to see if our candidate list contains the currently selected
			//component object. If it doesn't then we clear the selection. If it does then we reject all
			//candidates other than the component object and process its component selection.
			if (GetSelectionMode() == E_SelectionMode_Component)
			{
				//Get current selection.
				EditorLib::C_Selection selection;
				EditorLib::C_Core::Instance().GetSelection(selection);

				{
					Btl::String selectedNodeIdStr;
					const Ut::Vector<U32>* pNewComponents;

					// If nothing is currently selected, add the new item
					if (candidateItems.Size() == 0)
					{
						// Find the first item with components, otherwise use the first selectable item
						C_DgNodeSelection::C_Item *pItem = 0;
						for (C_DgNodeSelection::ItemArray::ReverseIterator it=candidateItems.ReverseBegin(); it!=candidateItems.ReverseEnd(); ++it)
						{
							if (!pItem && IsSelectable(it->pDgNode))
								pItem = &(*it);

							if (IsSelectable(it->pDgNode) && it->components.Size())
							{
								pItem = &(*it);
								break;
							}
						}

						if (!pItem)
						{
							if (m_Flags & Dirty)
								DrawSelectionRect();	//clear selection rect
							return;
						}

						pNewComponents = &pItem->components;
						selectedNodeIdStr = pItem->pDgNode->GetId().AsString();
					}
					else
					{
						bool componentsSelected = false;

						// Find the first item with components, otherwise use the first selectable item
						C_DgNodeSelection::C_Item *pItem = 0;
						for (C_DgNodeSelection::ItemArray::ReverseIterator it=candidateItems.ReverseBegin(); it!=candidateItems.ReverseEnd(); ++it)
						{
							//use the first selection candidate that has components and is already in the selection...
							if (selection.Find(it->pDgNode->GetId()) && it->components.Size())
							{
								pItem = &(*it);
								break;
							}
							
							if (!componentsSelected && IsSelectable(it->pDgNode))
							{
								if (it->components.Size())
								{
									//candidate has components selected...use it
									pItem = &(*it);
									componentsSelected = true;
								}
								else if (!pItem)
								{
									//possibly use this candidate...although we prefer one with components selected...
									pItem = &(*it);
								}
							}
						}

						if (!pItem)
						{
							if (m_Flags & Dirty)
								DrawSelectionRect();	//clear selection rect
							return;
						}

						pNewComponents = &pItem->components;
						selectedNodeIdStr = pItem->pDgNode->GetId().AsString();
					}

					//Build component index string.
					Btl::String componentIndexStr;
					for (S32 i = 0; i < pNewComponents->Size(); i++)
					{
						Btl::String tmp;
						tmp.Printf("%u,", (*pNewComponents)[i]);
						componentIndexStr = componentIndexStr + tmp;
					}
					//Crappy way of trimming whitespace. Could do with tidying this.
					if (componentIndexStr.Length() > 0)
						componentIndexStr = componentIndexStr(0, componentIndexStr.Length()-1);

					//If we've found a candidate object already in our selection.
					if (pNewComponents->Size())
					{
						//Found component object - reject all others.
						Btl::String args;
						if (e.ShiftDown() && e.ControlDown())
						{
							// Add components.
							args.Printf("%s -i %s -a", 
								selectedNodeIdStr.Chars(), componentIndexStr.Chars());
						}
						else if (e.ShiftDown())
						{
							//Toggle components.
							args.Printf("%s -i %s -t", 
								selectedNodeIdStr.Chars(), componentIndexStr.Chars());
						}
						else if (e.ControlDown())
						{
							//Deselect components.
							args.Printf("%s -i %s -r", 
								selectedNodeIdStr.Chars(), componentIndexStr.Chars());
						}
						else
						{
							//Set selected components.
							args.Printf("%s -i %s", 
								selectedNodeIdStr.Chars(), componentIndexStr.Chars());
						}
						EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", args);
					}
					//Set selected object to the last one in the candidate list.
					else
					{
						EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", selectedNodeIdStr);
					}
				}
			}
			//Object level selection.
			else
			{
				Btl::String objIdStr;
				const C_DgNodeSelection::ItemArray& candidateItems = selectionCandidates.GetItems();
				for (C_DgNodeSelection::ItemArray::ConstIterator it=candidateItems.Begin(); it!=candidateItems.End(); ++it)
				{
					if (!IsSelectable(it->pDgNode))
						continue;

					//Build object id list.
					Btl::String tmp;
					tmp.Printf("%s,", it->pDgNode->GetId().AsString().Chars());
					objIdStr = objIdStr + tmp;

					//only select one object if not in marquee selection mode
					//this seems to be what maya does...
					if (!marqueeSelect)
						break;
				}

				if (objIdStr.Length() == 0)
				{
					//no objects to select
					if (!e.ShiftDown() && !e.ControlDown())
						//unselect all...
						EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", "-c");
					else if (m_Flags & Dirty)
						DrawSelectionRect();	//clear selection rect

					return;
				}

				//Crappy way of trimming whitespace. Could do with tidying this.
				objIdStr = objIdStr(0, objIdStr.Length()-1);
				
				Btl::String args;

				
				if (e.ShiftDown() && e.ControlDown())
				{
					// Add to selection.
					args = objIdStr + " -a";
				}
				else if (e.ShiftDown())
				{
					//Toggle selection.
					args = objIdStr + " -t";
				}
				else if (e.ControlDown())
				{
					//Deselect.
					args = objIdStr + " -r";
				}
				else
				{
					//Select
					args = objIdStr;
				}

				EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", args);
			}
		}
		//Nothing selected.
		else if (!e.ShiftDown() && !e.ControlDown())
		{
			EditorLib::C_Selection currentSelection;
			EditorLib::C_Core::Instance().GetSelection(currentSelection);
			if (currentSelection.GetItems().Size() != 0)
				//unselect all...
				EditorLib::C_Core::Instance().GetCommandProcessor().Execute("select", "-c");
		}
	}
	else if (e.ButtonDClick(wxMOUSE_BTN_LEFT))
	{
		EditorLib::C_Selection currentSelection;
		EditorLib::C_Core::Instance().GetSelection(currentSelection);

		if (currentSelection.GetItems().Size())
			Tasks::ShowNodePropertiesWindow();
	}
	else
		e.Skip();
}

void C_SelectManipulator::OnMouseCaptureLostEvent(wxMouseCaptureLostEvent &e)
{
	if (wxWindow::GetCapture() == m_pViewport)
		m_pViewport->ReleaseMouse();

	m_Flags = 0;
	e.Skip();
}

bool C_SelectManipulator::IsSelectable(const EditorLib::I_DgNode *pNode)
{
	if (pNode->IsA(EditorLib::E_ObjType_SceneAftkNode))
		return GetSelectionMask().mesh;
	else if (pNode->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
		return GetSelectionMask().property;
	else
		return GetSelectionMask().locator;
}

void C_SelectManipulator::Render(const C_RenderViewport *pViewport) const
{
	if (pViewport == m_pViewport && (m_Flags & Active))
		m_Flags = m_Flags & ~Dirty;
}

void C_SelectManipulator::DrawSelectionRect() const
{
	wxScreenDC dc;
	dc.SetLogicalFunction(wxINVERT);
	dc.SetPen(*wxGREY_PEN);
	dc.SetBrush(*wxTRANSPARENT_BRUSH);

	wxPoint points[5];
	points[0] = m_StartPosition;
	points[1] = wxPoint(m_EndPosition.x, m_StartPosition.y);
	points[2] = m_EndPosition;
	points[3] = wxPoint(m_StartPosition.x, m_EndPosition.y);
	points[4] = m_StartPosition;

	points[0] = m_pViewport->ClientToScreen(points[0]);
	points[1] = m_pViewport->ClientToScreen(points[1]);
	points[2] = m_pViewport->ClientToScreen(points[2]);
	points[3] = m_pViewport->ClientToScreen(points[3]);
	points[4] = m_pViewport->ClientToScreen(points[4]);

	dc.DrawLines(5, points);

	dc.SetLogicalFunction(wxCOPY);

	//toggle dirty bit
	m_Flags = (m_Flags & Dirty) ? m_Flags & ~Dirty : m_Flags | Dirty;
}