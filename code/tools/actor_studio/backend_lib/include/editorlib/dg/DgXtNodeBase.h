#ifndef EDITOR_LIB_DG_XT_NODE_BASE_H
#define EDITOR_LIB_DG_XT_NODE_BASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Dg/DgNodeBase.h>

namespace EditorLib
{
	class C_DgXtNodeBase : public C_DgNodeBase, public C_XtObject
	{
	//From I_DgObject
	public:
		virtual bool IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual void SetName(const Btl::String& name);

		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool IsNonXtDirty() const;

		virtual void ResetNonXtDirty();

		virtual C_XtObject*			AsXtObject();
		virtual const C_XtObject*	AsXtObject() const;

	//From C_XtObject
	public:
		virtual C_Status		XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	//From C_XtObject
	protected:
		virtual void			OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	//From C_DgXtNodeBase
	protected:
		union DgXtNodeBaseNodeDirtyFlags
		{
			struct
			{
			};
			U32 data;
		};

		C_DgXtNodeBase();
		virtual ~C_DgXtNodeBase();

		DgXtNodeBaseNodeDirtyFlags	m_dgXtNodeBaseDirtyFlags;
	};
}

#endif
