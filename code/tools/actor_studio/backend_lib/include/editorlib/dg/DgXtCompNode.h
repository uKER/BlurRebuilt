#ifndef EDITOR_LIB_DG_XT_COMP_NODE_H
#define EDITOR_LIB_DG_XT_COMP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLIb/Xt/XtObject.h>
#include "DgXtPropNodeBase.h"

namespace EditorLib
{
	typedef Btl::Vector<Sys::WeakPtr<C_DgXtPropNodeBase> >			DgPropArray;
	typedef Btl::Vector<const Sys::WeakPtr<C_DgXtPropNodeBase> >	ConstDgPropArray;

	class C_DgXtCompNode : public C_DgNodeBase, public C_XtObject
	{
	//From C_XtObject
	public:
		virtual void XtBind() const;

		virtual void XtUnbind() const;

	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;

		virtual void ResetNonXtDirty();

		virtual C_XtObject* AsXtObject();

		virtual const C_XtObject* AsXtObject() const;


	//From C_DgXtCompNode
	public:
		static const char*		GetStaticTypeName();

		virtual const char*		GetTypeName() const;

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual E_ObjType GetType() const;

		virtual bool IsA(E_ObjType type) const;

		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType,
			Xt::DomElement* pData);

		const DgPropArray& GetProperties();

		const ConstDgPropArray& GetProperties() const;

		I_DgNode* GetParent();

		const I_DgNode* GetParent() const;

		///Only for use during construction.
		void SetParent(I_DgNode* pParent, U32 childId);

	//From C_XtObject
	protected:
		virtual void			OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	protected:
		union CompNodeDirtyFlags
		{
			struct
			{
				mutable bool hierarchyDirty : 1;
			};
			U32 data;
		};
		union CompNodeFlags
		{
			struct
			{
				mutable bool initialUpdatePending : 1;
			};
			U32 data;
		};

		C_DgXtCompNode();
		virtual ~C_DgXtCompNode();

		C_DgXtPropNodeBase* CreateProperty(
			S32 index, 
			LarvaType* pType,
			Xt::DomElement* pElement,
			const char* name,
			C_Status& status);

		void DestroyProperty(C_DgXtPropNodeBase* pProperty);

		C_Status RefreshPropertiesFromSchema();

		I_DgNode*			m_pParent;
		Btl::Vector<Sys::WeakPtr<C_DgXtPropNodeBase> >
							m_properties;

		CompNodeDirtyFlags	m_compNodeDirtyFlags;
		CompNodeFlags		m_compNodeFlags;
	};
}

#endif

