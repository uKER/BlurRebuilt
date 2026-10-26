#ifndef AFTK_UT_EXPAND_UNIQUE_REMAPPED_INSTANCES_H
#define AFTK_UT_EXPAND_UNIQUE_REMAPPED_INSTANCES_H

namespace AftkUt
{
	class C_ExpandUniqueRemappedInstances
	{
	public:
		///\brief Expands unique combinations of remapped instances in to meshes and non-remapped instances.
		///
		///@return Status class, indicating the result of the operation.
		static Aftk::C_Status Expand(
			Aftk::C_Scene* pAftkScene,
			void (*pMessageCallbackFunc)(const std::string& text) = 0,
			void (*pWarningCallback)(const std::string& text) = 0,
			void (*pErrorCallback)(const std::string& text) = 0,
			Aftk::I_Progress* pProgress = 0);

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
