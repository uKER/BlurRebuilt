#include "stdafx.h"

#include "Generic.h"
#include "Embedded/Close.h"
#include "../Visitor.h"

#include "Array.h"

using namespace RecordGraph;

C_GenericNode* C_GenericNode::AddChild(C_GenericNode* pNode)
{
	if(pNode->GetParent())
		return NULL;

	I_ParentNode::AddChild(pNode);

	if(GetType() == E_NodeType_ArrayItem && pNode->GetType() != E_NodeType_WeakPtr)
	{
		C_CloseIcon* pCloseIcon = new C_CloseIcon;
		
		wxSize iconSize = pCloseIcon->GetSize();
		wxSize nodeSize = pNode->GetSize();
		pCloseIcon->SetPosition(wxPoint(nodeSize.x - iconSize.x - 4, 4));

		pNode->AddGenericChild(pCloseIcon);
	}

	return pNode;
}

C_GenericNode* C_GenericNode::GetChild(U32 index)
{
	return m_Children[index];
}



void C_GenericNode::Accept(I_RecordGraphVisitor* pVisitor)
{
	pVisitor->Visit(this);

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		m_Children[i]->Accept(pVisitor);
	}

	for(S32 i = 0; i < m_GenericChildren.Size(); ++i)
	{
		m_GenericChildren[i]->Accept(pVisitor);
	}
}

// Xt stuff

// virtuals

bool C_GenericNode::OnPopulate()
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if(!GetChild(i)->OnPopulate())
			return false;
	}

	return true;
}

bool C_GenericNode::OnBuild()
{
	return true;
}

void C_GenericNode::OnBind(Xt::DomField* parentDom)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom);
	}

}

void C_GenericNode::OnBind(Xt::DomBlock* parentDom)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom);
	}

}

void C_GenericNode::OnBind(Xt::DomArray* parentDom, int index)
{
	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Bind(parentDom, index);
	}
}

void C_GenericNode::OnLayout(S32 index)
{
	if(m_pParent)
	{
		if(m_pParent->GetType() == E_NodeType_ArrayItem)
		{
			C_ArrayNode* pArrayNode = (C_ArrayNode*)m_pParent->GetParent();
			if(pArrayNode->GetItemConnections() == E_ArrayItemConnections_Enabled)
				m_bDrawConnection = true;
			else
				m_bDrawConnection = false;
		}
	}
}

void C_GenericNode::OnLayout2(S32 index)
{
	GenerateBitmap();
}

// none virtuals

bool C_GenericNode::Populate()
{
	return OnPopulate();
}

bool C_GenericNode::Build()
{
	if (!m_bBuilt)
	{
		m_bBuilt = true;

		if (!OnBuild())
			return false;
	}

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		if (!GetChild(i)->Build())
			return false;
	}
	
	return true;
}

void C_GenericNode::Bind(Xt::DomField* parentDom)
{
	OnBind(parentDom);
}

void C_GenericNode::Bind(Xt::DomBlock* parentDom)
{
	OnBind(parentDom);
}

void C_GenericNode::Bind(Xt::DomArray* parentDom, int index)
{
	OnBind(parentDom, index);
}

void C_GenericNode::Layout(S32 index)
{
	OnLayout(index);

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Layout(i);
	}
}

void C_GenericNode::Layout2(S32 index)
{
	OnLayout2(index);

	for(S32 i = 0; i < m_Children.Size(); ++i)
	{
		GetChild(i)->Layout2(i);
	}
}


// UI stuff

bool C_GenericNode::EndDrag(I_Node* pTarget, wxPoint point)
{
	SetPositionAbsolute(point);
	return false;
}

void C_GenericNode::PrepareDC(wxDC& dc) const
{
	if(m_bSelected)
	{
		wxPen pen;
		pen.SetColour(255, 255, 0);
		pen.SetWidth(2);

		dc.SetPen(pen);
	}
	else
		dc.SetPen(*wxBLACK_PEN);


	wxColour colour;

	if(m_bDragDest)
		colour = wxColour(255, 0, 0);
	else
		colour = m_Colour;

	dc.SetBrush(wxBrush(colour));
}

void C_GenericNode::Draw(wxDC& dc) const
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

	// set device origin for children
	
	S32 x, y;
	dc.GetDeviceOrigin(&x, &y);
	dc.SetDeviceOrigin(x + m_Position.x, y + m_Position.y);

	for(int i = 0; i < m_GenericChildren.Size(); ++i)
	{
		m_GenericChildren[i]->Draw(dc);
	}
	
	if(m_bDrawChildren)
	{
		S32 numConnections = 0;
		for(int i = 0; i < m_Children.Size(); ++i)
		{
			if(m_Children[i]->IsVisible() && m_Children[i]->GetDrawConnection())
			{
				++numConnections;
			}
		}

		if(numConnections > 0)
		{
			wxPen pen;
			pen.SetColour(0, 0, 0);
			pen.SetWidth(2);

			dc.SetPen(pen);

			wxPoint start0, start1;

			if(m_Alignment == E_NodeAlignment_Vertical)
			{
				start0 = wxPoint(m_Size.GetWidth() / 2, m_Size.GetHeight());
				start1 = wxPoint(m_Size.GetWidth() / 2, m_Size.GetHeight() + 10);
			}
			else
			{
				start0 = wxPoint(m_Size.GetWidth(), m_Size.GetHeight() / 2);
				start1 = wxPoint(m_Size.GetWidth() + 10, m_Size.GetHeight() / 2);
			}
			dc.DrawLine(start0, start1);
			
			for(int i = 0; i < m_Children.Size(); ++i)
			{
				if(m_Children[i]->IsVisible() && m_Children[i]->GetDrawConnection())
				{
					wxPoint end0, end1;

					end1 = m_Children[i]->GetPosition();
					
					if(m_Alignment == E_NodeAlignment_Vertical)
					{
						end1.x += m_Children[i]->GetSize().GetWidth() / 2;
						end0 = end1;
						end0.y -= 10;
					}
					else
					{
						end1.y += m_Children[i]->GetSize().GetHeight() / 2;
						end0 = end1;
						end0.x -= 10;
					}
				
					dc.DrawLine(start1, end0);
					dc.DrawLine(end0, end1);
				}
			}
		}
		for(int i = 0; i < m_Children.Size(); ++i)
		{
			m_Children[i]->Draw(dc);
		}
	}

	// restore device origin
	dc.SetDeviceOrigin(x, y);
}
