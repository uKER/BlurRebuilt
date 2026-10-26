#ifndef EDITOR_LIB_SCENE_XT_NODE_BASE_H
#define EDITOR_LIB_SCENE_XT_NODE_BASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/SceneNode.h>

namespace EditorLib
{
	class C_SceneXtNodeBase : public C_SceneNode, public C_XtObject
	{
	//From I_DgObject
	public:
		virtual bool IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual void SetName(const Btl::String& name);

		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;

		virtual void ResetNonXtDirty();

		virtual C_XtObject*			AsXtObject();
		virtual const C_XtObject*	AsXtObject() const;

	//From C_SceneNode
	protected:
		virtual bool VetoParentChange(const C_SceneNode* pCandidateParent) const;

		virtual void OnStoreTransform();

		virtual void OnStoreParent();

		virtual void ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents = true);

	//From C_XtObject
	public:

		virtual void			XtBind() const;

		virtual void			XtUnbind() const;

		virtual C_Status		XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	//From C_SceneXtNodeBase
	public:
		const DgActorComponentArray& GetComponents();

		const ConstDgActorComponentArray& GetComponents() const;

	//From C_XtObject
	protected:
		virtual void			OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	//From C_SceneXtNodeBase
	protected:
		union SceneXtNodeBaseNodeDirtyFlags
		{
			struct
			{
			};
			U32 data;
		};
		union SceneXtNodeBaseNodeFlags
		{
			struct
			{
				bool hasDefaultComponent : 1;
			};
			U32 data;
		};

		C_SceneXtNodeBase();
		virtual ~C_SceneXtNodeBase();

		C_DgXtCompNode* CreateComponent(
			LarvaType* pType, Xt::DomElement* pElement, const char* name, C_Status& status);

		void DestroyComponent(C_DgXtCompNode* pComponent);

		C_Status RefreshComponentsFromSchema();

		DgActorComponentArray			m_components;

		SceneXtNodeBaseNodeDirtyFlags	m_sceneXtNodeDirtyFlags;
		SceneXtNodeBaseNodeFlags		m_sceneXtNodeFlags;
	};
}

#endif
