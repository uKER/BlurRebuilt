#pragma once

class C_ParticleConverter
{
public:
	static bool Convert(
		DynamicEffect::Particle::C_UserParticleContainer& output,
		const Aftk::C_Scene& aftkScene,
		const C_Options& options);

private:
	static bool ConvertVariants(
		DynamicEffect::Particle::C_UserShape3DVariants& variants,
		const Aftk::C_SceneNode& aftkVariantParent,
		const C_Options& options);

	static bool TraverseVariants3DParticleMesh(
		DynamicEffect::Particle::C_UserShape3DVariants& variants,
		const Aftk::C_SceneNode& aftkVariantParent,
		const C_Options& options);

	static bool FindVariantsExtent(
		F32& extent,
		const Aftk::C_SceneNode& aftkVariantParent,
		const C_Options& options);

	static bool Convert3DParticle(
		DynamicEffect::Particle::C_UserShape3DData& output,
		U32& numVertices,
		const F32 inverseScale,
		const Aftk::C_SceneMeshNode& aftkParticleMesh,
		const C_Options& options);

	static bool IsVariantParent(const Aftk::C_SceneNode& aftkSceneNode);
};


