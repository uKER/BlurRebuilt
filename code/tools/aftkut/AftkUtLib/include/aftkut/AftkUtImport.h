#ifndef AFTK_UT_IMPORT_H
#define AFTK_UT_IMPORT_H

namespace AftkUt
{
	class C_Import
	{
	public:
		///\brief Imports animation data into this scene from another file.
		///
		///The import process works by matching the scene nodes of this scene with animated scene nodes 
		///in the given file. This matching is done via a case-sensitive string comparison of the node names.
		///For each matching pair, any animation data is first removed from this scene, the destination.
		///Then, the animation curves are copied from the source scene.
		///
		///@param fileName The name of the file to import.
		///@param format The type of file to load. This is optional, by default the AFTK uses the file extension to determine the correct format.
		///@return C_Status class, indicating the result of the operation.
		static Aftk::C_Status ImportAnimation(
			Aftk::C_Scene* pTargetScene,
			const std::string &fileName, 
			Aftk::E_FileType format = Aftk::E_FileType_BestGuess);

		///\brief Imports animation data into this scene from another scene.
		///
		///The import process works by matching the scene nodes of this scene with animated scene nodes 
		///in the given file. This matching is done via a case-sensitive string comparison of the node names.
		///For each matching pair, any animation data is first removed from this scene, the destination.
		///Then, the animation curves are copied from the source scene.
		///
		///@param pScene The scene to import the animation from.
		///@return C_Status class, indicating the result of the operation.
		static Aftk::C_Status ImportAnimation(
			Aftk::C_Scene* pTargetScene,
			const Aftk::C_Scene* pSourceScene);

		///\brief Imports all data from the given scene into the root level of this scene.
		///
		///@param pScene The scene to import from.
		///@param C_Status class, indicating the result of the operation.
		static Aftk::C_Status Import(
			Aftk::C_Scene* pTargetScene,
			const std::string &fileName, 
			Aftk::E_FileType format = Aftk::E_FileType_BestGuess,
			Aftk::I_Progress* pProgress = 0);

		///\brief Imports all data from the given scene into the root level of this scene.
		///
		///@param pScene The scene to import from.
		///@param C_Status class, indicating the result of the operation.
		static Aftk::C_Status Import(
			Aftk::C_Scene* pTargetScene,
			const Aftk::C_Scene* pSourceScene,
			Aftk::I_Progress* pProgress);

	private:
		static bool IsAnimNode(const Aftk::I_Object* pObj);

		static Aftk::C_Status C_Import::ImportPlugConnections(
			const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
			const Aftk::I_Plug& plug);

		static Aftk::C_Status MirrorConnectivity(
			const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
			const Aftk::C_PlugLocationInfo& rhsLocationInfo);

		static Aftk::C_Status RebuildInfluenceCluster(
			const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
			Aftk::C_InfluenceClusterNode* pTargetInfluenceCluster,
			const Aftk::C_InfluenceClusterNode* pSourceInfluenceCluster);

		static Aftk::C_Status C_Import::RebuildNodes(
			const Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable);

		static Aftk::C_SceneNode* Import(
			Aftk::C_Scene* pTargetScene,
			const Aftk::C_SceneNode* pSceneNode, 
			Ut::UtMap<const Aftk::C_Node*, Aftk::C_Node*>& nodeMappingTable,
			const Aftk::MaterialRemappingHashTable& materialResourceMappingTable,
			Aftk::C_Status& status,
			U32& workDone,
			U32& workToDo,
			Aftk::I_Progress* pProgressCallback);
	};
}

#endif
