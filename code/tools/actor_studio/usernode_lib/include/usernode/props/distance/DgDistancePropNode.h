#ifndef EDITOR_LIB_DISTANCE_PROP_H
#define EDITOR_LIB_DISTANCE_PROP_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/Attr/IPlug.h>

namespace EditorLib
{
	class C_DgDistancePropNode : public C_DgXtPropNodeBase
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;


	//From C_DgDistancePropNode
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

		const I_DgNode* GetLinkedNodeA() const;

		const I_DgNode* GetLinkedNodeB() const;
		
		C_StaticPlug<MathsStorage::Matrix44>& GetTargetAWorldMatrixPlug();

		const C_StaticPlug<MathsStorage::Matrix44>& GetTargetAWorldMatrixPlug() const;

		C_StaticPlug<MathsStorage::Matrix44>& GetTargetBWorldMatrixPlug();

		const C_StaticPlug<MathsStorage::Matrix44>& GetTargetBWorldMatrixPlug() const;

	protected:
		enum E_StaticPlug
		{
			E_StaticPlug_WorldMatrixA,
			E_StaticPlug_WorldMatrixB,

			E_StaticPlug_Count
		};

		union DistanceNodeDirtyFlags
		{
			struct
			{
				bool targetChanged : 1;
			};
			U32 data;
		};

		C_DgDistancePropNode();
		virtual ~C_DgDistancePropNode();

		C_DgDistancePropNode(const C_DgDistancePropNode& rhs);
		const C_DgDistancePropNode& operator=(const C_DgDistancePropNode& rhs);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

		static void OnTargetPlugChanged(I_Plug* pPlug);

		DistanceNodeDirtyFlags	m_distanceNodeDirtyFlags;
		I_Plug*					m_plugs[E_StaticPlug_Count];
	};
}

#endif
