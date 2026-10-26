#ifndef EDITOR_LIB_AFTK_REF_H
#define EDITOR_LIB_AFTK_REF_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <btl/utility/string.h>
#include <EditorLib/Misc/Status.h>

namespace Aftk
{
	class C_SceneNode;
	class C_GenericAttribute;
}

namespace EditorLib
{
	class C_SceneNode;

	class C_AftkRef
	{
	public:
		//We pass in the root scene node to which the content for this reference is parented.
		C_AftkRef(const Btl::String& filepath, bool isVisualisation);
		~C_AftkRef();

		///Could be for example, a maya or XSI file.
		const Btl::String& GetOriginalFilePath() const;

		///Converted A3D filepath.
		const Btl::String& GetCacheFilePath() const;

		C_SceneNode* GetSceneRootNode();

		const C_SceneNode* GetSceneRootNode() const;

		bool IsCacheOutOfDate() const;

		bool IsLoaded() const;
		
		bool IsVisualisation() const;

		C_Status Load() const;

		C_Status Unload() const;

	private:
		C_AftkRef(const C_AftkRef& rhs);
		const C_AftkRef& operator=(const C_AftkRef& rhs);

		C_SceneAftkNode* MirrorAftkScene(
			AftkDgMap& aftkDgMap, 
			Aftk::C_SceneNode* pAftkSceneNode) const;

		bool CreateLayers(const AftkDgMap& aftkDgMap, const Aftk::C_Scene& scene) const;

		bool						m_isVisualisation;
		Btl::String					m_originalFilePath;
		mutable Btl::String			m_cacheFilePath;
		mutable C_SceneNode*		m_pSceneNode;
	};
}

#endif
