#ifndef EDITOR_LIB_SCENE_XT_GROUP_NODE_H
#define EDITOR_LIB_SCENE_XT_GROUP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "SceneXtNodeBase.h"

namespace EditorLib
{
	class C_SceneXtNode : public C_SceneXtNodeBase
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);


	//From C_SceneXtNode
	public:
		static const char*		GetStaticTypeName();

		virtual const char*		GetTypeName() const;

		static E_ObjType		GetStaticType();
		
		static void*			Create();

		virtual E_ObjType		GetType() const;

		virtual bool			IsA(E_ObjType type) const;

		virtual C_Status		XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	protected:
		C_SceneXtNode();
		virtual ~C_SceneXtNode();

		virtual bool VetoParentChange(const C_SceneNode* pCandidateParent) const;
	};
}

#endif
