#ifndef MAYA_IO_MATERIAL_IMPORT_H
#define MAYA_IO_MATERIAL_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_Importer;

	class C_MaterialImport
	{
	public:
		static const Aftk::C_SharedResource<Aftk::C_MaterialNode>* Import(
			C_Importer* pImporter,
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode,
			Aftk::C_Status& status);

		static bool DoWeImportThisPlug(
			C_Importer* pImporter,
			const MPlug& mayaPlug);

	private:
		static bool CanImportNode(const MFnDependencyNode& shaderDependencyNode);

		struct ConnectionMapping
		{
			//SPARSE ARRAY
			Ut::UtVector< Ut::DynamicVector<S32> >			m_mayaToAftkIndexMapping;
			//Indexes into the above mapping array, so that we only process the nodes that we have
			//decided to import
			Ut::UtVector< Ut::DynamicVector<U32> >			m_connectedMappingIndices;
		};

		static Aftk::C_Status ImportMeshMaterialFull(
			C_Importer* pImporter,
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_MaterialNode& aftkMaterialNode,
			Aftk::C_Scene* pAftkScene);

		static Aftk::C_Status ImportMeshMaterialSimple(
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_MaterialNode& aftkMaterialNode,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode);

		static bool DoWeImportThisPlugAsDynamic(
			const MPlug& mayaPlug);

		static Aftk::C_Status ImportPlugValue(
			const MPlug& mayaPlug,
			Aftk::C_GenericAttribute& aftkGenericAttribute);

		static Aftk::I_Plug* GetPlugPtrRecursive(
			Aftk::C_Node& node,
			const std::string& name,
			bool& found,
			Aftk::C_Status& status);

		static Aftk::C_Status ResolvePlugConnections(
			C_Importer* pImporter,
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_MaterialNode& aftkMaterialNode,
			Aftk::C_Scene* pAftkScene,
			Ut::UtMap<std::string, Aftk::C_Node*>& shaderMapping);

		static Aftk::C_Status CreateShaderTreeNodes(
			C_Importer* pImporter,
			Ut::UtVector< Ut::DynamicVector<ConnectionMapping> >&	connectionMappings,
			Ut::UtMap<std::string, Aftk::C_Node*>&					shaderMapping,
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_MaterialNode& aftkMaterialNode,
			Aftk::C_Scene* pAftkScene);

		static Aftk::C_Status CreateShaderTreeNodesSimple(
			const MFnDependencyNode& shaderDependencyNode,
			Aftk::C_MaterialNode& aftkMaterialNode,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneMeshNode* pAftkSceneMeshNode);

		static Aftk::C_Status MayaPlugToAftkPlug(
			C_Importer* pImporter,
			const MPlug& mayaPlug,
			Aftk::C_DynamicPlug& aftkPlug);
	};
}

#endif
