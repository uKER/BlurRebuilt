#ifndef MAYA_IO_ANIMATION_IMPORT_H
#define MAYA_IO_ANIMATION_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif


namespace MayaIo
{
	class C_Importer;

	class C_ConnectedDgImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImporter,
			MDagPath& transformDagPath,
			MDagPath* pShapeDagPath,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode);

		static Aftk::I_Object* ImportDirectlyConnectedPlug(
			C_Importer* pImporter,
			const MPlug& animatedPlug,
			Aftk::C_Status& status);

		static Aftk::I_Plug* FindPlug(
			Aftk::C_SceneNode* pAftkSceneNode,
			const std::string& plugName);

		static Aftk::C_Node* CreateNode(
			C_Importer* pImporter,
			MFnDependencyNode& fnSourceNode);
	};
}

#endif
