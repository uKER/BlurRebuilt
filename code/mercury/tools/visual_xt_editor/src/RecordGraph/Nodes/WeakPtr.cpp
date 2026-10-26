#include "stdafx.h"

#include "WeakPtr.h"
#include "WeakSocket.h"
#include "Struct.h"
#include "../Graph.h"

#include "WeakConnection.h"

using namespace RecordGraph;

C_WeakPtrNode::C_WeakPtrNode(LfWeakPtr* pWeakPtr, C_NodeGraph* pGraph) :
	C_GenericNode(E_NodeType_WeakPtr),
	m_pWeakPtr(pWeakPtr),
	m_pWeakSocket(NULL)
{
	m_Colour = wxColour(128, 128, 128);
	m_Size = wxSize(8, 8);

	m_bDrawConnection = false;

	m_DomMonitor.SetCallback(DomCallback, this);

	m_pGraph = pGraph;
}

C_WeakPtrNode::~C_WeakPtrNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

void C_WeakPtrNode::OnBind(Xt::DomBlock* parentDom)
{
	SetDom(Xt::AcquireField(parentDom, m_pWeakPtr->name));
}

void C_WeakPtrNode::OnBind(Xt::DomArray* parentDom, int index)
{
	SetDom(Xt::AcquireField(parentDom, index));
}

void C_WeakPtrNode::SetDom(Xt::DomField* pField)
{
	Xt::Monitor(&m_DomMonitor, pField);

	RefreshControls();
}

void C_WeakPtrNode::OnLayout(S32 index)
{
	wxPoint point;
	if(GetParent()->GetType() == E_NodeType_ArrayItem)
	{
		if(GetParent()->GetAlignment() == E_NodeAlignment_Vertical)
		{
			point.x = GetParent()->GetNumChildren(E_NodeType_WeakPtr, index) * GetSize().GetWidth();
			point.y = GetParent()->GetSize().GetHeight();
		}
		else
		{
			point.x = GetSize().GetWidth();
			point.y = GetParent()->GetNumChildren(E_NodeType_WeakPtr, index) * GetParent()->GetSize().GetHeight();
		}
	}
	else
	{
		point.x = GetParent()->GetSize().GetWidth();
		point.y = GetParent()->GetNumChildren(E_NodeType_WeakPtr, index) * GetParent()->GetSize().GetHeight();
	}
	SetPosition(point);

	C_GenericNode::OnLayout(index);
}

void C_WeakPtrNode::OnLayout2(S32 index)
{
	// visit the entire node tree and looking for matching weak sockets
	m_pWeakSocket = NULL;
	C_WeakConnection connection(this);
	C_Graph* pGraph = (C_Graph*)m_pGraph;
	C_GenericNode* pRoot = pGraph->GetRootNode();
	pRoot->Accept(&connection);

	C_GenericNode::OnLayout2(index);
}

// store in dom block
void C_WeakPtrNode::RefreshControls()
{
	
}

bool C_WeakPtrNode::EndDrag(I_Node* pTarget, wxPoint point)
{
	if(pTarget)
	{
		C_GenericNode* pGenericNode = (C_GenericNode*)pTarget;

		// check if target is a struct
		if(pGenericNode->GetType() == E_NodeType_Struct)
		{
			C_StructNode* pStructNode = (C_StructNode*)pGenericNode;

			const LfStruct* pLfStruct = pStructNode->GetLfStruct();
			const LtStruct* pLtStruct = pLfStruct->type;

			// see if struct types match
			if(pLtStruct == m_pWeakPtr->type)
			{
				// get id from target block
				if(Xt::DomBlock* pBlock = Xt::AsBlock(pStructNode->GetDomMonitor()))
				{
					XtString id = Xt::AsText(At(pBlock, "__id"));
					
					// store id in weak ptr
					if(Xt::DomField* pField = Xt::AsField(m_DomMonitor))
					{
						m_pGraph->SelectNode(pTarget);
						Xt::Set(pField, id);					
					}
				}
			}
		}
	}

	return false;
}

void C_WeakPtrNode::Draw(wxDC& dc) const
{
	if(!m_bVisible)
		return;

	PrepareDC(dc);
	
	// draw window
	{
		const wxPen& pen = dc.GetPen();
		S32 shift = pen.GetWidth() - 1;
		dc.DrawRectangle(m_Position.x + shift, m_Position.y + shift, m_Size.x - shift, m_Size.y - shift);
	}

	if(m_pWeakSocket)
	{
		S32 x, y;
		dc.GetDeviceOrigin(&x, &y);
		dc.SetDeviceOrigin(0, 0);

		wxPen pen;
		pen.SetColour(0, 0, 0);
		pen.SetWidth(2);

		dc.SetPen(pen);

		wxPoint position = GetPositionAbsolute();

		wxPoint start0(position.x + m_Size.GetWidth(), position.y + m_Size.GetHeight() / 2);
		wxPoint start1(position.x + m_Size.GetWidth() + 10, position.y + m_Size.GetHeight() / 2);
		dc.DrawLine(start0, start1);
		
		wxPoint end1(m_pWeakSocket->GetPositionAbsolute());
		end1.y += m_pWeakSocket->GetSize().GetHeight() / 2;
		
		wxPoint end0(end1);
		end0.x -= 10;
			
		dc.DrawLine(start1, end0);
		dc.DrawLine(end0, end1);

		// restore device origin
		dc.SetDeviceOrigin(x, y);
	}
}

void C_WeakPtrNode::DomCallback(Xt::DomMonitor* pMonitor, Xt::DomEvent* pEvent, C_WeakPtrNode* pNode)
{
	C_Graph* pGraph = (C_Graph*)pNode->GetGraph();
	pGraph->LayoutNodes();
	pGraph->Refresh();
}