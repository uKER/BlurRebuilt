#ifndef RECORD_GRAPH_NODE_TYPE_H
#define RECORD_GRAPH_NODE_TYPE_H

#pragma once

enum E_NodeType
{
	// generic
	E_NodeType_Generic,

	// xt dom elements
	E_NodeType_Record,
	E_NodeType_Array,
	E_NodeType_ArrayItem,
	E_NodeType_Struct,
	E_NodeType_WeakPtr,
	E_NodeType_ParticleEvent,
	E_NodeType_ParticleEmitter,

	// other helper symbols and icons
	E_NodeType_HideIcon,
	E_NodeType_CloseIcon,
	E_NodeType_AddIcon,
	E_NodeType_WeakSocket
};

#endif // RECORD_GRAPH_NODE_TYPE_H
