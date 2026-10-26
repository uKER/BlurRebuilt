#ifndef RECORD_GRAPH_NODE_H
#define RECORD_GRAPH_NODE_H

#pragma once

#include "Generic.h"
#include "Xt/XtTool.h"

namespace RecordGraph
{

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

}

#endif // RECORD_GRAPH_NODE_H
