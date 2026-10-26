#ifndef EDITOR_LIB_LINK_PROP_H
#define EDITOR_LIB_LINK_PROP_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/Attr/IPlug.h>

namespace EditorLib
{
	class C_DgLinkPropNode : public C_DgXtPropNodeBase
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;


	//From C_DgLinkPropNode
	public:
		virtual const char*		GetTypeName() const;

		static const char*		GetStaticTypeName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		///\brief Get this objects's type enum.
		///
		///Node types are all identified by the enumerators of E_ObjType. This function returns the
		///enum indicating the type of this object.
		///
		///@return object's type enum.
		virtual E_ObjType GetType() const;

		///\brief Returns true if this object implements the given type.
		///
		///@param The object type to test against.
		///@return true if we implement the given object type.
		virtual bool IsA(E_ObjType type) const;

		virtual bool IsVisible() const;

		virtual bool IsDirty() const;

		virtual void ResetDirty();

		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

		const I_DgNode* GetLinkedNode() const;
		
		C_StaticPlug<MathsStorage::Matrix44>& GetTargetWorldMatrixPlug();

		const C_StaticPlug<MathsStorage::Matrix44>& GetTargetWorldMatrixPlug() const;

	protected:
		union LinkNodeDirtyFlags
		{
			struct
			{
				bool targetChanged : 1;
			};
			U32 data;
		};

		C_DgLinkPropNode();
		virtual ~C_DgLinkPropNode();

		C_DgLinkPropNode(const C_DgLinkPropNode& rhs);
		const C_DgLinkPropNode& operator=(const C_DgLinkPropNode& rhs);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

		static void OnTargetPlugChanged(I_Plug* pPlug);

		LinkNodeDirtyFlags	m_linkNodeDirtyFlags;
		I_Plug*				m_pTargetWorldMatrixPlug;
	};
}

#endif
