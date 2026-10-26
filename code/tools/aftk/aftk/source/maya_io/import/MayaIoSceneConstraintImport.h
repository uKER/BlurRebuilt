#ifndef MAYA_IO_SCENE_CONSTRAINT_IMPORT_H
#define MAYA_IO_SCENE_CONSTRAINT_IMPORT_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_Importer;

	class C_SceneConstraintImport : public C_SceneNodeImport
	{
	public:
		static Aftk::C_Status Import(
			C_Importer* pImport,
			const MDagPath &transformDagPath,
			const Aftk::Matrix44* pExclusiveMatrix,
			Aftk::C_Scene* pAftkScene,
			Aftk::C_SceneNode* pAftkSceneNode,
			const C_ImportOptions* pOptions);

		static Aftk::I_Plug* FindPlug(
			Aftk::C_SceneNode* pAftkNode,
			const char* pPlugName);

		static Aftk::C_Status ResolveConnectionsSceneWide(	
			C_Importer* pImporter,
			Aftk::C_Scene* pAftkScene,
			const C_ImportOptions* pOptions);

	private:
	};
}

#endif
