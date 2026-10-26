#ifndef EDITOR_LIB_I_DG_PROP_NODE_BASE_H
#define EDITOR_LIB_I_DG_PROP_NODE_BASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLib/Xt/XtObject.h>

namespace EditorLib
{
	class C_DgXtPropNodeBase : public C_DgNodeBase, public C_XtObject
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;

		virtual void ResetNonXtDirty();

		virtual C_XtObject* AsXtObject();

		virtual const C_XtObject* AsXtObject() const;


	//From C_DgXtPropNodeBase
	public:
		virtual bool IsA(E_ObjType type) const;

		virtual C_DgXtCompNode* GetParent();

		virtual const C_DgXtCompNode* GetParent() const;

		virtual void SetParent(C_DgXtCompNode* pParent, U32 childId);

		virtual bool IsVisible() const = 0;

		virtual bool IsDirty() const;

		virtual void ResetDirty();

	//From C_XtObject
	public:
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	// From C_XtObject
	protected:
		void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	//From C_DgXtPropNodeBase
	protected:
		union PropNodeDirtyFlags
		{
			struct
			{
				bool hierarchyDirty : 1;
			};
			U32 data;
		};

		C_DgXtPropNodeBase();
		virtual ~C_DgXtPropNodeBase();

		C_DgXtPropNodeBase(const C_DgXtPropNodeBase& rhs);
		const C_DgXtPropNodeBase& operator=(const C_DgXtPropNodeBase& rhs);

		C_DgXtCompNode* CreateComponent(
			LarvaType* pType, Xt::DomElement* pElement, const char* name, C_Status& status);

		void DestroyComponent(C_DgXtCompNode* pComponent);

		C_Status RefreshComponentsFromSchema();

		C_DgXtCompNode*							m_pParent;
		PropNodeDirtyFlags						m_propNodeDirtyFlags;
	};
}

#endif
