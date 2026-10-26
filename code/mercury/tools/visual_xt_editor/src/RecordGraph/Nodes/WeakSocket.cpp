#include "stdafx.h"

#include "WeakSocket.h"
#include "WeakPtr.h"
#include "Struct.h"
#include "Type.h"

#include "../Graph.h"
#include "../Visitor.h"

using namespace RecordGraph;

// event node
C_WeakSocket::C_WeakSocket(LfWeakPtr* pWeakPtr, Xt::DomField* pField) : m_pWeakPtr(pWeakPtr), m_pField(pField), m_pWeakPtrNode(NULL), I_Node(E_NodeType_WeakSocket)
{
	m_Size = wxSize(8, 8);
	m_bDraggable = true;
}

C_WeakSocket::~C_WeakSocket()
{
}

bool C_WeakSocket::SetSelected(const bool bSelected)
{
	return m_pParent->SetSelected(bSelected);
}

bool C_WeakSocket::EndDrag(I_Node* pTarget, wxPoint point)
{
	if(!pTarget)
	{
		m_pGraph->SelectNode(NULL);
		
		Xt::Set(m_pField, "");
	}
	else
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
					m_pGraph->SelectNode(pTarget);
					
					Xt::Set(m_pField, id);
				}
			}
		}
	}

	return false;
}

void C_WeakSocket::Draw(wxDC& dc) const
{
	PrepareDC(dc);
	
	// draw window
	{
		const wxPen& pen = dc.GetPen();
		S32 shift = pen.GetWidth() - 1;
		dc.DrawRectangle(m_Position.x + shift, m_Position.y + shift, m_Size.x - shift, m_Size.y - shift);
	}
}

void C_WeakSocket::Accept(I_NodeVisitor* pVisitor)
{
	// assume visitor is a record graph visitor, any generic visitor won't work!
	I_RecordGraphVisitor* pRecordGraphVisitor = (I_RecordGraphVisitor*)pVisitor;
	pRecordGraphVisitor->Visit(this);
}

