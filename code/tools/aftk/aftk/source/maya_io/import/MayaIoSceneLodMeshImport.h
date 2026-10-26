#ifndef MAYA_IO_SCENE_LOD_MESH_IMPORT_H
#define MAYA_IO_SCENE_LOD_MESH_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_SceneLodMeshImport
	{
	public:
		static Aftk::C_Status MergeLodMeshesSceneWide(
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions);

	private:
		static Aftk::C_Status CopyLodGroupTransformProporties(
			Aftk::C_SceneNode* pSource,
			Aftk::C_SceneNode* pDestination);

		static Aftk::C_Status ImportLodGroup(
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			const MDagPath& lodGroupPath,
			Ut::UtSet< Aftk::C_SceneNode*>& lodLevelsToDelete);

		struct LodGroupMeshes
		{
			bool operator == (const LodGroupMeshes& rhs) const
			{
				if (meshes.Size() != rhs.meshes.Size())
				{
					return false;
				}

				for (U32 i = 0; i < (U32)meshes.Size(); i++)
				{
					if (meshes[i]->GetUniqueId() != rhs.meshes[i]->GetUniqueId())
					{
						return false;
					}
				}

				return true;
			}

			bool operator < (const LodGroupMeshes& rhs) const
			{
				if (meshes.Size() != rhs.meshes.Size())
				{
					if (meshes.Size() < rhs.meshes.Size())
					{
						return true;
					}
					else
					{
						return false;
					}
				}

				for (U32 i = 0; i < (U32)meshes.Size(); i++)
				{
					if (meshes[i]->GetUniqueId() < rhs.meshes[i]->GetUniqueId())
					{
						return true;
					}
				}

				return false;
			}

			bool operator()(const LodGroupMeshes& a, const LodGroupMeshes& b) const
			{
				return (a < b);
			}

			Ut::UtDynamicVector<Aftk::C_SceneNode*> meshes;
		};
	};
}

#endif
