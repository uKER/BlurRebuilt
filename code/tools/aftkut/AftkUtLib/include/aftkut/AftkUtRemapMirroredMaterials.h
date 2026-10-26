#ifndef AFTK_UT_REMAP_FLIPPED_MATERIALS_H
#define AFTK_UT_REMAP_FLIPPED_MATERIALS_H


#ifdef _MSC_VER_
#pragma once
#endif

namespace AftkUt
{
	class C_RemapMirroredMaterials
	{

		enum E_MirrorDirection
		{
			E_MirrorDirection_U,
			E_MirrorDirection_V
		};

		typedef Ut::UtMap<const Aftk::C_MaterialResource*, const Aftk::C_MaterialResource*> MaterialMapping;
		typedef Ut::UtMap<const Aftk::C_MaterialResource*, Aftk::C_SceneMeshNode*> BaseInstanceMapping;

	public:
		static Aftk::C_Status Remap(
			Aftk::C_Scene *pScene,
			const std::string& namediscriminator,
			const std::string& nameconvention1 = std::string("_left_"),
			const std::string& nameconvention2 = std::string("_right_"),
			E_MirrorDirection mirrorDirection = E_MirrorDirection_U,
			S16 texCoordSet = -1);

	private:
		static Aftk::C_Status BuildMaterialRemappingTable(
			Aftk::C_Scene *pScene,
			const std::string& namediscriminator,
			const std::string& nameconvention1,
			const std::string& nameconvention2,
			MaterialMapping& mapping);

		static Aftk::C_Status MirrorTextureCoords(
			Aftk::C_SceneMeshNode* pMeshNode,
			const Aftk::C_MaterialResource* pMaterialRes,
			E_MirrorDirection mirrorDirection = E_MirrorDirection_U,
			S16 texCoordSet = -1);

		static Aftk::C_Status MirrorTextureCoords(
			Aftk::C_SceneMeshNode *pMeshNode,
			Aftk::C_Polygon& polygon,
			Ut::UtMap<S32, S32>& newPVIndexMap,
			E_MirrorDirection mirrorDirection = E_MirrorDirection_U,
			S16 texCoordSet = -1);
	};
}


#endif
