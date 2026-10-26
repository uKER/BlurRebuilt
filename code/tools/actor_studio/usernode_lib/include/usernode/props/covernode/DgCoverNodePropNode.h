#ifndef EDITOR_PROPS_DG_CoverNode_PROP_NODE_H
#define EDITOR_PROPS_DG_CoverNode_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	class C_XtDirty;

	class C_DgCoverNodePropNode : public C_DgXtPropNodeBase
	{
	//From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

		virtual bool DirtyDependents(bool setThisDirty = true);

		virtual bool IsNonXtDirty() const;


	//From C_DgCoverNodePropNode
	public:
		enum E_CoverNodeType
		{
			E_CoverNodeType_HighWall,
			E_CoverNodeType_LowWall,
			E_CoverNodeType_LowStepOutLeft,
			E_CoverNodeType_LowStepOutRight,
			E_CoverNodeType_HighStepOutLeft,
			E_CoverNodeType_HighStepOutRight,
		};

		virtual const char*		GetTypeName() const;

		static const char*		GetStaticTypeName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		virtual E_ObjType GetType() const;

		virtual bool IsA(E_ObjType type) const;

		virtual bool IsVisible() const;

		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

		E_CoverNodeType GetCoverNodeType() const;

		const I_DgNode* GetLinkedNode() const;

		C_StaticPlug<MathsStorage::Matrix44>& GetTargetWorldMatrixPlug();

		const C_StaticPlug<MathsStorage::Matrix44>& GetTargetWorldMatrixPlug() const;

	protected:
		C_DgCoverNodePropNode();
		virtual ~C_DgCoverNodePropNode();

		C_DgCoverNodePropNode(const C_DgCoverNodePropNode& rhs);
		const C_DgCoverNodePropNode& operator=(const C_DgCoverNodePropNode& rhs);

		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

		static void OnTargetPlugChanged(I_Plug* pPlug);

		union CoverNodeDirtyFlags
		{
			struct
			{
				bool targetChanged : 1;
			};
			U32 data;
		};

		CoverNodeDirtyFlags	m_coverNodeDirtyFlags;
		E_CoverNodeType		m_coverNodeType;
		C_StaticPlug<MathsStorage::Matrix44>*	
							m_pTargetWorldMatrixPlug;
	};
}

#endif
