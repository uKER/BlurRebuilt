#ifndef NEXUS_RENDER_MESH_EFFECT_CONSTANTS_BUILDER_H
#define NEXUS_RENDER_MESH_EFFECT_CONSTANTS_BUILDER_H

namespace NexusRender
{
	class MeshEffectConstantsBuilder
	{
	public:
		static Tu::Status MeshEffectConstantsBuilder::Build(
			EffectConstants** ppEffectConstants, 
			const Aftk::C_MaterialNode* pAftkMaterial);
	};
}

#endif