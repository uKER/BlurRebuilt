#ifndef EDITOR_LIB_I_PLUG_H
#define EDITOR_LIB_I_PLUG_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/Attr/Connection.h>
#include <EditorLib/Dg/Attr/IAttribute.h>

namespace EditorLib
{
	enum E_PlugType
	{
		E_PlugType_Input,
		E_PlugType_Output,
		E_PlugType_InputOutput
	};

	class I_Plug : public I_Attribute
	{
	public:
		virtual ~I_Plug() {}

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		virtual const I_Plug* GetSourceConnection() const = 0;

		///\brief Returns the source connectivity for this plug.
		///
		///@return Connection source.
		virtual I_Plug* GetSourceConnection() = 0;

		///\brief Return the number of children in this plug.
		///
		///@return The number of children.
		virtual U32	GetNumChildren() const = 0;

		virtual I_Plug* GetParentPlug() = 0;

		virtual I_DgNode* GetParentNode() = 0;

		virtual const I_Plug* GetParentPlug() const = 0;

		virtual const I_DgNode* GetParentNode() const = 0;

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual I_Plug* GetChild(U32 idx) = 0;

		///\brief Returns the nth child of this plug
		///
		///This function is only valid if this is a compound plug.
		///
		///@param instance The index of the child to fetch.
		///@return Parental plug pointer.
		virtual const I_Plug* GetChild(U32 idx) const = 0;

		virtual bool AddChild(I_Plug* pChild) = 0;
		
		virtual void SetSourceConnection(I_Plug* pSource, bool triggerTopLevelParentOnChanged = true) = 0;

		virtual void InvokeDependentOnChanged(bool triggerTopLevelParentOnChanged) = 0;

		virtual bool IsNodeDirty() const = 0;

		virtual bool IsTopLevel() const = 0;

		virtual void ResetDirty() = 0;

		virtual C_ConnectedPlugs& GetConnections() = 0;

		virtual const C_ConnectedPlugs& GetConnections() const = 0;

	protected:
	};
}

#endif
