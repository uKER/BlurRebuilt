#ifndef AFTK_RENDER_MATERIAL_BUILDER_H
#define AFTK_RENDER_MATERIAL_BUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "core/containers/BitVector.h"

namespace Aftk
{
	class C_MaterialNode;
}

namespace AftkRender
{
	class MaterialBuilder
	{
	public:
		static bool Build(
			const Aftk::C_Scene *pScene,
			Ut::UtVector< Ut::DynamicVector<C_Material> >&	materials,
			Ut::UtVector< Ut::DynamicVector<S32> >&	materialMap);

		static bool Build(
			const Aftk::C_SceneNode *pSceneNode,
			Ut::UtVector< Ut::DynamicVector<C_Material> >&	materials,
			Ut::UtVector< Ut::DynamicVector<S32> >&	materialMap);

	private:
		
		static void BuildMaterialUsageArray(
			const Aftk::C_SceneNode* pNode,
			Ut::BitVector& indexedMaterialInUse);


		static bool Build(C_Material& material, const Aftk::C_MaterialNode* pAftkMaterial);
	};


}

#endif