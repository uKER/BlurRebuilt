#ifndef EVENT_GRAPH_NODE_H
#define EVENT_GRAPH_NODE_H

#pragma once

#include "../../NodeInterface.h"
#include "Xt/XtTool.h"

enum E_NodeType
{
	E_NodeType_Generic,
	E_NodeType_Record,
	E_NodeType_Array,
	E_NodeType_Struct,
	E_NodeType_ParticleEvent,
	E_NodeType_ParticleEmitter,

	E_NodeType_HideIcon,
};

class C_HideIcon : public I_Node
{
public:
	C_HideIcon();
	virtual ~C_HideIcon() {};

	void	Draw(wxDC& dc) const;
	bool	MouseClick();
	bool	SetSelected(const bool bSelected);
};

// similar to ScarabNode
class C_GenericNode : public I_ParentNode
{
public:

	C_GenericNode() : I_ParentNode(E_NodeType_Generic) { };
	C_GenericNode(S32 type) : I_ParentNode(type) { };
	virtual ~C_GenericNode() {};

	virtual void			PrepareDC(wxDC& dc) const;
    virtual void			Draw(wxDC& dc) const;
	
	virtual bool			EndDrag(I_Node* pTarget, wxPoint point);

public:
	bool Populate() { return OnPopulate(); }
	bool Build() { return OnBuild(); }
	void Bind(Xt::DomField* parentDom) {}
	void Bind(Xt::DomBlock* parentDom) {}
	void Bind(Xt::DomArray* parentDom, int index) {}

protected:

	virtual bool OnPopulate();
	virtual bool OnBuild() { return true; }
	virtual void OnBind(Xt::DomField* parentDom) {}
	virtual void OnBind(Xt::DomBlock* parentDom) {}
	virtual void OnBind(Xt::DomArray* parentDom, int index) {}

	wxColour				m_Colour;
};

bool BuildNode(C_GenericNode* pParentNode, LarvaField* pField);

// Generic classes

class C_RecordNode : public C_GenericNode
{
public:
	C_RecordNode() : C_GenericNode(E_NodeType_Record) {}
	C_RecordNode(LtStruct* pType);
protected:
	bool		OnPopulate();

	LtStruct*	m_pType;
};

class C_ArrayNode : public C_GenericNode
{
public:

	C_ArrayNode() : C_GenericNode(E_NodeType_Array) {};
	C_ArrayNode(LfArray* pArray);
	virtual ~C_ArrayNode() {};

	bool	OnPopulate();

protected:

	LfArray*	m_pArray;
};

class C_StructNode : public C_GenericNode
{
public:

	C_StructNode() : C_GenericNode(E_NodeType_Struct) {};
	C_StructNode(LfStruct* pStruct);
	virtual ~C_StructNode() {};

	bool	OnPopulate();

protected:

	LfStruct*	m_pStruct;
};










// SPECIFIC CLASSES


class C_EventNode : public C_GenericNode
{
public:

	C_EventNode() : C_GenericNode(E_NodeType_ParticleEvent) {};
	C_EventNode(Xt::DomElement* pDomElement);
	virtual ~C_EventNode() {};

protected:

	Xt::DomElement*			m_pDomElement;
};

class C_EmitterNode : public C_GenericNode
{
public:

	C_EmitterNode() : C_GenericNode(E_NodeType_ParticleEmitter) {};
	C_EmitterNode(Xt::DomElement* pDomElement);
	virtual ~C_EmitterNode() {};

protected:

	Xt::DomElement*			m_pDomElement;
};

//
// BuildNode
//



#endif // EVENT_GRAPH_NODE_H
