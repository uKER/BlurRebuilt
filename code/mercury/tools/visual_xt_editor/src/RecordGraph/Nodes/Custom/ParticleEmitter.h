#ifndef RECORD_GRAPH_PARTICLE_EMITTER_NODE_H
#define RECORD_GRAPH_PARTICLE_EMITTER_NODE_H

#pragma once

#include "../Struct.h"

namespace RecordGraph
{

class C_ParticleEmitterNode : public C_StructNode
{
public:

	C_ParticleEmitterNode() : C_StructNode() {};
	C_ParticleEmitterNode(LfStruct* pStruct);
	virtual ~C_ParticleEmitterNode() {};

};

} // RecordGraph

#endif // RECORD_GRAPH_PARTICLE_EVENT_NODE_H
