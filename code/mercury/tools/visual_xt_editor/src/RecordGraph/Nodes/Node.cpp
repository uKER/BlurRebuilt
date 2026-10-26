#include "stdafx.h"

#include "Node.h"

using namespace RecordGraph;

C_EventNode::C_EventNode(Xt::DomElement* pDomElement) : C_GenericNode(E_NodeType_ParticleEvent), m_pDomElement(pDomElement)
{
	m_Colour = wxColour(128, 160, 128);
	m_Size = wxSize(128, 64);
	m_bDragRedraw = true;
}

C_EmitterNode::C_EmitterNode(Xt::DomElement* pDomElement) : C_GenericNode(E_NodeType_ParticleEmitter), m_pDomElement(pDomElement)
{
	m_Colour = wxColour(128, 128, 160);
	m_Size = wxSize(64, 64);
}



