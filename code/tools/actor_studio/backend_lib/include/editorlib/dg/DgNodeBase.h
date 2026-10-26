#ifndef EDITOR_LIB_DG_NODE_BASE_H
#define EDITOR_LIB_DG_NODE_BASE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Misc/Definitions.h>

namespace EditorLib
{
	class C_DgNodeBase : public I_DgNode
	{
	//From I_DgObject
	public:
		virtual bool IsA(E_ObjType type) const;

	//From I_DgNode
	public:
		virtual void SetName(const Btl::String& name);

		virtual void GetPlugs(Btl::Vector<I_Plug*>& plugs);

		virtual void GetPlugs(Btl::Vector<const I_Plug*>& plugs) const;

		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;

		virtual void ResetNonXtDirty();

		virtual bool IsReadOnly() const;

		virtual bool IsDeletable() const;

		virtual bool IsExternal() const;

		virtual bool IsAlwaysHidden() const;

		virtual bool IsRenameable() const;

		virtual C_XtObject* AsXtObject();
		virtual const C_XtObject* AsXtObject() const;

		virtual bool IsDirty() const;

		virtual void ResetDirty();

		virtual void PostInitialiseImmediatePropagate() {}

	//From C_DgBaseNode
	public:
		struct Flags
		{
			union
			{
				struct
				{
					bool readOnly : 1;
					bool canRename : 1;
					bool userDeletable : 1;
					bool baseInstance : 1;
					bool hidePermanently : 1;
					bool external : 1;
					bool postCreateSignalled : 1;
				};
				U32 data;
			};
		};

		struct NodeBaseDirtyFlags
		{
			union
			{
				struct
				{
					bool nameDirty : 1;
				};
				U32 data;
			};
		};

		const C_NodeId& GetId() const;

		Flags& GetNodeFlags();

		const Flags& GetNodeFlags() const;

		const Btl::String& GetName() const;

	protected:
		C_DgNodeBase();
		C_DgNodeBase(const C_DgNodeBase& rhs);
		~C_DgNodeBase();
		C_DgNodeBase& operator=(const C_DgNodeBase& rhs);

		virtual void SetPostCreateSignalled();

		C_NodeId					m_id;
		C_StaticPlug<Btl::String>	m_name;
		Flags						m_nodeFlags;
		NodeBaseDirtyFlags			m_nodeDirtyFlags;

	private:
		static void OnNodeBasePlugChanged(I_Plug* pPlug);
	};
}

#endif
