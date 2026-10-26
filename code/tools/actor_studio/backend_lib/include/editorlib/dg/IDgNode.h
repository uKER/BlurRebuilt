#ifndef EDITOR_LIB_IDG_NODE_H
#define EDITOR_LIB_IDG_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include "IDgObject.h"

namespace EditorLib
{
	class I_DgNode : public I_DgObject
	{
	//From I_DgObject
	public:
		virtual bool IsA(E_ObjType type) const
		{
			if (__super::IsA(type))
				return true;
			if (type == E_ObjType_IDgNode)
				return true;

			return false;
		}

	public:
		virtual void SetName(const Btl::String& name) = 0;

		virtual const C_NodeId& GetId() const = 0;

		virtual const Btl::String& GetName() const = 0;

		virtual void GetPlugs(Btl::Vector<I_Plug*>& plugs) = 0;

		virtual void GetPlugs(Btl::Vector<const I_Plug*>& plugs) const = 0;

		virtual bool Update(C_UpdateHintCollection* pHc = 0) = 0;

		virtual bool DirtyDependents(bool setThisDirty = true) = 0;

		virtual bool IsNonXtDirty() const = 0;

		virtual void ResetNonXtDirty() = 0;

		virtual bool IsReadOnly() const = 0;

		virtual bool IsDeletable() const = 0;

		virtual bool IsExternal() const = 0;

		virtual bool IsAlwaysHidden() const = 0;

		virtual bool IsRenameable() const = 0;

		//Cast to C_XtObject (may not have xt object derived implementation however).
		virtual C_XtObject* AsXtObject() = 0;
		virtual const C_XtObject* AsXtObject() const = 0;

		virtual bool IsDirty() const = 0;

		virtual void ResetDirty() = 0;

		virtual void PostInitialiseImmediatePropagate() = 0;

	protected:
		I_DgNode() : I_DgObject() {}
		virtual ~I_DgNode() {}

		virtual void SetPostCreateSignalled() = 0;

		FRIEND_CLASS C_DgManager;
	};
}

#endif
