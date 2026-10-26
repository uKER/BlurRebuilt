#include "stdafx.h"

#include "Struct.h"
#include "../Graph.h"
#include "src/Record/Record.h"
#include "src/DynamicForm/Form.h"

#include "../../Common/Weak.h"
#include "WeakSocket.h"

using namespace RecordGraph;

C_StructNode::C_StructNode(LfStruct* pStruct, C_NodeGraph* pGraph) :
	C_GenericNode(E_NodeType_Struct),
	m_pStruct(pStruct)	
{
	m_Colour = wxColour(128, 128, 128);
	m_Size = wxSize(64, 64);

	m_pGraph = pGraph;
}

C_StructNode::~C_StructNode()
{
	Xt::Monitor(&m_DomMonitor, NULL);
}

bool C_StructNode::SetSelected(const bool bSelected)
{
	if(bSelected)
	{
		C_Graph* pGraph = (C_Graph*)m_pGraph;

		DynamicForm::C_Form* pForm = pGraph->GetRecord()->GetForm();

		if(Xt::DomBlock* pBlock = Xt::AsBlock(m_DomMonitor))
		{
			pForm->LoadBlock(pBlock, m_pStruct->type);
		}
	}

	return I_Node::SetSelected(bSelected);
}

bool C_StructNode::OnPopulate()
{
	for(XtUt::List<LarvaField>::Iterator it(&m_pStruct->type->fields); it; ++it)
	{
		if (it->hidden)
			continue;

		if (!BuildNode(this, it))
			return false;
	}

	return C_GenericNode::OnPopulate();
}

void C_StructNode::OnBind(Xt::DomBlock* parentDom)
{
	Xt::DomBlock* dom = Xt::AcquireBlock(parentDom, m_pStruct->name);
	Xt::Monitor(&m_DomMonitor, dom);
	//SetEnabled(dom != 0);

	RefreshControls();
	C_GenericNode::OnBind(dom);
}

void C_StructNode::OnBind(Xt::DomArray* parentDom, int index)
{
	Xt::DomBlock* dom = Xt::AcquireBlock(parentDom, index);
	Xt::Monitor(&m_DomMonitor, dom);
	//SetEnabled(dom != 0);

	RefreshControls();
	C_GenericNode::OnBind(dom);
}

void C_StructNode::OnLayout(S32 index)
{
	// weak ptrs might have changed so refresh controls
	RefreshControls();

	C_GenericNode::OnLayout(index);
}

// store in dom block
void C_StructNode::RefreshControls()
{
	if(Xt::DomBlock* pBlock = Xt::AsBlock(m_DomMonitor))
	{
		wxPoint point = GetPosition();

		if(Xt::DomField* pField = Xt::AcquireField(pBlock, "__x"))
			point.x = atoi((const char*)pField->value);
		if(Xt::DomField* pField = Xt::AcquireField(pBlock, "__y"))
			point.y = atoi((const char*)pField->value);

		if(point.x != 0 && point.y != 0)
			SetPosition(point);
	
		
		if(Xt::DomField* pField = Xt::AcquireField(pBlock, "__id"))
			m_StructId = pField->value;

		ClearGenericChildren(E_NodeType_WeakSocket);

		if (Xt::DomRecord* record = Xt::RecordOf(m_DomMonitor))
		{
			WeakContext wc(this, Xt::DbOf(record)->larvaSchema);

			if (LarvaType* type = XtUt::At(&wc.schema->types, record->type))
				wc.VisitType(type, &record->root, 0, 0);
		}
	
	}

	
}

bool C_StructNode::EndDrag(I_Node* pTarget, wxPoint point)
{
	SetPositionAbsolute(point);

	wxPoint relative = GetPosition();

	if(Xt::DomBlock* pBlock = Xt::AsBlock(m_DomMonitor))
	{
		Xt::DomField* pX = Xt::AcquireField(pBlock, "__x");
		Xt::DomField* pY = Xt::AcquireField(pBlock, "__y");

		XtString xValue;
		XtsFormat(xValue, 128, "%i", relative.x);
		Xt::Set(pX, xValue);

		XtString yValue;
		XtsFormat(yValue, 128, "%i", relative.y);
		Xt::Set(pY, yValue);
	}
	
	return false;
}

void C_StructNode::AddWeakPtr(LfWeakPtr* pWeakPtr, Xt::DomField* field)
{
	// check types are compatible
	if(IsBaseOf(pWeakPtr->type, m_pStruct->type))
	{
		C_WeakSocket* pWeakSocket = new C_WeakSocket(pWeakPtr, field);
							
		wxPoint point;
		point.x = -pWeakSocket->GetSize().GetWidth();
		point.y = pWeakSocket->GetSize().GetHeight() * GetNumGenericChildren(E_NodeType_WeakSocket, 1000);

		pWeakSocket->SetPosition(point);
		pWeakSocket->GenerateBitmap();
		
		AddGenericChild(pWeakSocket);
	}
}
