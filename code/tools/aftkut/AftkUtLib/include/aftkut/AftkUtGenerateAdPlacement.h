#ifndef AFTK_GENERATE_AD_PLACEMENT_H
#define AFTK_GENERATE_AD_PLACEMENT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace AftkUt
{
	class C_GenerateAdPlacement
	{
	public:
		struct Material
		{
			const Aftk::C_MaterialResource* pMaterial;
			U32 placementIndex;
		};
		typedef Ut::UtVector<Ut::DynamicVector<Material> > MaterialArray;

		struct Quad
		{
			Aftk::Position3 v[4];

			U32 placementIndex;
		};
		typedef Ut::UtVector<Ut::DynamicVector<Quad> > QuadArray;

		typedef bool(*ProcessThisNodeFunc)(const Aftk::C_SceneNode& sceneNode);

		static Aftk::C_Status Generate(
			QuadArray& quadArray,
			const Aftk::C_Scene* pScene,
			const Aftk::C_LayerNode* pLayer,
			ProcessThisNodeFunc pProcessThisNodeFunc);

		static Aftk::C_Status GenerateAftkMeshFromQuadArray(
			Aftk::C_Scene* pScene,
			const QuadArray& quadArray);

		static bool HasAdvertising(
			const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode);

		static bool IsAdvertisingMaterial(
			const Aftk::C_MaterialNode& material);

		static bool IsAdvertisingMaterial(
			const Util::C_String& textureFilePath);

		static S32 GetPlacementIndex(const Aftk::C_MaterialNode& material);

		static S32 GetPlacementIndex(const Util::C_String& fileName);

	private:
		static Aftk::C_Status BuildAdvertisingMaterialList(
			MaterialArray& materialList,
			const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode);

		static Aftk::C_Status GenerateIsland(
			Ut::UtVector<Ut::DynamicVector<U32> >& island,
			const MaterialArray& materialList,
			const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode,
			U32 firstPolyIndex);

		static bool HasAdvertising(
			const MaterialArray& materialList,
			const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode);

		static S32 HasAdvertising(
			const MaterialArray& materialList,
			const Aftk::C_SceneMeshRemappableNode& materialSource,
			const Aftk::C_Polygon& aftkPolygon);

		static Aftk::C_Status GenerateFromStaticMesh(
			QuadArray& quadArray,
			const MaterialArray& materialList,
			const Aftk::Matrix44& worldMatrix,
			const Aftk::C_SceneMeshRemappableNode& aftkSceneMeshNode);

		static Aftk::C_Status GenerateFromDeformedMesh(
			QuadArray& quadArray,
			const MaterialArray& materialList,
			const Aftk::C_SceneMeshBonedInstanceNode& aftkSceneMeshBonedInstance);

		static Aftk::C_Status GenerateAftkMeshFromQuad(
			Aftk::C_SceneMeshNode& aftkSceneMeshNode,
			const Quad& quad);

		static const char* m_namingConvention;
	};
}

#endif
