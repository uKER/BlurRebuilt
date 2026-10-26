//Static class for manipulating duplicated nodes in the AFTK
#pragma once

typedef Aftk::C_Status (*ProcessDuplicateTaskCallback)(
	std::set<Aftk::C_Node*>& equalNodes,
	Aftk::C_Scene* pScene,
	U32 flags,
	void (*pMessageCallbackFunc)(const std::string& text),
	void (*pWarningCallback)(const std::string& text),
	void (*pErrorCallback)(const std::string& text));


namespace AftkUt
{
	class C_ProcessDuplicateNodes
	{
	public:

		//behaviour flags

		enum AftkUtEqualityFlags
		{
			AFTKUT_EQUALITY_TEST_LOCAL_CLASS_ONLY = 1,
			AFTKUT_EQUALITY_MESHES_ONLY = 2,
			AFTKUT_EQUALITY_SCENENODES = 4,
			AFTKUT_EQUALITY_MATERIALNODES = 8,
			AFTKUT_EQUALITY_BLINDDATANODES = 16,
		};

		///Deletes duplicate meshes which are exactly the same mesh in the same position in world space
		///making sure any instances of the deleted meshes are made instances of the one kept. 
		static Aftk::C_Status RemoveDuplicateNodes(	
			Aftk::C_Scene* pScene,
			U32 flags = AFTKUT_EQUALITY_MESHES_ONLY,
			void (*pMessageCallbackFunc)(const std::string& text) = 0,
			void (*pWarningCallback)(const std::string& text) = 0,
			void (*pErrorCallback)(const std::string& text) = 0,
			Aftk::I_Progress* pProgress = 0);

		///Identifies meshes which are exactly the same (although maybe in different world space positions) 
		///and turns them into instances. Any instances of the changed meshes are resourced to the new 
		///source mesh. 
		static Aftk::C_Status CreateInstances(
			Aftk::C_Scene* pScene, 
			void (*pMessageCallbackFunc)(const std::string& text) = 0,
			void (*pWarningCallback)(const std::string& text) = 0,
			void (*pErrorCallback)(const std::string& text) = 0,
			Aftk::I_Progress* pProgress = 0);

	private:
		static Aftk::C_Status Process(
			Aftk::C_Scene* pScene,
			U32 flags,
			Aftk::E_ObjType classToAbortTest,
			ProcessDuplicateTaskCallback taskCallback,
			void (*pMessageCallbackFunc)(const std::string& text) = 0,
			void (*pWarningCallback)(const std::string& text) = 0,
			void (*pErrorCallback)(const std::string& text) = 0,
			Aftk::I_Progress* pProgress = 0
			);

		static Aftk::C_Status FindDuplicateNodes(
			Ut::UtVector<Ut::DynamicVector< std::set<Aftk::C_Node*>* > >& equalNodes,
			Aftk::C_Scene* pScene,
			U32 flags,
			Aftk::E_ObjType classToAbortTest,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallback)(const std::string& text),
			void (*pErrorCallback)(const std::string& text),
			Aftk::I_Progress* pProgress);

		///Actually perform the duplicate removal processing.
		static Aftk::C_Status RemoveDuplicateNodesTask(
			std::set<Aftk::C_Node*>& equalNodes,
			Aftk::C_Scene* pScene,
			U32 flags,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallback)(const std::string& text),
			void (*pErrorCallback)(const std::string& text));

		///Actually perform the instance processing.
		static Aftk::C_Status ConvertDuplicateNodesToInstancesTask(
			std::set<Aftk::C_Node*>& equalNodes,
			Aftk::C_Scene* pScene,
			U32 flags,
			void (*pMessageCallbackFunc)(const std::string& text),
			void (*pWarningCallback)(const std::string& text),
			void (*pErrorCallback)(const std::string& text));

		static Aftk::C_Status SortNodesIntoBuckets(Aftk::C_Scene* pScene, 
				Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >** ppBuckets,
				U32& nBuckets, U32 flags);

		static Aftk::C_Status GetEqualNodeSetsFromBuckets(Aftk::C_Scene* pScene, 
				const Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets,
				const U32 nBuckets,
				Ut::UtVector<Ut::DynamicVector< std::set<Aftk::C_Node*>* > >& pEqualNodes, 
				U32 flags,
				Aftk::E_ObjType classToAbortTest);

		static Aftk::C_Status AddSceneNodesToBuckets(Aftk::C_Scene* pScene,
			Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, U32 nBuckets, U32 flags);

		static Aftk::C_Status AddMaterialNodesToBuckets(Aftk::C_Scene* pScene,
			Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, U32 nBuckets, U32 flags);

		static Aftk::C_Status AddBlindDataNodesToBuckets(Aftk::C_Scene* pScene,
			Ut::UtVector<Ut::DynamicVector<Aftk::C_Node*> >* pBuckets, U32 nBuckets, U32 flags);

	};

}
