#ifndef EDITOR_LIB_SCENE_XT_ACTOR_NODE_H
#define EDITOR_LIB_SCENE_XT_ACTOR_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "SceneXtNodeBase.h"

namespace EditorLib
{
	class C_SceneXtActorNode : public C_SceneXtNodeBase
	{
	//From I_DgObject
		virtual bool IsA(E_ObjType type) const;


	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;


	//From C_SceneXtActorNode
	public:
		virtual C_Status		XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

		static const char*		GetStaticTypeName();

		virtual const char*		GetTypeName() const;

		static E_ObjType		GetStaticType();
		
		static void*			Create();

		virtual E_ObjType		GetType() const;

	//From C_XtSceneNodeBase
	protected:
		virtual void ImmediatePropagate(const C_ImmediateHintCollection& hc, bool dirtyDependents = true);

	//From C_SceneXtActorNode
	protected:
		C_SceneXtActorNode();
		virtual ~C_SceneXtActorNode();

		void InitialiseExtActorProperties();

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);
	};
}

#endif
