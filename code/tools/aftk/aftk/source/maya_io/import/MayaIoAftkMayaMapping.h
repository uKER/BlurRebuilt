#ifndef MAYA_IO_AFTK_MAYA_MAPPING_H
#define MAYA_IO_AFTK_MAYA_MAPPING_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace MayaIo
{
	class C_AftkMayaMapping
	{
	public:
		C_AftkMayaMapping();

		Aftk::C_Status Create();

		void Destroy();

		void Clear();

		bool GetMayaSceneNodeFromNameString(
			const std::string& name,
			MDagPath& dagPath);

		bool GetMayaSceneNodeFromDagPathString(
			const std::string& dagPathString,
			MDagPath& dagPath);

		bool GetAftkSceneNodeFromDagPathString(
			const std::string& dagPathString,
			Aftk::C_SceneNode** ppAftkSceneNode);

		bool GetAftkSceneNodeFromMayaDagPath(
			const MDagPath& mayaDagPath,
			Aftk::C_SceneNode** ppAftkSceneNode);

		bool GetMayaDagPathFromAftkSceneNode(
			Aftk::C_SceneNode* pSceneNode,
			MDagPath* pMayaDagPath);

		bool GetMayaDagPathFromAftkSceneNode(
			const Aftk::C_SceneNode* pSceneNode,
			MDagPath* pMayaDagPath);

		void SetMayaAftkSceneNodeMapping(
			const MDagPath& mayaDagPath,
			Aftk::C_SceneNode* pAftkSceneNode);

		void SetMayaAftkSceneNodeMapping(
			const MDagPath& mayaDagPath,
			const Aftk::C_SceneNode* pAftkSceneNode);

		void SetAftkMayaSceneNodeMapping(
			Aftk::C_SceneNode* pAftkSceneNode,
			const MDagPath& mayaDagPath);

		void SetAftkMayaSceneNodeMapping(
			const Aftk::C_SceneNode* pAftkSceneNode,
			const MDagPath& mayaDagPath);

		void SetNameStringMayaSceneNodeMapping(
			const std::string& nodeName,
			const MDagPath& mayaDagPath);

		void SetDagPathStringMayaSceneNodeMapping(
			const std::string& dagPathString,
			const MDagPath& mayaDagPath);

		void DeleteAftkMayaSceneNodeMapping(
			Aftk::C_SceneNode* pAftkSceneNode);

	//private:
		bool m_created;

		//Storage for mapping between the Aftk and Maya
		Ut::UtMap<std::string, MDagPath>*				m_pNameStringMayaSceneNodeMapping;
		Ut::UtMap<std::string, MDagPath>*				m_pDagPathStringMayaSceneNodeMapping;
		Ut::UtMap<std::string, Aftk::C_SceneNode*>*		m_pMayaAftkSceneNodeMapping;
		Ut::UtMap<Aftk::C_SceneNode*, MDagPath>*		m_pAftkMayaSceneNodeMapping;
	};
}

#endif
