#include "stdafx.h"

#include "Factory.h"

#include "ParticleEmitter.h"
#include "../Struct.h"
#include "Xt/XtTool.h"

using namespace RecordGraph;

C_StructNode* C_CustomNodeFactory::GetNode(struct LfStruct* field)
{
	if(field->type->name == "Particle_Emitter")
		return new C_ParticleEmitterNode(field);
	else
		return new C_StructNode(field);

}
