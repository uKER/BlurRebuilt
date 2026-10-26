#ifndef NEXUS_RENDER_EXPAND_UNIQUE_REMAPPED_INSTANCES_H
#define NEXUS_RENDER_EXPAND_UNIQUE_REMAPPED_INSTANCES_H

namespace NexusRender
{
	class ExpandUniqueRemappedInstances
	{
	public:
		///\brief Expands unique combinations of remapped instances into meshes and non-remapped instances.
		///
		///@return Status class, indicating the result of the operation.
		static Tu::Status Expand(
			SceneRenderer* pSceneRenderer,
			Ut::UtVector<Ut::DynamicVector<RenderNode*> >& renderNodes,
			Nexus::Progress* pProgress);

	private:
		static void GetUniqueKey(
			Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
			const Aftk::C_SceneMeshRemappableNode* pRemappableInstance);

		static void GetUniqueKey(
			Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
			const Aftk::C_SceneMeshRemappedInstanceNode* pRemappedInstance);

		static void GetUniqueKey(
			Ut::UtVector<Ut::DynamicVector<U32> >& uniqueMappingKey,
			const Aftk::C_SceneMeshNode* pRemappedInstance);

		static bool KeysEqual(
			const Ut::UtVector<Ut::DynamicVector<U32> >& key1,
			const Ut::UtVector<Ut::DynamicVector<U32> >& key2);
	};
}

#endif