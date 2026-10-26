#ifndef EDITOR_PROPS_DG_Poly_PROP_NODE_H
#define EDITOR_PROPS_DG_Poly_PROP_NODE_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Dg/DgXtPropNodeBase.h>

namespace EditorLib
{
	struct C_CollisionSpline;
	struct C_InternalCollisionInstance;

	class C_XtDirty;

	class C_DgPolyPropNode : public C_DgXtPropNodeBase
	{
	// From C_XtObject
	public:
		virtual C_Status XtInitialise(
			C_XtLevel* pLevel, 
			LarvaType* pLarvaType, 
			Xt::DomElement* pData);

	// From I_DgObject
	public:
		virtual E_ObjType GetType() const;

		virtual const char*		GetTypeName() const;

		virtual bool IsA(E_ObjType type) const;

	// From I_DgNode
	public:
		virtual bool Update(C_UpdateHintCollection* pHc = 0);

	// From I_DgXtActorPropNode
	public:
		virtual bool IsVisible() const;

	//From C_DgPolyPropNode
	public:
		static const char*		GetStaticTypeName();

		static const char*		GetStaticDisplayName();

		static E_ObjType		GetStaticType();

		static void*			Create();

		C_Status XtSave();
		C_Status Apply();
		C_Status AddPoint(const Maths::HVec3& position);
		C_Status DeletePoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds);
		Ut::Vector<U32> InsertPoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds);

		U32 GetNumComponents() const;
		Maths::HVec3 GetComponent(U32 index) const;
		void SetComponent(U32 index, const Maths::HVec3& pos);

		float GetHeight() const;

	// From C_XtObject
	protected:
		virtual void OnXtDomMonitorEvent(
			Xt::DomMonitor* pMonitor, 
			Xt::DomEvent* pEvent,
			bool typeChanged);

	// From C_DgPolyPropNode
	protected:
		C_DgPolyPropNode();
		virtual ~C_DgPolyPropNode();

		C_DgPolyPropNode(const C_DgPolyPropNode& rhs);
		const C_DgPolyPropNode& operator=(const C_DgPolyPropNode& rhs);

		void OnPolyChanged();

		void UpdateCollisionSource();

		void UpdateCollisionInstance();

		Btl::Vector<Maths::HVec3>		m_polyPoints;
		float							m_height;

		C_InternalCollisionInstance*	m_pCollisionInstance;
		C_CollisionSpline*				m_pCollisionSpline;
	};
}

#endif
