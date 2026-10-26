#ifndef EDITOR_LIB_AFTK_MANAGER_H
#define EDITOR_LIB_AFTK_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_AftkRefManager
	{
	public:
		C_AftkRefManager();
		~C_AftkRefManager();

		C_SceneNode* GetRefSceneNode();

		void GetRefs(ConstAftkRefList& refList) const;

		C_AftkRef* GetRef(const Btl::String& filepath);

		const C_AftkRef* GetRef(const Btl::String& filepath) const;

		//Should only be called by C_XtLevel class.
		C_AftkRef* CreateRef(const Btl::String& filepath, bool isVisualisation);

		//Should only be called by C_XtLevel class.
		C_Status DestroyRef(const Btl::String& filepath);

		void SetCacheDirectory(const Btl::String& dir);

		const Btl::String& GetCacheDirectory() const;

		//Set path and params for nexus.  params can utilise %scene% and %cache% macros
		void SetConversionTool(const Btl::String& path, const Btl::String& params, const Btl::String& metaQueryParams);

		void GetConverstionTool(Btl::String& path, Btl::String& params, Btl::String& metaQueryParams) const;

	private:
		//We need to track reference counts as multiple levels may use the same reference.
		struct C_AftkRefRecord
		{
			C_AftkRef*	pRef;
			U32			refCount;
		};
		typedef Ut::UtList<C_AftkRefRecord>	AftkRefRecordList;
		AftkRefRecordList	m_references;

		Btl::String		m_CacheDirectory;
		Btl::String		m_ConversionToolPath;
		Btl::String		m_ConversionToolParams;
		Btl::String		m_ConversionToolMetaQueryParams;

		C_SceneNode*	m_ReferenceSceneNode;
	};
}

#endif
